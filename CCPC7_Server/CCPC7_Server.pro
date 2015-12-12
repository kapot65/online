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

DEFINES += \
    VIRTUAL_MODE\
    TEST_MODE

greaterThan(QT_MAJOR_VERSION, 4){
    CONFIG += c++11
} else {
    #QMAKE_CXXFLAGS += -std=c++11
}

include(../common/common.pri )
include(../easylogging.pri )
include(../tcp/tcp.pri)
include(../ccpc/ccpc.pri)

INCLUDEPATH += $$PWD

SOURCES += main.cpp\
    camacserver.cpp \
    commandhandler.cpp \
    camacserversettings.cpp \
    camacserverhandler.cpp \
    tempfolder.cpp \
    camacalgoritm.cpp

HEADERS  += \
    camacserver.h \
    commandhandler.h \
    camacserversettings.h \
    camacserverhandler.h \
    tempfolder.h \
    camacalgoritm.h
