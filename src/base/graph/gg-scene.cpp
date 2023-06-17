#ifdef QT_GUI_LIB

#include "gg-scene.h"
#include "ggraphwidget.h"

GGScene::GGScene(QObject *parent) : QGraphicsScene(parent) {
	mode_ = MoveItem;
	graphWidget_ = dynamic_cast<GGraphWidget*>(parent);
	Q_ASSERT(graphWidget_ != nullptr);
	signalSlotForm_ = nullptr;
	dragLine_ = nullptr;
}

GGScene::~GGScene() {
	while (items().count() > 0) {
		QGraphicsItem* item = items().first();
		if (item->type() == GGArrow::Type) {
			GGArrow *arrow = (GGArrow*)(item);
			arrow->startText()->removeArrow(arrow);
			arrow->endText()->removeArrow(arrow);
			delete item;
		};
		if (item->type() == GGText::Type) {
			GGText* text = (GGText*)item;
			text->removeArrows();
			delete text;
		}
	}
	if (signalSlotForm_ != nullptr) {
		delete signalSlotForm_;
		signalSlotForm_ = nullptr;
	}
}

void GGScene::clear() {
	QGraphicsScene::clear();
}

GGText* GGScene::createText(GObj* node, QPointF pos) {
	GGText* text = new GGText(node);
	text->setPlainText(node->objectName());
	addItem(text);
	text->setPos(pos);
	return text;
}

GGArrow* GGScene::createArrow(GGText* startText, GGText* endText, GGraph::Connection* connection) {
	GGArrow* arrow = new GGArrow(startText, endText, connection);
	arrow->setColor(Qt::black);
	startText->addArrow(arrow);
	endText->addArrow(arrow);
	addItem(arrow);
	arrow->updatePosition();
	return arrow;
}

GGArrow* GGScene::createArrow(QString startNodeName, QString endNodeName, GGraph::Connection* connection) {
	GGText* startText = findTextByObjectName(startNodeName);
	if (startText == nullptr) return nullptr;
	GGText* endText = findTextByObjectName(endNodeName);
	if (endText == nullptr) return nullptr;
	return createArrow(startText, endText, connection);
}

GGText* GGScene::findTextByObjectName(QString objectName) {
	int _count = this->items().count();
	for (int i = 0; i < _count; i++) {
		QGraphicsItem* item = this->items().at(i);
		GGText* res = dynamic_cast<GGText*>(item);
		if (res == nullptr) continue;
		if (res->node_->objectName() == objectName)
			return res;
	}
	qWarning() << QString("can not find for '%1'").arg(objectName);
	return nullptr;
}

void GGScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	switch (mode_) {
		case InsertItem: {
			GObj* newNode = graphWidget_->createNodeIfItemNodeSelected();
			if (newNode != nullptr) {
				createText(newNode, event->scenePos());
			}
			setMode(MoveItem);
		}
			break;
		case MoveItem:
			break;
		case InsertLine:
			dragLine_ = new QGraphicsLineItem(QLineF(event->scenePos(), event->scenePos()));
#ifdef Q_OS_ANDROID
	qreal width = 20;
#else // Q_OS_ANDROID
	qreal width = 5;
#endif // Q_OS_ANDROID
			dragLine_->setPen(QPen(Qt::black, width, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
			addItem(dragLine_);
			break;
	}
	QGraphicsScene::mousePressEvent(event);
}

void GGScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
	switch (mode_) {
		case InsertItem:
			break;
		case MoveItem:
			break;
		case InsertLine:
			if (dragLine_ != nullptr && signalSlotForm_ == nullptr) {
				QLineF line(dragLine_->line().p1(), event->scenePos());
				dragLine_->setLine(line);
				return;
			}
			break;
	}
	QGraphicsScene::mouseMoveEvent(event);
}

void GGScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
	switch (mode_) {
		case InsertItem:
			break;
		case MoveItem:
			break;
		case InsertLine:
			if (dragLine_ != nullptr) {
				QList<QGraphicsItem*> startItems = items(dragLine_->line().p1());
				if (startItems.count() && startItems.first() == dragLine_)
					startItems.removeFirst();
				QList<QGraphicsItem *> endItems = items(dragLine_->line().p2());
				if (endItems.count() && endItems.first() == dragLine_)
					endItems.removeFirst();

				if (startItems.count() > 0 && endItems.count() > 0 && startItems.first() != endItems.first()) {
					GGText *startText = (GGText*)startItems.first();
					GGText *endText = (GGText*)endItems.first();
					GGraph* graph = graphWidget_->graph();
					Q_ASSERT(graph != nullptr);

					QStringList _signalList = startText->node_->signalList();
					for (QString& name: graphWidget_->ignoreSignalNames_) {
						_signalList.removeAll(name);
					}

					QStringList _slotList = endText->node_->slotList();
					for (QString& name: graphWidget_->ignoreSlotNames_) {
						_slotList.removeAll(name);
					}

					if (signalSlotForm_ == nullptr)
						signalSlotForm_ = new GGSignalSlotForm((QWidget*)this->parent());
					signalSlotForm_->lwSignalList_->clear();
					signalSlotForm_->lwSignalList_->addItems(_signalList);
					signalSlotForm_->lwSlotList_->clear();
					signalSlotForm_->lwSlotList_->addItems(_slotList);
#ifdef Q_OS_ANDROID
					signalSlotForm_->showMaximized();
#else
					signalSlotForm_->show();
#endif
					signalSlotForm_->exec();
					if (signalSlotForm_->result() == QDialog::Accepted) {
						QString signal = signalSlotForm_->lwSignalList_->selectedItems().first()->text();
						QString slot = signalSlotForm_->lwSlotList_->selectedItems().first()->text();

						bool res = GObj::connect(
							startText->node_, qPrintable(signal),
							endText->node_, qPrintable(slot),
							Qt::DirectConnection);
						if (!res) {
							QString msg = QString("Can not connect %1:%2 > %3:%4").arg(
								startText->node_->objectName(), signal,
								endText->node_->objectName(), slot);
							QMessageBox::warning(nullptr, "Error", msg);
							return;
						}
						GGraph::Connection* connection = new GGraph::Connection;
						connection->sender_ = startText->node_;
						connection->signal_ = signal;
						connection->receiver_ = endText->node_;
						connection->slot_ = slot;
						graphWidget_->graph()->connections_.push_back(connection);

						createArrow(startText, endText, connection);
					}
					delete signalSlotForm_;
					signalSlotForm_ = nullptr;
				}
				removeItem(dragLine_);
				delete dragLine_;
				dragLine_ = nullptr;
			}
			break;
	}

	QGraphicsScene::mouseReleaseEvent(event);
}

#endif // QT_GUI_LIB
