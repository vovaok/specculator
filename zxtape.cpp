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
        m_changed = true;
        f.close();
    }
    else
    {
        qDebug() << "{ZxTape] Error: Can't open TAP file!";
    }
}

void ZxTape::play()
{
    if (isPlaying())
        return;

    if (m_port && !m_buffer.isEmpty())
    {
        m_playing = true;
        m_recording = false;
        m_state = Idle;
        m_len = 0;
        m_bit = 0;
        m_time_ns = 0;
        m_period = 0;

        nextBlock();
    }
}

void ZxTape::rec()
{
    if (isRecording())
        return;

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

void ZxTape::stop()
{
    m_ptr = m_curBlock;
    m_recording = false;
    m_playing = false;
    m_state = Idle;
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
//            qDebug() << m_period;
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

bool ZxTape::isChanged()
{
    bool r = m_changed;
    m_changed = false;
    return r;
}

void ZxTape::nextBlock()
{
    if (m_buffer.isEmpty())
        return;

    uint8_t *end = this->end();
    if (!m_ptr)
        m_ptr = begin();
    m_curBlock = m_ptr;
    m_len = readLen();
    if (m_ptr >= end || m_ptr + m_len > end)
    {
        m_ptr = nullptr;
        m_len = 0;
        m_playing = false;
//        qDebug() << "Stop the tape";
    }
    else
    {
        m_state = PilotTone;
        if (*m_ptr == 0x00) // if this is header
            m_pilotCount = 3000;
        else
            m_pilotCount = 1500;
        nextBit();
    }
    m_changed = true;
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
//        qDebug() << "data bit";
        if (!m_bit)
        {
            m_buffer.append('\0');
            m_ptr = end() - 1;
        }
        bitValue = (m_period > ZX_BIT_1_us * 750);
        if (bitValue)
            *m_ptr |= (1 << (7 - m_bit));
        if (++m_bit >= 8)
        {
            m_bit = 0;
            m_len++;
//            qDebug() << "next" << m_len;
        }
    }
}

void ZxTape::endBlock()
{
    if (m_state == DataBits)
    {
//        qDebug() << "len" << m_len << "bit" << m_bit;
        if (m_bit > 0) // odd bits
        {
            m_bit = 0;
            m_buffer.chop(1);
        }
        *reinterpret_cast<uint16_t *>(m_buffer.data() + m_blockOffset) = m_len;
        m_changed = true;
        m_state = Idle;
        saveTap();
//        qDebug() << "Block saved, len =" << m_len;
    }
}

void ZxTape::saveTap()
{
    QFile f(m_filename);
    if (f.open(QIODevice::WriteOnly))
    {
        f.write(m_buffer);
        f.close();
    }
    else
    {
        qDebug() << "{ZxTape] Error: Can't write TAP file!";
    }
}

uint8_t *ZxTape::begin()
{
    return reinterpret_cast<uint8_t *>(m_buffer.data());
}

uint8_t *ZxTape::end()
{
    return reinterpret_cast<uint8_t *>(m_buffer.data() + m_buffer.size());
}

uint16_t ZxTape::readLen()
{
    uint16_t len = *reinterpret_cast<uint16_t *>(m_ptr);
    m_ptr += 2;
    return len;
}

int ZxTape::curOffset()
{
    return m_ptr - begin();
}

