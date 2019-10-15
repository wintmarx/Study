#-------------------------------------------------
#
# Project created by QtCreator 2017-09-10T10:50:32
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = AntennaPattern
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += $$PWD/include

DEFINES += CHARTDIR_HIDE_OBSOLETE _CRT_SECURE_NO_WARNINGS

win32 {
    contains(QMAKE_HOST.arch, x86_64) {
        LIBS += $$PWD/lib64/chartdir60.lib
        QMAKE_POST_LINK += copy /Y $$system_path($$PWD/lib64/chartdir60.dll) $(DESTDIR)
    } else {
        LIBS += $$PWD/lib32/chartdir60.lib
        QMAKE_POST_LINK += copy /Y $$system_path($$PWD/lib32/chartdir60.dll) $(DESTDIR)
    }
}

CONFIG += c++11

SOURCES += main.cpp\
        antennacell.cpp \
        mainwindow.cpp \
        qchartviewer.cpp

HEADERS  += mainwindow.h \
        antennacell.h \
        qchartviewer.h

FORMS += mainwindow.ui
