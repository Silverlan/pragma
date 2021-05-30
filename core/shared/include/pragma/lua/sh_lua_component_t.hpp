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

namespace Lua
{
	template<class TLuaClass>
		void register_base_entity_component(TLuaClass &classDef)
	{
		classDef.def(luabind::tostring(luabind::self));
		classDef.def(luabind::constructor<>());
		classDef.def("SetNetworked",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&,bool)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent,bool bNetworked) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->SetNetworked(bNetworked);
		}));
		classDef.def("IsNetworked",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::PushBool(l,hComponent->IsNetworked());
		}));
		classDef.def("SetShouldTransmitSnapshotData",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&,bool)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent,bool bSend) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->SetShouldTransmitSnapshotData(bSend);
		}));
		classDef.def("ShouldTransmitSnapshotData",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::PushBool(l,hComponent->ShouldTransmitSnapshotData());
		}));
		classDef.def("GetVersion",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent) {
			pragma::Lua::check_component(l,hComponent);
			Lua::PushInt(l,hComponent->GetVersion());
		}));
		classDef.def("RegisterNetEvent",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&,const std::string&)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent,const std::string &name) {
			pragma::Lua::check_component(l,hComponent);
			Lua::PushInt(l,hComponent->SetupNetEvent(name));
		}));
		classDef.def("FlagCallbackForRemoval",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&,CallbackHandle&,uint32_t,BaseEntityComponentHandle&)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent,CallbackHandle &hCb,uint32_t callbackType,BaseEntityComponentHandle &hComponentOther) {
			pragma::Lua::check_component(l,hComponent);
			pragma::Lua::check_component(l,hComponentOther);
			if(hCb.IsValid() == false)
				return;
			hComponent->FlagCallbackForRemoval(hCb,static_cast<pragma::BaseEntityComponent::CallbackType>(callbackType),hComponentOther.get());
		}));
		classDef.def("FlagCallbackForRemoval",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&,CallbackHandle&,uint32_t)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent,CallbackHandle &hCb,uint32_t callbackType) {
			pragma::Lua::check_component(l,hComponent);
			if(hCb.IsValid() == false)
				return;
			hComponent->FlagCallbackForRemoval(hCb,static_cast<pragma::BaseEntityComponent::CallbackType>(callbackType));
		}));
		classDef.def("BindComponentInitEvent",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&,uint32_t,luabind::object)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent,uint32_t componentId,luabind::object methodNameOrFunction) {
			pragma::Lua::check_component(l,hComponent);
			Lua::Push<CallbackHandle>(l,hComponent->BindInitComponentEvent(l,componentId,methodNameOrFunction));
		}));
		classDef.def("BindEvent",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&,uint32_t,luabind::object)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent,uint32_t eventId,luabind::object methodNameOrFunction) {
			pragma::Lua::check_component(l,hComponent);
			Lua::Push<CallbackHandle>(l,hComponent->BindEvent(l,eventId,methodNameOrFunction));
		}));
		classDef.def("GetEntityComponent",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&,uint32_t)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent,uint32_t componentId) {
			pragma::Lua::check_component(l,hComponent);
			auto c = hComponent->GetEntity().FindComponent(componentId);
			if(c.valid())
				c->PushLuaObject(l);
		}));
		classDef.def("GetEntityComponent",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&,const std::string&)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent,const std::string &componentName) {
			pragma::Lua::check_component(l,hComponent);
			auto c = hComponent->GetEntity().FindComponent(componentName);
			if(c.valid())
				c->PushLuaObject(l);
		}));
		classDef.def("AddEntityComponent",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&,uint32_t)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent,uint32_t componentId) {
			pragma::Lua::check_component(l,hComponent);
			auto c = hComponent->GetEntity().AddComponent(componentId);
			if(c.valid())
				c->PushLuaObject(l);
		}));
		classDef.def("AddEntityComponent",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&,uint32_t,luabind::object)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent,uint32_t componentId,luabind::object methodNameOrFunction) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->BindInitComponentEvent(l,componentId,methodNameOrFunction);
			auto c = hComponent->GetEntity().AddComponent(componentId);
			if(c.valid())
				c->PushLuaObject(l);
		}));
		classDef.def("AddEntityComponent",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&,const std::string&)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent,const std::string &name) {
			pragma::Lua::check_component(l,hComponent);
			auto c = hComponent->GetEntity().AddComponent(name);
			if(c.valid())
				c->PushLuaObject(l);
		}));
		classDef.def("AddEntityComponent",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&,const std::string&,luabind::object)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent,const std::string &name,luabind::object methodNameOrFunction) {
			pragma::Lua::check_component(l,hComponent);
			auto hNewComponent = hComponent->GetEntity().AddComponent(name);
			if(hNewComponent.expired())
				return;
			hComponent->BindInitComponentEvent(l,hNewComponent->GetComponentId(),methodNameOrFunction);
			hNewComponent->PushLuaObject(l);
		}));
		classDef.def("OnMemberValueChanged",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&,uint32_t)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent,uint32_t memberIndex) {
			pragma::Lua::check_component(l,hComponent);
			hComponent->OnMemberValueChanged(memberIndex);
		}));
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
