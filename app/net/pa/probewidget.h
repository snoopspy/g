#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QToolButton>

#include <GProp>

struct G_EXPORT ProbeWidget : QWidget, GProp {
public:
	ProbeWidget(QWidget* parent = nullptr);
	~ProbeWidget() override;

public:
	QTableWidget* tableWidget_{nullptr};
	QToolButton* tbStart_{nullptr};
	QToolButton* tbStop_{nullptr};
	QToolButton* tbOption_{nullptr};

signals:
	void tbStart_clicked(bool checked = false);
	void tbStop_clicked(bool checked = false);
	void tbOption_clicked(bool checked = false);
};
