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
            qDebug() << "WR" << Qt::hex << addr << "<-" << data;
        else
        {
            data = 23;
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
    scrWidget->setFixedSize(256, 192);

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
    memset(mem+0x4000, 0, 0xC000);
    cpu->reset();
    updateRegs();
}

void MainWindow::step()
{
    m_running = false;
    cpu->halt = false;
    cpu->step();
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
    uint16_t bkpt = bkptEdit->text().toInt(&ok, 16);

    QElapsedTimer perftimer;
    if (etimer.isValid())
    {
        qint64 frame_ns = etimer.nsecsElapsed();
        int N = frame_ns * 3500000 / 1000000000;
        etimer.start();
        int endT = cpu->T + N;
        perftimer.start();
        if (m_running)
        {
            while (cpu->T < endT)
            {
                cpu->step();

                if (bkpt == cpu->PC)
                    cpu->halt = true;

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
    scrWidget->setPixmap(QPixmap::fromImage(img));

    status->setNum(perf);
}


