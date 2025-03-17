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
    m_time_ns += dt_ns;
    uint idx = m_time_ns * (sampleFreq / 1000) / 1000000;
    if (idx >= m_bufferSize)
    {
        m_buffer = (m_buffer == m_buffer1)? m_buffer2: m_buffer1;
        m_time_ns = m_time_ns % (m_audioBuf_ms * 1000000);
//        if (m_buffer == m_buffer1)
//            qDebug() << "sw to 1";
//        else
//            qDebug() << "sw to 2";
        m_wait.wakeAll();
    }
    else
    {
        m_buffer[idx] = (*m_port & 0x10)? 0x7fff: 0x0000;
    }
}

void ZxBeeper::requestInterruption()
{
    QThread::requestInterruption();
    m_wait.wakeAll();
}

void ZxBeeper::run()
{
    // audio -------------------------------------------------
    QAudioFormat format;
    format.setSampleRate(sampleFreq);
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

    audioOutput = new QAudioOutput(format);
    audioOutput->setVolume(0.1);
    audioOutput->setBufferSize(sampleFreq * 2);
    m_device = audioOutput->start();

    while (!isInterruptionRequested())
    {
        QMutex moo;
        moo.lock();
        m_wait.wait(&moo);
        moo.unlock();

        int16_t *buf = (m_buffer == m_buffer1)? m_buffer2: m_buffer1;
//        if (buf == m_buffer1)
//            qDebug() << "read from 1";
//        else
//            qDebug() << "read from 2";
        m_device->write(reinterpret_cast<const char *>(buf), m_bufferSize * sizeof(int16_t));

//        int cnt = (audioOutput->bufferSize() - audioOutput->bytesFree()) / 2;
//        msleep(cnt * 500 / sampleFreq);
    }

    audioOutput->stop();
    delete audioOutput;
}
