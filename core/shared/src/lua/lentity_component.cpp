/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
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
#include "pragma/lua/policies/game_object_policy.hpp"
#include "pragma/lua/l_entity_handles.hpp"
#include "pragma/lua/lua_entity_component.hpp"
#include "pragma/lua/classes/lproperty.hpp"
#include "pragma/entities/entity_component_event.hpp"
#include "pragma/lua/lua_call.hpp"
#include "pragma/lua/lua_component_event.hpp"
#include <sharedutils/datastream.h>
#include <udm.hpp>

extern DLLNETWORK Engine *engine;

namespace Lua
{
	namespace BaseEntityComponent
	{
		static std::string GetName(lua_State *l,pragma::BaseEntityComponent &component);
	};
};
void Game::RegisterLuaEntityComponent(luabind::class_<pragma::BaseEntityComponent> &def)
{
	def.def("BroadcastEvent",static_cast<util::EventReply(pragma::BaseEntityComponent::*)(pragma::ComponentEventId) const>(&pragma::BaseEntityComponent::BroadcastEvent));
	def.def("BroadcastEvent",static_cast<util::EventReply(*)(lua_State*,pragma::BaseEntityComponent&,uint32_t,const luabind::tableT<void>&)>([](lua_State *l,pragma::BaseEntityComponent &hComponent,uint32_t eventId,const luabind::tableT<void> &eventArgs) {
		int32_t t = 3;
		if(::engine->GetNetworkState(l)->GetGameState()->BroadcastEntityEvent(hComponent,eventId,t) == false)
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
			auto handled = hComponent.BroadcastEvent(eventId,luaEvent);
			//hComponent->InvokeEventCallbacks(eventId,luaEvent);
			return handled;
		}
		return util::EventReply::Handled;
	}));
	def.def("InvokeEventCallbacks",static_cast<util::EventReply(pragma::BaseEntityComponent::*)(pragma::ComponentEventId) const>(&pragma::BaseEntityComponent::InvokeEventCallbacks));
	def.def("InvokeEventCallbacks",static_cast<util::EventReply(*)(lua_State*,pragma::BaseEntityComponent&,uint32_t,const luabind::tableT<void>&)>([](lua_State *l,pragma::BaseEntityComponent &hComponent,uint32_t eventId,const luabind::tableT<void> &eventArgs) {
		int32_t t = 3;

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
		return hComponent.InvokeEventCallbacks(eventId,luaEvent);
	}));
	def.def("GetEntity",static_cast<BaseEntity&(pragma::BaseEntityComponent::*)()>(&pragma::BaseEntityComponent::GetEntity),luabind::game_object_policy<0>{});
	def.def("GetComponentId",&pragma::BaseEntityComponent::GetComponentId);
	def.def("SetTickPolicy",&pragma::BaseEntityComponent::SetTickPolicy);
	def.def("GetTickPolicy",&pragma::BaseEntityComponent::GetTickPolicy);
	def.def("GetNextTick",&pragma::BaseEntityComponent::GetNextTick);
	def.def("SetNextTick",&pragma::BaseEntityComponent::SetNextTick);
	def.def("IsValid",static_cast<bool(*)(lua_State*,pragma::BaseEntityComponent*)>([](lua_State *l,pragma::BaseEntityComponent *hComponent) {
		return hComponent != nullptr;
	}));
	def.def("RegisterNetEvent",static_cast<void(*)(lua_State*,pragma::BaseEntityComponent&,const std::string&)>([](lua_State *l,pragma::BaseEntityComponent &hComponent,const std::string &eventName) {
		hComponent.SetupNetEvent(eventName);
		auto *nw = engine->GetNetworkState(l);
		auto *game = nw->GetGameState();
		auto *componentInfo = game->GetEntityComponentManager().GetComponentInfo(hComponent.GetComponentId());
		if(componentInfo && umath::is_flag_set(componentInfo->flags,pragma::ComponentFlags::Networked) == false)
			Con::cwar<<"WARNING: Component '"<<componentInfo->name<<"' has uses net-events, but was not registered as networked, this means networking will be disabled for this component! Set the 'ents.EntityComponent.FREGISTER_BIT_NETWORKED' flag when registering the component to fix this!"<<Con::endl;
	}));
	def.def("GetComponentName",&Lua::BaseEntityComponent::GetName);
	def.def("AddEventCallback",static_cast<CallbackHandle(*)(lua_State*,pragma::BaseEntityComponent&,uint32_t,const Lua::func<void>&)>([](lua_State *l,pragma::BaseEntityComponent &hComponent,uint32_t eventId,const Lua::func<void> &function) {
		auto hCb = hComponent.AddEventCallback(eventId,[l,function](std::reference_wrapper<pragma::ComponentEvent> ev) -> util::EventReply {
			// We need to create a copy of the lua-state pointer, since the callback can remove itself, which
			// would also cause the std::function-object to be destroyed (and therefore the captured variables).
			auto lTmp = l;
			auto oCallbackTmp = function;

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
		return hCb;
	}));
	def.def("InjectEvent",static_cast<util::EventReply(pragma::BaseEntityComponent::*)(pragma::ComponentEventId)>(&pragma::BaseEntityComponent::InjectEvent));
	def.def("InjectEvent",static_cast<void(*)(lua_State*,pragma::BaseEntityComponent&,uint32_t,const luabind::tableT<void>&)>([](lua_State *l,pragma::BaseEntityComponent &hComponent,uint32_t eventId,const luabind::tableT<void> &eventArgs) {
		int32_t t = 3;
		if(engine->GetNetworkState(l)->GetGameState()->InjectEntityEvent(hComponent,eventId,t) == false)
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
			hComponent.InjectEvent(eventId,luaEvent);
		}
	}));
	def.def("Save",&pragma::BaseEntityComponent::Save);
	def.def("Load",static_cast<void(pragma::BaseEntityComponent::*)(udm::LinkedPropertyWrapperArg)>(&pragma::BaseEntityComponent::Load));
	def.def("Copy",static_cast<void(*)(lua_State*,pragma::BaseEntityComponent&,pragma::BaseEntityComponent&)>([](lua_State *l,pragma::BaseEntityComponent &hComponent,pragma::BaseEntityComponent &hComponentOther) {
		if(hComponent.GetComponentId() != hComponentOther.GetComponentId() || &hComponent == &hComponentOther)
			return;
		auto el = udm::Property::Create<udm::Element>();
		udm::LinkedPropertyWrapper prop {*el};
		hComponent.Save(prop);
		hComponentOther.Load(prop);
	}));
	def.add_static_constant("FREGISTER_NONE",umath::to_integral(pragma::ComponentFlags::None));
	def.add_static_constant("FREGISTER_BIT_NETWORKED",umath::to_integral(pragma::ComponentFlags::Networked));

	def.add_static_constant("CALLBACK_TYPE_ENTITY",umath::to_integral(pragma::BaseEntityComponent::CallbackType::Entity));
	def.add_static_constant("CALLBACK_TYPE_COMPONENT",umath::to_integral(pragma::BaseEntityComponent::CallbackType::Component));
}
std::string Lua::BaseEntityComponent::GetName(lua_State *l,pragma::BaseEntityComponent &component)
{
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	auto &componentManager = game->GetEntityComponentManager();
	auto &info = *componentManager.GetComponentInfo(component.GetComponentId());
	return info.name;
}
