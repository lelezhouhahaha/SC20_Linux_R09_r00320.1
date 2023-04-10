TEMPLATE = app
TARGET = qmltel

QT += qml quick
CONFIG += c++11

SOURCES += main.cpp \
    tel_sdk.cpp


LIBS += -lpthread -lql_mcm_api

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    tel_sdk.h \
	inc/ql_in.h

