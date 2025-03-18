#ifndef ZXSCREEN_H
#define ZXSCREEN_H

#include <QImage>

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
    ZxScreen(char *data);

    static uint32_t zxColor(int col, int br);

    uint32_t getPixel(int x, int y);

    QImage toImage();

    //! @brief Input flash signal (~1/3 s)
    bool flash = false;

    //    void setPixel(int x, int y, uint8_t attr);
    //    void resetPixel(int x, int y, uint8_t attr);

private:
    uint8_t *m_data = nullptr;
    ZxScreenAttr *m_attributes = nullptr;
    static uint32_t m_colorTable[16];
};

#endif // ZXSCREEN_H
