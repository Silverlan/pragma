/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/animation_driver_component.hpp"
#include "pragma/entities/components/animated_2_component.hpp"
#include "pragma/entities/entity_iterator.hpp"
#include "pragma/lua/lua_call.hpp"
#include <sharedutils/util_hash.hpp>
#include <udm.hpp>

using namespace pragma;
#pragma optimize("",off)
static EntityUuidComponentMemberRef get_member_ref(util::Uuid entUuid,util::Path var)
{
	auto componentName = var.GetFront();
	var.PopFront();
	auto &memberName = var.GetString();
	return EntityUuidComponentMemberRef{entUuid,componentName,memberName};
}
AnimationDriverVariable::AnimationDriverVariable(util::Uuid entUuid,const util::Path &var)
	: memberRef{get_member_ref(entUuid,var)}
{}

////////////

const ComponentMemberInfo *pragma::AnimationDriver::GetMemberInfo(const BaseEntityComponent &component) const
{
	return memberReference.GetMemberInfo(component);
}
const ComponentMemberInfo *pragma::AnimationDriver::GetMemberInfo(const BaseEntity &ent) const
{
	auto hComponent = ent.FindComponent(componentId);
	if(hComponent.expired())
		return nullptr;
	return GetMemberInfo(*hComponent);
}

////////////

static inline pragma::AnimationDriverComponent::AnimationDriverHash get_animation_driver_hash(ComponentId componentId,ComponentMemberIndex memberIdx)
{
	return util::hash_combine<uint64_t>(util::hash_combine<uint64_t>(0,componentId),memberIdx);
}
void pragma::AnimationDriverComponent::RegisterEvents(pragma::EntityComponentManager &componentManager)
{

}

pragma::AnimationDriverComponent::AnimationDriverComponent(BaseEntity &ent)
	: BaseEntityComponent{ent}
{}

void pragma::AnimationDriverComponent::Initialize()
{
	BaseEntityComponent::Initialize();
}
void pragma::AnimationDriverComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	//if(m_cbOnAnimationsUpdated.IsValid())
	//	m_cbOnAnimationsUpdated.Remove();
}
void AnimationDriverComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	/*if(typeid(component) == typeid(Animated2Component))
	{
		if(m_cbOnAnimationsUpdated.IsValid())
			m_cbOnAnimationsUpdated.Remove();
		m_cbOnAnimationsUpdated = component.AddEventCallback(Animated2Component::EVENT_ON_ANIMATIONS_UPDATED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
			ApplyDrivers();
			return util::EventReply::Unhandled;
		});
	}*/
}
void AnimationDriverComponent::OnEntityComponentRemoved(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentRemoved(component);
	/*if(typeid(component) == typeid(Animated2Component))
	{
		if(m_cbOnAnimationsUpdated.IsValid())
			m_cbOnAnimationsUpdated.Remove();
	}*/
}
void pragma::AnimationDriverComponent::ClearDrivers()
{
	m_drivers.clear();
}
void pragma::AnimationDriverComponent::RemoveDriver(ComponentId componentId,ComponentMemberIndex memberIdx)
{
	auto it = m_drivers.find(get_animation_driver_hash(componentId,memberIdx));
	if(it != m_drivers.end())
		m_drivers.erase(it);
}
void pragma::AnimationDriverComponent::RemoveDriver(ComponentId componentId,const std::string &memberName)
{
	auto memberIdx = FindComponentMember(componentId,memberName);
	if(!memberIdx.has_value())
		return;
	RemoveDriver(componentId,*memberIdx);
}
void pragma::AnimationDriverComponent::RemoveDrivers(ComponentId componentId)
{
	for(auto it=m_drivers.begin();it!=m_drivers.end();)
	{
		auto &driver = it->second;
		if(driver.componentId == componentId)
			it = m_drivers.erase(it);
		else
			++it;
	}
}
std::optional<ComponentMemberIndex> pragma::AnimationDriverComponent::FindComponentMember(ComponentId componentId,const std::string &memberName)
{
	auto *info = GetEntity().GetComponentManager()->GetComponentInfo(componentId);
	if(info)
	{
		auto memberIdx = info->FindMember(memberName);
		if(memberIdx.has_value())
			return memberIdx;
	}
	auto hComponent = GetEntity().FindComponent(componentId);
	if(hComponent.expired())
		return {};
	return hComponent->GetMemberIndex(memberName);
}
bool pragma::AnimationDriverComponent::AddDriver(ComponentId componentId,const std::string &memberName,const std::string &expression,AnimationDriverVariableList &&vars)
{
	auto memberIdx = FindComponentMember(componentId,memberName);
	if(!memberIdx.has_value())
		return false;
	AddDriver(componentId,*memberIdx,expression,std::move(vars));
	return true;
}
void pragma::AnimationDriverComponent::AddDriver(ComponentId componentId,ComponentMemberIndex memberIdx,const std::string &expression,AnimationDriverVariableList &&vars)
{
	auto hComponent = GetEntity().FindComponent(componentId);
	if(hComponent.expired())
		return;
	auto *memberInfo = hComponent->GetMemberInfo(memberIdx);
	if(!memberInfo)
		return;
	RemoveDriver(componentId,memberIdx);

	AnimationDriver driver {};
	driver.expression = expression;
	driver.variables = std::move(vars);
	driver.componentId = componentId;
	driver.memberReference = ComponentMemberReference{memberInfo->GetName()};
	
	driver.dataValue.type = memberInfo->type;
	udm::visit_ng(memberInfo->type,[&driver](auto tag) {
		using T = decltype(tag)::type;
		driver.dataValue.data = {
			new T{},
			[](void *value) {delete static_cast<T*>(value);}
		};
	});

	std::string argList = "value";
	for(auto &pair : driver.variables)
		argList += ',' +pair.first;

	auto *l = GetLuaState();
	std::string luaStr = "return function(" +argList +") return " +expression +" end";
	auto r = Lua::RunString(l,luaStr,1,"internal"); /* 1 */
	if(r == Lua::StatusCode::Ok)
	{
		luabind::object oFunc {luabind::from_stack(l,-1)};
		driver.luaExpression = oFunc;
		Lua::Pop(l,1); /* 0 */
	}
	else
		Lua::HandleLuaError(l);

	m_drivers[get_animation_driver_hash(componentId,memberIdx)] = std::move(driver);
}
bool pragma::AnimationDriverComponent::HasDriver(ComponentId componentId,ComponentMemberIndex memberIdx) const
{
	return FindDriver(componentId,memberIdx) != nullptr;
}
pragma::AnimationDriver *pragma::AnimationDriverComponent::FindDriver(ComponentId componentId,ComponentMemberIndex memberIdx)
{
	auto it = m_drivers.find(get_animation_driver_hash(componentId,memberIdx));
	return (it != m_drivers.end()) ? &it->second : nullptr;
}
void pragma::AnimationDriverComponent::ApplyDrivers()
{
	auto *l = GetLuaState();
	auto &game = *GetEntity().GetNetworkState()->GetGameState();
	for(auto &pair : m_drivers)
	{
		auto &driver = pair.second;
		if(driver.luaExpression)
		{
			auto hComponent = GetEntity().FindComponent(driver.componentId);
			if(hComponent.expired())
				continue;
			auto &component = *hComponent;
			auto *member = driver.GetMemberInfo(component);
			if(!member)
				continue;
			// At this point driver.dataValue should contain the interpolated value, which was assigned by the
			// animated2 component. We just need to run our expression and then apply the value.
			luabind::object arg;
			udm::visit_ng(driver.dataValue.type,[l,&arg,&driver](auto tag) {
				using T = decltype(tag)::type;
				if constexpr(udm::is_numeric_type(udm::type_to_enum<T>()))
					arg = luabind::object{l,*static_cast<T*>(driver.dataValue.data.get())};
				else
					arg = luabind::object{l,static_cast<T*>(driver.dataValue.data.get())};
			});
			driver.luaExpression.push(l);
			arg.push(l);
			uint32_t numPushed = 2;
			auto argsValid = true;
			for(auto &pair : driver.variables)
			{
				auto &var = pair.second;
				auto *memInfo = var.memberRef.GetMemberInfo(game);
				if(memInfo == nullptr)
				{
					argsValid = false;
					break;
				}
				auto *c = var.memberRef.GetComponent(game);
				auto o = udm::visit_ng(memInfo->type,[memInfo,c,l](auto tag) {
					using T = decltype(tag)::type;
					T value;
					memInfo->getterFunction(*memInfo,*c,&value);
					return luabind::object{l,value};
				});
				o.push(l);
				++numPushed;
			}
			if(!argsValid)
			{
				// Can't execute the driver if one or more of the arguments
				// couldn't be determined
				Lua::Pop(l,numPushed);
				continue;
			}
			auto c = Lua::ProtectedCall(l,numPushed -1,1);
			if(c != Lua::StatusCode::Ok)
				Lua::HandleLuaError(l,c);
			else
			{
				luabind::object result {luabind::from_stack(l,-1)};
				Lua::Pop(l,1);

				if(result)
				{
					udm::visit_ng(driver.dataValue.type,[l,&driver,&result,&member,&component](auto tag) {
						using T = decltype(tag)::type;
						try
						{
							auto ret = luabind::object_cast<T>(result);
							member->setterFunction(*member,component,&ret);
						}
						catch(const luabind::cast_failed &e)
						{
							Con::cwar<<"WARNING: Driver expression '"<<driver.expression<<"' return value is incompatible with expected type '"<<magic_enum::enum_name(driver.dataValue.type)<<"'!"<<Con::endl;
						}
					});
				}
			}
		}
	}
}

void pragma::AnimationDriverComponent::InitializeLuaObject(lua_State *l) {pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l);}
void pragma::AnimationDriverComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);
}

void pragma::AnimationDriverComponent::Load(udm::LinkedPropertyWrapperArg udm,uint32_t version)
{
	BaseEntityComponent::Load(udm,version);

}
#pragma optimize("",on)
