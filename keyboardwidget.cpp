#include "keyboardwidget.h"

uint8_t KeyboardWidget::m_keymap[40] =
{
    Key_1, Key_2, Key_3, Key_4, Key_5, Key_6, Key_7, Key_8, Key_9, Key_0,
    Key_Q, Key_W, Key_E, Key_R, Key_T, Key_Y, Key_U, Key_I, Key_O, Key_P,
    Key_A, Key_S, Key_D, Key_F, Key_G, Key_H, Key_J, Key_K, Key_L, Key_Enter,
   Key_CS, Key_Z, Key_X, Key_C, Key_V, Key_B, Key_N, Key_M, Key_SS, Key_Space
};

KeyboardWidget::KeyboardWidget(QWidget *parent)
    : QWidget{parent}
{
    setAttribute(Qt::WA_StyledBackground);
    setAttribute(Qt::WA_AcceptTouchEvents, true);

    m_keys = QStringList{"1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
                         "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P",
                         "A", "S", "D", "F", "G", "H", "J", "K", "L", "$",
                         "#", "Z", "X", "C", "V", "B", "N", "M", "%", "\""};

    for (int i=0; i<40; i++)
    {
        QPushButton *btn = new QPushButton(m_keys[i], this);
        int port = m_keymap[i] >> 5;
        int pin = m_keymap[i] & 0x1F;

        connect(btn, &QPushButton::pressed, this, [=]() {
            if (m_keyb)
                m_keyb->m_keyport[port] |= pin;
        });

        connect(btn, &QPushButton::released, this, [=]() {
            if (m_keyb)
                m_keyb->m_keyport[port] &= ~pin;
        });
    }
}

void KeyboardWidget::updateState()
{
    if (!m_keyb)
        return;
}

bool KeyboardWidget::event(QEvent *event)
{
    switch (event->type())
    {
    case QEvent::TouchBegin:
    case QEvent::TouchEnd:
    case QEvent::TouchUpdate:
        touchEvent(dynamic_cast<QTouchEvent *>(event));
        return true;
    case QEvent::Show: updateLayout(size());
    default:
        return QWidget::event(event);
    }
}

void KeyboardWidget::resizeEvent(QResizeEvent *e)
{
    updateLayout(e->size());
}

void KeyboardWidget::updateLayout(QSize sz)
{
    int ah = sz.width() * 5 / 20;

    float bw = sz.width() / 11.f;
    float bh = sz.height() / 5.f;
    if (ah < sz.height())
        bh = ah / 5.f;

    if (bw > bh * 1.8)
        bw = bh * 1.8;

    int bx = (sz.width() - bw * 11) / 2;

    int i = 0;
    for (QObject *obj: children())
    {
        QPushButton *b = qobject_cast<QPushButton *>(obj);
        if (!b)
            continue;
        int x = i % 10;
        int y = i / 10;
        b->setStyleSheet(QString("font-size: %1px; height: %2px;").arg(static_cast<int>(bh)).arg(sz.height() / 6));
        b->move(bx + (x + 0.3f) * bw + ((y % 3) * bh/3), (y + 0.2f) * b->height() * 1.2);
        i++;
    }
}

void KeyboardWidget::touchEvent(QTouchEvent *e)
{
    for (const QTouchEvent::TouchPoint &tp: e->touchPoints())
    {
        QPoint P = tp.pos().toPoint();
        QWidget *w = childAt(P);
        QPushButton *btn = qobject_cast<QPushButton*>(w);
        switch (tp.state())
        {
        case Qt::TouchPointPressed:
//            setStyleSheet("background-color: white;");
            if (btn)
            {
                m_touchedButtons[tp.id()] = btn;
                btn->setDown(true);
                emit btn->pressed();
            }
            break;

        case Qt::TouchPointReleased:
//            setStyleSheet("");
            if (m_touchedButtons.contains(tp.id()))
            {
                QPushButton *btn = m_touchedButtons[tp.id()];
                m_touchedButtons.remove(tp.id());
                btn->setDown(false);
                emit btn->released();
            }

        default:;
        }
    }
}
