/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_CUBEMAPSIDE_H__
#define __C_CUBEMAPSIDE_H__

#include "pragma/clientdefinitions.h"
#include <cinttypes>

enum class DLLCLIENT CubeMapSide : uint32_t
{
	Left = 0,
	Right = 1,
	Top = 2,
	Bottom = 3,
	Front = 4,
	Back = 5
};

#endif
