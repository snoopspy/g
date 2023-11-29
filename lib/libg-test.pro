TEMPLATE = app
QT -= gui
CONFIG += console c++17
CONFIG += G_BUILD
DEFINES += G_BUILD
include(../g.pri)
TARGET = $${G_NAME}-test
DESTDIR = $${PWD}/../bin
include(libg-files.pri)
DEFINES *= GTEST
win32 {
	INCLUDEPATH *= $${PWD}/../../gtest/googletest/include
	LIBS *= -L$${PWD}/../../gtest/googletest/lib
	CONFIG(debug, debug|release) LIBS *= -lgtest_maind -lgtestd
	CONFIG(release, debug|release) LIBS *= -lgtest_main -lgtest
}
linux {
	LIBS *= -lgtest_main -lgtest
}
