#include "ggraph.h"
#include "base/sys/gthread.h"

// ----------------------------------------------------------------------------
// GGraph::Nodes
// ----------------------------------------------------------------------------
GObj* GGraph::Nodes::findNode(QString objectName) {
	for (GGraph::GObj* node: *this) {
		if (node->objectName() == objectName)
			return node;
	}
	return nullptr;
}

// ----------------------------------------------------------------------------
// GGraph::Connections
// ----------------------------------------------------------------------------
void GGraph::Connections::clear() {
	for (Connection* connection: *this) {
		delete connection;
	}
	QList::clear();
}

void GGraph::Connections::load(QJsonArray ja) {
	clear();
	GGraph* graph = dynamic_cast<GGraph*>(parent());
	Q_ASSERT(graph != nullptr);
	for (QJsonValue jv: ja) {
		QJsonObject connectionJo = jv.toObject();

		QString senderObjectName = connectionJo["sender"].toString();
		GObj* sender = graph->nodes_.findNode(senderObjectName);
		if (sender == nullptr) {
			qWarning() << QString("can not find node for %1").arg(senderObjectName);
			continue;
		}
		QString signal = connectionJo["signal"].toString();
		QString receiverObjectName = connectionJo["receiver"].toString();
		GObj* receiver = graph->nodes_.findNode(receiverObjectName);
		if (receiver == nullptr) {
			qWarning() << QString("can not find node for %1").arg(receiverObjectName);
			continue;
		}
		QString slot = connectionJo["slot"].toString();

		bool res = GObj::connect(sender, qPrintable(signal), receiver, qPrintable(slot), Qt::DirectConnection);
		if (!res) continue;

		Connection* connection = new Connection;
		connection->sender_ = sender;
		connection->signal_ = signal;
		connection->receiver_ = receiver;
		connection->slot_ = slot;
		push_back(connection);
	}
}

void GGraph::Connections::save(QJsonArray& ja) {
	for (Connection* connection: *this) {
		QJsonObject connectionJo;
		connectionJo["sender"] = connection->sender_->objectName();
		connectionJo["signal"] = connection->signal_;
		connectionJo["receiver"] = connection->receiver_->objectName();
		connectionJo["slot"] = connection->slot_;
		ja.append(connectionJo);
	}
}

// ----------------------------------------------------------------------------
// GGraph::Factory
// ----------------------------------------------------------------------------
GGraph::Factory::Factory(QObject* parent) : GObj(parent) {
}

GGraph::Factory::~Factory() {
}

// ----------------------------------------------------------------------------
// GGraph
// ----------------------------------------------------------------------------
bool GGraph::doOpen() {
	GThreadMgr::suspendStart();
	for (GObj* obj: nodes_) {
		GStateObj* stateObj = dynamic_cast<GStateObj*>(obj);
		if (stateObj != nullptr) {
			if (closeGraphOnNodeClosed_)
				QObject::connect(stateObj, &GStateObj::closed, this, &GGraph::stop);
			bool res = stateObj->open();
			if (!res) {
				QString msg;
				if (stateObj->err == nullptr) {
					msg = QString("unknown error(%1)").arg(stateObj->objectName());
					SET_ERR(GErr::Unknown, msg);
				} else {
					msg = QString("%1 (%2)").arg(stateObj->err->msg(), stateObj->metaObject()->className());
					SET_ERR(stateObj->err->code(), msg);
				}
				doClose();
				GThreadMgr::resumeStart(false);
				return false;
			}
		}
	}
	GThreadMgr::resumeStart();
	return true;
}

bool GGraph::doClose() {
	for (GObj* obj: nodes_) {
		GStateObj* stateObj = dynamic_cast<GStateObj*>(obj);
		if (stateObj != nullptr) {
			if (closeGraphOnNodeClosed_)
				QObject::disconnect(stateObj, &GStateObj::closed, this, &GGraph::stop);
			stateObj->close();
		}
	}
	return true;
}

void GGraph::clear() {
	nodes_.clear();
	connections_.clear();
}

void GGraph::start() {
	open();
}

void GGraph::stop() {
	close();
}

void GGraph::propLoad(QJsonObject jo) {
	nodes_.load(jo["nodes"].toArray());
	connections_.load(jo["connections"].toArray());
}

void GGraph::propSave(QJsonObject& jo) {
	QJsonArray nodesJa;
	nodes_.save(nodesJa);
	jo["nodes"] = nodesJa;
	QJsonArray connectionsJa;
	connections_.save(connectionsJa);
	jo["connections"] = connectionsJa;
}
