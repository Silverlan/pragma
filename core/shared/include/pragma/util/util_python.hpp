/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __PRAGMA_UTIL_PYTHON_HPP__
#define __PRAGMA_UTIL_PYTHON_HPP__

#include "pragma/networkdefinitions.h"
#include <cinttypes>

namespace pragma::python
{
	DLLNETWORK bool run(const char *code);
	DLLNETWORK bool exec(std::string fileName,uint32_t argc,const char **argv);

	DLLNETWORK bool init_blender();
};

#endif
