/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2025 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/material_property_override.hpp"
#include "pragma/entities/components/material_override.hpp"
#include "pragma/entities/components/c_time_scale_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
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
void CMaterialPropertyOverrideComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { BaseModelComponent::RegisterEvents(componentManager, registerEvent); }

CMaterialPropertyOverrideComponent::ShaderMaterialPropertyInfo::ShaderMaterialPropertyInfo() : propertyOverride {std::make_unique<MaterialPropertyOverride>()} {}
CMaterialPropertyOverrideComponent::ShaderMaterialPropertyInfo::ShaderMaterialPropertyInfo(const ShaderMaterialPropertyInfo &other)
    : propertyOverride {other.propertyOverride ? std::make_unique<MaterialPropertyOverride>(*other.propertyOverride) : std::unique_ptr<MaterialPropertyOverride> {}}, name {other.name}, enabled {other.enabled}, materialIndex {other.materialIndex}
{
}

static std::vector<CMaterialPropertyOverrideComponent *> g_components;
static CallbackHandle g_updateRenderBuffersCallback = {};
CMaterialPropertyOverrideComponent::CMaterialPropertyOverrideComponent(BaseEntity &ent) : BaseEntityComponent(ent)
{
	if(g_components.empty()) {
		g_updateRenderBuffersCallback = GetGame().AddCallback("UpdateRenderBuffers", FunctionCallback<void, std::reference_wrapper<const util::DrawSceneInfo>>::Create([](std::reference_wrapper<const util::DrawSceneInfo> drawSceneInfo) {
			for(auto *c : g_components)
				c->UpdateRenderBuffers(*drawSceneInfo.get().commandBuffer);
		}));
	}
	g_components.push_back(this);
}

CMaterialPropertyOverrideComponent::~CMaterialPropertyOverrideComponent()
{
	auto it = std::find(g_components.begin(), g_components.end(), this);
	if(it != g_components.end())
		g_components.erase(it);
	if(g_components.empty()) {
		if(g_updateRenderBuffersCallback.IsValid())
			g_updateRenderBuffersCallback.Remove();
	}
}

Material *CMaterialPropertyOverrideComponent::GetTargetMaterial(uint32_t matIdx)
{
	auto &mdl = GetEntity().GetModel();
	if(!mdl)
		return nullptr;
	auto &mats = mdl->GetMaterials();
	if(matIdx >= mats.size())
		return nullptr;
	return mats[matIdx].get();
}

CMaterial *CMaterialPropertyOverrideComponent::GetRenderMaterial(uint32_t idx) const
{
	if(idx >= m_materialOverrides.size())
		return nullptr;
	return static_cast<CMaterial *>(m_materialOverrides[idx].material.get());
}

udm::Property *CMaterialPropertyOverrideComponent::InitializeMaterialProperty(uint32_t matIdx, const char *key)
{
	auto *mat = GetTargetMaterial(matIdx);
	if(!mat)
		return nullptr;
	auto *shaderMat = GetShaderMaterial(static_cast<CMaterial &>(*mat));
	if(!shaderMat)
		return nullptr;
	auto *smProp = shaderMat->FindProperty(key);
	auto *tex = !smProp ? shaderMat->FindTexture(key) : nullptr;
	if(!smProp && !tex)
		return nullptr;
	if(matIdx >= m_materialOverrides.size()) {
		UpdateMaterialOverride(matIdx, static_cast<CMaterial &>(*mat), true);
		if(matIdx >= m_materialOverrides.size())
			return nullptr;
	}
	auto &matData = m_materialOverrides[matIdx];
	auto it = matData.properties.find(key);
	if(it != matData.properties.end())
		return it->second.property.get();
	auto udmType = tex ? udm::Type::String : pragma::shadergraph::to_udm_type((*smProp)->type);
	auto prop = udm::Property::Create(udmType);
	auto res = udm::visit(udmType, [&prop, &mat, key](auto tag) -> bool {
		using T = typename decltype(tag)::type;
		if constexpr(msys::is_property_type<T>) {
			T val;
			if(!mat->GetProperty<T>(key, &val))
				return false;
			*prop = val;
			return true;
		}
		return false;
	});
	if(!res)
		return nullptr;
	matData.properties[key] = {prop, true, tex ? true : false};
	return prop.get();
}

void CMaterialPropertyOverrideComponent::ClearMaterialProperty(uint32_t matIdx, const char *key)
{
	if(matIdx >= m_materialOverrides.size())
		return;
	auto &matData = m_materialOverrides[matIdx];
	auto it = matData.properties.find(key);
	if(it == matData.properties.end())
		return;
	auto propType = it->second.property->type;
	matData.properties.erase(it);
	if(matData.material) {
		auto valType = matData.material->GetPropertyValueType(key);
		matData.material->ClearProperty(key);
		auto *shaderMat = GetShaderMaterial(static_cast<CMaterial &>(*matData.material));
		if(shaderMat) {
			udm::visit(propType, [this, &matData, &key, shaderMat](auto tag) {
				using T = typename decltype(tag)::type;
				if constexpr(msys::is_property_type<T>) {
					Vector3 val;
					if(matData.material->GetProperty(key, &val))
						SetMaterialPropertyBufferValue(*matData.material, *shaderMat, key, val);
				}
			});
		}
		if(matData.properties.empty()) {
			//matData.material = nullptr;
			// TODO: Force model component to reload
		}

		if(valType == ds::ValueType::Texture) {
			if(matIdx < m_materialOverrides.size()) {
				auto &matInfo = m_materialOverrides[matIdx];
				if(matInfo.material) {
					UpdateMaterialOverride(static_cast<CMaterial &>(*matInfo.material));
					//UpdateMaterialOverride(matIdx, static_cast<CMaterial &>(*matInfo.material), true);
				}
			}
		}
	}
}

udm::Property *CMaterialPropertyOverrideComponent::InitializeMaterialProperty(const char *key)
{
#if 0
	auto *mat = GetTargetMaterial(matIdx);
	if(!mat)
		return nullptr;
	auto propType = mat->GetPropertyType(key);
	if(propType == msys::PropertyType::None || propType == msys::PropertyType::Block)
		return nullptr;
	auto valType = mat->GetPropertyValueType(key);
	if(valType == ds::ValueType::Invalid)
		return nullptr;
	if(matIdx >= m_materialOverrides.size()) {
		UpdateMaterialOverride(matIdx, static_cast<CMaterial &>(*mat), true);
		if(matIdx >= m_materialOverrides.size())
			return nullptr;
	}
	auto &matData = m_materialOverrides[matIdx];
	auto it = matData.properties.find(key);
	if(it != matData.properties.end())
		return it->second.property.get();
	auto udmType = msys::to_udm_type(valType);
	auto prop = udm::Property::Create(udmType);
	auto res = udm::visit(udmType, [&prop, &mat, key](auto tag) -> bool {
		using T = typename decltype(tag)::type;
		if constexpr(msys::is_property_type<T>) {
			T val;
			if(!mat->GetProperty<T>(key, &val))
				return false;
			*prop = val;
			return true;
		}
		return false;
	});
	if(!res)
		return nullptr;
	matData.properties[key] = {prop};
	return prop.get();
#endif
	return nullptr;
}

void CMaterialPropertyOverrideComponent::ClearMaterialProperty(const char *key)
{
#if 0
	if(matIdx >= m_materialOverrides.size())
		return;
	auto &matData = m_materialOverrides[matIdx];
	auto it = matData.properties.find(key);
	if(it == matData.properties.end())
		return;
	auto propType = it->second.property->type;
	matData.properties.erase(it);
	if(matData.material) {
		matData.material->ClearProperty(key);
		auto *shader = dynamic_cast<pragma::ShaderGameWorldLightingPass *>(pragma::get_cengine()->GetShader("pbr").get());
		if(shader) {
			auto *shaderMat = shader->GetShaderMaterial();
			if(shaderMat) {
				udm::visit(propType, [this, &matData, &key, shaderMat](auto tag) {
					using T = typename decltype(tag)::type;
					if constexpr(msys::is_property_type<T>) {
						Vector3 val;
						if(matData.material->GetProperty(key, &val))
							SetMaterialPropertyBufferValue(*matData.material, *shaderMat, key, val);
					}
				});
			}
		}
		if(matData.properties.empty()) {
			//matData.material = nullptr;
			// TODO: Force model component to reload
		}
	}
#endif
}

std::shared_ptr<Material> CMaterialPropertyOverrideComponent::InitializeMaterialCopy(const char *materialName)
{

#if 0
	auto *mat = GetTargetMaterial(materialName);
	if(!mat)
		return nullptr;
	auto *matData = FindMaterialData(materialName);
	if(!matData)
		return nullptr;
	if(matData->material)
		return matData->material->shared_from_this();
	auto newMat = mat->Copy();
	if(!newMat)
		return nullptr;
	matData->material = newMat;

	// TODO
	auto &mdl = GetEntity().GetModel();
	auto &mats = mdl->GetMaterials();
	for(size_t i = 0; i < mats.size(); ++i) {
		auto &mdlMat = mats[i];
		if(mdlMat.get() != mat)
			continue;
		if(i >= m_materialOverrides.size())
			m_materialOverrides.resize(i + 1);
		m_materialOverrides[i] = static_cast<CMaterial *>(newMat.get());
	}
	UpdateMaterialOverride(*newMat);

	return newMat;
#endif
	return nullptr;
}

CMaterialPropertyOverrideComponent::MaterialData *CMaterialPropertyOverrideComponent::FindMaterialData(const char *materialName)
{

#if 0
	auto itMatData = m_materialProperties.find(materialName);
	if(itMatData == m_materialProperties.end())
		return nullptr;
	return &itMatData->second;
#endif
	return nullptr;
}

CMaterialPropertyOverrideComponent::PropertyInfo *CMaterialPropertyOverrideComponent::FindMaterialPropertyInfo(uint32_t matIdx, const char *key)
{
	if(matIdx >= m_materialOverrides.size())
		return nullptr;
	auto &matInfo = m_materialOverrides[matIdx];
	auto it = matInfo.properties.find(key);
	if(it == matInfo.properties.end())
		return nullptr;
	return &it->second;
}

udm::Property *CMaterialPropertyOverrideComponent::FindMaterialProperty(uint32_t matIdx, const char *key)
{
	if(matIdx >= m_materialOverrides.size())
		return nullptr;
	auto *info = FindMaterialPropertyInfo(matIdx, key);
	if(!info)
		return nullptr;
	return info->property.get();
}

void CMaterialPropertyOverrideComponent::SetTexture(uint32_t matIdx, const char *key, const std::string &tex)
{

	SetMaterialProperty<std::string, true>(matIdx, key, tex);

	if(matIdx < m_materialOverrides.size()) {
		auto &matInfo = m_materialOverrides[matIdx];
		//if(matInfo.material)
		//	UpdateMaterialOverride(matIdx, static_cast<CMaterial &>(*matInfo.material), true);
	}
	/*auto *mat = GetRenderMaterial(0);
	if(!mat)
		return;
	mat->SetTextureProperty(keyName, tex);
	UpdateMaterialOverride(*mat);*/
}
std::string CMaterialPropertyOverrideComponent::GetTexture(uint32_t matIdx, const char *key) const
{
	//return GetProperty<std::string>(matIdx, key);
	return {};
	/*auto *mat = GetRenderMaterial(0);
	if(!mat)
		return {};
	return mat->GetProperty<std::string>(keyName, std::string {});*/
	//return {};
}

template<typename T, bool TEXTURE>
void CMaterialPropertyOverrideComponent::SetMaterialProperty(uint32_t matIdx, const char *key, const T &value)
{
	auto *prop = FindMaterialProperty(matIdx, key);
	if(!prop) {
		prop = InitializeMaterialProperty(matIdx, key);
		if(!prop)
			return;
	}
	*prop = value;

	// TODO: Re-create material if target material has changed
	//InitializeMaterialCopy(matIdx);

	// TODO
	//auto *mat = m_materialOverrides.front();

	// TODO: Create material copy
	// TODO: Apply property if enabled

	// TODO: Property to material(s)?
	auto *mat = GetRenderMaterial(0);
	if(!mat) {
		//SetMaterialPropertyOverride({});
		mat = GetRenderMaterial(0);
	}
	if(!mat)
		return;
	auto *shaderMat = GetShaderMaterial(*mat);
	if(!shaderMat)
		return;
	//MaterialPropertyOverride *o;
	//o->block.GetPropertyBlock();
	// TODO: Also apply to property override block?
	// TODO: How to handle texture overrides?
	if constexpr(TEXTURE)
		mat->SetTexture(key, value);
	else {
		mat->SetProperty(key, value);
		SetMaterialPropertyBufferValue(*mat, *shaderMat, key, value);
	}
}

void CMaterialPropertyOverrideComponent::UpdateRenderBuffers(prosper::IPrimaryCommandBuffer &drawCmd)
{
	while(!m_bufferUpdateQueue.empty()) {
		auto &info = m_bufferUpdateQueue.back();
		constexpr auto pipelineStages = prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::ComputeShaderBit | prosper::PipelineStageFlags::GeometryShaderBit;
		drawCmd.RecordBufferBarrier(*info.buffer, pipelineStages, prosper::PipelineStageFlags::TransferBit, prosper::AccessFlags::ShaderReadBit, prosper::AccessFlags::TransferWriteBit);
		drawCmd.RecordUpdateBuffer(*info.buffer, info.startOffset, info.size, info.data.get());
		drawCmd.RecordBufferBarrier(*info.buffer, prosper::PipelineStageFlags::TransferBit, pipelineStages, prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::ShaderReadBit);
		m_bufferUpdateQueue.pop();
	}
}

std::string CMaterialPropertyOverrideComponent::GetNormalizedMaterialName(std::string name)
{
	ustring::to_lower(name);
	return name;
}
std::string CMaterialPropertyOverrideComponent::GetNormalizedMaterialName(const CMaterial &mat) { return GetNormalizedMaterialName(std::string {util::FilePath(const_cast<CMaterial &>(mat).GetName()).GetBack()}); }

const rendering::shader_material::ShaderMaterial *CMaterialPropertyOverrideComponent::GetShaderMaterial(const CMaterial &mat)
{
	auto *shader = dynamic_cast<pragma::ShaderGameWorldLightingPass *>(const_cast<CMaterial &>(mat).GetPrimaryShader());
	if(!shader)
		return nullptr;
	return shader->GetShaderMaterial();
}

void CMaterialPropertyOverrideComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CMaterialPropertyOverrideComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BindEvent(CModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		m_materialOverrides.clear();
		if(GetEntity().IsSpawned())
			PopulateProperties();
		return util::EventReply::Unhandled;
	});
	BindEvent(CMaterialOverrideComponent::EVENT_ON_MATERIAL_OVERRIDE_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &evOc = static_cast<CEOnMaterialOverrideChanged &>(evData.get());
		UpdateMaterialOverride(evOc.materialIndex, evOc.material);
		return util::EventReply::Unhandled;
	});
}

void CMaterialPropertyOverrideComponent::UpdateMaterialOverride(uint32_t idx, const CMaterial &mat, bool forceInitialize)
{
	// TODO: If not base material index, skip!
	if(idx >= m_materialOverrides.size()) {
		if(!forceInitialize)
			return;
		m_materialOverrides.resize(idx + 1);
	}
	auto &matInfo = m_materialOverrides[idx];
	matInfo.material = nullptr;
	if(matInfo.properties.empty() && !forceInitialize)
		return;
	auto newMat = mat.Copy(false);
	if(!newMat)
		return;
	newMat->SetBaseMaterial(&const_cast<CMaterial &>(mat));
	matInfo.material = newMat;
	for(auto &[name, propInfo] : matInfo.properties) {
		if(!propInfo.enabled)
			continue;
		udm::visit(propInfo.property->type, [&newMat, &name, &propInfo](auto tag) {
			using T = typename decltype(tag)::type;
			if constexpr(msys::is_property_type<T>) {
				auto &val = propInfo.property->GetValue<T>();
				if constexpr(std::is_same_v<T, udm::String>) {
					if(propInfo.texture) {
						newMat->SetTextureProperty(name, val);
						return;
					}
				}
				newMat->SetProperty<T>(name, val);
			}
		});
	}
	UpdateMaterialOverride(*newMat);
}

void CMaterialPropertyOverrideComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	PopulateProperties();
}
/*
void CMaterialPropertyOverrideComponent::SetMaterialOverride(uint32_t idx, const std::string &matOverride)
{
	auto *mat = GetNetworkState().LoadMaterial(matOverride);
	if(mat)
		SetMaterialOverride(idx, static_cast<CMaterial &>(*mat));
	else
		ClearMaterialOverride(idx);
}
void CMaterialPropertyOverrideComponent::SetMaterialOverride(uint32_t idx, CMaterial &mat)
{
	if(idx >= m_materialOverrides.size())
		m_materialOverrides.resize(idx + 1);
	m_materialOverrides.at(idx).materialOverride = mat.GetHandle();
	UpdateMaterialOverride(mat);
}*/
void CMaterialPropertyOverrideComponent::UpdateMaterialOverride(Material &mat)
{
	auto *mdlC = static_cast<CModelComponent *>(GetEntity().GetModelComponent());
	if(mdlC)
		mdlC->SetRenderBufferListUpdateRequired();
	static_cast<CMaterial &>(mat).LoadTextures(false, true);
	mat.UpdateTextures(true); // Ensure all textures have been fully loaded
	if(mdlC)
		mdlC->UpdateRenderBufferList();
}
/*
void CMaterialPropertyOverrideComponent::ClearMaterialOverride(uint32_t idx)
{
	if(idx >= m_materialOverrides.size())
		return;
	m_materialOverrides.at(idx).materialOverride = {};
	auto *mdlC = static_cast<CModelComponent *>(GetEntity().GetModelComponent());
	if(mdlC)
		mdlC->SetRenderBufferListUpdateRequired();
}
void CMaterialPropertyOverrideComponent::ClearMaterialOverrides()
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
CMaterial *CMaterialPropertyOverrideComponent::GetMaterialOverride(uint32_t idx) const { return (idx < m_materialOverrides.size()) ? static_cast<CMaterial *>(m_materialOverrides.at(idx).materialOverride.get()) : nullptr; }

CMaterial *CMaterialPropertyOverrideComponent::GetRenderMaterial(uint32_t idx) const
{
	if(idx >= m_materialOverrides.size())
		return nullptr;
	auto &matOverride = m_materialOverrides[idx];
	if(matOverride.propertyOverride)
		return static_cast<CMaterial *>(matOverride.propertyOverride.get());
	return static_cast<CMaterial *>(matOverride.materialOverride.get());
}
*/
void CMaterialPropertyOverrideComponent::SetPropertyEnabled(const ComponentMemberInfo &info, CMaterialPropertyOverrideComponent &component, bool enabled)
{
	if(info.userIndex >= component.m_shaderMaterialPropertyInfos.size())
		return;
	auto &propInfo = component.m_shaderMaterialPropertyInfos[info.userIndex];
	propInfo.enabled = enabled;
}
void CMaterialPropertyOverrideComponent::GetPropertyEnabled(const ComponentMemberInfo &info, CMaterialPropertyOverrideComponent &component, bool &outEnabled)
{
	if(info.userIndex >= component.m_shaderMaterialPropertyInfos.size())
		return;
	auto &propInfo = component.m_shaderMaterialPropertyInfos[info.userIndex];
	outEnabled = propInfo.enabled;
}

void CMaterialPropertyOverrideComponent::PopulateProperties()
{
	ClearMembers();
	m_shaderMaterialPropertyInfos.clear();
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
		auto *shaderMat = GetShaderMaterial(static_cast<CMaterial &>(*mat));
		if(!shaderMat)
			continue;
		auto matName = std::string {util::FilePath(mat->GetName()).GetFileName()};
		ustring::to_lower(matName);
		std::string matPropName = "material/" + matName + "/";

		for(auto &[name, index] : shaderMat->GetPropertyMap()) {
			if(index >= shaderMat->properties.size())
				continue;
			auto &prop = shaderMat->properties[index];
			if(umath::is_flag_set(prop.propertyFlags, pragma::rendering::Property::Flags::HideInEditor))
				continue;

			auto propName = matPropName + name;
			auto propNameEnabled = propName + "_enabled";

			auto *cPropNameEnabled = pragma::register_global_string(propNameEnabled);
			auto *cPropName = pragma::register_global_string(name);
			auto idx = m_shaderMaterialPropertyInfos.size();
			m_shaderMaterialPropertyInfos.push_back({});
			auto &info = m_shaderMaterialPropertyInfos.back();
			info.name = cPropName;
			info.materialIndex = matIdx;

			auto memberInfo = pragma::ComponentMemberInfo::CreateDummy();
			memberInfo.SetName(propName);
			memberInfo.type = pragma::ents::udm_type_to_member_type(pragma::shadergraph::to_udm_type(prop->type));
			memberInfo.SetMin(prop->min);
			memberInfo.SetMax(prop->max);
			memberInfo.AddTypeMetaData<pragma::ents::OptionalTypeMetaData>().enabledProperty = cPropNameEnabled;
			memberInfo.userIndex = idx;
			using TComponent = CMaterialPropertyOverrideComponent;
			pragma::shadergraph::visit(prop->type, [&prop, &memberInfo](auto tag) {
				using T = typename decltype(tag)::type;
				T val;
				if(prop->defaultValue.Get<T>(val))
					memberInfo.SetDefault(val);

				using TValue = T;
				memberInfo.SetGetterFunction<TComponent, TValue, +[](const pragma::ComponentMemberInfo &memberInfo, TComponent &component, TValue &outValue) {
					auto &propInfo = component.m_shaderMaterialPropertyInfos[memberInfo.userIndex];
					outValue = component.GetProperty<TValue>(propInfo.name);
				}>();
				memberInfo.SetSetterFunction<TComponent, TValue, +[](const pragma::ComponentMemberInfo &memberInfo, TComponent &component, const TValue &value) {
					auto &propInfo = component.m_shaderMaterialPropertyInfos[memberInfo.userIndex];
					component.SetMaterialProperty(propInfo.materialIndex, propInfo.name, value);
				}>();
			});
			RegisterMember(std::move(memberInfo));

			auto memberInfoToggle = create_component_member_info<TComponent, bool, &CMaterialPropertyOverrideComponent::SetPropertyEnabled, &CMaterialPropertyOverrideComponent::GetPropertyEnabled>(cPropNameEnabled, false);
			memberInfoToggle.AddTypeMetaData<pragma::ents::EnablerTypeMetaData>().targetProperty = cPropName;
			RegisterMember(std::move(memberInfoToggle));

			/*


			using TValue = T;
			memberInfo.SetGetterFunction<TComponent, TValue, +[](const pragma::ComponentMemberInfo &memberInfo, TComponent &component, TValue &outValue) {
				auto &propInfo = component.m_shaderMaterialPropertyInfos[memberInfo.userIndex];
				outValue = component.GetProperty<TValue>(propInfo.name);
			}>();
			memberInfo.SetSetterFunction<TComponent, TValue, +[](const pragma::ComponentMemberInfo &memberInfo, TComponent &component, const TValue &value) {
				auto &propInfo = component.m_shaderMaterialPropertyInfos[memberInfo.userIndex];
				component.SetProperty(propInfo.name, value);
			}>();
		});*/
		}
	}
	OnMembersChanged();

#if 0
	ClearMembers();
	m_shaderMaterialPropertyInfos.clear();
	util::ScopeGuard sg {[this]() { OnMembersChanged(); }};

	auto &mdl = GetEntity().GetModel();
	if(!mdl)
		return;
	auto &mats = mdl->GetMaterials();
	std::unordered_set<const pragma::rendering::shader_material::ShaderMaterial *> shaderMaterials;
	shaderMaterials.reserve(mats.size());
	for(auto &mat : mats) {
		if(!mat)
			continue;
		auto *shaderMat = GetShaderMaterial(static_cast<CMaterial &>(*mat));
		if(!shaderMat)
			continue;
		shaderMaterials.insert(shaderMat);
	}

	std::unordered_map<std::string, const pragma::rendering::Property *> shaderProps;
	std::unordered_map<std::string, const pragma::rendering::shader_material::Texture *> shaderTextures;
	for(auto *shaderMat : shaderMaterials) {
		shaderProps.reserve(shaderProps.size() + shaderMat->properties.size());
		for(auto &[name, index] : shaderMat->GetPropertyMap()) {
			if(index >= shaderMat->properties.size())
				continue;
			std::string strName = name;
			if(shaderProps.find(strName) != shaderProps.end())
				continue;
			shaderProps[std::move(strName)] = &shaderMat->properties[index];
		}

		shaderTextures.reserve(shaderTextures.size() + shaderMat->textures.size());
		for(auto &tex : shaderMat->textures) {
			std::string strName = tex.name;
			if(shaderTextures.find(strName) != shaderTextures.end())
				continue;
			shaderTextures[std::move(strName)] = &tex;
		}
	}

	ReserveMembers(shaderProps.size() * 2 + shaderTextures.size());
	m_shaderMaterialPropertyInfos.reserve(shaderProps.size() + shaderTextures.size());
	for(auto &[name, prop] : shaderProps) {
		auto *cname = pragma::register_global_string(name);
		auto *cnameEnabled = pragma::register_global_string(name + "_enabled");

		auto idx = m_shaderMaterialPropertyInfos.size();
		m_shaderMaterialPropertyInfos.push_back({});
		auto &info = m_shaderMaterialPropertyInfos.back();
		info.name = cname;

		auto memberInfo = pragma::ComponentMemberInfo::CreateDummy();
		memberInfo.SetName(cname);
		memberInfo.type = pragma::ents::udm_type_to_member_type(pragma::shadergraph::to_udm_type((*prop)->type));
		memberInfo.SetMin((*prop)->min);
		memberInfo.SetMax((*prop)->max);
		memberInfo.AddTypeMetaData<pragma::ents::OptionalTypeMetaData>().enabledProperty = cnameEnabled;
		memberInfo.userIndex = idx;
		using TComponent = CMaterialPropertyOverrideComponent;
		pragma::shadergraph::visit((*prop)->type, [&prop, &memberInfo](auto tag) {
			using T = typename decltype(tag)::type;
			T val;
			if((*prop)->defaultValue.Get<T>(val))
				memberInfo.SetDefault(val);

			using TValue = T;
			memberInfo.SetGetterFunction<TComponent, TValue, +[](const pragma::ComponentMemberInfo &memberInfo, TComponent &component, TValue &outValue) {
				auto &propInfo = component.m_shaderMaterialPropertyInfos[memberInfo.userIndex];
				outValue = component.GetProperty<TValue>(propInfo.name);
			}>();
			memberInfo.SetSetterFunction<TComponent, TValue, +[](const pragma::ComponentMemberInfo &memberInfo, TComponent &component, const TValue &value) {
				auto &propInfo = component.m_shaderMaterialPropertyInfos[memberInfo.userIndex];
				component.SetProperty(propInfo.name, value);
			}>();
		});
		RegisterMember(std::move(memberInfo));

		{
			auto memberInfo = create_component_member_info<TComponent, bool, &CMaterialPropertyOverrideComponent::SetPropertyEnabled, &CMaterialPropertyOverrideComponent::GetPropertyEnabled>(cnameEnabled, false);
			memberInfo.AddTypeMetaData<pragma::ents::EnablerTypeMetaData>().targetProperty = cname;
			RegisterMember(std::move(memberInfo));
		}
	}

	for(auto &[name, tex] : shaderTextures) {
		auto idx = m_shaderMaterialPropertyInfos.size();
		m_shaderMaterialPropertyInfos.push_back({});
		auto &info = m_shaderMaterialPropertyInfos.back();
		info.name = *tex->name;

		auto memberInfo = pragma::ComponentMemberInfo::CreateDummy();
		memberInfo.SetName(tex->name);
		memberInfo.type = pragma::ents::EntityMemberType::String;
		memberInfo.userIndex = idx;
		if(tex->defaultTexturePath)
			memberInfo.SetDefault(*tex->defaultTexturePath);
		using TComponent = CMaterialPropertyOverrideComponent;
		using TValue = udm::String;
		memberInfo.SetGetterFunction<TComponent, TValue, +[](const pragma::ComponentMemberInfo &memberInfo, TComponent &component, TValue &outValue) {
			auto &propInfo = component.m_shaderMaterialPropertyInfos[memberInfo.userIndex];
			outValue = component.GetTexture(0, propInfo.name);
		}>();
		memberInfo.SetSetterFunction<TComponent, TValue, +[](const pragma::ComponentMemberInfo &memberInfo, TComponent &component, const TValue &value) {
			auto &propInfo = component.m_shaderMaterialPropertyInfos[memberInfo.userIndex];
			component.SetTexture(0, propInfo.name, value);
		}>();
		RegisterMember(std::move(memberInfo));
	}
#endif
}

template<typename T>
void CMaterialPropertyOverrideComponent::SetProperty(const char *keyName, const T &v)
{
#if 0
	// TODO: Property to material(s)?
	auto *mat = GetRenderMaterial(0);
	if(!mat) {
		SetMaterialPropertyOverride({});
		mat = GetRenderMaterial(0);
	}
	if(!mat)
		return;
	auto *shader = dynamic_cast<pragma::ShaderGameWorldLightingPass *>(pragma::get_cengine()->GetShader("pbr").get());
	if(!shader)
		return;
	auto *shaderMat = shader->GetShaderMaterial();
	if(!shaderMat)
		return;
	//MaterialPropertyOverride *o;
	//o->block.GetPropertyBlock();
	// TODO: Also apply to property override block?
	// TODO: How to handle texture overrides?
	mat->SetProperty(keyName, v);
	SetMaterialPropertyBufferValue(*mat, *shaderMat, keyName, v);
#endif
}
template<typename T>
T CMaterialPropertyOverrideComponent::GetProperty(const char *keyName) const
{
	/*auto *mat = GetRenderMaterial(0);
	if(!mat)
		return T {};
	return mat->GetProperty<T>(keyName, T {});*/
	return T {};
}

const ComponentMemberInfo *CMaterialPropertyOverrideComponent::GetMemberInfo(ComponentMemberIndex idx) const
{
	auto numStatic = GetStaticMemberCount();
	if(idx < numStatic)
		return BaseEntityComponent::GetMemberInfo(idx);
	return DynamicMemberRegister::GetMemberInfo(idx);
}

std::optional<ComponentMemberIndex> CMaterialPropertyOverrideComponent::DoGetMemberIndex(const std::string &name) const
{
	auto idx = BaseEntityComponent::DoGetMemberIndex(name);
	if(idx.has_value())
		return idx;
	idx = DynamicMemberRegister::GetMemberIndex(name);
	if(idx.has_value())
		return *idx; // +GetStaticMemberCount();
	return std::optional<ComponentMemberIndex> {};
}

template<typename T>
void CMaterialPropertyOverrideComponent::SetMaterialPropertyBufferValue(Material &mat, const pragma::rendering::shader_material::ShaderMaterial &shaderMat, const char *keyName, const T &newVal)
{
	auto &descriptor = shaderMat;
	auto *buf = static_cast<CMaterial &>(mat).GetSettingsBuffer();
	if(!buf)
		return;
	auto range = descriptor.GetPropertyDataRange(keyName);
	if(!range)
		return;
	auto [startOffset, size] = *range;
	UTIL_ASSERT(sizeof(T) == size);
	BufferUpdateInfo updateInfo {};
	updateInfo.buffer = buf->shared_from_this();
	updateInfo.startOffset = startOffset;
	updateInfo.size = size;
	updateInfo.data = std::make_unique<std::byte[]>(updateInfo.size);
	memcpy(updateInfo.data.get(), &newVal, sizeof(newVal));
	m_bufferUpdateQueue.emplace(std::move(updateInfo));
}

void CMaterialPropertyOverrideComponent::ApplyMaterialPropertyOverride(Material &mat, const pragma::rendering::MaterialPropertyBlock &matPropOverride)
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

//size_t CMaterialPropertyOverrideComponent::GetMaterialOverrideCount() const { return m_materialOverrides.size(); }

void CMaterialPropertyOverrideComponent::UpdateMaterialPropertyOverride(size_t matIdx)
{
#if 0
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
#endif
}
#if 0
void CMaterialPropertyOverrideComponent::ClearMaterialPropertyOverride() { m_globalMaterialPropertyOverride = nullptr; }
void CMaterialPropertyOverrideComponent::SetMaterialPropertyOverride(const pragma::rendering::MaterialPropertyBlock &block)
{
	m_globalMaterialPropertyOverride = std::make_unique<MaterialPropertyOverride>();
	m_globalMaterialPropertyOverride->block = block;
	UpdateMaterialPropertyOverride(0);
}
const pragma::rendering::MaterialPropertyBlock *CMaterialPropertyOverrideComponent::GetMaterialPropertyOverride() const
{
	if(!m_globalMaterialPropertyOverride)
		return nullptr;
	return &m_globalMaterialPropertyOverride->block;
}

void CMaterialPropertyOverrideComponent::ClearMaterialPropertyOverrides() { m_materialPropertyOverrides.clear(); }
void CMaterialPropertyOverrideComponent::ClearMaterialPropertyOverride(size_t materialIndex)
{
	//auto it = std::find_if(m_materialPropertyOverrides.begin(), m_materialPropertyOverrides.end(), [materialIndex](const MaterialPropertyOverride &override) { return override.materialIndex == materialIndex; });
	//if(it != m_materialPropertyOverrides.end())
	//	m_materialPropertyOverrides.erase(it);
}

void CMaterialPropertyOverrideComponent::SetMaterialPropertyOverride(size_t materialIndex, const pragma::rendering::MaterialPropertyBlock &block)
{
	//auto it = std::find_if(m_materialPropertyOverrides.begin(), m_materialPropertyOverrides.end(), [materialIndex](const MaterialPropertyOverride &override) { return override.materialIndex == materialIndex; });
	//if(it != m_materialPropertyOverrides.end())
	//	it->block = block;
	//else
	//	m_materialPropertyOverrides.push_back({materialIndex, block});
}
const pragma::rendering::MaterialPropertyBlock *CMaterialPropertyOverrideComponent::GetMaterialPropertyOverride(size_t materialIndex) const
{
	//auto it = std::find_if(m_materialPropertyOverrides.begin(), m_materialPropertyOverrides.end(), [materialIndex](const MaterialPropertyOverride &override) { return override.materialIndex == materialIndex; });
	//return (it != m_materialPropertyOverrides.end()) ? &it->block : nullptr;
	return nullptr;
}
#endif
void CMaterialPropertyOverrideComponent::RegisterLuaBindings(lua_State *l, luabind::module_ &modEnts)
{
	auto def = pragma::lua::create_entity_component_class<pragma::CMaterialPropertyOverrideComponent, pragma::BaseEntityComponent>("MaterialPropertyOverrideComponent");
	def.def("SetMaterialProperty", +[](CMaterialPropertyOverrideComponent &component, uint32_t matIdx, const char *key, const Vector3 &value) { component.SetMaterialProperty(matIdx, key, value); });
	def.def("ClearMaterialProperty", +[](CMaterialPropertyOverrideComponent &component, uint32_t matIdx, const char *key) { component.ClearMaterialProperty(matIdx, key); });
	def.def("SetTexture", &pragma::CMaterialPropertyOverrideComponent::SetTexture);
	def.def("GetTexture", &pragma::CMaterialPropertyOverrideComponent::GetTexture);
	//template<typename T>
	//void CMaterialPropertyOverrideComponent::SetMaterialProperty(const char *materialName, const char *key, const T &value)

	//def.def("SetMaterialOverride", static_cast<void (pragma::CMaterialPropertyOverrideComponent::*)(uint32_t, const std::string &)>(&pragma::CMaterialPropertyOverrideComponent::SetMaterialOverride));
	//def.def("SetMaterialOverride", static_cast<void (pragma::CMaterialPropertyOverrideComponent::*)(uint32_t, CMaterial &)>(&pragma::CMaterialPropertyOverrideComponent::SetMaterialOverride));
	/*def.def(
	  "SetMaterialOverride", +[](lua_State *l, pragma::CMaterialPropertyOverrideComponent &hModel, const std::string &matSrc, const std::string &matDst) {
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
	  "SetMaterialOverride", +[](lua_State *l, pragma::CMaterialPropertyOverrideComponent &hModel, const std::string &matSrc, CMaterial &matDst) {
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
	  });*/
	/*def.def(
	  "ClearMaterialOverride", +[](lua_State *l, pragma::CMaterialPropertyOverrideComponent &hModel, const std::string &matSrc) {
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
	def.def("ClearMaterialOverride", &pragma::CMaterialPropertyOverrideComponent::ClearMaterialOverride);
	def.def("ClearMaterialOverrides", &pragma::CMaterialPropertyOverrideComponent::ClearMaterialOverrides);
	def.def("GetMaterialOverride", &pragma::CMaterialPropertyOverrideComponent::GetMaterialOverride);
	def.def("GetMaterialOverrideCount", &pragma::CMaterialPropertyOverrideComponent::GetMaterialOverrideCount);
	def.def("GetRenderMaterial", &pragma::CMaterialPropertyOverrideComponent::GetRenderMaterial);
	def.def("PopulateProperties", &pragma::CMaterialPropertyOverrideComponent::PopulateProperties);
	def.def("ClearMaterialPropertyOverrides", static_cast<void (pragma::CMaterialPropertyOverrideComponent::*)()>(&pragma::CMaterialPropertyOverrideComponent::ClearMaterialPropertyOverrides));
	def.def("ClearMaterialPropertyOverride", static_cast<void (pragma::CMaterialPropertyOverrideComponent::*)(size_t)>(&pragma::CMaterialPropertyOverrideComponent::ClearMaterialPropertyOverride));
	def.def("ClearMaterialPropertyOverride", static_cast<void (pragma::CMaterialPropertyOverrideComponent::*)()>(&pragma::CMaterialPropertyOverrideComponent::ClearMaterialPropertyOverride));
	def.def("SetMaterialPropertyOverride", static_cast<void (pragma::CMaterialPropertyOverrideComponent::*)(size_t, const pragma::rendering::MaterialPropertyBlock &)>(&pragma::CMaterialPropertyOverrideComponent::SetMaterialPropertyOverride));
	def.def("SetMaterialPropertyOverride", static_cast<void (pragma::CMaterialPropertyOverrideComponent::*)(const pragma::rendering::MaterialPropertyBlock &)>(&pragma::CMaterialPropertyOverrideComponent::SetMaterialPropertyOverride));
	def.def("GetMaterialPropertyOverride", static_cast<const pragma::rendering::MaterialPropertyBlock *(pragma::CMaterialPropertyOverrideComponent::*)(size_t) const>(&pragma::CMaterialPropertyOverrideComponent::GetMaterialPropertyOverride));
	def.def("GetMaterialPropertyOverride", static_cast<const pragma::rendering::MaterialPropertyBlock *(pragma::CMaterialPropertyOverrideComponent::*)() const>(&pragma::CMaterialPropertyOverrideComponent::GetMaterialPropertyOverride));
	*/
	modEnts[def];
}
