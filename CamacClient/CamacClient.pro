#-------------------------------------------------
#
# Project created by QtCreator 2015-02-19T17:45:02
#
#-------------------------------------------------

QT       += core gui network printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CamacClient
TEMPLATE = app

greaterThan(QT_MAJOR_VERSION, 4){
    CONFIG += c++11
} else {
    #QMAKE_CXXFLAGS += -std=c++11
}

include(../easylogging.pri )
include(../common/common.pri )
include(../DataVisualizer/datavisualizer.pri)
include(../tcp/tcp.pri)



#включение тестовых функций
DEFINES += TEST_MODE
#виртуальный режим
#DEFINES += VIRTUAL_MODE

win32{
INCLUDEPATH += \
               $$PWD
CONFIG += c++11
}

HEADERS  += \
    camacclientform.h \
    ccpc7handlerform.h \
    hvhandler.h \
    hvhandlerform.h \
    serverhandler.h \
    ccpc7handler.h \
    online.h \
    onlineform.h

SOURCES += main.cpp\
    camacclientform.cpp \
    ccpc7handlerform.cpp \
    hvhandler.cpp \
    hvhandlerform.cpp \
    serverhandler.cpp \
    ccpc7handler.cpp \
    online.cpp \
    onlineform.cpp

FORMS    += \
    ccpc7handlerform.ui \
    camacclientform.ui \
    hvhandlerform.ui \
    onlineform.ui

RESOURCES += \
    recource.qrc
