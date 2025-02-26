/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/entities/components/c_global_shader_input.hpp"
#include "pragma/entities/components/base_entity_component_logging.hpp"
#include "pragma/rendering/global_shader_input_manager.hpp"
#include "pragma/lua/lua_util_component.hpp"
#include "pragma/game/gamemode/gamemodemanager.h"
#include <pragma/entities/entity_component_manager_t.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CEngine *c_engine;

using namespace pragma;
UdmPropertyList::UdmPropertyList() : m_properties {udm::Property::Create<udm::Element>()} {}
const udm::PProperty &UdmPropertyList::GetUdmData() const { return m_properties; }
void UdmPropertyList::AddProperty(const pragma::ComponentMemberInfo &memberInfo)
{
	if(!pragma::ents::is_udm_member_type(memberInfo.type))
		return;
	auto type = pragma::shadergraph::to_data_type(static_cast<udm::Type>(memberInfo.type));
	auto udmProps = (*m_properties)["properties"];
	auto *a = udmProps.GetValuePtr<udm::Array>();
	if(!a) {
		udm::LinkedPropertyWrapper parentProp {*m_properties};
		udmProps = parentProp.AddArray("properties", 1);
		a = udmProps.GetValuePtr<udm::Array>();
		if(!a)
			return;
	}
	else {
		auto it = m_nameToIndex.find(memberInfo.GetName());
		if(it != m_nameToIndex.end()) {
			auto item = (*a)[it->second];
			item["type"] << type;
			item["min"] << memberInfo.GetMin();
			item["max"] << memberInfo.GetMax();
			return;
		}

		a->Resize(a->GetSize() + 1);
	}
	auto idx = a->GetSize() - 1;
	auto newItem = (*a)[idx];
	newItem["name"] << memberInfo.GetName().c_str();
	newItem["type"] << type;
	newItem["min"] << memberInfo.GetMin();
	newItem["max"] << memberInfo.GetMax();

	m_nameToIndex[memberInfo.GetName()] = idx;
	if(idx >= m_indexToName.size())
		m_indexToName.resize(idx + 1);
	m_indexToName[idx] = memberInfo.GetName();
}
void UdmPropertyList::UpdateNameCache()
{
	m_nameToIndex.clear();
	m_indexToName.clear();
	auto udmProps = (*m_properties)["properties"];
	auto *a = udmProps.GetValuePtr<udm::Array>();
	if(a) {
		m_nameToIndex.reserve(a->GetSize());
		m_indexToName.reserve(a->GetSize());
		size_t idx = 0;
		for(auto &el : *a) {
			std::string name;
			if(el["name"] >> name) {
				m_nameToIndex[name] = idx;
				m_indexToName.push_back(name);
			}
			++idx;
		}
	}
}
bool UdmPropertyList::HasProperty(const std::string &name) const { return m_nameToIndex.find(name) != m_nameToIndex.end(); }

std::optional<size_t> UdmPropertyList::GetPropertyIndex(const std::string &name) const
{
	auto it = m_nameToIndex.find(name);
	if(it != m_nameToIndex.end())
		return it->second;
	return std::optional<size_t> {};
}
const std::string *UdmPropertyList::GetPropertyName(size_t index) const { return (index < m_indexToName.size()) ? &m_indexToName[index] : nullptr; }

// CGlobalShaderInputComponent

void CGlobalShaderInputComponent::RegisterLuaBindings(lua_State *l, luabind::module_ &modEnts)
{
	// TODO: This should be moved to a more appropriate location
	client->RegisterConCommand("debug_print_shader_input_properties", +[](NetworkState *nw, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv, float) { CGlobalShaderInputComponent::DebugPrintProperties(); });

	auto def = pragma::lua::create_entity_component_class<pragma::CGlobalShaderInputComponent, pragma::BaseEntityComponent>("GlobalShaderInputComponent");
	modEnts[def];
}

void CGlobalShaderInputComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	{
		using TRigConfig = pragma::ents::Element;
		auto memberInfo = create_component_member_info<CGlobalShaderInputComponent, TRigConfig, +[](const ComponentMemberInfo &memberInfo, CGlobalShaderInputComponent &component, const pragma::ents::Element &value) { component.UpdatePropertyCache(); },
		  +[](const ComponentMemberInfo &memberInfo, CGlobalShaderInputComponent &component, pragma::ents::Element &value) { value = component.m_propertyList.GetUdmData(); }>("properties");
		registerMember(std::move(memberInfo));
	}
}

void CGlobalShaderInputComponent::DebugPrintProperties()
{
	if(!c_game) {
		Con::cwar << "No game instance available" << Con::endl;
		return;
	}
	auto &inputDataManager = c_game->GetGlobalShaderInputDataManager();
	auto &descriptor = inputDataManager.GetDescriptor();
	auto &buf = inputDataManager.GetBuffer();
	auto &propMap = descriptor.GetPropertyMap();
	auto &props = descriptor.properties;

	if(props.empty()) {
		Con::cwar << "No global shader input properties available" << Con::endl;
		return;
	}

	pragma::rendering::ShaderInputData inputData {descriptor};
	inputData.ResizeToDescriptor();
	if(!buf->Read(0, inputData.data.size(), inputData.data.data())) {
		Con::cwar << "Failed to read global shader input data" << Con::endl;
		return;
	}

	std::stringstream ss;
	ss << std::left << std::setw(20) << "Property Name" << std::setw(10) << "Offset" << std::setw(10) << "Size"
	   << "Value" << "\n";
	ss << std::string(50, '-') << "\n"; // Add a separator line
	for(auto &[name, propIdx] : propMap) {
		auto &prop = props.at(propIdx);
		auto size = prop.GetSize();
		auto offset = prop.offset;
		auto udmType = pragma::shadergraph::to_udm_type(prop->type);
		auto strVal = udm::visit_ng(udmType, [&inputData, &name](auto tag) -> std::optional<std::string> {
			using TProp = typename decltype(tag)::type;
			if constexpr(pragma::shadergraph::is_data_type_v<TProp> && udm::is_convertible<TProp, std::string>()) {
				auto val = inputData.GetValue<TProp>(name.c_str());
				if(!val)
					return {};
				return udm::convert<TProp, std::string>(*val);
			}
			return {};
		});
		// Format and add the property information to the stream
		ss << std::left << std::setw(20) << name << std::setw(10) << offset << std::setw(10) << size << (strVal ? *strVal : "N/A") // Handle optional value
		   << "\n";
	}
	Con::cout << "Global shader input properties:\n";
	Con::cout << ss.str() << Con::endl;
}

CGlobalShaderInputComponent::CGlobalShaderInputComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}

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
	SyncShaderVarsToProperties();
}

void CGlobalShaderInputComponent::UpdatePropertyCache()
{
	m_propertyList.UpdateNameCache();
	UpdateMembers();
}

void CGlobalShaderInputComponent::UpdateMembers()
{
	auto propData = m_propertyList.GetPropertyData();
	auto *a = propData.GetValuePtr<udm::Array>();
	if(!a)
		return;

	std::vector<pragma::ComponentMemberInfo> newMembers;
	newMembers.reserve(a->GetSize());
	std::unordered_set<std::string> newPropNames;
	size_t propIdx = 0;
	for(auto udmProp : *a) {
		util::ScopeGuard idxGuard {[&propIdx] { ++propIdx; }};

		std::string name;
		udmProp["name"] >> name;
		if(name.empty()) {
			LogWarn("Invalid shader input property name at index {}", propIdx);
			continue;
		}
		auto idx = BaseEntityComponent::GetMemberIndex(name);
		if(idx.has_value()) {
			const_cast<pragma::ComponentMemberInfo *>(GetMemberInfo(*idx))->userIndex = propIdx;
			continue;
		}
		auto type = pragma::shadergraph::DataType::Invalid;
		udmProp["type"] >> type;
		if(type == pragma::shadergraph::DataType::Invalid) {
			LogWarn("Invalid shader input property type for property {}", name);
			continue;
		}
		auto memberInfo = pragma::ComponentMemberInfo::CreateDummy();
		memberInfo.SetName(name);
		memberInfo.type = static_cast<pragma::ents::EntityMemberType>(pragma::shadergraph::to_udm_type(type));

		float min;
		if(udmProp["min"] >> min)
			memberInfo.SetMin(min);
		float max;
		if(udmProp["max"] >> max)
			memberInfo.SetMax(max);

		memberInfo.userIndex = propIdx;

		using TComponent = CGlobalShaderInputComponent;
		pragma::shadergraph::visit(type, [this, &memberInfo](auto tag) {
			using T = typename decltype(tag)::type;
			memberInfo.SetGetterFunction<TComponent, T, +[](const pragma::ComponentMemberInfo &memberInfo, TComponent &component, T &outValue) {
				auto *name = component.m_propertyList.GetPropertyName(memberInfo.userIndex);
				if(!name)
					return;
				component.GetShaderInputValue(*name, outValue);
			}>();
			memberInfo.SetSetterFunction<TComponent, T, +[](const pragma::ComponentMemberInfo &memberInfo, TComponent &component, const T &value) {
				auto *name = component.m_propertyList.GetPropertyName(memberInfo.userIndex);
				if(!name)
					return;
				component.SetShaderInputValue(*name, value);
			}>();
		});

		newMembers.push_back(std::move(memberInfo));
		newPropNames.insert(name);
	}

	std::queue<std::string> toRemove;
	uint32_t i = GetStaticMemberCount();
	auto *info = GetMemberInfo(i++);
	while(info != nullptr) {
		std::string name {info->GetName()};
		if(m_propertyList.HasProperty(name) == false)
			toRemove.push(name);
		info = GetMemberInfo(i++);
	}

	while(toRemove.empty() == false) {
		RemoveMember(toRemove.front());
		toRemove.pop();
	}

	for(auto &memberInfo : newMembers) {
		auto udmType = pragma::ents::member_type_to_udm_type(memberInfo.type);
		pragma::rendering::Property prop {memberInfo.GetName(), pragma::shadergraph::to_data_type(udmType)};
		auto min = memberInfo.GetMin();
		if(min)
			prop->min = *min;
		auto max = memberInfo.GetMax();
		if(max)
			prop->max = *max;

		udm::visit_ng(udmType, [&memberInfo, &prop](auto tag) {
			using T = typename decltype(tag)::type;
			T defVal;
			if(memberInfo.GetDefault<T>(defVal))
				prop->defaultValue.Set(defVal);
		});

		c_game->GetGlobalShaderInputDataManager().AddProperty(std::move(prop));
		RegisterMember(std::move(memberInfo));
	}

	OnMembersChanged();
}

void CGlobalShaderInputComponent::SyncShaderVarsToProperties()
{
	auto &inputManager = c_game->GetGlobalShaderInputDataManager();
	auto &descriptor = inputManager.GetDescriptor();
	for(auto &prop : descriptor.properties) {
		auto idx = BaseEntityComponent::GetMemberIndex(prop->name);
		if(idx.has_value())
			continue;
		auto memberInfo = pragma::ComponentMemberInfo::CreateDummy();
		memberInfo.SetName(prop->name);
		memberInfo.type = static_cast<ents::EntityMemberType>(shadergraph::to_udm_type(prop.parameter.type));
		memberInfo.SetMin(prop->min);
		memberInfo.SetMax(prop->max);
		m_propertyList.AddProperty(memberInfo);
	}

	UpdateMembers();
}
