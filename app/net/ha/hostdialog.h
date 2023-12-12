#pragma once

#include <QDialog>
#include <GHostDb>
#include <GProp>

struct HostDialog : QDialog, GProp {
	Q_OBJECT
public:
	explicit HostDialog(QWidget* parent, GHostDb* hostDb) {}
	~HostDialog() {}
};
