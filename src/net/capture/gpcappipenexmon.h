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

#include "gpcappipe.h"

// ----------------------------------------------------------------------------
// GPcapPipeNexmon
// ----------------------------------------------------------------------------
struct G_EXPORT GPcapPipeNexmon : GPcapPipe {
	Q_OBJECT

public:
	Q_INVOKABLE GPcapPipeNexmon(QObject* parent = nullptr);
	~GPcapPipeNexmon() override;
};
