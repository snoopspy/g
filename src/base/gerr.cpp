#include "gerr.h"

// ----------------------------------------------------------------------------
// GErr
// ----------------------------------------------------------------------------
QDebug operator << (QDebug os, GErr& err) {
	os
		<< err.name() << ":"
		<< (err.msg().isEmpty() ? "no msg" : err.msg())
		<< " code =" << err.code();
	return os;
}

QDebug operator << (QDebug os, GErr&& err) {
	return operator << (os, err);
}

QDebug operator << (QDebug os, GErr* err) {
	if (err == nullptr) {
		os << "err is null";
		return os;
	}
	return operator << (os, *err);
}

QDebug operator << (QDebug os, QSharedPointer<GErr> err) {
	return operator << (os, err.data());
}

// ----------------------------------------------------------------------------
// GTEST
// ----------------------------------------------------------------------------
#ifdef GTEST
#include <gtest/gtest.h>

TEST(GErr, lastErrTest) {
#ifdef Q_OS_LINUX
	int lastErrIdx = EHWPOISON;
#endif
#ifdef Q_OS_WIN
	int lastErrIdx = 42; // "Illegal byte sequence"
#endif
	for (int n = 0; n <= lastErrIdx + 10; n++) {
		errno = n;
		GLastErr lastErr;
		qDebug() << lastErr;
		EXPECT_TRUE(lastErr.code() == n);
	}
}

TEST(GErr, errTest) {
	GErr err(GErr::NotSupported, "not supported");
	qDebug() << err;
}

TEST(GErr, nullTest) {
	GLastErr* lastErr = nullptr;
	qDebug() << lastErr;
}

#endif // GTEST
