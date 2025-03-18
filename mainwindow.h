#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include <QDebug>
#include "z80.h"
#include "zxscreen.h"
#include "zxtape.h"
#include "zxbeeper.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    Z80 *cpu = nullptr;
    uint8_t mem[0x10000];
    ZxScreen *scr;
    ZxTape *tap;
    ZxBeeper *beeper;

    bool m_running = false;

    QLabel *scrWidget;
    QMap<QString, QLineEdit *> regEdits;
    QLineEdit *bkptEdit;
    QLabel *status;
    QElapsedTimer etimer;
    double perf;

    constexpr static int cpuFreq = 3500000;
    constexpr static int videoFreq = 7375000;
    constexpr static int intFreq = 50;
    constexpr static int cyclesPerFrame = cpuFreq / 25;
    constexpr static int cyclesPerLine = cyclesPerFrame / 625;
    constexpr static int cyclesHSync = cyclesPerLine * 4 / 64;
    constexpr static int cyclesBackPorch = cyclesPerLine * 8 / 64;
    constexpr static int lineStartT = cyclesHSync + cyclesBackPorch;

    union
    {
        uint8_t port254;
        struct
        {
            uint8_t borderR: 1;
            uint8_t borderG: 1;
            uint8_t borderB: 1;
            uint8_t tape: 1;
            uint8_t beep: 1;
        };
    };

    QImage frm;
    uint32_t *videoptr;

    void reset();
    void step();
    void run();

    void updateRegs();
    void updateScreen();

    void doStep();

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;

    enum ZxKeyCode
    {
        Key_CS  = 0x01,
        Key_Z   = 0x02,
        Key_X   = 0x04,
        Key_C   = 0x08,
        Key_V   = 0x10,
        Key_A   = 0x21,
        Key_S   = 0x22,
        Key_D   = 0x24,
        Key_F   = 0x28,
        Key_G   = 0x30,
        Key_Q   = 0x41,
        Key_W   = 0x42,
        Key_E   = 0x44,
        Key_R   = 0x48,
        Key_T   = 0x50,
        Key_1   = 0x61,
        Key_2   = 0x62,
        Key_3   = 0x64,
        Key_4   = 0x68,
        Key_5   = 0x70,
        Key_0   = 0x81,
        Key_9   = 0x82,
        Key_8   = 0x84,
        Key_7   = 0x88,
        Key_6   = 0x90,
        Key_P   = 0xA1,
        Key_O   = 0xA2,
        Key_I   = 0xA4,
        Key_U   = 0xA8,
        Key_Y   = 0xB0,
        Key_Enter = 0xC1,
        Key_L   = 0xC2,
        Key_K   = 0xC4,
        Key_J   = 0xC8,
        Key_H   = 0xD0,
        Key_Space = 0xE1,
        Key_SS  = 0xE2,
        Key_M   = 0xE4,
        Key_N   = 0xE8,
        Key_B   = 0xF0,
    };

    QMap<int, int> m_keysPressed;
    uint8_t m_keyport[8] {0};
    QMap<int, QVector<ZxKeyCode>> m_keyMap;

    void updateKeyboard();
    uint8_t readKeys(uint8_t addr);
};

#endif // MAINWINDOW_H
