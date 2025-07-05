// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __ENGINE_VERSION_H__
#define __ENGINE_VERSION_H__

#include "pragma/definitions.h"
#include <sharedutils/utildefinitions.h>
#include <string>

#define ENGINE_VERSION_MAJOR 1
#define ENGINE_VERSION_MINOR 3
#define ENGINE_VERSION_REVISION 0

namespace util {
	struct DLLSHUTIL Version;
}

DLLNETWORK util::Version &get_engine_version();
DLLNETWORK std::string get_pretty_engine_version();

#endif
