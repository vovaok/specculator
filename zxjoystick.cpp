#include "zxjoystick.h"
#include <QDebug>

ZxJoystick::ZxJoystick()
{
    QGamepadManager *gpm = QGamepadManager::instance();
    QObject::connect(gpm, &QGamepadManager::connectedGamepadsChanged, [=]()
    {
//        qDebug() << "Physical gamepads:" << gpm->connectedGamepads();

        m_gamepad = new QGamepad(gpm->connectedGamepads().first());
        qDebug() << m_gamepad->isConnected();

        QObject::connect(m_gamepad, &QGamepad::axisLeftXChanged, [this](double value){
            m_state.right = (value > 0.3);
            m_state.left = (value < -0.3);
        });
        QObject::connect(m_gamepad, &QGamepad::axisLeftYChanged, [this](double value){
            m_state.down = (value > 0.3);
            m_state.up = (value < -0.3);
        });
        QObject::connect(m_gamepad, &QGamepad::buttonLeftChanged, [this](bool value){m_state.left = value;});
        QObject::connect(m_gamepad, &QGamepad::buttonRightChanged, [this](bool value){m_state.right = value;});
        QObject::connect(m_gamepad, &QGamepad::buttonUpChanged, [this](bool value){m_state.up = value;});
        QObject::connect(m_gamepad, &QGamepad::buttonDownChanged, [this](bool value){m_state.down = value;});
        QObject::connect(m_gamepad, &QGamepad::buttonAChanged, [this](bool){
            m_state.fire = m_gamepad->buttonA() | m_gamepad->buttonB();
        });
        QObject::connect(m_gamepad, &QGamepad::buttonBChanged, [this](bool){
            m_state.fire = m_gamepad->buttonA() | m_gamepad->buttonB();
        });


    });
}

ZxJoystick::~ZxJoystick()
{
    if (m_gamepad)
    {
        m_gamepad->disconnect();
        delete m_gamepad;
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
