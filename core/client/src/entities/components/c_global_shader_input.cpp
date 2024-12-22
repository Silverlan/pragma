/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_global_shader_input.hpp"
#include "pragma/rendering/global_shader_input_manager.hpp"
#include "pragma/lua/lua_util_component.hpp"
#include <pragma/entities/entity_component_manager_t.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT CEngine *c_engine;

using namespace pragma;
void CGlobalShaderInputComponent::RegisterLuaBindings(lua_State *l, luabind::module_ &modEnts)
{
	auto def = pragma::lua::create_entity_component_class<pragma::CGlobalShaderInputComponent, pragma::BaseEntityComponent>("GlobalShaderInputComponent");
	modEnts[def];
}

void CGlobalShaderInputComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CGlobalShaderInputComponent::Initialize() { BaseEntityComponent::Initialize(); }

const ComponentMemberInfo *CGlobalShaderInputComponent::GetMemberInfo(ComponentMemberIndex idx) const
{
	auto numStatic = GetStaticMemberCount();
	if(idx < numStatic)
		return BaseEntityComponent::GetMemberInfo(idx);
	return DynamicMemberRegister::GetMemberInfo(idx);
}

std::optional<ComponentMemberIndex> CGlobalShaderInputComponent::DoGetMemberIndex(const std::string &name) const
{
	auto idx = BaseEntityComponent::DoGetMemberIndex(name);
	if(idx.has_value())
		return idx;
	idx = DynamicMemberRegister::GetMemberIndex(name);
	if(idx.has_value())
		return *idx; // +GetStaticMemberCount();
	return std::optional<ComponentMemberIndex> {};
}

rendering::GlobalShaderInputDataManager &CGlobalShaderInputComponent::GetInputManager() { return c_game->GetGlobalShaderInputDataManager(); }

void CGlobalShaderInputComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	UpdateComponentMembers();
}

void CGlobalShaderInputComponent::UpdateComponentMembers()
{
	auto &inputManager = c_game->GetGlobalShaderInputDataManager();
	auto &descriptor = inputManager.GetDescriptor();
	ReserveMembers(descriptor.properties.size());
	m_inputVarNames.reserve(m_inputVarNames.size() + descriptor.properties.size());
	for(auto &prop : descriptor.properties) {
		auto idx = BaseEntityComponent::GetMemberIndex(prop->name);
		if(idx.has_value())
			continue;
		auto memberInfo = pragma::ComponentMemberInfo::CreateDummy();
		memberInfo.SetName(prop->name);
		memberInfo.type = static_cast<ents::EntityMemberType>(shadergraph::to_udm_type(prop.parameter.type));
		memberInfo.SetMin(prop->min);
		memberInfo.SetMax(prop->max);
		m_inputVarNames.push_back(prop->name);
		idx = m_inputVarNames.size() - 1;
		memberInfo.userIndex = *idx;

		using TComponent = CGlobalShaderInputComponent;
		pragma::shadergraph::visit(prop->type, [this, &memberInfo](auto tag) {
			using T = typename decltype(tag)::type;
			memberInfo.SetGetterFunction<TComponent, T, static_cast<void (*)(const pragma::ComponentMemberInfo &, TComponent &, T &)>([](const pragma::ComponentMemberInfo &memberInfo, TComponent &component, T &outValue) {
				auto &name = component.m_inputVarNames[memberInfo.userIndex];
				component.GetShaderInputValue(name, outValue);
			})>();
			memberInfo.SetSetterFunction<TComponent, T, static_cast<void (*)(const pragma::ComponentMemberInfo &, TComponent &, const T &)>([](const pragma::ComponentMemberInfo &memberInfo, TComponent &component, const T &value) {
				auto &name = component.m_inputVarNames[memberInfo.userIndex];
				component.SetShaderInputValue(name, value);
			})>();
		});

		RegisterMember(std::move(memberInfo));
	}
	OnMembersChanged();
}
