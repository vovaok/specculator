#ifndef KEYBOARDWIDGET_H
#define KEYBOARDWIDGET_H

#include <QWidget>
#include <QEvent>
#include <QTouchEvent>
#include <QPushButton>
#include "zxkeyboard.h"

class KeyboardWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KeyboardWidget(QWidget *parent = nullptr);
    void bindKeyboard(ZxKeyboard *keyb) {m_keyb = keyb;}

    void updateState();

signals:

protected:
    virtual bool event(QEvent *event) override;
    virtual void resizeEvent(QResizeEvent *e) override;
    void touchEvent(QTouchEvent *e);

private:
    ZxKeyboard *m_keyb = nullptr;
    static uint8_t m_keymap[40];
    QStringList m_keys;

    QMap<int, QPushButton *> m_touchedButtons;

    void updateLayout(QSize sz);
};

#endif // KEYBOARDWIDGET_H
