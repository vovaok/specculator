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

        uint8_t ink = attr.ink;
        uint8_t paper = attr.paper;
        uint32_t col;
        if (attr.flash && flash)
            std::swap(ink, paper);
        if (pixel)
            col = zxColor(ink, attr.bright);
        else
            col = zxColor(paper, attr.bright);
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

    bool flash = false;

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
//    uint8_t mem[0x10000];
    ZxScreen *scr;

    bool m_running = false;

    QLabel *scrWidget;
    QMap<QString, QLineEdit *> regEdits;
    QLineEdit *bkptEdit;
    QLabel *status;
    QElapsedTimer etimer;
    double perf;

    constexpr static int cpuFreq = 3500000;
    constexpr static int videoFreq = 7375000;
    constexpr static int cyclesPerFrame = cpuFreq / 25;
    constexpr static int cyclesPerLine = cyclesPerFrame / 625;
    constexpr static int cyclesHSync = cyclesPerLine * 4 / 64;
    constexpr static int cyclesBackPorch = cyclesPerLine * 8 / 64;
    constexpr static int lineStartT = cyclesHSync + cyclesBackPorch;

    union
    {
        uint8_t port254;
        struct
        {
            uint8_t borderR: 1;
            uint8_t borderG: 1;
            uint8_t borderB: 1;
            uint8_t tape: 1;
            uint8_t beep: 1;
        };
    };

    QImage frm;
    uint32_t *videoptr;

    void reset();
    void step();
    void run();

    void updateRegs();
    void updateScreen();

    void doStep();

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;

    enum ZxKeyCode
    {
        Key_CS  = 0x01,
        Key_Z   = 0x02,
        Key_X   = 0x04,
        Key_C   = 0x08,
        Key_V   = 0x10,
        Key_A   = 0x21,
        Key_S   = 0x22,
        Key_D   = 0x24,
        Key_F   = 0x28,
        Key_G   = 0x30,
        Key_Q   = 0x41,
        Key_W   = 0x42,
        Key_E   = 0x44,
        Key_R   = 0x48,
        Key_T   = 0x50,
        Key_1   = 0x61,
        Key_2   = 0x62,
        Key_3   = 0x64,
        Key_4   = 0x68,
        Key_5   = 0x70,
        Key_0   = 0x81,
        Key_9   = 0x82,
        Key_8   = 0x84,
        Key_7   = 0x88,
        Key_6   = 0x90,
        Key_P   = 0xA1,
        Key_O   = 0xA2,
        Key_I   = 0xA4,
        Key_U   = 0xA8,
        Key_Y   = 0xB0,
        Key_Enter = 0xC1,
        Key_L   = 0xC2,
        Key_K   = 0xC4,
        Key_J   = 0xC8,
        Key_H   = 0xD0,
        Key_Space = 0xE1,
        Key_SS  = 0xE2,
        Key_M   = 0xE4,
        Key_N   = 0xE8,
        Key_B   = 0xF0,
    };

    QMap<int, int> m_keysPressed;
//    uint8_t m_keyport[8] {0};
    QMap<int, QVector<ZxKeyCode>> m_keyMap;

    void updateKeyboard();
    uint8_t readKeys(uint8_t addr);
};

#endif // MAINWINDOW_H
