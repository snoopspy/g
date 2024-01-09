#pragma once

#include <QDialog>
#include <QImage>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTcpServer>
#include <QVBoxLayout>

#include <GProp>

#include "qrcode/QrCodeGenerator.h"

struct QrCodeDialog : QDialog, GProp {
	Q_OBJECT
	Q_PROPERTY(int port MEMBER port_)

public:
	int port_{1234};

public:
	QrCodeDialog(QWidget* parent);
	~QrCodeDialog() override;

	QLabel* lblQrCode_;
	QPushButton* pbGenerate_;
	QTcpServer* tcpServer_;
	QrCodeGenerator* generator_;

public slots:
	void pbGenerate_clicked(bool checked = false);

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;
};
