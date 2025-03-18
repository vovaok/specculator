#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include <QDebug>
#include "z80.h"
#include "zxscreen.h"
#include "zxkeyboard.h"
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
    ZxKeyboard *keyb;
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

    uint8_t keyport[8] {0};

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


};

#endif // MAINWINDOW_H
