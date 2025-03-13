#include "z80.h"

// Common operations

uint8_t Z80::inc(uint8_t v)
{
    v++;
    F = (F & 0x01) | (v & 0xA8);
    flags.Z = !v;
    flags.P = !!(v == 0x80);
    flags.H = !(v & 0x0f);
    return v;
}

uint8_t Z80::dec(uint8_t v)
{
    flags.H = !(v & 0x0f);
    v--;
    F = (F & 0x11) | (v & 0xA8);
    flags.Z = !v;
    flags.N = 1;
    flags.P = !!(v == 0x7f);
    return v;
}

void Z80::add(uint8_t v)
{
    int r = A + v;
    F = (r & 0xA8);
    flags.Z = !(r & 0xFF);
    flags.H = ((A & 0xF) + (v & 0xF)) > 0xF;
    flags.P = (unsigned int)((int8_t)A + (int8_t)v + 0x80) > 0xFF;
    //    flags.P = !!(((A & v & ~r) | (~A & ~v & r)) & 0x80); // this is slower :c
    flags.C = !!(r >> 8);
    A = r;
}

void Z80::adc(uint8_t v)
{
    uint8_t c = flags.C;
    int r = A + v + c;
    F = (r & 0xA8);
    flags.Z = !(r & 0xFF);
    flags.H = ((A & 0xF) + (v & 0xF) + c) > 0xF;
    flags.P = (unsigned int)((int8_t)A + (int8_t)v + c + 0x80) > 0xFF;
    flags.C = !!(r >> 8);
    A = r;
}

void Z80::sub(uint8_t v)
{
    int r = A - v;
    F = (r & 0xA8);
    flags.Z = !(r & 0xFF);
    flags.H = (A & 0xF) < (v & 0xF);
    flags.P = (unsigned int)((int8_t)A - (int8_t)v + 0x80) > 0xFF;
    flags.N = 1;
    flags.C = !!(r >> 8);
    A = r;
}

void Z80::sbc(uint8_t v)
{
    uint8_t c = flags.C;
    int r = A - v - c;
    F = (r & 0xA8);
    flags.Z = !(r & 0xFF);
    flags.H = (A & 0xF) < ((v & 0xF) + c);
    flags.P = (unsigned int)((int8_t)A - (int8_t)v - c + 0x80) > 0xFF;
    flags.N = 1;
    flags.C = !!(r >> 8);
    A = r;
}

void Z80::and_(uint8_t v)
{
    A &= v;
    F = A & 0xA8;
    flags.Z = !A;
    flags.H = 1;
    flags.P = parity(A);
}

void Z80::or_(uint8_t v)
{
    A |= v;
    F = A & 0xA8;
    flags.Z = !A;
    flags.H = 1;
    flags.P = parity(A);
}

void Z80::xor_(uint8_t v)
{
    A ^= v;
    F = A & 0xA8;
    flags.Z = !A;
    flags.P = parity(A);
}

void Z80::cp(uint8_t v)
{
    int r = A - v;
    F = (v & 0x28);
    flags.S = !!(r & 0x80);
    flags.Z = !(r & 0xFF);
    flags.H = (A & 0xF) < ((v & 0xF));
    flags.P = (unsigned int)((int8_t)A - (int8_t)v + 0x80) > 0xFF;
    flags.N = 1;
    flags.C = !!(r >> 8);
}

// 16-bit arithmetic operations

uint16_t Z80::add16(uint16_t v1, uint16_t v2)
{
    uint32_t r = v1 + v2;
    F = (F & 0xC4) | ((r >> 8) & 0x28);
    flags.H = ((v1 & 0x0F00) + (v2 & 0x0F00)) > 0x0FFF;
    flags.C = r > 0xFFFF;
    return r;
}

uint16_t Z80::adc16(uint16_t v1, uint16_t v2)
{
    uint8_t c = flags.C;
    uint32_t r = v1 + v2 + c;
    F = (r >> 8) & 0xA8;
    flags.Z = !(r & 0xFFFF);
    flags.H = ((v1 & 0x0F00) + (v2 & 0x0F00) + c) > 0x0FFF;
    flags.P = (uint32_t)((int16_t)v1 + (int16_t)v2 + c + 0x8000) > 0xFFFF;
    flags.C = r >> 16;
    return r;
}

//uint16_t Z80::sub16(uint16_t v1, uint16_t v2)
//{
//    uint32_t r = v1 - v2;
//    F = (r >> 8) & 0xA8;
//    flags.Z = !(r & 0xFFFF);
//    flags.H = (v1 & 0x0F00) < ((v2 & 0x0F00)/* + c*/);
//    flags.P = (uint32_t)((int16_t)v1 - (int16_t)v2 + 0x8000) > 0xFFFF;
//    flags.N = 1;
//    flags.C = r >> 16;
//    return r;
//}

uint16_t Z80::sbc16(uint16_t v1, uint16_t v2)
{
    uint8_t c = flags.C;
    uint32_t r = v1 - v2 - c;
    F = (r >> 8) & 0xA8;
    flags.Z = !(r & 0xFFFF);
    flags.H = (v1 & 0x0F00) < ((v2 & 0x0F00)/* + c*/);
    flags.P = (uint32_t)((int16_t)v1 - (int16_t)v2 - c + 0x8000) > 0xFFFF;
    flags.N = 1;
    flags.C = r >> 16;
    return r;
}

void Z80::rrd()
{
    uint8_t tmp = rd(HL);
    T += 4;
    wr(HL, (A << 4) | (tmp >> 4));
    A = (A & 0xF0) | (tmp & 0x0F);
    F = (F & 0x01) | (A & 0xA8);
    flags.Z = !A;
    flags.P = parity(A);
}

void Z80::rld()
{
    uint8_t tmp = rd(HL);
    T += 4;
    wr(HL, (tmp << 4) | (A & 0x0F));
    A = (A & 0xF0) | (tmp >> 4);
    F = (F & 0x01) | (A & 0xA8);
    flags.Z = !A;
    flags.P = parity(A);
}

// Accumulator-specific operations

void Z80::rlca()
{
    A = (A<<1) | (A>>7);
    F = (F & 0xC4) | (A & 0x28);
    flags.C = A & 1;
}

void Z80::rrca()
{
    flags.C = A & 1;
    A = (A>>1) | (A<<7);
    F = (F & 0xC5) | (A & 0x28);
}

void Z80::rla()
{
    uint8_t c = A >> 7;
    A = (A << 1) | flags.C;
    F = (F & 0xC4) | (A & 0x28) | c;
}

void Z80::rra()
{
    uint8_t c = A & 1;
    A = (A >> 1) | (flags.C << 7);
    F = (F & 0xC4) | (A & 0x28) | c;
}

void Z80::daa()
{
    if (!flags.N)
    {
        if ((A & 0x0F) > 9 || flags.H)
            A += 6;
        if ((A & 0xF0) > 0x90)
            A += 0x60;
    }
    else
    {
        if (flags.H)
            A -= 6;
        if (flags.C)
            A -= 0x60;
    }
    //! @todo test this shit and change the flags!
}

void Z80::cpl()
{
    A = ~A;
    flags.N = 1;
    flags.H = 1;
}

void Z80::neg()
{
    A = -A;
    F = (A & 0xA8);
    flags.Z = !A;
    flags.H = !!(A & 0xF);
    flags.P = (A == 0x80);
    flags.N = 1;
    flags.C = !!A;

//    uint8_t tmp = A;
//    A = 0;
//    sub(tmp);
}

// Rotations (and shift)

uint8_t Z80::rlc(uint8_t v)
{
    v = (v<<1) | (v>>7);
    F = v & 0xA9;
    flags.Z = !v;
    flags.P = parity(v);
    return v;
}

uint8_t Z80::rrc(uint8_t v)
{
    v = (v>>1) | (v<<7);
    F = v & 0xA8;
    flags.Z = !v;
    flags.P = parity(v);
    flags.C = v >> 7;
    return v;
}

uint8_t Z80::rl(uint8_t v)
{
    bool c = v >> 7;
    v = (v << 1) | flags.C;
    F = v & 0xA8;
    flags.Z = !v;
    flags.P = parity(v);
    flags.C = c;
    return v;
}

uint8_t Z80::rr(uint8_t v)
{
    bool c = v & 1;
    v = (v >> 1) | (flags.C << 7);
    F = v & 0xA8;
    flags.Z = !v;
    flags.P = parity(v);
    flags.C = c;
    return v;
}

uint8_t Z80::sla(uint8_t v)
{
    bool c = v >> 7;
    v <<= 1;
    F = v & 0xA8;
    flags.Z = !v;
    flags.P = parity(v);
    flags.C = c;
    return v;
}

uint8_t Z80::sra(uint8_t v)
{
    bool c = v & 1;
    v = (int8_t)v >> 1;
    F = v & 0xA8;
    flags.Z = !v;
    flags.P = parity(v);
    flags.C = c;
    return v;
}

uint8_t Z80::sll(uint8_t v)
{
    bool c = v >> 7;
    v = (v << 1) | 0x01;
    F = v & 0xA8;
    flags.Z = !v;
    flags.P = parity(v);
    flags.C = c;
    return v;
}

uint8_t Z80::srl(uint8_t v)
{
    bool c = v & 1;
    v >>= 1;
    F = v & 0xA8;
    flags.Z = !v;
    flags.P = parity(v);
    flags.C = c;
    return v;
}
