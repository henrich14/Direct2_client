#-------------------------------------------------
#
# Project created by QtCreator 2016-04-29T15:06:07
#
#-------------------------------------------------

QT       += core gui network widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Direct2_client
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    waypoint.cpp \
    flightplan.cpp \
    myaircraft.cpp \
    aircraftwidget.cpp \
    buttonwidget.cpp

HEADERS  += mainwindow.h \
    waypoint.h \
    flightplan.h \
    myaircraft.h \
    aircraftwidget.h \
    buttonwidget.h

FORMS    += mainwindow.ui \
    aircraftwidget.ui \
    buttonwidget.ui
