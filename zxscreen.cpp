#include "zxscreen.h"
#include <QDebug>

uint32_t ZxScreen::m_colorTable[16] =
{
    0xff000000, 0xff0000c0, 0xffc00000, 0xffc000c0, 0xff00c000, 0xff00c0c0, 0xffc0c000, 0xffc0c0c0,
    0xff000000, 0xff0000ff, 0xffff0000, 0xffff00ff, 0xff00ff00, 0xff00ffff, 0xffffff00, 0xffffffff
};

ZxScreen::ZxScreen(char *data, int width, int height)
{
    m_data = reinterpret_cast<uint8_t*>(data);
    m_attributes = reinterpret_cast<ZxScreenAttr*>(m_data + 6144);
    m_image = QImage(width, height, QImage::Format_ARGB32_Premultiplied);
    m_image.fill(Qt::black);
    m_w = width;
    m_h = height;
    m_scol = (400 - width) / 2;
    m_srow = 6 + (304 - height) / 2;
    m_sx = (width - 256) / 2;
    m_sy = (height - 192) / 2;
    m_bufBegin = reinterpret_cast<uint32_t*>(m_image.bits());
    m_bufEnd = m_bufBegin + 320*240;
    m_videoptr = m_bufBegin;
}

void ZxScreen::clear()
{
    m_image.fill(Qt::black);
}

void ZxScreen::update(qint64 T)
{
    m_flash = (T / (cpuFreq / 3)) & 1;
    m_borderColor = zxColor(*m_port & 7, 0);

    // hor line 64us - visual 52us
    // 625 lines per frame interlaced

    int frame_T = T % (cyclesPerFrame);
    int frame_line = frame_T / cyclesPerLine + 1; // [1 ... 625]
    if (frame_line > 312)
        frame_line -= 312; // odd/even lines
    int frm_y = frame_line - m_srow;
    if (frm_y >= 0 && frm_y < m_h)
    {
        int line_T = frame_T % cyclesPerLine;
        int frm_x = (line_T - lineStartT) * videoFreq / cpuFreq - m_scol;

        if (frm_x > m_w)
            frm_x = m_w;
        else if (frm_x < 0)
        {
            if (!m_oldX)
                return;
            frm_x = m_w;
            frm_y--;
        }

        int x = m_oldX - m_sx;
        int y = frm_y - m_sy;
        int cnt = frm_x - m_oldX;
        uint32_t *vptr = m_bufBegin + frm_y * m_w + m_oldX;
//        while (--cnt >= 0)
//            *vptr++ = getPixel(x++, y);
        setCurRow(y);
        while (--cnt >= 0)
            *vptr++ = getPixel(x++);
        m_oldX = frm_x % m_w;

//            for (; m_oldX < frm_x; m_oldX++)
//                m_bufBegin[m_oldX + frm_y * m_w] = getPixel(m_oldX - m_sx, frm_y - m_sy);
//            if (frm_x == m_w)
//                m_oldX = 0;
    }
}

uint32_t ZxScreen::zxColor(int col, int br)
{
    return m_colorTable[(col | (br << 3)) & 0xf];
}

uint32_t ZxScreen::getPixel(int x, int y) const
{
    if (x < 0 || y < 0 || x >= 256 || y >= 192)
        return m_borderColor;

    int bt = ((((y & 7) << 3) + ((y & 63) >> 3) + (y & ~63)) << 5) + (x >> 3);
    int bit = (~x & 7);
    int abt = (x >> 3) + ((y << 2) & ~31);

    uint8_t byte = m_data[bt];
    ZxScreenAttr attr = m_attributes[abt];
    bool pixel = ((byte >> bit) ^ (attr.flash & m_flash)) & 0x01;

    if (pixel)
        return zxColor(attr.ink, attr.bright);
    return zxColor(attr.paper, attr.bright);
}

QImage ZxScreen::screenshot() const
{
    QImage img(256, 192, QImage::Format_ARGB32_Premultiplied);
    for (int y=0; y<192; y++)
        for (int x=0; x<256; x++)
            img.setPixel(x, y, getPixel(x, y));
    return img;
}

void ZxScreen::setCurRow(int y)
{
    if (y < 0 || y >= 192)
    {
        m_curData = nullptr;
        return;
    }

    m_curData = m_data + ((((y & 7) << 3) + ((y & 63) >> 3) + (y & ~63)) << 5);
    m_curAttr = m_attributes + ((y << 2) & ~31);
}

uint32_t ZxScreen::getPixel(int x) const
{
    if (!m_curData || x < 0 || x >= 256)
        return m_borderColor;

    int xoff = x >> 3;
    int bit = (~x & 7);

    uint8_t byte = m_curData[xoff];
    ZxScreenAttr attr = m_curAttr[xoff];

    if (((byte >> bit) ^ (attr.flash & m_flash)) & 0x01)
        return zxColor(attr.ink, attr.bright);
    return zxColor(attr.paper, attr.bright);
}
