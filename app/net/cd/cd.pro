QT += widgets
include(../../../g.pri)
DESTDIR = $$PWD/../../../bin

SOURCES += \
	cd.cpp \
	cdwidget.cpp \
	cycledetect.cpp

HEADERS += \
	cdwidget.h \
	cycledetect.h \

RESOURCES += $$PWD/cd.qrc

contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
	ANDROID_PACKAGE_SOURCE_DIR = \
		$$PWD/android
}
