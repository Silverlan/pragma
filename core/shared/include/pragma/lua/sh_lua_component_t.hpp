/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __SH_LUA_COMPONENT_T_HPP__
#define __SH_LUA_COMPONENT_T_HPP__

#include "pragma/lua/lentity_component_lua.hpp"
#include "pragma/lua/lua_entity_component.hpp"
#include "pragma/lua/sh_lua_component_wrapper.hpp"
#include "pragma/lua/policies/game_object_policy.hpp"

namespace Lua
{
	template<class TComponent,class TDef>
		void register_base_entity_component(TDef &classDef)
	{
		classDef.def(luabind::tostring(luabind::self));
		classDef.def("SetNetworked",&TComponent::SetNetworked);
		classDef.def("IsNetworked",&TComponent::IsNetworked);
		classDef.def("SetShouldTransmitSnapshotData",&TComponent::SetShouldTransmitSnapshotData);
		classDef.def("ShouldTransmitSnapshotData",&TComponent::ShouldTransmitSnapshotData);
		classDef.def("GetVersion",&TComponent::GetVersion);
		classDef.def("RegisterNetEvent",&TComponent::SetupNetEvent);
		classDef.def("FlagCallbackForRemoval",static_cast<void(*)(lua_State*,TComponent&,CallbackHandle&,pragma::BaseEntityComponent::CallbackType,TComponent&)>([](lua_State *l,TComponent &hComponent,CallbackHandle &hCb,pragma::BaseEntityComponent::CallbackType callbackType,TComponent &hComponentOther) {
			if(hCb.IsValid() == false)
				return;
			hComponent.FlagCallbackForRemoval(hCb,callbackType,&hComponentOther);
		}));
		classDef.def("FlagCallbackForRemoval",static_cast<void(*)(lua_State*,TComponent&,CallbackHandle&,pragma::BaseEntityComponent::CallbackType)>([](lua_State *l,TComponent &hComponent,CallbackHandle &hCb,pragma::BaseEntityComponent::CallbackType callbackType) {
			if(hCb.IsValid() == false)
				return;
			hComponent.FlagCallbackForRemoval(hCb,callbackType);
		}));
		classDef.def("BindComponentInitEvent",static_cast<CallbackHandle(*)(lua_State*,TComponent&,uint32_t,luabind::object)>([](lua_State *l,TComponent &hComponent,uint32_t componentId,luabind::object methodNameOrFunction) {
			return hComponent.BindInitComponentEvent(l,componentId,methodNameOrFunction);
		}));
		classDef.def("BindEvent",static_cast<CallbackHandle(*)(lua_State*,TComponent&,uint32_t,luabind::object)>([](lua_State *l,TComponent &hComponent,uint32_t eventId,luabind::object methodNameOrFunction) {
			return hComponent.BindEvent(l,eventId,methodNameOrFunction);
		}));
		classDef.def("GetEntityComponent",static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent>(*)(lua_State*,TComponent&,uint32_t)>([](lua_State *l,TComponent &hComponent,uint32_t componentId) -> pragma::ComponentHandle<pragma::BaseEntityComponent> {
			return hComponent.GetEntity().FindComponent(componentId);
		}),luabind::game_object_policy<0>{});
		classDef.def("GetEntityComponent",static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent>(*)(lua_State*,TComponent&,const std::string&)>([](lua_State *l,TComponent &hComponent,const std::string &componentName) -> pragma::ComponentHandle<pragma::BaseEntityComponent> {
			return hComponent.GetEntity().FindComponent(componentName);
		}),luabind::game_object_policy<0>{});
		classDef.def("AddEntityComponent",static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent>(*)(lua_State*,TComponent&,uint32_t)>([](lua_State *l,TComponent &hComponent,uint32_t componentId) -> pragma::ComponentHandle<pragma::BaseEntityComponent> {
			return hComponent.GetEntity().AddComponent(componentId);
		}),luabind::game_object_policy<0>{});
		classDef.def("AddEntityComponent",static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent>(*)(lua_State*,TComponent&,uint32_t,luabind::object)>([](lua_State *l,TComponent &hComponent,uint32_t componentId,luabind::object methodNameOrFunction) -> pragma::ComponentHandle<pragma::BaseEntityComponent> {
			hComponent.BindInitComponentEvent(l,componentId,methodNameOrFunction);
			return hComponent.GetEntity().AddComponent(componentId);
		}),luabind::game_object_policy<0>{});
		classDef.def("AddEntityComponent",static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent>(*)(lua_State*,TComponent&,const std::string&)>([](lua_State *l,TComponent &hComponent,const std::string &name) -> pragma::ComponentHandle<pragma::BaseEntityComponent> {
			return hComponent.GetEntity().AddComponent(name);
		}),luabind::game_object_policy<0>{});
		classDef.def("AddEntityComponent",static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent>(*)(lua_State*,TComponent&,const std::string&,luabind::object)>([](lua_State *l,TComponent &hComponent,const std::string &name,luabind::object methodNameOrFunction) -> pragma::ComponentHandle<pragma::BaseEntityComponent> {
			auto hNewComponent = hComponent.GetEntity().AddComponent(name);
			if(hNewComponent.expired())
				return hNewComponent;
			hComponent.BindInitComponentEvent(l,hNewComponent->GetComponentId(),methodNameOrFunction);
			return hNewComponent;
		}),luabind::game_object_policy<0>{});
		classDef.def("OnMemberValueChanged",&TComponent::OnMemberValueChanged);
		classDef.scope[luabind::def("RegisterMember",static_cast<void(*)(lua_State*,luabind::object,const std::string&,uint32_t,luabind::object,uint32_t,uint32_t)>([](lua_State *l,luabind::object o,const std::string &memberName,uint32_t memberType,luabind::object oDefault,uint32_t memberFlags,uint32_t version) {
			auto anyInitialValue = Lua::GetAnyValue(l,static_cast<::util::VarType>(memberType),4);
			pragma::BaseLuaBaseEntityComponent::RegisterMember(o,memberName,static_cast<::util::VarType>(memberType),anyInitialValue,static_cast<pragma::BaseLuaBaseEntityComponent::MemberFlags>(memberFlags),version);
		}))];
		classDef.scope[luabind::def("RegisterMember",static_cast<void(*)(lua_State*,luabind::object,const std::string&,uint32_t,luabind::object,uint32_t)>([](lua_State *l,luabind::object o,const std::string &memberName,uint32_t memberType,luabind::object oDefault,uint32_t memberFlags) {
			auto anyInitialValue = Lua::GetAnyValue(l,static_cast<::util::VarType>(memberType),4);
			pragma::BaseLuaBaseEntityComponent::RegisterMember(o,memberName,static_cast<::util::VarType>(memberType),anyInitialValue,static_cast<pragma::BaseLuaBaseEntityComponent::MemberFlags>(memberFlags),0u);
		}))];
		classDef.scope[luabind::def("RegisterMember",static_cast<void(*)(lua_State*,luabind::object,const std::string&,uint32_t,luabind::object)>([](lua_State *l,luabind::object o,const std::string &memberName,uint32_t memberType,luabind::object oDefault) {
			auto anyInitialValue = Lua::GetAnyValue(l,static_cast<::util::VarType>(memberType),4);
			pragma::BaseLuaBaseEntityComponent::RegisterMember(o,memberName,static_cast<::util::VarType>(memberType),anyInitialValue,pragma::BaseLuaBaseEntityComponent::MemberFlags::Default,0u);
		}))];

		classDef.add_static_constant("MEMBER_FLAG_NONE",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::None));
		classDef.add_static_constant("MEMBER_FLAG_BIT_PROPERTY",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::PropertyBit));
		classDef.add_static_constant("MEMBER_FLAG_BIT_GETTER",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::GetterBit));
		classDef.add_static_constant("MEMBER_FLAG_BIT_SETTER",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::SetterBit));
		classDef.add_static_constant("MEMBER_FLAG_BIT_STORE",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::StoreBit));
		classDef.add_static_constant("MEMBER_FLAG_BIT_KEY_VALUE",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::KeyValueBit));
		classDef.add_static_constant("MEMBER_FLAG_BIT_INPUT",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::InputBit));
		classDef.add_static_constant("MEMBER_FLAG_BIT_OUTPUT",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::OutputBit));
		classDef.add_static_constant("MEMBER_FLAG_BIT_NETWORKED",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::NetworkedBit));
		classDef.add_static_constant("MEMBER_FLAG_BIT_USE_HAS_GETTER",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::UseHasGetterBit));
		classDef.add_static_constant("MEMBER_FLAG_BIT_USE_IS_GETTER",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::UseIsGetterBit));
		
		classDef.add_static_constant("MEMBER_FLAG_TRANSMIT_ON_CHANGE",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::TransmitOnChange));
		classDef.add_static_constant("MEMBER_FLAG_SNAPSHOT_DATA",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::SnapshotData));
		classDef.add_static_constant("MEMBER_FLAG_DEFAULT",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::Default));
		classDef.add_static_constant("MEMBER_FLAG_DEFAULT_NETWORKED",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::DefaultNetworked));
		classDef.add_static_constant("MEMBER_FLAG_DEFAULT_TRANSMIT",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::DefaultTransmit));
		classDef.add_static_constant("MEMBER_FLAG_DEFAULT_SNAPSHOT",umath::to_integral(pragma::BaseLuaBaseEntityComponent::MemberFlags::DefaultSnapshot));

		classDef.def("Initialize",&LuaBaseEntityComponentWrapper::Initialize,&LuaBaseEntityComponentWrapper::default_Initialize);
		classDef.def("OnTick",&LuaBaseEntityComponentWrapper::OnTick,&LuaBaseEntityComponentWrapper::default_OnTick);
		classDef.def("OnRemove",&LuaBaseEntityComponentWrapper::OnRemove,&LuaBaseEntityComponentWrapper::default_OnRemove);
		classDef.def("OnEntitySpawn",&LuaBaseEntityComponentWrapper::OnEntitySpawn,&LuaBaseEntityComponentWrapper::default_OnEntitySpawn);
		classDef.def("OnAttachedToEntity",&LuaBaseEntityComponentWrapper::OnAttachedToEntity,&LuaBaseEntityComponentWrapper::default_OnAttachedToEntity);
		classDef.def("OnDetachedFromEntity",&LuaBaseEntityComponentWrapper::OnDetachedToEntity,&LuaBaseEntityComponentWrapper::default_OnDetachedToEntity);
		// classDef.def("OnEntityComponentAdded",&LuaBaseEntityComponentWrapper::OnEntityComponentAdded,&LuaBaseEntityComponentWrapper::default_OnEntityComponentAdded);
		// classDef.def("OnEntityComponentRemoved",&LuaBaseEntityComponentWrapper::OnEntityComponentRemoved,&LuaBaseEntityComponentWrapper::default_OnEntityComponentRemoved);
		// HandleEvent is variadic and can't be defined like this in luabind!
		//classDef.def("HandleEvent",&LuaBaseEntityComponentWrapper::HandleEvent,&LuaBaseEntityComponentWrapper::default_HandleEvent);
		classDef.def("Save",&LuaBaseEntityComponentWrapper::Save,&LuaBaseEntityComponentWrapper::default_Save);
		classDef.def("Load",&LuaBaseEntityComponentWrapper::Load,&LuaBaseEntityComponentWrapper::default_Load);
	}
};

#endif
