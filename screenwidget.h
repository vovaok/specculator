#ifndef SCREENWIDGET_H
#define SCREENWIDGET_H

#include <QWidget>
#include "zxscreen.h"

class ScreenWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ScreenWidget(QWidget *parent = nullptr);
    void bindScreen(ZxScreen *scr);

protected:
    void paintEvent(QPaintEvent *e);

private:
    ZxScreen *m_scr = nullptr;
};

#endif // SCREENWIDGET_H
