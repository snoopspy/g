#include "gpluginfactory.h"
#include <QCoreApplication>
#include <QDir>

// ----------------------------------------------------------------------------
// GPluginFactory
// ----------------------------------------------------------------------------
GPluginFactory::GPluginFactory(QObject* parent) : GGraph::Factory(parent) {
	loadDefault();
	load("plugin");
}

GPluginFactory::~GPluginFactory() {
	for (QLibrary* library: libraries_) {
		delete library;
	}
}

void GPluginFactory::loadDefault() {
	loadBlock();
	loadCapture();
	loadConvert();
	loadChange();
	loadDelay();
	loadFilter();
	loadManage();
	loadProcess();
	loadWrite();
}

// ----------------------------------------------------------------------------
// Block
// ----------------------------------------------------------------------------
#include <GArpBlock>
#include <GBlock>
#include <GDnsBlock>
#include <GDot11Block>
#include <GTcpBlock>

void GPluginFactory::loadBlock() {
	qRegisterMetaType<GArpBlock*>();
	qRegisterMetaType<GBlock*>();
	qRegisterMetaType<GDnsBlock*>();
	qRegisterMetaType<GDot11Block*>();
	qRegisterMetaType<GTcpBlock*>();

	ItemCategory* category = new ItemCategory("Block");
	category->items_.push_back(new ItemNode("GArpBlock"));
	category->items_.push_back(new ItemNode("GBlock"));
	category->items_.push_back(new ItemNode("GDnsBlock"));
	category->items_.push_back(new ItemNode("GDot11Block"));
	category->items_.push_back(new ItemNode("GTcpBlock"));

	items_.push_back(category);
}

// ----------------------------------------------------------------------------
// Capture
// ----------------------------------------------------------------------------
#include <GArpSpoof>
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
#include <GAsyncNetFilter>
#endif
#include <GAutoArpSpoof>
#ifdef Q_OS_LINUX
#include <GMonitorDevice>
#include <GNetFilter>
#endif
#include <GPcapDevice>
#include <GPcapFile>
#include <GPcapPipe>
#include <GPcapPipeNexmon>
#ifdef Q_OS_WIN
#include <GWinDivert>
#endif

void GPluginFactory::loadCapture() {
	qRegisterMetaType<GArpSpoof*>();
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
	qRegisterMetaType<GAsyncNetFilter*>();
#endif
	qRegisterMetaType<GAutoArpSpoof*>();
#ifndef Q_OS_WIN
	qRegisterMetaType<GMonitorDevice*>();
	qRegisterMetaType<GNetFilter*>();
#endif
	qRegisterMetaType<GPcapDevice*>();
	qRegisterMetaType<GPcapFile*>();
	qRegisterMetaType<GPcapPipe*>();
	qRegisterMetaType<GPcapPipeNexmon*>();
#ifdef Q_OS_WIN
	qRegisterMetaType<GWinDivert*>();
#endif

	ItemCategory* category = new ItemCategory("Capture");
	category->items_.push_back(new ItemNode("GArpSpoof"));
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
	category->items_.push_back(new ItemNode("GAsyncNetFilter"));
#endif
	category->items_.push_back(new ItemNode("GAutoArpSpoof"));
#ifdef Q_OS_LINUX
	category->items_.push_back(new ItemNode("GMonitorDevice"));
	category->items_.push_back(new ItemNode("GNetFilter"));
#endif
	category->items_.push_back(new ItemNode("GPcapDevice"));
	category->items_.push_back(new ItemNode("GPcapFile"));
	category->items_.push_back(new ItemNode("GPcapPipe"));
	category->items_.push_back(new ItemNode("GPcapPipeNexmon"));
#ifdef Q_OS_WIN
	category->items_.push_back(new ItemNode("GWinDivert"));
#endif

	items_.push_back(category);
}

// ----------------------------------------------------------------------------
// Change
// ----------------------------------------------------------------------------
#include <GFind>
#include <GReplace>
#include <GTtlReplace>

void GPluginFactory::loadChange() {
	qRegisterMetaType<GFind*>();
	qRegisterMetaType<GReplace*>();
	qRegisterMetaType<GTtlReplace*>();

	ItemCategory* category = new ItemCategory("Change");
	category->items_.push_back(new ItemNode("GFind"));
	category->items_.push_back(new ItemNode("GReplace"));
	category->items_.push_back(new ItemNode("GTtlReplace"));

	items_.push_back(category);
}

// ----------------------------------------------------------------------------
// Convert
// ----------------------------------------------------------------------------
#include <GConvertEth>
#include <GConvertEthAutoMac>
#include <GConvertIp>

void GPluginFactory::loadConvert() {
	qRegisterMetaType<GConvertEth*>();
	qRegisterMetaType<GConvertEthAutoMac*>();
	qRegisterMetaType<GConvertIp*>();

	ItemCategory* category = new ItemCategory("Convert");
	category->items_.push_back(new ItemNode("GConvertEth"));
	category->items_.push_back(new ItemNode("GConvertEthAutoMac"));
	category->items_.push_back(new ItemNode("GConvertIp"));

	items_.push_back(category);
}

// ----------------------------------------------------------------------------
// Delay
// ----------------------------------------------------------------------------
#include <GDelay>
#include <GSyncDelay>
#include <GTimeStampSyncDelay>

void GPluginFactory::loadDelay() {
	qRegisterMetaType<GDelay*>();
	qRegisterMetaType<GSyncDelay*>();
	qRegisterMetaType<GTimeStampSyncDelay*>();

	ItemCategory* category = new ItemCategory("Delay");
	category->items_.push_back(new ItemNode("GDelay"));
	category->items_.push_back(new ItemNode("GSyncDelay"));
	category->items_.push_back(new ItemNode("GTimeStampSyncDelay"));

	items_.push_back(category);
}

// ----------------------------------------------------------------------------
// Filter
// ----------------------------------------------------------------------------
#include <GBpFilter>

void GPluginFactory::loadFilter() {
	qRegisterMetaType<GBpFilter*>();

	ItemCategory* category = new ItemCategory("Filter");
	category->items_.push_back(new ItemNode("GBpFilter"));

	items_.push_back(category);
}

// ----------------------------------------------------------------------------
// Manage
// ----------------------------------------------------------------------------
#include <GHostDb>
#include <GHostMgr>
#include <GHostScan>
#include <GHostWatch>
#include <GIpFlowMgr>
#include <GIpMgr>
#include <GTcpFlowMgr>
#include <GUdpFlowMgr>

void GPluginFactory::loadManage() {
	qRegisterMetaType<GHostDb*>();
	qRegisterMetaType<GHostMgr*>();
	qRegisterMetaType<GHostScan*>();
	qRegisterMetaType<GHostWatch*>();
	qRegisterMetaType<GIpFlowMgr*>();
	qRegisterMetaType<GIpMgr*>();
	qRegisterMetaType<GTcpFlowMgr*>();
	qRegisterMetaType<GUdpFlowMgr*>();

	ItemCategory* category = new ItemCategory("Manage");
	category->items_.push_back(new ItemNode("GHostDb"));
	category->items_.push_back(new ItemNode("GHostMgr"));
	category->items_.push_back(new ItemNode("GHostScan"));
	category->items_.push_back(new ItemNode("GHostWatch"));
	category->items_.push_back(new ItemNode("GIpFlowMgr"));
	category->items_.push_back(new ItemNode("GIpMgr"));
	category->items_.push_back(new ItemNode("GTcpFlowMgr"));
	category->items_.push_back(new ItemNode("GUdpFlowMgr"));

	items_.push_back(category);
}

// ----------------------------------------------------------------------------
// Process
// ----------------------------------------------------------------------------
#include <GBeaconFlood>
#include <GChannelHop>
#include <GClientHelloSplit>
#include <GCommand>
#include <GCorrectChecksum>
#include <GDnsProcessor>
#include <GPacketDebug>
#include <GPacketMgrDebug>
#include <GScreenKeeper>
#include <GScreenSaver>
#include <GTraceRoute>

void GPluginFactory::loadProcess() {
#ifndef Q_OS_WIN
	qRegisterMetaType<GBeaconFlood*>();
	qRegisterMetaType<GChannelHop*>();
#endif
	qRegisterMetaType<GClientHelloSplit*>();
	qRegisterMetaType<GCorrectChecksum*>();
	qRegisterMetaType<GCommand*>();
	qRegisterMetaType<GDnsProcessor*>();
	qRegisterMetaType<GPacketDebug*>();
	qRegisterMetaType<GPacketMgrDebug*>();
	qRegisterMetaType<GScreenKeeper*>();
	qRegisterMetaType<GScreenSaver*>();
	qRegisterMetaType<GTraceRoute*>();

	ItemCategory* category = new ItemCategory("Process");
	category->items_.push_back(new ItemNode("GBeaconFlood"));
	category->items_.push_back(new ItemNode("GChannelHop"));
	category->items_.push_back(new ItemNode("GClientHelloSplit"));
	category->items_.push_back(new ItemNode("GCommand"));
	category->items_.push_back(new ItemNode("GCorrectChecksum"));
	category->items_.push_back(new ItemNode("GDnsProcessor"));
	category->items_.push_back(new ItemNode("GPacketDebug"));
	category->items_.push_back(new ItemNode("GPacketMgrDebug"));
	category->items_.push_back(new ItemNode("GScreenKeeper"));
	category->items_.push_back(new ItemNode("GScreenSaver"));
	category->items_.push_back(new ItemNode("GTraceRoute"));

	items_.push_back(category);
}

// ----------------------------------------------------------------------------
// Write
// ----------------------------------------------------------------------------
#ifdef Q_OS_LINUX
#include <GMonitorDeviceWrite>
#endif
#include <GPcapDeviceWrite>
#include <GPcapFileWrite>
#include <GRawIpSocketWrite>

void GPluginFactory::loadWrite() {
#ifdef Q_OS_LINUX
	qRegisterMetaType<GMonitorDeviceWrite*>();
#endif
	qRegisterMetaType<GPcapDeviceWrite*>();
	qRegisterMetaType<GPcapFileWrite*>();
	qRegisterMetaType<GRawIpSocketWrite*>();

	ItemCategory* category = new ItemCategory("Write");
#ifdef Q_OS_LINUX
	category->items_.push_back(new ItemNode("GMonitorDeviceWrite"));
#endif
	category->items_.push_back(new ItemNode("GPcapDeviceWrite"));
	category->items_.push_back(new ItemNode("GPcapFileWrite"));
	category->items_.push_back(new ItemNode("GRawIpSocketWrite"));

	items_.push_back(category);
}

void GPluginFactory::load(QString folder) {
	QDir d(folder);
	if (d.isRelative())
		folder = QCoreApplication::applicationDirPath() + "/" + folder;
	if (!folder.endsWith("/"))
		folder += "/";
	loadFolder(nullptr, folder);
}

void GPluginFactory::loadFile(GGraph::Factory::ItemCategory* category, QString fileName) {
	QLibrary* library = new QLibrary(fileName);
	if (!library->load()) {
		qWarning() << QString("library->load return false for (%1) %2").arg(fileName, library->errorString());
		delete library;
		return;
	}

	typedef int (*CountFunc)();
	typedef const char* (*NameFunc)(int index);
	typedef void* (*CreateFunc)(int index);

	CountFunc countFunc = CountFunc(library->resolve("count"));
	if (countFunc == nullptr) {
		qWarning() << QString("can not find 'count' function for (%1)").arg(fileName);
		delete library;
		return;
	}
	NameFunc nameFunc= NameFunc(library->resolve("name"));
	if (nameFunc == nullptr) {
		qWarning() << QString("can not find 'name' function for (%1)").arg(fileName);
		delete library;
		return;
	}
	CreateFunc createFunc = CreateFunc(library->resolve("create"));
	if (createFunc == nullptr) {
		qWarning() << QString("can not find 'create' function for (%1)").arg(fileName);
		delete library;
		return;
	}

	int count = countFunc();
	for (int i = 0; i < count; i++) {
		const char* className = nameFunc(i);
		if (className == nullptr) {
			qCritical() << QString("call nameFunc(%1) return nullptr file=%2").arg(i).arg(fileName);
			break;
		}
		if (category == nullptr)
			this->items_.push_back(new ItemNode(className));
		else
			category->items_.push_back(new ItemNode(className));
	}

	this->libraries_.push_back(library);
}

void GPluginFactory::loadFolder(GGraph::Factory::ItemCategory* category, QString folder) {
	QDir dir(folder);

	//
	// file
	//
#ifdef Q_OS_LINUX
	QStringList files = QStringList("*.so");
#endif
#ifdef Q_OS_WIN
	QStringList files = QStringList("*.dll");
#endif
	QFileInfoList fileList = dir.entryInfoList(files);
	for (QFileInfo& fileInfo: fileList) {
		QString fileName = fileInfo.filePath();
		loadFile(category, fileName);
	}

	//
	// dir
	//
	QFileInfoList dirList = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
	for (QFileInfo& fileInfo: dirList) {
		QString fileName = fileInfo.fileName();
		QString subFolder = folder + fileName + "/";
		ItemCategory* subCategory = new ItemCategory(fileName);
		if (category == nullptr)
			this->items_.push_back(subCategory);
		else
			category->items_.push_back(subCategory);
		loadFolder(subCategory, subFolder);
	}
}
