#pragma once

#include <QDialog>
#include <QImage>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTcpServer>
#include <QTcpSocket>
#include <QVBoxLayout>

#include <GProp>

#include "qrcode/QrCodeGenerator.h"

struct QrCodeDialog : QDialog, GProp {
	Q_OBJECT
	Q_PROPERTY(int port MEMBER port_)
	Q_PROPERTY(int sessionTimeOutSec MEMBER sessionTimeOutSec_)

public:
	static const int Port = 1234;
	int port_{Port};
	int sessionTimeOutSec_{60};

public:
	QrCodeDialog(QWidget* parent);
	~QrCodeDialog() override;

	QLabel* lblQrCode_;
	QPushButton* pbGenerate_;
	QTcpServer* tcpServer_;
	QrCodeGenerator* generator_;

protected:
	struct Session {
		static const int SessionSize = 8;
		Session();
		QString bytesToString();
		time_t created_;
		QByteArray bytes_;
	};

	struct SessionList : QList<Session> {
		SessionList(QrCodeDialog* qrCodeDialog) : qrCodeDialog_(qrCodeDialog) {}
		QrCodeDialog* qrCodeDialog_;
		void deleteOldSession();
	} sessionList_{this};

public slots:
	void pbGenerate_clicked(bool checked = false);
	void tcpServer_newConnection();
	void tcpServer_ReadyRead();

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;
};
