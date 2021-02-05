/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/lua/lentity_components.hpp"
#include "pragma/entities/components/base_actor_component.hpp"
#include "pragma/entities/components/base_ai_component.hpp"
#include "pragma/entities/components/base_character_component.hpp"
#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/components/base_player_component.hpp"
#include "pragma/entities/components/base_vehicle_component.hpp"
#include "pragma/entities/components/base_weapon_component.hpp"
#include "pragma/lua/l_entity_handles.hpp"
#include "pragma/lua/lua_entity_component.hpp"
#include "pragma/lua/classes/lproperty.hpp"
#include "pragma/entities/entity_component_event.hpp"
#include "pragma/lua/lua_call.hpp"
#include "pragma/lua/lua_component_event.hpp"
#include <sharedutils/datastream.h>

extern DLLENGINE Engine *engine;

namespace Lua
{
	namespace BaseEntityComponent
	{
		static void GetEntity(lua_State *l,BaseEntityComponentHandle &component);
		static void GetComponentId(lua_State *l,BaseEntityComponentHandle &component);
		static void GetName(lua_State *l,BaseEntityComponentHandle &component);
	};
};
void Game::RegisterLuaEntityComponent(luabind::class_<BaseEntityComponentHandleWrapper> &def)
{
	def.def("BroadcastEvent",static_cast<void(*)(lua_State*,BaseEntityComponentHandle&,uint32_t)>([](lua_State *l,BaseEntityComponentHandle &hComponent,uint32_t eventId) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->BroadcastEvent(eventId);
	}));
	def.def("BroadcastEvent",static_cast<void(*)(lua_State*,BaseEntityComponentHandle&,uint32_t,luabind::object)>([](lua_State *l,BaseEntityComponentHandle &hComponent,uint32_t eventId,luabind::object) {
		pragma::Lua::check_component(l,hComponent);
		auto t = 3;
		Lua::CheckTable(l,t);
		if(::engine->GetNetworkState(l)->GetGameState()->BroadcastEntityEvent(*hComponent.get(),eventId,t) == false)
		{
			LuaComponentEvent luaEvent {};
			auto numArgs = Lua::GetObjectLength(l,t);
			luaEvent.arguments.reserve(numArgs);
			for(auto i=decltype(numArgs){0u};i<numArgs;++i)
			{
				Lua::PushInt(l,i +1u);
				Lua::GetTableValue(l,t);
				luaEvent.arguments.push_back(luabind::object(luabind::from_stack(l,-1)));
				Lua::Pop(l,1);
			}
			auto handled = hComponent->BroadcastEvent(eventId,luaEvent);
			//hComponent->InvokeEventCallbacks(eventId,luaEvent);
			Lua::PushInt(l,umath::to_integral(handled));
			return;
		}
		Lua::PushInt(l,umath::to_integral(util::EventReply::Handled));
	}));
	def.def("InvokeEventCallbacks",static_cast<void(*)(lua_State*,BaseEntityComponentHandle&,uint32_t)>([](lua_State *l,BaseEntityComponentHandle &hComponent,uint32_t eventId) {
		pragma::Lua::check_component(l,hComponent);
		auto handled = hComponent->InvokeEventCallbacks(eventId);
		Lua::PushInt(l,umath::to_integral(handled));
	}));
	def.def("InvokeEventCallbacks",static_cast<void(*)(lua_State*,BaseEntityComponentHandle&,uint32_t,luabind::object)>([](lua_State *l,BaseEntityComponentHandle &hComponent,uint32_t eventId,luabind::object) {
		pragma::Lua::check_component(l,hComponent);
		auto t = 3;
		Lua::CheckTable(l,t);

		LuaComponentEvent luaEvent {};
		auto numArgs = Lua::GetObjectLength(l,t);
		luaEvent.arguments.reserve(numArgs);
		for(auto i=decltype(numArgs){0u};i<numArgs;++i)
		{
			Lua::PushInt(l,i +1u);
			Lua::GetTableValue(l,t);
			luaEvent.arguments.push_back(luabind::object(luabind::from_stack(l,-1)));
			Lua::Pop(l,1);
		}
		auto handled = hComponent->InvokeEventCallbacks(eventId,luaEvent);
		Lua::PushInt(l,umath::to_integral(handled));
	}));
	def.def("GetEntity",&Lua::BaseEntityComponent::GetEntity);
	def.def("GetComponentId",&Lua::BaseEntityComponent::GetComponentId);
	def.def("SetTickPolicy",static_cast<void(*)(lua_State*,BaseEntityComponentHandle&,pragma::TickPolicy)>([](lua_State *l,BaseEntityComponentHandle &hComponent,pragma::TickPolicy tickPolicy) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetTickPolicy(tickPolicy);
	}));
	def.def("GetTickPolicy",static_cast<pragma::TickPolicy(*)(lua_State*,BaseEntityComponentHandle&)>([](lua_State *l,BaseEntityComponentHandle &hComponent) -> pragma::TickPolicy {
		pragma::Lua::check_component(l,hComponent);
		return hComponent->GetTickPolicy();
	}));
	def.def("GetNextTick",static_cast<double(*)(lua_State*,BaseEntityComponentHandle&)>([](lua_State *l,BaseEntityComponentHandle &hComponent) -> double {
		pragma::Lua::check_component(l,hComponent);
		return hComponent->GetNextTick();
	}));
	def.def("SetNextTick",static_cast<void(*)(lua_State*,BaseEntityComponentHandle&,double)>([](lua_State *l,BaseEntityComponentHandle &hComponent,double dt) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetNextTick(dt);
	}));
	def.def("IsValid",static_cast<void(*)(lua_State*,BaseEntityComponentHandle&)>([](lua_State *l,BaseEntityComponentHandle &hComponent) {
		Lua::PushBool(l,hComponent.expired() == false);
	}));
	def.def("RegisterNetEvent",static_cast<void(*)(lua_State*,BaseEntityComponentHandle&)>([](lua_State *l,BaseEntityComponentHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushInt(l,hComponent->SetupNetEvent(Lua::CheckString(l,1)));
		auto *nw = engine->GetNetworkState(l);
		auto *game = nw->GetGameState();
		auto *componentInfo = game->GetEntityComponentManager().GetComponentInfo(hComponent->GetComponentId());
		if(componentInfo && umath::is_flag_set(componentInfo->flags,pragma::ComponentFlags::Networked) == false)
			Con::cwar<<"WARNING: Component '"<<componentInfo->name<<"' has uses net-events, but was not registered as networked, this means networking will be disabled for this component! Set the 'ents.EntityComponent.FREGISTER_BIT_NETWORKED' flag when registering the component to fix this!"<<Con::endl;
	}));
	def.def("GetComponentName",&Lua::BaseEntityComponent::GetName);
	def.def("AddEventCallback",static_cast<void(*)(lua_State*,BaseEntityComponentHandle&,uint32_t,luabind::object)>([](lua_State *l,BaseEntityComponentHandle &hComponent,uint32_t eventId,luabind::object oCallback) {
		pragma::Lua::check_component(l,hComponent);
		Lua::CheckFunction(l,3);
		auto hCb = hComponent->AddEventCallback(eventId,[l,oCallback](std::reference_wrapper<pragma::ComponentEvent> ev) -> util::EventReply {
			// We need to create a copy of the lua-state pointer, since the callback can remove itself, which
			// would also cause the std::function-object to be destroyed (and therefore the captured variables).
			auto lTmp = l;
			auto oCallbackTmp = oCallback;

			auto nstack = Lua::GetStackTop(l);
			auto c = Lua::CallFunction(l,[&oCallbackTmp,&ev](lua_State *l) -> Lua::StatusCode {
				oCallbackTmp.push(l);
				ev.get().PushArguments(l);
				return Lua::StatusCode::Ok;
			},LUA_MULTRET);
			auto numRet = Lua::GetStackTop(l) -nstack;
			if(c == Lua::StatusCode::Ok && numRet > 0 && Lua::IsNone(lTmp,-1) == false)
			{
				auto result = Lua::IsNumber(lTmp,-numRet) ? static_cast<util::EventReply>(Lua::CheckInt(lTmp,-numRet)) : util::EventReply::Unhandled;
				if(result == util::EventReply::Handled)
					ev.get().HandleReturnValues(l);
				Lua::Pop(lTmp,numRet); // Pop result(s)
				return result;
			}
			return util::EventReply::Unhandled;
		});
		Lua::Push<CallbackHandle>(l,hCb);
	}));
	def.def("InjectEvent",static_cast<void(*)(lua_State*,BaseEntityComponentHandle&,uint32_t)>([](lua_State *l,BaseEntityComponentHandle &hComponent,uint32_t eventId) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->InjectEvent(eventId);
	}));
	def.def("InjectEvent",static_cast<void(*)(lua_State*,BaseEntityComponentHandle&,uint32_t,luabind::object)>([](lua_State *l,BaseEntityComponentHandle &hComponent,uint32_t eventId,luabind::object) {
		pragma::Lua::check_component(l,hComponent);
		auto t = 3;
		Lua::CheckTable(l,t);
		if(engine->GetNetworkState(l)->GetGameState()->InjectEntityEvent(*hComponent.get(),eventId,t) == false)
		{
			LuaComponentEvent luaEvent {};
			auto numArgs = Lua::GetObjectLength(l,t);
			luaEvent.arguments.reserve(numArgs);
			for(auto i=decltype(numArgs){0u};i<numArgs;++i)
			{
				Lua::PushInt(l,i +1u);
				Lua::GetTableValue(l,t);
				luaEvent.arguments.push_back(luabind::object(luabind::from_stack(l,-1)));
				Lua::Pop(l,1);
			}
			hComponent->InjectEvent(eventId,luaEvent);
		}
	}));
	def.def("Save",static_cast<void(*)(lua_State*,BaseEntityComponentHandle&,DataStream&)>([](lua_State *l,BaseEntityComponentHandle &hComponent,DataStream &ds) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->Save(ds);
	}));
	def.def("Load",static_cast<void(*)(lua_State*,BaseEntityComponentHandle&,DataStream&)>([](lua_State *l,BaseEntityComponentHandle &hComponent,DataStream &ds) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->Load(ds);
	}));
	def.def("Copy",static_cast<void(*)(lua_State*,BaseEntityComponentHandle&,BaseEntityComponentHandle&)>([](lua_State *l,BaseEntityComponentHandle &hComponent,BaseEntityComponentHandle &hComponentOther) {
		pragma::Lua::check_component(l,hComponent);
		pragma::Lua::check_component(l,hComponentOther);
		if(hComponent->GetComponentId() != hComponentOther->GetComponentId() || hComponent.get() == hComponentOther.get())
			return;
		DataStream ds {};
		ds->SetOffset(0);
		hComponent->Save(ds);
		ds->SetOffset(0);
		hComponentOther->Load(ds);
	}));
	def.add_static_constant("FREGISTER_NONE",umath::to_integral(pragma::ComponentFlags::None));
	def.add_static_constant("FREGISTER_BIT_NETWORKED",umath::to_integral(pragma::ComponentFlags::Networked));

	def.add_static_constant("CALLBACK_TYPE_ENTITY",umath::to_integral(pragma::BaseEntityComponent::CallbackType::Entity));
	def.add_static_constant("CALLBACK_TYPE_COMPONENT",umath::to_integral(pragma::BaseEntityComponent::CallbackType::Component));
}
void Lua::BaseEntityComponent::GetEntity(lua_State *l,BaseEntityComponentHandle &component)
{
	pragma::Lua::check_component(l,component);
	auto &ent = component->GetEntity();
	ent.GetLuaObject()->push(l);
}
void Lua::BaseEntityComponent::GetComponentId(lua_State *l,BaseEntityComponentHandle &component)
{
	pragma::Lua::check_component(l,component);
	Lua::PushInt(l,component->GetComponentId());
}
void Lua::BaseEntityComponent::GetName(lua_State *l,BaseEntityComponentHandle &component)
{
	pragma::Lua::check_component(l,component);
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	auto &componentManager = game->GetEntityComponentManager();
	auto &info = *componentManager.GetComponentInfo(component->GetComponentId());
	Lua::PushString(l,info.name);
}
