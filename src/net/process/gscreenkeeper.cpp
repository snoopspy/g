#include "gscreenkeeper.h"

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#include <QAndroidJniEnvironment>
#endif // Q_OS_ANDROID

// ----------------------------------------------------------------------------
// GScreenKeeper
// ----------------------------------------------------------------------------
#ifdef Q_OS_ANDROID
bool GScreenKeeper::doOpen() {
	return keepScreen(true);
}

bool GScreenKeeper::doClose() {
	return keepScreen(true);
}

bool GScreenKeeper::keepScreen(bool on) {
	QAndroidJniObject activity = QtAndroid::androidActivity();
	if (!activity.isValid()) {
		SET_ERR(GErr::Fail, "activity is not valid");
		return false;
	}

	QAndroidJniObject window = activity.callObjectMethod("getWindow", "()Landroid/view/Window;");
	if (!window.isValid()) {
		SET_ERR(GErr::Fail, "window is not valid");
		return false;
	}

	static const int FLAG_KEEP_SCREEN_ON = 128;
	if (on)
		window.callMethod<void>("addFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
	else
		window.callMethod<void>("clearFlags", "(I)V", FLAG_KEEP_SCREEN_ON);

	QAndroidJniEnvironment env;
	if (env->ExceptionCheck()) {
		env->ExceptionClear();
	}
	return true;
}
#else // Q_OS_ANDROID
bool GScreenKeeper::doOpen() {
	qWarning() << "do nothing";
	return true;
}

bool GScreenKeeper::doClose() {
	qWarning() << "do nothing";
	return true;
}
#endif // Q_OS_ANDROID
