#ifndef WIDGET_H
#define WIDGET_H

#include <QTcpServer>
#include <QUdpSocket>
#include <QSslServer>
#include <QWidget>
#include "option.h"

// ----------------------------------------------------------------------------
// Widget
// ----------------------------------------------------------------------------
namespace Ui {
	class Widget;
}

class Widget : public QWidget {
	Q_OBJECT

public:
	explicit Widget(QWidget *parent = 0);
	~Widget();

public:
	void initControl();
	void finiControl();
	void loadControl();
	void saveControl();
	void setControl();

public:
	QTcpServer tcpServer_;
	QUdpSocket udpSocket_;
	QSslServer sslServer_;
	QAbstractSocket* netClient_{nullptr};
	Option option_;

private slots:
	void doNewConnection();

	void doConnected();
	void doDisconnected();
	void doErrorOccured(QAbstractSocket::SocketError socketError);
	void doStateChanged(QAbstractSocket::SocketState socketState);
	void doReadyRead();

public:
	void showOption(NetServer* netServer);

private slots:
	void on_pbOpen_clicked();

	void on_pbClose_clicked();

	void on_pbClear_clicked();

	void on_tbTcpAdvance_clicked();

	void on_tbUdpAdvance_clicked();

	void on_tbSslAdvanced_clicked();

	void on_pbSend_clicked();

private:
	Ui::Widget *ui;
};

#endif // WIDGET_H
