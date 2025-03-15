#ifndef ZXTAPE_H
#define ZXTAPE_H

#include <stdint.h>
#include <QtCore>

class ZxTape
{
public:
    ZxTape();

    void bindPort(uint8_t *port) {m_port = port;}

    void openTap(QString filename);

    void play();
    bool isPlaying() const {return m_playing;}

    void update(int dt_ns);

private:
    struct TapHeader
    {
        uint8_t blockType; // 0x00 - header, 0xff - data
        uint8_t fileType; // 0x00 - Program, 0x01 - Numeric Array, 0x02 - Symbol Array, 0x03 - Bytes
        char name[10];
        uint16_t dataLength;
        uint16_t param; // autorun line or start address
        uint16_t programLength;
        //    uint8_t checksum;
    };

    enum ZxTapeState
    {
        Idle = 0,
        PilotTone = 1,
        StartBit = 2,
        DataBits = 3,
        Pause
    } m_state = Idle;

    QByteArray m_buffer;
    uint8_t *m_ptr = nullptr;
    uint8_t *m_end = nullptr;
    uint16_t m_len = 0;
    uint8_t m_bit = 0;
    int m_period = 0;
    int m_time_ns = 0;
    int m_pilotCount = 0;

    uint8_t *m_port = nullptr;
    bool m_playing = false;

    void nextBlock();
    void nextBit();
};

#endif // ZXTAPE_H
