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

#ifdef QT_GUI_LIB

#include <QAction>
#include <QFileDialog>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QToolBar>
#include <QVBoxLayout>

#include <GSplitter>
#include <GTreeWidget>
#include <GPlainTextEdit>

#include "base/prop/gpropwidget.h"
#include "gg-scene.h"

// ----------------------------------------------------------------------------
// GGraphWidget
// ----------------------------------------------------------------------------
struct G_EXPORT GGraphWidget : QWidget, GProp {
public:
	GGraphWidget(QWidget *parent = nullptr);
	~GGraphWidget() override;

public:
	void init();
	GGraph* graph() { return graph_; }
	void setGraph(GGraph* graph);
	void update();

public:
	void clear();
	void loadGraph(QJsonObject jo);
	void saveGraph(QJsonObject& jo);

protected:
	void updateFactory(GGraph::Factory::Item* item, QTreeWidgetItem* parent);

public:
	GObj* createInstance(QString className);
	GObj* createNodeIfItemNodeSelected();

protected:
	GGraph* graph_{nullptr};
	QString fileName_;
	QFileDialog fileDialog_{this};

public:
	bool toLowerFirstCharacter_{true};
	QStringList removePrefixNames_{"G"};
	QStringList ignoreSignalNames_{
		"destroyed(QObject*)",
		"destroyed()",
		"objectNameChanged(QString)",
		"opened()",
		"closed()"};
	QStringList ignoreSlotNames_{
		"deleteLater()",
		"open()",
		"close()"};

public:
	QAction* actionStartStop_;
	QAction* actionNewFile_;
	QAction* actionOpenFile_;
	QAction* actionSaveFile_;
	QAction* actionSaveFileAs_;
	QAction* actionEdit_;
	QAction* actionLink_;
	QAction* actionDelete_;
	QAction* actionAbout_;

	QVBoxLayout*  mainLayout_;
	QToolBar*     toolBar_;
	QTabWidget*   tabWidget_;
	GSplitter*      midSplitter_;
	GSplitter*        midLeftSplitter_;
	GTreeWidget*        factoryWidget_;
	GPropWidget*        propWidget_;
	GGScene*            scene_;
	QGraphicsView*    graphView_;
	GPlainTextEdit* plainTextEdit_;

public slots:
	void setControl();
	void stop();
	void writeLog(QString msg);

public slots:
	void actionStartStopTriggered(bool);
	void actionNewFileTriggered(bool);
	void actionOpenFileTriggered(bool);
	void actionSaveFileTriggered(bool);
	void actionSaveFileAsTriggered(bool);
	void actionEditTriggered(bool);
	void actionLinkTriggered(bool);
	void actionDeleteTriggered(bool);
	void actionAboutTriggered(bool);
	void factoryWidgetClicked(const QModelIndex&);

protected:
	static QRect getScreenRect();

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;
};

#endif // QT_GUI_LIB
