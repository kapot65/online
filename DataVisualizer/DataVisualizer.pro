#-------------------------------------------------
#
# Project created by QtCreator 2015-06-16T11:43:32
#
#-------------------------------------------------

QT       += core gui printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DataVisualizer
TEMPLATE = app

DEFINES += TEST_MODE
DEFINES += USE_QTJSON

include(../revision_info.pri )
include (datavisualizer.pri)
include(../easylogging.pri)
include(../tcp/tcp.pri)

INCLUDEPATH += $$PWD

SOURCES += main.cpp \
    DataVisualizerWindow.cpp

HEADERS  += \
    DataVisualizerWindow.h

FORMS    += \
    DataVisualizerWindow.ui
