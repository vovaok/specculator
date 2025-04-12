#ifndef JOYSTICKWIDGET_H
#define JOYSTICKWIDGET_H

#include <QWidget>
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
    virtual void paintEvent(QPaintEvent *event) override;

signals:

private:
    ZxJoystick *m_joy = nullptr;
    QPoint m_padPos;
    QPoint m_firePos;
};

#endif // JOYSTICKWIDGET_H
