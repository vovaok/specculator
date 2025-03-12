#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include <QDebug>
#include "z80.h"

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
    ZxScreen(char *data)
    {
        m_data = reinterpret_cast<uint8_t*>(data);
        m_attributes = reinterpret_cast<ZxScreenAttr*>(m_data + 6144);
    }

    constexpr static uint32_t zxColor(int col, int br)
    {
        switch (col)
        {
        case 0: return br? 0xff000000: 0xff000000;
        case 1: return br? 0xff0000ff: 0xff0000c0;
        case 2: return br? 0xffff0000: 0xffc00000;
        case 3: return br? 0xffff00ff: 0xffc000c0;
        case 4: return br? 0xff00ff00: 0xff00c000;
        case 5: return br? 0xff00ffff: 0xff00c0c0;
        case 6: return br? 0xffffff00: 0xffc0c000;
        case 7: return br? 0xffffffff: 0xffc0c0c0;
        }
    }

    uint32_t getPixel(int x, int y)
    {
        int bt = 32*(8*(y%8)+(y%64)/8+y/64*64)+x/8;
        int bit = 7-x%8;
        int abt = x/8+32*(y/8);
        uint8_t byte = m_data[bt];
        ZxScreenAttr attr = m_attributes[abt];
        //        int pixel = (byte % int(pow(2, (bit+1)))) / int(pow(2, bit));
        bool pixel = byte & (1 << bit);

        uint32_t col;
        if (pixel)
            col = zxColor(attr.ink, attr.bright);
        else
            col = zxColor(attr.paper, attr.bright);
//        col |= 0xff000000;
        //        col += 0xff000000 * attr.flash;

        return col;
    }

    QImage toImage()
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

    //    void setPixel(int x, int y, uint8_t attr);
    //    void resetPixel(int x, int y, uint8_t attr);

private:
    //    uint8_t data[6912];
    uint8_t *m_data = nullptr;
    ZxScreenAttr *m_attributes = nullptr;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    Z80 *cpu = nullptr;
    uint8_t mem[0x10000];
    ZxScreen *scr;

    bool m_running = false;

    QLabel *scrWidget;
    QMap<QString, QLineEdit *> regEdits;
    QLineEdit *bkptEdit;
    QLabel *status;
    QElapsedTimer etimer;
    double perf;

    uint8_t port254;

    QImage frm;
    uint32_t *videoptr;

    void reset();
    void step();
    void run();

    void updateRegs();
    void updateScreen();

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;

    QMap<int, bool> m_keysPressed;
};

#endif // MAINWINDOW_H
