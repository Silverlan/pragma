// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :scripting.lua.classes.ai_memory;

import :ai.memory;
import pragma.shared;

namespace Lua {
	namespace AIMemoryFragment {
		static pragma::ecs::BaseEntity *GetEntity(pragma::ai::Memory::Fragment &fragment);
		static bool IsInView(const pragma::ai::Memory::Fragment &fragment);
		static Vector3 GetLastKnownPosition(const pragma::ai::Memory::Fragment &fragment);
		static Vector3 GetLastVelocity(const pragma::ai::Memory::Fragment &fragment);
		// static void GetLastKnownDistance(lua::State *l,ai::Memory::Fragment *fragment); // Only useful for internal purposes
		static float GetLastCheckTime(const pragma::ai::Memory::Fragment &fragment);
		static float GetLastTimeSeen(const pragma::ai::Memory::Fragment &fragment);
		static float GetLastTimeHeared(const pragma::ai::Memory::Fragment &fragment);
		static float GetLastTimeSensed(const pragma::ai::Memory::Fragment &fragment);
	};
	namespace AIMemory {
		static luabind::tableT<pragma::ai::Memory::Fragment> GetFragments(lua::State *l, pragma::ai::Memory &mem);
		static uint32_t GetFragmentCount(const pragma::ai::Memory &mem);
	};
};

void Lua::AIMemory::register_class(lua::State *l, luabind::module_ &mod)
{
	auto classDef = luabind::class_<pragma::ai::Memory>("Memory");
	classDef.def("GetFragments", &GetFragments);
	classDef.def("GetFragmentCount", &GetFragmentCount);
	classDef.add_static_constant("TYPE_VISUAL", pragma::math::to_integral(pragma::ai::Memory::MemoryType::Visual));
	classDef.add_static_constant("TYPE_SOUND", pragma::math::to_integral(pragma::ai::Memory::MemoryType::Sound));
	classDef.add_static_constant("TYPE_SMELL", pragma::math::to_integral(pragma::ai::Memory::MemoryType::Smell));
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

luabind::tableT<pragma::ai::Memory::Fragment> Lua::AIMemory::GetFragments(lua::State *l, pragma::ai::Memory &mem)
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

pragma::ecs::BaseEntity *Lua::AIMemoryFragment::GetEntity(pragma::ai::Memory::Fragment &fragment) { return fragment.hEntity.get(); }
bool Lua::AIMemoryFragment::IsInView(const pragma::ai::Memory::Fragment &fragment) { return fragment.visible; }
Vector3 Lua::AIMemoryFragment::GetLastKnownPosition(const pragma::ai::Memory::Fragment &fragment) { return fragment.lastPosition; }
Vector3 Lua::AIMemoryFragment::GetLastVelocity(const pragma::ai::Memory::Fragment &fragment) { return fragment.lastVelocity; }
float Lua::AIMemoryFragment::GetLastCheckTime(const pragma::ai::Memory::Fragment &fragment) { return fragment.lastCheck; }
float Lua::AIMemoryFragment::GetLastTimeSeen(const pragma::ai::Memory::Fragment &fragment) { return fragment.lastSeen; }
float Lua::AIMemoryFragment::GetLastTimeHeared(const pragma::ai::Memory::Fragment &fragment) { return fragment.lastHeared; }
float Lua::AIMemoryFragment::GetLastTimeSensed(const pragma::ai::Memory::Fragment &fragment) { return fragment.GetLastTimeSensed(); }
