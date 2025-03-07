#include "mainwindow.h"
#include <thread>
#include <chrono>
#include <QElapsedTimer>

using namespace std::literals;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    cpu = new Z80(mem);

    reset();

    cpu->test();


//    QElapsedTimer etimer;
//    for (int i=0; i<10; i++)
//    {
//        etimer.start();
//        std::this_thread::sleep_for(400ns);
////        qDebug() << etimer.nsecsElapsed();
//    }

}

MainWindow::~MainWindow()
{
    delete cpu;
}

void MainWindow::reset()
{
    memset(mem, 0, sizeof(mem));
    cpu->reset();
}


