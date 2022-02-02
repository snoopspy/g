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

#include "base/gstateobj.h"
#ifdef Q_OS_ANDROID
#include "net/demon/gdemonclient.h"
#endif
#include <QProcess>

// ----------------------------------------------------------------------------
// GCommandItem
// ----------------------------------------------------------------------------
struct G_EXPORT GCommandItem : public GObj {
	Q_OBJECT
	Q_PROPERTY(QStringList commands MEMBER commands_)
	Q_PROPERTY(GCommandType commandType MEMBER commandType_)
	Q_ENUMS(GCommandType)

public:
	enum GCommandType {
		Execute,
		StartStop,
		StartDetach
	};

	QStringList commands_;
	GCommandType commandType_{Execute};

	Q_INVOKABLE GCommandItem(QObject* parent = nullptr);
	Q_INVOKABLE GCommandItem(QObject* parent, QStringList commands, GCommandType commandType = Execute);
	~GCommandItem() override;

	QList<void*> processList_; // int64_t is pid_t or QProcess*
};
typedef GCommandItem* PCommandItem;

// ----------------------------------------------------------------------------
// GCommand
// ----------------------------------------------------------------------------
struct G_EXPORT GCommand : GStateObj {
	Q_OBJECT
	Q_PROPERTY(GObjRefArrayPtr openCommands READ getOpenCommands)
	Q_PROPERTY(GObjRefArrayPtr closeCommands READ getCloseCommands)

public:
	GObjRefArrayPtr getOpenCommands() { return &openCommands_; }
	GObjRefArrayPtr getCloseCommands() { return &closeCommands_; }

public:
	Q_INVOKABLE GCommand(QObject* parent = nullptr) : GStateObj(parent) {}
	~GCommand() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	bool split(QString command, QString& program, QStringList &arguments);
	bool stopCommands();

protected:
	virtual bool cmdExecute(QString command);
	virtual void* cmdStart(QString command);
	virtual bool cmdStop(void* pid);
	virtual void* cmdStartDetached(QString command);

public:
	GObjRefArray<GCommandItem> openCommands_;
	GObjRefArray<GCommandItem> closeCommands_;

#ifdef Q_OS_ANDROID
protected:
	GDemonClient* demonClient_{nullptr};
#endif
};
