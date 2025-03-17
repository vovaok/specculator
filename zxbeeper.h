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

    constexpr static int sampleFreq = 44100;
    constexpr static int m_audioBuf_ms = 20;

    void update(int dt_ns);

    void requestInterruption();

protected:
    void run() override;

private:
    uint8_t *m_port = nullptr;
    QAudioOutput *audioOutput = nullptr;
    QIODevice *m_device = nullptr;
    QWaitCondition m_wait;

    constexpr static int m_bufferSize = sampleFreq * m_audioBuf_ms / 1000;
    int16_t m_buffer1[m_bufferSize] {0};
    int16_t m_buffer2[m_bufferSize] {0};
    int16_t *m_buffer = m_buffer1;
    uint m_time_ns = 0;
};

#endif // ZXBEEPER_H
