QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport widgets

TARGET = tbpp
TEMPLATE = app

SOURCES += \
    libs/jsoncpp.cpp \
    libs/qcustomplot.cpp \
    backend.cpp \
    bigvector.cpp \
    civil.cpp \
    color.cpp \
    globaltime.cpp \
    maingui.cpp \
    mainwindow.cpp \
    mesh.cpp \
    myopenglwidget.cpp \
    serial.cpp \
    space.cpp \
    statwindow.cpp \
    utils.cpp \

HEADERS += \
    libs/jsoncpp.h \
    libs/qcustomplot.h \
    backend.h \
    civil.h \
    globaltime.h \
    mainwindow.h \
    myopenglwidget.h \
    space.h \
    statwindow.h \
    utils.h \

FORMS += \
    mainwindow.ui \
    statwindow.ui \

LIBS += -lopengl32

QMAKE_CXXFLAGS += -O3 -frename-registers
