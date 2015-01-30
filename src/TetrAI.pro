#-------------------------------------------------
#
# Project created by QtCreator 2014-07-04T06:31:08
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TetrAI
TEMPLATE = app


SOURCES += main.cpp \
    tetraiwindow.cpp \
    tetrai.cpp \
    tetraiworker.cpp \
    rectboardview.cpp \
    aimodule_a.cpp \
    aimodule_b.cpp \
    figures.cpp

HEADERS  += \
    tetraiwindow.h \
    tetrai.h \
    tetraiworker.h \
    rectboardview.h \
    history.h \
    qdeepcopy.h \
    aimodule.h \
    aimodule_a.h \
    aimodule_a_intf_a.h \
    aimodule_b.h \
    aimodule_a_intf_bb.h \
    aimodule_a_intf_ba.h \
    aimodule_b_intf_aa.h \
    aimodule_b_intf_ab.h

FORMS    += \
    tetraiwindow.ui

RESOURCES += \
    resources.qrc
