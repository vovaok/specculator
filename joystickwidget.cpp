#include "joystickwidget.h"
#include <QResizeEvent>
#include <QPainter>
#include <QHBoxLayout>
#include <QPushButton>
#include <QGridLayout>

JoystickWidget::JoystickWidget(QWidget *parent)
    : QWidget{parent}
{
    setAttribute(Qt::WA_AcceptTouchEvents, true);
    setFocusPolicy(Qt::NoFocus);

    QPushButton *btnL = new QPushButton(QChar(0xf104));
    QPushButton *btnR = new QPushButton(QChar(0xf105));
    QPushButton *btnU = new QPushButton(QChar(0xf106));
    QPushButton *btnD = new QPushButton(QChar(0xf107));
    m_btnFire = new QPushButton(QChar(0xf192), this);
    m_btnFire->setObjectName("fire");

    m_buttons << btnL << btnR << btnU << btnD << m_btnFire;

    connect(btnL, &QPushButton::pressed, [this](){if (m_joy) m_joy->m_state.left = true;});
    connect(btnL, &QPushButton::released, [this](){if (m_joy) m_joy->m_state.left = false;});
    connect(btnR, &QPushButton::pressed, [this](){if (m_joy) m_joy->m_state.right = true;});
    connect(btnR, &QPushButton::released, [this](){if (m_joy) m_joy->m_state.right = false;});
    connect(btnU, &QPushButton::pressed, [this](){if (m_joy) m_joy->m_state.up = true;});
    connect(btnU, &QPushButton::released, [this](){if (m_joy) m_joy->m_state.up = false;});
    connect(btnD, &QPushButton::pressed, [this](){if (m_joy) m_joy->m_state.down = true;});
    connect(btnD, &QPushButton::released, [this](){if (m_joy) m_joy->m_state.down = false;});
    connect(m_btnFire, &QPushButton::pressed, [this](){if (m_joy) m_joy->m_state.fire = true;});
    connect(m_btnFire, &QPushButton::released, [this](){if (m_joy) m_joy->m_state.fire = false;});

    m_pad = new QWidget(this);//(QChar(0xf047));
    m_pad->setObjectName("pad");
    QGridLayout *padlay = new QGridLayout;
    m_pad->setLayout(padlay);
    padlay->setContentsMargins(0, 0, 0, 0);
    padlay->setSpacing(0);
    padlay->addWidget(btnL, 1, 0);
    padlay->addWidget(btnR, 1, 2);
    padlay->addWidget(btnU, 0, 1);
    padlay->addWidget(btnD, 2, 1);

//    QWidget *fire = new QWidget;//(QChar(0xf047));
//    fire->setObjectName("fire");
//    fire->setLayout(new QHBoxLayout);
//    fire->layout()->addWidget(m_btnFire);

    QHBoxLayout *lay = new QHBoxLayout;
    setLayout(lay);
    lay->addWidget(m_pad, 0, Qt::AlignBottom);
    lay->addStretch(1);
    lay->addWidget(m_btnFire, 0, Qt::AlignBottom);
}

void JoystickWidget::setJoystickType(ZxJoystick::Type type)
{
    if (m_joy)
        m_joy->setType(type);
}

bool JoystickWidget::event(QEvent *event)
{
    switch (event->type())
    {
    case QEvent::TouchBegin:
    case QEvent::TouchEnd:
    case QEvent::TouchUpdate:
        touchEvent(dynamic_cast<QTouchEvent *>(event));
        return true;
    case QEvent::MouseButtonPress:
        return false;
    default:
        return QWidget::event(event);
    }
}

#include <QDebug>
void JoystickWidget::resizeEvent(QResizeEvent *e)
{
    QRegion r1(m_pad->rect().translated(m_pad->pos()).adjusted(-2, -2, 2, 2), QRegion::Ellipse);
//    qDebug() << m_btnFire->pos() << m_btnFire->rect();
//    qDebug() << m_btnFire->parentWidget()->pos();
//    qDebug() << this->pos();
//    qDebug() << m_btnFire->
    QRegion r2(m_btnFire->rect().translated(m_btnFire->pos()).adjusted(-1, -1, 2, 2), QRegion::Ellipse);
    setMask(r1.united(r2));

//    int w = e->size().width();
//    int h = e->size().height();
//    m_padPos = QPoint(w/4, h/2);
//    m_firePos = QPoint(3*w/4, h/2);
}

void JoystickWidget::touchEvent(QTouchEvent *e)
{
    for (const QTouchEvent::TouchPoint &tp: e->touchPoints())
    {
        QPoint P = tp.pos().toPoint();
//        QWidget *w = childAt(P);
//        QPushButton *btn = qobject_cast<QPushButton*>(w);

        for (QPushButton *btn: m_buttons)
        {
            QRect r = btn->rect();
            r.translate(btn->mapTo(this, btn->pos()));
            if (r.contains(P))
                qDebug() << btn;
        }

//        switch (tp.state())
//        {
//        case Qt::TouchPointPressed:
//            if (btn)
//            {
//                m_touchedButtons[tp.id()] = btn;
//                btn->setDown(true);
//                emit btn->pressed();
//            }
//            break;

//        case Qt::TouchPointReleased:
//            if (m_touchedButtons.contains(tp.id()))
//            {
//                QPushButton *btn = m_touchedButtons[tp.id()];
//                m_touchedButtons.remove(tp.id());
//                btn->setDown(false);
//                emit btn->released();
//            }

//        default:;
//        }
    }
}

//void JoystickWidget::paintEvent(QPaintEvent *event)
//{
//    QPainter p(this);
//    p.drawEllipse(m_padPos, );
//}

