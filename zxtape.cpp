#include "zxtape.h"

#define ZX_PILOT_TONE_us    (620*2)
#define ZX_START_BIT_us     (186*2)
#define ZX_BIT_0_us         (250*2)
#define ZX_BIT_1_us         (500*2)

ZxTape::ZxTape()
{

}

void ZxTape::openTap(QString filename)
{
    QFile f(filename);
    if (f.open(QIODevice::ReadOnly))
    {
        m_buffer = f.readAll();
        f.close();
    }
    else
    {
        qDebug() << "{ZxTape] Error: Can't open TAP file!";
    }

    if (m_buffer.size() > 2)
    {
//        m_ptr = reinterpret_cast<uint8_t *>(m_buffer.data());
        m_end = reinterpret_cast<uint8_t *>(m_buffer.data()) + m_buffer.size();

    }
}

void ZxTape::play()
{
    if (m_port && !m_buffer.isEmpty())
    {
        m_ptr = reinterpret_cast<uint8_t *>(m_buffer.data());
        m_playing = true;
        m_state = Idle;
        nextBit();
    }
}

void ZxTape::update(int dt_ns)
{
    if (!m_port)
        return;

    if (!m_playing)
        return;

    if (!m_period)
        return;

    m_time_ns += dt_ns;
    if (m_time_ns > m_period)
    {
        m_time_ns = m_time_ns % m_period;
        nextBit();
    }

    if (m_time_ns < m_period / 2)
        *m_port |= (1 << 6);
    else
        *m_port &= ~(1 << 6);
}

void ZxTape::nextBlock()
{
    m_len = *reinterpret_cast<uint16_t *>(m_ptr);
    m_ptr += 2;
    if (m_len + 2 > m_buffer.size())
    {
        m_ptr = m_end = nullptr;
        m_len = 0;
        qDebug() << "[ZxTape] An error has occured when opening the tape";
    }
    else
    {
        m_state = PilotTone;
        m_pilotCount = 2000;
        nextBit();
    }
}

void ZxTape::nextBit()
{
    switch (m_state)
    {
    case Idle:
//        m_playing = false;
        if (m_ptr < m_end)
            nextBlock();
        else
            m_playing = false;
        break;

    case PilotTone:
        m_period = ZX_PILOT_TONE_us * 1000;
        --m_pilotCount;
        if (m_pilotCount <= 0)
            m_state = StartBit;
        break;

    case StartBit:
        m_period = ZX_START_BIT_us * 1000;
        m_state = DataBits;
        break;

    case DataBits:
        if (*m_ptr & (1 << (7 - m_bit)))
            m_period = ZX_BIT_1_us * 1000;
        else
            m_period = ZX_BIT_0_us * 1000;
        m_bit++;
        if (m_bit >= 8)
        {
            m_ptr++;
            --m_len;
            m_bit = 0;
        }
        if (!m_len)
            m_state = Idle;
        break;
    }

}

