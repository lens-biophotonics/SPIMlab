QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SPIMlab

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    version.cpp

HEADERS += \
    mainwindow.h \
    version.h

RESOURCES += \
    ../resources.qrc
