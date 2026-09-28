#include "hg/runtime.hpp"
#include "hg/diagnostic_input.hpp"
#include "hg/keyboard_input.hpp"
#include "controls_panel_regression.hpp"
#include "hg/vu_xyz.hpp"
#if defined(_M_X64) || defined(__SSE2__)
#include <xmmintrin.h>
#endif
#include "gif_stream_regression.hpp"
#include "vif_unpack_regression.hpp"
#include "vif_direct_regression.hpp"
#include "vif_dma_regression.hpp"
#include "fpu_madd4_er_regression.hpp"
#include "vu_specialization_regression.hpp"
#include "hg/ipu_dct_tables.hpp"
#include "hg/ipu_dct_lookup.hpp"
#include <iostream>
#include <cmath>
#include <type_traits>
#include <limits>
#include <cstring>
#include <memory>
#define CHECK(x) do { if (!(x)) { std::cerr << "Failed line " << __LINE__ << ": " << #x << '\n'; return 1; } } while (0)
static int packed_word_extension_tests() {
    hg::State s;
    std::array<hg::Register,32> original{};
    for(unsigned n=0;n<32;++n)
        original[n]={0x8123456700000000ull+n*0x100000001ull+0x76543210ull,
                     0xfedcba9800000000ull-n*0x100000001ull+0x01234567ull};
    s.hi=11;s.lo=22;s.hi1=33;s.lo1=44;s.sa=55;
    s.vu0.status=0x123;s.vu0.mac=0x456;
    for(bool upper:{false,true})
    for(unsigned source=0;source<32;++source)
    for(unsigned other=0;other<32;++other)
    for(unsigned destination=0;destination<32;++destination) {
        s.gpr=original;
        auto expected=original;
        std::array<std::uint32_t,4> a{},b{},out{};
        for(unsigned lane=0;lane<4;++lane) {
            if(source)a[lane]=std::uint32_t((lane<2?original[source].lo:original[source].hi)>>((lane%2)*32));
            if(other)b[lane]=std::uint32_t((lane<2?original[other].lo:original[other].hi)>>((lane%2)*32));
        }
        for(unsigned lane=0;lane<4;++lane)
            out[lane]=(lane%2?a:b)[(upper?2:0)+lane/2];
        if(destination)expected[destination]={out[0]|(std::uint64_t(out[1])<<32),
                                             out[2]|(std::uint64_t(out[3])<<32)};
        s.extend_words(destination,source,other,upper);
        for(unsigned n=0;n<32;++n)
            CHECK(s.gpr[n].lo==expected[n].lo && s.gpr[n].hi==expected[n].hi);
        CHECK(s.hi==11 && s.lo==22 && s.hi1==33 && s.lo1==44 && s.sa==55);
        CHECK(s.vu0.status==0x123 && s.vu0.mac==0x456);
    }
    // Walk every input bit to distinguish all source lanes and discarded halves.
    for(bool upper:{false,true})for(unsigned source:{1u,2u})for(unsigned bit=0;bit<128;++bit) {
        s.gpr[1]={};s.gpr[2]={};s.gpr[3]=original[3];
        (bit<64?s.gpr[source].lo:s.gpr[source].hi)=std::uint64_t(1)<<(bit%64);
        s.extend_words(3,1,2,upper);
        hg::Register expected{};
        if((bit>=64)==upper) {
            const unsigned output_bit=((bit%64)/32*2+(source==1?1:0))*32+bit%32;
            (output_bit<64?expected.lo:expected.hi)=std::uint64_t(1)<<(output_bit%64);
        }
        CHECK(s.gpr[3].lo==expected.lo && s.gpr[3].hi==expected.hi);
    }
    return 0;
}

static int vu_broadcast_product_tests() {
    using Op=hg::VuBroadcastProduct;
    const auto bits=[](float value){std::uint32_t out;std::memcpy(&out,&value,sizeof(out));return out;};
    const auto pack=[](const std::array<std::uint32_t,4>& a) {
        return hg::Register{std::uint64_t(a[0])|(std::uint64_t(a[1])<<32),
                            std::uint64_t(a[2])|(std::uint64_t(a[3])<<32)};
    };
    const auto lane=[](hg::Register v,unsigned n){return std::uint32_t((n<2?v.lo:v.hi)>>((n&1)*32));};
    const std::array<float,4> source={1,-2,4,-8},scalar={2,4,8,16};
    const std::array<std::uint32_t,4> original_acc={bits(10),bits(10),bits(10),bits(10)};
    const auto a=pack({bits(1),bits(-2),bits(4),bits(-8)});
    const auto b=pack({bits(2),bits(4),bits(8),bits(16)});
    const auto sentinel=pack({bits(20),bits(21),bits(22),bits(23)});
    hg::State s;
    for(const auto operation:{Op::multiply_accumulator,Op::add_accumulator,Op::add_vector})
    for(unsigned bc=0;bc<4;++bc)for(unsigned mask=0;mask<16;++mask) {
        s.vu0={};s.vu0.vf[1]=a;s.vu0.vf[2]=b;s.vu0.vf[3]=sentinel;
        s.vu0.acc=original_acc;s.vu0.mac=0xffff;s.vu0.status=0xc30;
        s.vu_broadcast_product(3,1,2,mask,bc,operation);
        unsigned expected_mac=0,expected_flags=0;
        for(unsigned n=0;n<4;++n) {
            const bool active=(mask&(8u>>n))!=0;
            // Small integral operands have exact products/sums, independently
            // checking all selectors without using the runtime FPU as oracle.
            const float result=source[n]*scalar[bc]+(operation==Op::multiply_accumulator?0:10);
            const auto expected=bits(result);
            CHECK(s.vu0.acc[n]==(active && operation!=Op::add_vector?expected:original_acc[n]));
            CHECK(lane(s.vu0.vf[3],n)==(active && operation==Op::add_vector?expected:lane(sentinel,n)));
            if(active && result<0){expected_mac|=1u<<(7-n);expected_flags|=2;}
            if(active && result==0){expected_mac|=1u<<(3-n);expected_flags|=1;}
        }
        CHECK(s.vu0.mac==expected_mac && s.vu0.status==(0xc30|expected_flags|(expected_flags<<6)));
    }
    for(unsigned destination:{1u,2u}) {
        s.vu0={};s.vu0.vf[1]=a;s.vu0.vf[2]=b;s.vu0.acc=original_acc;
        s.vu_broadcast_product(destination,1,2,15,0,Op::add_vector);
        for(unsigned n=0;n<4;++n)CHECK(lane(s.vu0.vf[destination],n)==bits(10+2*source[n]));
        CHECK(s.vu0.acc==original_acc);
    }
    // Architectural VF0 is constant even when its backing storage is poisoned.
    s.vu0={};s.vu0.vf[0]={~0ull,~0ull};
    s.vu_broadcast_product(0,0,0,15,3,Op::multiply_accumulator);
    CHECK(s.vu0.acc[0]==0 && s.vu0.acc[1]==0 && s.vu0.acc[2]==0 && s.vu0.acc[3]==bits(1));
    CHECK(s.vu0.mac==14 && s.vu0.status==0x41);
    s.vu_broadcast_product(0,0,0,15,3,Op::add_vector);
    CHECK(s.vu0.vf[0].lo==~0ull && s.vu0.vf[0].hi==~0ull && s.vu0.acc[3]==bits(1));
    CHECK(s.vu0.mac==14);
    // MADD's normal flags describe its sum, not a negative or zero product.
    s.vu0={};s.vu0.vf[1]=pack({bits(-1),0,0,0});s.vu0.vf[2]=pack({bits(1),0,0,0});
    s.vu0.acc=original_acc;s.vu0.status=0xc30;
    s.vu_broadcast_product(3,1,2,12,0,Op::add_vector);
    CHECK(s.vu0.mac==0 && s.vu0.status==0xc30);
    CHECK(lane(s.vu0.vf[3],0)==bits(9) && lane(s.vu0.vf[3],1)==bits(10));
    s.vu0={};s.vu0.vf[1]=pack({0x80000001,0x80000000,bits(-2),0});
    s.vu0.vf[2]=pack({bits(1),0,0,0});s.vu0.status=0xc30;
    s.vu_broadcast_product(0,1,2,15,0,Op::multiply_accumulator);
    CHECK(s.vu0.acc[0]==0x80000000 && s.vu0.acc[1]==0x80000000 && s.vu0.acc[2]==bits(-2));
    CHECK(s.vu0.mac==0x2d && s.vu0.status==0xcf3); // Negative zero has Z but no S.
    s.vu_broadcast_product(0,1,2,0,0,Op::add_accumulator);
    CHECK(s.vu0.mac==0 && s.vu0.status==0xcf0); // Empty mask clears current flags only.
    s.vu0={};s.vu0.vf[1]=pack({0x7fffffff,0,0,0});s.vu0.vf[2]=pack({bits(2),0,0,0});
    s.vu_broadcast_product(0,1,2,8,0,Op::multiply_accumulator);
    CHECK(s.vu0.acc[0]==0x7fffffff && s.vu0.mac==0x8000 && s.vu0.status==0x208);
    s.vu0={};s.vu0.vf[1]=pack({0x80800000,0,0,0});s.vu0.vf[2]=pack({bits(0.5f),0,0,0});
    s.vu_broadcast_product(0,1,2,8,0,Op::multiply_accumulator);
    CHECK(s.vu0.acc[0]==0x80000000 && s.vu0.mac==0x0808 && s.vu0.status==0x145);
    // Independent powers of two: min-normal * 0.5 underflows, and adding its
    // flushed signed zero leaves every nonzero normal ACC exactly unchanged.
    // Sony VU6.0 p42 requires sticky US, but not current U or MAC U.
    const std::array<std::uint32_t,4> underflow_acc={0x00800001,0x80800001,0x7f7fffff,0xff7fffff};
    for(const auto operation:{Op::add_accumulator,Op::add_vector})
    for(unsigned mask=0;mask<16;++mask)for(unsigned bc=0;bc<4;++bc)
    for(unsigned destination=0;destination<4;++destination)
    for(unsigned signs=0;signs<16;++signs)for(unsigned scalar_sign=0;scalar_sign<2;++scalar_sign)
    for(unsigned old_status:{0x30u,0xcf3u,0xffffu}) {
        std::array<std::uint32_t,4> small{},others={bits(1),bits(2),bits(4),bits(8)};
        for(unsigned n=0;n<4;++n)small[n]=0x00800000u|((signs&(1u<<n))?0x80000000u:0);
        others[bc]=0x3f000000u|(scalar_sign<<31);
        s.vu0={};s.vu0.vf[0]={~0ull,~0ull};s.vu0.vf[1]=pack(small);
        s.vu0.vf[2]=pack(others);s.vu0.vf[3]=sentinel;s.vu0.acc=underflow_acc;
        s.vu0.mac=0xffff;s.vu0.status=std::uint16_t(old_status);
        s.vu0.q=0x12345678;s.vu0.pending_q=0x76543210;s.vu0.q_pending=true;
        const auto before=s.vu0;const auto fpu_before=s.fpu;
        s.vu_broadcast_product(destination,1,2,mask,bc,operation);
        unsigned expected_mac=0,expected_current=0;
        for(unsigned n=0;n<4;++n)if(mask&(8u>>n)) {
            if(underflow_acc[n]&0x80000000u){expected_mac|=1u<<(7-n);expected_current|=2;}
        }
        CHECK(s.vu0.acc==before.acc && s.vu0.mac==expected_mac);
        CHECK(s.vu0.status==((old_status&~15u)|expected_current|
              ((expected_current|(mask?4u:0u))<<6)));
        for(unsigned reg=0;reg<32;++reg)for(unsigned n=0;n<4;++n) {
            const bool written=operation==Op::add_vector && destination && reg==destination && (mask&(8u>>n));
            CHECK(lane(s.vu0.vf[reg],n)==(written?underflow_acc[n]:lane(before.vf[reg],n)));
        }
        CHECK(s.vu0.q==before.q && s.vu0.pending_q==before.pending_q && s.vu0.q_pending);
        CHECK(s.fpu.acc==fpu_before.acc && s.fpu.control==fpu_before.control);
    }
    // Unsupported exceptional accumulation fails in the fourth lane, after earlier
    // lanes have computed, so the snapshot checks exercise atomic rejection.
    const std::array<std::array<std::uint32_t,3>,5> exceptions={{
        {0x7fffffff,0x40000000,0x3f800000}, // Product overflow.
        {0x00800000,0x3f000000,0x00000000}, // Product underflow with zero ACC remains unverified.
        {0x3f800000,0x3f800000,0x7f800000}, // Exceptional ACC exponent.
        {0x7f7fffff,0x40000000,0x7f7fffff}, // Sum overflow.
        {0x80800001,0x3f800000,0x00800000}, // Sum underflow.
    }};
    for(const auto operation:{Op::add_accumulator,Op::add_vector})for(const auto& example:exceptions) {
        s.vu0={};s.vu0.vf[1]=pack({bits(1),bits(2),bits(4),example[0]});
        s.vu0.vf[2]=pack({example[1],0,0,0});s.vu0.vf[3]=sentinel;
        s.vu0.acc=original_acc;s.vu0.acc[3]=example[2];s.vu0.mac=0x1234;s.vu0.status=0xc30;
        s.vu0.q=0x43210000;s.vu0.pending_q=0x45670000;s.vu0.q_pending=true;
        const auto before=s.vu0;const auto fpu_before=s.fpu;
        bool faulted=false;
        try{s.vu_broadcast_product(3,1,2,15,0,operation);}catch(const hg::Fault&){faulted=true;}
        CHECK(faulted && s.vu0.acc==before.acc && s.vu0.mac==before.mac && s.vu0.status==before.status);
        CHECK(s.vu0.q==before.q && s.vu0.pending_q==before.pending_q && s.vu0.q_pending);
        CHECK(s.fpu.acc==fpu_before.acc && s.fpu.control==fpu_before.control);
        for(unsigned n=0;n<32;++n)CHECK(s.vu0.vf[n].lo==before.vf[n].lo && s.vu0.vf[n].hi==before.vf[n].hi);
        s.vu_broadcast_product(3,1,2,14,0,operation); // Masked exceptional lane is not evaluated.
        CHECK(s.vu0.acc[3]==before.acc[3] && lane(s.vu0.vf[3],3)==lane(before.vf[3],3));
    }
    // A later unsupported lane must not publish an earlier newly supported US.
    for(const auto operation:{Op::add_accumulator,Op::add_vector})
    for(unsigned last_acc:{0u,0x80000000u,1u,0x80000001u,0x7f800000u,0xff800000u}) {
        s.vu0={};s.vu0.vf[1]=pack({0x00800000,0x00800000,0x00800000,0x00800000});
        s.vu0.vf[2]=pack({bits(0.5f),0,0,0});s.vu0.vf[3]=sentinel;
        s.vu0.acc=underflow_acc;s.vu0.acc[3]=last_acc;s.vu0.status=0x30;s.vu0.mac=0x1234;
        const auto before=s.vu0;bool faulted=false;
        try{s.vu_broadcast_product(3,1,2,15,0,operation);}catch(const hg::Fault&){faulted=true;}
        CHECK(faulted && s.vu0.acc==before.acc && s.vu0.mac==before.mac && s.vu0.status==before.status);
        for(unsigned reg=0;reg<32;++reg)
            CHECK(s.vu0.vf[reg].lo==before.vf[reg].lo && s.vu0.vf[reg].hi==before.vf[reg].hi);
    }
    return 0;
}
static int vu_conversion_tests() {
    hg::State s;
    const auto lane=[](hg::Register r,unsigned n){return std::uint32_t((n<2?r.lo:r.hi)>>((n&1)*32));};
    const auto pack=[](const std::array<std::uint32_t,4>& v){return hg::Register{
        std::uint64_t(v[0])|(std::uint64_t(v[1])<<32),std::uint64_t(v[2])|(std::uint64_t(v[3])<<32)};};
    const std::array<std::uint32_t,4> floats{0x3fc00000,0xbfc00000,0x4f000000,0xcf000000};
    const std::array<std::uint32_t,4> integers{0x01000001,0xfeffffff,0x7fffffff,0x80000000};
    for(bool to_float:{false,true})for(unsigned scale:{0u,4u,12u,15u})
    for(unsigned mask=0;mask<16;++mask)for(unsigned source:{0u,1u})for(unsigned dest:{0u,1u,2u}) {
        s.vu0.vf[0]={~0ull,~0ull};s.vu0.vf[1]=pack(to_float?integers:floats);
        s.vu0.vf[2]={0x123456789abcdef0ull,0xfedcba9876543210ull};
        s.vu0.status=0xabc;s.vu0.mac=0x5a5a;s.vu0.acc={1,2,3,4};s.vu0.q=0x1234;
        const auto before=s.vu0;const auto input=source?before.vf[source]:hg::Register{0,0x3f80000000000000ull};
        s.vu_convert(dest,source,mask,scale,to_float);
        for(unsigned reg=0;reg<32;++reg)for(unsigned n=0;n<4;++n) {
            auto expected=lane(before.vf[reg],n);
            if(reg==dest && dest && (mask&(8u>>n))) {
                const auto bits=lane(input,n);
                if(to_float) {
                    // Independent host conversion, correcting nearest rounding
                    // to the VU's toward-zero rule before exact power-of-two scaling.
                    const double exact=double(std::int64_t(bits&0x80000000u?std::int64_t(bits)-0x100000000ll:bits));
                    float value=float(exact);
                    if(std::abs(double(value))>std::abs(exact))value=std::nextafter(value,0.0f);
                    value=std::ldexp(value,-int(scale));std::memcpy(&expected,&value,4);
                } else {
                    float value;std::memcpy(&value,&bits,4);
                    const auto exact=std::trunc(std::ldexp(double(value),int(scale)));
                    expected=exact>=2147483647.0?0x7fffffffu:exact<=-2147483648.0?0x80000000u:std::uint32_t(std::int64_t(exact));
                }
            }
            CHECK(lane(s.vu0.vf[reg],n)==expected);
        }
        CHECK(s.vu0.status==before.status && s.vu0.mac==before.mac && s.vu0.acc==before.acc && s.vu0.q==before.q);
    }
    // Exponent-zero inputs flush to zero; VU exponent255 is a finite range,
    // so both signs saturate, rather than following host NaN conversion rules.
    s.vu0.vf[1]=pack({0x007fffff,0x807fffff,0x7fffffff,0xffffffff});
    s.vu_convert(1,1,15,15,false);
    CHECK(s.vu0.vf[1].lo==0 && s.vu0.vf[1].hi==0x800000007fffffffull);
    return 0;
}
static int keyboard_input_tests() {
    using Key=hg::KeyboardKey;
    const auto index=[](Key key){return static_cast<std::size_t>(key);};
    // Explicit physical-key expectations, independent of the production bit loop.
    const std::array<Key,16> button_keys{{Key::backspace,Key::left_shift,Key::right_shift,Key::enter,
        Key::up,Key::right,Key::down,Key::left,Key::digit1,Key::digit3,Key::q,Key::e,
        Key::v,Key::x,Key::space,Key::c}};
    const std::array<unsigned,12> pressure_bits{{5,7,4,6,12,13,14,15,10,11,8,9}};
    const hg::HostInput neutral;
    for(unsigned held=0;held<65536;++held) {
        hg::KeyboardKeys keys{};
        for(unsigned bit=0;bit<16;++bit)keys[index(button_keys[bit])]=(held&(1u<<bit))!=0;
        const auto input=hg::keyboard_input(keys);
        CHECK(input.buttons==std::uint16_t(held^0xffffu));
        CHECK(input.axes==neutral.axes);
        for(unsigned pressure=0;pressure<12;++pressure)
            CHECK(input.pressures[pressure]==((held&(1u<<pressure_bits[pressure]))?255:0));
        CHECK(hg::keyboard_input(keys,false)==neutral);
    }
    const std::array<Key,8> direction_keys{{Key::j,Key::l,Key::i,Key::k,Key::a,Key::d,Key::w,Key::s}};
    for(unsigned held=0;held<256;++held) {
        hg::KeyboardKeys keys{};
        for(unsigned bit=0;bit<8;++bit)keys[index(direction_keys[bit])]=(held&(1u<<bit))!=0;
        const auto input=hg::keyboard_input(keys);
        CHECK(input.buttons==neutral.buttons && input.pressures==neutral.pressures);
        for(unsigned axis=0;axis<4;++axis) {
            const unsigned pair=(held>>(axis*2))&3;
            CHECK(input.axes[axis]==(pair==1?0:pair==2?255:128));
        }
        CHECK(hg::keyboard_input(keys,false)==neutral);
        // Both triggers and face/shoulder buttons can coexist with both sticks.
        keys[index(Key::digit1)]=keys[index(Key::digit3)]=keys[index(Key::space)]=keys[index(Key::e)]=true;
        const auto combined=hg::keyboard_input(keys);
        CHECK(combined.axes==input.axes && combined.buttons==std::uint16_t(0xffffu^0x4b00u));
        CHECK(combined.pressures[10]==255 && combined.pressures[11]==255);
        CHECK(combined.pressures[6]==255 && combined.pressures[9]==255);
        keys.fill(false);
        CHECK(hg::keyboard_input(keys)==neutral);
    }
    std::cout<<"Keyboard input: 65536 button combinations and 256 stick combinations passed\n";
    return 0;
}

static int gif_qword_serialization_tests() {
    auto gs=std::make_unique<hg::GsRegisterState>();
    hg::GifPath path;
    std::vector<std::uint8_t> expected;
    const auto append_reference=[&](std::uint64_t low,std::uint64_t high) {
        for(unsigned n=0;n<8;++n)expected.push_back(std::uint8_t(low>>(8*n)));
        for(unsigned n=0;n<8;++n)expected.push_back(std::uint8_t(high>>(8*n)));
    };
    std::uint64_t random=0x76543210fedcba98ull;
    const auto next=[&]() {random=random*6364136223846793005ull+1442695040888963407ull;return random;};
    for(unsigned index=0;index<4096;++index) {
        path.write_control(1);expected.clear();
        const auto prefix=index%33;
        if(!prefix) {
            const auto tag=(2ull<<58)|(1ull<<15)|2ull; // EOP IMAGE, missing its last qword.
            path.submit_qword(tag,0,*gs);append_reference(tag,0);
        } else {
            // Deliberately nonaligned transport offsets. NLOOP is at least255,
            // so these synthetic prefixes cannot complete during one append.
            path.pending.assign(prefix,0);path.pending[0]=0xff;
            if(prefix>1)path.pending[1]=0x7f;
            expected=path.pending;
        }
        const auto low=index<64?1ull<<index:next();
        const auto high=index<64?~low:next();
        path.submit_qword(low,high,*gs);append_reference(low,high);
        CHECK(path.pending==expected);
    }
    path.write_control(1);expected.clear();
    const auto tag=(2ull<<58)|(1ull<<15)|513ull;
    path.submit_qword(tag,0,*gs);append_reference(tag,0);
    for(unsigned index=0;index<512;++index) {
        const auto low=next(),high=next();
        path.submit_qword(low,high,*gs);append_reference(low,high);
        CHECK(path.pending==expected); // Covers repeated growth without early GS application.
    }
    path.write_control(1);CHECK(path.pending.empty());
    path.submit_qword(1ull<<15,0,*gs);CHECK(path.pending.empty()); // Empty EOP still completes.
    path.submit_qword((2ull<<58)|(1ull<<15)|1ull,0,*gs);
    bool fault=false;
    try {path.submit_qword(0,0,*gs);} catch(const std::runtime_error&) {fault=true;}
    CHECK(fault && path.pending.size()==32); // No configured IMAGE destination; retain original fault and bytes.
    std::cout<<"GIF qword serialization: 4608 byte-exact cases plus reset/EOP/fault checks passed\n";
    return 0;
}
static int xyz_combined_integer_tests() {
    // The member methods with mask14 retain the original scalar implementation;
    // their mask15 accelerator is unreachable in this reference comparison.
    const std::array<std::uint32_t,16> edges{0u,0x80000000u,1u,0x807fffffu,
        0x00800000u,0x80800000u,0x3f800000u,0xbf800000u,0x3f7fffffu,0x3f800001u,
        0x7f000000u,0xff000000u,0x7f7fffffu,0xff7fffffu,0x7fffffffu,0xffffffffu};
    std::uint32_t seed=0x5631595au;
    const auto next=[&](){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;};
#if defined(_M_X64) || defined(__SSE2__)
    struct RestoreCsr {unsigned value=_mm_getcsr();~RestoreCsr(){_mm_setcsr(value);}} restore;
    constexpr unsigned modes=16;
#else
    constexpr unsigned modes=1;
#endif
    std::uint64_t cases=0;
    for(unsigned mode=0;mode<modes;++mode) {
#if defined(_M_X64) || defined(__SSE2__)
        const auto csr=(restore.value&~0xe040u)|((mode&3u)<<13)|((mode&4u)?0x8000u:0u)|((mode&8u)?0x40u:0u);
        _mm_setcsr(csr);
#endif
        for(unsigned sample=0;sample<4096;++sample)for(unsigned op=0;op<3;++op) {
            hg::Vu1State expected;
            for(unsigned reg=0;reg<3;++reg) {
                for(auto& bits:expected.vf[reg])bits=(sample&1)?next():edges[next()%edges.size()];
                expected.vf_defined[reg]=std::uint8_t(sample%8?0xa7u:next());
            }
            for(auto& bits:expected.acc)bits=(sample&1)?next():edges[next()%edges.size()];
            expected.acc_defined=std::uint8_t(sample%8?0xb7u:next());
            expected.status=next();expected.mac=next();expected.clip=next();
            expected.issue_cycle=(std::uint64_t(next())<<32)|next();
            expected.q=next();expected.p=next();expected.i=next();
            expected.pending_q=next();expected.pending_p=next();
            expected.q_pending=(sample&1)!=0;expected.p_pending=(sample&2)!=0;
            expected.q_cycles_remaining=sample%17;expected.p_cycles_remaining=sample%23;
            expected.vf_ready[1].fill(expected.issue_cycle+3);expected.vi_ready[1]=expected.issue_cycle+5;
            const unsigned source=sample%31==0?32:sample%17==0?0:1;
            const unsigned other=sample%37==0?32:sample%19==0?0:sample%11==0?1:2;
            const unsigned destination=sample%29==0?32:sample%4==3?31:sample%4;
            const unsigned broadcast=sample%41==0?4:sample%4;
            auto actual=expected;std::string expected_error,actual_error;
            try {
                if(op==2)expected.madd_vector(destination,source,other,14,broadcast);
                else expected.multiply_acc(source,other,14,broadcast,op==1);
            } catch(const std::runtime_error& error){expected_error=error.what();}
            try {
                if(op==2)hg::vu_xyz_madd(actual,destination,source,other,broadcast);
                else hg::vu_xyz_multiply_acc(actual,source,other,broadcast,op==1);
            } catch(const std::runtime_error& error){actual_error=error.what();}
            CHECK(expected_error==actual_error);
            CHECK(actual.vf==expected.vf && actual.vf_defined==expected.vf_defined);
            CHECK(actual.acc==expected.acc && actual.acc_defined==expected.acc_defined);
            CHECK(actual.mac==expected.mac && actual.status==expected.status && actual.clip==expected.clip);
            CHECK(actual.vi==expected.vi && actual.tpc==expected.tpc && actual.issue_cycle==expected.issue_cycle);
            CHECK(actual.vf_ready==expected.vf_ready && actual.vi_ready==expected.vi_ready);
            CHECK(actual.q==expected.q && actual.p==expected.p && actual.i==expected.i);
            CHECK(actual.pending_q==expected.pending_q && actual.pending_p==expected.pending_p);
            CHECK(actual.q_pending==expected.q_pending && actual.p_pending==expected.p_pending);
            CHECK(actual.q_cycles_remaining==expected.q_cycles_remaining && actual.p_cycles_remaining==expected.p_cycles_remaining);
#if defined(_M_X64) || defined(__SSE2__)
            CHECK(_mm_getcsr()==csr);
#endif
            ++cases;
        }
    }
    std::array<std::uint32_t,4> in{},acc{},out{};unsigned under=0,over=0;
    const bool packed_available=hg::try_fpu_madd4(in,0,acc,out,under,over);
    std::cout<<"XYZ integer MADD: "<<cases<<" scalar state/fault comparisons across "<<modes
             <<" host modes; packed_backend="<<packed_available<<'\n';
    return 0;
}
static int vu1_overlap_dirty_merge_tests() {
    {
        std::array<std::uint32_t,4096> live{},work{};
        std::array<std::uint8_t,1024> live_defined{},work_defined{};
        live_defined.fill(0x0f);work_defined=live_defined;
        hg::Vu1MemoryWriteTracker vif;
        work[5]=0x11111111u;work[3000]=0xaabbccddu;work_defined[750]=3;
        hg::merge_vu1_overlap_memory(live,live_defined,work,work_defined,vif);
        CHECK(live[5]==0x11111111u && live[3000]==0xaabbccddu);
        CHECK(live_defined[750]==3 && work_defined[750]==3);
    }
    {
        std::array<std::uint32_t,4096> live{},work{};
        std::array<std::uint8_t,1024> live_defined{},work_defined{};
        live_defined.fill(0x0f);work_defined=live_defined;
        hg::Vu1MemoryWriteTracker vif;
        vif.before_write(live,live_defined,9,2);live[9]=0x22222222u;
        hg::merge_vu1_overlap_memory(live,live_defined,work,work_defined,vif);
        CHECK(live[9]==0x22222222u && work[9]==0x22222222u);
    }
    {
        std::array<std::uint32_t,4096> live{},work{};
        std::array<std::uint8_t,1024> live_defined{},work_defined{};
        live_defined.fill(0x0f);work_defined=live_defined;
        hg::Vu1MemoryWriteTracker vif;
        vif.before_write(live,live_defined,12,3);
        live[12]=work[12]=0x33333333u;
        hg::merge_vu1_overlap_memory(live,live_defined,work,work_defined,vif);
        CHECK(live[12]==0x33333333u && work[12]==0x33333333u);
    }
    {
        std::array<std::uint32_t,4096> live{},work{};
        std::array<std::uint8_t,1024> live_defined{},work_defined{};
        hg::Vu1MemoryWriteTracker vif;
        vif.before_write(live,live_defined,16,4);live[16]=0x44u;live_defined[4]|=1u;
        work[17]=0x55u;work_defined[4]|=2u;
        hg::merge_vu1_overlap_memory(live,live_defined,work,work_defined,vif);
        CHECK(live[16]==0x44u && work[16]==0x44u && live[17]==0x55u && work[17]==0x55u);
        CHECK(live_defined[4]==3 && work_defined[4]==3);
    }
    {
        std::array<std::uint32_t,4096> live{},work{};
        std::array<std::uint8_t,1024> live_defined{},work_defined{};
        live_defined.fill(0x0f);work_defined=live_defined;
        hg::Vu1MemoryWriteTracker vif;
        vif.before_write(live,live_defined,20,5);
        live[20]=1;work[20]=2;bool rejected=false;
        try {hg::merge_vu1_overlap_memory(live,live_defined,work,work_defined,vif);}
        catch(const std::runtime_error& error) {rejected=std::string(error.what())=="VU1 overlap memory conflict";}
        CHECK(rejected);
    }
    {
        // VIF write tracking still captures the synchronized first-touch baseline.
        std::array<std::uint32_t,4096> memory{};
        std::array<std::uint8_t,1024> defined{};defined.fill(0x0f);
        hg::Vu1MemoryWriteTracker vif;
        memory[12]=0x12345678u;defined[3]=7;
        vif.before_write(memory,defined,12,3);
        memory[12]=0x87654321u;defined[3]=15;
        CHECK(vif.word_indices.size()==1 && vif.word_indices[0]==12 && vif.baseline_words[12]==0x12345678u);
        CHECK(vif.vector_indices.size()==1 && vif.vector_indices[0]==3 && vif.baseline_defined[3]==7);
    }
    std::cout<<"VU1 overlap VIF-only merge: VU-only/VIF-only/identical/disjoint/conflict/bulk-copy tracking passed\n";
    return 0;
}
static int vif_checked_word_tests() {
    // Frozen original expression, including .at exceptions on invalid spans.
    const auto original=[](const std::vector<std::uint8_t>& bytes,std::size_t at) {
        return std::uint32_t(bytes.at(at))|(std::uint32_t(bytes.at(at+1))<<8)|
               (std::uint32_t(bytes.at(at+2))<<16)|(std::uint32_t(bytes.at(at+3))<<24);
    };
    std::vector<std::uint8_t> bytes;bytes.reserve(2048);
    std::uint32_t seed=0x51f04b17u;std::uint64_t cases=0;
    const auto check=[&](std::size_t at) {
        std::uint32_t expected=0,actual=0;std::string first,second;
        bool expected_fault=false,actual_fault=false;
        try {expected=original(bytes,at);}catch(const std::out_of_range& error){expected_fault=true;first=error.what();}
        try {actual=hg::Vif1Path::word(bytes,at);}catch(const std::out_of_range& error){actual_fault=true;second=error.what();}
        ++cases;
        return expected_fault==actual_fault&&first==second&&(expected_fault||expected==actual);
    };
    for(std::size_t size=0;size<=1024;++size) {
        bytes.resize(size);
        for(auto& byte:bytes){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;byte=std::uint8_t(seed);}
        const auto before=bytes;
        for(std::size_t at=0;at<=size+4;++at)CHECK(check(at));
        for(std::size_t n=0;n<5;++n)CHECK(check((std::numeric_limits<std::size_t>::max)()-n));
        CHECK(before==bytes);
    }
    bytes={0x12,0x34,0x56,0x78};CHECK(hg::Vif1Path::word(bytes,0)==0x78563412u);
    std::cout<<"VIF word reads: "<<cases<<" value/fault comparisons including all offsets, extra capacity and SIZE_MAX passed\n";
    return 0;
}
int main() {
    CHECK(hg_test::run_vu_specialization_regression());
    CHECK(vif_dma_regression::run()==0);
    CHECK(vif_direct_regression::run()==0);
    CHECK(vif_unpack_regression::run()==0);
    CHECK(vif_checked_word_tests()==0);
    CHECK(vu1_overlap_dirty_merge_tests()==0);
    CHECK(xyz_combined_integer_tests()==0);
    CHECK(madd_er_regression());
    CHECK(gif_qword_serialization_tests()==0);
    CHECK(gif_stream_regression::run()==0);
    CHECK(keyboard_input_tests()==0);
    CHECK(controls_panel_regression()==0);
    {
        hg::HostInput pad;
        const std::string good="pad:49151,0,127,128,255,0,1,2,3,4,5,255,7,8,9,10,11";
        CHECK(hg::parse_diagnostic_pad(good,pad));
        CHECK(pad.buttons==49151 && pad.axes[0]==0 && pad.axes[3]==255);
        CHECK(pad.pressures[0]==0 && pad.pressures[6]==255 && pad.pressures[11]==11);
        const auto saved=pad;
        for(const auto bad:{"pad:","pad:65536,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0",
            "pad:0,256,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0",
            "pad:-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0",
            "pad:9999999999999999999999999999", "pad:0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0",
            "pad:0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,", "pad: 0", "none"}) {
            CHECK(!hg::parse_diagnostic_pad(bad,pad));CHECK(pad==saved);
        }
    }
    CHECK(vu_conversion_tests()==0);
    CHECK(packed_word_extension_tests()==0);
    CHECK(vu_broadcast_product_tests()==0);
    {
        hg::State fast,reference;
        fast.protect_kernel_memory=reference.protect_kernel_memory=true;
        for(auto alias:{0u,0x20000000u,0x80000000u,0xa0000000u})
        for(auto base:{0x100003u,0x198cbcfu,0x1ffffffu,0x2000000u,0x90000u,
                      0x6fffffffu,0x70000000u,0x7000000fu,0x70003ff0u,0x70003fffu,0x70004000u})
        for(unsigned reg:{0u,2u}) {
            const auto address=alias+base,aligned=address&~15u;
            fast.gpr[2]=reference.gpr[2]={0x81726354fedcba98ull,0x123456789abcdef0ull};
            bool a=false,b=false;
            try{fast.store_quad(address,reg);}catch(const hg::Fault&){a=true;}
            try{
                reference.memory(aligned,8);reference.memory(aligned+8,8);
                const auto v=reg?reference.gpr[reg]:hg::Register{};
                reference.store(aligned,8,v.lo);reference.store(aligned+8,8,v.hi);
            }catch(const hg::Fault&){b=true;}
            CHECK(a==b && fast.ram_write_trace_cursor==reference.ram_write_trace_cursor);
            a=b=false;
            try{fast.load_quad(address,reg);}catch(const hg::Fault&){a=true;}
            try{
                const hg::Register v{reference.load(aligned,8,false),reference.load(aligned+8,8,false)};
                if(reg)reference.gpr[reg]=v;
            }catch(const hg::Fault&){b=true;}
            CHECK(a==b && fast.gpr[reg].lo==reference.gpr[reg].lo && fast.gpr[reg].hi==reference.gpr[reg].hi);
        }
        CHECK(fast.ram==reference.ram && fast.scratch==reference.scratch);
    }
    const auto scalar_equivalence=[](auto width) {
        constexpr unsigned size=decltype(width)::value;
        hg::State fast,reference;fast.protect_kernel_memory=reference.protect_kernel_memory=true;
        for(auto alias:{0u,0x20000000u,0x30000000u,0x80000000u,0xa0000000u})
        for(auto base:{0x100000u,0x100001u,0x1fffff8u,0x2000000u,0x198cbc8u,0x90000u,
                      0x6fffffffu,0x70000000u,0x70000001u,0x70003ff8u,0x70003ffcu,
                      0x70003ffeu,0x70003fffu,0x70004000u,0x70004001u,0x1000b000u}) {
            const auto address=alias+base;bool first_fault=false,second_fault=false;
            try{fast.store_scalar<size>(address,0x81726354fedcba98ull);}catch(const hg::Fault&){first_fault=true;}
            try{reference.store(address,size,0x81726354fedcba98ull);}catch(const hg::Fault&){second_fault=true;}
            CHECK(first_fault==second_fault);
            for(bool sign:{false,true}) {
                std::uint64_t first=0,second=0;first_fault=second_fault=false;
                try{first=sign?fast.load_scalar<size,true>(address):fast.load_scalar<size,false>(address);}catch(const hg::Fault&){first_fault=true;}
                try{second=reference.load(address,size,sign);}catch(const hg::Fault&){second_fault=true;}
                CHECK(first_fault==second_fault && first==second);
            }
        }
        CHECK(fast.ram==reference.ram && fast.scratch==reference.scratch);
        CHECK(fast.ram_write_trace_cursor==reference.ram_write_trace_cursor);
        CHECK(fast.ram_write_trace_cursor>0);
        CHECK(fast.dmac.channels[3].chcr==reference.dmac.channels[3].chcr);
        return 0;
    };
    CHECK(scalar_equivalence(std::integral_constant<unsigned,1>{})==0);
    CHECK(scalar_equivalence(std::integral_constant<unsigned,2>{})==0);
    CHECK(scalar_equivalence(std::integral_constant<unsigned,4>{})==0);
    CHECK(scalar_equivalence(std::integral_constant<unsigned,8>{})==0);
    // Exhaustively retain every valid/invalid prefix and the original first
    // match in both numeric codebooks; sign bits are consumed by the caller.
    for(bool one:{false,true})for(unsigned prefix=0;prefix<65536;++prefix) {
        const auto& table=one?hg::ipu_dct_1:hg::ipu_dct_0;
        const hg::IpuDctCode* expected=nullptr;
        for(const auto& entry:table)if((prefix>>(16-entry.bits))==entry.code){expected=&entry;break;}
        CHECK(hg::lookup_ipu_dct(one,prefix<<16)==expected);
        CHECK(hg::lookup_ipu_dct(one,(prefix<<16)|0xffff)==expected);
    }
    {
        hg::State memory;memory.protect_kernel_memory=true;
        memory.kernel_code_regions={{0x90000,0x90004},{0x200000,0x210000}};
        for(auto alias:{0u,0x20000000u,0x30000000u,0x80000000u,0xa0000000u}) {
            memory.store(alias+0x100000,4,0x81234567);
            CHECK(memory.load(alias+0x100000,4,true)==0xffffffff81234567ull);
            memory.store(alias+0x90000,4,0x76543210);
            CHECK(memory.load(alias+0x90000,4,false)==0x76543210);
            for(auto address:{0x80000u,0x90004u}) {
                bool fault=false;try{memory.load(alias+address,4,false);}catch(const hg::Fault&){fault=true;}
                CHECK(fault);
                fault=false;try{memory.store(alias+address,4,0);}catch(const hg::Fault&){fault=true;}
                CHECK(fault);
            }
            bool span_fault=false;try{memory.load(alias+0x90000,8,false);}catch(const hg::Fault&){span_fault=true;}
            CHECK(span_fault);
        }
        memory.boot.table_ready=true;memory.store(0x7fffc,4,0x88776655);
        CHECK(memory.load(0x7fffc,4,false)==0x88776655);
        memory.store(0x70000000,8,0x123456789abcdef0ull);
        CHECK(memory.load(0x70000000,8,false)==0x123456789abcdef0ull);
        memory.protect_kernel_memory=false;memory.store(0x80000,4,0xabcdef01);
        CHECK(memory.load(0x80000,4,false)==0xabcdef01);
    }
    // Independent bit-by-bit reference across byte, 64-bit and qword edges.
    for(unsigned seed=0;seed<8;++seed) {
        hg::Ipu ipu;std::array<unsigned char,32> bytes{};
        for(unsigned n=0;n<bytes.size();++n) {
            bytes[n]=static_cast<unsigned char>((n*73+seed*151)^(n<<seed));
            auto& q=ipu.buffered[n/16];
            (n%16<8?q.low:q.high)|=std::uint64_t(bytes[n])<<((n%8)*8);
        }
        ipu.buffered_count=2;
        for(unsigned offset=0;offset<128;++offset)for(unsigned bits=0;bits<=32;++bits) {
            ipu.bit_position=offset;std::uint32_t expected=0;
            for(unsigned n=0;n<bits;++n)expected=(expected<<1)|((bytes[(offset+n)/8]>>(7-(offset+n)%8))&1);
            CHECK(ipu.peek(bits)==expected);CHECK(ipu.bit_position==offset && ipu.buffered_count==2);
        }
    }
    // H.262's DC-only inverse transform is constant (reconstructed DC / 8).
    // Cover all block destinations, precision scales, signs and clipping edges.
    for(bool intra:{false,true})for(unsigned precision=0;precision<4;++precision)
    for(int level:{-2048,-128,-4,-1,0,1,4,127,255,1023,2047})for(unsigned block=0;block<6;++block) {
        hg::Ipu ipu;ipu.pending=(intra?0x08000000u:0u)|0x10000u;ipu.settings=precision<<16;
        for(auto& q:ipu.non_intra_iq)q={0x1010101010101010ull,0x1010101010101010ull};
        ipu.coefficients[0]=level;ipu.block=block;ipu.pixels.fill(-1234);ipu.transform_block();
        const auto dc=std::clamp(intra?level*int(8>>precision):2*level+(level>0?1:level<0?-1:0),-2048,2047);
        const auto expected=std::clamp(int(std::floor(dc/8.0+0.5)),intra?0:-256,255);
        unsigned changed=0;for(auto sample:ipu.pixels)if(sample!=-1234){CHECK(sample==expected);++changed;}
        CHECK(changed==64);
    }
    // Compare sparse AC, dense and cancellation blocks to the full separable
    // mathematical transform. Generate zigzag positions geometrically, and
    // retain all eight terms in both reference passes, including zero tails.
    {
        unsigned zigzag[64]{},position=0;
        for(int diagonal=0;diagonal<=14;++diagonal) {
            const int low=std::max(0,diagonal-7),high=std::min(7,diagonal);
            for(int k=low;k<=high;++k) {
                const int y=diagonal%2?k:high-(k-low),x=diagonal-y;
                zigzag[y*8+x]=position++;
            }
        }
        double basis[8][8]{};
        for(unsigned x=0;x<8;++x)for(unsigned u=0;u<8;++u)
            basis[x][u]=(u?1.0:std::sqrt(0.5))*std::cos((2*x+1)*u*std::acos(-1.0)/16.0)/2.0;
        std::uint32_t seed=0x715ac321;
        for(bool intra:{false,true})for(unsigned precision=0;precision<4;++precision)
        for(unsigned pattern=0;pattern<160;++pattern) {
            hg::Ipu ipu;ipu.pending=(intra?0x08000000u:0u)|0x10000u;ipu.settings=precision<<16;
            for(auto& q:ipu.intra_iq)q={0x1010101010101010ull,0x1010101010101010ull};
            ipu.non_intra_iq=ipu.intra_iq;ipu.block=pattern%6;ipu.pixels.fill(-1234);
            int reconstructed[64]{};
            for(unsigned n=0;n<64;++n) {
                seed=seed*1664525u+1013904223u;
                const bool selected=pattern<64?n==pattern:pattern<128?n<=pattern-64:(seed&3)==0;
                const int level=selected?int((seed>>8)%4096)-2048:0;
                ipu.coefficients[zigzag[n]]=level;
                reconstructed[n]=std::clamp(intra?(n?2*level:level*int(8>>precision)):
                    2*level+(level>0?1:level<0?-1:0),-2048,2047);
            }
            double horizontal[8][8]{};
            for(unsigned v=0;v<8;++v)for(unsigned x=0;x<8;++x)
                for(unsigned u=0;u<8;++u)horizontal[v][x]+=reconstructed[v*8+u]*basis[x][u];
            auto expected=ipu.pixels;
            for(unsigned y=0;y<8;++y)for(unsigned x=0;x<8;++x) {
                double value=0;for(unsigned v=0;v<8;++v)value+=horizontal[v][x]*basis[y][v];
                const auto b=ipu.block;
                const unsigned dst=b<4?((b/2)*8+y)*16+(b%2)*8+x:256+(b-4)*64+y*8+x;
                expected[dst]=std::int16_t(std::clamp(int(std::floor(value+0.5+1e-9)),intra?0:-256,255));
            }
            ipu.transform_block();CHECK(ipu.pixels==expected);
        }
    }
    {
        hg::State s;s.store(0x1000,8,0x1122334455667788ull);s.store(0x1008,8,0x99aabbccddeeff00ull);
        s.w(2,0xfeedfacecafebeefull);s.load_double_unaligned(2,0x1007,true);s.load_double_unaligned(2,0x1000,false);
        CHECK(s.r(2)==0x1122334455667788ull);
        s.w(2,0);s.load_double_unaligned(2,0x100f,true);s.load_double_unaligned(2,0x1008,false);
        CHECK(s.r(2)==0x99aabbccddeeff00ull);
        s.store(0x1020,8,0);s.w(3,0x0123456789abcdefull);s.store_double_unaligned(0x1027,s.r(3),true);s.store_double_unaligned(0x1020,s.r(3),false);
        CHECK(s.load(0x1020,8,false)==0x0123456789abcdefull);
        s.store(0x1030,4,0x11223344);s.w(2,0);s.load_word_unaligned(2,0x1033,true);s.load_word_unaligned(2,0x1030,false);
        CHECK(s.r(2)==0x11223344);s.store(0x1034,4,0);s.w(3,0x55667788);s.store_word_unaligned(0x1037,std::uint32_t(s.r(3)),true);s.store_word_unaligned(0x1034,std::uint32_t(s.r(3)),false);
        CHECK(s.load(0x1034,4,false)==0x55667788);
    }
    hg::State s;
    // Byte-lane oracle: partial stores must replace selected bytes and preserve
    // every other byte. Exercise both widths at every possible alignment.
    for(unsigned width:{4u,8u})for(unsigned offset=0;offset<width;++offset) {
        const std::uint64_t value=0x8172635445362718ull;
        for(bool left:{false,true}) {
            for(unsigned n=0;n<16;++n)s.store(0x1100+n,1,0xe0+n);
            if(width==8)s.store_double_unaligned(0x1100+offset,value,left);
            else s.store_word_unaligned(0x1100+offset,std::uint32_t(value),left);
            for(unsigned n=0;n<16;++n) {
                const bool selected=n<width && (left?n<=offset:n>=offset);
                const unsigned source=left?width-1-offset+n:n-offset;
                const auto expected=selected?((value>>(8*source))&0xffu):0xe0+n;
                CHECK(s.load(0x1100+n,1,false)==expected);
            }
        }
        for(bool reverse:{false,true}) {
            for(unsigned n=0;n<24;++n)s.store(0x1100+n,1,0xa0+n);
            for(unsigned step=0;step<2;++step) {
                const bool left=(step==0)!=reverse;
                const auto address=0x1100+offset+(left?width-1:0);
                if(width==8)s.store_double_unaligned(address,value,left);
                else s.store_word_unaligned(address,std::uint32_t(value),left);
            }
            for(unsigned n=0;n<24;++n) {
                const auto expected=n>=offset && n<offset+width?
                    ((value>>(8*(n-offset)))&0xffu):0xa0+n;
                CHECK(s.load(0x1100+n,1,false)==expected);
            }
        }
    }
    CHECK(s.add_trace_watch(0x1234) && s.add_trace_watch(0x1234) && s.trace_watches.size()==1);
    s.pc=0x1234;s.w(2,0x22);s.w(4,0x44);s.w(31,0x88);s.trace_pc();
    CHECK(s.trace_watches[0].cursor==1 && s.trace_watches[0].history[0].pc==0x1234);
    CHECK(s.trace_watches[0].history[0].v0==0x22 && s.trace_watches[0].history[0].a0==0x44 && s.trace_watches[0].history[0].ra==0x88);
    s.store(0x20001234,4,0x12345678);
    CHECK(s.load(0x1234,4,false)==0x12345678 && s.load(0x30001234,4,false)==0x12345678);
    CHECK(hg::physical_address(0x22000000)==0x22000000 && hg::physical_address(0x32000000)==0x32000000);
    CHECK(hg::sx32(0x80000000u) == 0xffffffff80000000ull);
    CHECK(hg::sra32(0x80000000u, 0) == 0xffffffff80000000ull);
    CHECK(hg::sra32(0x80000000u, 31) == ~0ull);
    CHECK(hg::signed_less(~0ull, 0));
    CHECK(!hg::signed_less(0, ~0ull));
    s.w(0, 42); CHECK(s.r(0) == 0);
    s.gpr[1].hi = 123; s.w(1, 42); CHECK(s.gpr[1].hi == 123);
    s.gpr[2] = {55, 66}; s.pxor(2, 2, 2);
    CHECK(s.gpr[2].lo == 0 && s.gpr[2].hi == 0);
    s.gpr[1] = {0xfffffff000000003ull, 0x00000004ffffffffull};
    s.gpr[2] = {0x0000002000000005ull, 0x0000000100000001ull};
    s.padduw(1, 1, 2);
    CHECK(s.gpr[1].lo == 0xffffffff00000008ull);
    CHECK(s.gpr[1].hi == 0x00000005ffffffffull);
    s.padduw(0, 1, 2); CHECK(s.r(0) == 0);
    s.gpr[1]={0x12345678,0xabcdef01}; s.pcpy(1,0,1,0);
    CHECK(s.gpr[1].lo==0x5678567856785678ull && s.gpr[1].hi==0xef01ef01ef01ef01ull);
    s.gpr[1]={11,22}; s.gpr[2]={33,44}; s.pcpy(1,1,2,1);
    CHECK(s.gpr[1].lo==33 && s.gpr[1].hi==11);
    s.pcpy(2,1,2,2); CHECK(s.gpr[2].lo==11 && s.gpr[2].hi==44);
    s.cp0_status=0x10010; s.set_interrupts(false); CHECK(s.cp0_status==0x10010);
    s.cp0_status|=0x20000; s.set_interrupts(false); CHECK(s.cp0_status==0x20010);
    s.set_interrupts(true); CHECK(s.cp0_status==0x30010);
    s.cp0_status=0; s.set_interrupts(true); CHECK(s.cp0_status==0x10000);
    s.store(0x80000100, 4, 0x80000001);
    CHECK(s.load(0xa0000100, 4, true) == 0xffffffff80000001ull);
    CHECK(s.load(0x100, 4, false) == 0x80000001);
    s.store(0xb0001000,4,0x1234);
    CHECK(s.load(0x90001000,4,false)==0x1234);
    CHECK(s.load(0x10001000,4,false)==0x1234);
    s.intc.raise(11); CHECK(!s.intc.pending());
    s.store(0xb000f010,4,1u<<11); CHECK(s.intc.pending());
    CHECK(s.load(0x9000f000,4,false)==1u<<11);
    s.store(0x1000f000,4,0); CHECK(s.intc.pending());
    s.store(0x1000f000,4,1u<<11); CHECK(!s.intc.pending());
    s.store(0x1000f010,4,1u<<11); CHECK(s.intc.mask==0);
    s.store(0x1000f010,4,0xffff8000); CHECK(s.intc.mask==0);
    s.store(0x12000000,8,0x1234);CHECK(s.gs.privileged_pmode==0x1234);
    s.store(0x12002020,8,0x5678);CHECK(s.gs.privileged_smode2==0x5678); // Documented mirror.
    s.store(0xb2001010,8,0x1f00);CHECK(s.gs.privileged_imr==0x1f00); // KSEG1 alias.
    s.store(0x12001040,8,1);CHECK(s.gs.privileged_busdir==1);
    bool gs_fault=false;try{s.store(0x12001040,8,2);}catch(const hg::Fault&){gs_fault=true;}
    CHECK(gs_fault && s.gs.privileged_busdir==1);
    gs_fault=false;try{s.store(0x12000000,4,0);}catch(const hg::Fault&){gs_fault=true;}CHECK(gs_fault);
    gs_fault=false;try{(void)s.load(0x12001010,8,false);}catch(const hg::Fault&){gs_fault=true;}CHECK(gs_fault);
    s.gs.write_ad(0x62,(0xffff0000ull<<32)|0x12345678u);
    s.gs.write_ad(0x60,(0x00ffff00ull<<32)|0x00abcdefu);
    s.gs.write_ad(0x61,0);
    CHECK((s.load(0x12001000,8,false)&0x4003)==0x4003);
    CHECK(s.load(0x12001080,8,false)==0x1234000000abcd00ull);
    s.store(0x12001000,8,1);CHECK((s.load(0x12001000,8,false)&3)==2);
    s.gs.write_ad(0x60,(0xffffffffull<<32)|0x76543210u);
    s.store(0x12001010,8,0x1e00);CHECK((s.intc.status&1)!=0); // Unmask pending SIGNAL.
    s.store(0x1000f000,4,1);s.store(0x12001000,8,1);CHECK((s.intc.status&1)==0);
    gs_fault=false;try{s.store(0x12001000,8,1ull<<8);}catch(const hg::Fault&){gs_fault=true;}CHECK(gs_fault);
    // Synthetic bytes and recorded FDEC observations (docs/ORACLE.md).
    {
        hg::Ipu ipu;
        auto quad=[](unsigned start) {hg::IpuQuad q;for(unsigned n=0;n<8;++n){q.low|=std::uint64_t(start+n)<<(n*8);q.high|=std::uint64_t(start+n+8)<<(n*8);}return q;};
        ipu.command(0);for(unsigned n=0;n<8;++n)ipu.push(quad(n*16));
        CHECK(ipu.position()==0x10700 && ipu.control()==7);
        const unsigned fb[]={0,0,1,7,24,32,32,31,1,32,32};
        const unsigned expected[]={0x00010203,0x00010203,0x00020406,0x01020304,0x04050607,0x08090a0b,0x0c0d0e0f,0x88088909,0x10111213,0x14151617,0x18191a1b};
        for(unsigned n=0;n<11;++n){ipu.command(0x40000000|fb[n]);CHECK(!ipu.busy && ipu.read(0x10002000,8)==expected[n] && ipu.read(0x10002030,8)==expected[n]);if(n==7)CHECK(ipu.position()==0x2067f);if(n==8)CHECK(ipu.position()==0x10600);}
        ipu.command(0);ipu.command(0x40000000);
        CHECK(ipu.busy && (ipu.read(0x10002000,8)>>63)==1 && (ipu.read(0x10002030,8)>>63)==1);
        ipu.push(quad(0));CHECK(!ipu.busy && ipu.result==0x00010203 && ipu.position()==0x10000);
        ipu.command(120);ipu.push(quad(0));ipu.command(0x40000000);
        CHECK(ipu.busy && ipu.position()==0x10078);
        ipu.push(quad(16));CHECK(!ipu.busy && ipu.result==0x0f101112 && ipu.position()==0x20078);
        ipu.command(120);bool rejected=false;
        try{ipu.command(0x40000020);}catch(const std::runtime_error&){rejected=true;}
        CHECK(rejected && !ipu.busy && ipu.position()==120);
        ipu.command(0);for(unsigned n=0;n<8;++n)ipu.push(quad(n*16));
        ipu.command(0x50000000);CHECK(ipu.position()==0x10300 && ipu.intra_iq[3].high==quad(48).high);
        ipu.command(0x58000000);CHECK(ipu.position()==0 && ipu.non_intra_iq[0].low==quad(64).low);
        // Manual-derived non-byte-aligned table extraction, supplied piecemeal.
        ipu.command(0);ipu.command(0x50000001);CHECK(ipu.busy);
        for(unsigned n=0;n<5;++n)ipu.push(quad(n*16));
        CHECK(!ipu.busy && ipu.bit_position==1 && ipu.buffered_count==1);
        for(unsigned n=0;n<64;++n){auto q=ipu.intra_iq[n/16];auto lane=n%16<8?q.low:q.high;CHECK(((lane>>((n%8)*8))&255)==n*2);}
        ipu.command(0x60000000);CHECK(ipu.busy);ipu.push(quad(80));ipu.push(quad(96));
        CHECK(!ipu.busy && ipu.bit_position==1 && ipu.vq[0].low==0x8e8c8a8886848280ull);
        ipu.command(0);for(unsigned n=0;n<9;++n)ipu.push(quad(n*16));
        CHECK(ipu.input_count==8 && ipu.buffered_count==1);
        rejected=false;try{ipu.push(quad(144));}catch(const std::runtime_error&){rejected=true;}CHECK(rejected && ipu.input_count==8);
        for(unsigned command:{0x40000021u,0x40000040u,0x10000000u}){rejected=false;try{ipu.command(command);}catch(const std::runtime_error&){rejected=true;}CHECK(rejected);}
    }
    // Every hardware VLC table prefix, both MP1 profiles and all bit offsets.
    {
        auto verify=[](const auto& entries,unsigned command,unsigned settings) {
            for(const auto& entry:entries)for(unsigned offset=0;offset<128;++offset) {
                if(entry.mpeg1!=-1 && entry.mpeg1!=int((settings>>23)&1))continue;
                hg::Ipu ipu;ipu.settings=settings;ipu.command(offset);
                std::array<unsigned char,32> bytes{};
                for(unsigned bit=0;bit<entry.length;++bit)
                    if((entry.bits>>(entry.length-bit-1))&1)bytes[(offset+bit)/8]|=1u<<(7-(offset+bit)%8);
                for(unsigned q=0;q<2;++q){hg::IpuQuad quad;for(unsigned n=0;n<8;++n){quad.low|=std::uint64_t(bytes[q*16+n])<<(n*8);quad.high|=std::uint64_t(bytes[q*16+n+8])<<(n*8);}ipu.push(quad);}
                ipu.command(command);
                if(ipu.busy || ipu.result!=entry.result || ipu.bit_position!=(offset+(entry.result>>16))%128 || bool(ipu.status&0x4000)!=(entry.result==0))return false;
            }
            return true;
        };
        CHECK(verify(hg::ipu_address,0x30000000,0));CHECK(verify(hg::ipu_address,0x30000000,0x800000));
        CHECK(verify(hg::ipu_type_i,0x34000000,0x1000000));CHECK(verify(hg::ipu_type_p,0x34000000,0x2000000));
        CHECK(verify(hg::ipu_type_b,0x34000000,0x3000000));CHECK(verify(hg::ipu_type_d,0x34000000,0x4000000));
        CHECK(verify(hg::ipu_motion,0x38000000,0));CHECK(verify(hg::ipu_dmvector,0x3c000000,0));
        hg::Ipu ipu;ipu.command(0);ipu.command(0x30000000);CHECK(ipu.busy && (ipu.read(0x10002000,8)>>63));
        ipu.push({0x80,0});CHECK(!ipu.busy && ipu.result==0x10001 && ipu.bit_position==1 && ipu.top==0);
        // Independent examples: address increment3=010, escape=00000001000,
        // B-picture backward coded=011, motion -2=0011, dmv +1=10.
        for(auto row:std::array<std::array<unsigned,4>,5>{{{0x30000000,0,0x40,0x30003},{0x30000000,0,0x0001,0xb0023},{0x34000000,0x3000000,0x60,0x30006},{0x38000000,0,0x30,0x4fffe},{0x3c000000,0,0x80,0x20001}}}) {
            ipu.command(0);ipu.settings=row[1];ipu.push({row[2],0});ipu.command(row[0]);CHECK(ipu.result==row[3]);
        }
        ipu.command(100);ipu.settings=0;ipu.push({0,0x0000000800000000ull});ipu.command(0x30000000);CHECK(ipu.busy);
        ipu.push({0,0});CHECK(!ipu.busy && ipu.result==0x10001 && ipu.bit_position==101);
        ipu.command(96);ipu.push({0,0x0000008000000000ull});ipu.command(0x30000000);
        CHECK(ipu.busy && ipu.vlc_decoded && ipu.bit_position==97);
        ipu.push({0,0});CHECK(!ipu.busy && ipu.result==0x10001 && ipu.bit_position==97);
    }
    {hg::Ipu ipu;ipu.push_output({1,2});ipu.command(0);CHECK(ipu.output_count==1);ipu.reset();CHECK(ipu.output_count==0);}
    // MPEG2 intra blocks: literal encoded DC/EOB, backpressure and scan position.
    {
        auto run=[](hg::Ipu& ipu,const std::string& bits,unsigned settings,unsigned command=0x2c010000u) {
            ipu.settings=settings;ipu.command(0);
            for(auto& q:ipu.intra_iq)q={0x1010101010101010ull,0x1010101010101010ull};
            ipu.non_intra_iq=ipu.intra_iq;
            std::string padded=bits;while(padded.size()%128)padded+='0';
            ipu.command(command);
            unsigned offset=0,drained=0;
            while(offset<padded.size() || ipu.busy || ipu.output_count) {
                if(offset<padded.size() && ipu.input_count<8) {
                    hg::IpuQuad q;
                    for(unsigned bit=0;bit<128;++bit)if(padded[offset+bit]=='1') {
                        const unsigned byte=bit/8;auto& lane=byte<8?q.low:q.high;
                        lane|=1ull<<((byte%8)*8+7-bit%8);
                    }
                    ipu.push(q);offset+=128;
                }
                if(ipu.output_count){ipu.pop_output();++drained;}
                else if(offset==padded.size() && ipu.busy)return false;
            }
            return drained==48;
        };
        for(unsigned format:{0u,0x200000u})for(unsigned precision:{0u,1u,2u}) {
            hg::Ipu ipu;const std::string eob=format?"0110":"10";std::string bits;
            for(unsigned n=0;n<6;++n)bits+=(n<4?"100":"00")+eob;
            while(bits.size()%8)bits+='0';bits+="00000000000000000000000110110011";
            CHECK(run(ipu,bits,format|(precision<<16)));
            CHECK(ipu.status==0xbf00 && ipu.top==0x1b3);
            for(auto pixel:ipu.pixels)CHECK(pixel==128);
        }
        for(unsigned format:{0u,0x200000u}) {
            const auto& table=format?hg::ipu_dct_1:hg::ipu_dct_0;
            for(const auto& entry:table) {
                std::string code;for(unsigned n=0;n<entry.bits;++n)code+=((entry.code>>(entry.bits-1-n))&1)?'1':'0';
                std::string bits;for(unsigned n=0;n<6;++n)bits+=(n<4?"100":"00")+code+"1"+(format?"0110":"10");
                while(bits.size()%8)bits+='0';bits+="00000000000000000000000110110011";
                hg::Ipu ipu;CHECK(run(ipu,bits,format));CHECK(ipu.coefficients[entry.run+1]==-int(entry.level));
            }
        }
        hg::Ipu ipu;std::string escape;
        for(unsigned n=0;n<6;++n)escape+=(n<4?"100":"00")+std::string("00000100000011111111111110");
        while(escape.size()%8)escape+='0';escape+="00000000000000000000000110110011";
        CHECK(run(ipu,escape,0) && ipu.coefficients[1]==-1);
        // Analytic DC + first horizontal AC: rounded 128+64*cos*sqrt(2)/8.
        std::string ac;
        for(unsigned n=0;n<6;++n)ac+=(n<4?"100":"00")+std::string("0000010000000000010000000110");
        while(ac.size()%8)ac+='0';ac+="00000000000000000000000110110011";
        CHECK(run(ipu,ac,0x620000));
        const int row[]={139,137,134,130,126,122,119,117};
        for(unsigned y=0;y<16;++y)for(unsigned x=0;x<16;++x)CHECK(ipu.pixels[y*16+x]==row[x%8]);
        for(const auto& entry:hg::ipu_cbp)if(entry.mask) {
            std::string bits;for(unsigned n=entry.bits;n;--n)bits+=entry.code&(1u<<(n-1))?'1':'0';
            for(unsigned block=0;block<6;++block)if(entry.mask&(32u>>block))bits+="00000100000000000100000010";
            bits+=std::string(32,'1');hg::Ipu residual;
            CHECK(run(residual,bits,0x400000,0x20010000));CHECK(residual.status==(entry.mask<<8));
            for(unsigned block=0;block<6;++block)for(unsigned y=0;y<8;++y)for(unsigned x=0;x<8;++x) {
                unsigned n=block<4?((block/2)*8+y)*16+(block%2)*8+x:256+(block-4)*64+y*8+x;
                CHECK(residual.pixels[n]==(entry.mask&(32u>>block)?8:0));
            }
        }
        std::string negative="001100";for(unsigned b=0;b<6;++b)negative+="00000100000011101011010110";
        negative+=std::string(32,'1');hg::Ipu residual;CHECK(run(residual,negative,0x400000,0x20010000));
        for(auto value:residual.pixels)CHECK(value==-41);
        bool rejected=false;try{ipu.command(0x22010000);}catch(const std::runtime_error&){rejected=true;}CHECK(rejected);
    }
    {
        // Exhaust every RAW8 color against an independently expressed rational
        // reference. Powers of two are exact in double at these magnitudes.
        const auto reference=[](unsigned y,unsigned cb,unsigned cr,std::uint32_t thresholds) {
            const auto luma=std::floor(149.0*std::max(0,int(y)-16)/64.0);
            const double c=int(cb)-128,r=int(cr)-128;
            const auto channel=[](double value){return unsigned(std::clamp(std::floor((value+1.0)/2.0),0.0,255.0));};
            const auto red=channel(luma+std::floor(204.0*r/64.0));
            const auto green=channel(luma+std::floor(-50.0*c/64.0)+std::floor(-104.0*r/64.0));
            const auto blue=channel(luma+std::floor(258.0*c/64.0));
            const auto maximum=std::max({red,green,blue});
            if(maximum<(thresholds&511))return std::uint32_t(0);
            const auto alpha=maximum<((thresholds>>16)&511)?64u:128u;
            return red|(green<<8)|(blue<<16)|(alpha<<24);
        };
        for(unsigned cb=0;cb<256;++cb)for(unsigned cr=0;cr<256;++cr)for(unsigned y=0;y<256;++y)
            CHECK(hg::Ipu::rgba(y,cb,cr,0)==reference(y,cb,cr,0));
        for(unsigned y:{0u,1u,15u,16u,17u,127u,254u,255u,256u,300u})
        for(unsigned cb:{0u,1u,127u,128u,254u,255u,256u})
        for(unsigned cr:{0u,1u,127u,128u,254u,255u,256u}) {
            const auto color=reference(y,cb,cr,0);
            const auto maximum=std::max({color&255,(color>>8)&255,(color>>16)&255});
            for(auto low:{0u,maximum,maximum+1,256u,511u})
            for(auto high:{0u,maximum,maximum+1,256u,511u}) {
                const auto thresholds=low|(high<<16)|0xfe00fe00u;
                CHECK(hg::Ipu::rgba(y,cb,cr,thresholds)==reference(y,cb,cr,thresholds));
            }
        }
    }
    {
        // Independent CSC oracle samples: low luma, chroma clipping, green
        // half-bit rounding and transparency thresholds (docs/ORACLE.md).
        CHECK(hg::Ipu::rgba(0,12,250,0)==0x800000c2u);
        CHECK(hg::Ipu::rgba(2,69,73,0)==0x80004400u);
        CHECK(hg::Ipu::rgba(4,252,63,0)==0x80fa0400u);
        CHECK(hg::Ipu::rgba(127,22,18,0)==0x8000ff00u);
        CHECK(hg::Ipu::rgba(255,122,171,0)==0x80fff5ffu);
        CHECK(hg::Ipu::rgba(0,30,180,0x01000001)==0x40000053u);
        CHECK(hg::Ipu::rgba(127,38,198,0x00c00040)==0x80006bf1u);
        CHECK(hg::Ipu::rgba(16,128,128,1)==0);
        hg::Ipu ipu;ipu.command(0);ipu.command(0x70000002);
        CHECK(ipu.busy && !ipu.output_count);
        for(unsigned block=0;block<2;++block) {
            for(unsigned q=0;q<24;++q) {
                const auto value=q<16?(block?0xebebebebebebebebull:0x1010101010101010ull):0x8080808080808080ull;
                ipu.push({value,value});
                if(q<23)CHECK(!ipu.output_count);
            }
            CHECK(ipu.output_count==8 && ipu.busy && ipu.csc_pixel==32);
            for(unsigned q=0;q<64;++q) {
                CHECK(ipu.output_count);
                const auto color=block?0x80ffffff80ffffffull:0x8000000080000000ull;
                CHECK(ipu.output[0].low==color && ipu.output[0].high==color);
                ipu.pop_output();
            }
            CHECK(!ipu.output_count && ipu.busy==(block==0));
        }
        CHECK(ipu.position()==0 && !ipu.csc_remaining);
        for(auto cmd:{0x70000000u,0x78000001u,0x74000001u}) {
            bool rejected=false;try{ipu.command(cmd);}catch(const std::runtime_error&){rejected=true;}CHECK(rejected);
        }
    }
    // CSC lane consumption must match the byte-at-a-time stream at every bit
    // offset, including input starvation and output FIFO backpressure.
    for(unsigned offset=0;offset<128;++offset) {
        hg::Ipu fast,reference;
        fast.busy=reference.busy=true;fast.csc_remaining=reference.csc_remaining=2;
        fast.bit_position=reference.bit_position=offset;
        const auto slow=[](hg::Ipu& ipu) {
            while(ipu.csc_remaining) {
                while(ipu.csc_bytes<ipu.raw_pixels.size()) {
                    if(!ipu.ensure(8))return;
                    ipu.raw_pixels[ipu.csc_bytes++]=std::uint8_t(ipu.peek(8));ipu.advance(8);
                }
                while(ipu.csc_pixel<256) {
                    if(ipu.output_count==ipu.output.size())return;
                    hg::IpuQuad q;
                    for(unsigned lane=0;lane<4;++lane) {
                        const auto n=ipu.csc_pixel++,chroma=(n/32)*8+(n%16)/2;
                        const auto color=hg::Ipu::rgba(ipu.raw_pixels[n],ipu.raw_pixels[256+chroma],ipu.raw_pixels[320+chroma],ipu.thresholds);
                        (lane<2?q.low:q.high)|=std::uint64_t(color)<<((lane%2)*32);
                    }
                    ipu.push_output(q);
                }
                --ipu.csc_remaining;ipu.csc_bytes=ipu.csc_pixel=0;
            }
            ipu.busy=false;
        };
        unsigned supplied=0;
        for(unsigned tick=0;tick<2048 && fast.busy;++tick) {
            if(tick%3 && supplied<50 && fast.input_count<8) {
                const hg::IpuQuad q{0x137be02ad64f958cull*(supplied+1),0xaf841d39b750c26eull*(supplied+1)};
                fast.input[fast.input_count++]=q;reference.input[reference.input_count++]=q;++supplied;
            }
            fast.service_csc();slow(reference);
            CHECK(fast.position()==reference.position() && fast.busy==reference.busy);
            CHECK(fast.csc_remaining==reference.csc_remaining && fast.csc_bytes==reference.csc_bytes && fast.csc_pixel==reference.csc_pixel);
            CHECK(fast.raw_pixels==reference.raw_pixels && fast.output_count==reference.output_count);
            for(unsigned n=0;n<fast.output_count;++n)CHECK(fast.output[n].low==reference.output[n].low && fast.output[n].high==reference.output[n].high);
            for(unsigned n=0;n<fast.buffered_count;++n)CHECK(fast.buffered[n].low==reference.buffered[n].low && fast.buffered[n].high==reference.buffered[n].high);
            for(unsigned n=0;n<fast.input_count;++n)CHECK(fast.input[n].low==reference.input[n].low && fast.input[n].high==reference.input[n].high);
            if(tick%5 && fast.output_count) {
                for(unsigned n=1;n<fast.output_count;++n){fast.output[n-1]=fast.output[n];reference.output[n-1]=reference.output[n];}
                --fast.output_count;--reference.output_count;
            }
        }
        CHECK(!fast.busy && !reference.busy);
    }
    // IPU startup retains both quantizer matrices and the VQ table.
    s.store(0x10002010,4,0x40000000);CHECK(s.load(0x10002010,4,false)==0);
    for(unsigned n=0;n<8;++n){s.gpr[2]={0x100+n,0x200+n};s.store_quad(0x10007010,2);}
    CHECK((s.load(0x10002010,4,false)&15)==8);
    s.store(0x10002000,4,0x50000000);CHECK(s.ipu.input_count==3 && s.ipu.buffered_count==1 && s.ipu.intra_iq[0].low==0x100);
    s.store(0x10002000,4,0x58000000);CHECK(s.ipu.input_count==0 && s.ipu.non_intra_iq[3].high==0x207);
    for(unsigned n=0;n<2;++n){s.gpr[2]={0x300+n,0x400+n};s.store_quad(0x10007010,2);}
    s.store(0x10002000,4,0x60000000);CHECK(s.ipu.input_count==0 && s.ipu.vq[1].low==0x301);
    s.store(0x10002000,4,0x90120134);CHECK(s.ipu.thresholds==0x00120134);
    s.gpr[2]={1,2};s.store_quad(0x10007010,2);s.store(0x10002000,4,0x11);
    CHECK(s.ipu.input_count==0 && s.ipu.bit_position==0x11);
    s.store(0x70003ff8, 8, 0x123456789abcdef0ull);
    CHECK(s.load(0x70003ff8, 8, false) == 0x123456789abcdef0ull);
    for (auto a : {0x101u, 0x02000000u, 0xb0001001u, 0x70004000u, 0xfffffffcu}) {
        bool fault = false;
        try { s.load(a, 4, false); } catch (const hg::Fault&) { fault = true; }
        CHECK(fault);
    }
    CHECK(s.rasterize_gs_draws()==0); // Empty GS queue is an explicit no-op.
    s.vif1.pending={1,2,3};s.vif1.pending_phase=12;s.vif1.pending_phase_breaks={{1,8}};s.vif1.cycle=0x0201;
    s.vif1.row[0]=17;s.vif1.path3_masked=true;s.vif1.vu_mem[0]=0x12345678;
    s.vif1.vu_micro_mem[0]=0xabcdef;
    s.store(0xb0003c10,4,1);
    CHECK(s.vif1.pending.empty() && s.vif1.pending_phase==0 && s.vif1.pending_phase_breaks.empty() && s.vif1.cycle==0);
    CHECK(s.vif1.row[0]==0 && !s.vif1.path3_masked);
    CHECK(s.vif1.vu_mem[0]==0x12345678 && s.vif1.vu_micro_mem[0]==0xabcdef);
    s.store(0x10003c20,4,2);CHECK(s.load(0x10003c20,4,false)==2);
    bool bad_vif=false;try{s.store(0x10003c20,4,4);}catch(const hg::Fault&){bad_vif=true;}
    CHECK(bad_vif && s.vif1.error_mask==2);
    bad_vif=false;try{s.store(0x10003c10,8,1);}catch(const hg::Fault&){bad_vif=true;}
    CHECK(bad_vif && s.vif1.error_mask==2);
    s.store(0x10003c10,4,1);CHECK(s.vif1.error_mask==0);
    // CPU SQ must submit both halves to the same parser used by DMA.
    s.gpr[4]={0x0300000801000201ull,0x0700123402000004ull};
    s.store_quad(0xb0005000,4);
    CHECK(s.vif1.cycle==0x0201 && s.vif1.base==8 && s.vif1.offset==4);
    CHECK(s.vif1.mark==0x1234 && s.vif1.pending.empty());
    bool fifo_width=false;try{s.store(0x10005000,8,0);}catch(const hg::Fault&){fifo_width=true;}
    CHECK(fifo_width);
    s.store_quad(0x10005000,0);CHECK(s.vif1.mark==0x1234); // Four NOPs.
    // A VIF payload may span a source-chain TTE tag. The retained logical bytes
    // cross from qword phase 12 to tag phase 8, then back to qword phase 0.
    // Preserve those physical phases so later MPG/DIRECT alignment stays exact.
    {
        auto& v=s.vif1;v.reset_interface();v.vu_mem.fill(0);v.vu_mem_defined.fill(0x0f);

        const std::uint32_t unpack=0x6c010000u; // UNPACK V4-32, NUM=1, address 0.
        v.submit_qword(0,std::uint64_t(unpack)<<32,s.gif,s.gs);
        CHECK(v.pending.size()==4 && v.pending_phase==12 && v.pending_phase_breaks.empty());
        CHECK(v.pending_byte_phase(0)==12 && v.pending_byte_phase(4)==0);
        v.submit_dma_tag(std::uint64_t(0x11111111u)|(std::uint64_t(0x22222222u)<<32),s.gif,s.gs);
        CHECK(v.pending.size()==12 && v.pending_phase==12 && v.pending_phase_breaks.size()==1);
        CHECK(v.pending_phase_breaks[0].offset==4 && v.pending_phase_breaks[0].phase==8);
        CHECK(v.pending_byte_phase(4)==8 && v.pending_byte_phase(12)==0);
        const std::uint64_t payload=std::uint64_t(0x33333333u)|(std::uint64_t(0x44444444u)<<32);
        const std::uint64_t following=std::uint64_t(0x01000201u)|(std::uint64_t(0x03000008u)<<32);
        v.submit_qword(payload,following,s.gif,s.gs);
        CHECK(v.vu_mem[0]==0x11111111u && v.vu_mem[1]==0x22222222u);
        CHECK(v.vu_mem[2]==0x33333333u && v.vu_mem[3]==0x44444444u);



        CHECK(v.cycle==0x0201 && v.base==8);
        CHECK(v.pending.empty() && v.pending_phase==0 && v.pending_phase_breaks.empty());

    }
    s.gif.pending={1,2,3,4};s.gs.privileged_bgcolor=0x123456;
    s.store(0xb0003000,4,1);
    CHECK(s.gif.pending.empty() && s.gs.privileged_bgcolor==0x123456);
    s.gif.pending={9};
    bool bad_gif=false;try{s.store(0x10003000,4,8);}catch(const hg::Fault&){bad_gif=true;}
    CHECK(bad_gif && s.gif.pending.size()==1);
    bad_gif=false;try{s.store(0x10003000,8,1);}catch(const hg::Fault&){bad_gif=true;}
    CHECK(bad_gif && s.gif.pending.size()==1);
    s.store(0x10003000,4,1);
    s.store(0x12001040,8,0);s.vif1.consume_state(6,0);
    CHECK(s.load(0xb0003020,4,false)==0);
    s.vif1.consume_state(6,0x8000);
    CHECK(s.load(0x10003020,4,false)==2);
    s.store(0x12001040,8,1);
    CHECK(s.load(0x10003020,4,false)==0x1002);
    s.gif.pending={0};
    bad_gif=false;try{s.load(0x10003020,4,false);}catch(const hg::Fault&){bad_gif=true;}
    CHECK(bad_gif);
    s.store(0x10003000,4,1);
    bad_gif=false;try{s.load(0x10003020,8,false);}catch(const hg::Fault&){bad_gif=true;}
    CHECK(bad_gif);
    s.gs.vram.assign(1024*1024,0);s.gs.vram[17]=0x87654321;
    s.gs.privileged_imr=0;s.gs.privileged_csr_events=3;
    s.gs.gif_to_vram_active=true;s.gs.transfer_remaining=23;
    s.gs.vertex_queue.resize(1);s.gs.draws.resize(1);
    s.gs.crtc_configured=true;s.gs.crtc_mode=2;
    s.store(0x12001000,8,0x200);
    CHECK(s.gs.vram[17]==0x87654321 && s.gs.vram.size()==1024*1024);
    CHECK(s.gs.privileged_imr==0x1f00 && s.gs.privileged_csr_events==0);
    CHECK(!s.gs.gif_to_vram_active && s.gs.transfer_remaining==0);
    CHECK(s.gs.vertex_queue.empty() && s.gs.draws.empty() && !s.gs.crtc_configured);
    CHECK((s.load(0x12001000,8,false)&0x200)==0);
    s.vu0.vf[4]={0xc040000040000000ull,0x3f80000040800000ull}; // 2,-3,4,1
    s.vu_arithmetic(5,4,4,14,true,0);
    CHECK(s.vu0.vf[5].lo==0x4110000040800000ull && s.vu0.vf[5].hi==0x41800000);
    s.vu_arithmetic(5,5,5,8,false,1); // x=4+9; aliasing must read original y.
    CHECK(std::uint32_t(s.vu0.vf[5].lo)==0x41500000);
    s.vu_arithmetic(5,5,5,8,false,2); // x=13+16.
    CHECK(std::uint32_t(s.vu0.vf[5].lo)==0x41e80000 && s.vu0.mac==0);
    s.vu0.vf[7]={0x3e800000u,0x1122334455667788ull};
    s.vu_arithmetic(7,0,7,1,false,8); // VSUBx.w VF7,VF0,VF7x: 1 - 0.25.
    CHECK(s.vu0.vf[7].lo==0x3e800000u && std::uint32_t(s.vu0.vf[7].hi)==0x55667788u);
    CHECK(std::uint32_t(s.vu0.vf[7].hi>>32)==0x3f400000u);
    s.vu_arithmetic(6,0,0,15,true,0);
    CHECK(s.vu0.vf[6].lo==0 && s.vu0.vf[6].hi==0x3f80000000000000ull && s.vu0.mac==14);
    s.vu0.vf[7]={0x400000007fffffffull,0};
    s.vu_arithmetic(0,7,7,8,true,0);
    CHECK(s.vu0.mac==0x8000 && (s.vu0.status&0x208)==0x208);
    s.vu0.vf[7].lo=0x40000000;s.vu_sqrt(7,0);
    CHECK(s.vu0.q_pending && s.vu0.q==0);
    bool q_rejected=false;try{s.vu_arithmetic(5,0,0,8,false,4);}catch(const hg::Fault&){q_rejected=true;}CHECK(q_rejected);
    s.vu_waitq();CHECK(!s.vu0.q_pending && s.vu0.q==0x3fb504f3);
    s.vu_arithmetic(5,0,0,8,false,4);CHECK(std::uint32_t(s.vu0.vf[5].lo)==0x3fb504f3);
    s.vu0.vf[7].lo=0xc0800000;s.vu_sqrt(7,0);s.vu_waitq();
    CHECK(s.vu0.q==0x40000000 && (s.vu0.status&0x410)==0x410);
    s.vu_sqrt(0,0);s.vu_waitq();CHECK(s.vu0.q==0 && (s.vu0.status&0x30)==0 && (s.vu0.status&0x400));
    s.cp0_tag_lo=0xffffffff;s.store(0x1000,4,0x12345678);
    s.cache_load_tag(0xfc1);CHECK(s.cp0_tag_lo==0 && s.load(0x1000,4,false)==0x12345678);
    s.vu_control_write(28,0xc0c);CHECK(s.vu_control_read(28)==0xc0c);
    s.vu_control_write(28,0x202);CHECK(s.vu_control_read(28)==0 && s.vu0.mac==0 && s.vu0.status==0);
    CHECK(s.vu0.vf[7].lo==0xc0800000);
    CHECK(s.vu_control_read(29)==0);
    s.vu0.vf[4]={0x40800000,0x3f80000000000000ull}; // (4,0,0,1)
    s.vu_arithmetic(5,4,4,14,true,0);
    s.vu_arithmetic(5,5,5,8,false,1);s.vu_arithmetic(5,5,5,8,false,2);
    s.vu_sqrt(5,0);s.vu_waitq();s.vu_arithmetic(5,0,0,8,false,4);
    s.vu_divide(0,5,3,0);s.vu_arithmetic(6,0,0,15,false,5);s.vu_waitq();
    s.vu_arithmetic(6,4,0,14,true,4);
    CHECK(s.vu0.vf[6].lo==0x3f800000 && s.vu0.vf[6].hi==0);
    s.vu_divide(0,0,3,0);s.vu_waitq();CHECK(s.vu0.q==0x7fffffff && (s.vu0.status&0x30)==0x20);
    s.vu_divide(0,0,0,0);s.vu_waitq();CHECK(s.vu0.q==0x7fffffff && (s.vu0.status&0x30)==0x10);
    s.vu0.vf[4]={0xc000000040000000ull,0x3f80000040800000ull};
    s.vu0.vf[5]={0x40000000,0};s.vu_arithmetic(5,4,5,14,true,8);
    CHECK(s.vu0.vf[5].lo==0xc080000040800000ull && s.vu0.vf[5].hi==0x41000000);
    s.gpr[4]={~0ull,~0ull};s.qmtc2(4,0);s.lqc2(0x1000,0);s.qmfc2(4,0);
    CHECK(s.gpr[4].lo==0 && s.gpr[4].hi==0x3f80000000000000ull);
    s.vmove(5,0,15);CHECK(s.vu0.vf[5].lo==0 && s.vu0.vf[5].hi==0x3f80000000000000ull);
    s.sqc2(0x1020,0);CHECK(s.load(0x1028,8,false)==0x3f80000000000000ull);
    s.vu0.vf[4]={0x3f800000,0x3f80000000000000ull};s.vu0.vf[5]={0x3f80000000000000ull,0};
    s.vu_outer(0,4,5,false);s.vu_outer(4,5,4,true);
    CHECK(s.vu0.vf[4].lo==0 && s.vu0.vf[4].hi==0x3f8000003f800000ull);
    CHECK((s.vu0.mac&15)==12); // x/y zero, z positive, inactive w cleared.
    s.vu_arithmetic(4,4,4,15,false,6);
    CHECK(s.vu0.vf[4].hi==0x4000000040000000ull);
    return 0;
}
