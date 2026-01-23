// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:scripting.lua.libraries.util;

export import pragma.shared;
export import luabind;

export namespace Lua {
	namespace util {
		namespace Server {
			DLLSERVER luabind::object fire_bullets(lua::State *l, const pragma::game::BulletInfo &bulletInfo, bool hitReport);
			DLLSERVER luabind::object fire_bullets(lua::State *l, const pragma::game::BulletInfo &bulletInfo);
			DLLSERVER void create_giblet(lua::State *l, const GibletCreateInfo &gibletInfo);
			DLLSERVER void create_explosion(lua::State *l, const pragma::util::SplashDamageInfo &splashDamageInfo);
		};
	};
};
