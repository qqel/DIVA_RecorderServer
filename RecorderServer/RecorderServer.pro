QT += core gui concurrent websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ../../RecorderServer/release/RecorderServer

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += $$PWD/libqcap/INC/
LIBS += $$PWD/libqcap/LIB/X64/QCAP.X64.LIB


SOURCES += \
    main.cpp \
    mainwindow.cpp \
    qcaphandler.cpp \
    websockethandler.cpp

HEADERS += \
    mainwindow.h \
    qcaphandler.h \
    websockethandler.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
