TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt
DESTDIR = $${PWD}/../../../bin
INCLUDEPATH += $${PWD}/..
win32: LIBS += -lws2_32

SOURCES += \
	*.c \
	*.cpp \
	$${PWD}/../gaux.cpp

HEADERS += \
	*.h \
	$${PWD}/../gaux.h
