/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#include "stdafx_server.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/lua/classes/s_laimemory.h"
#include "luasystem.h"
#include "pragma/ai/ai_memory.h"

namespace Lua
{
	namespace AIMemoryFragment
	{
		static void GetEntity(lua_State *l,pragma::ai::Memory::Fragment *fragment);
		static void IsInView(lua_State *l,pragma::ai::Memory::Fragment *fragment);
		static void GetLastKnownPosition(lua_State *l,pragma::ai::Memory::Fragment *fragment);
		static void GetLastVelocity(lua_State *l,pragma::ai::Memory::Fragment *fragment);
		// static void GetLastKnownDistance(lua_State *l,ai::Memory::Fragment *fragment); // Only useful for internal purposes
		static void GetLastCheckTime(lua_State *l,pragma::ai::Memory::Fragment *fragment);
		static void GetLastTimeSeen(lua_State *l,pragma::ai::Memory::Fragment *fragment);
		static void GetLastTimeHeared(lua_State *l,pragma::ai::Memory::Fragment *fragment);
		static void GetLastTimeSensed(lua_State *l,pragma::ai::Memory::Fragment *fragment);
	};
	namespace AIMemory
	{
		static void GetFragments(lua_State *l,pragma::ai::Memory *mem);
		static void GetFragmentCount(lua_State *l,pragma::ai::Memory *mem);
	};
};

void Lua::AIMemory::register_class(lua_State *l,luabind::module_ &mod)
{
	auto classDef = luabind::class_<pragma::ai::Memory>("Memory");
	classDef.def("GetFragments",&GetFragments);
	classDef.def("GetFragmentCount",&GetFragmentCount);
	classDef.add_static_constant("TYPE_VISUAL",umath::to_integral(pragma::ai::Memory::MemoryType::Visual));
	classDef.add_static_constant("TYPE_SOUND",umath::to_integral(pragma::ai::Memory::MemoryType::Sound));
	classDef.add_static_constant("TYPE_SMELL",umath::to_integral(pragma::ai::Memory::MemoryType::Smell));
	mod[classDef];

	auto classDefFragment = luabind::class_<pragma::ai::Memory::Fragment>("MemoryFragment");
	classDefFragment.def("GetEntity",&AIMemoryFragment::GetEntity);
	classDefFragment.def("IsInView",&AIMemoryFragment::IsInView);
	classDefFragment.def("GetLastKnownPosition",&AIMemoryFragment::GetLastKnownPosition);
	classDefFragment.def("GetLastKnownVelocity",&AIMemoryFragment::GetLastVelocity);
	classDefFragment.def("GetLastCheckTime",&AIMemoryFragment::GetLastCheckTime);
	classDefFragment.def("GetLastTimeSeen",&AIMemoryFragment::GetLastTimeSeen);
	classDefFragment.def("GetLastTimeHeared",&AIMemoryFragment::GetLastTimeHeared);
	classDefFragment.def("GetLastTimeSensed",&AIMemoryFragment::GetLastTimeSensed);
	mod[classDefFragment];
}

void Lua::AIMemory::GetFragments(lua_State *l,pragma::ai::Memory *mem)
{
	auto t = Lua::CreateTable(l);
	for(auto i=decltype(mem->fragments.size()){0};i<mem->fragments.size();++i)
	{
		auto &fragment = mem->fragments[i];
		if(fragment.occupied == false)
			continue;
		Lua::PushInt(l,i +1);
		Lua::Push<pragma::ai::Memory::Fragment*>(l,&fragment);
		Lua::SetTableValue(l,t);
	}
}

void Lua::AIMemory::GetFragmentCount(lua_State *l,pragma::ai::Memory *mem)
{
	Lua::PushInt(l,mem->occupiedFragmentCount);
}

void Lua::AIMemoryFragment::GetEntity(lua_State *l,pragma::ai::Memory::Fragment *fragment)
{
	if(!fragment->hEntity.IsValid())
		return;
	fragment->hEntity->GetLuaObject()->push(l);
}
void Lua::AIMemoryFragment::IsInView(lua_State *l,pragma::ai::Memory::Fragment *fragment)
{
	Lua::PushBool(l,fragment->visible);
}
void Lua::AIMemoryFragment::GetLastKnownPosition(lua_State *l,pragma::ai::Memory::Fragment *fragment)
{
	Lua::Push<Vector3>(l,fragment->lastPosition);
}
void Lua::AIMemoryFragment::GetLastVelocity(lua_State *l,pragma::ai::Memory::Fragment *fragment)
{
	Lua::Push<Vector3>(l,fragment->lastVelocity);
}
void Lua::AIMemoryFragment::GetLastCheckTime(lua_State *l,pragma::ai::Memory::Fragment *fragment)
{
	Lua::PushNumber(l,fragment->lastCheck);
}
void Lua::AIMemoryFragment::GetLastTimeSeen(lua_State *l,pragma::ai::Memory::Fragment *fragment)
{
	Lua::PushNumber(l,fragment->lastSeen);
}
void Lua::AIMemoryFragment::GetLastTimeHeared(lua_State *l,pragma::ai::Memory::Fragment *fragment)
{
	Lua::PushNumber(l,fragment->lastHeared);
}
void Lua::AIMemoryFragment::GetLastTimeSensed(lua_State *l,pragma::ai::Memory::Fragment *fragment)
{
	Lua::PushNumber(l,fragment->GetLastTimeSensed());
}
