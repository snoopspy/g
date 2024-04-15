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
	android/res/drawable-hdpi/icon.png \
	android/res/drawable-ldpi/icon.png \
	android/res/drawable-mdpi/icon.png \
	android/res/drawable-xhdpi/icon.png \
	android/res/drawable-xxhdpi/icon.png \
	android/res/drawable-xxxhdpi/icon.png \
	android/res/values/libs.xml

#------------------------------------------------------------------------------
# android
#------------------------------------------------------------------------------
android {
	deployment.files += $${G_DIR}/bin/ssdemon
	deployment.path = /assets
	INSTALLS += deployment
}
