// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LMATERIAL_H__
#define __LMATERIAL_H__

#include "pragma/networkdefinitions.h"
#include "pragma/lua/ldefinitions.h"

class Material;
namespace Lua {
	namespace Material {
		DLLNETWORK void register_class(luabind::class_<::Material> &classDef);
	};
};

#endif
