/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2025 Silverlan
 */

module;

#include "stdafx_client.h"
#include "pragma/entities/components/c_time_scale_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/model/c_model.h"
#include "pragma/lua/lua_util_component.hpp"
#include <pragma/asset/util_asset.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <cmaterial.h>

module pragma.client.entities.components.material_override;

using namespace pragma;
ComponentEventId CMaterialOverrideComponent::EVENT_ON_MATERIAL_OVERRIDES_CLEARED = INVALID_COMPONENT_ID;
void CMaterialOverrideComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	BaseModelComponent::RegisterEvents(componentManager, registerEvent);
	EVENT_ON_MATERIAL_OVERRIDES_CLEARED = registerEvent("EVENT_ON_MATERIAL_OVERRIDES_CLEARED", ComponentEventInfo::Type::Broadcast);
}

CMaterialOverrideComponent::~CMaterialOverrideComponent() {}
void CMaterialOverrideComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CMaterialOverrideComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BindEvent(CModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		m_materialOverrides.clear();
		return util::EventReply::Unhandled;
	});
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
	UpdateMaterialOverride(mat);
}
void CMaterialOverrideComponent::UpdateMaterialOverride(Material &mat)
{
	auto *mdlC = static_cast<CModelComponent *>(GetEntity().GetModelComponent());
	if(mdlC)
		mdlC->SetRenderBufferListUpdateRequired();
	static_cast<CMaterial &>(mat).LoadTextures(false, true);
	mat.UpdateTextures(true); // Ensure all textures have been fully loaded
	if(mdlC)
		mdlC->UpdateRenderBufferList();
}
void CMaterialOverrideComponent::ClearMaterialOverride(uint32_t idx)
{
	if(idx >= m_materialOverrides.size())
		return;
	m_materialOverrides.at(idx).materialOverride = {};
	auto *mdlC = static_cast<CModelComponent *>(GetEntity().GetModelComponent());
	if(mdlC)
		mdlC->SetRenderBufferListUpdateRequired();
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

CMaterial *CMaterialOverrideComponent::GetRenderMaterial(uint32_t idx) const
{
	if(idx >= m_materialOverrides.size())
		return nullptr;
	auto &matOverride = m_materialOverrides[idx];
	if(matOverride.propertyOverride)
		return static_cast<CMaterial *>(matOverride.propertyOverride.get());
	return static_cast<CMaterial *>(matOverride.materialOverride.get());
}

void CMaterialOverrideComponent::ApplyMaterialPropertyOverride(Material &mat, const pragma::rendering::MaterialPropertyBlock &matPropOverride)
{
	for(auto &pair : udm::LinkedPropertyWrapper {matPropOverride.GetPropertyBlock()}.ElIt()) {
		udm::visit(pair.property->type, [&mat, &pair](auto tag) {
			using T = typename decltype(tag)::type;
			if constexpr(msys::is_property_type<T>) {
				auto &val = pair.property->GetValue<T>();
				mat.SetProperty<T>(pair.key, val);
			}
		});
	}

	for(auto &pair : udm::LinkedPropertyWrapper {matPropOverride.GetTextureBlock()}.ElIt()) {
		udm::visit(pair.property->type, [&mat, &pair](auto tag) {
			using T = typename decltype(tag)::type;
			if constexpr(udm::is_convertible<T, udm::String>()) {
				auto val = pair.property->ToValue<udm::String>();
				if(val)
					mat.SetTextureProperty(pair.key, *val);
			}
		});
	}
}

size_t CMaterialOverrideComponent::GetMaterialOverrideCount() const { return m_materialOverrides.size(); }

void CMaterialOverrideComponent::UpdateMaterialPropertyOverride(size_t matIdx)
{
	// TODO: Clear current override
	auto *propOverride = GetMaterialPropertyOverride(matIdx);
	if(!m_globalMaterialPropertyOverride && !propOverride)
		return;
	auto &mdl = GetEntity().GetModel();
	if(!mdl)
		return;
	auto *matOverride = GetMaterialOverride(matIdx);
	auto *mat = matOverride ? matOverride : mdl->GetMaterial(matIdx);
	if(!mat)
		return;
	// TODO: Don't create a full copy if we already have one?
	auto newMat = mat->Copy();
	if(!newMat)
		return;
	if(m_globalMaterialPropertyOverride)
		ApplyMaterialPropertyOverride(*newMat, m_globalMaterialPropertyOverride->block);
	if(propOverride)
		ApplyMaterialPropertyOverride(*newMat, *propOverride);

	if(matIdx >= m_materialOverrides.size())
		m_materialOverrides.resize(matIdx + 1);
	m_materialOverrides[matIdx].propertyOverride = newMat->shared_from_this();
	UpdateMaterialOverride(*newMat);
}

void CMaterialOverrideComponent::ClearMaterialPropertyOverride() { m_globalMaterialPropertyOverride = nullptr; }
void CMaterialOverrideComponent::SetMaterialPropertyOverride(const pragma::rendering::MaterialPropertyBlock &block)
{
	m_globalMaterialPropertyOverride = std::make_unique<MaterialPropertyOverride>();
	m_globalMaterialPropertyOverride->block = block;
	UpdateMaterialPropertyOverride(0);
}
const pragma::rendering::MaterialPropertyBlock *CMaterialOverrideComponent::GetMaterialPropertyOverride() const
{
	if(!m_globalMaterialPropertyOverride)
		return nullptr;
	return &m_globalMaterialPropertyOverride->block;
}

void CMaterialOverrideComponent::ClearMaterialPropertyOverrides() { m_materialPropertyOverrides.clear(); }
void CMaterialOverrideComponent::ClearMaterialPropertyOverride(size_t materialIndex)
{
	//auto it = std::find_if(m_materialPropertyOverrides.begin(), m_materialPropertyOverrides.end(), [materialIndex](const MaterialPropertyOverride &override) { return override.materialIndex == materialIndex; });
	//if(it != m_materialPropertyOverrides.end())
	//	m_materialPropertyOverrides.erase(it);
}

void CMaterialOverrideComponent::SetMaterialPropertyOverride(size_t materialIndex, const pragma::rendering::MaterialPropertyBlock &block)
{
	//auto it = std::find_if(m_materialPropertyOverrides.begin(), m_materialPropertyOverrides.end(), [materialIndex](const MaterialPropertyOverride &override) { return override.materialIndex == materialIndex; });
	//if(it != m_materialPropertyOverrides.end())
	//	it->block = block;
	//else
	//	m_materialPropertyOverrides.push_back({materialIndex, block});
}
const pragma::rendering::MaterialPropertyBlock *CMaterialOverrideComponent::GetMaterialPropertyOverride(size_t materialIndex) const
{
	//auto it = std::find_if(m_materialPropertyOverrides.begin(), m_materialPropertyOverrides.end(), [materialIndex](const MaterialPropertyOverride &override) { return override.materialIndex == materialIndex; });
	//return (it != m_materialPropertyOverrides.end()) ? &it->block : nullptr;
	return nullptr;
}

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
	def.def("ClearMaterialPropertyOverrides", static_cast<void (pragma::CMaterialOverrideComponent::*)()>(&pragma::CMaterialOverrideComponent::ClearMaterialPropertyOverrides));
	def.def("ClearMaterialPropertyOverride", static_cast<void (pragma::CMaterialOverrideComponent::*)(size_t)>(&pragma::CMaterialOverrideComponent::ClearMaterialPropertyOverride));
	def.def("ClearMaterialPropertyOverride", static_cast<void (pragma::CMaterialOverrideComponent::*)()>(&pragma::CMaterialOverrideComponent::ClearMaterialPropertyOverride));
	def.def("SetMaterialPropertyOverride", static_cast<void (pragma::CMaterialOverrideComponent::*)(size_t, const pragma::rendering::MaterialPropertyBlock &)>(&pragma::CMaterialOverrideComponent::SetMaterialPropertyOverride));
	def.def("SetMaterialPropertyOverride", static_cast<void (pragma::CMaterialOverrideComponent::*)(const pragma::rendering::MaterialPropertyBlock &)>(&pragma::CMaterialOverrideComponent::SetMaterialPropertyOverride));
	def.def("GetMaterialPropertyOverride", static_cast<const pragma::rendering::MaterialPropertyBlock *(pragma::CMaterialOverrideComponent::*)(size_t) const>(&pragma::CMaterialOverrideComponent::GetMaterialPropertyOverride));
	def.def("GetMaterialPropertyOverride", static_cast<const pragma::rendering::MaterialPropertyBlock *(pragma::CMaterialOverrideComponent::*)() const>(&pragma::CMaterialOverrideComponent::GetMaterialPropertyOverride));
	modEnts[def];
}
