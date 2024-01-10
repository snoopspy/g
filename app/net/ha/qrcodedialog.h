#pragma once

#include <QDialog>
#include <QImage>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include <GProp>

#include "qrcode/QrCodeGenerator.h"

struct QrCodeDialog : QDialog, GProp {
	Q_OBJECT

public:
	QrCodeDialog(QWidget* parent);
	~QrCodeDialog() override;

	QLabel* lblUrl_;
	QLabel* lblQrCode_;
	QPushButton* pbGenerate_;
	QrCodeGenerator* generator_;

public slots:
	void pbGenerate_clicked(bool checked = false);

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;
};
