#include "gscreenkeeper.h"

#ifdef Q_OS_ANDROID
#include <QCoreApplication>
#include <QJniEnvironment>
#include <QJniObject>
#endif // Q_OS_ANDROID

// ----------------------------------------------------------------------------
// GScreenKeeper
// ----------------------------------------------------------------------------
#ifdef Q_OS_ANDROID
bool GScreenKeeper::doOpen() {
	return keepScreen(true);
}

bool GScreenKeeper::doClose() {
	return keepScreen(false);
}

bool GScreenKeeper::keepScreen(bool on) {
	bool res = true;
	QNativeInterface::QAndroidApplication::runOnAndroidMainThread([this, on, &res] {
		QJniObject activity = QNativeInterface::QAndroidApplication::context();
		if (!activity.isValid()) {
			SET_ERR(GErr::Fail, "activity is not valid");
			res = false;
			return;
		}

		QJniObject window = activity.callObjectMethod("getWindow", "()Landroid/view/Window;");
		if (!window.isValid()) {
			SET_ERR(GErr::Fail, "window is not valid");
			res = false;
			return;
		}

		static const int FLAG_KEEP_SCREEN_ON = 128;
		if (on)
			window.callMethod<void>("addFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
		else
			window.callMethod<void>("clearFlags", "(I)V", FLAG_KEEP_SCREEN_ON);

		QJniEnvironment env;
		if (env->ExceptionCheck())
			env->ExceptionClear();
	});
	return res;
}
#else // Q_OS_ANDROID
bool GScreenKeeper::doOpen() {
	return true;
}

bool GScreenKeeper::doClose() {
	return true;
}
#endif // Q_OS_ANDROID
