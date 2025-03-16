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
    m_filename = filename;
    m_buffer.clear();
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
}

void ZxTape::play()
{
    if (m_port && !m_buffer.isEmpty())
    {
        m_playing = true;
        m_recording = false;
        m_state = Idle;
        if (!m_ptr)
            nextBlock();
    }
}

void ZxTape::rec()
{
    if (m_recPort)
    {
//        saveTap();
//        m_buffer.clear();
        m_recording = true;
        m_playing = false;
        m_state = Idle;
        m_blockOffset = 0;
        m_len = 0;
        m_bit = 0;
        m_time_ns = 0;
        m_period = 0;
    }
}

void ZxTape::update(int dt_ns)
{
    if (!m_port)
        return;

    if (m_playing)
    {
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
    else if (m_recording)
    {
        m_time_ns += dt_ns;
        bool level = *m_recPort & 0x08;
        if (level && !m_oldLevel)
        {
            m_period = m_time_ns;
            m_time_ns = 0;
            recBit();
        }
        else if (m_state == DataBits && m_time_ns > ZX_PILOT_TONE_us * 2000)
        {
            endBlock();
        }
        m_oldLevel = level;
    }
}

void ZxTape::nextBlock()
{
    if (m_buffer.isEmpty())
        return;

    m_end = reinterpret_cast<uint8_t *>(m_buffer.data()) + m_buffer.size();
    if (!m_ptr)
        m_ptr = reinterpret_cast<uint8_t *>(m_buffer.data());
    m_len = *reinterpret_cast<uint16_t *>(m_ptr);
    m_ptr += 2;
    if (m_ptr >= m_end || m_ptr + m_len > m_end)
    {
        m_ptr = m_end = nullptr;
        m_len = 0;
        m_playing = false;
        qDebug() << "Stop the tape";
//        qDebug() << "[ZxTape] An error has occured when opening the tape";
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
        nextBlock();
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
        if (!m_len)
        {
            m_state = Idle;
            break;
        }
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
        break;
    }

}

void ZxTape::recBit()
{
    bool bitValue;
    if (m_period > ZX_PILOT_TONE_us * 900)
    {
        if (m_state == Idle)
        {
            m_state = PilotTone;
//            qDebug() << "pilot";
        }
        else if (m_state == DataBits)
            endBlock();
    }
    else if (m_state == PilotTone)
    {
        if (m_period > ZX_START_BIT_us * 900)
        {
            m_state = StartBit;
            m_blockOffset = m_buffer.size();
            // reserve the word for block length
            m_buffer.append('\0');
            m_buffer.append('\0');
//            qDebug() << "start bit";
            m_len = 0;
            m_state = DataBits;
        }
    }
    else if (m_state == DataBits)
    {
        if (!m_bit)
        {
            m_buffer.append('\0');
            m_ptr = reinterpret_cast<uint8_t *>(m_buffer.data()) + m_buffer.size() - 1;
        }
        bitValue = (m_period > ZX_BIT_1_us * 750);
        if (bitValue)
            *m_ptr |= (1 << (7 - m_bit));
        if (++m_bit >= 8)
        {
            m_bit = 0;
            m_len++;
        }
    }
}

void ZxTape::endBlock()
{
    if (m_state == DataBits)
    {
        *reinterpret_cast<uint16_t *>(m_buffer.data() + m_blockOffset) = m_len;
        m_state = Idle;
        saveTap();
    }
}

void ZxTape::saveTap()
{
    QFile f(m_filename);
    if (f.open(QIODevice::WriteOnly))
    {
        qDebug() << "Block saved, len =" << m_len;
        f.write(m_buffer);
        f.close();
    }
    else
    {
        qDebug() << "{ZxTape] Error: Can't open TAP file!";
    }
}

