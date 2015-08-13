#-------------------------------------------------
#
# Project created by QtCreator 2015-04-16T17:20:13
#
#-------------------------------------------------

QT       += core network serialport
QT       -= gui

TARGET = HV_Server
TEMPLATE = app

CONFIG   += console
CONFIG   -= app_bundle

include(../revision_info.pri )
include(../tcp/tcp.pri)
include(../easylogging.pri)

unix: !macx{
#LIBS += /home/user/QTProjects/QJson_build/lib/libqjson.so.0
LIBS += -L$$PWD/../QJson_build/lib/ -lqjson
INCLUDEPATH += /home/user/QTProjects/qjson-0.8.1/include
}

win32{

contains(QT_MAJOR_VERSION, 4){
LIBS += -L$$PWD/../../SDK/qjson_master_build_4_8/src/ -llibqjson
INCLUDEPATH += D:/SDK/qjsonsrc/include
}

contains(QT_MAJOR_VERSION, 5){

DEFINES += USE_QTJSON \
           TEST_MODE \
           VIRTUAL_MODE

CONFIG += c++11

INCLUDEPATH += \
               D:/SDK/qjsonsrc/include

}
}

SOURCES += main.cpp\
    hvserver.cpp \
    hvserverhandler.cpp \
    comport.cpp \
    hvcontroler.cpp \
    dividerreader.cpp \

HEADERS  += \
    hvserver.h \
    hvserverhandler.h \
    comport.h \
    hvcontroler.h \
    dividerreader.h \
