#include "hg/runtime.hpp"
#include "hg/ipu_dct_tables.hpp"
#include "hg/ipu_dct_lookup.hpp"
#include <iostream>
#include <cmath>
#include <type_traits>
#define CHECK(x) do { if (!(x)) { std::cerr << "Failed line " << __LINE__ << ": " << #x << '\n'; return 1; } } while (0)
int main() {
    {
        hg::State fast,reference;
        fast.protect_kernel_memory=reference.protect_kernel_memory=true;
        for(auto alias:{0u,0x20000000u,0x80000000u,0xa0000000u})
        for(auto base:{0x100003u,0x198cbcfu,0x1ffffffu,0x2000000u,0x90000u,0x70000000u})
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
        for(auto base:{0x100000u,0x100001u,0x1fffff8u,0x2000000u,0x198cbc8u,0x90000u,0x70000000u,0x1000b000u}) {
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
    s.vif1.pending={1,2,3};s.vif1.pending_phase=12;s.vif1.cycle=0x0201;
    s.vif1.row[0]=17;s.vif1.path3_masked=true;s.vif1.vu_mem[0]=0x12345678;
    s.vif1.vu_micro_mem[0]=0xabcdef;
    s.store(0xb0003c10,4,1);
    CHECK(s.vif1.pending.empty() && s.vif1.pending_phase==0 && s.vif1.cycle==0);
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
