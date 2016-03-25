#-------------------------------------------------
#
# Project created by QtCreator 2015-04-16T17:20:13
#
#-------------------------------------------------

QT       += core network
QT       -= gui

greaterThan(QT_MAJOR_VERSION, 4){
    QT += serialport
    QT += widgets
}else{
    CONFIG += serialport
}

greaterThan(QT_MAJOR_VERSION, 4){
    CONFIG += c++11
} else {
    #QMAKE_CXXFLAGS += -std=c++11
}

CONFIG   += console
CONFIG   -= app_bundle

TARGET = HV_Server
TEMPLATE = app


include(../common/common.pri )
include(../tcp/tcp.pri)
include(../easylogging.pri)
include(../ccpc/ccpc.pri)

INCLUDEPATH += $$PWD

CONFIG(debug, debug|release){
DEFINES += TEST_MODE
}

contains(QT_MAJOR_VERSION, 5){

DEFINES += \
           VIRTUAL_MODE

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
