#-------------------------------------------------
#
# Project created by QtCreator 2015-02-24T12:11:42
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4){
    QT += serialport
}else{
    CONFIG += serialport
}


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = serial_port_test
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
