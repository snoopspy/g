#pragma once

#include <GDefaultWidget>
#include <GTreeWidget>
#include "certmanager.h"

struct G_EXPORT CmWidget : GDefaultWidget, GProp {
	Q_OBJECT

public:
	CmWidget(QWidget* parent = nullptr);
	~CmWidget() override;

public:
	GTreeWidget* treeWidget_;
	CertManager certManager_{this};

public:
	void closeEvent(QCloseEvent* event) override;

public slots:
	void setControl();

private slots:
	void tbStart_clicked(bool checked = false);
	void tbStop_clicked(bool checked = false);
	void tbOption_clicked(bool checked = false);

	void processClosed();

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;
};
