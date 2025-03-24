#include "z80.h"
#include <thread>
#include <chrono>
#include <QDebug>

using namespace std::literals;

Z80::Z80(void *memory) :
    mem(reinterpret_cast<uint8_t *>(memory))
{
    reset();

//    op[0] = [](){};
//    op[1] = [this](){A = B;};
//    op[2] = [this](){C = rd(HL++);};
//    op[3] = [this](){D = rd(PC++);};
}

void Z80::reset()
{
    AF = A_F_ = 0;
    BC = B_C_ = 0;
    DE = D_E_ = 0;
    HL = H_L_ = 0;
    IX = IY = 0;
    SP = 0;
    PC = 0;
    I = 0;
    R = 0;
    IFF1 = IFF2 = 0;
    IM = 0;

    halt = 0;
    T = 0;

//    if (_cpu)
//        z80_reset(_cpu);
}

void Z80::nmi()
{
    halt = 0;
    IFF1 = 0;
    T += 4;
    call(0x0066);
}

void Z80::irq()
{
    _int = 1;
}

void Z80::acceptInt()
{
    _int = 0;
    halt = 0;
    IFF1 = IFF2 = 0;
    //    I = data;

    T += 4;

    uint16_t addr;
    uint8_t tmp;

    if (blk)
        F = (F & ~0x28) | ((PC >> 8) & 0x28);

    switch (IM)
    {
    case 0:
        T += 2;
        halt = 1;
        qDebug() << "WARNING! Interrupt mode 0 not implemented!";
        dump();
        break;

    case 1:
        T += 2;
        call(0x0038);
        break;

    case 2:
        T += 2;
        addr = (I << 8) | 0;// data;
        push(PC);
        tmp = rd(addr++);
        PC = (tmp << 8) | rd(addr);
        break;
    }

}

void Z80::step()
{
    // test with ROM code only:
//    if (PC > 0x4000 && lastPC != PC)
//    {
//        halt = 1;
//        dump();
//    }

    lastPC = PC;

    if (bkpt && bkpt == PC)
        halt = true;

    if (_int && IFF1)
        acceptInt();
    else if (halt)
        exec(fetchNop()); // dummy NOP cycle when halt
    else
        exec(fetchByte());

    if (enable_interrupt)
    {
        --enable_interrupt;
        if (!enable_interrupt)
            IFF1 = IFF2 = 1;
    }

    //    T = 0;
}



#define OP(n, t, ...) case 0x##n: T += t; __VA_ARGS__; break

void Z80::exec(uint8_t opcode)
{
    uint8_t tmp;
    uint16_t addr;

    // shadow real HL register
    uint16_t HL;
    uint8_t &L = reinterpret_cast<uint8_t *>(&HL)[0];
    uint8_t &H = reinterpret_cast<uint8_t *>(&HL)[1];

    switch (prefix)
    {
        case 0xDD: HL = IX; break;
        case 0xFD: HL = IY; break;
        default: HL = this->HL;
    }

    switch (opcode)
    {
        OP(00, 4);
        OP(01, 4,   C = rd(PC++); B = rd(PC++));
        OP(02, 4,   wr(BC, A));
        OP(03, 6,   BC++);
        OP(04, 4,   B = inc(B));
        OP(05, 4,   B = dec(B));
        OP(06, 4,   B = rd(PC++));
        OP(07, 4,   rlca());
        OP(08, 4,   std::swap(AF, A_F_));
        OP(09, 11,  HL = add16(HL, BC));
        OP(0A, 4,   A = rd(BC));
        OP(0B, 6,   BC--);
        OP(0C, 4,   C = inc(C));
        OP(0D, 4,   C = dec(C));
        OP(0E, 4,   C = rd(PC++));
        OP(0F, 4,   rrca());

        OP(10, 5,   tmp = rd(PC++); if (--B) {PC += (int8_t)tmp; T += 5;});
        OP(11, 4,   E = rd(PC++); D = rd(PC++));
        OP(12, 4,   wr(DE, A));
        OP(13, 6,   DE++);
        OP(14, 4,   D = inc(D));
        OP(15, 4,   D = dec(D));
        OP(16, 4,   D = rd(PC++));
        OP(17, 4,   rla());
        OP(18, 4,   tmp = rd(PC++); PC += (int8_t)tmp; T += 5;);
        OP(19, 11,  HL = add16(HL, DE));
        OP(1A, 4,   A = rd(DE));
        OP(1B, 6,   DE--);
        OP(1C, 4,   E = inc(E));
        OP(1D, 4,   E = dec(E));
        OP(1E, 4,   E = rd(PC++));
        OP(1F, 4,   rra());

        OP(20, 4,   tmp = rd(PC++); if (!flags.Z) {PC += (int8_t)tmp; T += 5;});
        OP(21, 4,   L = rd(PC++); H = rd(PC++));
        OP(22, 4,   addr = readWord(); wr(addr++, L); wr(addr, H););
        OP(23, 6,   HL++);
        OP(24, 4,   H = inc(H));
        OP(25, 4,   H = dec(H));
        OP(26, 4,   H = rd(PC++));
        OP(27, 4,   daa());
        OP(28, 4,   tmp = rd(PC++); if (flags.Z) {PC += (int8_t)tmp; T += 5;});
        OP(29, 11,  HL = add16(HL, HL));
        OP(2A, 4,   addr = readWord(); L = rd(addr++); H = rd(addr););
        OP(2B, 6,   HL--);
        OP(2C, 4,   L = inc(L));
        OP(2D, 4,   L = dec(L));
        OP(2E, 4,   L = rd(PC++));
        OP(2F, 4,   cpl());

        OP(30, 4,   tmp = rd(PC++); if (!flags.C) {PC += (int8_t)tmp; T += 5;});
        OP(31, 4,   SPL = rd(PC++); SPH = rd(PC++));
        OP(32, 4,   addr = readWord(); wr(addr, A););
        OP(33, 6,   SP++);
        OP(34, 4,   addr = pointer(); tmp = rd(addr); tmp = inc(tmp); T++; wr(addr, tmp););
        OP(35, 4,   addr = pointer(); tmp = rd(addr); tmp = dec(tmp); T++; wr(addr, tmp););
/*@*/   OP(36, 4,   addr = pointer(); wr(addr, rd(PC++)); /*T -= 3*/); //! @todo check cycle count!
        OP(37, 4,   F = (F & 0xC4) | (A & 0x28); flags.C = 1); // SCF
        OP(38, 4,   tmp = rd(PC++); if (flags.C) {PC += (int8_t)tmp; T += 5;});
        OP(39, 11,  HL = add16(HL, SP));
        OP(3A, 4,   addr = readWord(); A = rd(addr););
        OP(3B, 6,   SP--);
        OP(3C, 4,   A = inc(A));
        OP(3D, 4,   A = dec(A));
        OP(3E, 4,   A = rd(PC++));
        OP(3F, 4,   F = (F & 0xC5) | (A & 0x28); flags.H = flags.C; flags.C ^= 1); // CCF

        OP(40, 4,   B = B);
        OP(41, 4,   B = C);
        OP(42, 4,   B = D);
        OP(43, 4,   B = E);
        OP(44, 4,   B = H);
        OP(45, 4,   B = L);
        OP(46, 4,   addr = pointer(); B = rd(addr));
        OP(47, 4,   B = A);
        OP(48, 4,   C = B);
        OP(49, 4,   C = C);
        OP(4A, 4,   C = D);
        OP(4B, 4,   C = E);
        OP(4C, 4,   C = H);
        OP(4D, 4,   C = L);
        OP(4E, 4,   addr = pointer(); C = rd(addr));
        OP(4F, 4,   C = A);

        OP(50, 4,   D = B);
        OP(51, 4,   D = C);
        OP(52, 4,   D = D);
        OP(53, 4,   D = E);
        OP(54, 4,   D = H);
        OP(55, 4,   D = L);
        OP(56, 4,   addr = pointer(); D = rd(addr));
        OP(57, 4,   D = A);
        OP(58, 4,   E = B);
        OP(59, 4,   E = C);
        OP(5A, 4,   E = D);
        OP(5B, 4,   E = E);
        OP(5C, 4,   E = H);
        OP(5D, 4,   E = L);
        OP(5E, 4,   addr = pointer(); E = rd(addr));
        OP(5F, 4,   E = A);

        OP(60, 4,   H = B);
        OP(61, 4,   H = C);
        OP(62, 4,   H = D);
        OP(63, 4,   H = E);
        OP(64, 4,   H = H);
        OP(65, 4,   H = L);
        OP(66, 4,   addr = pointer(); H = rd(addr); L = this->L; prefix = 0x00); // clear prefix to write to real HL register
        OP(67, 4,   H = A);
        OP(68, 4,   L = B);
        OP(69, 4,   L = C);
        OP(6A, 4,   L = D);
        OP(6B, 4,   L = E);
        OP(6C, 4,   L = H);
        OP(6D, 4,   L = L);
        OP(6E, 4,   addr = pointer(); L = rd(addr); H = this->H; prefix = 0x00);
        OP(6F, 4,   L = A);

        OP(70, 4,   addr = pointer(); wr(addr, B));
        OP(71, 4,   addr = pointer(); wr(addr, C));
        OP(72, 4,   addr = pointer(); wr(addr, D));
        OP(73, 4,   addr = pointer(); wr(addr, E));
        OP(74, 4,   addr = pointer(); wr(addr, this->H));
        OP(75, 4,   addr = pointer(); wr(addr, this->L));
        OP(76, 4,   halt = 1; /*PC--*/); // why PC--?
        OP(77, 4,   addr = pointer(); wr(addr, A));
        OP(78, 4,   A = B);
        OP(79, 4,   A = C);
        OP(7A, 4,   A = D);
        OP(7B, 4,   A = E);
        OP(7C, 4,   A = H);
        OP(7D, 4,   A = L);
        OP(7E, 4,   addr = pointer(); A = rd(addr));
        OP(7F, 4,   A = A);

        OP(80, 4,   add(B));
        OP(81, 4,   add(C));
        OP(82, 4,   add(D));
        OP(83, 4,   add(E));
        OP(84, 4,   add(H));
        OP(85, 4,   add(L));
        OP(86, 4,   addr = pointer(); add(rd(addr)));
        OP(87, 4,   add(A));
        OP(88, 4,   adc(B));
        OP(89, 4,   adc(C));
        OP(8A, 4,   adc(D));
        OP(8B, 4,   adc(E));
        OP(8C, 4,   adc(H));
        OP(8D, 4,   adc(L));
        OP(8E, 4,   addr = pointer(); adc(rd(addr)));
        OP(8F, 4,   adc(A));

        OP(90, 4,   sub(B));
        OP(91, 4,   sub(C));
        OP(92, 4,   sub(D));
        OP(93, 4,   sub(E));
        OP(94, 4,   sub(H));
        OP(95, 4,   sub(L));
        OP(96, 4,   addr = pointer(); sub(rd(addr)));
        OP(97, 4,   sub(A));
        OP(98, 4,   sbc(B));
        OP(99, 4,   sbc(C));
        OP(9A, 4,   sbc(D));
        OP(9B, 4,   sbc(E));
        OP(9C, 4,   sbc(H));
        OP(9D, 4,   sbc(L));
        OP(9E, 4,   addr = pointer(); sbc(rd(addr)));
        OP(9F, 4,   sbc(A));

        OP(A0, 4,   and_(B));
        OP(A1, 4,   and_(C));
        OP(A2, 4,   and_(D));
        OP(A3, 4,   and_(E));
        OP(A4, 4,   and_(H));
        OP(A5, 4,   and_(L));
        OP(A6, 4,   addr = pointer(); and_(rd(addr)));
        OP(A7, 4,   and_(A));
        OP(A8, 4,   xor_(B));
        OP(A9, 4,   xor_(C));
        OP(AA, 4,   xor_(D));
        OP(AB, 4,   xor_(E));
        OP(AC, 4,   xor_(H));
        OP(AD, 4,   xor_(L));
        OP(AE, 4,   addr = pointer(); xor_(rd(addr)));
        OP(AF, 4,   xor_(A));

        OP(B0, 4,   or_(B));
        OP(B1, 4,   or_(C));
        OP(B2, 4,   or_(D));
        OP(B3, 4,   or_(E));
        OP(B4, 4,   or_(H));
        OP(B5, 4,   or_(L));
        OP(B6, 4,   addr = pointer(); or_(rd(addr)));
        OP(B7, 4,   or_(A));
        OP(B8, 4,   cp(B));
        OP(B9, 4,   cp(C));
        OP(BA, 4,   cp(D));
        OP(BB, 4,   cp(E));
        OP(BC, 4,   cp(H));
        OP(BD, 4,   cp(L));
        OP(BE, 4,   addr = pointer(); cp(rd(addr)));
        OP(BF, 4,   cp(A));

        OP(C0, 5,   if (!flags.Z) ret());
        OP(C1, 4,   BC = pop());
        OP(C2, 4,   addr = readWord(); if (!flags.Z) PC = addr;);
        OP(C3, 4,   addr = readWord(); PC = addr;);
        OP(C4, 4,   addr = readWord(); if (!flags.Z) {T++; call(addr);});
        OP(C5, 5,   push(BC));
        OP(C6, 4,   add(rd(PC++)));
        OP(C7, 5,   call(0x00)); // RST 0
        OP(C8, 5,   if (flags.Z) ret());
        OP(C9, 5,   ret());
        OP(CA, 4,   addr = readWord(); if (flags.Z) PC = addr;);
        OP(CB, 4,   execCB());
        OP(CC, 4,   addr = readWord(); if (flags.Z) {T++; call(addr);});
        OP(CD, 4,   addr = readWord(); call(addr););
        OP(CE, 4,   adc(rd(PC++)));
        OP(CF, 5,   call(0x08)); // RST 8

        OP(D0, 5,   if (!flags.C) ret());
        OP(D1, 4,   DE = pop());
        OP(D2, 4,   addr = readWord(); if (!flags.C) PC = addr;);
        OP(D3, 4,   addr = rd(PC++) | (A<<8); out(addr, A););
        OP(D4, 4,   addr = readWord(); if (!flags.C) {T++; call(addr);});
        OP(D5, 5,   push(DE));
        OP(D6, 4,   sub(rd(PC++)));
        OP(D7, 5,   call(0x10)); // RST 10H
        OP(D8, 5,   if (flags.C) ret());
        OP(D9, 4,   std::swap(BC, B_C_); std::swap(DE, D_E_); std::swap(HL, H_L_); prefix = 0x00); // EXX
        OP(DA, 4,   addr = readWord(); if (flags.C) PC = addr;);
        OP(DB, 4,   addr = rd(PC++) | (A<<8); A = in(addr););
        OP(DC, 4,   addr = readWord(); if (flags.C) {T++; call(addr);});
        OP(DD, 4,   prefix = 0xDD; exec(fetchByte()); HL = this->HL);
        OP(DE, 4,   sbc(rd(PC++)));
        OP(DF, 5,   call(0x18)); // RST 18H

        OP(E0, 5,   if (!flags.P) ret());
        OP(E1, 4,   HL = pop());
        OP(E2, 4,   addr = readWord(); if (!flags.P) PC = addr;);
        OP(E3, 4,   addr = pop(); T++; push(HL); T += 2; HL = addr;); // EX (SP),HL
        OP(E4, 4,   addr = readWord(); if (!flags.P) {T++; call(addr);});
        OP(E5, 5,   push(HL));
        OP(E6, 4,   and_(rd(PC++)));
        OP(E7, 5,   call(0x20)); // RST 20H
        OP(E8, 5,   if (flags.P) ret());
        OP(E9, 4,   PC = HL); // JP (HL)
        OP(EA, 4,   addr = readWord(); if (flags.P) PC = addr;);
        OP(EB, 4,   std::swap(HL, DE)); // EX DE,HL
        OP(EC, 4,   addr = readWord(); if (flags.P) {T++; call(addr);});
        OP(ED, 4,   execED()); // prefix used for disable change of HL
        OP(EE, 4,   xor_(rd(PC++)));
        OP(EF, 5,   call(0x28)); // RST 28H

        OP(F0, 5,   if (!flags.S) ret());
        OP(F1, 4,   AF = pop());
        OP(F2, 4,   addr = readWord(); if (!flags.S) PC = addr;);
        OP(F3, 4,   IFF1 = IFF2 = 0); // DI
        OP(F4, 4,   addr = readWord(); if (!flags.S) {T++; call(addr);});
        OP(F5, 5,   push(AF));
        OP(F6, 4,   or_(rd(PC++)));
        OP(F7, 5,   call(0x30)); // RST 30H
        OP(F8, 5,   if (flags.S) ret());
        OP(F9, 6,   SP = HL);
        OP(FA, 4,   addr = readWord(); if (flags.S) PC = addr;);
        OP(FB, 4,   enable_interrupt = 2); // set IFF1 = IFF2 = 1 after the next step
        OP(FC, 4,   addr = readWord(); if (flags.S) {T++; call(addr);});
        OP(FD, 4,   prefix = 0xFD; exec(fetchByte()); HL = this->HL);
        OP(FE, 4,   cp(rd(PC++)));
        OP(FF, 5,   call(0x38)); // RST 38H
    }

    switch (prefix)
    {
        case 0x00: this->HL = HL; break;
        case 0xDD: IX = HL; break;
        case 0xFD: IY = HL; break;
    }
    prefix = 0x00;
}

void Z80::execCB()
{
    bool mem_operand = false;
    uint8_t tmp;
    uint8_t hptr = PC >> 8;
    uint8_t *r = &tmp;

    T += 4;

    uint16_t addr = pointer();
    uint8_t opcode = fetchByte();

    switch (prefix)
    {
    case 0x00:
        prefix = 0xCB;
        break;
    case 0xDD:
    case 0xFD:
        mem_operand = true;
        hptr = addr >> 8;
    }

    switch (opcode & 7)
    {
    case 0: r = &B; break;
    case 1: r = &C; break;
    case 2: r = &D; break;
    case 3: r = &E; break;
    case 4: r = &H; break;
    case 5: r = &L; break;
    case 6: mem_operand = true; break;
    case 7: r = &A; break;
    }

    if (mem_operand)
    {
        tmp = rd(addr);
        T++;
    }
    else
        tmp = *r;

    int bit = (opcode >> 3) & 7;

    switch (opcode & 0xC0)
    {
    case 0x00:
        switch (bit)
        {
        case 0: tmp = rlc(tmp); break;
        case 1: tmp = rrc(tmp); break;
        case 2: tmp = rl(tmp); break;
        case 3: tmp = rr(tmp); break;
        case 4: tmp = sla(tmp); break;
        case 5: tmp = sra(tmp); break;
        case 6: tmp = sll(tmp); break;
        case 7: tmp = srl(tmp); break;
        }
        break;

    case 0x40:
        tmp = tmp & (1 << bit);
        if (mem_operand)
            F = (F & 1) | (hptr & 0x28) | (tmp & 0x80);
        else
            F = (F & 1) | (tmp & 0xA8);
        flags.Z = flags.P = !tmp;
        flags.H = 1;
        return;

    case 0x80: tmp &= ~(1 << bit); break;
    case 0xC0: tmp |= (1 << bit); break;
    }

    if (mem_operand)
        wr(addr, tmp);
    *r = tmp;
}

void Z80::execED()
{
//    uint8_t tmp;
    uint16_t addr;

    prefix = 0xED;
    uint8_t opcode = fetchByte();

    switch (opcode)
    {
        OP(40, 4, B = in());
        OP(41, 4, out(BC, B));
        OP(42, 11, HL = sbc16(HL, BC));
        OP(43, 4, addr = readWord(); wr(addr++, C), wr(addr, B));
        OP(44, 4, neg());
        OP(45, 4, IFF1 = IFF2; ret()); // RETN
        OP(46, 4, IM = 0);
        OP(47, 5, I = A);
        OP(48, 4, C = in());
        OP(49, 4, out(BC, C));
        OP(4A, 11, HL = adc16(HL, BC));
        OP(4B, 4, addr = readWord(); C = rd(addr++); B = rd(addr));
        OP(4C, 4, neg());
        OP(4D, 4, ret()); // RETI
        OP(4E, 4, IM = 0);
        OP(4F, 5, R = A);

        OP(50, 4, D = in());
        OP(51, 4, out(BC, D));
        OP(52, 11, HL = sbc16(HL, DE));
        OP(53, 4, addr = readWord(); wr(addr++, E), wr(addr, D));
        OP(54, 4, neg());
        OP(55, 4, IFF1 = IFF2; ret()); // RETN
        OP(56, 4, IM = 1);
/**/    OP(57, 5, A = I; F = (F & 0x01) | (A & 0xA8); flags.Z = !A; flags.P = IFF2; /*resPV = 1*/);
        OP(58, 4, E = in());
        OP(59, 4, out(BC, E));
        OP(5A, 11, HL = adc16(HL, DE));
        OP(5B, 4, addr = readWord(); E = rd(addr++); D = rd(addr));
        OP(5C, 4, neg());
        OP(5D, 4, ret()); // RETI
        OP(5E, 4, IM = 2);
/**/    OP(5F, 5, A = R; F = (F & 0x01) | (A & 0xA8); flags.Z = !A; flags.P = IFF2; /*resPV = 1*/); // if INT coming after this opcode, reset PV flag

        OP(60, 4, H = in());
        OP(61, 4, out(BC, H));
        OP(62, 11, HL = sbc16(HL, HL));
        OP(63, 4, addr = readWord(); wr(addr++, L), wr(addr, H));
        OP(64, 4, neg());
        OP(65, 4, IFF1 = IFF2; ret()); // RETN
        OP(66, 4, IM = 0);
        OP(67, 5, rrd());
        OP(68, 4, L = in());
        OP(69, 4, out(BC, L));
        OP(6A, 11, HL = adc16(HL, HL));
        OP(6B, 4, addr = readWord(); L = rd(addr++); H = rd(addr));
        OP(6C, 4, neg());
        OP(6D, 4, ret()); // RETI
        OP(6E, 4, IM = 0);
        OP(6F, 5, rld());

        OP(70, 4, in());
        OP(71, 4, out(BC, 0));
        OP(72, 11, HL = sbc16(HL, SP));
        OP(73, 4, addr = readWord(); wr(addr++, SPL), wr(addr, SPH));
        OP(74, 4, neg());
        OP(75, 4, IFF1 = IFF2; ret()); // RETN
        OP(76, 4, IM = 1);
        OP(77, 4, ); // nop*
        OP(78, 4, A = in());
        OP(79, 4, out(BC, A));
        OP(7A, 11, HL = adc16(HL, SP));
        OP(7B, 4, addr = readWord(); SPL = rd(addr++); SPH = rd(addr));
        OP(7C, 4, neg());
        OP(7D, 4, ret()); // RETI
        OP(7E, 4, IM = 2);
        OP(7F, 4, ); // nop*

        OP(A0, 4, ldi());
        OP(A1, 4, cpi());
        OP(A2, 4, ini());
        OP(A3, 4, outi());

        OP(A8, 4, ldd());
        OP(A9, 4, cpd());
        OP(AA, 4, ind());
        OP(AB, 4, outd());

        OP(B0, 4, ldi(); if ((blk = !!BC)) {PC -= 2; T += 5;});
        OP(B1, 4, cpi(); if ((blk = (flags.P && !flags.Z))) {PC -= 2; T += 5;});
        OP(B2, 4, ini(); if (B) {PC -= 2; T += 5;});
        OP(B3, 4, outi(); if (B) {PC -= 2; T += 5;});

        OP(B8, 4, ldd(); if ((blk = !!BC)) {PC -= 2; T += 5;});
        OP(B9, 4, cpd(); if ((blk = (flags.P && !flags.Z))) {PC -= 2; T += 5;});
        OP(BA, 4, ind(); if (B) {PC -= 2; T += 5;});
        OP(BB, 4, outd(); if (B) {PC -= 2; T += 5;});

        default: T += 4; // nop*
    }
}

#undef OP


void Z80::wr(uint16_t addr, uint8_t value)
{
    if (addr >= 0x4000)
    {
        mem[addr] = value;
        T += 3;
    }
    else
    {
//        halt = 1;
//        qDebug() << "WARNING! memory write attempt @" << addr;
//        dump();
    }
}

uint8_t Z80::rd(uint16_t addr)
{
    T += 3;
    return mem[addr];
}

uint8_t Z80::fetchByte()
{
    R = (R + 1) & 0x7F;
    return mem[PC++];
}

uint8_t Z80::fetchNop()
{
    R = (R + 1) & 0x7F;
    return 0;
}

uint16_t Z80::readWord()
{
    uint16_t w = rd(PC++);
    w |= rd(PC++) << 8UL;
    return w;
}

uint16_t Z80::pointer()
{
    switch (prefix)
    {
    case 0xDD: return IX + (int8_t)rd(PC++); T += 5; break;
    case 0xFD: return IY + (int8_t)rd(PC++); T += 5; break;
    default: return HL;
    }
}

void Z80::out(uint16_t addr, uint8_t data)
{
    T += 4;

    ioreq(addr, data, true);
}

uint8_t Z80::in(uint16_t addr)
{
    T += 4;

    uint8_t data;
    ioreq(addr, data, false);
    return data;

}

uint8_t Z80::in()
{
    uint8_t v = in(BC);
    F = (F & 0x01) | (v & 0xA8);
    flags.Z = !v;
    flags.P = parity(v);
    return v;
}

void Z80::push(uint16_t v)
{
    wr(--SP, v >> 8);
    wr(--SP, v);
}

uint16_t Z80::pop()
{
    uint16_t v = rd(SP++);
    v |= rd(SP++) << 8;
    return v;
}

void Z80::call(uint16_t addr)
{
    push(PC);
    PC = addr;
}

void Z80::ret()
{
    PC = pop();
}



QString hex(uint8_t v)
{
    return QString().asprintf("%02X", v);
}

QString hex(uint16_t v)
{
    return QString().asprintf("%04X", v);
}

QString Z80::flagString()
{
    QString str = "SZ5H3PNC";
    for (int i=0; i<8; i++)
        if (!(F & (1<<i)))
            str[7-i] = '-';
    return str;
}

void Z80::dump()
{
    qDebug() << "PC:" << hex(PC) << "SP:" << hex(SP) << "T:" << T;
    qDebug() << "AF :" << hex(AF) << "BC :" << hex(BC) << "DE :" << hex(DE) << "HL :" << hex(HL);
    qDebug() << "AF':" << hex(A_F_) << "BC':" << hex(B_C_) << "DE':" << hex(D_E_) << "HL':" << hex(H_L_);
    qDebug() << "IX" << hex(IX) << "IY:" << hex(IY);
    qDebug() << "IM:" << IM << " IFF1:" << (int)IFF1 << " IFF2:" << (int)IFF2;
    qDebug() << "I:" << hex(I) << "R:" << hex(R);
    qDebug() << "FLAGS:" << flagString();
    qDebug() << "MEM:" << hex(lastPC) << ":" << QByteArray((const char *)mem + lastPC, 16).toHex(' ');
    qDebug() << "MEM:" << hex(HL) << ":" << QByteArray((const char *)mem + HL, 16).toHex(' ');
}

void Z80::saveState(QDataStream &out)
{
    out << AF << BC << DE << HL;
    out << A_F_ << B_C_ << D_E_ << H_L_;
    out << IX << IY << SP << PC;
    out << I << R;
    out << IFF1 << IFF2 << IM;
}

void Z80::restoreState(QDataStream &in)
{
    in >> AF >> BC >> DE >> HL;
    in >> A_F_ >> B_C_ >> D_E_ >> H_L_;
    in >> IX >> IY >> SP >> PC;
    in >> I >> R;
    in >> IFF1 >> IFF2 >> IM;
}


void Z80::test()
{

}

