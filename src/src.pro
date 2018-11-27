QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SPIMlab

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    version.cpp \
    logwidget.cpp \
    logger.cpp \
    nidevice.cpp \
    centralwidget.cpp \
    cameradisplay.cpp \
    cameratrigger.cpp

HEADERS += \
    mainwindow.h \
    version.h \
    logwidget.h \
    logger.h \
    nidevice.h \
    centralwidget.h \
    cameradisplay.h \
    cameratrigger.h

RESOURCES += \
    ../resources.qrc

LIBS += \
    -lqwt-qt5

WITH_HARDWARE {
    DEFINES += WITH_HARDWARE
    LIBS += -lnidaqmx
}

CONFIG(debug) {
    DISTFILES += \
        ../.astylerc \
}
