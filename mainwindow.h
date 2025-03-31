#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include <QDebug>

#include "computer.h"

#include "screenwidget.h"
#include "cpuwidget.h"
#include "tapewidget.h"
#include "keyboardwidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    Computer *computer = nullptr;

    // GUI
    ScreenWidget *scrWidget = nullptr;
    CpuWidget *cpuWidget = nullptr;
    TapeWidget *tapeWidget = nullptr;
    KeyboardWidget *keybWidget = nullptr;
    QLabel *status;

    void reset();
    void step();
    void run();

    void bindWidgets();
    void unbindWidgets();
    void updateScreen();

protected:
    void closeEvent(QCloseEvent *) override;
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
};

#endif // MAINWINDOW_H
