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

    // Computer
    Z80 *cpu = nullptr;
    uint8_t mem[0x10000];
    ZxScreen *scr;
    ZxKeyboard *keyb;
    ZxTape *tap;
    ZxBeeper *beeper;

    uint8_t port254;
    uint8_t keyport[8] {0};

    bool m_running = false;

    void reset();
    void step();
    void run();

    constexpr static int cpuFreq = 3'500'000;
    constexpr static int videoFreq = 7'375'000;
    constexpr static int intFreq = 50;

    // GUI
    QLabel *scrWidget;
    QMap<QString, QLineEdit *> regEdits;
    QLineEdit *bkptEdit;
    QLabel *status;
    QElapsedTimer etimer;
    double perf;

    uint32_t *videoptr;


    void updateRegs();
    void updateScreen();

    void doStep();

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;


};

#endif // MAINWINDOW_H
