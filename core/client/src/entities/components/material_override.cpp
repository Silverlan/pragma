/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2025 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/material_override.hpp"
#include "pragma/entities/components/c_time_scale_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/entities/components/material_property_override.hpp"
#include "pragma/entities/entity_component_system_t.hpp"
#include "pragma/model/c_model.h"
#include "pragma/lua/lua_util_component.hpp"
#include "pragma/rendering/global_shader_input_manager.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include <pragma/asset/util_asset.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <prosper_command_buffer.hpp>
#include <buffers/prosper_buffer.hpp>
#include <cmaterial.h>

using namespace pragma;
ComponentEventId CMaterialOverrideComponent::EVENT_ON_MATERIAL_OVERRIDES_CLEARED = INVALID_COMPONENT_ID;
ComponentEventId CMaterialOverrideComponent::EVENT_ON_MATERIAL_OVERRIDE_CHANGED = INVALID_COMPONENT_ID;
void CMaterialOverrideComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	BaseModelComponent::RegisterEvents(componentManager, registerEvent);
	EVENT_ON_MATERIAL_OVERRIDES_CLEARED = registerEvent("EVENT_ON_MATERIAL_OVERRIDES_CLEARED", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_MATERIAL_OVERRIDE_CHANGED = registerEvent("EVENT_ON_MATERIAL_OVERRIDE_CHANGED", ComponentEventInfo::Type::Broadcast);
}

CMaterialOverrideComponent::CMaterialOverrideComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}

CMaterialOverrideComponent::~CMaterialOverrideComponent() {}
void CMaterialOverrideComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CMaterialOverrideComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BindEventUnhandled(CModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		m_materialOverrides.clear();
		if(GetEntity().IsSpawned())
			PopulateProperties();
	});
}

void CMaterialOverrideComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	PopulateProperties();
}

void CMaterialOverrideComponent::OnRemove()
{
	m_materialOverrides.clear();
	BaseEntityComponent::OnRemove();
	if(GetEntity().IsRemoved())
		return;
	auto *mdlC = static_cast<CModelComponent *>(GetEntity().GetModelComponent());
	if(!mdlC)
		return;
	mdlC->SetRenderBufferListUpdateRequired();
	mdlC->UpdateRenderBufferList();
}

void CMaterialOverrideComponent::SetMaterialOverride(uint32_t idx, const std::string &matOverride)
{
	auto *mat = GetNetworkState().LoadMaterial(matOverride);
	if(mat)
		SetMaterialOverride(idx, static_cast<CMaterial &>(*mat));
	else
		ClearMaterialOverride(idx);
}
void CMaterialOverrideComponent::SetMaterialOverride(uint32_t idx, CMaterial &mat)
{
	if(idx >= m_materialOverrides.size())
		m_materialOverrides.resize(idx + 1);
	m_materialOverrides.at(idx).materialOverride = mat.GetHandle();
	UpdateMaterialOverride(idx, mat);
}
void CMaterialOverrideComponent::UpdateMaterialOverride(uint32_t matIdx, CMaterial &mat)
{
	auto *mdlC = static_cast<CModelComponent *>(GetEntity().GetModelComponent());
	if(mdlC)
		mdlC->SetRenderBufferListUpdateRequired();
	static_cast<CMaterial &>(mat).LoadTextures(false, true);
	mat.UpdateTextures(true); // Ensure all textures have been fully loaded
	if(mdlC)
		mdlC->UpdateRenderBufferList();

	BroadcastEvent(EVENT_ON_MATERIAL_OVERRIDE_CHANGED, CEOnMaterialOverrideChanged {matIdx, mat});
}
void CMaterialOverrideComponent::ClearMaterialOverride(uint32_t idx)
{
	if(idx >= m_materialOverrides.size())
		return;
	m_materialOverrides.at(idx).materialOverride = {};
	auto *mdlC = static_cast<CModelComponent *>(GetEntity().GetModelComponent());
	if(mdlC) {
		mdlC->SetRenderBufferListUpdateRequired();
		mdlC->UpdateRenderBufferList();
	}
}
void CMaterialOverrideComponent::ClearMaterialOverrides()
{
	if(m_materialOverrides.empty())
		return;
	for(auto &mo : m_materialOverrides)
		mo.materialOverride = {};
	auto *mdlC = static_cast<CModelComponent *>(GetEntity().GetModelComponent());
	if(mdlC)
		mdlC->SetRenderBufferListUpdateRequired();
	BroadcastEvent(EVENT_ON_MATERIAL_OVERRIDES_CLEARED);
}
CMaterial *CMaterialOverrideComponent::GetMaterialOverride(uint32_t idx) const { return (idx < m_materialOverrides.size()) ? static_cast<CMaterial *>(m_materialOverrides.at(idx).materialOverride.get()) : nullptr; }

const ComponentMemberInfo *CMaterialOverrideComponent::GetMemberInfo(ComponentMemberIndex idx) const
{
	auto numStatic = GetStaticMemberCount();
	if(idx < numStatic)
		return BaseEntityComponent::GetMemberInfo(idx);
	return DynamicMemberRegister::GetMemberInfo(idx);
}

std::optional<ComponentMemberIndex> CMaterialOverrideComponent::DoGetMemberIndex(const std::string &name) const
{
	auto idx = BaseEntityComponent::DoGetMemberIndex(name);
	if(idx.has_value())
		return idx;
	idx = DynamicMemberRegister::GetMemberIndex(name);
	if(idx.has_value())
		return *idx; // +GetStaticMemberCount();
	return std::optional<ComponentMemberIndex> {};
}

void CMaterialOverrideComponent::PopulateProperties()
{
	ClearMembers();
	util::ScopeGuard sg {[this]() { OnMembersChanged(); }};

	auto &mdl = GetEntity().GetModel();
	if(!mdl)
		return;

	auto &mats = mdl->GetMaterials();
	ReserveMembers(mats.size());
	for(size_t matIdx = 0; matIdx < mats.size(); ++matIdx) {
		auto &mat = mats[matIdx];
		if(!mat)
			continue;
		auto matName = std::string {util::FilePath(mat->GetName()).GetFileName()};
		ustring::to_lower(matName);

		auto propName = matName;
		auto *cPropName = pragma::register_global_string(propName);

		auto memberInfo = pragma::ComponentMemberInfo::CreateDummy();

		auto &metaData = memberInfo.AddMetaData();
		metaData["assetType"] = "material";
		metaData["rootPath"] = util::Path::CreatePath(pragma::asset::get_asset_root_directory(pragma::asset::Type::Material)).GetString();
		metaData["extensions"] = pragma::asset::get_supported_extensions(pragma::asset::Type::Material, pragma::asset::FormatType::All);
		metaData["stripRootPath"] = true;
		metaData["stripExtension"] = true;

		memberInfo.SetSpecializationType(AttributeSpecializationType::File);
		memberInfo.SetName(matName);
		memberInfo.type = ents::EntityMemberType::String;
		memberInfo.SetDefault<std::string>("");
		memberInfo.userIndex = matIdx;
		using TComponent = CMaterialOverrideComponent;
		using TValue = udm::String;
		memberInfo.SetGetterFunction<TComponent, TValue, +[](const pragma::ComponentMemberInfo &memberInfo, TComponent &component, TValue &outValue) {
			auto *matOverride = component.GetMaterialOverride(memberInfo.userIndex);
			if(!matOverride) {
				outValue = {};
				return;
			}
			outValue = util::FilePath(matOverride->GetName()).GetFileName();
		}>();
		memberInfo.SetSetterFunction<TComponent, TValue, +[](const pragma::ComponentMemberInfo &memberInfo, TComponent &component, const TValue &value) {
			if(value.empty()) {
				component.ClearMaterialOverride(memberInfo.userIndex);
				return;
			}
			component.SetMaterialOverride(memberInfo.userIndex, value);
		}>();
		RegisterMember(std::move(memberInfo));
	}
}

CMaterial *CMaterialOverrideComponent::GetRenderMaterial(uint32_t idx) const
{
	auto c = GetEntity().GetComponent<CMaterialPropertyOverrideComponent>();
	if(c.valid()) {
		auto *mat = c->GetRenderMaterial(idx);
		if(mat)
			return mat;
	}
	if(idx >= m_materialOverrides.size())
		return nullptr;
	auto &matOverride = m_materialOverrides[idx];
	return static_cast<CMaterial *>(matOverride.materialOverride.get());
}

size_t CMaterialOverrideComponent::GetMaterialOverrideCount() const { return m_materialOverrides.size(); }

void CMaterialOverrideComponent::RegisterLuaBindings(lua_State *l, luabind::module_ &modEnts)
{
	auto def = pragma::lua::create_entity_component_class<pragma::CMaterialOverrideComponent, pragma::BaseEntityComponent>("MaterialOverrideComponent");
	def.add_static_constant("EVENT_ON_MATERIAL_OVERRIDES_CLEARED", pragma::CMaterialOverrideComponent::EVENT_ON_MATERIAL_OVERRIDES_CLEARED);
	def.def("SetMaterialOverride", static_cast<void (pragma::CMaterialOverrideComponent::*)(uint32_t, const std::string &)>(&pragma::CMaterialOverrideComponent::SetMaterialOverride));
	def.def("SetMaterialOverride", static_cast<void (pragma::CMaterialOverrideComponent::*)(uint32_t, CMaterial &)>(&pragma::CMaterialOverrideComponent::SetMaterialOverride));
	def.def(
	  "SetMaterialOverride", +[](lua_State *l, pragma::CMaterialOverrideComponent &hModel, const std::string &matSrc, const std::string &matDst) {
		  auto &mdl = hModel.GetEntity().GetModel();
		  if(!mdl)
			  return;
		  auto &mats = mdl->GetMaterials();
		  auto it = std::find_if(mats.begin(), mats.end(), [&matSrc](const msys::MaterialHandle &hMat) {
			  if(!hMat)
				  return false;
			  return pragma::asset::matches(hMat.get()->GetName(), matSrc, pragma::asset::Type::Material);
		  });
		  if(it == mats.end())
			  return;
		  hModel.SetMaterialOverride(it - mats.begin(), matDst);
	  });
	def.def(
	  "SetMaterialOverride", +[](lua_State *l, pragma::CMaterialOverrideComponent &hModel, const std::string &matSrc, CMaterial &matDst) {
		  auto &mdl = hModel.GetEntity().GetModel();
		  if(!mdl)
			  return;
		  auto &mats = mdl->GetMaterials();
		  auto it = std::find_if(mats.begin(), mats.end(), [&matSrc](const msys::MaterialHandle &hMat) {
			  if(!hMat)
				  return false;
			  return pragma::asset::matches(hMat.get()->GetName(), matSrc, pragma::asset::Type::Material);
		  });
		  if(it == mats.end())
			  return;
		  hModel.SetMaterialOverride(it - mats.begin(), matDst);
	  });
	def.def(
	  "ClearMaterialOverride", +[](lua_State *l, pragma::CMaterialOverrideComponent &hModel, const std::string &matSrc) {
		  auto &mdl = hModel.GetEntity().GetModel();
		  if(!mdl)
			  return;
		  auto &mats = mdl->GetMaterials();
		  auto it = std::find_if(mats.begin(), mats.end(), [&matSrc](const msys::MaterialHandle &hMat) {
			  if(!hMat)
				  return false;
			  return pragma::asset::matches(hMat.get()->GetName(), matSrc, pragma::asset::Type::Material);
		  });
		  if(it == mats.end())
			  return;
		  hModel.ClearMaterialOverride(it - mats.begin());
	  });
	def.def("ClearMaterialOverride", &pragma::CMaterialOverrideComponent::ClearMaterialOverride);
	def.def("ClearMaterialOverrides", &pragma::CMaterialOverrideComponent::ClearMaterialOverrides);
	def.def("GetMaterialOverride", &pragma::CMaterialOverrideComponent::GetMaterialOverride);
	def.def("GetMaterialOverrideCount", &pragma::CMaterialOverrideComponent::GetMaterialOverrideCount);
	def.def("GetRenderMaterial", &pragma::CMaterialOverrideComponent::GetRenderMaterial);
	modEnts[def];
}

////////////

CEOnMaterialOverrideChanged::CEOnMaterialOverrideChanged(uint32_t idx, CMaterial &mat) : materialIndex {idx}, material {mat} {}
void CEOnMaterialOverrideChanged::PushArguments(lua_State *l) { Lua::PushInt(l, materialIndex); }
