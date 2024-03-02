TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt
DESTDIR = $${PWD}/../../../bin
INCLUDEPATH += $${PWD}/..
linux: LIBS += -lpcap
win32 {
	INCLUDEPATH += $${PWD}/../../../../npcap/Include
	LIBS += -L$${PWD}/../../../../npcap/Lib/x64
	LIBS += -lws2_32 -lwpcap -lpacket
}

SOURCES += \
	*.cpp \
	$${PWD}/../gaux.cpp

HEADERS += \
	*.h \
	$${PWD}/../gaux.h
