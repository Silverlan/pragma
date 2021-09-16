/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/sh_lua_component.hpp"
#include "pragma/lua/sh_lua_component_wrapper.hpp"
#include "pragma/lua/lua_call.hpp"
#include "pragma/lua/classes/lproperty.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include "pragma/entities/entity_iterator.hpp"
#include "pragma/entities/components/base_io_component.hpp"
#include "pragma/entities/components/base_player_component.hpp"
#include "pragma/entities/components/panima_component.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/lua/base_lua_handle_method.hpp"
#include "pragma/lua/sh_lua_component_t.hpp"
#include "pragma/lua/converters/game_type_converters_t.hpp"
#include "pragma/lua/lua_util_component.hpp"
#include "pragma/lua/types/udm.hpp"
#include <sharedutils/scope_guard.h>
#include <sharedutils/datastream.h>
#include <sharedutils/netpacket.hpp>
#include <udm.hpp>

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
static ClassMembers *get_class_members(const luabind::object &oClass)
{
	auto &members = get_class_member_list(oClass.interpreter());
	auto it = std::find_if(members.begin(),members.end(),[&oClass](const ClassMembers &classMembers) {
		return oClass == classMembers.classObject;
	});
	if(it == members.end())
		return nullptr;
	return &*it;
}
static uint32_t get_class_member_index(const luabind::object &oClass)
{
	auto &members = get_class_member_list(oClass.interpreter());
	auto it = std::find_if(members.begin(),members.end(),[&oClass](const ClassMembers &classMembers) {
		return oClass == classMembers.classObject;
	});
	if(it == members.end())
		return std::numeric_limits<uint32_t>::max();
	return it -members.begin();
}
static std::optional<BaseLuaBaseEntityComponent::MemberIndex> find_member_info_index(const luabind::object &oClass,const std::string &memberName)
{
	auto *members = get_class_members(oClass);
	if(!members)
		return {};
	auto itMember = std::find_if(members->memberDeclarations.begin(),members->memberDeclarations.end(),[&memberName](const BaseLuaBaseEntityComponent::MemberInfo &memberInfo) {
		return ustring::compare(memberName,memberInfo.name,false);
	});
	if(itMember == members->memberDeclarations.end())
		return {};
	return itMember -members->memberDeclarations.begin();
}
BaseLuaBaseEntityComponent::MemberIndex BaseLuaBaseEntityComponent::RegisterMember(
	const luabind::object &oClass,const std::string &memberName,udm::Type memberType,const std::any &initialValue,MemberFlags memberFlags,const Lua::map<std::string,void> &attributes
)
{
	if(memberName.empty())
		return INVALID_MEMBER;
	auto *l = oClass.interpreter();
	/*if((memberFlags &MemberFlags::StoreBit) != MemberFlags::None)
	{
		std::string err = "If store flag is set, version number mustn't be 0! Please check 'lua_help ents.BaseEntityComponent.RegisterMember' for more information!";
		lua_pushstring(l,err.c_str());
		lua_error(l);
		return INVALID_MEMBER;
	}*/
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
	if(itMember == it->memberDeclarations.end())
	{
		auto tmpMemberName = memberName;
		std::optional<ComponentMemberInfo> componentMemberInfo {};
		// TODO: Add support for properties
		auto isAnimatableType = false;
		if(pragma::is_animatable_type(memberType))
		{
			auto vs = [&tmpMemberName,&memberName,memberType](auto tag) -> pragma::ComponentMemberInfo {
				using T = decltype(tag)::type;
				if constexpr(pragma::is_animatable_type_v<T>)
				{
					return create_component_member_info<
						BaseLuaBaseEntityComponent,T,
						[](const ComponentMemberInfo &memberInfo,BaseLuaBaseEntityComponent &component,const T &value) {
							component.GetLuaObject()[memberInfo.GetName()] = value;
						},[](const ComponentMemberInfo &memberInfo,BaseLuaBaseEntityComponent &component,T &value) {
							auto *v = luabind::object_cast_nothrow<T*>(component.GetLuaObject()[memberInfo.GetName()],static_cast<T*>(nullptr));
							if(!v)
								value = {};
							else
								value = *v;
						}
					>(std::move(tmpMemberName));
				}
				else
				{
					// Unreachable
					throw std::runtime_error{"Member " +memberName +" of type " +std::string{magic_enum::enum_name(memberType)} +" cannot be animated!"};
					return pragma::ComponentMemberInfo::CreateDummy();
				}
			};
			if(udm::is_ng_type(memberType))
			{
				componentMemberInfo = std::move(udm::visit_ng<false>(memberType,vs));
				for(luabind::iterator it{attributes},end;it!=end;++it)
				{
					auto key = luabind::object_cast<std::string>(it.key());
					luabind::object val {*it};
					using namespace ustring::string_switch;
					switch(hash(key))
					{
					case "min"_:
						componentMemberInfo->SetMin(luabind::object_cast<float>(val));
						break;
					case "max"_:
						componentMemberInfo->SetMax(luabind::object_cast<float>(val));
						break;
					case "stepSize"_:
						componentMemberInfo->SetStepSize(luabind::object_cast<float>(val));
						break;
					}
				}
				isAnimatableType = true;
			}
		}
		it->memberDeclarations.push_back({memberName,memberType,initialValue,memberFlags,std::move(componentMemberInfo)});
		itMember = it->memberDeclarations.end() -1;
		if(isAnimatableType)
		{
			auto &initialValue = itMember->initialValue;
			udm::visit_ng(memberType,[&componentMemberInfo,&initialValue](auto tag) {
				using T = decltype(tag)::type;
				componentMemberInfo->SetDefault<T>(std::any_cast<T>(initialValue));
			});
		}
	}
	auto idx = itMember -it->memberDeclarations.begin();

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
			setter += " self:OnMemberValueChanged(" +std::to_string(idx) +")";
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
	return idx;
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
void BaseLuaBaseEntityComponent::SetupLua(const luabind::object &o) {SetLuaObject(o);}

////////////////

BaseLuaBaseEntityComponent::BaseLuaBaseEntityComponent(BaseEntity &ent)
	: pragma::BaseEntityComponent(ent)
{}

void BaseLuaBaseEntityComponent::InitializeLuaObject(lua_State *l) {}

luabind::object *BaseLuaBaseEntityComponent::GetClassObject()
{
	auto &game = *GetEntity().GetNetworkState()->GetGameState();
	auto &componentManager = game.GetEntityComponentManager();
	auto *pInfo = componentManager.GetComponentInfo(GetComponentId());
	if(!pInfo)
		return nullptr;
	auto &luaEntityManager = game.GetLuaEntityManager();
	return luaEntityManager.GetComponentClassObject(pInfo->name);
}

void BaseLuaBaseEntityComponent::Initialize()
{
	// The underlying handle for the lua object has not yet been assigned to our shared ptr, so we have to do it now.
	// This has to be done before any Lua member-functions are called, but can't be done inside the constructor, because
	// at that point the object hasn't been assigned to a shared ptr yet!
	//auto *pHandleWrapper = luabind::object_cast_nothrow<BaseLuaBaseEntityComponentHandleWrapper*>(GetLuaObject(),static_cast<BaseLuaBaseEntityComponentHandleWrapper*>(nullptr));
	//if(pHandleWrapper != nullptr)
	//	pHandleWrapper->handle = util::WeakHandle<pragma::BaseEntityComponent>(shared_from_this());
	
	auto &game = *GetEntity().GetNetworkState()->GetGameState();
	auto &componentManager = game.GetEntityComponentManager();
	if((componentManager.GetComponentInfo(GetComponentId())->flags &pragma::ComponentFlags::Networked) != pragma::ComponentFlags::None)
		SetNetworked(true);

	auto *l = GetLuaState();
	InitializeLuaObject(l);

	auto *o = GetClassObject();
	if(o != nullptr)
	{
		m_classMemberIndex = get_class_member_index(*o);
		auto *p = get_class_member_declarations(*o);
		if(p != nullptr)
			InitializeMembers(p->memberDeclarations);
	}
	if(m_networkedMemberInfo != nullptr)
		m_networkedMemberInfo->netEvSetMember = SetupNetEvent("set_member_value");

	CallLuaMethod("Initialize");

	PushLuaObject(); /* 1 */
	auto t = Lua::GetStackTop(l);
	Lua::PushString(l,"Version"); /* 2 */
	Lua::GetTableValue(l,t); /* 2 */
	if(Lua::IsSet(l,-1))
		m_version = Lua::CheckInt(l,-1);
	Lua::Pop(l,2); /* 0 */

	auto &ent = GetEntity();
	CallLuaMethod("OnAttachedToEntity");
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
			Lua::PushNewAnyProperty(l,detail::udm_type_to_util_type(member.type),member.initialValue); /* 3 */
		else
			Lua::PushAny(l,detail::udm_type_to_util_type(member.type),member.initialValue); /* 3 */
		if(Lua::IsNil(l,-1))
			Con::cwar<<"WARNING: Invalid member type "<<magic_enum::enum_name(member.type)<<" for member '"<<member.name<<"' of entity component '"<<GetEntity().GetNetworkState()->GetGameState()->GetEntityComponentManager().GetComponentInfo(GetComponentId())->name<<"'! Ignoring..."<<Con::endl;
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
			SetMemberValue(member,string_to_any(*GetEntity().GetNetworkState()->GetGameState(),kvData.value,detail::udm_type_to_util_type(member.type)));
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
			SetMemberValue(member,string_to_any(*GetEntity().GetNetworkState()->GetGameState(),ioData.data,detail::udm_type_to_util_type(member.type)));
			return util::EventReply::Handled;
		});
	}
}

void BaseLuaBaseEntityComponent::OnTick(double dt)
{
	CallLuaMethod<void,double>("OnTick",dt);
}

void BaseLuaBaseEntityComponent::InitializeMember(const MemberInfo &memberInfo) {}

std::optional<ComponentMemberIndex> BaseLuaBaseEntityComponent::DoGetMemberIndex(const std::string &name) const
{
	auto *o = GetClassObject();
	if(!o)
		return {};
	return find_member_info_index(*o,name);
}
const ComponentMemberInfo *BaseLuaBaseEntityComponent::GetMemberInfo(ComponentMemberIndex idx) const
{
	auto &classMembers = get_class_member_list(GetLuaObject().interpreter());
	if(m_classMemberIndex >= classMembers.size())
		return nullptr;
	auto &members = classMembers[m_classMemberIndex];
	if(idx >= members.memberDeclarations.size())
		return nullptr;
	auto &memberInfo = members.memberDeclarations[idx];
	return memberInfo.componentMemberInfo.has_value() ? &*memberInfo.componentMemberInfo : nullptr;
}

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
	CallLuaMethod("OnAttachedToEntity");
}
void BaseLuaBaseEntityComponent::OnDetached(BaseEntity &ent)
{
	pragma::BaseEntityComponent::OnDetached(ent);
	CallLuaMethod("OnDetachedToEntity");
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
		auto value = Lua::GetAnyValue(l,detail::udm_type_to_util_type(memberInfo.type),-1);
		Lua::Pop(l,2); /* 0 */
		return value;
	}
	auto value = Lua::GetAnyPropertyValue(l,-1,detail::udm_type_to_util_type(memberInfo.type));
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
		Lua::SetAnyPropertyValue(l,-1,detail::udm_type_to_util_type(memberInfo.type),value); /* 2 */
		Lua::Pop(l,1); /* 1 */
	}
	else
	{
		Lua::PushAny(l,detail::udm_type_to_util_type(memberInfo.type),value); /* 3 */
		Lua::SetTableValue(l,t); /* 1 */
	}
	Lua::Pop(l,1); /* 0 */
}

static void write_value(udm::LinkedPropertyWrapperArg udm,const std::any &value,util::VarType type)
{
	switch(type)
	{
	case util::VarType::Bool:
		udm = std::any_cast<udm::Boolean>(value);
		break;
	case util::VarType::Double:
		udm = std::any_cast<udm::Double>(value);
		break;
	case util::VarType::Float:
		udm = std::any_cast<udm::Float>(value);
		break;
	case util::VarType::Int8:
		udm = std::any_cast<udm::Int8>(value);
		break;
	case util::VarType::Int16:
		udm = std::any_cast<udm::Int16>(value);
		break;
	case util::VarType::Int32:
		udm = std::any_cast<udm::Int32>(value);
		break;
	case util::VarType::Int64:
		udm = std::any_cast<udm::Int64>(value);
		break;
	case util::VarType::LongDouble:
		udm = static_cast<udm::Double>(std::any_cast<long double>(value));
		break;
	case util::VarType::String:
		udm = std::any_cast<udm::String>(value);
		break;
	case util::VarType::UInt8:
		udm = std::any_cast<udm::UInt8>(value);
		break;
	case util::VarType::UInt16:
		udm = std::any_cast<udm::UInt16>(value);
		break;
	case util::VarType::UInt32:
		udm = std::any_cast<udm::UInt32>(value);
		break;
	case util::VarType::UInt64:
		udm = std::any_cast<udm::UInt64>(value);
		break;
	case util::VarType::EulerAngles:
		udm = std::any_cast<udm::EulerAngles>(value);
		break;
	case util::VarType::Color:
	{
		auto color = std::any_cast<Color>(value);
		udm = Vector4{color.r,color.g,color.b,color.a};
		break;
	}
	case util::VarType::Vector:
		udm = std::any_cast<udm::Vector3>(value);
		break;
	case util::VarType::Vector2:
		udm = std::any_cast<udm::Vector2>(value);
		break;
	case util::VarType::Vector4:
		udm = std::any_cast<udm::Vector4>(value);
		break;
	case util::VarType::Entity:
	{
		auto hEnt = std::any_cast<EntityHandle>(value);
		if(hEnt.valid())
			udm = util::uuid_to_string(hEnt->GetUuid());
		//else
		//	udm = udm::Nil{};
		break;
	}
	case util::VarType::Quaternion:
		udm = std::any_cast<udm::Quaternion>(value);
		break;
	}
}

static void read_value(Game &game,udm::LinkedPropertyWrapperArg udm,std::any &value,util::VarType type)
{
	switch(type)
	{
	case util::VarType::Bool:
	{
		auto val = udm.ToValue<udm::Boolean>();
		if(val.has_value())
			value = *val;
		break;
	}
	case util::VarType::Double:
	{
		auto val = udm.ToValue<udm::Double>();
		if(val.has_value())
			value = *val;
		break;
	}
	case util::VarType::Float:
	{
		auto val = udm.ToValue<udm::Float>();
		if(val.has_value())
			value = *val;
		break;
	}
	case util::VarType::Int8:
	{
		auto val = udm.ToValue<udm::Int8>();
		if(val.has_value())
			value = *val;
		break;
	}
	case util::VarType::Int16:
	{
		auto val = udm.ToValue<udm::Int16>();
		if(val.has_value())
			value = *val;
		break;
	}
	case util::VarType::Int32:
	{
		auto val = udm.ToValue<udm::Int32>();
		if(val.has_value())
			value = *val;
		break;
	}
	case util::VarType::Int64:
	{
		auto val = udm.ToValue<udm::Int64>();
		if(val.has_value())
			value = *val;
		break;
	}
	case util::VarType::LongDouble:
	{
		auto val = udm.ToValue<udm::Double>();
		if(val.has_value())
			value = *val;
		break;
	}
	case util::VarType::String:
	{
		auto val = udm.ToValue<udm::String>();
		if(val.has_value())
			value = *val;
		break;
	}
	case util::VarType::UInt8:
	{
		auto val = udm.ToValue<udm::UInt8>();
		if(val.has_value())
			value = *val;
		break;
	}
	case util::VarType::UInt16:
	{
		auto val = udm.ToValue<udm::UInt16>();
		if(val.has_value())
			value = *val;
		break;
	}
	case util::VarType::UInt32:
	{
		auto val = udm.ToValue<udm::UInt32>();
		if(val.has_value())
			value = *val;
		break;
	}
	case util::VarType::UInt64:
	{
		auto val = udm.ToValue<udm::UInt64>();
		if(val.has_value())
			value = *val;
		break;
	}
	case util::VarType::EulerAngles:
	{
		auto val = udm.ToValue<udm::EulerAngles>();
		if(val.has_value())
			value = *val;
		break;
	}
	case util::VarType::Color:
	{
		auto val = udm.ToValue<udm::Vector4>();
		if(val.has_value())
			value = Color{static_cast<int16_t>(val->x),static_cast<int16_t>(val->y),static_cast<int16_t>(val->z),static_cast<int16_t>(val->w)};
		break;
	}
	case util::VarType::Vector:
	{
		auto val = udm.ToValue<udm::Vector3>();
		if(val.has_value())
			value = *val;
		break;
	}
	case util::VarType::Vector2:
	{
		auto val = udm.ToValue<udm::Vector2>();
		if(val.has_value())
			value = *val;
		break;
	}
	case util::VarType::Vector4:
	{
		auto val = udm.ToValue<udm::Vector4>();
		if(val.has_value())
			value = *val;
		break;
	}
	case util::VarType::Entity:
	{
		auto val = udm.ToValue<udm::String>();
		if(val.has_value() && util::is_uuid(*val))
		{
			EntityIterator entIt {game,EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending};
			entIt.AttachFilter<EntityIteratorFilterUuid>(util::uuid_string_to_bytes(*val));
			auto it = entIt.begin();
			auto *ent = (it != entIt.end()) ? *it : nullptr;
			value = ent ? ent->GetHandle() : EntityHandle{};
		}
		break;
	}
	case util::VarType::Quaternion:
	{
		auto val = udm.ToValue<udm::Quaternion>();
		if(val.has_value())
			value = *val;
		break;
	}
	}
}

void BaseLuaBaseEntityComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	// CallLuaMember<void,luabind::object>("OnEntityComponentAdded",component.GetLuaObject()); // Unused: Already covered by EVENT_ON_COMPONENT_ADDED
}
void BaseLuaBaseEntityComponent::OnEntityComponentRemoved(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentRemoved(component);
	// CallLuaMember<void,luabind::object>("OnEntityComponentRemoved",component.GetLuaObject()); // Unused: Already covered by EVENT_ON_COMPONENT_REMOVED
}

const std::vector<BaseLuaBaseEntityComponent::MemberInfo> &BaseLuaBaseEntityComponent::GetMembers() const {return m_members;}
void BaseLuaBaseEntityComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	pragma::BaseEntityComponent::Save(udm);
	for(auto &member : m_members)
	{
		if((member.flags &MemberFlags::StoreBit) == MemberFlags::None)
			continue;
		auto value = GetMemberValue(member);
		write_value(udm["members." +member.name],value,detail::udm_type_to_util_type(member.type));
	}
	CallLuaMethod<void,udm::LinkedPropertyWrapper>("Save",udm);
}
void BaseLuaBaseEntityComponent::Load(udm::LinkedPropertyWrapperArg udm,uint32_t version)
{
	pragma::BaseEntityComponent::Load(udm,version);
	auto &game = *GetEntity().GetNetworkState()->GetGameState();
	for(auto &member : m_members)
	{
		if((member.flags &MemberFlags::StoreBit) == MemberFlags::None)
			continue;
		std::any value;
		auto udmMember = udm["members." +member.name];
		if(udmMember)
		{
			read_value(game,udmMember,value,detail::udm_type_to_util_type(member.type));
			SetMemberValue(member,value);
		}
	}
	CallLuaMethod<void,udm::LinkedPropertyWrapper,uint32_t>("Load",udm,version);
}
uint32_t BaseLuaBaseEntityComponent::GetVersion() const {return m_version;}

void BaseLuaBaseEntityComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	CallLuaMethod("OnEntitySpawn");
}

void BaseLuaBaseEntityComponent::OnRemove()
{
	pragma::BaseEntityComponent::OnRemove();
	CallLuaMethod("OnRemove");
}

void BaseLuaBaseEntityComponent::SetNetworked(bool b) {m_bShouldTransmitNetData = b;}
bool BaseLuaBaseEntityComponent::IsNetworked() const {return m_bShouldTransmitNetData;}
void BaseLuaBaseEntityComponent::SetShouldTransmitSnapshotData(bool b) {m_bShouldTransmitSnapshotData = b;}
bool BaseLuaBaseEntityComponent::ShouldTransmitSnapshotData() const {return m_bShouldTransmitSnapshotData;}

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
			CallLuaMethod<void,luabind::object>(methodName,component.get().GetLuaObject());
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

void Lua::register_base_entity_component(luabind::module_ &modEnts)
{
	auto classDef = pragma::lua::create_entity_component_class<pragma::BaseLuaBaseEntityComponent,pragma::BaseEntityComponent>("BaseBaseEntityComponent");
	classDef.def("SetNetworked",&pragma::BaseLuaBaseEntityComponent::SetNetworked);
	classDef.def("IsNetworked",&pragma::BaseLuaBaseEntityComponent::IsNetworked);
	classDef.def("SetShouldTransmitSnapshotData",&pragma::BaseLuaBaseEntityComponent::SetShouldTransmitSnapshotData);
	classDef.def("ShouldTransmitSnapshotData",&pragma::BaseLuaBaseEntityComponent::ShouldTransmitSnapshotData);
	classDef.def("GetVersion",&pragma::BaseLuaBaseEntityComponent::GetVersion);
	classDef.def("RegisterNetEvent",&pragma::BaseLuaBaseEntityComponent::SetupNetEvent);
	classDef.def("FlagCallbackForRemoval",static_cast<void(*)(lua_State*,pragma::BaseLuaBaseEntityComponent&,CallbackHandle&,pragma::BaseEntityComponent::CallbackType,pragma::BaseLuaBaseEntityComponent&)>([](lua_State *l,pragma::BaseLuaBaseEntityComponent &hComponent,CallbackHandle &hCb,pragma::BaseEntityComponent::CallbackType callbackType,pragma::BaseLuaBaseEntityComponent &hComponentOther) {
		if(hCb.IsValid() == false)
			return;
		hComponent.FlagCallbackForRemoval(hCb,callbackType,&hComponentOther);
	}));
	classDef.def("FlagCallbackForRemoval",static_cast<void(*)(lua_State*,pragma::BaseLuaBaseEntityComponent&,CallbackHandle&,pragma::BaseEntityComponent::CallbackType)>([](lua_State *l,pragma::BaseLuaBaseEntityComponent &hComponent,CallbackHandle &hCb,pragma::BaseEntityComponent::CallbackType callbackType) {
		if(hCb.IsValid() == false)
			return;
		hComponent.FlagCallbackForRemoval(hCb,callbackType);
	}));
	classDef.def("BindComponentInitEvent",static_cast<CallbackHandle(*)(lua_State*,pragma::BaseLuaBaseEntityComponent&,uint32_t,luabind::object)>([](lua_State *l,pragma::BaseLuaBaseEntityComponent &hComponent,uint32_t componentId,luabind::object methodNameOrFunction) {
		return hComponent.BindInitComponentEvent(l,componentId,methodNameOrFunction);
	}));
	classDef.def("BindEvent",static_cast<CallbackHandle(*)(lua_State*,pragma::BaseLuaBaseEntityComponent&,uint32_t,luabind::object)>([](lua_State *l,pragma::BaseLuaBaseEntityComponent &hComponent,uint32_t eventId,luabind::object methodNameOrFunction) {
		return hComponent.BindEvent(l,eventId,methodNameOrFunction);
	}));
	classDef.def("GetEntityComponent",static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent>(*)(lua_State*,pragma::BaseLuaBaseEntityComponent&,uint32_t)>([](lua_State *l,pragma::BaseLuaBaseEntityComponent &hComponent,uint32_t componentId) -> pragma::ComponentHandle<pragma::BaseEntityComponent> {
		return hComponent.GetEntity().FindComponent(componentId);
	}));
	classDef.def("GetEntityComponent",static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent>(*)(lua_State*,pragma::BaseLuaBaseEntityComponent&,const std::string&)>([](lua_State *l,pragma::BaseLuaBaseEntityComponent &hComponent,const std::string &componentName) -> pragma::ComponentHandle<pragma::BaseEntityComponent> {
		return hComponent.GetEntity().FindComponent(componentName);
	}));
	classDef.def("AddEntityComponent",static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent>(*)(lua_State*,pragma::BaseLuaBaseEntityComponent&,uint32_t)>([](lua_State *l,pragma::BaseLuaBaseEntityComponent &hComponent,uint32_t componentId) -> pragma::ComponentHandle<pragma::BaseEntityComponent> {
		return hComponent.GetEntity().AddComponent(componentId);
	}));
	classDef.def("AddEntityComponent",static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent>(*)(lua_State*,pragma::BaseLuaBaseEntityComponent&,uint32_t,luabind::object)>([](lua_State *l,pragma::BaseLuaBaseEntityComponent &hComponent,uint32_t componentId,luabind::object methodNameOrFunction) -> pragma::ComponentHandle<pragma::BaseEntityComponent> {
		hComponent.BindInitComponentEvent(l,componentId,methodNameOrFunction);
		return hComponent.GetEntity().AddComponent(componentId);
	}));
	classDef.def("AddEntityComponent",static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent>(*)(lua_State*,pragma::BaseLuaBaseEntityComponent&,const std::string&)>([](lua_State *l,pragma::BaseLuaBaseEntityComponent &hComponent,const std::string &name) -> pragma::ComponentHandle<pragma::BaseEntityComponent> {
		return hComponent.GetEntity().AddComponent(name);
	}));
	classDef.def("AddEntityComponent",static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent>(*)(lua_State*,pragma::BaseLuaBaseEntityComponent&,const std::string&,luabind::object)>([](lua_State *l,pragma::BaseLuaBaseEntityComponent &hComponent,const std::string &name,luabind::object methodNameOrFunction) -> pragma::ComponentHandle<pragma::BaseEntityComponent> {
		auto hNewComponent = hComponent.GetEntity().AddComponent(name);
		if(hNewComponent.expired())
			return hNewComponent;
		hComponent.BindInitComponentEvent(l,hNewComponent->GetComponentId(),methodNameOrFunction);
		return hNewComponent;
	}));
	classDef.def("OnMemberValueChanged",&pragma::BaseLuaBaseEntityComponent::OnMemberValueChanged);
	classDef.scope[luabind::def("RegisterMember",+[](lua_State *l,const Lua::classObject &o,const std::string &memberName,udm::Type memberType,Lua::udm_type oDefault,const Lua::map<std::string,void> &attributes,pragma::BaseLuaBaseEntityComponent::MemberFlags memberFlags) {
		auto anyInitialValue = Lua::GetAnyValue(l,detail::udm_type_to_util_type(memberType),4);
		pragma::BaseLuaBaseEntityComponent::RegisterMember(o,memberName,memberType,anyInitialValue,memberFlags,attributes);
	})];
	classDef.scope[luabind::def("RegisterMember",+[](lua_State *l,const Lua::classObject &o,const std::string &memberName,udm::Type memberType,Lua::udm_type oDefault,const Lua::map<std::string,void> &attributes) {
		auto anyInitialValue = Lua::GetAnyValue(l,detail::udm_type_to_util_type(memberType),4);
		pragma::BaseLuaBaseEntityComponent::RegisterMember(o,memberName,memberType,anyInitialValue,pragma::BaseLuaBaseEntityComponent::MemberFlags::Default,attributes);
	})];
	classDef.scope[luabind::def("RegisterMember",+[](lua_State *l,const Lua::classObject &o,const std::string &memberName,udm::Type memberType,Lua::udm_type oDefault) {
		auto anyInitialValue = Lua::GetAnyValue(l,detail::udm_type_to_util_type(memberType),4);
		pragma::BaseLuaBaseEntityComponent::RegisterMember(o,memberName,memberType,anyInitialValue,pragma::BaseLuaBaseEntityComponent::MemberFlags::Default,luabind::newtable(l));
	})];

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

	classDef.def("Initialize",&pragma::BaseLuaBaseEntityComponent::Lua_Initialize,&pragma::BaseLuaBaseEntityComponent::default_Lua_Initialize);
	classDef.def("OnTick",&pragma::BaseLuaBaseEntityComponent::Lua_OnTick,&pragma::BaseLuaBaseEntityComponent::default_Lua_OnTick);
	classDef.def("OnRemove",&pragma::BaseLuaBaseEntityComponent::Lua_OnRemove,&pragma::BaseLuaBaseEntityComponent::default_Lua_OnRemove);
	classDef.def("OnEntitySpawn",&pragma::BaseLuaBaseEntityComponent::Lua_OnEntitySpawn,&pragma::BaseLuaBaseEntityComponent::default_Lua_OnEntitySpawn);
	classDef.def("OnAttachedToEntity",&pragma::BaseLuaBaseEntityComponent::Lua_OnAttachedToEntity,&pragma::BaseLuaBaseEntityComponent::default_Lua_OnAttachedToEntity);
	classDef.def("OnDetachedFromEntity",&pragma::BaseLuaBaseEntityComponent::Lua_OnDetachedToEntity,&pragma::BaseLuaBaseEntityComponent::default_Lua_OnDetachedToEntity);
	// classDef.def("OnEntityComponentAdded",&pragma::BaseLuaBaseEntityComponent::Lua_OnEntityComponentAdded,&pragma::BaseLuaBaseEntityComponent::default_Lua_OnEntityComponentAdded);
	// classDef.def("OnEntityComponentRemoved",&pragma::BaseLuaBaseEntityComponent::Lua_OnEntityComponentRemoved,&pragma::BaseLuaBaseEntityComponent::default_Lua_OnEntityComponentRemoved);
	// HandleEvent is variadic and can't be defined like this in luabind!
	//classDef.def("HandleEvent",&pragma::BaseLuaBaseEntityComponent::Lua_HandleEvent,&pragma::BaseLuaBaseEntityComponent::default_Lua_HandleEvent);
	classDef.def("Save",&pragma::BaseLuaBaseEntityComponent::Lua_Save,&pragma::BaseLuaBaseEntityComponent::default_Lua_Save);
	classDef.def("Load",&pragma::BaseLuaBaseEntityComponent::Lua_Load,&pragma::BaseLuaBaseEntityComponent::default_Lua_Load);
	modEnts[classDef];
}
