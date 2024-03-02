TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt
DESTDIR = $${PWD}/../../../bin
INCLUDEPATH += $${PWD}/.. $${PWD}/../../../src
LIBS += -lpcap -lnetfilter_queue

SOURCES += \
	*.cpp \
	$${PWD}/../gaux.cpp \
	$${PWD}/../../../src/net/demon/gdemon.cpp

HEADERS += \
	*.h \
	$${PWD}/../gaux.h \
	$${PWD}/../../../src/net/demon/gdemon.h
