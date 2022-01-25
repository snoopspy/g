QT -= qt gui
CONFIG += console
DESTDIR = $${PWD}/../../../bin
INCLUDEPATH += $${PWD}/../../../src $${PWD}/..
LIBS += -lpcap -lnetfilter_queue
SOURCES += \
	*.cpp \
	$${PWD}/../../../src/net/demon/gdemon.cpp \
	$${PWD}/../../../src/net/iw/giw.cpp \
	$${PWD}/../../../src/net/iw/iwlib.c \
	$${PWD}/../gaux.cpp

HEADERS += \
	*.h \
	$${PWD}/../../../src/net/demon/gdemon.h \
	$${PWD}/../../../src/net/iw/giw.h \
	$${PWD}/../../../src/net/iw/wireless.h \
	$${PWD}/../gaux.h
