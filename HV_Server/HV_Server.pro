#-------------------------------------------------
#
# Project created by QtCreator 2015-04-16T17:20:13
#
#-------------------------------------------------

QT       += core network
QT       -= gui

greaterThan(QT_MAJOR_VERSION, 4){
    QT += serialport
}else{
    CONFIG += serialport
}

TARGET = HV_Server
TEMPLATE = app

CONFIG   += console
CONFIG   -= app_bundle

include(../revision_info.pri )
include(../tcp/tcp.pri)
include(../easylogging.pri)
include(../ccpc/ccpc.pri)

INCLUDEPATH += $$PWD


contains(QT_MAJOR_VERSION, 5){

DEFINES += TEST_MODE \
#           VIRTUAL_MODE

CONFIG += c++11
}

SOURCES += main.cpp\
    hvserver.cpp \
    hvserverhandler.cpp \
    comport.cpp \
    hvcontroler.cpp \
    dividerreader.cpp \
    hvmaincontroller.cpp \

HEADERS  += \
    hvserver.h \
    hvserverhandler.h \
    comport.h \
    hvcontroler.h \
    dividerreader.h \
    hvmaincontroller.h \
