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
        if (wr)
        {
            qDebug() << "WR" << Qt::hex << addr << "<-" << data;
            if ((addr & 0xFF) == 0xFE)
            {
                port254 = data;
            }
            else
            {

            }
        }
        else
        {
            if (addr == 0xFEFE)
            {
                data = 0xFF;
//                static uint8_t jojo = 0;
//                if (m_keysPressed.size())
//                    data = jojo++;//m_keysPressed.firstKey();
//                else
//                    data = 0;
            }
            qDebug() << "RD" << Qt::hex << addr << "->" << data;
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
    timer->start(20);

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
    cpu->halt = false;
    cpu->step(true);
    updateRegs();
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
    cpu->bkpt = bkptEdit->text().toInt(&ok, 16);

    // hor line 64us - visual 52us
    // 625 lines per frame interlaced

    constexpr int cpuFreq = 3500000;
    constexpr int videoFreq = 7375000;
    constexpr int cyclesPerFrame = cpuFreq / 25;
    constexpr int cyclesPerLine = cyclesPerFrame / 625;
    constexpr int cyclesHSync = cyclesPerLine * 4 / 64;
    constexpr int cyclesBackPorch = cyclesPerLine * 8 / 64;
    constexpr int lineStartT = cyclesHSync + cyclesBackPorch;
    uint32_t *scrBuf = reinterpret_cast<uint32_t*>(frm.bits());
    videoptr = scrBuf;

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
                        if (frm_x >= 320)
                            frm_x = 319;
                        uint32_t *last = scrBuf + frm_y*320 + frm_x;
                        while (videoptr < last)
                            *videoptr++ = ZxScreen::zxColor(port254 & 7, 0);
                    }
                }

                if (cpu->halt)
                {
                    m_running = false;
                    break;
                }
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

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    int key = e->key();
    m_keysPressed[key] = true;
    cpu->irq(0);
}

void MainWindow::keyReleaseEvent(QKeyEvent *e)
{
    int key = e->key();
    m_keysPressed.remove(key);
    cpu->irq(0);
}
