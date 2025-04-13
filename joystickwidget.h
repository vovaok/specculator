#ifndef JOYSTICKWIDGET_H
#define JOYSTICKWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QTouchEvent>
#include "zxjoystick.h"

class JoystickWidget : public QWidget
{
    Q_OBJECT
public:
    explicit JoystickWidget(QWidget *parent = nullptr);
    void bindJoystick(ZxJoystick *joystick) {m_joy = joystick;}
    void setJoystickType(ZxJoystick::Type type);

protected:
    virtual bool event(QEvent *event) override;
    virtual void resizeEvent(QResizeEvent *e) override;
    void touchEvent(QTouchEvent *e);
//    virtual void paintEvent(QPaintEvent *event) override;

signals:

private:
    ZxJoystick *m_joy = nullptr;
//    QVector<QPushButton *> m_padButtons;
    QPushButton *m_btnL;
    QPushButton *m_btnR;
    QPushButton *m_btnU;
    QPushButton *m_btnD;
    QPushButton *m_btnFire;
    QWidget *m_pad;
    int m_padTouchId = -1;
    int m_fireTouchId = -1;

    void setButtonState(QPushButton *btn, bool state);
};

#endif // JOYSTICKWIDGET_H
