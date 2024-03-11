TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt
DESTDIR = $${PWD}/../../../bin
INCLUDEPATH += $${PWD}/..

SOURCES += \
	*.c \
	*.cpp \
	$${PWD}/../gaux.cpp

HEADERS += \
	*.h \
	$${PWD}/../gaux.h
