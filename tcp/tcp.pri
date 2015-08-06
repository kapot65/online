QT += network

include(../easylogging.pri )

INCLUDEPATH += $$PWD

SOURCES += $$PWD/event.cpp \
    $$PWD/tcpprotocol.cpp \
    $$PWD/tcpclient.cpp \
    $$PWD/tcpserver.cpp \
    $$PWD/inimanager.cpp \
    $$PWD/tcpbase.cpp

HEADERS  += $$PWD/event.h \
    $$PWD/tcpprotocol.h \
    $$PWD/tcpclient.h \
    $$PWD/tcpserver.h \
    $$PWD/inimanager.h \
    $$PWD/tcpbase.h
