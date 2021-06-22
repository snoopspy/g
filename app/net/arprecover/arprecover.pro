QT -= qt gui
CONFIG += console
DESTDIR = $${PWD}/../../../bin
LIBS += -lpcap
SOURCES += *.cpp
HEADERS += *.h
