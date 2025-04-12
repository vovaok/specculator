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
    setAttribute(Qt::WA_TranslucentBackground, true);
    setFocusPolicy(Qt::NoFocus);

    QPushButton *btnL = new QPushButton(QChar(0xf104));
    QPushButton *btnR = new QPushButton(QChar(0xf105));
    QPushButton *btnU = new QPushButton(QChar(0xf106));
    QPushButton *btnD = new QPushButton(QChar(0xf107));
    QPushButton *btnFire = new QPushButton(QChar(0xf192));

    connect(btnL, &QPushButton::pressed, [this](){if (m_joy) m_joy->m_state.left = true;});
    connect(btnL, &QPushButton::released, [this](){if (m_joy) m_joy->m_state.left = false;});
    connect(btnR, &QPushButton::pressed, [this](){if (m_joy) m_joy->m_state.right = true;});
    connect(btnR, &QPushButton::released, [this](){if (m_joy) m_joy->m_state.right = false;});
    connect(btnU, &QPushButton::pressed, [this](){if (m_joy) m_joy->m_state.up = true;});
    connect(btnU, &QPushButton::released, [this](){if (m_joy) m_joy->m_state.up = false;});
    connect(btnD, &QPushButton::pressed, [this](){if (m_joy) m_joy->m_state.down = true;});
    connect(btnD, &QPushButton::released, [this](){if (m_joy) m_joy->m_state.down = false;});
    connect(btnFire, &QPushButton::pressed, [this](){if (m_joy) m_joy->m_state.fire = true;});
    connect(btnFire, &QPushButton::released, [this](){if (m_joy) m_joy->m_state.fire = false;});

    QWidget *pad = new QWidget;//(QChar(0xf047));
    pad->setObjectName("pad");
    QGridLayout *padlay = new QGridLayout;
    pad->setLayout(padlay);
    padlay->setContentsMargins(0, 0, 0, 0);
    padlay->setSpacing(0);
    padlay->addWidget(btnL, 1, 0);
    padlay->addWidget(btnR, 1, 2);
    padlay->addWidget(btnU, 0, 1);
    padlay->addWidget(btnD, 2, 1);

    QWidget *fire = new QWidget;//(QChar(0xf047));
    fire->setObjectName("fire");
    fire->setLayout(new QHBoxLayout);
    fire->layout()->addWidget(btnFire);

    QHBoxLayout *lay = new QHBoxLayout;
    setLayout(lay);
    lay->addWidget(pad, 0, Qt::AlignBottom);
    lay->addStretch(1);
    lay->addWidget(fire, 0, Qt::AlignBottom);
}

void JoystickWidget::setJoystickType(ZxJoystick::Type type)
{
    if (m_joy)
        m_joy->setType(type);
}

bool JoystickWidget::event(QEvent *event)
{

}

void JoystickWidget::resizeEvent(QResizeEvent *e)
{
//    int w = e->size().width();
//    int h = e->size().height();
//    m_padPos = QPoint(w/4, h/2);
//    m_firePos = QPoint(3*w/4, h/2);
}

void JoystickWidget::touchEvent(QTouchEvent *e)
{

}

void JoystickWidget::paintEvent(QPaintEvent *event)
{
//    QPainter p(this);
//    p.drawEllipse(m_padPos, );
}

