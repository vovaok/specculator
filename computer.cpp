#include "computer.h"

Computer::Computer(QObject *parent)
    : QThread{parent}
{
}

void Computer::reset()
{
    if (!m_cpu)
        return;

    m_scr->clear();
    memset(m_mem+0x4000, 0, 0xC000);
    m_cpu->reset();
}

void Computer::pause()
{
    if (!m_cpu)
        return;

    m_running = false;
    m_cpu->stop();
}

void Computer::step()
{
    if (!m_cpu)
        return;

    m_running = false;
    m_cpu->run();
    doStep();
    m_cpu->stop();
}

void Computer::resume()
{
    if (!m_cpu)
        return;

    m_running = true;
    m_cpu->run();
}

void Computer::saveState(QString path)
{
    if (!m_cpu)
        return;

    bool tmp = m_running;
    m_running = false;
    QFile f(path);
    if (f.open(QIODevice::WriteOnly))
    {
        f.write(reinterpret_cast<const char *>(m_mem), 0x10000); /// @todo ZxMemory class
        QDataStream out(&f);
        m_cpu->saveState(out);
        out << m_port254;
        f.close();
    }
    m_running = tmp;
}

void Computer::restoreState(QString path)
{
    if (!m_cpu)
        return;

    bool tmp = m_running;
    m_running = false;
    QFile f(path);
    if (f.open(QIODevice::ReadOnly))
    {
        f.read(reinterpret_cast<char *>(m_mem), 0x10000); /// @todo ZxMemory class
        QDataStream in(&f);
        m_cpu->restoreState(in);
        in >> m_port254;
        f.close();
    }
    m_running = tmp;
}

void Computer::run()
{
    m_mem = new uint8_t[0x10000];

    m_cpu = new Z80(m_mem);
    m_cpu->ioreq = [=](uint16_t addr, uint8_t &data, bool wr){
        uint8_t port = addr & 0xFF;
        if (wr)
        {
            if (port == 0xFE)
                m_port254 = data;
//            else
//                qDebug() << "WR" << Qt::hex << addr << "<-" << data;
        }
        else
        {
            if (port == 0xFE)
                data = m_keyb->readKeys(addr >> 8); // data.bit6 = mafon;
            else
            {
                data = 0xFF;
//                qDebug() << "RD" << Qt::hex << addr << "->" << data;
            }
        }
    };

    QFile f(":/rom/1982.rom");
    if (f.open(QIODevice::ReadOnly))
    {
        int sz = f.read(reinterpret_cast<char*>(m_mem), 16384);
        f.close();
        if (sz != 16384)
            qDebug() << "WARNING! unexpected ROM size";
    }
    else
    {
        qDebug() << "WARNING! ROM file not found :(";
    }

    m_scr = new ZxScreen((char *)m_mem + 0x4000, 320, 240);
    m_scr->bindBorderPort(&m_port254);

    m_keyb = new ZxKeyboard(m_keyport);

    m_tap = new ZxTape();
    m_tap->bindPlayPort(m_keyport + 7);
    m_tap->bindRecPort(&m_port254);

    m_beeper = new ZxBeeper(&m_port254);

//    cpu->test();

    emit powerOn();

    reset();

    restoreState("autosave.snap");

    resume();

    QElapsedTimer etimer;

    while (!isInterruptionRequested())
    {
        qint64 frame_ns = base_frame_ns;
        qint64 elapsed_ns = etimer.nsecsElapsed();
        if (etimer.isValid() && !turbo)
            frame_ns = elapsed_ns;
        int N = frame_ns * (cpuFreq / 100000) / 10000;
        etimer.start();

        qint64 endT = m_cpu->cyclesCount() + N;
        while (m_cpu->cyclesCount() < endT && m_running)
            doStep();

        qint64 run_ns = etimer.nsecsElapsed();
        m_cpuUsagePercent = run_ns * 100 / elapsed_ns;

        if (!turbo && run_ns < base_frame_ns)
            usleep((base_frame_ns - run_ns) / 1000);

        if (m_saveState)
        {
            saveState(m_snapshotFilename);
            m_saveState = false;
        }
        if (m_restoreState)
        {
            restoreState(m_snapshotFilename);
            m_restoreState = false;
        }
    }

    m_running = false;

    saveState("autosave.snap");

    emit powerOff();

    safeDelete(m_scr);
    safeDelete(m_keyb);
    safeDelete(m_tap);
    safeDelete(m_beeper);
    safeDelete(m_cpu);
    delete [] m_mem;
}

void Computer::doStep()
{
    qint64 oldT = m_cpu->cyclesCount();
    m_cpu->step();
    qint64 cpuT = m_cpu->cyclesCount();
    int dt_ns = (cpuT - oldT) * 10'000 / (cpuFreq / 100'000);

    // generate interrupt
    if (m_running)
    {
        int ot = oldT % (cpuFreq / intFreq);
        int t = cpuT % (cpuFreq / intFreq);
        if (ot > t) // interrupt on timer overflow
        {
            emit vsync();
            m_cpu->irq();
        }
    }

    m_scr->update(cpuT);

    //! @todo reimplement this as hook
    if (m_cpu->programCounter() == 0x0556 && !m_tap->isPlaying())
    {
        m_tap->play();
//        qDebug() << "Start the tape";
    }
    else if (m_cpu->programCounter() == 0x04C2 && !m_tap->isRecording())
    {
//        qDebug() << "Start tape recording";
        m_tap->rec();
    }

    if (m_tap->isPlaying() || m_tap->isRecording())
        m_tap->update(dt_ns);

//    if (tap->isPlaying())
//    {
//        if (m_keyport[7] & 0x40)
//            port254 |= 0x10;
//        else
//            port254 &= ~0x10;
//    }

    m_beeper->update(dt_ns);
}

template<typename T>
void Computer::safeDelete(T *&x)
{
    T *tmp = x;
    x = nullptr;
    delete tmp;
}
