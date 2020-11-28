/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_RENDERMODE_H__
#define __C_RENDERMODE_H__
#include "pragma/clientdefinitions.h"

enum class DLLCLIENT RenderMode : uint32_t
{
	None = 0,
	World,
	View,
	Skybox,
	Water,
	Count
};

#endif