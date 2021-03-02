/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __ENGINE_VERSION_H__
#define __ENGINE_VERSION_H__

#include "pragma/definitions.h"
#include <sharedutils/utildefinitions.h>
#include <string>

#define ENGINE_VERSION_MAJOR 0
#define ENGINE_VERSION_MINOR 9
#define ENGINE_VERSION_REVISION 13

namespace util
{
	struct DLLSHUTIL Version;
}

DLLNETWORK util::Version &get_engine_version();
DLLNETWORK std::string get_pretty_engine_version();

#endif