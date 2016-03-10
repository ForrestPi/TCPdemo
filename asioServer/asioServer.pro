TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

include(deployment.pri)
qtcAddDeployment()

INCLUDEPATH += D:\API\boost_1_60_0

QMAKE_CXXFLAGS += -std=c++11

HEADERS += \
    RWHandler.h \
    Message.h \
    Server.h \
    test.h

