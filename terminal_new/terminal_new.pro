#-------------------------------------------------
#
# Project created by QtCreator 2018-10-04T21:28:53
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network multimedia printsupport


TARGET = terminal_new
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwidget.cpp \
    qcustomplot.cpp \
    ipwidgets.cpp \
    audiowidget.cpp \
    videowidget.cpp \
    audiowarper.cpp \
    spectrumhelper.cpp \
    audiobase.cpp \
    gaussmodel/gaussianmg.cpp \
    gaussmodel/tensor.cpp

HEADERS += \
        mainwidget.h \
    qcustomplot.h \
    ipwidgets.h \
    audiowidget.h \
    videowidget.h \
    audiowarper.h \
    spectrumhelper.h \
    common.h \
    audiobase.h \
    ffft/Array.h \
    ffft/Array.hpp \
    ffft/def.h \
    ffft/DynArray.h \
    ffft/DynArray.hpp \
    ffft/FFTReal.h \
    ffft/FFTReal.hpp \
    ffft/FFTRealFixLen.h \
    ffft/FFTRealFixLen.hpp \
    ffft/FFTRealFixLenParam.h \
    ffft/FFTRealPassDirect.h \
    ffft/FFTRealPassDirect.hpp \
    ffft/FFTRealPassInverse.h \
    ffft/FFTRealPassInverse.hpp \
    ffft/FFTRealSelect.h \
    ffft/FFTRealSelect.hpp \
    ffft/FFTRealUseTrigo.h \
    ffft/FFTRealUseTrigo.hpp \
    ffft/OscSinCos.h \
    ffft/OscSinCos.hpp \
    gaussmodel/gaussianmg.h \
    gaussmodel/tensor.h

FORMS += \
        mainwidget.ui \
    audiowidget.ui \
    videowidget.ui

CONFIG += C11
