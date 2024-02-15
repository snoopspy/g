CONFIG -= qt
CONFIG += console
DESTDIR = $${PWD}/../../../bin
SOURCES += *.cpp
LIBS *= -liphlpapi
