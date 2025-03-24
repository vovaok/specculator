#ifndef ZXTAPE_H
#define ZXTAPE_H

#include <stdint.h>
#include <QtCore>

class ZxTape
{
public:
    ZxTape();

    void bindPlayPort(uint8_t *port) {m_port = port;}
    void bindRecPort(uint8_t *port) {m_recPort = port;}

    void openTap(QString filename);

    void play();
    bool isPlaying() const {return m_playing;}

    void rec();
    bool isRecording() const {return m_recording;}

    void stop();
    bool isStopped() const {return !m_playing && !m_recording;}

    void update(int dt_ns);

    bool isChanged();

private:
    friend class TapeWidget;

    enum ZxTapeState
    {
        Idle = 0,
        PilotTone = 1,
        StartBit = 2,
        DataBits = 3,
//        Pause
    } m_state = Idle;

    QString m_filename;
    QByteArray m_buffer;
    uint8_t *m_curBlock = nullptr;
    uint8_t *m_ptr = nullptr;
    uint16_t m_len = 0;
    uint8_t m_bit = 0;
    int m_period = 0;
    int m_time_ns = 0;
    int m_pilotCount = 0;

    uint8_t *m_port = nullptr;
    uint8_t *m_recPort = nullptr;
    bool m_playing = false;
    bool m_recording = false;
    bool m_oldLevel;
    int m_blockOffset = 0;
    bool m_changed = false;

    void nextBlock();
    void nextBit();

    void recBit();
    void endBlock();
    void saveTap();

    uint8_t *begin();
    uint8_t *end();
    uint16_t readLen();

    int curOffset();
};

#endif // ZXTAPE_H
