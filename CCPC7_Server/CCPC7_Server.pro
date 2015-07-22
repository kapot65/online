#-------------------------------------------------
#
# Project created by QtCreator 2015-03-12T16:02:14
#
#-------------------------------------------------

QT       += core network
QT       -= gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG   += console
CONFIG   -= app_bundle

TARGET = CCPC7_Server
TEMPLATE = app

include(../revision_info.pri )
include(../easylogging.pri )
include(../tcp/tcp.pri)

unix: !macx{
#LIBS += /home/user/QTProjects/QJson_build/lib/libqjson.so.0
LIBS += -L/home/user/QTProjects/QJson_build/lib/ -lqjson
INCLUDEPATH += /home/user/QTProjects/qjson-0.8.1/include \
               ccpc/original/

SOURCES += \
    ccpc/original/ccpc7.cpp \
    ccpc/original/camacop.cpp \
    ccpc/original/camacimpl.cpp \
    ccpc/original/ccpcisa.cpp \

HEADERS  += \
        ccpc/original/ccpc7.h \

}
win32{
contains(QT_MAJOR_VERSION, 4){
LIBS += -L$$PWD/../../SDK/qjson_master_build_4_8/src/ -llibqjson
INCLUDEPATH += D:/SDK/qjsonsrc/include \
               ccpc/virtual

SOURCES += \
        ccpc/virtual/ccpc7.cpp \
        ccpc/virtual/ccpc7base.cpp \

HEADERS  += \
        ccpc/virtual/ccpc7.h \
        ccpc/virtual/ccpc7base.h \
}

contains(QT_MAJOR_VERSION, 5){
LIBS += D:/SDK/qjson/lib/libqjson.dll.a
INCLUDEPATH += D:/SDK/qjsonsrc/include \
               ../CCPC7_Server/easylogging \
               ../CCPC7_Server/ccpc/virtual

CONFIG += c++11

SOURCES += \
        ccpc/virtual/ccpc7.cpp \
        ccpc/virtual/ccpc7base.cpp \

HEADERS  += \
        ccpc/virtual/ccpc7.h \
        ccpc/virtual/ccpc7base.h \
        easylogging/easylogging++.h
}
}

SOURCES += main.cpp\
    camacserver.cpp \
    camacalgoritm.cpp \
    commandhandler.cpp \
    camacserversettings.cpp \
    camacserverhandler.cpp

HEADERS  += \
    camacserver.h \
    camacalgoritm.h \
    commandhandler.h \
    camacserversettings.h \
    camacserverhandler.h
