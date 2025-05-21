#ifndef ZXJOYSTICK_H
#define ZXJOYSTICK_H

#include <QGamepad>

class ZxJoystick
{
public:
    ZxJoystick();
    virtual ~ZxJoystick();

    enum Type
    {
        NoJoystick = 0,
        Kempston,
        Sinclair,
        Cursor,
        InterfaceII
    };

    void setType(Type type) {m_type = type;}
    Type type() const {return m_type;}

    uint8_t readKempston();
    uint8_t readKeys(uint8_t addr);

    QString status;
    void connectGamepad(QGamepad *gp);

private:
    friend class JoystickWidget;
    QMap<int, QGamepad *> m_gamepads;
    Type m_type = Kempston;
    union
    {
        uint8_t byte = 0xE0;
        struct
        {
            uint8_t right: 1;
            uint8_t left: 1;
            uint8_t down: 1;
            uint8_t up: 1;
            uint8_t fire: 1;
        };
    } m_state;
};

#endif // ZXJOYSTICK_H
