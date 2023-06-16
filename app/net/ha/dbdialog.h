#pragma once

#include <QDialog>
#include <QTableView>
#include <QTabWidget>
#include <QVBoxLayout>
#include <GProp>

struct DbDialog : QDialog, GProp {
public:
	explicit DbDialog(QWidget* parent = nullptr);
	~DbDialog();

public:
	QVBoxLayout* mainLayout_{nullptr};
	QTabWidget* tabWidget_{nullptr};

	QTableView* hostView_{nullptr};
	QTableView* logView_{nullptr};

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;

public slots:
	int exec() override;
};
