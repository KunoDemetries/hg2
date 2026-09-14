#include "hg/runtime.hpp"
#include <iostream>
#define CHECK(x) do { if (!(x)) { std::cerr << "Failed line " << __LINE__ << ": " << #x << '\n'; return 1; } } while (0)
int main() {
    hg::Fpu f;
    CHECK(f.add(0, 0) == 0);
    CHECK(f.add(0x80000000, 0) == 0);
    CHECK(f.add(0x80000000, 0x80000000) == 0x80000000);
    CHECK(f.add(0x80000001, 0x80000100) == 0x80000000);
    CHECK(f.add(0x3f800000, 0x3f800000) == 0x40000000); // 1+1=2
    CHECK(f.add(0x3f800000, 0x3f000000) == 0x3fc00000); // 1+0.5=1.5
    CHECK(f.add(0x3f800000, 0xbf800000) == 0); // cancellation
    CHECK(f.add(0x3f800000, 0xbf000000) == 0x3f000000);
    CHECK(f.add(0xbf800000, 0x3f000000) == 0xbf000000);
    CHECK(f.add(0x7f800000, 0) == 0x7f800000); // exponent 255 is finite
    CHECK(f.add(0x7fffffff, 0x7fffffff) == 0x7fffffff);
    CHECK((f.control & 0x8010) == 0x8010);
    CHECK(f.add(0x00800001, 0x80800000) == 0); // positive underflow
    CHECK((f.control & 0xc018) == 0x4018); // O clears; SO stays
    CHECK(f.add(0x80800001, 0x00800000) == 0x80000000);
    f.write_control(~0u); CHECK(f.control == 0x0183c079);
    f.write_control(0); CHECK(f.control == 0x01000001);
    hg::State s; s.pc = 0x1234; s.fpr[1] = 0x3f800000; s.fpr[2] = 0x33800000;
    s.add_float(1, 2, 0, true);
    CHECK(s.fpu.acc == 0x3f800000); // No GRS bits: the aligned 2^-24 term is chopped.
    s.gpr[1] = {0x12345678, 0xabcdef01};
    CHECK(f.madd(0x42a00000, 0xbf000000) == 0xc2200000); // ACC is 0 after control-only tests: 80 * -0.5.
    f.acc=0xc1800000; CHECK(f.madd(0x42a00000,0xbf000000)==0xc2600000);
    CHECK(f.msub(0x42a00000,0xbf000000)==0x41c00000);
    f.acc=0;CHECK(f.madd(0x3f800001,0x3f800001)==0x3f800002); // Product low bit is chopped.
    CHECK(f.multiply(0x3fc00000,0x3fc00000)==0x40100000); // Wide normalization: 1.5 * 1.5.
    CHECK(f.multiply(0x7fffffff,0x40000000)==0x7fffffff && (f.control&0x8010u)==0x8010u);
    CHECK(f.multiply(0x00800000,0x3f000000)==0 && (f.control&0x4008u)==0x4008u);
    CHECK(f.multiply(0x80000001,0x3f800000)==0x80000000); // Exponent-zero signed zero.
    CHECK(f.divide(0x42a00000,0x3f000000)==0x43200000);
    CHECK(f.divide(0x3f800000,0x40400000)==0x3eaaaaaa); // 1/3, chopped (nearest would end AB).
    CHECK(f.divide(0xbf800000,0x40400000)==0xbeaaaaaa); // Toward zero for negatives too.
    CHECK(f.divide(0x40000000,0x40400000)==0x3f2aaaaa); // Other normalization branch.
    CHECK(f.divide(0x7f800000,0x3f800000)==0x7f800000); // EE exponent 255 is finite.
    f.write_control(0);
    CHECK(f.divide(0x3f800000,0x80000001)==0xffffffff && (f.control&0x3c078)==0x10020);
    CHECK(f.divide(0x80000000,0x80000100)==0x7fffffff && (f.control&0x3c078)==0x20060);
    CHECK(f.divide(0x80000001,0x3f800000)==0x80000000 && (f.control&0x3c078)==0x60);
    CHECK(f.divide(0x80800000,0x40000000)==0x80000000 && (f.control&0x3c078)==0x4068);
    CHECK(f.divide(0x7fffffff,0x3f000000)==0x7fffffff && (f.control&0x3c078)==0x8078);
    CHECK(f.divide(0x3f800000,0x3f800000)==0x3f800000 && (f.control&0x3c078)==0x78);
    CHECK(f.sqrt_exact(0x41100000)==0x40400000); // sqrt(9) = 3.
    CHECK(f.sqrt_exact(0x40800000)==0x40000000); // odd unbiased exponent: sqrt(4) = 2.
    CHECK(f.sqrt_exact(0x80000001)==0x80000000); // exponent-zero signed zero.
    CHECK(f.sqrt_exact(0x40000000)==0x3fb504f3); // Measured original startup input2.
    CHECK(f.sqrt_exact(0x3f000000)==0x3f3504f3); // Same significand, negative odd exponent.
    f.write_control(0x3c078);CHECK(f.sqrt_exact(0x40000000)==0x3fb504f3);
    CHECK(f.control==0x100c079); // Clear I/D causes; retain U/O and sticky flags.
    CHECK(f.sqrt_exact(0x40a00000)==0x400f1bbd); // Round upward, unlike sqrt(2).
    CHECK(f.sqrt_exact(0x3f800001)==0x3f800000);
    CHECK(f.sqrt_exact(0x00800000)==0x20000000);
    CHECK(f.sqrt_exact(0x7f7fffff)==0x5f7fffff);
    bool sqrt_fault=false;try {f.sqrt_exact(0xbf800000);}catch(const std::runtime_error&){sqrt_fault=true;}CHECK(sqrt_fault);
    CHECK(f.cvt_s_w_exact(123)==0x42f60000 && f.cvt_s_w_exact(0xffffff85)==0xc2f60000);
    bool convert_fault=false;try {f.cvt_s_w_exact(0x01000001);}catch(const std::runtime_error&){convert_fault=true;}CHECK(convert_fault);
    CHECK(f.cvt_w_s(0x40200000)==2 && f.cvt_w_s(0xc0200000)==0xfffffffe && f.cvt_w_s(0x4f000000)==0x7fffffff);
    f.compare(0x80000000,0,false);CHECK((f.control&0x00800000u)!=0);
    f.compare(0xc0000000,0x3f800000,true);CHECK((f.control&0x00800000u)!=0);
    f.compare(0x3f800000,0xc0000000,true);CHECK((f.control&0x00800000u)==0);
    // Clamp boundaries, both signs, equal zeros and unchanged exception flags.
    const auto flags=f.control&~0x00800000u;
    for(const auto pair: {std::array<std::uint32_t,3>{0x3f800000,0x3f800000,1},
                         {0x3f000000,0x3f800000,1},{0x40000000,0x3f800000,0},
                         {0xc0000000,0xbf800000,1},{0xbf800000,0xc0000000,0},
                         {0x80000000,0,1},{0,0x80000000,1}}) {
        f.compare(pair[0],pair[1],true,true);
        CHECK(bool(f.control&0x00800000u)==bool(pair[2]));
        CHECK((f.control&~0x00800000u)==flags);
    }
    s.store_quad(0x10f, 1); s.load_quad(0x107, 2);
    CHECK(s.gpr[2].lo == 0x12345678 && s.gpr[2].hi == 0xabcdef01);
    s.store_quad(0x101, 0); CHECK(s.load(0x100, 8, false) == 0);
    return 0;
}
