QT -= qt gui
CONFIG += console
DESTDIR = $${PWD}/../../../bin
linux: LIBS += -lpcap
win32 {
	INCLUDEPATH += $${PWD}/../../../../npcap/Include
	LIBS += -L$${PWD}/../../../../npcap/Lib/x64
	LIBS += -lws2_32 -lwpcap -lpacket
}
SOURCES += *.cpp
HEADERS += *.h
