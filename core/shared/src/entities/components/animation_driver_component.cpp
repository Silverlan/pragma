/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/animation_driver_component.hpp"
#include "pragma/entities/components/animated_2_component.hpp"
#include <sharedutils/util_hash.hpp>
#include <udm.hpp>

using namespace pragma;
#pragma optimize("",off)
const ComponentMemberInfo *pragma::AnimationDriver::GetMemberInfo(const BaseEntity &ent) const
{
	auto hComponent = ent.FindComponent(componentId);
	if(hComponent.expired())
		return nullptr;
	return hComponent->GetMemberInfo(memberIndex);
}
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
void pragma::AnimationDriverComponent::AddDriver(ComponentId componentId,ComponentMemberIndex memberIdx,const std::string &expression,udm::PProperty constants)
{
	AnimationDriver driver {};
	driver.expression = expression;
	driver.constants = constants;
	driver.componentId = componentId;
	driver.memberIndex = memberIdx;
	
	auto *memberInfo = driver.GetMemberInfo(GetEntity());
	if(!memberInfo)
		return;
	driver.dataValue.type = memberInfo->type;
	udm::visit_ng(memberInfo->type,[&driver](auto tag) {
		using T = decltype(tag)::type;
		driver.dataValue.data = {
			new T{},
			[](void *value) {delete static_cast<T*>(value);}
		};
	});

	auto *l = GetLuaState();
	std::string luaStr = "return function(value) return " +expression +" end";
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
	for(auto &pair : m_drivers)
	{
		auto &driver = pair.second;
		if(driver.luaExpression)
		{
			auto hComponent = GetEntity().FindComponent(driver.componentId);
			if(hComponent.expired())
				continue;
			auto *member = hComponent->GetMemberInfo(driver.memberIndex);
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
			auto result = driver.luaExpression(arg);
			if(result)
			{
				udm::visit_ng(driver.dataValue.type,[l,&driver,&result,&member,&hComponent](auto tag) {
					using T = decltype(tag)::type;
					try
					{
						auto ret = luabind::object_cast<T>(result);
						member->setterFunction(*member,*hComponent,&ret);
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
