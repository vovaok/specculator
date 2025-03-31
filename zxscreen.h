#ifndef ZXSCREEN_H
#define ZXSCREEN_H

#include <QImage>
#include <QDebug>

struct ZxScreenAttr
{
    uint8_t ink: 3;
    uint8_t paper: 3;
    uint8_t bright: 1;
    uint8_t flash: 1;
};

class ZxScreen
{
public:
    ZxScreen(char *data, int width=320, int height=240);

    void bindBorderPort(uint8_t *port) {m_port = port;}

    void clear();

    void update(qint64 T);

    static uint32_t zxColor(int col, int br);

    uint32_t getPixel(int x, int y) const;

    QImage screenshot() const;

    QImage frame() const {return m_image;}

    QColor borderColor() const {return QColor::fromRgb(m_borderColor);}

    //    void setPixel(int x, int y, uint8_t attr);
    //    void resetPixel(int x, int y, uint8_t attr);

    constexpr static int cpuFreq = 3'500'000;
    constexpr static int videoFreq = 7'375'000;
    constexpr static int intFreq = 50;

private:
    QImage m_image;
    uint8_t *m_data = nullptr;
    uint8_t *m_port = nullptr;
    ZxScreenAttr *m_attributes = nullptr;
    static uint32_t m_colorTable[16];
    bool m_flash = false;
    uint32_t m_borderColor = 0;
    int m_w=0, m_h=0;
    int m_srow=0, m_scol=0;
    int m_sx=0, m_sy=0;
    uint32_t *m_bufBegin = nullptr, *m_bufEnd = nullptr;
    uint32_t *m_videoptr = nullptr;
    int m_oldX = 0;

    uint8_t *m_curData = nullptr;
    ZxScreenAttr *m_curAttr = nullptr;
    void setCurRow(int y);
    uint32_t getPixel(int x) const;

    constexpr static int cyclesPerFrame = cpuFreq / 25;
    constexpr static int cyclesPerLine = cyclesPerFrame / 625;
    constexpr static int cyclesHSync = cyclesPerLine * 4 / 64;
    constexpr static int cyclesBackPorch = cyclesPerLine * 8 / 64;
    constexpr static int lineStartT = cyclesHSync + cyclesBackPorch;
};

#endif // ZXSCREEN_H
