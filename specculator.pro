QT       += core gui multimedia

android: {
    QT += androidextras
}

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
    computer.cpp \
    cpuwidget.cpp \
    keyboardwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    screenwidget.cpp \
    sinclairbasic.cpp \
    tapewidget.cpp \
    z80.cpp \
    z80_alu.cpp \
    z80_blockcmd.cpp \
    zxbeeper.cpp \
    zxkeyboard.cpp \
    zxscreen.cpp \
    zxtape.cpp

HEADERS += \
    computer.h \
    cpuwidget.h \
    keyboardwidget.h \
    mainwindow.h \
    screenwidget.h \
    sinclairbasic.h \
    tapewidget.h \
    z80.h \
    zxbeeper.h \
    zxkeyboard.h \
    zxscreen.h \
    zxtape.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

#DEFINES += ANDROID_NDK
#DEFINES += LOG_NDEBUG
#DEFINES += ACCESS_RAW_DESCRIPTORS
#DEFINES += SIZEOF_SIZE_T=8

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle.properties \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml \
    android/res/xml/device_filter.xml \
    android/src/org/qtproject/example/serial/BuildCheck.java \
    android/src/org/qtproject/example/serial/DeviceFilter.java \
    android/src/org/qtproject/example/serial/HandlerThreadHandler.java \
    android/src/org/qtproject/example/serial/Serial.java \
    android/src/org/qtproject/example/serial/USBMonitor.java \
    android/src/org/qtproject/example/serial/USBVendorId.java \
    style.css

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
ANDROID_MIN_SDK_VERSION = "29"
ANDROID_TARGET_SDK_VERSION = "29"
