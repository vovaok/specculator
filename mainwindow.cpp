#include "mainwindow.h"
#include <thread>
#include <chrono>
#include <QElapsedTimer>

using namespace std::literals;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setStyleSheet("font-family: 'Consolas';");

    scrWidget = new ScreenWidget();
    scrWidget->setFocusPolicy(Qt::StrongFocus);
    scrWidget->setFocus();

    cpuWidget = new CpuWidget;
    cpuWidget->hide();

    tapeWidget = new TapeWidget;
    tapeWidget->hide();

    computer = new Computer();
    connect(computer, &Computer::powerOn, this, &MainWindow::bindWidgets);
    connect(computer, &Computer::powerOff, this, &MainWindow::unbindWidgets);
    connect(computer, &Computer::vsync, this, &MainWindow::updateScreen);
    computer->start();


    QToolBar *toolbar = addToolBar("main");
    toolbar->addAction("reset", this, &MainWindow::reset)->setShortcut(QKeySequence("F2"));
    toolbar->addAction("step", this, &MainWindow::step)->setShortcut(QKeySequence("F10"));
    toolbar->addAction("run", this, &MainWindow::run)->setShortcut(QKeySequence("F5"));
    toolbar->addAction("tape", this, [this](){tapeWidget->setVisible(!tapeWidget->isVisible());})->setShortcut(QKeySequence("F7"));
    toolbar->addAction("quick save", computer, &Computer::save)->setShortcut(QKeySequence("F8"));
    toolbar->addAction("quick load", computer, &Computer::restore)->setShortcut(QKeySequence("F9"));

    status = new QLabel();

    QHBoxLayout *lay = new QHBoxLayout;
    setCentralWidget(new QWidget());
    centralWidget()->setLayout(lay);
    lay->addWidget(tapeWidget);
    lay->addWidget(cpuWidget);
    lay->addWidget(scrWidget, 1);

    statusBar()->addWidget(status);

//    QTimer *timer = new QTimer(this);
//    connect(timer, &QTimer::timeout, this, &MainWindow::updateScreen);
//    timer->start(20);
}

MainWindow::~MainWindow()
{
}

void MainWindow::reset()
{
    computer->reset();
    updateScreen();
}

void MainWindow::step()
{
    computer->step();
    cpuWidget->show();
    cpuWidget->updateRegs();
}

void MainWindow::run()
{
    cpuWidget->hide();
}

void MainWindow::bindWidgets()
{
    scrWidget->bindScreen(computer->screen());
    cpuWidget->bindCpu(computer->cpu());
    tapeWidget->bindTape(computer->tape());
}

void MainWindow::unbindWidgets()
{
    scrWidget->bindScreen(nullptr);
    cpuWidget->bindCpu(nullptr);
    tapeWidget->bindTape(nullptr);
}

void MainWindow::updateScreen()
{
    if (scrWidget)
        scrWidget->update();

//    if (cpuWidget)
//        cpuWidget->updateRegs();

    if (tapeWidget)
        tapeWidget->updateState();

//    status->setText(QString("%1").arg(cpu->cyclesCount()));
    status->setText(QString("CPU usage:%1%").arg(computer->cpuUsage(), 3));
//    status->setText(QString("%1/%2").arg(run_us).arg(frame_us));
    //    qDebug() << QString("%1/%2 - %3").arg(run_us).arg(frame_us).arg(NN);
}

void MainWindow::closeEvent(QCloseEvent *)
{
    computer->requestInterruption();
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    if (e->isAutoRepeat())
        return;
    int key = e->key();
    if (key == Qt::Key_Escape)
        computer->turbo = true;
    if (computer->keyboard())
        computer->keyboard()->setKeyState(key, true);
}

void MainWindow::keyReleaseEvent(QKeyEvent *e)
{
    if (e->isAutoRepeat())
        return;
    int key = e->key();
    if (key == Qt::Key_Escape)
        computer->turbo = false;
    if (computer->keyboard())
        computer->keyboard()->setKeyState(key, false);
}
