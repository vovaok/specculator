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
    IFF1 = IFF2 = 0;

    halt = 0;
    T = 0;
}

void Z80::irq(int N)
{
    halt = 0;
    I = N;
    IFF1 = IFF2 = 0;
    //! @todo todo


}

void Z80::step()
{
    R++;
    uint8_t opcode = 0;
    if (!halt)
        opcode = fetchByte();

    exec(opcode);

    sleepCycles(T);
    T = 0;
}

#define OP(n, t, ...) case 0x##n: T += t; __VA_ARGS__; break

void Z80::exec(uint8_t opcode)
{
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

        OP(10, 5,   {int8_t off = rd(PC++); if (--B) {PC += off; T += 5;}});
        OP(11, 4,   E = rd(PC++); D = rd(PC++));
        OP(12, 4,   wr(DE, A));
        OP(13, 6,   DE++);
        OP(14, 4,   D = inc(D));
        OP(15, 4,   D = dec(D));
        OP(16, 4,   D = rd(PC++));
        OP(17, 4,   rla());
        OP(18, 4,   {int8_t off = rd(PC++); PC += off; T += 5;});
        OP(19, 11,  HL = add16(HL, DE));
        OP(1A, 4,   A = rd(DE));
        OP(1B, 6,   DE--);
        OP(1C, 4,   E = inc(E));
        OP(1D, 4,   E = dec(E));
        OP(1E, 4,   E = rd(PC++));
        OP(1F, 4,   rra());

        OP(20, 4,   {int8_t off = rd(PC++); if (!flags.Z) {PC += off; T += 5;}});
        OP(21, 4,   L = rd(PC++); H = rd(PC++));
        OP(22, 4,   {uint16_t addr = readWord(); wr(addr++, L); wr(addr, H);});
        OP(23, 6,   HL++);
        OP(24, 4,   H = inc(H));
        OP(25, 4,   H = dec(H));
        OP(26, 4,   H = rd(PC++));
        OP(27, 4,   daa());
        OP(28, 4,   {int8_t off = rd(PC++); if (flags.Z) {PC += off; T += 5;}});
        OP(29, 11,  HL = add16(HL, HL));
        OP(2A, 4,   {uint16_t addr = readWord(); L = rd(addr++); H = rd(addr);});
        OP(2B, 6,   HL--);
        OP(2C, 4,   L = inc(L));
        OP(2D, 4,   L = dec(L));
        OP(2E, 4,   L = rd(PC++));
        OP(2F, 4,   cpl());

        OP(30, 4,   {int8_t off = rd(PC++); if (!flags.C) {PC += off; T += 5;}});
        OP(31, 4,   SPL = rd(PC++); SPH = rd(PC++));
        OP(32, 4,   {uint16_t addr = readWord(); wr(addr, A);});
        OP(33, 6,   SP++);
        OP(34, 4,   {uint8_t tmp = rd(HL); tmp = inc(tmp); T++; wr(HL, tmp);});
        OP(35, 4,   {uint8_t tmp = rd(HL); tmp = dec(tmp); T++; wr(HL, tmp);});
        OP(36, 4,   wr(HL, rd(PC++)));
        OP(37, 4,   F = (F & 0xC4) | (A & 0x28); flags.C = 1); // SCF
        OP(38, 4,   {int8_t off = rd(PC++); if (flags.C) {PC += off; T += 5;}});
        OP(39, 11,  HL = add16(HL, SP));
        OP(3A, 4,   {uint16_t addr = readWord(); A = rd(addr);});
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
        OP(46, 4,   B = rd(HL));
        OP(47, 4,   B = A);
        OP(48, 4,   C = B);
        OP(49, 4,   C = C);
        OP(4A, 4,   C = D);
        OP(4B, 4,   C = E);
        OP(4C, 4,   C = H);
        OP(4D, 4,   C = L);
        OP(4E, 4,   C = rd(HL));
        OP(4F, 4,   C = A);

        OP(50, 4,   D = B);
        OP(51, 4,   D = C);
        OP(52, 4,   D = D);
        OP(53, 4,   D = E);
        OP(54, 4,   D = H);
        OP(55, 4,   D = L);
        OP(56, 4,   D = rd(HL));
        OP(57, 4,   D = A);
        OP(58, 4,   E = B);
        OP(59, 4,   E = C);
        OP(5A, 4,   E = D);
        OP(5B, 4,   E = E);
        OP(5C, 4,   E = H);
        OP(5D, 4,   E = L);
        OP(5E, 4,   E = rd(HL));
        OP(5F, 4,   E = A);

        OP(60, 4,   H = B);
        OP(61, 4,   H = C);
        OP(62, 4,   H = D);
        OP(63, 4,   H = E);
        OP(64, 4,   H = H);
        OP(65, 4,   H = L);
        OP(66, 4,   H = rd(HL));
        OP(67, 4,   H = A);
        OP(68, 4,   L = B);
        OP(69, 4,   L = C);
        OP(6A, 4,   L = D);
        OP(6B, 4,   L = E);
        OP(6C, 4,   L = H);
        OP(6D, 4,   L = L);
        OP(6E, 4,   L = rd(HL));
        OP(6F, 4,   L = A);

        OP(70, 4,   wr(HL, B));
        OP(71, 4,   wr(HL, C));
        OP(72, 4,   wr(HL, D));
        OP(73, 4,   wr(HL, E));
        OP(74, 4,   wr(HL, H));
        OP(75, 4,   wr(HL, L));
        OP(76, 4,   halt = 1; PC--); // why PC--?
        OP(77, 4,   wr(HL, A));
        OP(78, 4,   A = B);
        OP(79, 4,   A = C);
        OP(7A, 4,   A = D);
        OP(7B, 4,   A = E);
        OP(7C, 4,   A = H);
        OP(7D, 4,   A = L);
        OP(7E, 4,   A = rd(HL));
        OP(7F, 4,   A = A);

        OP(80, 4,   A = add(B));
        OP(81, 4,   A = add(C));
        OP(82, 4,   A = add(D));
        OP(83, 4,   A = add(E));
        OP(84, 4,   A = add(H));
        OP(85, 4,   A = add(L));
        OP(86, 4,   A = add(rd(HL)));
        OP(87, 4,   A = add(A));
        OP(88, 4,   A = adc(B));
        OP(89, 4,   A = adc(C));
        OP(8A, 4,   A = adc(D));
        OP(8B, 4,   A = adc(E));
        OP(8C, 4,   A = adc(H));
        OP(8D, 4,   A = adc(L));
        OP(8E, 4,   A = adc(rd(HL)));
        OP(8F, 4,   A = adc(A));

        OP(90, 4,   A = sub(B));
        OP(91, 4,   A = sub(C));
        OP(92, 4,   A = sub(D));
        OP(93, 4,   A = sub(E));
        OP(94, 4,   A = sub(H));
        OP(95, 4,   A = sub(L));
        OP(96, 4,   A = sub(rd(HL)));
        OP(97, 4,   A = sub(A));
        OP(98, 4,   A = sub(B, flags.C));
        OP(99, 4,   A = sub(C, flags.C));
        OP(9A, 4,   A = sub(D, flags.C));
        OP(9B, 4,   A = sub(E, flags.C));
        OP(9C, 4,   A = sub(H, flags.C));
        OP(9D, 4,   A = sub(L, flags.C));
        OP(9E, 4,   A = sub(rd(HL), flags.C));
        OP(9F, 4,   A = sub(A, flags.C));

        OP(A0, 4,   and_(B));
        OP(A1, 4,   and_(C));
        OP(A2, 4,   and_(D));
        OP(A3, 4,   and_(E));
        OP(A4, 4,   and_(H));
        OP(A5, 4,   and_(L));
        OP(A6, 4,   and_(rd(HL)));
        OP(A7, 4,   and_(A));
        OP(A8, 4,   xor_(B));
        OP(A9, 4,   xor_(C));
        OP(AA, 4,   xor_(D));
        OP(AB, 4,   xor_(E));
        OP(AC, 4,   xor_(H));
        OP(AD, 4,   xor_(L));
        OP(AE, 4,   xor_(rd(HL)));
        OP(AF, 4,   xor_(A));

        OP(B0, 4,   or_(B));
        OP(B1, 4,   or_(C));
        OP(B2, 4,   or_(D));
        OP(B3, 4,   or_(E));
        OP(B4, 4,   or_(H));
        OP(B5, 4,   or_(L));
        OP(B6, 4,   or_(rd(HL)));
        OP(B7, 4,   or_(A));
        OP(B8, 4,   cp(B));
        OP(B9, 4,   cp(C));
        OP(BA, 4,   cp(D));
        OP(BB, 4,   cp(E));
        OP(BC, 4,   cp(H));
        OP(BD, 4,   cp(L));
        OP(BE, 4,   cp(rd(HL)));
        OP(BF, 4,   cp(A));

        OP(C0, 5,   if (flags.Z) ret());
        OP(C1, 4,   BC = pop());
        OP(C2, 4,   {uint16_t addr = readWord(); if (!flags.Z) PC = addr;});
        OP(C3, 4,   {uint16_t addr = readWord(); PC = addr;});
        OP(C4, 4,   {uint16_t addr = readWord(); if (!flags.Z) {T++; call(addr);}});
        OP(C5, 5,   push(BC));
        OP(C6, 4,   A = add(rd(PC++)));
        OP(C7, 5,   call(0x00)); // RST 0
        OP(C8, 5,   if (flags.Z) ret());
        OP(C9, 5,   ret());
        OP(CA, 4,   {uint16_t addr = readWord(); if (flags.Z) PC = addr;});
        OP(CB, 4,   execCB(fetchByte()));
        OP(CC, 4,   {uint16_t addr = readWord(); if (flags.Z) {T++; call(addr);}});
        OP(CD, 4,   {uint16_t addr = readWord(); call(addr);});
        OP(CE, 4,   A = adc(rd(PC++)));
        OP(CF, 5,   call(0x08)); // RST 8

        OP(D0, 5,   if (!flags.C) ret());
        OP(D1, 4,   DE = pop());
        OP(D2, 4,   {uint16_t addr = readWord(); if (!flags.C) PC = addr;});
        OP(D3, 4,   {uint16_t addr = rd(PC++) | (A<<8); out(addr, A);});
        OP(D4, 4,   {uint16_t addr = readWord(); if (!flags.C) {T++; call(addr);}});
        OP(D5, 5,   push(DE));
        OP(D6, 4,   A = sub(rd(PC++)));
        OP(D7, 5,   call(0x10)); // RST 10H
        OP(D8, 5,   if (flags.C) ret());
        OP(D9, 4,   std::swap(BC, B_C_); std::swap(DE, D_E_); std::swap(HL, H_L_);); // EXX
        OP(DA, 4,   {uint16_t addr = readWord(); if (flags.C) PC = addr;});
        OP(DB, 4,   {uint16_t addr = rd(PC++) | (A<<8); A = in(addr);});
        OP(DC, 4,   {uint16_t addr = readWord(); if (flags.C) {T++; call(addr);}});
        OP(DD, 4,   execDD(fetchByte()));
        OP(DE, 4,   A = sub(rd(PC++), flags.C));
        OP(DF, 5,   call(0x18)); // RST 18H

        OP(E0, 5,   if (!flags.P) ret());
        OP(E1, 4,   HL = pop());
        OP(E2, 4,   {uint16_t addr = readWord(); if (!flags.P) PC = addr;});
        OP(E3, 4,   {uint16_t addr = pop(); T++; push(HL); T += 2; HL = addr;}); // EX (SP),HL
        OP(E4, 4,   {uint16_t addr = readWord(); if (!flags.P) {T++; call(addr);}});
        OP(E5, 5,   push(HL));
        OP(E6, 4,   and_(rd(PC++)));
        OP(E7, 5,   call(0x20)); // RST 20H
        OP(E8, 5,   if (flags.P) ret());
        OP(E9, 4,   PC = HL); // JP (HL)
        OP(EA, 4,   {uint16_t addr = readWord(); if (flags.P) PC = addr;});
        OP(EB, 4,   std::swap(HL, DE)); // EX DE,HL
        OP(EC, 4,   {uint16_t addr = readWord(); if (flags.P) {T++; call(addr);}});
        OP(ED, 4,   execED(fetchByte()));
        OP(EE, 4,   xor_(rd(PC++)));
        OP(EF, 5,   call(0x28)); // RST 28H

        OP(F0, 5,   if (!flags.S) ret());
        OP(F1, 4,   AF = pop());
        OP(F2, 4,   {uint16_t addr = readWord(); if (!flags.S) PC = addr;});
        OP(F3, 4,   IFF1 = IFF2 = 0); // DI
        OP(F4, 4,   {uint16_t addr = readWord(); if (!flags.S) {T++; call(addr);}});
        OP(F5, 5,   push(AF));
        OP(F6, 4,   or_(rd(PC++)));
        OP(F7, 5,   call(0x30)); // RST 30H
        OP(F8, 5,   if (flags.S) ret());
        OP(F9, 6,   SP = HL);
        OP(FA, 4,   {uint16_t addr = readWord(); if (flags.S) PC = addr;});
        OP(FB, 4,   IFF1 = IFF2 = 1);
        OP(FC, 4,   {uint16_t addr = readWord(); if (flags.S) {T++; call(addr);}});
        OP(FD, 4,   execFD(fetchByte()));
        OP(FE, 4,   cp(rd(PC++)));
        OP(FF, 5,   call(0x38)); // RST 38H
    }
}

void Z80::execCB(uint8_t opcode)
{

}

void Z80::execDD(uint8_t opcode)
{

}

void Z80::execED(uint8_t opcode)
{

}

void Z80::execFD(uint8_t opcode)
{

}

#undef OP

uint8_t Z80::fetchByte()
{
    return mem[PC++];
}

uint16_t Z80::readWord()
{
    uint16_t w = rd(PC++);
    w |= rd(PC++) << 8UL;
    return w;
}

void Z80::out(uint16_t addr, uint8_t data)
{

}

uint8_t Z80::in(uint16_t addr)
{

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



void Z80::sleepCycles(int n)
{
    //std::this_thread::sleep_for(n * 400ns);
}

void Z80::test()
{
    A = 0x23;
    B = 0xF3;
    add(B);

//    for (int k=0; k<2; k++)
//    {
//        for (int i=0; i<255; i++)
//        {
//            B = i;
//            for (int j=0; j<255; j++)
////            int j = 0;
//            {
//                A = j;
//                F = 0;
//                uint8_t tmp = sub8(B, k);
//                uint8_t fl = F;
//                F = 0;
//                A = sub(B, k);
//                if (A != tmp)
//                {
//                    qDebug() << "pizdec @ A ="<<tmp<<"B ="<<B;
//                    return;
//                }
//                if ((F & 0xFF) != (fl & 0xFF))
//                {
//                    qDebug() << "flagi @ add8 ="<<fl<<"add ="<<F<<"; A ="<<j<<"B ="<<B << "C =" << k;
//    //                break;
//                }
//            }
//        }
//    }

//    mem[0] = 0x01;
//    mem[1] = 0x42;
//    mem[2] = 0x23;
//    mem[3] = 0x03;
//    mem[4] = 0x02;
//    mem[5] = 0x07;

//    mem[0x2343] = 0x87;

//    for (int i=0; i<10; i++)
//        step();
}

