#ifndef ZXKEYBOARD_H
#define ZXKEYBOARD_H

#include <QMap>

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
    _DisableShift = 0xFE
};

class ZxKeyboard
{
public:
    ZxKeyboard(uint8_t *port);

    uint8_t readKeys(uint8_t addr);

    //! @brief Obtain current state of the native key
    bool keyState(int keycode);

    //! @brief Must be called from system when native key state changed
    void setKeyState(int keycode, bool state);

private:
    QMap<int, int> m_keysPressed;
    uint8_t *m_keyport;
    QMap<int, QVector<ZxKeyCode>> m_keyMap;

    void update();
};

#endif // ZXKEYBOARD_H
