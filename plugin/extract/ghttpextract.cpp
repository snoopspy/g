#include "ghttpextract.h"
#include <cstring> // for strcmp
#include <QFileInfo>

// ----------------------------------------------------------------------------
// GHttpExtract::FlowItem
// ----------------------------------------------------------------------------
GHttpExtract::FlowItem::FlowItem(GHttpExtract* httpExtract) {
	httpExtract_ = httpExtract;
	clear();
}

GHttpExtract::FlowItem::~FlowItem() {
	close();
}

void GHttpExtract::FlowItem::clear() {
	state_ = None;
	fileName_ = "";
	contentLength_ = 0;
	fp_ = nullptr;
	firstHttpResponseBodySeq_ = 0;
}

void GHttpExtract::FlowItem::close() {
	if (fp_ != nullptr) {
		fclose(fp_);
		emit httpExtract_->finished(fileName_);
		clear();
	}
}

// ----------------------------------------------------------------------------
// GHttpExtract
// ----------------------------------------------------------------------------
bool GHttpExtract::doOpen() {
	if (!enabled_) return true;

	if (tcpFlowMgr_ == nullptr) {
		SET_ERR(GErr::OBJECT_IS_NULL, "tcpFlowMgr must be specified");
		return false;
	}
	tcpFlowOffset_ = tcpFlowMgr_->requestItems_.request("GFlowMgrTest_tcp", sizeof(FlowItem));
	tcpFlowMgr_->managables_.insert(this);

	currentFolder_ = folder_;
	if (!currentFolder_.endsWith(QDir::separator()))
		currentFolder_ += QDir::separator();
	currentFolder_ +=	QDateTime::currentDateTime().toString("yyMMdd-hhmmss") + QDir::separator();

	QDir dir;
	if (!dir.mkpath(currentFolder_)) {
		QString msg = QString("can not create directory(%1)").arg(currentFolder_);
		SET_ERR(GErr::FAIL, msg);
		return false;
	}

	return true;
}

bool GHttpExtract::doClose() {
	return true;
}

void GHttpExtract::tcpFlowCreated(GFlow::TcpFlowKey* key, GFlow::Value* value) {
	(void)key;
	if (!enabled_) return;
	FlowItem* flowItem = PFlowItem(value->mem(tcpFlowOffset_));
	new (flowItem) FlowItem(this);
}

void GHttpExtract::tcpFlowDeleted(GFlow::TcpFlowKey* key, GFlow::Value* value) {
	(void)key;
	if (!enabled_) return;
	FlowItem* flowItem = PFlowItem(value->mem(tcpFlowOffset_));
	flowItem->~FlowItem();
}

void GHttpExtract::write(GPacket* packet) {
	qDebug() << QString(*packet); // gilgil temp 2019.06.30

	if (!enabled_) return;
	if (!packet->tcpData_.valid()) return;

	if (tcpFlowMgr_->rVal_ != nullptr) {
		FlowItem* rFlowItem = PFlowItem(tcpFlowMgr_->rVal_->mem(tcpFlowOffset_));
		switch (rFlowItem->state_) {
			case FlowItem::None: break;
			case FlowItem::HttpRquestParsed: {

				// check start
				if (packet->tcpData_.size_ < 10) { rFlowItem->close(); return; }
				if (strncmp(pchar(packet->tcpData_.data_), "HTTP", 4) != 0) { rFlowItem->close(); return; }
				QString tcpData(QByteArray(pchar(packet->tcpData_.data_), int(packet->tcpData_.size_)));

				// check http status code
				QRegularExpressionMatch matchFirstLine = httpResponseFirstLine_.match(tcpData);
				if (!matchFirstLine.hasMatch()) { rFlowItem->close(); return; }

				// extract content-length
				QRegularExpressionMatch matchContentLength = httpResponseContentLength_.match(tcpData);
				if (!matchContentLength.hasMatch()) { rFlowItem->close(); return; }
				rFlowItem->contentLength_ = size_t(matchContentLength.captured(1).toLongLong());

				// extract file start position
				QRegularExpressionMatch bodyStart = httpResponseBodyStart_.match(tcpData);
				if (!bodyStart.hasMatch()) { rFlowItem->close(); return; }

				QString fileName = currentFolder_ + rFlowItem->fileName_;
				if (QFile::exists(fileName)) {
					QFileInfo f(fileName);
					QString suffix = QString(".") + f.suffix();
					int index = 2;
					while (true) {
						QString newFileName;
						if (suffix == ".")
							newFileName = QString("%1(%2)").arg(fileName).arg(QString::number(index));
						else
							newFileName = QString("%1(%2)%3").arg(fileName.left(fileName.size() - suffix.size())).arg(QString::number(index)).arg(suffix);
						if (!QFile::exists(newFileName)) {
							fileName = newFileName;
							break;
						}
						index++;
					}
				}

				rFlowItem->fileName_ = fileName;
				FILE* fp = fopen(qPrintable(fileName), "wb");
				if (fp == nullptr) {
					qWarning() << QString("can not open file %1").arg(fileName);
					{ rFlowItem->close(); return; }
				}

				int offset = bodyStart.capturedEnd(0);
				rFlowItem->firstHttpResponseBodySeq_ = packet->tcpHdr_->seq() + uint32_t(offset);
				int writeSize = int(packet->tcpData_.size_) - offset;
				if (writeSize > 0) {
					u_char* p = packet->tcpData_.data_ + offset;
					size_t res = fwrite(p, 1, size_t(writeSize), fp);
					if (res != size_t(writeSize)) {
						QString msg = QString("writeSize=%1 res=%2").arg(writeSize).arg(res);
						qWarning() << msg;
					}
				}

				rFlowItem->fp_ = fp;
				rFlowItem->state_ = FlowItem::HttpResponseParsed;
				return;
			}
			case FlowItem::HttpResponseParsed: {
				uint32_t currentOffset = packet->tcpHdr_->seq();
				uint32_t fileOffset = currentOffset - rFlowItem->firstHttpResponseBodySeq_;
				int res = fseek(rFlowItem->fp_, fileOffset, SEEK_SET);
				if (res == -1) {
					QString msg = QString("fseek(%1) for file %2 return -1 errno=%3").arg(fileOffset).arg(rFlowItem->fileName_).arg(errno);
					qWarning() << msg;
				}
				fwrite(packet->tcpData_.data_, 1, packet->tcpData_.size_, rFlowItem->fp_);
				if (fileOffset + packet->tcpData_.size_ >= rFlowItem->contentLength_) {
					qDebug() << QString("file(%1) write completed size=%2").arg(rFlowItem->fileName_).arg(QString::number(rFlowItem->contentLength_));
					rFlowItem->close();
					return;
				}
			}
		}
	}

	Q_ASSERT(tcpFlowMgr_->val_ != nullptr);
	FlowItem* flowItem = PFlowItem(tcpFlowMgr_->val_->mem(tcpFlowOffset_));
	if (flowItem->state_ == FlowItem::None) {

		// check start
		if (packet->tcpData_.size_ < 10) return;
		if (strncmp(pchar(packet->tcpData_.data_), "GET ", 4) != 0) return;
		QString tcpData(QByteArray(pchar(packet->tcpData_.data_), int(packet->tcpData_.size_)));

		// extract path
		QRegularExpressionMatch matchFirstLine = httpRequestFirstLine_.match(tcpData);
		if (!matchFirstLine.hasMatch()) return;
		int offset = matchFirstLine.capturedEnd(1) + 1;

		// extrace host
		QRegularExpressionMatch matchHost = httpRequestHost_.match(tcpData, offset);
		if (!matchHost.hasMatch()) return;

		QString fileName = matchHost.captured(1) + matchFirstLine.captured(1);
		fileName = fileName.replace("/", "_");
		flowItem->fileName_ = fileName;
		qDebug() << "fileName =" << flowItem->fileName_;

		flowItem->state_ = FlowItem::HttpRquestParsed;
		return;
	}
}
