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
    QToolBar *m_toolbar;
    QGridLayout *m_layout;

    QAction *m_debugAction, *m_tapeAction, *m_keybAction;
    QToolButton *m_turboBtn;

    void reset();
    void setDebugMode(bool enabled);
//    void step();
//    void run();

    void bindWidgets();
    void unbindWidgets();
    void updateScreen();

protected:
    void closeEvent(QCloseEvent *) override;
    void resizeEvent(QResizeEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
};

#endif // MAINWINDOW_H
