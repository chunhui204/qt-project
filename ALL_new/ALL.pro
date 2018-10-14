#-------------------------------------------------
#
# Project created by QtCreator 2018-07-18T18:19:31
#
#-------------------------------------------------

QT += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets multimedia printsupport

TARGET = ALL
TEMPLATE = app


SOURCES += main.cpp\
    mainwidget.cpp \
    qcustomplot.cpp \
    audiowidget.cpp \
    wavfile.cpp \
    videowidget.cpp \
    videodatathread.cpp \
    videoplotthread.cpp \
    framewidget.cpp \
    gaussmodel/gaussianmg.cpp \
    gaussmodel/tensor.cpp \
    spectrumanalyser.cpp \
    setting.cpp \
    spectrumhelper.cpp \
    audiobase.cpp \
    widgets/ipwidgets.cpp \
    adselectsetting.cpp \
    labelmngdialog.cpp \
    audiowarper.cpp \
    tcpaudiowarper.cpp \
    localaudiowarper.cpp

HEADERS += mainwidget.h \
    qcustomplot.h \
    audiowidget.h \
    common.h \
    wavfile.h \
    videowidget.h \
    videodatathread.h \
    videoplotthread.h \
    framewidget.h \
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
    gaussmodel/tensor.h \
    spectrumanalyser.h \
    setting.h \
    spectrumhelper.h \
    audiobase.h \
    widgets/ipwidgets.h \
    adselectsetting.h \
    labelmngdialog.h \
    audiowarper.h \
    audioserver.h \
    tcpaudiowarper.h \
    localaudiowarper.h

FORMS += mainwidget.ui \
    audiowidget.ui \
    videowidget.ui \
    setting.ui \
    adselectsetting.ui \
    labelmngdialog.ui

fftreal_dir = ffft
widget_dir = widgets

INCLUDEPATH += $${fftreal_dir}
INCLUDEPATH += $${widget_dir}
CONFIG += C++11

CONFIG += console
