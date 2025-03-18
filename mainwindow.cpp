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
    tap->openTap("test.TAP");

    beeper = new ZxBeeper(&port254);


    bkptEdit = new QLineEdit(0);
    bkptEdit->setFixedWidth(64);
    bkptEdit->setFont(QFont("Consolas"));

    cpu->test();

    QToolBar *toolbar = addToolBar("main");
    toolbar->addAction("reset", this, &MainWindow::reset)->setShortcut(QKeySequence("F2"));
    toolbar->addAction("step", this, &MainWindow::step)->setShortcut(QKeySequence("F10"));
    toolbar->addAction("run", this, &MainWindow::run)->setShortcut(QKeySequence("F5"));
    toolbar->addAction("play tap", [=](){tap->play();});
//    toolbar->addAction("nmi", [=](){cpu->nmi();});
//    toolbar->addAction("int", [=](){cpu->irq();});
//    toolbar->addAction("testSAVE", [=](){cpu->call(1218);}); // SAVE
//    toolbar->addAction("testLOAD", [=](){cpu->call(0x05E7);}); // LOAD
//    toolbar->addAction("testBEEP", [=](){cpu->call(949);}); // BEEP

    toolbar->addWidget(bkptEdit);

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


    QFormLayout *reglay = new QFormLayout;

    QStringList regNames = {"FLAGS", "AF", "BC", "DE", "HL", "AF'", "BC'", "DE'", "HL'",
                            "PC", "SP", "IX", "IY", "I", "R", "IM", "IFF1", "IFF2", "T"};
    for (const QString &name: regNames)
    {
        QLineEdit *edit = new QLineEdit;
        edit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
        edit->setAlignment(Qt::AlignRight);
        edit->setFixedWidth(64);
        edit->setReadOnly(true);
        regEdits[name] = edit;
        reglay->addRow(name, edit);
    }

    scrWidget = new QLabel;
    scrWidget->setFixedSize(320, 240);//256, 192);
    scrWidget->setFocusPolicy(Qt::StrongFocus);
    scrWidget->setFocus();

    status = new QLabel();

    QHBoxLayout *lay = new QHBoxLayout;
    setCentralWidget(new QWidget());
    centralWidget()->setLayout(lay);
    lay->addLayout(reglay);
    lay->addWidget(scrWidget);

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
//    updateRegs();
    updateScreen();
}

void MainWindow::step()
{
    m_running = false;

    cpu->halt = false;
    doStep();
    cpu->halt = true;

//    updateRegs();
}

void MainWindow::run()
{
    m_running = true;
    cpu->halt = false;
}

void MainWindow::updateRegs()
{
    regEdits["AF"]->setText(QString().asprintf("%04X", cpu->AF));
    regEdits["BC"]->setText(QString().asprintf("%04X", cpu->BC));
    regEdits["DE"]->setText(QString().asprintf("%04X", cpu->DE));
    regEdits["HL"]->setText(QString().asprintf("%04X", cpu->HL));
    regEdits["AF'"]->setText(QString().asprintf("%04X", cpu->A_F_));
    regEdits["BC'"]->setText(QString().asprintf("%04X", cpu->B_C_));
    regEdits["DE'"]->setText(QString().asprintf("%04X", cpu->D_E_));
    regEdits["HL'"]->setText(QString().asprintf("%04X", cpu->H_L_));
    regEdits["PC"]->setText(QString().asprintf("%04X", cpu->PC));
    regEdits["SP"]->setText(QString().asprintf("%04X", cpu->SP));
    regEdits["IX"]->setText(QString().asprintf("%04X", cpu->IX));
    regEdits["IY"]->setText(QString().asprintf("%04X", cpu->IY));
    regEdits["I"]->setText(QString().asprintf("%02X", cpu->I));
    regEdits["R"]->setText(QString().asprintf("%02X", cpu->R));
    regEdits["IM"]->setText(QString("%1").arg(cpu->IM));
    regEdits["IFF1"]->setText(QString("%1").arg(cpu->IFF1));
    regEdits["IFF2"]->setText(QString("%1").arg(cpu->IFF2));
    regEdits["FLAGS"]->setText(cpu->flagString());
    regEdits["T"]->setText(QString::number(cpu->T));
}



void MainWindow::updateScreen()
{
    bool ok;
    uint16_t bkpt = bkptEdit->text().toInt(&ok, 16);
//    cpu->bkpt = bkptEdit->text().toInt(&ok, 16);

    // hor line 64us - visual 52us
    // 625 lines per frame interlaced

//    // generate interrupt
//    if (m_running)
//        cpu->irq();

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
        qint64 endT = cpu->T + N;
        if (N > cpuFreq)
            N = cpuFreq;

        perftimer.start();
        if (m_running)
        {
            while (cpu->T < endT)
            {
                if (endT - cpu->T > cpuFreq)
                {
                    qDebug() << "WUT??";
                    break;
                }

                if (bkpt && cpu->PC == bkpt)
                    cpu->halt = true;

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

    updateRegs();

//    for (int i= 0x4000; i<0x10000; i++)
//    {
//        if (mem[i] != mem2[i])
//        {
//            qDebug() << Qt::hex << "CARAMBA! mem @ " << i << ":" << mem[i] << mem2[i];
//            m_running = false;
//        }
//    }


    QImage img = scr->frame();
    scrWidget->setPixmap(QPixmap::fromImage(img));

    status->setNum(perf);
}

void MainWindow::doStep()
{
    qint64 oldT = cpu->T;
    cpu->step();
    int dt_ns = (cpu->T - oldT) * 10'000 / (cpuFreq / 100'000);

    // generate interrupt
    if (m_running)
    {
        int ot = oldT % (cpuFreq / intFreq);
        int t = cpu->T % (cpuFreq / intFreq);
        if (ot > t) // interrupt on timer overflow
            cpu->irq();
    }

    scr->update(cpu->T);

    if (cpu->PC == 0x0556 && !tap->isPlaying())
    {
        tap->play();
        qDebug() << "Start the tape";
    }
    else if (cpu->PC == 0x04C2 && !tap->isRecording())
    {
        qDebug() << "Start tape recording";
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
//    qDebug() << e->nativeVirtualKey();
}

void MainWindow::keyReleaseEvent(QKeyEvent *e)
{
    if (e->isAutoRepeat())
        return;
    int key = e->nativeVirtualKey();
    keyb->setKeyState(key, false);
//    qDebug() << "release" << key;
}
