#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include "z80.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    Z80 *cpu = nullptr;
    uint8_t mem[0x10000];

    void reset();
};

#endif // MAINWINDOW_H
