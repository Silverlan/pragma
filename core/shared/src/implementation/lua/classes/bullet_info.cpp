// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;



module pragma.shared;

import :scripting.lua.classes.bullet_info;

namespace Lua {
	// These don't like to be in core/shared/src/lua/class.cpp for some reason (Something to do with EntityHandle hAttacker/hInflictor)
	void register_bullet_info(luabind::module_ &gameMod)
	{
		auto bulletInfo = luabind::class_<BulletInfo>("BulletInfo");
		bulletInfo.def(luabind::constructor<>());
		bulletInfo.def(luabind::tostring(luabind::self));
		bulletInfo.def_readwrite("spread", &BulletInfo::spread);
		bulletInfo.def_readwrite("force", &BulletInfo::force);
		bulletInfo.def_readwrite("distance", &BulletInfo::distance);
		bulletInfo.def_readwrite("damageType", reinterpret_cast<std::underlying_type_t<decltype(BulletInfo::damageType)> BulletInfo::*>(&BulletInfo::damageType));
		bulletInfo.def_readwrite("bulletCount", &BulletInfo::bulletCount);
		bulletInfo.def_readwrite("attacker", &BulletInfo::hAttacker);
		bulletInfo.def_readwrite("inflictor", &BulletInfo::hInflictor);
		bulletInfo.def_readwrite("tracerCount", &BulletInfo::tracerCount);
		bulletInfo.def_readwrite("tracerRadius", &BulletInfo::tracerRadius);
		bulletInfo.def_readwrite("tracerColor", &BulletInfo::tracerColor);
		bulletInfo.def_readwrite("tracerLength", &BulletInfo::tracerLength);
		bulletInfo.def_readwrite("tracerSpeed", &BulletInfo::tracerSpeed);
		bulletInfo.def_readwrite("tracerMaterial", &BulletInfo::tracerMaterial);
		bulletInfo.def_readwrite("tracerBloom", &BulletInfo::tracerBloom);
		bulletInfo.def_readwrite("ammoType", &BulletInfo::ammoType);
		bulletInfo.def_readwrite("direction", &BulletInfo::direction);
		bulletInfo.def_readwrite("effectOrigin", &BulletInfo::effectOrigin);
		bulletInfo.def_readwrite("damage", &BulletInfo::damage);
		gameMod[bulletInfo];
	}
};
