// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_LENGINE_H__
#define __S_LENGINE_H__

#include "pragma/serverdefinitions.h"
#include <pragma/lua/ldefinitions.h>

class Material;
namespace Lua::engine::server {
	DLLSERVER Material *LoadMaterial(const std::string &mat, bool reload);
	DLLSERVER Material *LoadMaterial(const std::string &mat);
}

#endif
