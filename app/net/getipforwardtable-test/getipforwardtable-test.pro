CONFIG -= qt
CONFIG += console
DESTDIR = $${PWD}/../../../bin
SOURCES += *.cpp
LIBS *= -lws2_32 -liphlpapi


