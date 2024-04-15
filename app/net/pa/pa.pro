QT += widgets
include(../../../g.pri)
DESTDIR = $$PWD/../../../bin

SOURCES += \
	pa.cpp \
	pawidget.cpp \
	probeanalyzer.cpp

HEADERS += \
	pawidget.h \
	probeanalyzer.h

contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
	ANDROID_PACKAGE_SOURCE_DIR = \
		$$PWD/android
}

DISTFILES += \
	android/AndroidManifest.xml \
	android/build.gradle \
	android/gradle.properties \
	android/gradle/wrapper/gradle-wrapper.jar \
	android/gradle/wrapper/gradle-wrapper.properties \
	android/gradlew \
	android/gradlew.bat \
	android/res/values/libs.xml

#------------------------------------------------------------------------------
# android
#------------------------------------------------------------------------------
android {
	deployment.files += $${G_DIR}/bin/ssdemon
	deployment.path = /assets
	INSTALLS += deployment
}
