/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/lua/sh_lua_component.hpp"
#include "pragma/lua/sh_lua_component_wrapper.hpp"
#include "pragma/lua/lua_call.hpp"
#include "pragma/lua/classes/lproperty.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include "pragma/entities/entity_iterator.hpp"
#include <sharedutils/scope_guard.h>
#include <sharedutils/datastream.h>
#include <sharedutils/netpacket.hpp>

using namespace pragma;


struct ClassMembers
{
	ClassMembers(const luabind::object &classObject)
		: classObject{classObject}
	{}
	luabind::object classObject;
	std::vector<BaseLuaBaseEntityComponent::MemberInfo> memberDeclarations;
};
static std::unordered_map<lua_State*,std::vector<ClassMembers>> s_classMembers {};
static std::vector<ClassMembers> &get_class_member_list(lua_State *l)
{
	auto it = s_classMembers.find(l);
	if(it == s_classMembers.end())
		it = s_classMembers.insert(std::make_pair(l,std::vector<ClassMembers>{})).first;
	return it->second;
}
static ClassMembers *get_class_member_declarations(const luabind::object &classObject)
{
	auto *l = classObject.interpreter();
	auto it = s_classMembers.find(l);
	if(it == s_classMembers.end())
		return nullptr;
	auto itClass = std::find_if(it->second.begin(),it->second.end(),[&classObject](const ClassMembers &classMember) {
		return classObject == classMember.classObject;
	});
	return (itClass != it->second.end()) ? &(*itClass) : nullptr;
}

static std::string get_member_variable_name(const std::string &memberName)
{
	if(memberName.empty())
		return "";
	std::stringstream ss {};
	ss<<"m_"<<std::string(1,tolower(memberName.front()))<<ustring::substr(memberName,1ull);
	return ss.str();
}
static std::any string_to_any(Game &game,const std::string &value,util::VarType type)
{
	static_assert(umath::to_integral(util::VarType::Count) == 21);
	switch(type)
	{
		case util::VarType::Bool:
			return static_cast<bool>(util::to_boolean(value));
		case util::VarType::Double:
			return static_cast<double>(util::to_float(value));
		case util::VarType::Float:
			return static_cast<float>(util::to_float(value));
		case util::VarType::Int8:
			return static_cast<int8_t>(util::to_int(value));
		case util::VarType::Int16:
			return static_cast<int16_t>(util::to_int(value));
		case util::VarType::Int32:
			return static_cast<int32_t>(util::to_int(value));
		case util::VarType::Int64:
			return static_cast<int64_t>(util::to_int(value));
		case util::VarType::LongDouble:
			return static_cast<long double>(util::to_int(value));
		case util::VarType::String:
			return value;
		case util::VarType::UInt8:
			return static_cast<uint8_t>(util::to_int(value));
		case util::VarType::UInt16:
			return static_cast<uint16_t>(util::to_int(value));
		case util::VarType::UInt32:
			return static_cast<uint32_t>(util::to_int(value));
		case util::VarType::UInt64:
			return static_cast<uint64_t>(util::to_int(value));
		case util::VarType::EulerAngles:
			return EulerAngles{value};
		case util::VarType::Color:
			return Color{value};
		case util::VarType::Vector:
			return uvec::create(value);
		case util::VarType::Vector2:
		{
			Vector2 r;
			ustring::string_to_array<Float,Double>(value,reinterpret_cast<float*>(&r),atof,2);
			return r;
		}
		case util::VarType::Vector4:
		{
			Vector4 r;
			ustring::string_to_array<Float,Double>(value,reinterpret_cast<float*>(&r),atof,4);
			return r;
		}
		case util::VarType::Entity:
		{
			EntityIterator entIt {game};
			entIt.AttachFilter<EntityIteratorFilterName>(value);
			auto it = entIt.begin();
			auto *ent = (it != entIt.end()) ? *it : nullptr;
			auto hEnt = (ent != nullptr) ? ent->GetHandle() : EntityHandle{};
			return hEnt;
		}
		case util::VarType::Quaternion:
		{
			Quat r;
			ustring::string_to_array<Float,Double>(value,reinterpret_cast<float*>(&r),atof,4);
			return r;
		}
	}
	return {};
}
void BaseLuaBaseEntityComponent::RegisterMember(const luabind::object &oClass,const std::string &memberName,util::VarType memberType,const std::any &initialValue,MemberFlags memberFlags,uint32_t version)
{
	if(memberName.empty())
		return;
	auto *l = oClass.interpreter();
	if((memberFlags &MemberFlags::StoreBit) != MemberFlags::None && version == 0u)
	{
		std::string err = "If store flag is set, version number mustn't be 0! Please check 'lua_help ents.BaseEntityComponent.RegisterMember' for more information!";
		lua_pushstring(l,err.c_str());
		lua_error(l);
		return;
	}
	auto lmemberName = get_member_variable_name(memberName);

	auto &members = get_class_member_list(l);
	auto it = std::find_if(members.begin(),members.end(),[&oClass](const ClassMembers &classMembers) {
		return oClass == classMembers.classObject;
	});
	if(it == members.end())
	{
		members.push_back({oClass});
		it = members.end() -1;
	}
	auto itMember = std::find_if(it->memberDeclarations.begin(),it->memberDeclarations.end(),[&memberName](const MemberInfo &memberInfo) {
		return ustring::compare(memberName,memberInfo.name,false);
	});
	if(itMember != it->memberDeclarations.end())
	{
		Con::cwar<<"Member '"<<memberName<<"' already defined for entity component! Skipping..."<<Con::endl;
		return;
	}
	it->memberDeclarations.push_back({memberName,memberType,initialValue,memberFlags,version});

	std::string getterName = "Get";
	auto bProperty = (memberFlags &MemberFlags::PropertyBit) != MemberFlags::None;
	if((memberFlags &MemberFlags::GetterBit) != MemberFlags::None)
	{
		std::string getter = "function(self) return self." +lmemberName;
		if(bProperty)
			getter += ":Get()";
		getter += " end";

		std::string err;
		if(Lua::PushLuaFunctionFromString(l,getter,"EntityComponentGetter",err) == false)
			Con::cwar<<"WARNING: Unable to register getter-function for member '"<<memberName<<"' for entity component: "<<err<<Con::endl;
		else
		{
			if((memberFlags &MemberFlags::UseHasGetterBit) != MemberFlags::None)
				getterName = "Has";
			else if((memberFlags &MemberFlags::UseIsGetterBit) != MemberFlags::None)
				getterName = "Is";

			auto idxFunc = Lua::GetStackTop(l);
			oClass.push(l); /* 2 */
			auto idxObject = Lua::GetStackTop(l);
			Lua::PushString(l,getterName +memberName); /* 3 */
			Lua::PushValue(l,idxFunc); /* 4 */
			Lua::SetTableValue(l,idxObject); /* 2 */
			Lua::Pop(l,2); /* 0 */
		}

		if(bProperty)
		{
			std::string getterProperty = "function(self) return self." +lmemberName +" end";
			std::string err;
			if(Lua::PushLuaFunctionFromString(l,getterProperty,"EntityComponentGetterProperty",err) == false)
				Con::cwar<<"WARNING: Unable to register property-get-function for member '"<<memberName<<"' for entity component: "<<err<<Con::endl;
			else
			{
				auto idxFunc = Lua::GetStackTop(l);
				oClass.push(l); /* 2 */
				auto idxObject = Lua::GetStackTop(l);
				Lua::PushString(l,"Get" +memberName +"Property"); /* 3 */
				Lua::PushValue(l,idxFunc); /* 4 */
				Lua::SetTableValue(l,idxObject); /* 2 */
				Lua::Pop(l,2); /* 0 */
			}
		}
	}
	if((memberFlags &MemberFlags::SetterBit) != MemberFlags::None)
	{
		auto bTransmit = (memberFlags &MemberFlags::TransmitOnChange) != MemberFlags::None;
		auto bSetterValid = true;
		std::string setter = "function(self,value) ";
		if(bTransmit)
			setter += "if(value == self:" +getterName +memberName +"()) then return end ";
		setter += "self." +lmemberName;
		if(bProperty)
			setter += ":Set(value)";
		else
			setter += " = value";
		if((memberFlags &MemberFlags::TransmitOnChange) == MemberFlags::TransmitOnChange || (memberFlags &MemberFlags::OutputBit) != MemberFlags::None)
			setter += " self:OnMemberValueChanged(" +std::to_string(it->memberDeclarations.size() -1u) +")";
		setter += " end";
		std::string err;
		if(Lua::PushLuaFunctionFromString(l,setter,"EntityComponentSetter",err) == false)
		{
			bSetterValid = false;
			Con::cwar<<"WARNING: Unable to register setter-function for member '"<<memberName<<"' for entity component: "<<err<<Con::endl;
		} /* else 1 */
		if(bSetterValid)
		{
			auto idxFunc = Lua::GetStackTop(l);
			oClass.push(l); /* 2 */
			auto idxObject = Lua::GetStackTop(l);
			Lua::PushString(l,"Set" +memberName); /* 3 */
			Lua::PushValue(l,idxFunc); /* 4 */
			Lua::SetTableValue(l,idxObject); /* 2 */
			Lua::Pop(l,2); /* 0 */
		}
	}
}
void BaseLuaBaseEntityComponent::OnMemberValueChanged(uint32_t memberIdx)
{
	auto &members = GetMembers();
	if(memberIdx >= members.size())
		return;
	auto &member = members.at(memberIdx);
	if((member.flags &MemberFlags::OutputBit) != MemberFlags::None)
	{
		auto *pIoComponent = static_cast<pragma::BaseIOComponent*>(GetEntity().FindComponent("io").get());
		if(pIoComponent != nullptr)
			pIoComponent->TriggerOutput("OnSet" +member.name,&GetEntity());
	}
}
std::vector<BaseLuaBaseEntityComponent::MemberInfo> *BaseLuaBaseEntityComponent::GetMemberInfos(const luabind::object &oClass)
{
	auto *p = get_class_member_declarations(oClass);
	return (p != nullptr) ? &p->memberDeclarations : nullptr;
}
void BaseLuaBaseEntityComponent::ClearMembers(lua_State *l)
{
	auto it = s_classMembers.find(l);
	if(it == s_classMembers.end())
		return;
	s_classMembers.erase(it);
}

////////////////

BaseLuaBaseEntityComponent::BaseLuaBaseEntityComponent(BaseEntity &ent,luabind::object &o)
	: pragma::BaseEntityComponent(ent),LuaObjectBase(o)
{
	m_luaObj = o;
}

void BaseLuaBaseEntityComponent::Initialize()
{
	// The underlying handle for the lua object has not yet been assigned to our shared ptr, so we have to do it now.
	// This has to be done before any Lua member-functions are called, but can't be done inside the constructor, because
	// at that point the object hasn't been assigned to a shared ptr yet!
	auto *pHandleWrapper = luabind::object_cast_nothrow<BaseLuaBaseEntityComponentHandleWrapper*>(GetLuaObject(),static_cast<BaseLuaBaseEntityComponentHandleWrapper*>(nullptr));
	if(pHandleWrapper != nullptr)
		pHandleWrapper->handle = util::WeakHandle<pragma::BaseEntityComponent>(shared_from_this());
	
	auto &game = *GetEntity().GetNetworkState()->GetGameState();
	auto &componentManager = game.GetEntityComponentManager();
	if((componentManager.GetComponentInfo(GetComponentId())->flags &pragma::ComponentFlags::Networked) != pragma::ComponentFlags::None)
		SetNetworked(true);

	InitializeLuaObject();
	auto *l = GetLuaState();

	auto *pInfo = componentManager.GetComponentInfo(GetComponentId());
	if(pInfo != nullptr)
	{
		auto &luaEntityManager = game.GetLuaEntityManager();
		auto *o = luaEntityManager.GetComponentClassObject(pInfo->name);
		if(o != nullptr)
		{
			auto *p = get_class_member_declarations(*o);
			if(p != nullptr)
				InitializeMembers(p->memberDeclarations);
		}
	}
	if(m_networkedMemberInfo != nullptr)
		m_networkedMemberInfo->netEvSetMember = SetupNetEvent("set_member_value");

	CallLuaMember("Initialize");

	PushLuaObject(); /* 1 */
	auto t = Lua::GetStackTop(l);
	Lua::PushString(l,"Version"); /* 2 */
	Lua::GetTableValue(l,t); /* 2 */
	if(Lua::IsSet(l,-1))
		m_version = Lua::CheckInt(l,-1);
	Lua::Pop(l,2); /* 0 */

	auto &ent = GetEntity();
	CallLuaMember("OnAttachedToEntity");
	pragma::BaseEntityComponent::Initialize();
}

void BaseLuaBaseEntityComponent::InitializeMembers(const std::vector<BaseLuaBaseEntityComponent::MemberInfo> &members)
{
	m_members = members;
	auto &o = GetLuaObject();
	auto *l = o.interpreter();
	o.push(l); /* 1 */
	auto t = Lua::GetStackTop(l);
	auto totalMemberFlags = MemberFlags::None;
	auto idxMember = 0u;
	for(auto &member : members)
	{
		auto memberVarName = get_member_variable_name(member.name);
		Lua::PushString(l,memberVarName); /* 2 */

		if((member.flags &MemberFlags::PropertyBit) != MemberFlags::None)
			Lua::PushNewAnyProperty(l,member.type,member.initialValue); /* 3 */
		else
			Lua::PushAny(l,member.type,member.initialValue); /* 3 */
		if(Lua::IsNil(l,-1))
			Con::cwar<<"WARNING: Invalid member type "<<umath::to_integral(member.type)<<" for member '"<<member.name<<"' of entity component '"<<GetEntity().GetNetworkState()->GetGameState()->GetEntityComponentManager().GetComponentInfo(GetComponentId())->name<<"'! Ignoring..."<<Con::endl;
		Lua::SetTableValue(l,t); /* 1 */

		if((member.flags &MemberFlags::NetworkedBit) != MemberFlags::None)
		{
			if(m_networkedMemberInfo == nullptr)
				m_networkedMemberInfo = std::unique_ptr<NetworkedMemberInfo>(new NetworkedMemberInfo{});
			m_networkedMemberInfo->memberIndexToNetworkedIndex[idxMember] = m_networkedMemberInfo->networkedMembers.size();
			m_networkedMemberInfo->networkedMembers.push_back(idxMember);

			if((member.flags &MemberFlags::SnapshotData) != MemberFlags::None)
				m_networkedMemberInfo->snapshotMembers.push_back(idxMember);
		}

		totalMemberFlags |= member.flags;
		InitializeMember(member);

		if((member.flags &(MemberFlags::KeyValueBit | MemberFlags::InputBit)) != MemberFlags::None)
		{
			// We only need this for quick keyvalue or input lookups
			auto lmemberName = member.name;
			ustring::to_lower(lmemberName);
			m_memberNameToIndex[lmemberName] = idxMember;
		}

		++idxMember;
	}
	Lua::Pop(l,1); /* 0 */

	if((totalMemberFlags &MemberFlags::KeyValueBit) != MemberFlags::None)
	{
		BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](const std::reference_wrapper<pragma::ComponentEvent> &evData) -> util::EventReply {
			auto &kvData = static_cast<pragma::CEKeyValueData&>(evData.get());
			auto it = m_memberNameToIndex.find(kvData.key);
			if(it == m_memberNameToIndex.end())
				return util::EventReply::Unhandled;
			auto &member = m_members.at(it->second);
			SetMemberValue(member,string_to_any(*GetEntity().GetNetworkState()->GetGameState(),kvData.value,member.type));
			return util::EventReply::Handled;
		});
	}
	if((totalMemberFlags &MemberFlags::InputBit) != MemberFlags::None)
	{
		BindEvent(pragma::BaseIOComponent::EVENT_HANDLE_INPUT,[this](const std::reference_wrapper<pragma::ComponentEvent> &evData) -> util::EventReply {
			auto &ioData = static_cast<pragma::CEInputData&>(evData.get());
			if(strncmp(ioData.input.data(),"set",3u) != 0)
				return util::EventReply::Unhandled;
			auto input = ioData.input.substr(3u);
			auto it = m_memberNameToIndex.find(input);
			if(it == m_memberNameToIndex.end())
				return util::EventReply::Unhandled;
			auto &member = m_members.at(it->second);
			SetMemberValue(member,string_to_any(*GetEntity().GetNetworkState()->GetGameState(),ioData.data,member.type));
			return util::EventReply::Handled;
		});
	}
}

void BaseLuaBaseEntityComponent::InitializeMember(const MemberInfo &memberInfo) {}

const luabind::object &BaseLuaBaseEntityComponent::GetLuaObject() const {return pragma::BaseEntityComponent::GetLuaObject();}
luabind::object &BaseLuaBaseEntityComponent::GetLuaObject() {return pragma::BaseEntityComponent::GetLuaObject();}

CallbackHandle BaseLuaBaseEntityComponent::BindInitComponentEvent(ComponentId componentId,const std::function<void(std::reference_wrapper<pragma::BaseEntityComponent>)> &callback)
{
	auto it = m_initComponentCallbacks.find(componentId);
	if(it != m_initComponentCallbacks.end())
	{
		if(it->second.IsValid())
			it->second.Remove();
		m_initComponentCallbacks.erase(it);
	}
	it = m_initComponentCallbacks.insert(std::make_pair(componentId,FunctionCallback<void,std::reference_wrapper<pragma::BaseEntityComponent>>::Create(callback))).first;
	auto pComponent = GetEntity().FindComponent(componentId);
	if(pComponent.valid())
		callback(std::reference_wrapper<pragma::BaseEntityComponent>(*pComponent));
	return it->second;
}

util::EventReply BaseLuaBaseEntityComponent::HandleEvent(ComponentEventId eventId,ComponentEvent &evData)
{
	if(BaseEntityComponent::HandleEvent(eventId,evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == pragma::BaseEntityComponent::EVENT_ON_ENTITY_COMPONENT_ADDED)
	{
		auto &componentAddedData = static_cast<pragma::CEOnEntityComponentAdded&>(evData);
		auto it = m_initComponentCallbacks.find(componentAddedData.component.GetComponentId());
		if(it != m_initComponentCallbacks.end())
		{
			auto &hCb = it->second;
			if(hCb.IsValid() == false)
				m_initComponentCallbacks.erase(it);
			else
				hCb.Call<void,std::reference_wrapper<pragma::BaseEntityComponent>>(componentAddedData.component);
		}
	}

	// Deprecated: Events always have to be explicitely registered by Lua-components
	// to avoid the overhead of pushing arguments for events that probably won't even
	// be used by the Lua-script.
	/*
	// CallLuaMember<void,uint32_t>("HandleEvent",eventId);
	// CallLuaMember does not work with a variable number of arguments,
	// so we have to call the method manually.
	// See also Lua::register_base_entity_component -> BindEvent
	auto o = LuaObjectBase::GetLuaObject();
	auto r = o["HandleEvent"];
	if(r)
	{
		auto *l = o.interpreter();
		o.push(l);
		Lua::PushString(l,"HandleEvent");
		Lua::GetTableValue(l,-2);
		if(Lua::IsFunction(l,-1))
		{
			// duplicate the self-object
			Lua::PushValue(l,-2);
			// remove the bottom self-object
			Lua::RemoveValue(l,-3);

			auto n = Lua::GetStackTop(l);
			Lua::PushInt(l,eventId);
			evData.PushArguments(l);
			auto numArgs = (Lua::GetStackTop(l) -n) +1u; // +1 because of self-object
			auto c = Lua::CallFunction(l,numArgs,evData.GetReturnCount());
			if(c != Lua::StatusCode::Ok)
				Lua::HandleLuaError(l,c);
			else
				evData.HandleReturnValues(l);
		}
	}*/
	return util::EventReply::Unhandled;
}

void BaseLuaBaseEntityComponent::OnAttached(BaseEntity &ent)
{
	pragma::BaseEntityComponent::OnAttached(ent);
	CallLuaMember("OnAttachedToEntity");
}
void BaseLuaBaseEntityComponent::OnDetached(BaseEntity &ent)
{
	pragma::BaseEntityComponent::OnDetached(ent);
	CallLuaMember("OnDetachedToEntity");
}

std::any BaseLuaBaseEntityComponent::GetMemberValue(const MemberInfo &memberInfo) const
{
	auto &o = const_cast<BaseLuaBaseEntityComponent*>(this)->GetLuaObject();
	auto *l = o.interpreter();
	o.push(l); /* 1 */
	auto t = Lua::GetStackTop(l);
	Lua::PushString(l,get_member_variable_name(memberInfo.name)); /* 2 */
	Lua::GetTableValue(l,t); /* 2 */
	if((memberInfo.flags &MemberFlags::PropertyBit) == MemberFlags::None)
	{
		auto value = Lua::GetAnyValue(l,memberInfo.type,-1);
		Lua::Pop(l,2); /* 0 */
		return value;
	}
	auto value = Lua::GetAnyPropertyValue(l,-1,memberInfo.type);
	Lua::Pop(l,2); /* 0 */
	return value;
}

void BaseLuaBaseEntityComponent::SetMemberValue(const MemberInfo &memberInfo,const std::any &value) const
{
	auto &o = const_cast<BaseLuaBaseEntityComponent*>(this)->GetLuaObject();
	auto *l = o.interpreter();
	o.push(l); /* 1 */
	auto t = Lua::GetStackTop(l);
	Lua::PushString(l,get_member_variable_name(memberInfo.name)); /* 2 */
	if((memberInfo.flags &MemberFlags::PropertyBit) != MemberFlags::None)
	{
		Lua::GetTableValue(l,t); /* 2 */
		Lua::SetAnyPropertyValue(l,-1,memberInfo.type,value); /* 2 */
		Lua::Pop(l,1); /* 1 */
	}
	else
	{
		Lua::PushAny(l,memberInfo.type,value); /* 3 */
		Lua::SetTableValue(l,t); /* 1 */
	}
	Lua::Pop(l,1); /* 0 */
}

const std::vector<BaseLuaBaseEntityComponent::MemberInfo> &BaseLuaBaseEntityComponent::GetMembers() const {return m_members;}
void BaseLuaBaseEntityComponent::Save(DataStream &ds)
{
	pragma::BaseEntityComponent::Save(ds);
	for(auto &member : m_members)
	{
		if((member.flags &MemberFlags::StoreBit) == MemberFlags::None)
			continue;
		auto value = GetMemberValue(member);
		if(member.type == util::VarType::Entity)
		{
			// If this is an entity it doesn't make sense to store an entity handle, so we'll store the entity's name instead
			auto hEnt = std::any_cast<EntityHandle>(value);
			std::string name {};
			if(hEnt.IsValid())
			{
				auto *pNameComponent = static_cast<pragma::BaseNameComponent*>(hEnt->FindComponent("name").get());
				if(pNameComponent != nullptr)
					name = pNameComponent->GetName();
			}
			ds->WriteString(name);
		}
		else
			Lua::WriteAny(ds,member.type,value);
	}
	CallLuaMember<void,DataStream>("Save",ds);
}
void BaseLuaBaseEntityComponent::Load(DataStream &ds,uint32_t version)
{
	pragma::BaseEntityComponent::Load(ds,version);
	auto &game = *GetEntity().GetNetworkState()->GetGameState();
	for(auto &member : m_members)
	{
		if((member.flags &MemberFlags::StoreBit) == MemberFlags::None || version < member.version)
			continue;
		std::any value;
		if(member.type == util::VarType::Entity)
		{
			auto name = ds->ReadString();
			EntityIterator entIt {game,EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending};
			entIt.AttachFilter<EntityIteratorFilterName>(name);
			auto it = entIt.begin();
			auto *ent = (it != entIt.end()) ? *it : nullptr;
			value = (ent != nullptr) ? ent->GetHandle() : EntityHandle{};
		}
		else
			Lua::ReadAny(game,ds,member.type,value);
		SetMemberValue(member,value);
	}
	CallLuaMember<void,DataStream,uint32_t>("Load",ds,version);
}
uint32_t BaseLuaBaseEntityComponent::GetVersion() const {return m_version;}

void BaseLuaBaseEntityComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	CallLuaMember("OnEntitySpawn");
}

void BaseLuaBaseEntityComponent::OnRemove()
{
	pragma::BaseEntityComponent::OnRemove();
	CallLuaMember("OnRemove");
}

void BaseLuaBaseEntityComponent::SetNetworked(bool b) {m_bShouldTransmitNetData = b;}
bool BaseLuaBaseEntityComponent::IsNetworked() const {return m_bShouldTransmitNetData;}
void BaseLuaBaseEntityComponent::SetShouldTransmitSnapshotData(bool b) {m_bShouldTransmitSnapshotData = b;}
bool BaseLuaBaseEntityComponent::ShouldTransmitSnapshotData() const {return m_bShouldTransmitSnapshotData;}

luabind::object BaseLuaBaseEntityComponent::InitializeLuaObject(lua_State *l)
{
	throw std::logic_error("Attempted to re-initialize lua object for lua entity component!");
	return BaseEntityComponent::InitializeLuaObject<LuaBaseEntityComponentWrapper>(l);
}

CallbackHandle BaseLuaBaseEntityComponent::BindInitComponentEvent(lua_State *l,pragma::ComponentId componentId,luabind::object methodNameOrFunction)
{
	methodNameOrFunction.push(l);
	util::ScopeGuard sg([l]() {
		Lua::Pop(l,1);
	});

	auto argTarget = -1;
	if(Lua::IsString(l,argTarget))
	{
		std::string methodName = Lua::CheckString(l,argTarget);
		auto hCb = BindInitComponentEvent(componentId,[this,methodName](std::reference_wrapper<pragma::BaseEntityComponent> component) {
			CallLuaMember<void,luabind::object>(methodName,component.get().GetLuaObject());
		});
		Lua::Push<CallbackHandle>(l,hCb);
		return hCb;
	}
	Lua::CheckFunction(l,argTarget);
	auto hCb = BindInitComponentEvent(componentId,[methodNameOrFunction](std::reference_wrapper<pragma::BaseEntityComponent> component) {
		auto &fc = methodNameOrFunction;
		auto *l = fc.interpreter();
		auto c = Lua::CallFunction(l,[&fc,&component](lua_State *l) -> Lua::StatusCode {
			fc.push(l);
			component.get().PushLuaObject(l);
			return Lua::StatusCode::Ok;
		},0);
	});
	Lua::Push<CallbackHandle>(l,hCb);
	return hCb;
}
CallbackHandle BaseLuaBaseEntityComponent::BindNetEvent(pragma::NetEventId eventId,const std::function<void(std::reference_wrapper<NetPacket>,pragma::BasePlayerComponent*)> &callback)
{
	auto it = m_boundNetEvents.find(eventId);
	if(it != m_boundNetEvents.end())
	{
		if(it->second.IsValid())
			it->second.Remove();
		m_boundNetEvents.erase(it);
	}
	auto hCb = FunctionCallback<void,std::reference_wrapper<NetPacket>,pragma::BasePlayerComponent*>::Create(callback);
	m_boundNetEvents.insert(std::make_pair(eventId,hCb));
	return hCb;
}
CallbackHandle BaseLuaBaseEntityComponent::BindNetEvent(lua_State *l,pragma::NetEventId eventId,luabind::object methodNameOrFunction)
{
	methodNameOrFunction.push(l);
	util::ScopeGuard sg([l]() {
		Lua::Pop(l,1);
	});

	auto argTarget = -1;
	if(Lua::IsString(l,argTarget))
	{
		std::string methodName = Lua::CheckString(l,argTarget);
		auto hCb = BindNetEvent(eventId,[this,methodName](std::reference_wrapper<NetPacket> packet,pragma::BasePlayerComponent *pl) {
			InvokeNetEventHandle(methodName,packet,pl);
		});
		Lua::Push<CallbackHandle>(l,hCb);
		return hCb;
	}
	Lua::CheckFunction(l,argTarget);
	auto hCb = BindNetEvent(eventId,[methodNameOrFunction](std::reference_wrapper<NetPacket> packet,pragma::BasePlayerComponent *pl) {
		auto &fc = methodNameOrFunction;
		auto *l = fc.interpreter();
		auto c = Lua::CallFunction(l,[&fc,&packet,&pl](lua_State *l) mutable -> Lua::StatusCode {
			fc.push(l);
			if(pl != nullptr)
				pl->PushLuaObject(l);
			Lua::Push<NetPacket>(l,packet.get());
			return Lua::StatusCode::Ok;
		},0);
	});
	Lua::Push<CallbackHandle>(l,hCb);
	return hCb;
}
CallbackHandle BaseLuaBaseEntityComponent::BindEvent(lua_State *l,pragma::ComponentId eventId,luabind::object methodNameOrFunction)
{
	methodNameOrFunction.push(l);
	util::ScopeGuard sg([l]() {
		Lua::Pop(l,1);
	});

	auto argTarget = -1;
	if(Lua::IsString(l,argTarget))
	{
		std::string methodName = Lua::CheckString(l,argTarget);
		auto hCb = BindEvent(eventId,[this,methodName](std::reference_wrapper<pragma::ComponentEvent> ev) -> ::util::EventReply {
			// CallLuaMember does not work with a variable number of arguments,
			// so we have to call the method manually.
			// See also BaseLuaBaseEntityComponent::HandleEvent
			auto o = GetLuaObject();
			auto r = o[methodName];
			if(r && luabind::type(r) == LUA_TFUNCTION)
			{
				auto *l = o.interpreter();
				auto numRet = ev.get().GetReturnCount();
				auto c = Lua::CallFunction(l,[&ev,&o,&r,&methodName](lua_State *l) -> Lua::StatusCode {
					r.push(l);
					o.push(l);
					ev.get().PushArguments(l);
					return Lua::StatusCode::Ok;
				},1 +numRet);
				if(c == Lua::StatusCode::Ok)
				{
					auto r = ::util::EventReply::Unhandled;
					auto idxFirst = -static_cast<int32_t>(numRet +1);
					if(Lua::IsSet(l,idxFirst) && Lua::IsNumber(l,idxFirst))
						r = static_cast<::util::EventReply>(Lua::CheckInt(l,idxFirst));
					Lua::RemoveValue(l,idxFirst); /* pop handled value */

					ev.get().HandleReturnValues(l);
					Lua::Pop(l,numRet);
					return r;
				}
			}
			return ::util::EventReply::Unhandled;
		});
		Lua::Push<CallbackHandle>(l,hCb);
		return hCb;
	}
	Lua::CheckFunction(l,argTarget);
	auto hCb = BindEvent(eventId,[methodNameOrFunction](std::reference_wrapper<pragma::ComponentEvent> ev) -> ::util::EventReply {
		auto &fc = methodNameOrFunction;
		auto *l = fc.interpreter();

		auto numRet = ev.get().GetReturnCount();
		auto c = Lua::CallFunction(l,[&fc,&ev](lua_State *l) -> Lua::StatusCode {
			fc.push(l);
			ev.get().PushArguments(l);
			return Lua::StatusCode::Ok;
		},1 +numRet);
		if(c == Lua::StatusCode::Ok)
		{
			auto r = ::util::EventReply::Unhandled;
			if(Lua::IsSet(l,-1))
				r = static_cast<::util::EventReply>(Lua::CheckInt(l,-1));
			Lua::Pop(l,1); /* pop handled value */

			ev.get().HandleReturnValues(l);
			Lua::Pop(l,numRet);
			return r;
		}
		return ::util::EventReply::Unhandled;
	});
	Lua::Push<CallbackHandle>(l,hCb);
	return hCb;
}

/////////////////

LuaBaseEntityComponentWrapper::LuaBaseEntityComponentWrapper(const util::WeakHandle<pragma::BaseEntityComponent> &hComponent)
	: BaseLuaBaseEntityHandle(hComponent)
{}
LuaBaseEntityComponentWrapper::LuaBaseEntityComponentWrapper()
	: LuaBaseEntityComponentWrapper(util::WeakHandle<pragma::BaseEntityComponent>{})
{}

