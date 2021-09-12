/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/animation_driver_component.hpp"
#include "pragma/entities/components/panima_component.hpp"
#include "pragma/entities/entity_iterator.hpp"
#include <sharedutils/util_hash.hpp>
#include <udm.hpp>

using namespace pragma;

static inline pragma::AnimationDriverComponent::ValueDriverHash get_value_driver_hash(ComponentId componentId,ComponentMemberIndex memberIdx)
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
	/*if(typeid(component) == typeid(PanimaComponent))
	{
		if(m_cbOnAnimationsUpdated.IsValid())
			m_cbOnAnimationsUpdated.Remove();
		m_cbOnAnimationsUpdated = component.AddEventCallback(PanimaComponent::EVENT_ON_ANIMATIONS_UPDATED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
			ApplyDrivers();
			return util::EventReply::Unhandled;
		});
	}*/
}
void AnimationDriverComponent::OnEntityComponentRemoved(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentRemoved(component);
	/*if(typeid(component) == typeid(PanimaComponent))
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
	auto it = m_drivers.find(get_value_driver_hash(componentId,memberIdx));
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
		if(driver.GetComponentId() == componentId)
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
bool pragma::AnimationDriverComponent::AddDriver(ComponentId componentId,const std::string &memberName,ValueDriverDescriptor descriptor)
{
	auto memberIdx = FindComponentMember(componentId,memberName);
	if(!memberIdx.has_value())
		return false;
	AddDriver(componentId,*memberIdx,std::move(descriptor));
	return true;
}
void pragma::AnimationDriverComponent::AddDriver(ComponentId componentId,ComponentMemberIndex memberIdx,ValueDriverDescriptor descriptor)
{
	auto hComponent = GetEntity().FindComponent(componentId);
	if(hComponent.expired())
		return;
	auto *memberInfo = hComponent->GetMemberInfo(memberIdx);
	if(!memberInfo)
		return;
	auto ref = ComponentMemberReference::Create(*hComponent,memberIdx);
	if(!ref.has_value())
		return;
	RemoveDriver(componentId,memberIdx);
	m_drivers[get_value_driver_hash(componentId,memberIdx)] = ValueDriver{componentId,std::move(*ref),std::move(descriptor)};
}
bool pragma::AnimationDriverComponent::HasDriver(ComponentId componentId,ComponentMemberIndex memberIdx) const
{
	return FindDriver(componentId,memberIdx) != nullptr;
}
pragma::ValueDriver *pragma::AnimationDriverComponent::FindDriver(ComponentId componentId,ComponentMemberIndex memberIdx)
{
	auto it = m_drivers.find(get_value_driver_hash(componentId,memberIdx));
	return (it != m_drivers.end()) ? &it->second : nullptr;
}
void pragma::AnimationDriverComponent::ApplyDrivers()
{
	auto *l = GetLuaState();
	auto &ent = GetEntity();
	auto &game = *ent.GetNetworkState()->GetGameState();
	for(auto &pair : m_drivers)
		pair.second.Apply(ent);
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
