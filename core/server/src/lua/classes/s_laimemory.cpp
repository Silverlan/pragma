/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/lua/classes/s_laimemory.h"
#include "luasystem.h"
#include "pragma/ai/ai_memory.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>

namespace Lua {
	namespace AIMemoryFragment {
		static BaseEntity *GetEntity(pragma::ai::Memory::Fragment &fragment);
		static bool IsInView(const pragma::ai::Memory::Fragment &fragment);
		static Vector3 GetLastKnownPosition(const pragma::ai::Memory::Fragment &fragment);
		static Vector3 GetLastVelocity(const pragma::ai::Memory::Fragment &fragment);
		// static void GetLastKnownDistance(lua_State *l,ai::Memory::Fragment *fragment); // Only useful for internal purposes
		static float GetLastCheckTime(const pragma::ai::Memory::Fragment &fragment);
		static float GetLastTimeSeen(const pragma::ai::Memory::Fragment &fragment);
		static float GetLastTimeHeared(const pragma::ai::Memory::Fragment &fragment);
		static float GetLastTimeSensed(const pragma::ai::Memory::Fragment &fragment);
	};
	namespace AIMemory {
		static luabind::tableT<pragma::ai::Memory::Fragment> GetFragments(lua_State *l, pragma::ai::Memory &mem);
		static uint32_t GetFragmentCount(const pragma::ai::Memory &mem);
	};
};

void Lua::AIMemory::register_class(lua_State *l, luabind::module_ &mod)
{
	auto classDef = luabind::class_<pragma::ai::Memory>("Memory");
	classDef.def("GetFragments", &GetFragments);
	classDef.def("GetFragmentCount", &GetFragmentCount);
	classDef.add_static_constant("TYPE_VISUAL", umath::to_integral(pragma::ai::Memory::MemoryType::Visual));
	classDef.add_static_constant("TYPE_SOUND", umath::to_integral(pragma::ai::Memory::MemoryType::Sound));
	classDef.add_static_constant("TYPE_SMELL", umath::to_integral(pragma::ai::Memory::MemoryType::Smell));
	mod[classDef];

	auto classDefFragment = luabind::class_<pragma::ai::Memory::Fragment>("MemoryFragment");
	classDefFragment.def("GetEntity", &AIMemoryFragment::GetEntity);
	classDefFragment.def("IsInView", &AIMemoryFragment::IsInView);
	classDefFragment.def("GetLastKnownPosition", &AIMemoryFragment::GetLastKnownPosition);
	classDefFragment.def("GetLastKnownVelocity", &AIMemoryFragment::GetLastVelocity);
	classDefFragment.def("GetLastCheckTime", &AIMemoryFragment::GetLastCheckTime);
	classDefFragment.def("GetLastTimeSeen", &AIMemoryFragment::GetLastTimeSeen);
	classDefFragment.def("GetLastTimeHeared", &AIMemoryFragment::GetLastTimeHeared);
	classDefFragment.def("GetLastTimeSensed", &AIMemoryFragment::GetLastTimeSensed);
	mod[classDefFragment];
}

luabind::tableT<pragma::ai::Memory::Fragment> Lua::AIMemory::GetFragments(lua_State *l, pragma::ai::Memory &mem)
{
	auto t = luabind::newtable(l);
	uint32_t idx = 1;
	for(auto &memFrag : mem.fragments) {
		if(memFrag.occupied == false)
			continue;
		t[idx++] = &memFrag;
	}
	return t;
}

uint32_t Lua::AIMemory::GetFragmentCount(const pragma::ai::Memory &mem) { return mem.occupiedFragmentCount; }

BaseEntity *Lua::AIMemoryFragment::GetEntity(pragma::ai::Memory::Fragment &fragment) { return fragment.hEntity.get(); }
bool Lua::AIMemoryFragment::IsInView(const pragma::ai::Memory::Fragment &fragment) { return fragment.visible; }
Vector3 Lua::AIMemoryFragment::GetLastKnownPosition(const pragma::ai::Memory::Fragment &fragment) { return fragment.lastPosition; }
Vector3 Lua::AIMemoryFragment::GetLastVelocity(const pragma::ai::Memory::Fragment &fragment) { return fragment.lastVelocity; }
float Lua::AIMemoryFragment::GetLastCheckTime(const pragma::ai::Memory::Fragment &fragment) { return fragment.lastCheck; }
float Lua::AIMemoryFragment::GetLastTimeSeen(const pragma::ai::Memory::Fragment &fragment) { return fragment.lastSeen; }
float Lua::AIMemoryFragment::GetLastTimeHeared(const pragma::ai::Memory::Fragment &fragment) { return fragment.lastHeared; }
float Lua::AIMemoryFragment::GetLastTimeSensed(const pragma::ai::Memory::Fragment &fragment) { return fragment.GetLastTimeSensed(); }
