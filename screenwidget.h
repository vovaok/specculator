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
    void paintEvent(QPaintEvent *e) override;
    virtual QSize sizeHint() const override {return m_scr? m_scr->frame().size() * 2: QSize(640, 480);}

private:
    ZxScreen *m_scr = nullptr;
};

#endif // SCREENWIDGET_H
