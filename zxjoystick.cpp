#include "zxjoystick.h"

ZxJoystick::ZxJoystick()
{
}

ZxJoystick::~ZxJoystick()
{
    for (QGamepad *gp: m_gamepads)
    {
        gp->disconnect();
        gp->deleteLater();
    }
}

uint8_t ZxJoystick::readKempston()
{
    if (m_type == Kempston)
        return m_state.byte;
    return 0xFF;
}

uint8_t ZxJoystick::readKeys(uint8_t addr)
{
    uint8_t r = 0;
    if (m_type == Cursor)
    {
        if (addr == 0xF7)
        {
            if (m_state.left)
                r |= 0x10;
        }
        else if (addr == 0xEF)
        {
            if (m_state.fire)
                r |= 0x01;
            if (m_state.right)
                r |= 0x04;
            if (m_state.up)
                r |= 0x08;
            if (m_state.down)
                r |= 0x10;
        }
    }
    else if (m_type == InterfaceII && addr == 0xEF)
    {
        if (m_state.fire)
            r |= 0x01;
        if (m_state.up)
            r |= 0x02;
        if (m_state.down)
            r |= 0x04;
        if (m_state.right)
            r |= 0x08;
        if (m_state.left)
            r |= 0x10;
    }
    return ~r;
}

void ZxJoystick::connectGamepad(QGamepad *gp)
{
    int id = gp->deviceId();
    if (m_gamepads.contains(id))
    {
        m_gamepads[id]->disconnect();
        m_gamepads[id]->deleteLater();
    }
    m_gamepads[id] = gp;

    QObject::connect(gp, &QGamepad::axisLeftXChanged, [this](double value){
        m_state.right = (value > 0.3);
        m_state.left = (value < -0.3);
    });
    QObject::connect(gp, &QGamepad::axisLeftYChanged, [this](double value){
        m_state.down = (value > 0.3);
        m_state.up = (value < -0.3);
    });
    QObject::connect(gp, &QGamepad::buttonLeftChanged, [this](bool value){m_state.left = value;});
    QObject::connect(gp, &QGamepad::buttonRightChanged, [this](bool value){m_state.right = value;});
    QObject::connect(gp, &QGamepad::buttonUpChanged, [this](bool value){m_state.up = value;});
    QObject::connect(gp, &QGamepad::buttonDownChanged, [this](bool value){m_state.down = value;});
    QObject::connect(gp, &QGamepad::buttonAChanged, [this, gp](bool){
        m_state.fire = gp->buttonA() | gp->buttonB();
    });
    QObject::connect(gp, &QGamepad::buttonBChanged, [this, gp](bool){
        m_state.fire = gp->buttonA() | gp->buttonB();
    });

}
