// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.classes.bullet_info;

namespace Lua {
	// These don't like to be in core/shared/src/lua/class.cpp for some reason (Something to do with EntityHandle hAttacker/hInflictor)
	void register_bullet_info(luabind::module_ &gameMod)
	{
		auto bulletInfo = luabind::class_<pragma::game::BulletInfo>("BulletInfo");
		bulletInfo.def(luabind::constructor<>());
		bulletInfo.def(luabind::tostring(luabind::self));
		bulletInfo.def_readwrite("spread", &pragma::game::BulletInfo::spread);
		bulletInfo.def_readwrite("force", &pragma::game::BulletInfo::force);
		bulletInfo.def_readwrite("distance", &pragma::game::BulletInfo::distance);
		bulletInfo.def_readwrite("damageType", reinterpret_cast<std::underlying_type_t<decltype(pragma::game::BulletInfo::damageType)> pragma::game::BulletInfo::*>(&pragma::game::BulletInfo::damageType));
		bulletInfo.def_readwrite("bulletCount", &pragma::game::BulletInfo::bulletCount);
		bulletInfo.def_readwrite("attacker", &pragma::game::BulletInfo::hAttacker);
		bulletInfo.def_readwrite("inflictor", &pragma::game::BulletInfo::hInflictor);
		bulletInfo.def_readwrite("tracerCount", &pragma::game::BulletInfo::tracerCount);
		bulletInfo.def_readwrite("tracerRadius", &pragma::game::BulletInfo::tracerRadius);
		bulletInfo.def_readwrite("tracerColor", &pragma::game::BulletInfo::tracerColor);
		bulletInfo.def_readwrite("tracerLength", &pragma::game::BulletInfo::tracerLength);
		bulletInfo.def_readwrite("tracerSpeed", &pragma::game::BulletInfo::tracerSpeed);
		bulletInfo.def_readwrite("tracerMaterial", &pragma::game::BulletInfo::tracerMaterial);
		bulletInfo.def_readwrite("tracerBloom", &pragma::game::BulletInfo::tracerBloom);
		bulletInfo.def_readwrite("ammoType", &pragma::game::BulletInfo::ammoType);
		bulletInfo.def_readwrite("direction", &pragma::game::BulletInfo::direction);
		bulletInfo.def_readwrite("effectOrigin", &pragma::game::BulletInfo::effectOrigin);
		bulletInfo.def_readwrite("damage", &pragma::game::BulletInfo::damage);
		gameMod[bulletInfo];
	}
};
