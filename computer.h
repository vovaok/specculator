#ifndef COMPUTER_H
#define COMPUTER_H

#include <QThread>
#include <QObject>

#include "z80.h"
#include "zxscreen.h"
#include "zxkeyboard.h"
#include "zxtape.h"
#include "zxbeeper.h"

class Computer : public QThread
{
    Q_OBJECT
public:
    explicit Computer(QObject *parent = nullptr);

    void reset();
    void pause();
    void step();
    void resume(); // ex run()

    bool isRunning() const {return m_running;}
    int cpuUsage() const {return lround(m_cpuUsagePercent);}

    // CPU & peripherals
    Z80         *cpu()      {return m_cpu;}
    uint8_t     *memory()   {return m_mem;}
    ZxScreen    *screen()   {return m_scr;}
    ZxKeyboard  *keyboard() {return m_keyb;}
    ZxTape      *tape()     {return m_tap;}
    ZxBeeper    *beeper()   {return m_beeper;}

    void setSnapshotFilename(QString path);
    void save() {m_saveState = true;}
    void restore() {m_restoreState = true;}

    bool turbo = false;

signals:
    void powerOn();
    void powerOff();
    void vsync();

protected:
    virtual void run() override;

private:
    Z80 *m_cpu = nullptr;
    uint8_t *m_mem = nullptr; //! @todo maybe make separate ZxMemory class
    ZxScreen *m_scr = nullptr;
    ZxKeyboard *m_keyb = nullptr;
    ZxTape *m_tap = nullptr;
    ZxBeeper *m_beeper = nullptr;

    uint8_t m_port254;
    uint8_t m_keyport[8] {0};

    bool m_running = false;
    int m_cpuUsagePercent = 0;

    constexpr static int cpuFreq = 3'500'000;
//    constexpr static int videoFreq = 7'375'000;
    constexpr static int intFreq = 50;
    constexpr static qint64 base_frame_ns = 1e+9 / intFreq;

    void doStep();

    /// @todo use thread messaging
    QString m_snapshotFilename = "0.snap";
    bool m_saveState = false;
    bool m_restoreState = false;
    void saveState(QString path);
    void restoreState(QString path);

    template<typename T>
    void safeDelete(T *&x);
};

#endif // COMPUTER_H
