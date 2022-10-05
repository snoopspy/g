// ----------------------------------------------------------------------------
//
// G Library
//
// http://gilgil.net
//
// Copyright (c) Gilbert Lee All rights reserved
//
// ----------------------------------------------------------------------------

#pragma once

#include "gextract.h"
#include "net/manage/gtcpflowmgr.h"
#include <QRegularExpression>

// ----------------------------------------------------------------------------
// GHttpExtract
// ----------------------------------------------------------------------------
struct G_EXPORT GHttpExtract : GExtract, GTcpFlowMgr::Managable {
	Q_OBJECT
	Q_PROPERTY(QString folder MEMBER folder_)
	Q_PROPERTY(GObjPtr tcpFlowMgr READ getTcpFlowMgr WRITE setTcpFlowMgr)

public:
	GObjPtr getTcpFlowMgr() { return tcpFlowMgr_; }
	void setTcpFlowMgr(GObjPtr value) { tcpFlowMgr_ = dynamic_cast<GTcpFlowMgr*>(value.data()); }

public:
	QString folder_{"extract"};
	GTcpFlowMgr* tcpFlowMgr_{nullptr};

public:
	// --------------------------------------------------------------------------
	// FlowItem
	// --------------------------------------------------------------------------
	struct FlowItem {
		GHttpExtract* httpExtract_;
		enum State {
			None,
			HttpRquestParsed,
			HttpResponseParsed,
		} state_;

		QString fileName_;
		size_t contentLength_;
		FILE* fp_;
		uint32_t firstHttpResponseBodySeq_;

		FlowItem(GHttpExtract* httpExtract);
		virtual ~FlowItem();

		void clear();
		void close();
	};
	typedef FlowItem *PFlowItem;
	// --------------------------------------------------------------------------

public:
	Q_INVOKABLE GHttpExtract(QObject* parent = nullptr) : GExtract(parent) {}
	~GHttpExtract() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	size_t tcpFlowOffset_{0};
	QString currentFolder_;

protected:
	QRegularExpression httpRequestFirstLine_{"^GET ([^ ]+) HTTP/1.1\r\n"};
	QRegularExpression httpRequestHost_{"\r\nHost: ([^\r]+)\r\n"};
	QRegularExpression httpResponseFirstLine_{"^HTTP/1.1 200 "};
	QRegularExpression httpResponseContentLength_{"\r\nContent-Length: ([0-9]+)"};
	QRegularExpression httpResponseBodyStart_{"\r\n\r\n"};

public:
	// GTcpFlowMgr::Managable
    void tcpFlowCreated(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue);
    void tcpFlowDeleted(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue);

public slots:
	void write(GPacket* packet);

signals:
	void written(GPacket* packet);
	void finished(QString fileName);
};
