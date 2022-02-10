OS_BIT_TYPE=$$(__DOTNET_PREFERRED_BITNESS)

QT += multimedia

win32 {
INCLUDEPATH += $$PWD/INC/
    equals(OS_BIT_TYPE, 64){
        LIBS += $$PWD/LIB/X64/QCAP.X64.LIB
    }
    else :equals(OS_BIT_TYPE, 32) {
        LIBS += $$PWD/LIB/X86/QCAP.X86.LIB
    }
}

unix {

}

SOURCES += \
    $$PWD/qcapbase.cpp \
    $$PWD/qcapdevice.cpp \
    $$PWD/qcapstream.cpp \
    $$PWD/qcapshare.cpp \
    $$PWD/qcapwebstream.cpp \


HEADERS += \
    $$PWD/qcapbase.h \
    $$PWD/qcapdevice.h \
    $$PWD/qcapstream.h \
    $$PWD/qcapshare.h \
    $$PWD/qcapwebstream.h \

