#ifdef QT_GUI_LIB

#include "gg-text.h"
#include "gg-scene.h"
#include "ggraphwidget.h"

GGText::GGText(GObj* node) {
	setFlag(QGraphicsItem::ItemIsMovable);
	setFlag(QGraphicsItem::ItemIsSelectable);
	node_ = node;
#ifdef Q_OS_ANDROID
	QFont f = font();
	int pointSize = f.pointSize();
	if (pointSize > 0)
		f.setPointSize(pointSize * 3 / 2);
	else
		f.setPointSize(20);
	setFont(f);
#endif
}

GGText::~GGText() {
}

void GGText::addArrow(GGArrow* arrow) {
	arrows_.append(arrow);
}

void GGText::removeArrow(GGArrow* arrow) {
	int index = arrows_.indexOf(arrow);
	if (index != -1)
		arrows_.removeAt(index);
}

void GGText::removeArrows() {
	for (GGArrow* arrow: arrows_) {
		arrow->startText()->removeArrow(arrow);
		arrow->endText()->removeArrow(arrow);
		delete arrow;
	}
}

void GGText::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	GGScene* gsscene = dynamic_cast<GGScene*>(scene());
	Q_ASSERT(gsscene != nullptr);
	QColor color = Qt::gray;

	if (gsscene->mode() == GGScene::InsertLine) {
		bool selected = false;
		if (gsscene->dragLine_ != nullptr) {
			QList<QGraphicsItem*> startItems = gsscene->items(gsscene->dragLine_->line().p1());
			for (QGraphicsItem* item: startItems) {
				GGText* text = dynamic_cast<GGText*>(item);
				if (text != nullptr && text == this) {
					selected = true;
					break;
				}
			}
			QList<QGraphicsItem*> endItems = gsscene->items(gsscene->dragLine_->line().p2());
			for (QGraphicsItem* item: endItems) {
				GGText* text = dynamic_cast<GGText*>(item);
				if (text != nullptr && text == this) {
					selected = true;
					break;
				}
			}
			if (selected) color = Qt::white;
		}
	}

	QRectF rect = boundingRect();
	painter->fillRect(rect, color);
	painter->drawRect(rect);
	QGraphicsTextItem::paint(painter, option, widget);

	bool updated = property("updated").toBool();
	if (!updated)
		update();
	setProperty("updated", !updated);
}

#endif // QT_GUI_LIB
