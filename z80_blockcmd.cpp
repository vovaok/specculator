#include "z80.h"

void Z80::ldi()
{
    uint8_t tmp = rd(HL++);
    wr(DE++, tmp);
    T += 2;
    BC--;
    tmp += A;
    flags._5 = !!(tmp & 2);
    flags.H = 0;
    flags._3 = !!(tmp & 8);
    flags.P = !!BC;
    flags.N = 0;
}

void Z80::ldd()
{
    uint8_t tmp = rd(HL--);
    wr(DE--, tmp);
    T += 2;
    BC--;
    tmp += A;
    flags._5 = !!(tmp & 2);
    flags.H = 0;
    flags._3 = !!(tmp & 8);
    flags.P = !!BC;
    flags.N = 0;
}

void Z80::cpi()
{
    uint8_t tmp = rd(HL);
    uint16_t r = A - tmp;
    HL++;
    BC--;
    flags.S = !!(r & 0x80);
    flags.Z = !r;
    flags.H = (A & 0xF) < (tmp & 0xF);
    flags.P = !!BC;
    flags.N = 1;
    if (flags.H)
        r--;
    flags._5 = !!(r & 2);
    flags._3 = !!(r & 8);
    T += 5;
}

void Z80::cpd()
{
    uint8_t tmp = rd(HL);
    uint16_t r = A - tmp;
    HL--;
    BC--;
    flags.S = !!(r & 0x80);
    flags.Z = !r;
    flags.H = (A & 0xF) < (tmp & 0xF);
    flags.P = !!BC;
    flags.N = 1;
    if (flags.H)
        r--;
    flags._5 = !!(r & 2);
    flags._3 = !!(r & 8);
    T += 5;
}

void Z80::ini()
{
    uint8_t tmp = in(BC);
    wr(HL++, tmp);
    B--;
    F = B & 0xA8;
    flags.Z = !B;
    flags.N = !!(tmp & 0x80);
    uint16_t r = tmp + ((C + 1) & 0xFF);
    flags.C = r > 255;
    flags.H = flags.C;
    flags.P = parity((r & 7) ^ B);
}

void Z80::ind()
{
    uint8_t tmp = in(BC);
    wr(HL--, tmp);
    B--;
    F = B & 0xA8;
    flags.Z = !B;
    flags.N = !!(tmp & 0x80);
    uint16_t r = tmp + ((C - 1) & 0xFF);
    flags.C = r > 255;
    flags.H = flags.C;
    flags.P = parity((r & 7) ^ B);
}

void Z80::outi()
{
    uint8_t tmp = rd(HL);
    B--;
    wr(BC, tmp);
    HL++;
    F = B & 0xA8;
    flags.Z = !B;
    flags.N = !!(tmp & 0x80);
    uint16_t r = tmp + L;
    flags.C = r > 255;
    flags.H = flags.C;
    flags.P = parity((r & 7) ^ B);
}

void Z80::outd()
{
    uint8_t tmp = rd(HL);
    B--;
    wr(BC, tmp);
    HL--;
    F = B & 0xA8;
    flags.Z = !B;
    flags.N = !!(tmp & 0x80);
    uint16_t r = tmp + L;
    flags.C = r > 255;
    flags.H = flags.C;
    flags.P = parity((r & 7) ^ B);
}
