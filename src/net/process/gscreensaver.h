// ----------------------------------------------------------------------------
//
// G Library
//
// http://gilgil.net
//
// Copyright (c) Gilbert Lee All rights reserved
//
// ----------------------------------------------------------------------------

#pragma once

#include "base/gstateobj.h"

#ifdef QT_GUI_LIB
#include <QWidget>
#include <QDateTime>
#include <QLabel>
#include <QTimer>

// ----------------------------------------------------------------------------
// ScreenSaverWidget
// ----------------------------------------------------------------------------
struct GScreenSaver;
struct ScreenSaverWidget : QWidget {
	Q_OBJECT

public:
	GScreenSaver* screenSaver_;
	QLabel* label_{nullptr};
	QTimer* updateTimer_{nullptr};
	QTimer* setLabelVisibleTimer_{nullptr};
	QRect lastRect_{};

public:
	explicit ScreenSaverWidget(GScreenSaver* screenSaver);
	~ScreenSaverWidget() override;

protected:
	void mousePressEvent(QMouseEvent *event) override;
	void paintEvent(QPaintEvent *event) override;
	void closeEvent(QCloseEvent *event) override;
	void keyPressEvent(QKeyEvent *event) override;

public slots:
	void updateCurrentTime();
	void setLabelVisible();
};

// ----------------------------------------------------------------------------
// GScreenSaver
// ----------------------------------------------------------------------------
struct G_EXPORT GScreenSaver : GStateObj {
	Q_OBJECT
	Q_PROPERTY(QString format MEMBER format_)
	Q_PROPERTY(int updateInterval MEMBER updateInterval_)
	Q_PROPERTY(ShowStyle showStyle MEMBER showStyle_)
	Q_PROPERTY(int red MEMBER red_)
	Q_PROPERTY(int green MEMBER green_)
	Q_PROPERTY(int blue MEMBER blue_)
	Q_PROPERTY(int fontSize MEMBER fontSize_)

	Q_ENUMS(ShowStyle)

public:
	enum ShowStyle {
		Show,
		ShowMaxized,
		ShowFullScreen
	};

public:
	QString format_{"h:mm AP"};
	int updateInterval_{10000}; // 10 sec
	ShowStyle showStyle_{ShowFullScreen};
	int red_{80};
	int green_{80};
	int blue_{80};
	int fontSize_{40};

public:
	Q_INVOKABLE GScreenSaver(QObject* parent = nullptr) : GStateObj(parent) {}
	~GScreenSaver() override { close(); }

protected:
	ScreenSaverWidget* screenSaverWidget_{nullptr};

protected:
	bool doOpen() override;
	bool doClose() override;
};
#else // QT_GUI_LIB
struct G_EXPORT GScreenSaver : GStateObj {
	Q_INVOKABLE GScreenSaver(QObject* parent = nullptr) : GStateObj(parent) {}
	~GScreenSaver() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;;
};
#endif // QT_GUI_LIB
