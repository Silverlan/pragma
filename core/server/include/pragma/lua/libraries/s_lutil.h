/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_LUTIL_H__
#define __S_LUTIL_H__

#include "pragma/serverdefinitions.h"
#include <pragma/lua/luaapi.h>

struct BulletInfo;
struct GibletCreateInfo;
namespace util {
	struct SplashDamageInfo;
};
namespace Lua {
	namespace util {
		namespace Server {
			DLLSERVER luabind::object fire_bullets(lua_State *l, const BulletInfo &bulletInfo, bool hitReport);
			DLLSERVER luabind::object fire_bullets(lua_State *l, const BulletInfo &bulletInfo);
			DLLSERVER void create_giblet(lua_State *l, const GibletCreateInfo &gibletInfo);
			DLLSERVER void create_explosion(lua_State *l, const ::util::SplashDamageInfo &splashDamageInfo);
		};
	};
};

#endif
