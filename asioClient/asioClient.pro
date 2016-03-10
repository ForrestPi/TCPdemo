TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

include(deployment.pri)
qtcAddDeployment()

QMAKE_CXXFLAGS += -std=c++11
INCLUDEPATH += D:\API\boost_1_60_0

HEADERS += \
    Connector.h \
    RWHandler.h

