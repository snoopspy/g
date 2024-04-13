#pragma once

#include <GDefaultWidget>
#include <GTreeWidget>
#include <QPlainTextEdit>
#include "cookiehijack.h"

struct G_EXPORT ChWidget : GDefaultWidget, GProp {
	Q_OBJECT

public:
	ChWidget(QWidget* parent = nullptr);
	~ChWidget() override;

public:
	void setControl();

public:
	QSplitter* splitter_{nullptr};
	GTreeWidget* treeWidget_{nullptr};
	QPlainTextEdit* plainTextEdit_{nullptr};
	CookieHijack cookieHijack_{this};

public:
	QToolButton* tbFirefox_{nullptr};

public:
	const static int ColumnHost = 0;
	const static int ColumnCookie = 1;

public:
	void addItem(QString host, QString cookie);

public:
	void closeEvent(QCloseEvent* event) override;

private slots:
	void tbStart_clicked(bool checked = false);
	void tbStop_clicked(bool checked = false);
	void tbOption_clicked(bool checked = false);
	void tbFirefox_clicked(bool checked = false);
	void treeWidget_itemSelectionChanged();

	void processProbeDetected(GMac mac, QString type, int channel, int signal);
	void processClosed();

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;
};
