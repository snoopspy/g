#include "widget.h"
#include "ui_widget.h"
#include <GNetInfo>

Widget::Widget(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::Widget)
{
	ui->setupUi(this);

	setWindowTitle("NetInfo");
	const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
	setFont(fixedFont);
	ui->plainTextEdit->setReadOnly(true);

	showIntrerfaceList();
	showRtm();
}

Widget::~Widget()
{
}

void Widget::showIntrerfaceList() {
	GIntfList& intfList = GNetInfo::instance().intfList();
	GRtmEntry* entry = GNetInfo::instance().rtm().getBestEntry(QString("8.8.8.8"));

	for(GIntf& intf: intfList) {
		bool best = entry != nullptr &&
				entry->intf() != nullptr &&
				entry->intf()->name() == intf.name();
		QString msg = QString("index %1 %2").arg(intf.index()).arg(best ? "(Best)" : "");
		ui->plainTextEdit->insertPlainText(msg + "\n");

		msg = QString("  %1").arg(intf.name());
		if (intf.desc() != "") msg += QString(" / %1").arg(intf.desc());
		ui->plainTextEdit->insertPlainText(msg + "\n");

		msg = "";
		if (!intf.mac().isNull()) msg += QString("mac:%1 ").arg(QString(intf.mac()));
		if (intf.ip() != 0) msg += QString("ip:%1 ").arg(QString(intf.ip()));
		if (intf.mask() != 0) msg += QString("mask:%1 ").arg(QString(intf.mask()));
		if (intf.gateway() != 0) msg += QString("gateway:%1 ").arg(QString(intf.gateway()));
		if (msg != "") {
			msg = QString("  %1").arg(msg);
			ui->plainTextEdit->insertPlainText(msg + "\n");
		}

		ui->plainTextEdit->insertPlainText("\n");
	}
}

void Widget::showRtm() {
	GRtm& rtm = GNetInfo::instance().rtm();

	ui->plainTextEdit->insertPlainText("dst             mask            gateway         metric intf\n");
	for (GRtmEntry& entry: rtm) {
		QString dstStr = QString(entry.dst()); dstStr += QString(" ").repeated(16 - dstStr.length());
		QString maskStr = QString(entry.mask()); maskStr += QString(" ").repeated(16 - maskStr.length());
		QString gatewayStr = QString(entry.gateway()); gatewayStr += QString(" ").repeated(16 - gatewayStr.length());
		QString metricStr = QString::number(entry.metric()); metricStr += QString(" ").repeated(7 - metricStr.length());
		QString intfStr = QString("%1 %2").arg(entry.intf()->index()).arg(entry.intf()->desc());
		QString msg = dstStr + maskStr + gatewayStr + metricStr + intfStr;
		ui->plainTextEdit->insertPlainText(msg + "\n");
	}
}
