QT += network

include(../easylogging.pri )

lessThan(QT_MAJOR_VERSION, 5){
    unix: !macx{
        LIBS += -L/usr/lib/i386-linux-gnu/ -lqjson
        INCLUDEPATH += /usr/include/qjson
    }

    win32{
        LIBS += D:/SDK/qjson/lib/libqjson.dll.a
        INCLUDEPATH += D:/SDK/qjsonsrc/src
    }
}else{
    DEFINES += USE_QTJSON
}

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
