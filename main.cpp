#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("vova_ok");
    QCoreApplication::setOrganizationDomain("vovaok.github.io/nerabotix");
    QCoreApplication::setApplicationName("Specculator");

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
