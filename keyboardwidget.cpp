#include "keyboardwidget.h"
#include <QPushButton>
#include <QGridLayout>

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
    setAttribute(Qt::WA_AcceptTouchEvents, true);

    m_keys = QStringList{"1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
                         "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P",
                         "A", "S", "D", "F", "G", "H", "J", "K", "L", "$",
                         "#", "Z", "X", "C", "V", "B", "N", "M", "%", "\""};

    QGridLayout *lay = new QGridLayout;
    lay->setSpacing(3);
//    lay->setContentsMargins(0, 0, 0, 0);
    setLayout(lay);

    for (int i=0; i<40; i++)
    {
        QPushButton *btn = new QPushButton(m_keys[i]);
        lay->addWidget(btn, i / 10, i % 10);
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

//    for (int i=0; i<8; i++)
//    {
//        for (int j=0; j<5; j++)
//        {

//        }
    //    }
}

#include <QDebug>

bool KeyboardWidget::event(QEvent *event)
{
    switch (event->type())
    {
    case QEvent::TouchBegin:
    case QEvent::TouchEnd:
    case QEvent::TouchUpdate:
        touchEvent(dynamic_cast<QTouchEvent *>(event));
        return true;
    default:
        return QWidget::event(event);
    }
}

void KeyboardWidget::resizeEvent(QResizeEvent *e)
{
    setStyleSheet(QString("font-size: %1px").arg(e->size().width() / 20));
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
            if (btn)
            {
                m_touchedButtons[tp.id()] = btn;
                emit btn->pressed();
            }
            break;

        case Qt::TouchPointReleased:
            if (m_touchedButtons.contains(tp.id()))
            {
                QPushButton *btn = m_touchedButtons[tp.id()];
                m_touchedButtons.remove(tp.id());
                emit btn->released();
            }

        default:;
        }
    }
}

