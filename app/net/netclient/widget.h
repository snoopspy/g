#ifndef WIDGET_H
#define WIDGET_H

#include <QAuthenticator>
#include <QNetworkDatagram>
#include <QNetworkProxy>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QSslPreSharedKeyAuthenticator>
#include <QSslSocket>
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
	void addText(QString msg, bool newLine);
	void showError(QString error);
	void prepareAbstractSocket(QAbstractSocket* socket);
	void prepareSslSocket(QSslSocket* socket);

public:
	const static int TcpTab = 0;
	const static int UdpTab = 1;
	const static int SslTab = 2;

public:
	QAbstractSocket* currSocket_{nullptr};
	Option option_;

public slots:
	// QIODevice
	void doAboutToClose();
	void doBytesWritten(qint64 bytes);
	void doChannelBytesWritten(int channel, qint64 bytes);
	void doChannelReadyRead(int channel);
	void doReadChannelFinished();
	void doReadyRead();

	// QAbstractSocket
	void doConnected();
	void doDisconnected();
	void doErrorOccurred(QAbstractSocket::SocketError socketError);
	void doHostFound();
	void doProxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator);
	void doStateChanged(QAbstractSocket::SocketState socketState);

	// QSslSocket
	void doAlertReceived(QSsl::AlertLevel level, QSsl::AlertType type, const QString &description);
	void doAlertSent(QSsl::AlertLevel level, QSsl::AlertType type, const QString &description);
	void doEncrypted();
	void doEncryptedBytesWritten(qint64 written);
	void doHandshakeInterruptedOnError(const QSslError &error);
	void doModeChanged(QSslSocket::SslMode mode);
	void doNewSessionTicketReceived();
	void doPeerVerifyError(const QSslError &error);
	void doPreSharedKeyAuthenticationRequired(QSslPreSharedKeyAuthenticator *authenticator);
	void doSslErrors(const QList<QSslError> &errors);

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
