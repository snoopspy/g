#ifdef QT_GUI_LIB

#include "ggraphwidget.h"

#include "base/gjson.h"
#include "base/log/glogmanager.h"
#include "base/log/glogqobject.h"

// ----------------------------------------------------------------------------
// GGraphWidget
// ----------------------------------------------------------------------------
GGraphWidget::GGraphWidget(QWidget *parent) : QWidget(parent) {
	init();
	setControl();
}

GGraphWidget::~GGraphWidget() {
	actionStop_->trigger();
	clear();
}

void GGraphWidget::init() {
	resize(QSize(640, 480));

	QStringList filters{"snoopspy files - *.ss(*.ss)", "any files - *(*)"};
	fileDialog_.setNameFilters(filters);
	fileDialog_.setDefaultSuffix("ss");
	fileDialog_.setViewMode(QFileDialog::Detail);

	(actionNewFile_ = new QAction(this))->setText("New");
	actionNewFile_->setIcon(QIcon(":/img/new.png"));

	(actionOpenFile_ = new QAction(this))->setText("Open");
	actionOpenFile_->setIcon(QIcon(":/img/open.png"));

	(actionSaveFile_ = new QAction(this))->setText("Save");
	actionSaveFile_->setIcon(QIcon(":/img/save.png"));

	(actionSaveFileAs_ = new QAction(this))->setText("Save As");
	actionSaveFileAs_->setIcon(QIcon(":/img/saveas.png"));

	(actionStart_ = new QAction(this))->setText("Start");
	actionStart_->setIcon(QIcon(":/img/start.png"));

	(actionStop_ = new QAction(this))->setText("Stop");
	actionStop_->setIcon(QIcon(":/img/stop.png"));

	(actionEdit_ = new QAction(this))->setText("Edit");
	actionEdit_->setIcon(QIcon(":/img/edit.png"));

	(actionLink_ = new QAction(this))->setText("Link");
	actionLink_->setIcon(QIcon(":/img/link.png"));

	(actionDelete_ = new QAction(this))->setText("Delete");
	actionDelete_->setIcon(QIcon(":/img/delete.png"));

	(actionAbout_ = new QAction(this))->setText("About");
	actionAbout_->setIcon(QIcon(":/img/about.png"));

	mainLayout_ = new QVBoxLayout(this);
	mainLayout_->setMargin(0);
	mainLayout_->setSpacing(0);

	toolBar_ = new QToolBar(this);
	toolBar_->setToolButtonStyle(Qt::ToolButtonIconOnly);

	tabWidget_ = new QTabWidget(this);
	tabWidget_->setTabPosition(QTabWidget::South);

	midSplitter_ = new GSplitter(Qt::Horizontal, this);
	midLeftSplitter_ = new GSplitter(Qt::Vertical, this);
	factoryWidget_ = new GTreeWidget(this);
	factoryWidget_->setHeaderLabel("node");
	propWidget_ = new GPropWidget(this);
	scene_ = new GGScene(this);
	graphView_ = new QGraphicsView(this);
	graphView_->setRenderHint(QPainter::Antialiasing);
	graphView_->setAcceptDrops(true);
	graphView_->setScene(scene_);

	plainTextEdit_ = new GPlainTextEdit(this);
	plainTextEdit_->setReadOnly(true);
	plainTextEdit_->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

	GLogManager& logManager = GLogManager::instance();
	GLogQObject* log = new GLogQObject;
	QObject::connect(log, &GLogQObject::writeLogRequred, this, &GGraphWidget::writeLog);
	logManager.push_back(log);

	mainLayout_->addWidget(toolBar_, 0);
	mainLayout_->addWidget(tabWidget_, 1);
	this->setLayout(mainLayout_);

	tabWidget_->addTab(midSplitter_, "Graph");
	tabWidget_->addTab(plainTextEdit_, "Log");

	midSplitter_->addWidget(midLeftSplitter_);
	midSplitter_->addWidget(graphView_);
	midSplitter_->setStretchFactor(0, 0);
	midSplitter_->setStretchFactor(1, 1);

	midLeftSplitter_->addWidget(factoryWidget_);
	midLeftSplitter_->addWidget(propWidget_);
	midLeftSplitter_->setStretchFactor(0, 0);
	midLeftSplitter_->setStretchFactor(1, 1);

	toolBar_->addAction(actionNewFile_);
	toolBar_->addAction(actionOpenFile_);
	toolBar_->addAction(actionSaveFile_);
	toolBar_->addAction(actionSaveFileAs_);
	toolBar_->addSeparator();
	toolBar_->addAction(actionStart_);
	toolBar_->addAction(actionStop_);
	toolBar_->addSeparator();
	toolBar_->addAction(actionEdit_);
	toolBar_->addAction(actionLink_);
	toolBar_->addAction(actionDelete_);
	toolBar_->addSeparator();
	toolBar_->addAction(actionAbout_);

	QObject::connect(actionNewFile_, &QAction::triggered, this, &GGraphWidget::actionNewFileTriggered);
	QObject::connect(actionOpenFile_, &QAction::triggered, this, &GGraphWidget::actionOpenFileTriggered);
	QObject::connect(actionSaveFile_, &QAction::triggered, this, &GGraphWidget::actionSaveFileTriggered);
	QObject::connect(actionSaveFileAs_, &QAction::triggered, this, &GGraphWidget::actionSaveFileAsTriggered);
	QObject::connect(actionStart_, &QAction::triggered, this, &GGraphWidget::actionStartTriggered);
	QObject::connect(actionStop_, &QAction::triggered, this, &GGraphWidget::actionStopTriggered);
	QObject::connect(actionEdit_, &QAction::triggered, this, &GGraphWidget::actionEditTriggered);
	QObject::connect(actionLink_, &QAction::triggered, this, &GGraphWidget::actionLinkTriggered);
	QObject::connect(actionDelete_, &QAction::triggered, this, &GGraphWidget::actionDeleteTriggered);
	QObject::connect(actionAbout_, &QAction::triggered, this, &GGraphWidget::actionAboutTriggered);

	QObject::connect(factoryWidget_, &QTreeWidget::clicked, this, &GGraphWidget::factoryWidgetClicked);
	QObject::connect(scene_, &GGScene::selectionChanged, this, &GGraphWidget::setControl);

#ifdef Q_OS_ANDROID
	toolBar_->setIconSize(QSize(96, 96));
	QRect rect = getScreenRect();
	midSplitter_->setSizes(QList<int>{ rect.width() / 2, rect.width() / 2});
	midLeftSplitter_->setSizes(QList<int>{ rect.height() / 2, rect.height() / 2});
#endif // Q_OS_ANDROID
}

void GGraphWidget::setGraph(GGraph* graph) {
	if (graph == graph_) return;
	graph_ = graph;
	QObject::connect(graph, &GStateObj::closed, this, &GGraphWidget::stop);
	update();
	setControl();
}

void GGraphWidget::update() {
	factoryWidget_->clear();
	Q_ASSERT(graph_ != nullptr);
	GGraph::Factory* factory = graph_->factory();
	Q_ASSERT(factory!= nullptr);
	for (GGraph::Factory::Item* item: factory->items_) {
		updateFactory(item, nullptr);
	}
	//factoryWidget_->expandAll();
}

void GGraphWidget::clear() {
	fileName_ = "";
	graph_->clear();
	scene_->clear();
}

void GGraphWidget::loadGraph(QJsonObject jo) {
	graph_->propLoad(jo);

	QJsonArray nodeJa = jo["nodes"].toArray();
	for (QJsonValue jv: nodeJa) {
		QJsonObject nodeJo = jv.toObject();
		QString objectName = nodeJo["objectName"].toString();
		if (objectName == "") {
			qWarning() << "objectName is empty" << nodeJo;
			continue;
		}
		GObj* node = graph_->nodes_.findNode(objectName);
		if (node == nullptr) {
			qWarning() << "node is null" << nodeJo;
			continue;
		}
		qreal x = nodeJo["_x"].toVariant().toReal();
		qreal y = nodeJo["_y"].toVariant().toReal();
		QPointF pos(x, y);
		scene_->createText(node, pos);
	}

	for (GGraph::Connection* connection: graph_->connections_) {
		QString startNodeName = connection->sender_->objectName();
		QString endNodeName = connection->receiver_->objectName();
		scene_->createArrow(startNodeName, endNodeName, connection);
	}
}

void GGraphWidget::saveGraph(QJsonObject& jo) {
	graph_->propSave(jo);

	QJsonArray nodeJa = jo["nodes"].toArray();
	for (int i = 0; i < nodeJa.count(); i++) {
		QJsonObject nodeJo = nodeJa.at(i).toObject();
		QString objectName = nodeJo["objectName"].toString();
		if (objectName == "") {
			qWarning() << "objectName is empty" << nodeJo;
			continue;
		}
		GGText* text = scene_->findTextByObjectName(objectName);
		if (text == nullptr) {
			qWarning() << "text is null" << nodeJo;
			continue;
		}
		nodeJo["_x"] = text->pos().x();
		nodeJo["_y"] = text->pos().y();
		nodeJa.removeAt(i);
		nodeJa.insert(i, nodeJo);
	}
	jo["nodes"] = nodeJa;
}

void GGraphWidget::updateFactory(GGraph::Factory::Item* item, QTreeWidgetItem* parent) {
	QTreeWidgetItem* newWidgetItem;
	if (parent != nullptr)
		newWidgetItem = new QTreeWidgetItem(parent);
	else
		newWidgetItem = new QTreeWidgetItem(factoryWidget_);

	newWidgetItem->setText(0, item->displayName_);
	QVariant v = QVariant::fromValue(pvoid(item));
	newWidgetItem->setData(0, Qt::UserRole, v);

	GGraph::Factory::ItemCategory* category = dynamic_cast<GGraph::Factory::ItemCategory*>(item);
	if (category != nullptr) {
		for (GGraph::Factory::Item* child: category->items_) {
			updateFactory(child, newWidgetItem);
		}
		return;
	}
}

GObj* GGraphWidget::createInstance(QString className) {
	GObj* node = GObj::createInstance(className, &graph_->nodes_);
	if (node == nullptr) return nullptr;

	QString objectName = node->metaObject()->className();
	for (QString removePrefixName: removePrefixNames_) {
		if (objectName.startsWith(removePrefixName))
			objectName = objectName.mid(removePrefixName.length());
	}
	if (objectName == "") {
		qWarning() << QString("objectName is empty for (%1)").arg(className);
		delete node;
		return nullptr;
	}
	if (toLowerFirstCharacter_) {
		char first = objectName.at(0).toLatin1();
		if (first >= 'A' && first <= 'Z') {
			first = first + 'a' - 'A';
			objectName = QString(first) + objectName.mid(1);
		}
	}

	int suffix = 1;
	while (true) {
		QString _objectName = objectName + QString::number(suffix);
		bool isExist = false;
		for (GObj* node: graph()->nodes_) {
			if (node->objectName() == _objectName) {
				isExist = true;
				break;
			}
		}
		if (!isExist) break;
		suffix++;
	}
	objectName = objectName + QString::number(suffix);
	node->setObjectName(objectName);

	return node;
}

GObj* GGraphWidget::createNodeIfItemNodeSelected() {
	QList<QTreeWidgetItem*> widgetItems = factoryWidget_->selectedItems();
	if (widgetItems.count() == 0)
		return nullptr;
	QTreeWidgetItem* widgetItem = widgetItems.at(0);
	QVariant variant = widgetItem->data(0, Qt::UserRole);
	void* p = qvariant_cast<void*>(variant);
	GGraph::Factory::Item* item = dynamic_cast<GGraph::Factory::Item*>(GGraph::Factory::PItem(p));
	GGraph::Factory::ItemNode* itemNode = dynamic_cast<GGraph::Factory::ItemNode*>(item);
	if (itemNode == nullptr)
		return nullptr;
	QString className = itemNode->className_;
	GObj* node = createInstance(className);
	if (node == nullptr) {
		qWarning() << QString("createInstance failed for (%1)").arg(className);
		return nullptr;
	}
	node->setParent(graph_);
	graph_->nodes_.push_back(node);
	return node;
}

void GGraphWidget::propLoad(QJsonObject jo) {
	jo["rect"] >> GJson::rect(this);

	QJsonObject splitter = jo["splitter"].toObject();
	splitter["mid"] >> GJson::splitterSizes(midSplitter_);
	splitter["left"] >> GJson::splitterSizes(midLeftSplitter_);
	splitter["prop"] >> *propWidget_;

	QString treeExpanded = jo["treeExpanded"].toString();
	for (int i = 0; i < factoryWidget_->topLevelItemCount(); i++) {
		QTreeWidgetItem* twi = factoryWidget_->topLevelItem(i);
		if (treeExpanded.length() <= i) break;
		QChar e = treeExpanded.at(i);
		twi->setExpanded(e == 'E');
	}
	tabWidget_->setCurrentIndex(jo["tabIndex"].toInt());

	toLowerFirstCharacter_ = jo["toLowerFirstCharacter"].toBool();
	removePrefixNames_ = jo["removePrefixNames"].toString().split(",");
	ignoreSignalNames_ = jo["ignoreSignalNames"].toString().split(",");
	ignoreSlotNames_ = jo["ignoreSlotNames"].toString().split(",");

	loadGraph(jo["graph"].toObject());
}

void GGraphWidget::propSave(QJsonObject& jo) {
	jo["rect"] << GJson::rect(this);

	QJsonObject splitter;
	splitter["mid"] << GJson::splitterSizes(midSplitter_);
	splitter["left"] << GJson::splitterSizes(midLeftSplitter_);
	splitter["prop"] << *propWidget_;
	jo["splitter"] = splitter;

	QString treeExpanded;
	for (int i = 0; i < factoryWidget_->topLevelItemCount(); i++) {
		QTreeWidgetItem* twi = factoryWidget_->topLevelItem(i);
		treeExpanded += twi->isExpanded() ? 'E' : 'C';
	}
	jo["treeExpanded"] = treeExpanded;
	jo["tabIndex"] = tabWidget_->currentIndex();

	jo["toLowerFirstCharacter"] = toLowerFirstCharacter_;
	jo["removePrefixNames"] = removePrefixNames_.join(",");
	jo["ignoreSignalNames"] = ignoreSignalNames_.join(",");
	jo["ignoreSlotNames"] = ignoreSlotNames_.join(",");

	QJsonObject graphJo;
	saveGraph(graphJo);
	jo["graph"] = graphJo;
}

void GGraphWidget::setControl() {
	QString title = "SnoopSpy";
	if (fileName_ != "") {
		QFileInfo fi(fileName_);
		title = fi.completeBaseName();
	}
	if (title != "")
		setWindowTitle(title);

	GGScene::Mode mode = scene_->mode();

	bool active = false;
	if (graph_ != nullptr)
		active = graph_->active();

	actionNewFile_->setEnabled(!active);
	actionOpenFile_->setEnabled(!active);
	actionSaveFile_->setEnabled(!active && fileName_ != "");
	actionSaveFileAs_->setEnabled(!active);
	actionStart_->setEnabled(!active);
	actionStop_->setEnabled(active);
	actionEdit_->setEnabled(!active && mode != GGScene::MoveItem);
	actionLink_->setEnabled(!active && mode != GGScene::InsertLine);
	actionAbout_->setEnabled(!active);

	factoryWidget_->setEnabled(!active);
	propWidget_->setEnabled(!active);
	graphView_->setEnabled(!active);

	bool selected = scene_->selectedItems().count() > 0;
	actionDelete_->setEnabled(!active && selected);
	GObj* selectedObj = nullptr;
	if (selected) {
		QGraphicsItem* item = scene_->selectedItems().first();
		GGText* text = dynamic_cast<GGText*>(item);
		if (text != nullptr)
			selectedObj = dynamic_cast<GObj*>(text->node_);
	}
	propWidget_->setObject(selectedObj);
}

void GGraphWidget::stop() {
	graph_->close();
	setControl();
}

#include <QScrollBar>
void GGraphWidget::writeLog(QString msg) {
	msg = msg.mid(7); // Omit year and date
	plainTextEdit_->insertPlainText(msg);
	plainTextEdit_->verticalScrollBar()->setValue(plainTextEdit_->verticalScrollBar()->maximum());
}

void GGraphWidget::actionNewFileTriggered(bool) {
	clear();
	setControl();
}

void GGraphWidget::actionOpenFileTriggered(bool) {
	fileDialog_.setAcceptMode(QFileDialog::AcceptOpen);
	fileDialog_.setFileMode(QFileDialog::ExistingFile);
	if (fileDialog_.exec() == QDialog::Accepted) {
		clear();
		fileName_ = fileDialog_.selectedFiles().first();
		QJsonObject jo = GJson::loadFromFile(fileName_);
		loadGraph(jo);
		setControl();
	}
}

void GGraphWidget::actionSaveFileTriggered(bool) {
	QJsonObject jo;
	saveGraph(jo);
	GJson::saveToFile(jo, fileName_);
	setControl();
}

void GGraphWidget::actionSaveFileAsTriggered(bool) {
	fileDialog_.setAcceptMode(QFileDialog::AcceptSave);
	fileDialog_.setFileMode(QFileDialog::AnyFile);
	if (fileDialog_.exec() == QDialog::Accepted) {
		fileName_ = fileDialog_.selectedFiles().first();
		actionSaveFileTriggered(false);
	}
}

void GGraphWidget::actionStartTriggered(bool) {
	plainTextEdit_->clear();
	bool res = graph_->open();
	if (!res) {
		QString msg = graph_->err->msg();
		QMessageBox::warning(nullptr, "Error", msg);
	}
	setControl();
}

void GGraphWidget::actionStopTriggered(bool) {
	graph_->close();
	setControl();
}

void GGraphWidget::actionEditTriggered(bool) {
	scene_->setMode(GGScene::MoveItem);
	setControl();
}

void GGraphWidget::actionLinkTriggered(bool) {
	scene_->setMode(GGScene::InsertLine);
	setControl();
}

void GGraphWidget::actionDeleteTriggered(bool) {
	QList<QGraphicsItem*> selectedItems = QList<QGraphicsItem*>(scene_->selectedItems());
	for (QGraphicsItem* item: selectedItems) {
		GGText* text = dynamic_cast<GGText*>(item);
		if (text != nullptr) {
			GObj* node = text->node_;
			for (QGraphicsItem* item: scene_->items()) {
				GGArrow* arrow = dynamic_cast<GGArrow*>(item);
				if (arrow == nullptr) continue;
				if (arrow->startText() == text || arrow->endText() == text) {
					GGraph::Connection* connection = arrow->connection_;
					GObj::disconnect(
						connection->sender_, qPrintable(connection->signal_),
						connection->receiver_, qPrintable(connection->slot_));
					graph_->connections_.removeOne(connection);
					delete connection;
					delete arrow;
				}
			}
			graph_->nodes_.removeOne(node);
			delete node;
			delete text;
		}

		GGArrow* arrow = dynamic_cast<GGArrow*>(item);
		if (arrow != nullptr) {
			GGraph::Connection* connection = arrow->connection_;
			GObj::disconnect(
				connection->sender_, qPrintable(connection->signal_),
				connection->receiver_, qPrintable(connection->slot_));
			graph_->connections_.removeOne(connection);
			delete arrow;
		}
	}
	setControl();
}

void GGraphWidget::actionAboutTriggered(bool) {
	QMessageBox::information(nullptr, "About", QString("SnoopSpy ") + G::version());
}

void GGraphWidget::factoryWidgetClicked(const QModelIndex&) {
	if (factoryWidget_->selectedItems().isEmpty()) return;
	scene_->setMode(GGScene::InsertItem);
}

#include <QApplication>
#include <QScreen>
QRect GGraphWidget::getScreenRect() {
	QGuiApplication* a = dynamic_cast<QGuiApplication*>(QApplication::instance());
	if (a == nullptr) {
		qCritical() << "application is nullptr";
		return QRect();
	}
	if (a->screens().count() == 0) {
		qCritical() << "screen count is zero";
		return QRect();
	}

	QScreen* screen = *(a->screens().begin());
	if (a == nullptr) {
		qCritical() << "screen is nullptr";
		return QRect();
	}

	QRect rect = screen->geometry();
	return rect;
	rect = screen->geometry();
}

#endif // QT_GUI_LIB
