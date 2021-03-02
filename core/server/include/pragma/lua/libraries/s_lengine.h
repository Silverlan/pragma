/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_LENGINE_H__
#define __S_LENGINE_H__

#include "pragma/serverdefinitions.h"
#include <pragma/lua/ldefinitions.h>

class Material;
namespace Lua::engine::server
{
	DLLSERVER Material *LoadMaterial(const std::string &mat,bool reload);
	DLLSERVER Material *LoadMaterial(const std::string &mat);
}

#endif
