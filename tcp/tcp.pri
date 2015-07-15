QT += network

INCLUDEPATH += $$PWD

unix: !macx{
message(Unix)
INCLUDEPATH += $$PWD/easylogging/c++9/
HEADERS  += $$PWD/easylogging/c++9/easylogging++.h
}
win32{
contains(QT_MAJOR_VERSION, 4){
message(win32:Qt_4)
INCLUDEPATH += $$PWD/easylogging/c++9/
HEADERS  += $$PWD/easylogging/c++9/easylogging++.h
}

contains(QT_MAJOR_VERSION, 5){
message(win32:Qt_5)
CONFIG += c++11
INCLUDEPATH += $$PWD/CCPC7_Server/easylogging
HEADERS  += $$PWD/easylogging/easylogging++.h
}
}

INCLUDEPATH += $$PWD

SOURCES += $$PWD/event.cpp \
    $$PWD/tcpprotocol.cpp \
    $$PWD/tcpclient.cpp \
    $$PWD/tcpserver.cpp \
    $$PWD/inimanager.cpp

HEADERS  += $$PWD/event.h \
    $$PWD/tcpprotocol.h \
    $$PWD/tcpclient.h \
    $$PWD/tcpserver.h \
    $$PWD/inimanager.h
