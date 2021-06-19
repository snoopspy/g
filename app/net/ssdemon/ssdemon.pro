QT -= qt gui
CONFIG += console
DESTDIR = $${PWD}/../../../bin
INCLUDEPATH += $${PWD}/../../../src/net/demon/
LIBS += -lpcap
SOURCES += *.cpp $${PWD}/../../../src/net/demon/gdemon.cpp
HEADERS += *.h $${PWD}/../../../src/net/demon/gdemon.h
