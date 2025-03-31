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
    ZxBeeper(uint8_t *port);
    virtual ~ZxBeeper();

    constexpr static int m_audioBuf_ms = 50;

    void update(int dt_ns);

    void requestInterruption();

protected:
    void run() override;

private:
    uint8_t *m_port = nullptr;
    QAudioOutput *audioOutput = nullptr;
    QIODevice *m_device = nullptr;
    QWaitCondition m_wait;

    int m_sampleFreq = 70000;
    int m_bufferSize = m_sampleFreq * m_audioBuf_ms / 1000;
    int16_t *m_buffer1 = nullptr;
    int16_t *m_buffer2 = nullptr;
    int16_t *m_buffer = nullptr;
    uint m_time_ns = 0;
};

#endif // ZXBEEPER_H
