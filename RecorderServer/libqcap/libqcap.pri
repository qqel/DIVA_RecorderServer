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

#SOURCES += \
#    $$PWD/qcapbase.cpp \
#    $$PWD/qcapdevice.cpp \
#    $$PWD/qcappgm.cpp \
#    $$PWD/qcapstream.cpp \
#    $$PWD/qcapshare.cpp \


#HEADERS += \
#    $$PWD/qcapbase.h \
#    $$PWD/qcapdevice.h \
#    $$PWD/qcappgm.h \
#    $$PWD/qcapstream.h \
#    $$PWD/qcapshare.h \

