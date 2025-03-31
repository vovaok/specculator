#include "zxbeeper.h"
#include <QDebug>

ZxBeeper::ZxBeeper(uint8_t *port) :
    m_port(port)
{
    start();
}

ZxBeeper::~ZxBeeper()
{
    requestInterruption();
}

void ZxBeeper::update(int dt_ns)
{
    if (!m_buffer)
        return;

    m_time_ns += dt_ns;
    int idx = m_time_ns * (m_sampleFreq / 1000) / 1000000;
    if (idx >= m_bufferSize)
    {
        m_buffer = (m_buffer == m_buffer1)? m_buffer2: m_buffer1;
        m_time_ns = m_time_ns % (m_audioBuf_ms * 1000000);
        m_wait.wakeAll();
    }
    else
    {
        m_buffer[idx] = (*m_port & 0x10)? 0x7fff: 0x0000;
    }
}

void ZxBeeper::requestInterruption()
{
    m_wait.wakeAll();
    QThread::requestInterruption();
    wait(500);
}

void ZxBeeper::run()
{
    // audio -------------------------------------------------
    QAudioFormat format;
    format.setSampleRate(m_sampleFreq);
    format.setChannelCount(1);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    QAudioDeviceInfo info = QAudioDeviceInfo::defaultOutputDevice();
    if (!info.isFormatSupported(format))
    {
        qWarning() << "bad format, but pofig";
        format = info.nearestFormat(format);
    }

    // obtain real sample frequency
    m_sampleFreq = format.sampleRate();
    m_bufferSize = m_sampleFreq * m_audioBuf_ms / 1000;

    m_buffer1 = new int16_t[m_bufferSize];
    m_buffer2 = new int16_t[m_bufferSize];
    memset(m_buffer1, 0, m_bufferSize * sizeof(int16_t));
    memset(m_buffer2, 0, m_bufferSize * sizeof(int16_t));
    m_buffer = m_buffer1;

    const int bufsize = m_bufferSize * sizeof(int16_t);
    audioOutput = new QAudioOutput(format);
    audioOutput->setVolume(0.5);
//    audioOutput->setBufferSize(bufsize * 8);
    m_device = audioOutput->start();

    while (!isInterruptionRequested())
    {
        QMutex moo;
        moo.lock();
        m_wait.wait(&moo);
        moo.unlock();

        if (isInterruptionRequested())
            break;

        int16_t *buf = (m_buffer == m_buffer1)? m_buffer2: m_buffer1;

        if (audioOutput->bytesFree() < bufsize)
            while (audioOutput->bytesFree() < bufsize * 2)
            {
                m_device->write(QByteArray());
//                qDebug() << "sleep";
                msleep(m_audioBuf_ms);
            }

//        int cnt = audioOutput->bytesFree();
        m_device->write(reinterpret_cast<const char *>(buf), bufsize);
//        qDebug() << cnt << "-" << bufsize << "=" << audioOutput->bytesFree();
    }

    audioOutput->stop();
    delete audioOutput;

    m_buffer = nullptr;
    delete [] m_buffer1;
    delete [] m_buffer2;
}
