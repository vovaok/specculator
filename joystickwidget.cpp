#include "joystickwidget.h"
#include <QResizeEvent>
#include <QPainter>
#include <QHBoxLayout>
#include <QPushButton>
#include <QGridLayout>
#include <QApplication>
#include <math.h>

JoystickWidget::JoystickWidget(QWidget *parent)
    : QWidget{parent}
{
    setAttribute(Qt::WA_AcceptTouchEvents, true);
    setFocusPolicy(Qt::NoFocus);
    int fontSizePx = qApp->property("fontSizePx").toInt();

    m_btnL = new QPushButton(QChar(0xf104));
    m_btnR = new QPushButton(QChar(0xf105));
    m_btnU = new QPushButton(QChar(0xf106));
    m_btnD = new QPushButton(QChar(0xf107));
    m_btnFire = new QPushButton(QChar(0xf192), this);
    m_btnFire->setObjectName("fire");

//    m_padButtons << btnL << btnR << btnU << btnD;

    connect(m_btnL, &QPushButton::pressed, [this](){if (m_joy) m_joy->m_state.left = true;});
    connect(m_btnL, &QPushButton::released, [this](){if (m_joy) m_joy->m_state.left = false;});
    connect(m_btnR, &QPushButton::pressed, [this](){if (m_joy) m_joy->m_state.right = true;});
    connect(m_btnR, &QPushButton::released, [this](){if (m_joy) m_joy->m_state.right = false;});
    connect(m_btnU, &QPushButton::pressed, [this](){if (m_joy) m_joy->m_state.up = true;});
    connect(m_btnU, &QPushButton::released, [this](){if (m_joy) m_joy->m_state.up = false;});
    connect(m_btnD, &QPushButton::pressed, [this](){if (m_joy) m_joy->m_state.down = true;});
    connect(m_btnD, &QPushButton::released, [this](){if (m_joy) m_joy->m_state.down = false;});
    connect(m_btnFire, &QPushButton::pressed, [this](){if (m_joy) m_joy->m_state.fire = true;});
    connect(m_btnFire, &QPushButton::released, [this](){if (m_joy) m_joy->m_state.fire = false;});

    m_pad = new QWidget(this);//(QChar(0xf047));
    m_pad->setObjectName("pad");
    QGridLayout *padlay = new QGridLayout;
    m_pad->setLayout(padlay);
    padlay->setContentsMargins(0, 0, 0, 0);
    padlay->setSpacing(0);
    padlay->addWidget(m_btnL, 1, 0);
    padlay->addWidget(m_btnR, 1, 2);
    padlay->addWidget(m_btnU, 0, 1);
    padlay->addWidget(m_btnD, 2, 1);

//    QWidget *fire = new QWidget;//(QChar(0xf047));
//    fire->setObjectName("fire");
//    fire->setLayout(new QHBoxLayout);
//    fire->layout()->addWidget(m_btnFire);

    QHBoxLayout *lay = new QHBoxLayout;
    setLayout(lay);
    lay->addSpacing(fontSizePx * 3);
    lay->addWidget(m_pad, 0, Qt::AlignBottom);
    lay->addStretch(1);
    lay->addWidget(m_btnFire, 0, Qt::AlignBottom);
    lay->addSpacing(fontSizePx * 3);
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
//    for (QPushButton *btn: m_buttons)
//    {
//        bool cont = false;
//        for (const QTouchEvent::TouchPoint &tp: e->touchPoints())
//        {
//            QPoint P = tp.pos().toPoint();
//            QRect r = btn->rect();
//            r.translate(btn->mapTo(this, QPoint()));//btn->pos()));
//            qDebug() << r << P;
//            if (r.contains(P))
//            {
//                if (!btn->isDown())
//                {
//                    btn->setDown(true);
//                    emit btn->pressed();
//                }
//                cont = true;
//            }
//        }
//        if (btn->isDown() && !cont)
//        {
//            btn->setDown(false);
//            emit btn->released();
//        }
//    }

    QRect padRect = m_pad->rect().translated(m_pad->pos());
    int thr = padRect.width() / 6;
//    int xL = -padRect.width() / 6;
//    int xR = padRect.width() / 6;
//    int yU = -padRect.height() / 6;
//    int yD = padRect.height() / 6;


    for (const QTouchEvent::TouchPoint &tp: e->touchPoints())
    {
        QPoint P = tp.pos().toPoint();
        QWidget *w = childAt(P);
        bool pad = (w == m_pad || (w && w->parent() == m_pad));
        bool fire = (w == m_btnFire);

        switch (tp.state())
        {
        case Qt::TouchPointPressed:
            if (fire)
            {
                m_fireTouchId = tp.id();
                setButtonState(m_btnFire, true);
            }
            else if (pad)
            {
                m_padTouchId = tp.id();
            }
            // no break;

        case Qt::TouchPointMoved:
            if (tp.id() == m_padTouchId)
            {
                P -= padRect.center();
                int len = hypotf(P.x(), P.y());
                if (len > 2 * thr)
                    P = P * (2 * thr) / len;
                setButtonState(m_btnL, P.x() < -thr);
                setButtonState(m_btnR, P.x() > thr);
                setButtonState(m_btnU, P.y() < -thr);
                setButtonState(m_btnD, P.y() > thr);
            }
            break;

        case Qt::TouchPointReleased:
            if (tp.id() == m_fireTouchId)
            {
                m_fireTouchId = -1;
                setButtonState(m_btnFire, false);
            }
            else if (tp.id() == m_padTouchId)
            {
                m_padTouchId = -1;
                setButtonState(m_btnL, false);
                setButtonState(m_btnR, false);
                setButtonState(m_btnU, false);
                setButtonState(m_btnD, false);
            }

        default:;
        }
    }
}

void JoystickWidget::setButtonState(QPushButton *btn, bool state)
{
    if (!btn->isDown() && state)
    {
        btn->setDown(true);
        emit btn->pressed();
    }
    else if (btn->isDown() && !state)
    {
        btn->setDown(false);
        emit btn->released();
    }
}

//void JoystickWidget::paintEvent(QPaintEvent *event)
//{
//    QPainter p(this);
//    p.drawEllipse(m_padPos, );
//}

