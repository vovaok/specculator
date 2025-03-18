QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

win32: {
    DESTDIR = $$PWD/$$TARGET
    CONFIG(release,debug|release) {
        QMAKE_POST_LINK += windeployqt --no-translations $$DESTDIR
    }
}

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    z80.cpp \
    z80_alu.cpp \
    z80_blockcmd.cpp \
    zxbeeper.cpp \
    zxkeyboard.cpp \
    zxscreen.cpp \
    zxtape.cpp

HEADERS += \
    mainwindow.h \
    z80.h \
    zxbeeper.h \
    zxkeyboard.h \
    zxscreen.h \
    zxtape.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
