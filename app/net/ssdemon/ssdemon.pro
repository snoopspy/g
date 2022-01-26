QT -= qt gui
CONFIG += console
DESTDIR = $${PWD}/../../../bin
INCLUDEPATH += $${PWD}/../../../src $${PWD}/..
LIBS += -lpcap -lnetfilter_queue
SOURCES += \
	*.cpp \
	$${PWD}/../../../src/net/demon/gdemon.cpp \
	$${PWD}/../gaux.cpp

HEADERS += \
	*.h \
	$${PWD}/../../../src/net/demon/gdemon.h \
	$${PWD}/../gaux.h
