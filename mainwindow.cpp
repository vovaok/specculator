#include "mainwindow.h"
#include <thread>
#include <chrono>
#include <QElapsedTimer>

using namespace std::literals;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    cpu = new Z80(mem);
    cpu->ioreq = [=](uint16_t addr, uint8_t &data, bool wr){
        uint8_t port = addr & 0xFF;
        if (wr)
        {
            if (port == 0xFE)
            {
                port254 = data;
            }
            else
            {
//                qDebug() << "WR" << Qt::hex << addr << "<-" << data;
            }
        }
        else
        {
            if (port == 0xFE)
            {
                data = keyb->readKeys(addr >> 8);
//                data.bit6 = mafon;
            }
            else
            {
//                qDebug() << "RD" << Qt::hex << addr << "->" << data;
            }
        }
    };


    QFile f("1982.rom");
    if (f.open(QIODevice::ReadOnly))
    {
        int sz = f.read(reinterpret_cast<char*>(mem), 16384);
        f.close();
        if (sz != 16384)
            qDebug() << "WARNING! unexpected ROM size";
    }

    scr = new ZxScreen((char *)mem + 0x4000, 320, 240);
    scr->bindBorderPort(&port254);

    keyb = new ZxKeyboard(keyport);

    tap = new ZxTape();
    tap->bindPlayPort(keyport + 7);
    tap->bindRecPort(&port254);
//    tap->openTap("test.TAP"); // now do it in the tapeWidget

    beeper = new ZxBeeper(&port254);


//    cpu->test();

    QToolBar *toolbar = addToolBar("main");
    toolbar->addAction("reset", this, &MainWindow::reset)->setShortcut(QKeySequence("F2"));
    toolbar->addAction("step", this, &MainWindow::step)->setShortcut(QKeySequence("F10"));
    toolbar->addAction("run", this, &MainWindow::run)->setShortcut(QKeySequence("F5"));
    toolbar->addAction("tape", this, [this](){tapeWidget->setVisible(!tapeWidget->isVisible());})->setShortcut(QKeySequence("F7"));


    toolbar->addAction("quick save", this, [=](){
        QFile f("snap.z80");
        if (f.open(QIODevice::WriteOnly))
        {
            f.write(reinterpret_cast<const char *>(mem), sizeof(mem));
            QDataStream out(&f);
            cpu->saveState(out);
            out << port254;
            f.close();
        }
    })->setShortcut(QKeySequence("F8"));

    toolbar->addAction("quick load", this, [=](){
        QFile f("snap.z80");
        if (f.open(QIODevice::ReadOnly))
        {
            f.read(reinterpret_cast<char *>(mem), sizeof(mem));
            QDataStream in(&f);
            cpu->restoreState(in);
            in >> port254;
            f.close();
        }
    })->setShortcut(QKeySequence("F9"));



    setStyleSheet("font-family: 'Consolas';");

    scrWidget = new ScreenWidget();
    scrWidget->bindScreen(scr);
//    scrWidget->setFixedSize(320, 240);
    scrWidget->setFocusPolicy(Qt::StrongFocus);
    scrWidget->setFocus();

    cpuWidget = new CpuWidget;
    cpuWidget->hide();
    cpuWidget->bindCpu(cpu);

    tapeWidget = new TapeWidget(tap);
    tapeWidget->hide();
    tapeWidget->open("test.TAP");

    status = new QLabel();

    QHBoxLayout *lay = new QHBoxLayout;
    setCentralWidget(new QWidget());
    centralWidget()->setLayout(lay);
    lay->addWidget(tapeWidget);
    lay->addWidget(cpuWidget);
    lay->addWidget(scrWidget, 1);

    statusBar()->addWidget(status);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateScreen);
    timer->start(16);

    reset();
    run();
}

MainWindow::~MainWindow()
{
    delete cpu;
}

void MainWindow::reset()
{
    scr->clear();
    memset(mem+0x4000, 0, 0xC000);
    cpu->reset();
    updateScreen();
}

void MainWindow::step()
{
    m_running = false;
    cpuWidget->show();

    cpu->run();
    doStep();
    cpu->stop();
}

void MainWindow::run()
{
    m_running = true;
    cpuWidget->hide();
    cpu->run();
}


void MainWindow::updateScreen()
{
    bool turbo = keyb->keyState(27);

    QElapsedTimer perftimer;
    if (etimer.isValid() || turbo)
    {
        qint64 frame_ns = etimer.nsecsElapsed();
        qint64 N = frame_ns * (cpuFreq / 100000) / 10000;
        etimer.start();
        if (turbo)
        {
            N = cpuFreq / 2;
//            etimer.invalidate();
        }
//        if (perf > 90)
//        {
//            N = N * 90 / perf;
//        }
        qint64 endT = cpu->cyclesCount() + N;
        if (N > cpuFreq)
            N = cpuFreq;

        perftimer.start();
        if (m_running)
        {
            while (cpu->cyclesCount() < endT)
            {
                if (endT - cpu->cyclesCount() > cpuFreq)
                {
                    qDebug() << "WUT??";
                    break;
                }

                doStep();
            }
        }
        qint64 run_ns = perftimer.nsecsElapsed();
        perf = run_ns * 100.0 / frame_ns;
    }
    else
    {
        etimer.start();
    }

    if (cpuWidget)
        cpuWidget->updateRegs();

    // this is done synchronized with CPU cycles later:
//    if (scrWidget)
//        scrWidget->update();

    if (tapeWidget)
        tapeWidget->updateState();

    status->setNum(perf);
}

void MainWindow::doStep()
{
    qint64 oldT = cpu->cyclesCount();
    cpu->step();
    qint64 cpuT = cpu->cyclesCount();
    int dt_ns = (cpuT - oldT) * 10'000 / (cpuFreq / 100'000);

    // generate interrupt
    if (m_running)
    {
        int ot = oldT % (cpuFreq / intFreq);
        int t = cpuT % (cpuFreq / intFreq);
        if (ot > t) // interrupt on timer overflow
        {
            cpu->irq();
            if (scrWidget)
                scrWidget->update();
        }
    }

    scr->update(cpuT);

    //! @todo reimplement this as hook
    if (cpu->programCounter() == 0x0556 && !tap->isPlaying())
    {
        tap->play();
//        qDebug() << "Start the tape";
    }
    else if (cpu->programCounter() == 0x04C2 && !tap->isRecording())
    {
//        qDebug() << "Start tape recording";
        tap->rec();
    }

    if (tap->isPlaying() || tap->isRecording())
        tap->update(dt_ns);

//    if (tap->isPlaying())
//    {
//        if (m_keyport[7] & 0x40)
//            port254 |= 0x10;
//        else
//            port254 &= ~0x10;
//    }
    beeper->update(dt_ns);
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    if (e->isAutoRepeat())
        return;
    int key = e->nativeVirtualKey();
    keyb->setKeyState(key, true);
}

void MainWindow::keyReleaseEvent(QKeyEvent *e)
{
    if (e->isAutoRepeat())
        return;
    int key = e->nativeVirtualKey();
    keyb->setKeyState(key, false);
}
