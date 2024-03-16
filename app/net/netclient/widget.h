#ifndef WIDGET_H
#define WIDGET_H

#include <QSslSocket>
#include <QTcpSocket>
#include <QUdpSocket>
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
	QTcpSocket tcpSocket_;
	QUdpSocket udpSocket_;
	QSslSocket sslSocket_;
	QAbstractSocket* netClient_{nullptr};
	Option option_;

private slots:
	void connected();
	void disconnected();
	void errorOccurred(QAbstractSocket::SocketError socketError);
	void stateChanged(QAbstractSocket::SocketState socketState);
	void readyRead();

public:
	void showOption(NetClient* netClient);

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
