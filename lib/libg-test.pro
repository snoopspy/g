TEMPLATE = app
QT -= gui
QT += sql
CONFIG += console c++17
CONFIG += G_BUILD
DEFINES += G_BUILD
include(../g.pri)
TARGET = $${G_NAME}-test
DESTDIR = $${PWD}/../bin
include(libg-files.pri)
DEFINES *= GTEST
win32 {
	INCLUDEPATH *= $${PWD}/../../googletest/googletest/include
	LIBS *= -L$${PWD}/../../googletest/googletest/lib
}
LIBS *= -lgtest_main -lgtest
