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
                qDebug() << "WR" << Qt::hex << addr << "<-" << data;
            }
        }
        else
        {
            if (port == 0xFE)
            {
                data = readKeys(addr >> 8);
//                data.bit6 = mafon;
            }
            else
            {
                qDebug() << "RD" << Qt::hex << addr << "->" << data;
            }
        }
    };


    m_keyMap['1']   = {Key_1};
    m_keyMap['2']   = {Key_2};
    m_keyMap['3']   = {Key_3};
    m_keyMap['4']   = {Key_4};
    m_keyMap['5']   = {Key_5};
    m_keyMap['6']   = {Key_6};
    m_keyMap['7']   = {Key_7};
    m_keyMap['8']   = {Key_8};
    m_keyMap['9']   = {Key_9};
    m_keyMap['0']   = {Key_0};
    m_keyMap['Q']   = {Key_Q};
    m_keyMap['W']   = {Key_W};
    m_keyMap['E']   = {Key_E};
    m_keyMap['R']   = {Key_R};
    m_keyMap['T']   = {Key_T};
    m_keyMap['Y']   = {Key_Y};
    m_keyMap['U']   = {Key_U};
    m_keyMap['I']   = {Key_I};
    m_keyMap['O']   = {Key_O};
    m_keyMap['P']   = {Key_P};
    m_keyMap['A']   = {Key_A};
    m_keyMap['S']   = {Key_S};
    m_keyMap['D']   = {Key_D};
    m_keyMap['F']   = {Key_F};
    m_keyMap['G']   = {Key_G};
    m_keyMap['H']   = {Key_H};
    m_keyMap['J']   = {Key_J};
    m_keyMap['K']   = {Key_K};
    m_keyMap['L']   = {Key_L};
    m_keyMap[13]    = {Key_Enter};
    m_keyMap[16]    = {Key_CS};
    m_keyMap['Z']   = {Key_Z};
    m_keyMap['X']   = {Key_X};
    m_keyMap['C']   = {Key_C};
    m_keyMap['V']   = {Key_V};
    m_keyMap['B']   = {Key_B};
    m_keyMap['N']   = {Key_N};
    m_keyMap['M']   = {Key_M};
    m_keyMap[17]    = {Key_SS};
    m_keyMap[' ']   = {Key_Space};

    m_keyMap[8]     = {Key_CS, Key_0}; // backspace
    m_keyMap[37]    = {Key_CS, Key_5}; // arrow left
    m_keyMap[38]    = {Key_CS, Key_7}; // arrow up
    m_keyMap[39]    = {Key_CS, Key_8}; // arrow right
    m_keyMap[40]    = {Key_CS, Key_6}; // arrow down
    m_keyMap[19]    = {Key_CS, Key_Space}; // break


    QFile f("1982.rom");
    if (f.open(QIODevice::ReadOnly))
    {
        int sz = f.read(reinterpret_cast<char*>(mem), 16384);
        f.close();
        if (sz != 16384)
            qDebug() << "WARNING! unexpected ROM size";
    }


    //    frm = QImage(320, 240, QImage::Format_Indexed8);
    //    frm.setColorTable(zxColors);
    frm = QImage(320, 240, QImage::Format_ARGB32_Premultiplied);
    frm.fill(Qt::black);
    videoptr = reinterpret_cast<uint32_t*>(frm.bits());

    scr = new ZxScreen((char *)mem + 0x4000);


    bkptEdit = new QLineEdit(0);
    bkptEdit->setFixedWidth(64);
    bkptEdit->setFont(QFont("Consolas"));
//    cpu->test();

    QToolBar *toolbar = addToolBar("main");
    toolbar->addAction("reset", this, &MainWindow::reset)->setShortcut(QKeySequence("F2"));
    toolbar->addAction("step", this, &MainWindow::step)->setShortcut(QKeySequence("F10"));
    toolbar->addAction("run", this, &MainWindow::run)->setShortcut(QKeySequence("F5"));
    toolbar->addAction("nmi", [=](){cpu->nmi();});
    toolbar->addAction("int", [=](){cpu->irq(0);});
    toolbar->addAction("testSAVE", [=](){cpu->call(1218);}); // SAVE
    toolbar->addAction("testLOAD", [=](){cpu->call(0x05E7);}); // LOAD
    toolbar->addAction("testBEEP", [=](){cpu->call(949);}); // BEEP

    toolbar->addWidget(bkptEdit);

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
    frm.fill(Qt::black);
    memset(mem+0x4000, 0, 0xC000);
    cpu->reset();
//    updateRegs();
    updateScreen();
}

void MainWindow::step()
{
    m_running = false;

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

    // generate interrupt
    if (m_running)
        cpu->irq(0);

    QElapsedTimer perftimer;
    if (etimer.isValid())
    {
        qint64 frame_ns = etimer.nsecsElapsed();
        int N = frame_ns * cpuFreq / 1000000000;
        etimer.start();
        int endT = cpu->T + N;
        perftimer.start();
        if (m_running)
        {
            while (cpu->T < endT)
            {
                if (bkpt && cpu->PC == bkpt)
                    cpu->halt = true;

                if (cpu->halt)
                    break;

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



    QImage img = scr->toImage();
    QPainter pai(&frm);
    pai.drawImage(32, 24, img);
    pai.end();
    scrWidget->setPixmap(QPixmap::fromImage(frm));

    status->setNum(perf);
}

void MainWindow::doStep()
{
    uint32_t *scrBuf = reinterpret_cast<uint32_t*>(frm.bits());
    uint32_t *end = scrBuf + 320*240;

    cpu->halt = false;
    cpu->step();

    int frame_T = cpu->T % (cyclesPerFrame);
    int frame_line = frame_T / cyclesPerLine + 1; // [1 ... 625]
    if (frame_line > 312)
        frame_line -= 312;
    int frm_y = frame_line - 6 - 32;
    if (frm_y >= 0 && frm_y < 240)
    {
        int line_T = frame_T % cyclesPerLine;
        int frm_x = (line_T - lineStartT) * videoFreq / cpuFreq - 40;
        if (frm_x >= 0)
        {
            if (frm_x > 320)
                frm_x = 320;
            uint32_t *last = scrBuf + frm_y*320 + frm_x;
            while (videoptr < last)
                *videoptr++ = ZxScreen::zxColor(port254 & 7, 0);
            if (videoptr >= end)
                videoptr = scrBuf;
        }
    }
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    int key = e->nativeVirtualKey();
    qDebug() << e->nativeScanCode() << e->nativeVirtualKey();
    m_keysPressed[key] = key;
    updateKeyboard();
}

void MainWindow::keyReleaseEvent(QKeyEvent *e)
{
    int key = e->nativeVirtualKey();
    m_keysPressed.remove(key);
    updateKeyboard();
}

void MainWindow::updateKeyboard()
{
    for (int i=0; i<8; i++)
        m_keyport[i] = 0;

    for (int key: m_keysPressed)
    {
        for (ZxKeyCode zxkey: m_keyMap[key])
        {
            int idx = zxkey >> 5;
            uint8_t mask = zxkey & 0x1F;
            m_keyport[idx] |= mask;
        }
    }
}

uint8_t MainWindow::readKeys(uint8_t addr)
{
    for (int idx=0; idx<8; idx++)
        if (!(addr & (1 << idx)))
            return ~m_keyport[idx];
    return 0xFF;

//    union
//    {
//        uint8_t r = 0xFF;
//        struct
//        {
//            uint8_t d0: 1;
//            uint8_t d1: 1;
//            uint8_t d2: 1;
//            uint8_t d3: 1;
//            uint8_t d4: 1;
//        } bits;
//    };



//    switch (addr)
//    {
//    case 0xFE:
//        bits.d0 = !m_keysPressed[Qt::Key_Shift];
//        bits.d1 = !m_keysPressed[Qt::Key_Z];
//        bits.d2 = !m_keysPressed[Qt::Key_X];
//        bits.d3 = !m_keysPressed[Qt::Key_C];
//        bits.d4 = !m_keysPressed[Qt::Key_V];
//        break;
//    case 0xFD:
//        bits.d0 = !m_keysPressed[Qt::Key_A];
//        bits.d1 = !m_keysPressed[Qt::Key_S];
//        bits.d2 = !m_keysPressed[Qt::Key_D];
//        bits.d3 = !m_keysPressed[Qt::Key_F];
//        bits.d4 = !m_keysPressed[Qt::Key_G];
//        break;
//    case 0xFB:
//        bits.d0 = !m_keysPressed[Qt::Key_Q];
//        bits.d1 = !m_keysPressed[Qt::Key_W];
//        bits.d2 = !m_keysPressed[Qt::Key_E];
//        bits.d3 = !m_keysPressed[Qt::Key_R];
//        bits.d4 = !m_keysPressed[Qt::Key_T];
//        break;
//    case 0xF7:
//        bits.d0 = !m_keysPressed[Qt::Key_1];
//        bits.d1 = !m_keysPressed[Qt::Key_2];
//        bits.d2 = !m_keysPressed[Qt::Key_3];
//        bits.d3 = !m_keysPressed[Qt::Key_4];
//        bits.d4 = !m_keysPressed[Qt::Key_5];
//        break;
//    case 0xEF:
//        bits.d0 = !m_keysPressed[Qt::Key_0];
//        bits.d1 = !m_keysPressed[Qt::Key_9];
//        bits.d2 = !m_keysPressed[Qt::Key_8];
//        bits.d3 = !m_keysPressed[Qt::Key_7];
//        bits.d4 = !m_keysPressed[Qt::Key_6];
//        break;
//    case 0xDF:
//        bits.d0 = !m_keysPressed[Qt::Key_P];
//        bits.d1 = !m_keysPressed[Qt::Key_O];
//        bits.d2 = !m_keysPressed[Qt::Key_I];
//        bits.d3 = !m_keysPressed[Qt::Key_U];
//        bits.d4 = !m_keysPressed[Qt::Key_Y];
//        break;
//    case 0xBF:
//        bits.d0 = !(m_keysPressed[Qt::Key_Return] || m_keysPressed[Qt::Key_Enter]);
//        bits.d1 = !m_keysPressed[Qt::Key_L];
//        bits.d2 = !m_keysPressed[Qt::Key_K];
//        bits.d3 = !m_keysPressed[Qt::Key_J];
//        bits.d4 = !m_keysPressed[Qt::Key_H];
//        break;
//    case 0x7F:
//        bits.d0 = !m_keysPressed[Qt::Key_Space];
//        bits.d1 = !m_keysPressed[Qt::Key_Control];
//        bits.d2 = !m_keysPressed[Qt::Key_M];
//        bits.d3 = !m_keysPressed[Qt::Key_N];
//        bits.d4 = !m_keysPressed[Qt::Key_B];
//        break;
//    }
//    return r;
}
