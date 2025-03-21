#ifndef Z80_H
#define Z80_H

#include <cstdint>
#include <functional>
#include <QString>
#include <QDataStream>

class Z80
{
public:
    Z80(void *memory);

    void reset();
    void step();
    void stop() {halt = 1;}
    void run() {halt = 0;}

    void irq();

    qint64 cyclesCount() const {return T;}
    uint16_t programCounter() const {return PC;}

    void dump();

    void saveState(QDataStream &out);
    void restoreState(QDataStream &in);

    std::function<void(uint16_t addr, uint8_t &data, bool wr)> ioreq;
//    std::function<void(void)> iack;

    // for internal use:
    void test();

private:
    friend class CpuWidget;

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

    bool IFF1, IFF2; // interrupt flags
    uint8_t IM; // interrupt mode

    bool halt;
    bool blk = false; // block operation in progress
    bool _int = false; // interrupt request
    int enable_interrupt = 0;
    int T = 0; // cycles

    void exec(uint8_t opcode);
    void execCB();
    void execED();

    uint8_t *mem = nullptr;

    uint16_t bkpt = 0;

    void wr(uint16_t addr, uint8_t value);
    uint8_t rd(uint16_t addr);
    uint8_t fetchByte();
    uint8_t fetchNop();
    uint16_t readWord();
    void out(uint16_t addr, uint8_t data);
    uint8_t in(uint16_t addr);
    uint8_t in(); // IN r,(BC)

    void nmi();
    void acceptInt();

    uint8_t prefix = 0x00;
    uint16_t pointer();

    void push(uint16_t v);
    uint16_t pop();
    void call(uint16_t addr);
    void ret();

    // ALU
    uint8_t inc(uint8_t v);
    uint8_t dec(uint8_t v);
    void add(uint8_t v);
    void adc(uint8_t v);
    void sub(uint8_t v);
    void sbc(uint8_t v);
    void and_(uint8_t v);
    void or_(uint8_t v);
    void xor_(uint8_t v);
    void cp(uint8_t v);
    uint16_t add16(uint16_t v1, uint16_t v2);
    uint16_t adc16(uint16_t v1, uint16_t v2);
//    uint16_t sub16(uint16_t v1, uint16_t v2);
    uint16_t sbc16(uint16_t v1, uint16_t v2);
    void rrd();
    void rld();
    void rlca();
    void rrca();
    void rla();
    void rra();
    void daa();
    void cpl();
    void neg();
    uint8_t rlc(uint8_t v);
    uint8_t rrc(uint8_t v);
    uint8_t rl(uint8_t v);
    uint8_t rr(uint8_t v);
    uint8_t sla(uint8_t v);
    uint8_t sra(uint8_t v);
    uint8_t sll(uint8_t v);
    uint8_t srl(uint8_t v);

    // block commands
    void ldi();
    void ldd();
    void cpi();
    void cpd();
    void ini();
    void ind();
    void outi();
    void outd();

    inline static bool parity(uint8_t v)
    {
        return ((0x9669 >> ((v ^ (v >> 4)) & 0xF)) & 1);
    }

    QString flagString();

    uint16_t lastPC = 0;
};


#endif // Z80_H
