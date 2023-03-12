#include "gwaitevent.h"

// ----------------------------------------------------------------------------
// GTEST
// ----------------------------------------------------------------------------
#ifdef GTEST
#include <gtest/gtest.h>

#include <QThread>
#include <QDebug>

template <class T>
struct TestThread : QThread {
	TestThread(T* we, int loop, GDuration firstTimeout, GDuration timeout) : QThread(nullptr) {
		we_ = we;
		loop_ = loop;
		firstTimeout_ = firstTimeout;
		timeout_ = timeout;
	}
protected:
	void run() override {
		qDebug() << "  bef msleep first" << firstTimeout_;
		QThread::msleep(firstTimeout_);
		qDebug() << "  aft msleep first" << firstTimeout_;
		we_->wakeAll();
		for (int i = 0; i < loop_ - 1; i++) {
			qDebug() << "  bef msleep" << i;
			QThread::msleep(timeout_);
			qDebug() << "  aft msleep" << i;
			we_->wakeAll();
			qDebug() << "  aft wake" << i;
		}
	}
	T* we_;
	int loop_;
	GDuration firstTimeout_;
	GDuration timeout_;
};

TEST(GWaitEvent, waitEvenTest) {
	GWaitEvent we;
	TestThread<GWaitEvent> t(&we, 5, 100, 1000);
	t.start();
	for (int i = 0; i < 5; i++) {
		qDebug() << "waitEvenTest" << i;
		ASSERT_TRUE(we.wait(2000));
	}
	t.wait();
}

TEST(GWaitEvent, waitEventFailTest) {
	GWaitEvent we;
	TestThread<GWaitEvent> t(&we, 1, 0, 0);
	t.start();
	QThread::msleep(1000);
	qDebug() << "waitEventFailTest";
	ASSERT_FALSE(we.wait(2000));
	t.wait();
}

TEST(GWaitEvent, stateWaitEventTest) {
	GStateWaitEvent we;
	TestThread<GStateWaitEvent> t(&we, 5, 100, 1000);
	t.start();
	QThread::msleep(1000);
	for (int i = 0; i < 5; i++) {
		qDebug() << "stateWaitEventTest" << i;
		ASSERT_TRUE(we.wait(2000));
	}
	t.wait();
}

TEST(GWaitEvent, countWaitEventTest) {
	GCountWaitEvent we;
	TestThread<GCountWaitEvent> t(&we, 5, 0, 0);
	t.start();
	for (int i = 0; i < 5; i++) {
		qDebug() << "countWaitEventTest" << i;
		ASSERT_TRUE(we.wait(1000));
	}
	t.wait();
}

#endif // GTEST
