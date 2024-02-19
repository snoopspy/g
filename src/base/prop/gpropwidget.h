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

#include <QVBoxLayout>
#include <QToolBar>
#include "base/gobj.h"
#include "base/widget/gtreewidget.h"

// ----------------------------------------------------------------------------
// GPropWidget
// ----------------------------------------------------------------------------
struct G_EXPORT GPropWidget : public QWidget, GProp {
	GPropWidget(QWidget *parent = nullptr);
	~GPropWidget() override;

public:
	void init();
	QObject* object() { return object_; }
	void setObject(QObject* object);
	void update();
	void clear();

	// ----- gilgil temp 2020.05.21 -----
	/*
public:
	QAction* actionOpen_;
	QAction* actionClose_;
	*/
	// ----------------------------------

public:
	QVBoxLayout* mainLayout_;
	// QToolBar* toolBar_; // gilgil temp 2020.05.21
	GTreeWidget* treeWidget_;

protected:
	QObject* object_{nullptr};

public slots:
	void setControl();
	// ----- gilgil temp 2020.05.21 -----
	/*
public slots:
	void actionOpenTriggered(bool);
	void actionCloseTriggered(bool);
	*/
	// ----------------------------------

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;
};

#endif // QT_GUI_LIB
