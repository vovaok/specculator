#include "screenwidget.h"
#include <QPainter>

ScreenWidget::ScreenWidget(QWidget *parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

void ScreenWidget::bindScreen(ZxScreen *scr)
{
    m_scr = scr;
    if (m_scr)
        setMinimumSize(m_scr->frame().size());
    update();
}

void ScreenWidget::paintEvent(QPaintEvent *)
{
    QImage frame = m_scr? m_scr->frame(): QImage();
    if (frame.isNull())
        return;

    int fw = frame.width();
    int fh = frame.height();
    int maxw = height() * fw / fh;
    int maxh = width() * fh / fw;
    int dw = width() - maxw;
    int dh = height() - maxh;
    QRect r = rect();
    if (dw > 0)
        r = QRect(dw/2, 0, maxw, height());
    else if (dh > 0)
        r = QRect(0, dh/2, width(), maxh);

    QPainter p(this);
    if (m_scr)
        p.fillRect(rect(), m_scr->borderColor());
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.drawImage(r, frame);
    p.end();
}
