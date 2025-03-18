#include "zxkeyboard.h"

ZxKeyboard::ZxKeyboard(uint8_t *port) :
    m_keyport(port)
{
    m_keyMap['1']   = {Key_1};
    m_keyMap['2']   = {Key_2};
    m_keyMap['3']   = {Key_3};
    m_keyMap['4']   = {Key_4};
    m_keyMap['5']   = {Key_5};
    m_keyMap['6']   = {Key_6};
    m_keyMap['7']   = {Key_7};
    m_keyMap['8']   = {Key_8};
    m_keyMap['9']   = {Key_9};
    m_keyMap['0']   = {Key_0};
    m_keyMap['Q']   = {Key_Q};
    m_keyMap['W']   = {Key_W};
    m_keyMap['E']   = {Key_E};
    m_keyMap['R']   = {Key_R};
    m_keyMap['T']   = {Key_T};
    m_keyMap['Y']   = {Key_Y};
    m_keyMap['U']   = {Key_U};
    m_keyMap['I']   = {Key_I};
    m_keyMap['O']   = {Key_O};
    m_keyMap['P']   = {Key_P};
    m_keyMap['A']   = {Key_A};
    m_keyMap['S']   = {Key_S};
    m_keyMap['D']   = {Key_D};
    m_keyMap['F']   = {Key_F};
    m_keyMap['G']   = {Key_G};
    m_keyMap['H']   = {Key_H};
    m_keyMap['J']   = {Key_J};
    m_keyMap['K']   = {Key_K};
    m_keyMap['L']   = {Key_L};
    m_keyMap[13]    = {Key_Enter};
    m_keyMap[16]    = {Key_CS};
    m_keyMap['Z']   = {Key_Z};
    m_keyMap['X']   = {Key_X};
    m_keyMap['C']   = {Key_C};
    m_keyMap['V']   = {Key_V};
    m_keyMap['B']   = {Key_B};
    m_keyMap['N']   = {Key_N};
    m_keyMap['M']   = {Key_M};
    m_keyMap[17]    = {Key_SS};
    m_keyMap[' ']   = {Key_Space};

    m_keyMap[8]     = {Key_CS, Key_0}; // backspace
    m_keyMap[37]    = {Key_CS, Key_5}; // arrow left
    m_keyMap[38]    = {Key_CS, Key_7}; // arrow up
    m_keyMap[39]    = {Key_CS, Key_8}; // arrow right
    m_keyMap[40]    = {Key_CS, Key_6}; // arrow down
    m_keyMap[19]    = {Key_CS, Key_Space}; // break
    m_keyMap[189]   = {Key_SS, Key_J}; // minus
    m_keyMap[187]   = {Key_SS, Key_L}; // equal
    m_keyMap[219]   = {Key_SS, Key_8}; // left bracket (parenthesis)
    m_keyMap[221]   = {Key_SS, Key_9}; // right bracket (parenthesis)
    m_keyMap[186]   = {Key_SS, Key_O}; // semicolon
    m_keyMap[222]   = {Key_SS, Key_P}; // quote
    m_keyMap[220]   = {Key_SS, Key_CS}; // backslash (toggle extended cursor)
    m_keyMap[226]   = {Key_SS, Key_CS}; // backslash (toggle extended cursor)
    m_keyMap[188]   = {Key_SS, Key_N}; // comma
    m_keyMap[190]   = {Key_SS, Key_M}; // dot
    m_keyMap[191]   = {Key_SS, Key_V}; // slash
}

uint8_t ZxKeyboard::readKeys(uint8_t addr)
{
    for (int idx=0; idx<8; idx++)
        if (!(addr & (1 << idx)))
            return ~m_keyport[idx];
    return 0xFF;
}

bool ZxKeyboard::keyState(int keycode)
{
    return m_keysPressed.contains(keycode);
}

void ZxKeyboard::setKeyState(int keycode, bool state)
{
    if (state)
        m_keysPressed[keycode] = keycode;
    else
        m_keysPressed.remove(keycode);
    update();
}

void ZxKeyboard::update()
{
    for (int i=0; i<8; i++)
        m_keyport[i] = 0;

    for (int key: m_keysPressed)
    {
        for (ZxKeyCode zxkey: m_keyMap[key])
        {
            int idx = zxkey >> 5;
            uint8_t mask = zxkey & 0x1F;
            m_keyport[idx] |= mask;
        }
    }
}
