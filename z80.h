#ifndef Z80_H
#define Z80_H

#include <cstdint>
#include <functional>

class Z80
{
public:
    Z80(void *memory);

    void reset();
    void irq(int N);

    void step();

private:
public:
    struct Flags
    {
        uint8_t C: 1; // carry
        uint8_t N: 1; // add/subtract
        uint8_t P: 1; // parity/overflow
        uint8_t _3: 1;
        uint8_t H: 1; // half carry
        uint8_t _5: 1;
        uint8_t Z: 1; // zero
        uint8_t S: 1; // sign
    };

#define Z80_REGPAIR(x, y, ...) \
    union {uint16_t x##y; struct {uint8_t y, x;}; __VA_ARGS__;}

#define Z80_REG(x) \
    union {uint16_t x; struct {uint8_t x##L, x##H;};}

    Z80_REGPAIR(A, F, Flags flags);
    Z80_REGPAIR(B, C);
    Z80_REGPAIR(D, E);
    Z80_REGPAIR(H, L);
    Z80_REGPAIR(A_, F_);
    Z80_REGPAIR(B_, C_);
    Z80_REGPAIR(D_, E_);
    Z80_REGPAIR(H_, L_);
    Z80_REG(IX);
    Z80_REG(IY);
    Z80_REG(SP);
    uint16_t PC;
    uint8_t I;
    uint8_t R;

    bool IFF1, IFF2;

    bool halt;
    int T = 0; // cycles

    void exec(uint8_t opcode);
    void execCB(uint8_t opcode);
    void execDD(uint8_t opcode);
    void execED(uint8_t opcode);
    void execFD(uint8_t opcode);

    uint8_t *mem = nullptr;

    void wr(uint16_t addr, uint8_t value) {mem[addr] = value; T += 3;}
    uint8_t rd(uint16_t addr) {return mem[addr]; T += 3;}
    uint8_t fetchByte();
    uint16_t readWord();

    void out(uint16_t addr, uint8_t data);
    uint8_t in(uint16_t addr);

    uint8_t inc(uint8_t v);
    uint8_t dec(uint8_t v);
    uint8_t add(uint8_t v);
    uint8_t adc(uint8_t v);
    uint8_t sub(uint8_t v, uint8_t c=0);
    void cp(uint8_t v);
    uint16_t add16(uint16_t v1, uint16_t v2);
    uint16_t adc16(uint16_t v1, uint16_t v2, uint8_t c);
    uint16_t sub16(uint16_t v1, uint16_t v2, uint8_t c);
    void and_(uint8_t v);
    void or_(uint8_t v);
    void xor_(uint8_t v);
    void rlca();
    void rrca();
    void rla();
    void rra();
    void daa();
    void cpl();
    uint8_t rlc(uint8_t v);
    uint8_t rrc(uint8_t v);
    uint8_t rl(uint8_t v);
    uint8_t rr(uint8_t v);
    uint8_t sla(uint8_t v);
    uint8_t sra(uint8_t v);
    uint8_t sll(uint8_t v);
    uint8_t srl(uint8_t v);


    void push(uint16_t v);
    uint16_t pop();
    void call(uint16_t addr);
    void ret();

    void sleepCycles(int n);

    union
    {
        uint16_t tmpw;
        struct
        {
            uint8_t ltw;
            uint8_t htw;
        };
    };

//    struct OpCode
//    {
//        int T; // cycle count
//        std::function<void(void)> exec;
//    };

//    OpCode opcodeSet[256]; // common set
//    OpCode opcodeSetCB[256]; // bitwise ops
//    OpCode opcodeSetED[256]; // extended ops
//    OpCode opcodeSetDD[256]; // ops with IX
//    OpCode opcodeSetFD[256]; // ops with IY

    void test();
};


#endif // Z80_H
