QT += widgets network
include(../../../g.pri)
DESTDIR = $$PWD/../../../bin

SOURCES += \
	ch.cpp \
	chwidget.cpp \
	cookiehijack.cpp \
	webserver.cpp

HEADERS += \
	chwidget.h \
	cookiehijack.h \
	webserver.h

RESOURCES += $${PWD}/ch.qrc

#------------------------------------------------------------------------------
# android
#------------------------------------------------------------------------------
android {
	ch_deployment.files += $${G_DIR}/bin/ffce
	ch_deployment.files += $${G_DIR}/bin/cert/root/root.crt
	ch_deployment.files += $${G_DIR}/bin/cert/root/root.key
	ch_deployment.files += $${G_DIR}/bin/cert/default.crt
	ch_deployment.files += $${G_DIR}/bin/cert/default.key
	ch_deployment.path = /assets
	INSTALLS += ch_deployment
}

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
