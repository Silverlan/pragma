// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.client;

import :entities.components.material_property_override;
import :engine;
import :entities.components.material_override;

using namespace pragma;
void CMaterialPropertyOverrideComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { BaseModelComponent::RegisterEvents(componentManager, registerEvent); }

CMaterialPropertyOverrideComponent::ShaderMaterialPropertyInfo::ShaderMaterialPropertyInfo() {}
CMaterialPropertyOverrideComponent::ShaderMaterialPropertyInfo::ShaderMaterialPropertyInfo(const ShaderMaterialPropertyInfo &other) : name {other.name}, materialIndex {other.materialIndex} {}

static std::vector<CMaterialPropertyOverrideComponent *> g_components;
static CallbackHandle g_updateRenderBuffersCallback = {};
CMaterialPropertyOverrideComponent::CMaterialPropertyOverrideComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent)
{
	if(g_components.empty()) {
		g_updateRenderBuffersCallback = GetGame().AddCallback("UpdateRenderBuffers", FunctionCallback<void, std::reference_wrapper<const rendering::DrawSceneInfo>>::Create([](std::reference_wrapper<const rendering::DrawSceneInfo> drawSceneInfo) {
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

material::Material *CMaterialPropertyOverrideComponent::GetTargetMaterial(uint32_t matIdx)
{
	auto &mdl = GetEntity().GetModel();
	if(!mdl)
		return nullptr;
	auto &mats = mdl->GetMaterials();
	if(matIdx >= mats.size())
		return nullptr;
	return mats[matIdx].get();
}

material::CMaterial *CMaterialPropertyOverrideComponent::GetRenderMaterial(uint32_t idx) const
{
	if(idx >= m_materialOverrides.size())
		return nullptr;
	return static_cast<material::CMaterial *>(m_materialOverrides[idx].material.get());
}

std::string CMaterialPropertyOverrideComponent::GetPropertyName(const material::CMaterial &mat, const char *key, bool texture) { return util::FilePath("material", GetNormalizedMaterialName(mat), texture ? "textures" : "properties", key).GetString(); }

CMaterialPropertyOverrideComponent::PropertyInfo *CMaterialPropertyOverrideComponent::InitializeMaterialProperty(uint32_t matIdx, const char *key)
{
	auto *mat = GetTargetMaterial(matIdx);
	if(!mat)
		return nullptr;
	auto *shaderMat = GetShaderMaterial(static_cast<material::CMaterial &>(*mat));
	if(!shaderMat)
		return nullptr;
	auto *smProp = shaderMat->FindProperty(key);
	auto *tex = !smProp ? shaderMat->FindTexture(key) : nullptr;
	if(!smProp && !tex)
		return nullptr;
	if(matIdx >= m_materialOverrides.size()) {
		UpdateMaterialOverride(matIdx, static_cast<material::CMaterial &>(*mat), true);
		if(matIdx >= m_materialOverrides.size())
			return nullptr;
	}
	auto &matData = m_materialOverrides[matIdx];
	auto it = matData.properties.find(key);
	if(it != matData.properties.end())
		return &it->second;
	auto udmType = tex ? udm::Type::String : pragma::shadergraph::to_udm_type((*smProp)->type);
	auto prop = udm::Property::Create(udmType);
	auto *memberInfo = FindMemberInfo(GetPropertyName(static_cast<material::CMaterial &>(*mat), key, tex ? true : false));
	auto res = udm::visit(udmType, [&prop, memberInfo, key](auto tag) -> bool {
		using T = typename decltype(tag)::type;
		if constexpr(material::is_property_type<T>) {
			T val;
			if(!memberInfo || !memberInfo->GetDefault<T>(val))
				return false;
			*prop = val;
			return true;
		}
		return false;
	});
	if(!res)
		return nullptr;
	matData.properties[key] = {prop, false, tex ? true : false};
	return &matData.properties[key];
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
		auto *shaderMat = GetShaderMaterial(static_cast<material::CMaterial &>(*matData.material));
		if(shaderMat) {
			udm::visit(propType, [this, &matData, &key, shaderMat](auto tag) {
				using T = typename decltype(tag)::type;
				if constexpr(material::is_property_type<T>) {
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

		if(valType == datasystem::ValueType::Texture) {
			if(matIdx < m_materialOverrides.size()) {
				auto &matInfo = m_materialOverrides[matIdx];
				if(matInfo.material) {
					UpdateMaterialOverride(static_cast<material::CMaterial &>(*matInfo.material));
					//UpdateMaterialOverride(matIdx, static_cast<material::CMaterial &>(*matInfo.material), true);
				}
			}
		}
	}
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

std::string CMaterialPropertyOverrideComponent::NormalizeTexturePath(const std::string &path) { return util::FilePath(pragma::asset::get_normalized_path(path, asset::Type::Texture)).GetString(); }

void CMaterialPropertyOverrideComponent::SetTextureProperty(uint32_t matIdx, const char *key, const std::string &tex) { SetMaterialProperty<std::string, true>(matIdx, key, NormalizeTexturePath(tex)); }
std::string CMaterialPropertyOverrideComponent::GetTextureProperty(uint32_t matIdx, const char *key) const
{
	std::string tex;
	if(!GetMaterialProperty(matIdx, key, tex))
		return {};
	return tex;
}

template<typename T>
bool CMaterialPropertyOverrideComponent::GetMaterialProperty(uint32_t matIdx, const char *key, T &outValue) const
{
	auto *propInfo = FindMaterialPropertyInfo(matIdx, key);
	if(!propInfo) {
		propInfo = const_cast<CMaterialPropertyOverrideComponent *>(this)->InitializeMaterialProperty(matIdx, key);
		if(!propInfo)
			return false;
	}
	outValue = propInfo->property->GetValue<T>();
	return true;
}

template<typename T, bool TEXTURE>
void CMaterialPropertyOverrideComponent::SetMaterialProperty(uint32_t matIdx, const char *key, const T &value)
{
	auto *propInfo = FindMaterialPropertyInfo(matIdx, key);
	if(!propInfo) {
		propInfo = InitializeMaterialProperty(matIdx, key);
		if(!propInfo)
			return;
	}
	*propInfo->property = value;
	if(!propInfo->enabled)
		return;

	// TODO: Re-create material if target material has changed
	//InitializeMaterialCopy(matIdx);

	// TODO
	//auto *mat = m_materialOverrides.front();

	// TODO: Create material copy
	// TODO: Apply property if enabled

	// TODO: Property to material(s)?
	ApplyMaterialProperty<T, TEXTURE>(matIdx, key, value);
}

template<typename T, bool TEXTURE>
void CMaterialPropertyOverrideComponent::ApplyMaterialProperty(uint32_t matIdx, const char *key, const T &value)
{
	auto *mat = GetRenderMaterial(matIdx);
	if(!mat) {
		//SetMaterialPropertyOverride({});
		mat = GetRenderMaterial(matIdx);
	}
	if(!mat)
		return;
	auto *shaderMat = GetShaderMaterial(*mat);
	if(!shaderMat)
		return;
	// TODO: Also apply to property override block?
	// TODO: How to handle texture overrides?
	if constexpr(TEXTURE) {
		mat->SetTexture(key, value);
		UpdateMaterialOverride(*mat);
	}
	else {
		mat->SetProperty(key, value);
		SetMaterialPropertyBufferValue(*mat, *shaderMat, key, value);
	}
}

void CMaterialPropertyOverrideComponent::ApplyMaterialProperty(uint32_t matIdx, const char *key)
{
	auto *propInfo = FindMaterialPropertyInfo(matIdx, key);
	if(!propInfo)
		return;
	if(!propInfo->enabled) {
		// Reset the material property
		if(matIdx < m_materialOverrides.size()) {
			auto &matInfo = m_materialOverrides[matIdx];
			if(matInfo.material) {
				auto *mat = GetTargetMaterial(matIdx);
				if(mat && matInfo.material->GetPropertyDataBlock()->HasValue(key)) {
					matInfo.material->ClearProperty(key);
					if(propInfo->texture)
						UpdateMaterialOverride(*matInfo.material);
					else {
						auto *shaderMat = GetShaderMaterial(static_cast<material::CMaterial &>(*matInfo.material));
						if(shaderMat) {
							udm::visit(propInfo->property->type, [this, mat, &matInfo, shaderMat, key](auto tag) {
								using T = typename decltype(tag)::type;
								if constexpr(material::is_property_type<T>) {
									T val;
									if(mat->GetProperty<T>(key, &val))
										SetMaterialPropertyBufferValue(*matInfo.material, *shaderMat, key, val);
								}
							});
						}
					}
				}
			}
		}
		return;
	}
	udm::visit(propInfo->property->type, [this, matIdx, key, &propInfo](auto tag) {
		using T = typename decltype(tag)::type;
		if constexpr(material::is_property_type<T>) {
			auto &val = propInfo->property->GetValue<T>();
			if(propInfo->texture) {
				if constexpr(std::is_same_v<T, udm::String>)
					ApplyMaterialProperty<T, true>(matIdx, key, val);
			}
			else
				ApplyMaterialProperty<T, false>(matIdx, key, val);
		}
	});
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
	string::to_lower(name);
	return name;
}
std::string CMaterialPropertyOverrideComponent::GetNormalizedMaterialName(const material::CMaterial &mat) { return GetNormalizedMaterialName(std::string {util::FilePath(const_cast<material::CMaterial &>(mat).GetName()).GetBack()}); }

const rendering::shader_material::ShaderMaterial *CMaterialPropertyOverrideComponent::GetShaderMaterial(const material::CMaterial &mat)
{
	auto *shader = dynamic_cast<ShaderGameWorldLightingPass *>(const_cast<material::CMaterial &>(mat).GetPrimaryShader());
	if(!shader)
		return nullptr;
	return shader->GetShaderMaterial();
}

void CMaterialPropertyOverrideComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CMaterialPropertyOverrideComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BindEvent(cModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		m_materialOverrides.clear();
		if(GetEntity().IsSpawned())
			PopulateProperties();
		return util::EventReply::Unhandled;
	});
	BindEvent(cMaterialOverrideComponent::EVENT_ON_MATERIAL_OVERRIDE_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &evOc = static_cast<CEOnMaterialOverrideChanged &>(evData.get());
		UpdateMaterialOverride(evOc.materialIndex, evOc.material);
		return util::EventReply::Unhandled;
	});

	GetEntity().AddComponent<CMaterialOverrideComponent>();
}

void CMaterialPropertyOverrideComponent::ReloadMaterialOverride(uint32_t matIdx)
{
	auto overrideC = GetEntity().GetComponent<CMaterialOverrideComponent>();
	if(overrideC.expired())
		return;
	auto *mat = GetTargetMaterial(matIdx);
	if(!mat)
		return;
	UpdateMaterialOverride(matIdx, static_cast<material::CMaterial &>(*mat), true);
}

void CMaterialPropertyOverrideComponent::UpdateMaterialOverride(uint32_t idx, const material::CMaterial &mat, bool forceInitialize)
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
	if(matInfo.shaderOverrideEnabled) {
		auto shaderOverride = get_cengine()->GetShader(matInfo.shaderOverride);
		if(shaderOverride.valid())
			static_cast<material::CMaterial &>(*newMat).SetPrimaryShader(*shaderOverride);
	}
	newMat->SetBaseMaterial(&const_cast<material::CMaterial &>(mat));
	matInfo.material = newMat;
	for(auto &[name, propInfo] : matInfo.properties) {
		if(!propInfo.enabled)
			continue;
		udm::visit(propInfo.property->type, [&newMat, &name, &propInfo](auto tag) {
			using T = typename decltype(tag)::type;
			if constexpr(material::is_property_type<T>) {
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

void CMaterialPropertyOverrideComponent::OnRemove()
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

void CMaterialPropertyOverrideComponent::UpdateMaterialOverride(material::Material &mat)
{
	auto *mdlC = static_cast<CModelComponent *>(GetEntity().GetModelComponent());
	if(mdlC)
		mdlC->SetRenderBufferListUpdateRequired();
	static_cast<material::CMaterial &>(mat).LoadTextures(false, true);
	mat.UpdateTextures(true); // Ensure all textures have been fully loaded
	if(mdlC)
		mdlC->UpdateRenderBufferList();
}
void CMaterialPropertyOverrideComponent::SetPropertyEnabled(const ComponentMemberInfo &info, CMaterialPropertyOverrideComponent &component, bool enabled)
{
	if(info.userIndex >= component.m_shaderMaterialPropertyInfos.size())
		return;
	auto &propInfo = component.m_shaderMaterialPropertyInfos[info.userIndex];
	if(propInfo.materialIndex >= component.m_materialOverrides.size())
		return;
	auto &matData = component.m_materialOverrides[propInfo.materialIndex];
	auto it = matData.properties.find(propInfo.name);
	if(it == matData.properties.end())
		return;
	it->second.enabled = enabled;
	component.ApplyMaterialProperty(propInfo.materialIndex, propInfo.name);
}
void CMaterialPropertyOverrideComponent::GetPropertyEnabled(const ComponentMemberInfo &info, CMaterialPropertyOverrideComponent &component, bool &outEnabled)
{
	if(info.userIndex >= component.m_shaderMaterialPropertyInfos.size()) {
		outEnabled = false;
		return;
	}
	auto &propInfo = component.m_shaderMaterialPropertyInfos[info.userIndex];
	if(propInfo.materialIndex >= component.m_materialOverrides.size()) {
		outEnabled = false;
		return;
	}
	auto &matData = component.m_materialOverrides[propInfo.materialIndex];
	auto it = matData.properties.find(propInfo.name);
	if(it == matData.properties.end()) {
		outEnabled = false;
		return;
	}
	outEnabled = it->second.enabled;
}

void CMaterialPropertyOverrideComponent::SetShader(const ComponentMemberInfo &info, CMaterialPropertyOverrideComponent &component, std::string shader)
{
	auto matIdx = info.userIndex;
	if(matIdx >= component.m_materialOverrides.size()) {
		component.ReloadMaterialOverride(matIdx);
		if(matIdx >= component.m_materialOverrides.size())
			return;
	}
	auto &matData = component.m_materialOverrides[matIdx];
	if(!matData.material)
		return;
	matData.shaderOverride = std::move(shader);
	component.UpdateShaderOverride(matIdx);
}
void CMaterialPropertyOverrideComponent::GetShader(const ComponentMemberInfo &info, CMaterialPropertyOverrideComponent &component, std::string &outShader)
{
	auto matIdx = info.userIndex;
	if(matIdx >= component.m_materialOverrides.size()) {
		outShader = {};
		return;
	}
	auto &matData = component.m_materialOverrides[matIdx];
	outShader = matData.shaderOverride;
}

void CMaterialPropertyOverrideComponent::SetShaderEnabled(const ComponentMemberInfo &info, CMaterialPropertyOverrideComponent &component, bool enabled)
{
	auto matIdx = info.userIndex;
	if(matIdx >= component.m_materialOverrides.size())
		return;
	auto &matData = component.m_materialOverrides[matIdx];
	matData.shaderOverrideEnabled = enabled;
	component.UpdateShaderOverride(matIdx);
}
void CMaterialPropertyOverrideComponent::GetShaderEnabled(const ComponentMemberInfo &info, CMaterialPropertyOverrideComponent &component, bool &outEnabled)
{
	auto matIdx = info.userIndex;
	if(matIdx >= component.m_materialOverrides.size()) {
		outEnabled = false;
		return;
	}
	auto &matData = component.m_materialOverrides[matIdx];
	outEnabled = matData.shaderOverrideEnabled;
}

void CMaterialPropertyOverrideComponent::UpdateShaderOverride(uint32_t matIdx)
{
	ReloadMaterialOverride(matIdx);
	ClearProperties(matIdx);
	auto *matTarget = GetTargetMaterial(matIdx);
	auto *matRender = GetRenderMaterial(matIdx);
	if(matTarget && matRender)
		PopulateProperties(std::string {util::FilePath(matTarget->GetName()).GetFileName()}, static_cast<material::CMaterial &>(*matRender), matIdx);
	OnMembersChanged();
}

void CMaterialPropertyOverrideComponent::AddMaterialTexturePropertyMember(const std::string &propPath, const rendering::shader_material::Texture &tex, std::optional<uint32_t> matIdx)
{
	material::Material *mat = nullptr;
	if(matIdx) {
		auto &mdl = GetEntity().GetModel();
		if(!mdl)
			return;
		auto &mats = mdl->GetMaterials();
		if(*matIdx >= mats.size())
			return;
		mat = mats[*matIdx].get();
	}

	std::string name {tex.name};
	auto propName = propPath + name;
	auto propNameEnabled = propName + "_enabled";
	auto *cPropName = register_global_string(name);
	auto *cPropNameEnabled = register_global_string(propNameEnabled);

	uint32_t idx;
	if(!m_freePropertyInfoIndices.empty()) {
		idx = m_freePropertyInfoIndices.front();
		m_freePropertyInfoIndices.pop();
	}
	else {
		idx = m_shaderMaterialPropertyInfos.size();
		m_shaderMaterialPropertyInfos.push_back({});
	}
	auto &info = m_shaderMaterialPropertyInfos[idx];
	info.name = cPropName;
	if(matIdx)
		info.materialIndex = *matIdx;

	auto memberInfo = ComponentMemberInfo::CreateDummy();
	memberInfo.SetName(propName);
	memberInfo.type = ents::EntityMemberType::String;
	memberInfo.userIndex = idx;
	memberInfo.AddTypeMetaData<ents::OptionalTypeMetaData>().enabledProperty = cPropNameEnabled;
	auto *texInfo = mat ? mat->GetTextureInfo(name) : nullptr;
	if(texInfo)
		memberInfo.SetDefault<std::string>(NormalizeTexturePath(texInfo->name));
	else if(tex.defaultTexturePath)
		memberInfo.SetDefault<std::string>(NormalizeTexturePath(*tex.defaultTexturePath));
	using TComponent = CMaterialPropertyOverrideComponent;
	using TValue = udm::String;
	memberInfo.SetGetterFunction<TComponent, TValue, +[](const ComponentMemberInfo &memberInfo, TComponent &component, TValue &outValue) {
		auto &propInfo = component.m_shaderMaterialPropertyInfos[memberInfo.userIndex];
		outValue = component.GetTextureProperty(propInfo.materialIndex, propInfo.name);
	}>();
	memberInfo.SetSetterFunction<TComponent, TValue, +[](const ComponentMemberInfo &memberInfo, TComponent &component, const TValue &value) {
		auto &propInfo = component.m_shaderMaterialPropertyInfos[memberInfo.userIndex];
		component.SetTextureProperty(propInfo.materialIndex, propInfo.name, value);
	}>();
	RegisterMember(std::move(memberInfo));

	auto memberInfoToggle = create_component_member_info<TComponent, bool, &CMaterialPropertyOverrideComponent::SetPropertyEnabled, &CMaterialPropertyOverrideComponent::GetPropertyEnabled>(cPropNameEnabled, false);
	memberInfoToggle.userIndex = idx;
	memberInfoToggle.AddTypeMetaData<ents::EnablerTypeMetaData>().targetProperty = cPropName;
	RegisterMember(std::move(memberInfoToggle));
}

void CMaterialPropertyOverrideComponent::AddMaterialPropertyMember(const rendering::Property &prop, const std::string &propPath, const std::string &name, std::optional<uint32_t> matIdx)
{
	material::Material *mat = nullptr;
	if(matIdx) {
		auto &mdl = GetEntity().GetModel();
		if(!mdl)
			return;
		auto &mats = mdl->GetMaterials();
		if(*matIdx >= mats.size())
			return;
		mat = mats[*matIdx].get();
	}

	auto propName = propPath + name;
	auto propNameEnabled = propName + "_enabled";

	auto *cPropNameEnabled = register_global_string(propNameEnabled);
	auto *cPropName = register_global_string(name);
	uint32_t idx;
	if(!m_freePropertyInfoIndices.empty()) {
		idx = m_freePropertyInfoIndices.front();
		m_freePropertyInfoIndices.pop();
	}
	else {
		idx = m_shaderMaterialPropertyInfos.size();
		m_shaderMaterialPropertyInfos.push_back({});
	}
	auto &info = m_shaderMaterialPropertyInfos[idx];
	info.name = cPropName;
	if(matIdx)
		info.materialIndex = *matIdx;

	auto memberInfo = ComponentMemberInfo::CreateDummy();
	memberInfo.SetName(propName);
	memberInfo.type = ents::udm_type_to_member_type(pragma::shadergraph::to_udm_type(prop->type));
	memberInfo.SetMin(prop->min);
	memberInfo.SetMax(prop->max);
	memberInfo.AddTypeMetaData<ents::OptionalTypeMetaData>().enabledProperty = cPropNameEnabled;
	memberInfo.userIndex = idx;
	using TComponent = CMaterialPropertyOverrideComponent;
	pragma::shadergraph::visit(prop->type, [mat, &prop, &memberInfo, &name](auto tag) {
		using T = typename decltype(tag)::type;
		T val;
		if(mat && mat->GetProperty<T>(name, &val))
			memberInfo.SetDefault(val);
		else if(prop->defaultValue.Get<T>(val))
			memberInfo.SetDefault(val);

		using TValue = T;
		memberInfo.SetGetterFunction<TComponent, TValue, +[](const ComponentMemberInfo &memberInfo, TComponent &component, TValue &outValue) {
			auto &propInfo = component.m_shaderMaterialPropertyInfos[memberInfo.userIndex];
			if(!component.GetMaterialProperty<TValue>(propInfo.materialIndex, propInfo.name, outValue))
				outValue = TValue {};
		}>();
		memberInfo.SetSetterFunction<TComponent, TValue, +[](const ComponentMemberInfo &memberInfo, TComponent &component, const TValue &value) {
			auto &propInfo = component.m_shaderMaterialPropertyInfos[memberInfo.userIndex];
			component.SetMaterialProperty(propInfo.materialIndex, propInfo.name, value);
		}>();
	});
	if(prop.specializationType && prop.specializationType == "color")
		memberInfo.SetSpecializationType(AttributeSpecializationType::Color);
	if(prop->enumSet) {
		auto nameToValue = prop->enumSet->getNameToValue();
		auto valueToName = prop->enumSet->getValueToName();
		std::vector<int64_t> values;
		values.reserve(nameToValue.size());
		for(auto &[name, value] : nameToValue)
			values.push_back(value);
		memberInfo.SetEnum(
		  [nameToValue = std::move(nameToValue)](const std::string &enumName) -> std::optional<int64_t> {
			  auto it = nameToValue.find(enumName);
			  return (it != nameToValue.end()) ? it->second : std::optional<int64_t> {};
		  },
		  [valueToName = std::move(valueToName)](int64_t enumValue) -> std::optional<std::string> {
			  auto it = valueToName.find(enumValue);
			  return (it != valueToName.end()) ? it->second : std::optional<std::string> {};
		  },
		  [values = std::move(values)]() -> std::vector<int64_t> { return values; });
	}
	RegisterMember(std::move(memberInfo));

	auto memberInfoToggle = create_component_member_info<TComponent, bool, &CMaterialPropertyOverrideComponent::SetPropertyEnabled, &CMaterialPropertyOverrideComponent::GetPropertyEnabled>(cPropNameEnabled, false);
	memberInfoToggle.userIndex = idx;
	memberInfoToggle.AddTypeMetaData<ents::EnablerTypeMetaData>().targetProperty = cPropName;
	RegisterMember(std::move(memberInfoToggle));
}

void CMaterialPropertyOverrideComponent::ClearProperties(uint32_t matIdx)
{
	if(matIdx >= m_materialOverrides.size())
		return;
	auto &matData = m_materialOverrides[matIdx];
	auto *matTarget = GetTargetMaterial(matIdx);
	if(!matTarget)
		return;
	auto matName = std::string {util::FilePath(matTarget->GetName()).GetFileName()};

	std::string matPropName = "material/" + matName + "/";
	RemoveMember(matPropName + "shader");
	RemoveMember(matPropName + "shader_enabled");

	std::string matPropPath = matPropName + "properties/";
	std::string matTexPath = matPropName + "textures/";
	size_t propIdx = 0;
	for(auto &propInfo : m_shaderMaterialPropertyInfos) {
		util::ScopeGuard sg {[&propIdx]() { ++propIdx; }};
		if(!propInfo.IsValid() || propInfo.materialIndex != matIdx)
			continue;

		RemoveMember(matTexPath + propInfo.name);
		RemoveMember(matTexPath + propInfo.name + "_enabled");
		RemoveMember(matPropPath + propInfo.name);
		RemoveMember(matPropPath + propInfo.name + "_enabled");

		propInfo.Invalidate();
		m_freePropertyInfoIndices.push(propIdx);
	}
	matData.properties.clear();
}

void CMaterialPropertyOverrideComponent::PopulateProperties(std::string matName, material::CMaterial &mat, uint32_t matIdx)
{
	auto *shaderMat = GetShaderMaterial(mat);
	if(!shaderMat)
		return;
	string::to_lower(matName);
	std::string matPropName = "material/" + matName + "/";

	auto &propertyMap = shaderMat->GetPropertyMap();
	auto &textures = shaderMat->textures;
	ReserveMembers(propertyMap.size() + textures.size() + 2); // +2 for shader override

	{
		auto shaderPropName = matPropName + "shader";
		auto shaderPropNameEnabled = shaderPropName + "_enabled";

		auto *cPropName = register_global_string(shaderPropName);
		auto *cPropNameEnabled = register_global_string(shaderPropNameEnabled);

		auto memberInfo = ComponentMemberInfo::CreateDummy();

		auto &metaData = memberInfo.AddMetaData();
		metaData["assetType"] = "shader";
		metaData["rootPath"] = util::Path::CreatePath(pragma::asset::get_asset_root_directory(asset::Type::ShaderGraph)).GetString();
		metaData["extensions"] = pragma::asset::get_supported_extensions(asset::Type::ShaderGraph, asset::FormatType::All);
		metaData["stripRootPath"] = true;
		metaData["stripExtension"] = true;

		memberInfo.SetSpecializationType(AttributeSpecializationType::File);
		memberInfo.SetName(shaderPropName);
		memberInfo.type = ents::EntityMemberType::String;
		memberInfo.userIndex = matIdx;
		memberInfo.AddTypeMetaData<ents::OptionalTypeMetaData>().enabledProperty = cPropNameEnabled;
		memberInfo.SetDefault<std::string>(mat.GetShaderIdentifier());
		using TComponent = CMaterialPropertyOverrideComponent;
		using TValue = udm::String;
		memberInfo.SetGetterFunction<TComponent, TValue, &CMaterialPropertyOverrideComponent::GetShader>();
		memberInfo.SetSetterFunction<TComponent, TValue, &CMaterialPropertyOverrideComponent::SetShader>();
		RegisterMember(std::move(memberInfo));

		auto memberInfoToggle = create_component_member_info<TComponent, bool, &CMaterialPropertyOverrideComponent::SetShaderEnabled, &CMaterialPropertyOverrideComponent::GetShaderEnabled>(cPropNameEnabled, false);
		memberInfoToggle.userIndex = matIdx;
		memberInfoToggle.AddTypeMetaData<ents::EnablerTypeMetaData>().targetProperty = cPropName;
		RegisterMember(std::move(memberInfoToggle));
	}

	std::unordered_set<std::string> sharedProps;

	for(auto &[name, index] : propertyMap) {
		if(index >= shaderMat->properties.size())
			continue;
		auto &prop = shaderMat->properties[index];
		if(math::is_flag_set(prop.propertyFlags, rendering::Property::Flags::HideInEditor))
			continue;

		/*auto it = sharedProps.find(name);
		if(it == sharedProps.end()) {
			sharedProps.insert(name);
			AddMaterialPropertyMember(prop, "properties/", name);
		}*/

		AddMaterialPropertyMember(prop, matPropName + "properties/", name, matIdx);
	}

	for(auto &tex : textures) {
		auto &name = tex.name;
		/*auto it = sharedProps.find(name);
		if(it == sharedProps.end()) {
			sharedProps.insert(name);
			AddMaterialTexturePropertyMember("textures/", tex);
		}*/

		AddMaterialTexturePropertyMember(matPropName + "textures/", tex, matIdx);
	}
}

void CMaterialPropertyOverrideComponent::PopulateProperties()
{
	ClearMembers();
	m_shaderMaterialPropertyInfos.clear();
	m_freePropertyInfoIndices = {};
	util::ScopeGuard sg {[this]() { OnMembersChanged(); }};

	auto &mdl = GetEntity().GetModel();
	if(!mdl)
		return;

	auto &mats = mdl->GetMaterials();
	for(size_t matIdx = 0; matIdx < mats.size(); ++matIdx) {
		auto &mat = mats[matIdx];
		if(!mat)
			continue;
		PopulateProperties(std::string {util::FilePath(mat->GetName()).GetFileName()}, static_cast<material::CMaterial &>(*mat), matIdx);
	}
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
void CMaterialPropertyOverrideComponent::SetMaterialPropertyBufferValue(material::Material &mat, const rendering::shader_material::ShaderMaterial &shaderMat, const char *keyName, const T &newVal)
{
	auto &descriptor = shaderMat;
	auto *buf = static_cast<material::CMaterial &>(mat).GetSettingsBuffer();
	if(!buf)
		return;
	auto range = descriptor.GetPropertyDataRange(keyName);
	if(!range)
		return;
	auto [startOffset, size] = *range;
	assert(sizeof(T) == size);
	BufferUpdateInfo updateInfo {};
	updateInfo.buffer = buf->shared_from_this();
	updateInfo.startOffset = startOffset;
	updateInfo.size = size;
	updateInfo.data = std::make_unique<std::byte[]>(updateInfo.size);
	memcpy(updateInfo.data.get(), &newVal, sizeof(newVal));
	m_bufferUpdateQueue.emplace(std::move(updateInfo));
}

void CMaterialPropertyOverrideComponent::ApplyMaterialPropertyOverride(material::Material &mat, const rendering::MaterialPropertyBlock &matPropOverride)
{
	for(auto &pair : udm::LinkedPropertyWrapper {matPropOverride.GetPropertyBlock()}.ElIt()) {
		udm::visit(pair.property->type, [&mat, &pair](auto tag) {
			using T = typename decltype(tag)::type;
			if constexpr(material::is_property_type<T>) {
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

void CMaterialPropertyOverrideComponent::RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts)
{
	auto def = pragma::LuaCore::create_entity_component_class<CMaterialPropertyOverrideComponent, BaseEntityComponent>("MaterialPropertyOverrideComponent");
	def.def("SetMaterialProperty", +[](CMaterialPropertyOverrideComponent &component, uint32_t matIdx, const char *key, const Vector3 &value) { component.SetMaterialProperty(matIdx, key, value); });
	def.def("ClearMaterialProperty", +[](CMaterialPropertyOverrideComponent &component, uint32_t matIdx, const char *key) { component.ClearMaterialProperty(matIdx, key); });
	def.def("SetTextureProperty", &CMaterialPropertyOverrideComponent::SetTextureProperty);
	def.def("GetTextureProperty", &CMaterialPropertyOverrideComponent::GetTextureProperty);
	//template<typename T>
	//void CMaterialPropertyOverrideComponent::SetMaterialProperty(const char *materialName, const char *key, const T &value)

	//def.def("SetMaterialOverride", static_cast<void (pragma::CMaterialPropertyOverrideComponent::*)(uint32_t, const std::string &)>(&pragma::CMaterialPropertyOverrideComponent::SetMaterialOverride));
	//def.def("SetMaterialOverride", static_cast<void (pragma::CMaterialPropertyOverrideComponent::*)(uint32_t, material::CMaterial &)>(&pragma::CMaterialPropertyOverrideComponent::SetMaterialOverride));
	/*def.def(
	  "SetMaterialOverride", +[](lua::State *l, pragma::CMaterialPropertyOverrideComponent &hModel, const std::string &matSrc, const std::string &matDst) {
		  auto &mdl = hModel.GetEntity().GetModel();
		  if(!mdl)
			  return;
		  auto &mats = mdl->GetMaterials();
		  auto it = std::find_if(mats.begin(), mats.end(), [&matSrc](const material::MaterialHandle &hMat) {
			  if(!hMat)
				  return false;
			  return pragma::asset::matches(hMat.get()->GetName(), matSrc, pragma::asset::Type::Material);
		  });
		  if(it == mats.end())
			  return;
		  hModel.SetMaterialOverride(it - mats.begin(), matDst);
	  });
	def.def(
	  "SetMaterialOverride", +[](lua::State *l, pragma::CMaterialPropertyOverrideComponent &hModel, const std::string &matSrc, material::CMaterial &matDst) {
		  auto &mdl = hModel.GetEntity().GetModel();
		  if(!mdl)
			  return;
		  auto &mats = mdl->GetMaterials();
		  auto it = std::find_if(mats.begin(), mats.end(), [&matSrc](const material::MaterialHandle &hMat) {
			  if(!hMat)
				  return false;
			  return pragma::asset::matches(hMat.get()->GetName(), matSrc, pragma::asset::Type::Material);
		  });
		  if(it == mats.end())
			  return;
		  hModel.SetMaterialOverride(it - mats.begin(), matDst);
	  });*/
	/*def.def(
	  "ClearMaterialOverride", +[](lua::State *l, pragma::CMaterialPropertyOverrideComponent &hModel, const std::string &matSrc) {
		  auto &mdl = hModel.GetEntity().GetModel();
		  if(!mdl)
			  return;
		  auto &mats = mdl->GetMaterials();
		  auto it = std::find_if(mats.begin(), mats.end(), [&matSrc](const material::MaterialHandle &hMat) {
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
