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

include(../revision_info.pri )
include(../easylogging.pri )
include(../tcp/tcp.pri)

INCLUDEPATH += $$PWD

unix: !macx{
INCLUDEPATH += ccpc/original/

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
INCLUDEPATH += \
               ccpc/virtual
CONFIG += c++11

SOURCES += \
        ccpc/virtual/ccpc7.cpp \
        ccpc/virtual/ccpc7base.cpp \

HEADERS  += \
        ccpc/virtual/ccpc7.h \
        ccpc/virtual/ccpc7base.h \
}

contains(QT_MAJOR_VERSION, 5){
INCLUDEPATH += ccpc/virtual
CONFIG += c++11

SOURCES += \
        ccpc/virtual/ccpc7.cpp \
        ccpc/virtual/ccpc7base.cpp \

HEADERS  += \
        ccpc/virtual/ccpc7.h \
        ccpc/virtual/ccpc7base.h \
}
}

SOURCES += main.cpp\
    camacserver.cpp \
    camacalgoritm.cpp \
    commandhandler.cpp \
    camacserversettings.cpp \
    camacserverhandler.cpp \
    tempfolder.cpp

HEADERS  += \
    camacserver.h \
    camacalgoritm.h \
    commandhandler.h \
    camacserversettings.h \
    camacserverhandler.h \
    tempfolder.h
