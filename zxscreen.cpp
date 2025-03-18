#include "zxscreen.h"

uint32_t ZxScreen::m_colorTable[16] =
{
    0xff000000, 0xff0000c0, 0xffc00000, 0xffc000c0, 0xff00c000, 0xff00c0c0, 0xffc0c000, 0xffc0c0c0,
    0xff000000, 0xff0000ff, 0xffff0000, 0xffff00ff, 0xff00ff00, 0xff00ffff, 0xffffff00, 0xffffffff
};

ZxScreen::ZxScreen(char *data)
{
    m_data = reinterpret_cast<uint8_t*>(data);
    m_attributes = reinterpret_cast<ZxScreenAttr*>(m_data + 6144);
}

uint32_t ZxScreen::zxColor(int col, int br)
{
    return m_colorTable[(col | (br << 3)) & 0xf];
}

uint32_t ZxScreen::getPixel(int x, int y)
{
//    int bt = 32*(8*(y%8)+(y%64)/8+y/64*64)+x/8;
//    int bit = 7-x%8;
//    int abt = x/8+32*(y/8);
    int bt = ((((y & 7) << 3) + ((y & 63) >> 3) + (y & ~63)) << 5) + (x >> 3);
    int bit = (x & 7);
    int abt = (x >> 3) + ((y << 2) & ~31);
    uint8_t byte = m_data[bt];
    ZxScreenAttr attr = m_attributes[abt];
    bool pixel = byte & (0x80 >> bit);

    uint8_t ink = attr.ink;
    uint8_t paper = attr.paper;
    uint32_t col;
    if (attr.flash && flash)
        std::swap(ink, paper);
    if (pixel)
        col = zxColor(ink, attr.bright);
    else
        col = zxColor(paper, attr.bright);

    return col;
}

QImage ZxScreen::toImage()
{
    QImage img(256, 192, QImage::Format_ARGB32_Premultiplied);
    for (int y=0; y<192; y++)
    {
        for (int x=0; x<256; x++)
        {
            img.setPixel(x, y, getPixel(x, y));
        }
    }
    return img;
}
