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
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/logging.hpp"
#include <sharedutils/util_hash.hpp>
#include <udm.hpp>

using namespace pragma;
void pragma::AnimationDriverComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) {}
static void set_parameters(const ComponentMemberInfo &memberInfo, AnimationDriverComponent &component, const pragma::ents::Element &value) { component.UpdateParameters(); }
static void get_parameters(const ComponentMemberInfo &memberInfo, AnimationDriverComponent &component, pragma::ents::Element &value) { value = component.GetParameters(); }
void AnimationDriverComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = AnimationDriverComponent;

	{
		using TDrivenObject = pragma::EntityUComponentMemberRef;
		auto memberInfo = create_component_member_info<T, TDrivenObject, static_cast<void (T::*)(const TDrivenObject &)>(&T::SetDrivenObject), static_cast<const TDrivenObject &(T::*)() const>(&T::GetDrivenObject)>("drivenObject", TDrivenObject {});
		registerMember(std::move(memberInfo));
	}
	{
		using TParameters = pragma::ents::Element;
		auto memberInfo = create_component_member_info<T, TParameters,
		  // For some reasons these don't work as lambdas (VS compiler bug?)
		  &set_parameters, &get_parameters>("parameters");
		registerMember(std::move(memberInfo));
	}
	{
		using TExpr = std::string;
		auto memberInfo = create_component_member_info<T, TExpr, &T::SetExpression, &T::GetExpression>("expression");
		registerMember(std::move(memberInfo));
	}
}

pragma::AnimationDriverComponent::AnimationDriverComponent(BaseEntity &ent) : BaseEntityComponent {ent}, m_parameters {udm::Property::Create<udm::Element>()}, m_descriptor {ent.GetLuaState()} {}

udm::PProperty &AnimationDriverComponent::GetParameters() { return m_parameters; }
void AnimationDriverComponent::UpdateParameters()
{
	m_driverDirty = true;

	m_descriptor.ClearConstants();
	auto udmConstants = (*m_parameters)["constants"];
	for(auto pair : udmConstants.ElIt()) {
		auto *prop = pair.property.GetProperty();
		if(!prop) {
			spdlog::trace("Animation driver '{}' has invalid constant parameter '{}'!", GetEntity().ToString(), pair.key);
			continue;
		}
		auto cpy = pair.property.GetProperty()->Copy();
		assert(cpy != nullptr);
		m_descriptor.AddConstant(std::string {pair.key}, cpy);
	}

	m_descriptor.ClearReferences();
	auto udmReferences = (*m_parameters)["references"];
	for(auto pair : udmReferences.ElIt()) {
		auto val = pair.property.ToValue<udm::String>();
		if(!val) {
			spdlog::trace("Animation driver '{}' has invalid reference parameter '{}'!", GetEntity().ToString(), pair.key);
			continue;
		}
		m_descriptor.AddReference(std::string {pair.key}, *val);
	}
}

void AnimationDriverComponent::SetDrivenObject(const pragma::EntityUComponentMemberRef &drivenObject) { m_drivenObject = drivenObject; }
const pragma::EntityUComponentMemberRef &AnimationDriverComponent::GetDrivenObject() const { return m_drivenObject; }

void pragma::AnimationDriverComponent::Initialize() { BaseEntityComponent::Initialize(); }
void pragma::AnimationDriverComponent::OnRemove() { BaseEntityComponent::OnRemove(); }
void AnimationDriverComponent::OnEntityComponentAdded(BaseEntityComponent &component) { BaseEntityComponent::OnEntityComponentAdded(component); }
void AnimationDriverComponent::OnEntityComponentRemoved(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentRemoved(component);
	/*if(typeid(component) == typeid(PanimaComponent))
	{
		if(m_cbOnAnimationsUpdated.IsValid())
			m_cbOnAnimationsUpdated.Remove();
	}*/
}
std::optional<ComponentMemberIndex> pragma::AnimationDriverComponent::FindComponentMember(ComponentId componentId, const std::string &memberName)
{
	auto *info = GetEntity().GetComponentManager()->GetComponentInfo(componentId);
	if(info) {
		auto memberIdx = info->FindMember(memberName);
		if(memberIdx.has_value())
			return memberIdx;
	}
	auto hComponent = GetEntity().FindComponent(componentId);
	if(hComponent.expired())
		return {};
	return hComponent->GetMemberIndex(memberName);
}
void pragma::AnimationDriverComponent::SetExpression(const std::string &expression)
{
	m_descriptor.SetExpression(expression);
	m_driverDirty = true;
}
const std::string &pragma::AnimationDriverComponent::GetExpression() const { return m_descriptor.GetExpression(); }

void pragma::AnimationDriverComponent::AddConstant(const std::string &name, const udm::PProperty &prop)
{
	m_descriptor.AddConstant(name, prop);
	m_driverDirty = true;
}
void pragma::AnimationDriverComponent::AddReference(const std::string &name, std::string path)
{
	m_descriptor.AddReference(name, path);
	m_driverDirty = true;
}

const std::unordered_map<std::string, udm::PProperty> &pragma::AnimationDriverComponent::GetConstants() const { return m_descriptor.GetConstants(); }
const std::unordered_map<std::string, std::string> &pragma::AnimationDriverComponent::GetReferences() const { return m_descriptor.GetReferences(); }
bool pragma::AnimationDriverComponent::ApplyDriver()
{
	if(m_driverDirty) {
		m_driverDirty = false;
		m_driver = nullptr;
		auto componentId = m_drivenObject.GetComponentId();
		auto uuid = m_drivenObject.GetUuid();
		if(componentId != INVALID_COMPONENT_ID && uuid.has_value())
			m_driver = std::make_unique<ValueDriver>(componentId, *m_drivenObject, m_descriptor, *uuid);
	}
	if(!m_driver)
		return false;
	auto &game = *GetEntity().GetNetworkState()->GetGameState();
	auto *ent = m_drivenObject.GetEntity(game);
	if(!ent) {
		spdlog::trace("Execution of animation driver '{}' failed: Driven entity does not exist!", GetEntity().ToString());
		return false;
	}
	auto *l = GetLuaState();
	auto res = m_driver->Apply(*ent);
	if(res != pragma::ValueDriver::Result::Success)
		spdlog::trace("Execution of animation driver '{}' failed with result code {}!", GetEntity().ToString(), magic_enum::enum_name(res));
	return res == pragma::ValueDriver::Result::Success;
}

void pragma::AnimationDriverComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void pragma::AnimationDriverComponent::Save(udm::LinkedPropertyWrapperArg udm) { BaseEntityComponent::Save(udm); }

void pragma::AnimationDriverComponent::Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) { BaseEntityComponent::Load(udm, version); }
