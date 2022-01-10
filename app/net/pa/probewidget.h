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
};
