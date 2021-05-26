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
	loadDelay();
	loadExtract();
	loadFilter();
	loadFlow();
	loadOther();
	loadProcess();
	loadWrite();
}

// ----------------------------------------------------------------------------
// Block
// ----------------------------------------------------------------------------
#include <GBlock>
#include <GTcpBlock>

void GPluginFactory::loadBlock() {
	qRegisterMetaType<GBlock*>();
	qRegisterMetaType<GTcpBlock*>();

	ItemCategory* category = new ItemCategory("block");
	category->items_.push_back(new ItemNode(GBlock::staticMetaObject.className()));
	category->items_.push_back(new ItemNode(GTcpBlock::staticMetaObject.className()));

	items_.push_back(category);
}

// ----------------------------------------------------------------------------
// Capture
// ----------------------------------------------------------------------------
#include <GArpSpoof>
#include <GAsyncNetFilter>
#include <GPcapDevice>
#include <GPcapFile>
#include <GNetFilter>
#include <GRemotePcapDevice>
#include <GWinDivert>

void GPluginFactory::loadCapture() {
	qRegisterMetaType<GArpSpoof*>();
#ifdef Q_OS_LINUX
	qRegisterMetaType<GAsyncNetFilter*>();
#endif
	qRegisterMetaType<GPcapDevice*>();
	qRegisterMetaType<GPcapFile*>();
#ifdef Q_OS_LINUX
	qRegisterMetaType<GNetFilter*>();
#endif
	qRegisterMetaType<GRemotePcapDevice*>();
#ifdef Q_OS_WIN
	qRegisterMetaType<GWinDivert*>();
#endif

	ItemCategory* category = new ItemCategory("capture");
	category->items_.push_back(new ItemNode(GArpSpoof::staticMetaObject.className()));
#ifdef Q_OS_LINUX
	category->items_.push_back(new ItemNode(GAsyncNetFilter::staticMetaObject.className()));
#endif
	category->items_.push_back(new ItemNode(GPcapDevice::staticMetaObject.className()));
	category->items_.push_back(new ItemNode(GPcapFile::staticMetaObject.className()));
#ifdef Q_OS_LINUX
	category->items_.push_back(new ItemNode(GNetFilter::staticMetaObject.className()));
#endif
	category->items_.push_back(new ItemNode(GRemotePcapDevice::staticMetaObject.className()));
#ifdef Q_OS_WIN
	category->items_.push_back(new ItemNode(GWinDivert::staticMetaObject.className()));
#endif

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

	ItemCategory* category = new ItemCategory("convert");
	category->items_.push_back(new ItemNode(GConvertEth::staticMetaObject.className()));
	category->items_.push_back(new ItemNode(GConvertEthAutoMac::staticMetaObject.className()));
	category->items_.push_back(new ItemNode(GConvertIp::staticMetaObject.className()));

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

	ItemCategory* category = new ItemCategory("delay");
	category->items_.push_back(new ItemNode(GDelay::staticMetaObject.className()));
	category->items_.push_back(new ItemNode(GSyncDelay::staticMetaObject.className()));
	category->items_.push_back(new ItemNode(GTimeStampSyncDelay::staticMetaObject.className()));

	items_.push_back(category);
}

// ----------------------------------------------------------------------------
// Extract
// ----------------------------------------------------------------------------
#include <GHttpExtract>

void GPluginFactory::loadExtract() {
	qRegisterMetaType<GHttpExtract*>();

	ItemCategory* category = new ItemCategory("extract");
	category->items_.push_back(new ItemNode(GHttpExtract::staticMetaObject.className()));

	items_.push_back(category);
}

// ----------------------------------------------------------------------------
// Filter
// ----------------------------------------------------------------------------
#include <GBpFilter>

void GPluginFactory::loadFilter() {
	qRegisterMetaType<GBpFilter*>();

	ItemCategory* category = new ItemCategory("filter");
	category->items_.push_back(new ItemNode(GBpFilter::staticMetaObject.className()));

	items_.push_back(category);
}

// ----------------------------------------------------------------------------
// Flow
// ----------------------------------------------------------------------------
#include <GIpFlowMgr>
#include <GTcpFlowMgr>
#include <GUdpFlowMgr>

void GPluginFactory::loadFlow() {
	qRegisterMetaType<GIpFlowMgr*>();
	qRegisterMetaType<GTcpFlowMgr*>();
	qRegisterMetaType<GUdpFlowMgr*>();

	ItemCategory* category = new ItemCategory("flow");
	category->items_.push_back(new ItemNode(GIpFlowMgr::staticMetaObject.className()));
	category->items_.push_back(new ItemNode(GTcpFlowMgr::staticMetaObject.className()));
	category->items_.push_back(new ItemNode(GUdpFlowMgr::staticMetaObject.className()));

	items_.push_back(category);
}

// ----------------------------------------------------------------------------
// Other
// ----------------------------------------------------------------------------
#include <GCommand>
#include <GRemoteCommand>

void GPluginFactory::loadOther() {
	qRegisterMetaType<GCommand*>();
	qRegisterMetaType<GRemoteCommand*>();

	ItemCategory* category = new ItemCategory("other");
	category->items_.push_back(new ItemNode(GCommand::staticMetaObject.className()));
	category->items_.push_back(new ItemNode(GRemoteCommand::staticMetaObject.className()));

	items_.push_back(category);
}

// ----------------------------------------------------------------------------
// Process
// ----------------------------------------------------------------------------
#include <GClientHelloSplit>
#include <GCorrectChecksum>
#include <GDnsProcessor>
#include <GFlowMgrDebugger>
#include <GPacketDebugger>

void GPluginFactory::loadProcess() {
	qRegisterMetaType<GClientHelloSplit*>();
	qRegisterMetaType<GCorrectChecksum*>();
	qRegisterMetaType<GDnsProcessor*>();
	qRegisterMetaType<GFlowMgrDebugger*>();
	qRegisterMetaType<GPacketDebugger*>();

	ItemCategory* category = new ItemCategory("process");
	category->items_.push_back(new ItemNode(GClientHelloSplit::staticMetaObject.className()));
	category->items_.push_back(new ItemNode(GCorrectChecksum::staticMetaObject.className()));
	category->items_.push_back(new ItemNode(GDnsProcessor::staticMetaObject.className()));
	category->items_.push_back(new ItemNode(GFlowMgrDebugger::staticMetaObject.className()));
	category->items_.push_back(new ItemNode(GPacketDebugger::staticMetaObject.className()));

	items_.push_back(category);
}

// ----------------------------------------------------------------------------
// Write
// ----------------------------------------------------------------------------
#include <GPcapDeviceWrite>
#include <GPcapFileWrite>
#include <GRawIpSocketWrite>
#include <GRemotePcapDeviceWrite>

void GPluginFactory::loadWrite() {
	qRegisterMetaType<GPcapDeviceWrite*>();
	qRegisterMetaType<GPcapFileWrite*>();
	qRegisterMetaType<GRawIpSocketWrite*>();
	qRegisterMetaType<GRemotePcapDeviceWrite*>();

	ItemCategory* category = new ItemCategory("write");
	category->items_.push_back(new ItemNode(GPcapDeviceWrite::staticMetaObject.className()));
	category->items_.push_back(new ItemNode(GPcapFileWrite::staticMetaObject.className()));
	category->items_.push_back(new ItemNode(GRawIpSocketWrite::staticMetaObject.className()));
	category->items_.push_back(new ItemNode(GRemotePcapDeviceWrite::staticMetaObject.className()));

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
	for (QFileInfo fileInfo: fileList) {
		QString fileName = fileInfo.filePath();
		loadFile(category, fileName);
	}

	//
	// dir
	//
	QFileInfoList dirList = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
	for (QFileInfo fileInfo: dirList) {
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
