/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/math/intersection.h>
#include <pragma/physics/collisionmesh.h>
#include <pragma/asset/util_asset.hpp>
#include <pragma/game/game_resources.hpp>
#include <sharedutils/util_file.h>
#include <image/prosper_image.hpp>
#include <image/prosper_render_target.hpp>
#include <image/prosper_sampler.hpp>
#include <prosper_util.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <sharedutils/util_library.hpp>
#include "pragma/entities/c_entityfactories.h"
#include "pragma/entities/util/c_util_pbr_converter.hpp"
#include "pragma/rendering/raytracing/cycles.hpp"
#include "pragma/rendering/shaders/util/c_shader_specular_to_roughness.hpp"
#include "pragma/rendering/shaders/util/c_shader_extract_diffuse_ambient_occlusion.hpp"
#include "pragma/rendering/shaders/util/c_shader_compose_rma.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/components/logic_component.hpp>
#include <util_texture_info.hpp>
#include <pragma/model/modelmanager.h>
#include <datasystem_t.hpp>
#include <cmaterialmanager.h>
#include <cmaterial_manager2.hpp>
#include <cmaterial.h>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

using namespace pragma;

LINK_ENTITY_TO_CLASS(util_pbr_converter, CUtilPBRConverter);

void CPBRConverterComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CPBRConverterComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	auto &ent = GetEntity();
	ent.AddComponent<LogicComponent>();
}

void CPBRConverterComponent::OnTick(double dt)
{
	PollEvents();
	if(m_workQueue.empty())
		SetTickPolicy(TickPolicy::Never);
}

void CPBRConverterComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	while(m_workQueue.empty() == false) {
		auto &item = m_workQueue.front();
		item.job.Cancel();
		item.job.Wait();
		m_workQueue.pop();
	}
	PollEvents();

	if(m_cbOnModelLoaded.IsValid())
		m_cbOnModelLoaded.Remove();
	if(m_cbOnMaterialLoaded.IsValid())
		m_cbOnMaterialLoaded.Remove();
	for(auto &pair : m_scheduledModelUpdates) {
		if(pair.second.cbOnMaterialsLoaded.IsValid())
			pair.second.cbOnMaterialsLoaded.Remove();
	}
}

void CPBRConverterComponent::ConvertMaterialsToPBR(Model &mdl)
{
	for(auto hMat : mdl.GetMaterials()) {
		if(!hMat)
			continue;
		auto &mat = static_cast<CMaterial &>(*hMat.get());
		if(ShouldConvertMaterial(mat) == false)
			continue;
		ConvertToPBR(static_cast<CMaterial &>(*hMat.get()));
	}
}

void CPBRConverterComponent::GenerateAmbientOcclusionMaps(Model &mdl, uint32_t w, uint32_t h, uint32_t samples, bool rebuild) { ScheduleModelUpdate(mdl, false, AmbientOcclusionInfo {w, h, samples, rebuild}); }

void CPBRConverterComponent::GenerateAmbientOcclusionMaps(BaseEntity &ent, uint32_t w, uint32_t h, uint32_t samples, bool rebuild)
{
	auto mdl = ent.GetModel();
	if(mdl == nullptr)
		return;
	ScheduleModelUpdate(*mdl, false, AmbientOcclusionInfo {w, h, samples, rebuild}, &ent);
}

void CPBRConverterComponent::UpdateModel(Model &mdl, ModelUpdateInfo &updateInfo, BaseEntity *optEnt)
{
	if(updateInfo.updateMetalness)
		UpdateMetalness(mdl);
	if(updateInfo.updateAmbientOcclusion.has_value())
		UpdateAmbientOcclusion(mdl, *updateInfo.updateAmbientOcclusion, optEnt);
	if(updateInfo.cbOnMaterialsLoaded.IsValid())
		updateInfo.cbOnMaterialsLoaded.Remove();
	auto it = m_scheduledModelUpdates.find(&mdl);
	if(it != m_scheduledModelUpdates.end())
		m_scheduledModelUpdates.erase(it);
}

void CPBRConverterComponent::ScheduleModelUpdate(Model &mdl, bool updateMetalness, std::optional<AmbientOcclusionInfo> updateAOInfo, BaseEntity *optEnt)
{
	auto itUpdateInfo = m_scheduledModelUpdates.find(&mdl);
	if(itUpdateInfo == m_scheduledModelUpdates.end())
		itUpdateInfo = m_scheduledModelUpdates.insert(std::make_pair(&mdl, ModelUpdateInfo {})).first;
	auto &updateInfo = itUpdateInfo->second;
	if(updateMetalness)
		updateInfo.updateMetalness = true;
	if(updateAOInfo.has_value())
		updateInfo.updateAmbientOcclusion = *updateAOInfo;
	auto hEnt = optEnt ? optEnt->GetHandle() : EntityHandle {};
	auto cb = mdl.CallOnMaterialsLoaded([this, &mdl, &updateInfo, hEnt]() { UpdateModel(mdl, updateInfo, const_cast<BaseEntity *>(hEnt.get())); });
	if(cb.IsValid())
		updateInfo.cbOnMaterialsLoaded = cb;
}

void CPBRConverterComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();

	m_cbOnModelLoaded = c_game->AddCallback("OnModelLoaded", FunctionCallback<void, std::reference_wrapper<std::shared_ptr<Model>>>::Create([this](std::reference_wrapper<std::shared_ptr<Model>> mdl) { ScheduleModelUpdate(*mdl.get(), true); }));
	m_cbOnMaterialLoaded = client->AddCallback("OnMaterialLoaded", FunctionCallback<void, CMaterial *>::Create([this](CMaterial *mat) {
		if(ShouldConvertMaterial(*mat) == false)
			return;
		ConvertToPBR(*mat);
	}));

	auto &matManager = client->GetMaterialManager();
	for(auto &pair : matManager.GetCache()) {
		auto asset = matManager.GetAsset(pair.second);
		if(!asset)
			continue;
		auto hMat = msys::CMaterialManager::GetAssetObject(*asset);
		if(!hMat || hMat.get()->IsLoaded() == false || ShouldConvertMaterial(static_cast<CMaterial &>(*hMat.get())) == false)
			continue;
		ConvertToPBR(static_cast<CMaterial &>(*hMat.get()));
	}

	auto &mdlManager = client->GetModelManager();
	auto &models = mdlManager.GetCache();
	for(auto &pair : models) {
		auto idx = pair.second;
		auto asset = mdlManager.GetAsset(idx);
		if(!asset)
			continue;
		UpdateMetalness(*pragma::asset::ModelManager::GetAssetObject(*asset));
		//UpdateAmbientOcclusion(*mdl);
	}
}
bool CPBRConverterComponent::ShouldConvertMaterial(CMaterial &mat) const
{
	if(m_convertedMaterials.find(mat.GetName()) != m_convertedMaterials.end() || IsPBR(mat) == false)
		return false;
	return mat.GetTextureInfo(Material::RMA_MAP_IDENTIFIER) == nullptr;
}

bool CPBRConverterComponent::IsPBR(CMaterial &mat) const
{
	auto shader = mat.GetShaderIdentifier();
	ustring::to_lower(shader);
	return shader == "pbr" || shader == "pbr_blend";
}

void CPBRConverterComponent::PollEvents() { ProcessQueue(); }

void CPBRConverterComponent::ApplyMiscMaterialProperties(CMaterial &mat, const SurfaceMaterial &surfMat, const std::string &surfMatName)
{
	auto ior = surfMat.GetIOR();
	if(ior.has_value() == false)
		return;
	mat.SetProperty("ior", *ior);
	if(surfMatName.find("glass") == std::string::npos)
		return;
	if(mat.GetPropertyType("cycles/shader") == msys::PropertyType::None)
		mat.SetProperty<std::string>("cycles/shader", "glass");
}

bool CPBRConverterComponent::ConvertToPBR(CMaterial &matTraditional)
{
	if(!matTraditional.HasPropertyBlock("rma_info"))
		return false;
	auto resWatcherLock = c_engine->ScopeLockResourceWatchers();
	Con::cout << "Converting material '" << matTraditional.GetName() << "' to PBR..." << Con::endl;
	m_convertedMaterials.insert(matTraditional.GetName());
	auto &setupCmd = c_engine->GetSetupCommandBuffer();

	auto matName = matTraditional.GetName();
	ufile::remove_extension_from_filename(matName);

	std::string surfMatName;
	auto hasSurfaceMaterial = matTraditional.GetProperty("surfacematerial", &surfMatName);
	auto *surfMat = hasSurfaceMaterial ? c_game->GetSurfaceMaterial(surfMatName) : nullptr;

	auto fSetMaterialValue = [&matTraditional]<typename T>(CMaterial &mat, const std::string &path, const T &value) {
		if(mat.GetPropertyType(path) == msys::PropertyType::None)
			return;
		mat.SetProperty(path, value);
	};

	auto *pbrInfo = surfMat ? &surfMat->GetPBRInfo() : nullptr;
	if(matTraditional.GetProperty("rma_info/requires_sss_update", false) && pbrInfo && pbrInfo->subsurface.factor != 0.f) {
		if(matTraditional.HasPropertyBlock("subsurface_scattering") == false) {
			fSetMaterialValue(matTraditional, "subsurface_scattering/factor", pbrInfo->subsurface.factor);
			fSetMaterialValue(matTraditional, "subsurface_scattering/method", 0);
			fSetMaterialValue(matTraditional, "subsurface_scattering/scatter_color", pbrInfo->subsurface.scatterColor);

			matTraditional.ClearProperty("rma_info/requires_sss_update");
		}
	}

	auto *shaderComposeRMA = static_cast<pragma::ShaderComposeRMA *>(c_engine->GetShader("compose_rma").get());
	if(shaderComposeRMA == nullptr)
		return false;

	auto fGetTexture = [&matTraditional](const std::string &name) -> prosper::Texture * {
		auto *map = matTraditional.GetTextureInfo(name);
		if(map && map->texture && std::static_pointer_cast<Texture>(map->texture)->HasValidVkTexture())
			return std::static_pointer_cast<Texture>(map->texture)->GetVkTexture().get();
		return nullptr;
	};

	prosper::Texture *roughnessMap = fGetTexture("roughness_map");
	prosper::Texture *metalnessMap = fGetTexture("metalness_map");
	prosper::Texture *aoMap = fGetTexture("ao_map");

	auto flags = pragma::ShaderComposeRMA::Flags::None;
	if(roughnessMap == nullptr) {
		roughnessMap = fGetTexture("specular_map");
		if(roughnessMap)
			flags |= pragma::ShaderComposeRMA::Flags::UseSpecularWorkflow;
	}

	std::string rmaMapName = "";
	if(roughnessMap || metalnessMap || aoMap) {
		auto rmaMap = shaderComposeRMA->ComposeRMA(c_engine->GetRenderContext(), roughnessMap, metalnessMap, aoMap, flags);

		rmaMapName = matName + "_rma";
		uimg::TextureInfo imgWriteInfo {};
		imgWriteInfo.alphaMode = uimg::TextureInfo::AlphaMode::None;
		imgWriteInfo.containerFormat = uimg::TextureInfo::ContainerFormat::DDS;
		imgWriteInfo.flags = uimg::TextureInfo::Flags::GenerateMipmaps;
		imgWriteInfo.inputFormat = uimg::TextureInfo::InputFormat::R8G8B8A8_UInt;
		imgWriteInfo.outputFormat = uimg::TextureInfo::OutputFormat::ColorMap;
		c_game->SaveImage(*rmaMap, "addons/converted/materials/" + rmaMapName, imgWriteInfo);

		if(metalnessMap)
			matTraditional.ClearProperty("rma_info/requires_metalness_update");
		if(aoMap)
			matTraditional.ClearProperty("rma_info/requires_ao_update");
	}

	if(surfMat)
		ApplyMiscMaterialProperties(matTraditional, *surfMat, surfMatName);

	float tmp;
	if(matTraditional.GetProperty("roughness_factor", &tmp))
		matTraditional.ClearProperty("requires_roughness_update");
	else if(roughnessMap == nullptr && pbrInfo) {
		matTraditional.SetProperty("roughness_factor", pbrInfo->roughness);
		matTraditional.ClearProperty("rma_info/requires_roughness_update");
	}

	if(matTraditional.GetProperty("metalness_factor", &tmp))
		matTraditional.ClearProperty("requires_metalness_update");
	else if(metalnessMap == nullptr && pbrInfo) {
		matTraditional.SetProperty("metalness_factor", pbrInfo->metalness);
		matTraditional.ClearProperty("rma_info/requires_metalness_update");
	}

	if(aoMap == nullptr)
		matTraditional.SetProperty("rma_info/requires_ao_update", true);
	else
		matTraditional.ClearProperty("requires_ao_update");

	if(rmaMapName.empty()) {
		rmaMapName = "pbr/rma_neutral";
		if(matTraditional.GetPropertyType("metalness_factor") == msys::PropertyType::None)
			matTraditional.SetProperty("metalness_factor", 0.f);
		if(matTraditional.GetPropertyType("roughness_factor") == msys::PropertyType::None)
			matTraditional.SetProperty("roughness_factor", 0.5f);
	}
	matTraditional.SetTextureProperty(Material::RMA_MAP_IDENTIFIER, rmaMapName);

	// Note: If no surface material could be found in the material,
	// the model's surface material will be checked as well in 'GenerateGeometryBasedTextures'.
	//

	matTraditional.UpdateTextures();

	// Overwrite old material with new PBR settings
	std::string err;
	auto savePath = pragma::asset::relative_path_to_absolute_path(matTraditional.GetName(), pragma::asset::Type::Material, util::CONVERT_PATH);
	if(matTraditional.Save(savePath.GetString(), err, true))
		client->LoadMaterial(matName, nullptr, true, true); // Reload material immediately
	static_cast<msys::CMaterialManager &>(client->GetMaterialManager()).GetTextureManager().ClearUnused();
	// Con::cout<<"Conversion complete!"<<Con::endl;
	return true;
}
std::shared_ptr<prosper::Texture> CPBRConverterComponent::ConvertSpecularMapToRoughness(prosper::Texture &specularMap)
{
	auto *shaderSpecularToRoughness = static_cast<pragma::ShaderSpecularToRoughness *>(c_engine->GetShader("specular_to_roughness").get());
	if(shaderSpecularToRoughness == nullptr)
		return nullptr;
	auto &setupCmd = c_engine->GetSetupCommandBuffer();
	// Specular descriptor set
	auto dsgSpecular = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderSpecularToRoughness::DESCRIPTOR_SET_TEXTURE);
	dsgSpecular->GetDescriptorSet()->SetBindingTexture(specularMap, 0u);

	// Initialize roughness image
	auto createInfoRoughness = specularMap.GetImage().GetCreateInfo();
	createInfoRoughness.format = prosper::Format::R8G8B8A8_UNorm;
	createInfoRoughness.postCreateLayout = prosper::ImageLayout::ColorAttachmentOptimal;
	createInfoRoughness.usage = prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::ColorAttachmentBit;
	auto roughnessMap = c_engine->GetRenderContext().CreateImage(createInfoRoughness);
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	auto roughnessTex = c_engine->GetRenderContext().CreateTexture({}, *roughnessMap, imgViewCreateInfo, samplerCreateInfo);
	auto roughnessRt = c_engine->GetRenderContext().CreateRenderTarget({roughnessTex}, shaderSpecularToRoughness->GetRenderPass());

	// Specular to roughness
	if(setupCmd->RecordBeginRenderPass(*roughnessRt) == true) {
		prosper::ShaderBindState bindState {*setupCmd};
		if(shaderSpecularToRoughness->RecordBeginDraw(bindState) == true) {
			shaderSpecularToRoughness->RecordDraw(bindState, *dsgSpecular->GetDescriptorSet());
			shaderSpecularToRoughness->RecordEndDraw(bindState);
		}
		setupCmd->RecordEndRenderPass();
	}
	c_engine->FlushSetupCommandBuffer();
	return roughnessTex;
}

////////

void CUtilPBRConverter::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CPBRConverterComponent>();
}
