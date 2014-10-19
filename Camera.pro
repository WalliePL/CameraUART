#-------------------------------------------------
#
# Project created by QtCreator 2014-08-18T23:30:19
#
#-------------------------------------------------

QT       += core
QT       += serialport multimediawidgets
QT       += widgets

QT       -= gui

TARGET = Recorder
CONFIG   += console
CONFIG   -= app_bundle
CONFIG += c++11

TEMPLATE = app


SOURCES += main.cpp \
    camerathread.cpp

# OPENCV
"E:\Download\opencv\build\include"
INCLUDEPATH += E:\\OpenCV\\opencv\\build\\include
LIBS += -LE:\\OpenCV_release\\bin \
        -lopencv_calib3d249 \
        -lopencv_contrib249 \
        -lopencv_core249 \
        -lopencv_features2d249 \
        -lopencv_flann249 \
        -lopencv_gpu249 \
        -lopencv_highgui249 \
        -lopencv_imgproc249 \
        -lopencv_legacy249 \
        -lopencv_ml249 \
        -lopencv_objdetect249 \
        -lopencv_video249

HEADERS += \
    camerathread.h \
    globals.h
