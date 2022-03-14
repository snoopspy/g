QT += widgets
include(../../../g.pri)
DESTDIR = $$PWD/../../../bin

SOURCES += \
	wa.cpp \
	wawidget.cpp \
	wifianalyzer.cpp

HEADERS += \
	wawidget.h \
	wifianalyzer.h

DISTFILES += \
	android/AndroidManifest.xml \
	android/build.gradle \
	android/gradle.properties \
	android/gradle/wrapper/gradle-wrapper.jar \
	android/gradle/wrapper/gradle-wrapper.properties \
	android/gradlew \
	android/gradlew.bat \
	android/res/values/libs.xml

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
