#include "z80.h"

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
    F = (F & 0x09) | (v & 0xA8);
    flags.Z = !v;
    flags.N = 1;
    flags.P = !!(v == 0x7f);
    return v;
}

uint8_t Z80::add(uint8_t v)
{
    int r = A + v;
    F = (r & 0xA8);
    flags.Z = !(r & 0xFF);
    flags.H = ((A & 0xF) + (v & 0xF)) > 0xF;
    flags.P = (unsigned int)((int8_t)A + (int8_t)v + 0x80) > 0xFF;
    //    flags.P = !!(((A & v & ~r) | (~A & ~v & r)) & 0x80); // this is slower :c
    flags.C = !!(r >> 8);
    return r;
}

uint8_t Z80::adc(uint8_t v)
{
    uint8_t c = flags.C;
    int r = A + v + c;
    F = (r & 0xA8);
    flags.Z = !(r & 0xFF);
    flags.H = ((A & 0xF) + (v & 0xF) + c) > 0xF;
    flags.P = (unsigned int)((int8_t)A + (int8_t)v + c + 0x80) > 0xFF;
    flags.C = !!(r >> 8);
    return r;
}

uint8_t Z80::sub(uint8_t v, uint8_t c)
{
    int r = A - v - c;
    F = (r & 0xA8);
    flags.Z = !(r & 0xFF);
    flags.H = (A & 0xF) < ((v & 0xF) + c);
    flags.P = (unsigned int)((int8_t)A - (int8_t)v - c + 0x80) > 0xFF;
    flags.N = 1;
    flags.C = !!(r >> 8);
    return r;
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

uint16_t Z80::add16(uint16_t v1, uint16_t v2)
{
    uint32_t r = v1 + v2;
    F = (F & 0xC4) | ((r >> 8) & 0x28);
    flags.H = ((v1 & 0x0F00) + (v2 & 0x0F00)) > 0x0FFF;
    flags.C = r > 0xFFFF;
    return r;
}

uint16_t Z80::adc16(uint16_t v1, uint16_t v2, uint8_t c)
{
    uint32_t r = v1 + v2 + c;
    F = (r >> 8) & 0xA8;
    flags.Z = !(r & 0xFFFF);
    flags.H = ((v1 & 0x0F00) + (v2 & 0x0F00) + c) > 0x0FFF;
    flags.P = (uint32_t)((int16_t)v1 + (int16_t)v2 + c + 0x8000) > 0xFFFF;
    flags.C = r >> 16;
    return r;
}

uint16_t Z80::sub16(uint16_t v1, uint16_t v2, uint8_t c)
{
    uint32_t r = v1 - v2 - c;
    F = (r >> 8) & 0xA8;
    flags.Z = !(r & 0xFFFF);
    flags.H = (v1 & 0x0F00) < ((v2 & 0x0F00)/* + c*/);
    flags.P = (uint32_t)((int16_t)v1 - (int16_t)v2 - c + 0x8000) > 0xFFFF;
    flags.N = 1;
    flags.C = r >> 16;
    return r;
}

inline static bool parity(uint8_t v)
{
    return ((0x9669 >> ((v ^ (v >> 4)) & 0xF)) & 1);
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
    //! @todo change flags!
}

void Z80::cpl()
{
    A = ~A;
    flags.N = 1;
    flags.H = 1;
}

uint8_t Z80::rlc(uint8_t v)
{
    //    v = (v<<1) | (v>>7);
    //    flags._5 = !!(A & 0x20);
    //    flags.H = 0;
    //    flags._3 = !!(A & 0x08);
    //    flags.N = 0;
    //    flags.C = A & 1;
}

uint8_t Z80::rrc(uint8_t v)
{

}

uint8_t Z80::rl(uint8_t v)
{

}

uint8_t Z80::rr(uint8_t v)
{

}

uint8_t Z80::sla(uint8_t v)
{

}

uint8_t Z80::sra(uint8_t v)
{

}

uint8_t Z80::sll(uint8_t v)
{

}

uint8_t Z80::srl(uint8_t v)
{

}
