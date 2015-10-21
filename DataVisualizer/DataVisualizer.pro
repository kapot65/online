#-------------------------------------------------
#
# Project created by QtCreator 2015-06-16T11:43:32
#
#-------------------------------------------------

QT       += core gui


DEFINES += TEST_MODE \
           APD_MODE

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = DataVisualizer
TEMPLATE = app

greaterThan(QT_MAJOR_VERSION, 4){
    CONFIG += c++11
} else {
    QMAKE_CXXFLAGS += -std=c++11
}

include(../revision_info.pri )
include(datavisualizer.pri)
include(../easylogging.pri)
include(../tcp/tcp.pri)



INCLUDEPATH += $$PWD

SOURCES += main.cpp \
    DataVisualizerWindow.cpp

HEADERS  += \
    DataVisualizerWindow.h

FORMS    += \
    DataVisualizerWindow.ui
