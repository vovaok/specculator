#include "mainwindow.h"
#include <QApplication>
#include <QProxyStyle>
#include <QStyleOptionToolButton>

class MyProxyStyle : public QProxyStyle
{
public:
    virtual int pixelMetric(PixelMetric pm, const QStyleOption* option, const QWidget* widget) const
    {
        if (pm == QStyle::PM_ToolBarExtensionExtent)
        {
//            width: 2em;
//            font-size: 24pt;
//            pixelSize = width * font-size * dpi / 72;
            int size_pt = 2 * 24;
//            if (widget && widget->width() < widget->height())
//                size_pt = 1.5 * 24;
            int dpi = QGuiApplication::primaryScreen()->logicalDotsPerInch();
            return size_pt * dpi / 72; // width for horizontal, height for vertical toolbars
        }
        return QProxyStyle::pixelMetric(pm, option, widget);
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("vova_ok");
    QCoreApplication::setOrganizationDomain("vovaok.github.io/nerabotix");
    QCoreApplication::setApplicationName("Specculator");

    QApplication a(argc, argv);
    a.setStyle(new MyProxyStyle);
    MainWindow w;
    w.show();
    return a.exec();
}
