#ifndef ZXBEEPER_H
#define ZXBEEPER_H

#include <QAudioOutput>
#include <QThread>
#include <QWaitCondition>
#include <QMutex>

class ZxBeeper : public QThread
{
    Q_OBJECT
public:
    ZxBeeper(uint8_t *port, uint8_t *tapePort);
    virtual ~ZxBeeper();

    constexpr static int m_audioBuf_ms = 50;

    void update(int dt_ns);

    void setTapeVolume(uint8_t value) {m_tapeVolume = value;}

    void requestInterruption();

    qint64 avg() const {return m_avg;}

protected:
    void run() override;

private:
    uint8_t *m_port = nullptr;
    uint8_t *m_tapePort = nullptr;
    QAudioOutput *audioOutput = nullptr;
    QIODevice *m_device = nullptr;
    QWaitCondition m_wait;

    int m_sampleFreq = 70000;
    int m_bufferSize = m_sampleFreq * m_audioBuf_ms / 1000;
    int16_t *m_buffer1 = nullptr;
    int16_t *m_buffer2 = nullptr;
    int16_t *m_buffer = nullptr;
    uint m_time_ns = 0;
    // filter:
    uint8_t m_tapeVolume = 32;
    qint64 m_signal = 0;
    qint64 m_avg = 0;
};

#endif // ZXBEEPER_H
