// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.util_pbr_converter;
import :client_state;
import :engine;
import :game;
import :rendering.shaders;

using namespace pragma;

void CPBRConverterComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

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

void CPBRConverterComponent::ConvertMaterialsToPBR(asset::Model &mdl)
{
	for(auto hMat : mdl.GetMaterials()) {
		if(!hMat)
			continue;
		auto &mat = static_cast<material::CMaterial &>(*hMat.get());
		if(ShouldConvertMaterial(mat) == false)
			continue;
		ConvertToPBR(static_cast<material::CMaterial &>(*hMat.get()));
	}
}

void CPBRConverterComponent::GenerateAmbientOcclusionMaps(asset::Model &mdl, uint32_t w, uint32_t h, uint32_t samples, bool rebuild) { ScheduleModelUpdate(mdl, false, AmbientOcclusionInfo {w, h, samples, rebuild}); }

void CPBRConverterComponent::GenerateAmbientOcclusionMaps(ecs::BaseEntity &ent, uint32_t w, uint32_t h, uint32_t samples, bool rebuild)
{
	auto mdl = ent.GetModel();
	if(mdl == nullptr)
		return;
	ScheduleModelUpdate(*mdl, false, AmbientOcclusionInfo {w, h, samples, rebuild}, &ent);
}

void CPBRConverterComponent::UpdateModel(asset::Model &mdl, ModelUpdateInfo &updateInfo, ecs::BaseEntity *optEnt)
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

void CPBRConverterComponent::ScheduleModelUpdate(asset::Model &mdl, bool updateMetalness, std::optional<AmbientOcclusionInfo> updateAOInfo, ecs::BaseEntity *optEnt)
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
	auto cb = mdl.CallOnMaterialsLoaded([this, &mdl, &updateInfo, hEnt]() { UpdateModel(mdl, updateInfo, const_cast<ecs::BaseEntity *>(hEnt.get())); });
	if(cb.IsValid())
		updateInfo.cbOnMaterialsLoaded = cb;
}

void CPBRConverterComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();

	auto *client = get_client_state();
	m_cbOnModelLoaded = get_cgame()->AddCallback("OnModelLoaded", FunctionCallback<void, std::reference_wrapper<std::shared_ptr<asset::Model>>>::Create([this](std::reference_wrapper<std::shared_ptr<asset::Model>> mdl) { ScheduleModelUpdate(*mdl.get(), true); }));
	m_cbOnMaterialLoaded = client->AddCallback("OnMaterialLoaded", FunctionCallback<void, material::CMaterial *>::Create([this](material::CMaterial *mat) {
		if(ShouldConvertMaterial(*mat) == false)
			return;
		ConvertToPBR(*mat);
	}));

	auto &matManager = client->GetMaterialManager();
	for(auto &pair : matManager.GetCache()) {
		auto asset = matManager.GetAsset(pair.second);
		if(!asset)
			continue;
		auto hMat = material::CMaterialManager::GetAssetObject(*asset);
		if(!hMat || hMat.get()->IsLoaded() == false || ShouldConvertMaterial(static_cast<material::CMaterial &>(*hMat.get())) == false)
			continue;
		ConvertToPBR(static_cast<material::CMaterial &>(*hMat.get()));
	}

	auto &mdlManager = client->GetModelManager();
	auto &models = mdlManager.GetCache();
	for(auto &pair : models) {
		auto idx = pair.second;
		auto asset = mdlManager.GetAsset(idx);
		if(!asset)
			continue;
		UpdateMetalness(*asset::ModelManager::GetAssetObject(*asset));
		//UpdateAmbientOcclusion(*mdl);
	}
}
bool CPBRConverterComponent::ShouldConvertMaterial(material::CMaterial &mat) const
{
	if(m_convertedMaterials.find(mat.GetName()) != m_convertedMaterials.end() || IsPBR(mat) == false)
		return false;
	return mat.GetTextureInfo(material::ematerial::RMA_MAP_IDENTIFIER) == nullptr;
}

bool CPBRConverterComponent::IsPBR(material::CMaterial &mat) const
{
	auto shader = mat.GetShaderIdentifier();
	string::to_lower(shader);
	return shader == "pbr" || shader == "pbr_blend";
}

void CPBRConverterComponent::PollEvents() { ProcessQueue(); }

void CPBRConverterComponent::ApplyMiscMaterialProperties(material::CMaterial &mat, const physics::SurfaceMaterial &surfMat, const std::string &surfMatName)
{
	auto ior = surfMat.GetIOR();
	if(ior.has_value() == false)
		return;
	mat.SetProperty("ior", *ior);
	if(surfMatName.find("glass") == std::string::npos)
		return;
	if(mat.GetPropertyType("cycles/shader") == material::PropertyType::None)
		mat.SetProperty<std::string>("cycles/shader", "glass");
}

bool CPBRConverterComponent::ConvertToPBR(material::CMaterial &matTraditional)
{
	if(!matTraditional.HasPropertyBlock("rma_info"))
		return false;
	auto resWatcherLock = get_cengine()->ScopeLockResourceWatchers();
	Con::COUT << "Converting material '" << matTraditional.GetName() << "' to PBR..." << Con::endl;
	m_convertedMaterials.insert(matTraditional.GetName());
	auto &setupCmd = get_cengine()->GetSetupCommandBuffer();

	auto matName = matTraditional.GetName();
	ufile::remove_extension_from_filename(matName);

	std::string surfMatName;
	auto hasSurfaceMaterial = matTraditional.GetProperty("surfacematerial", &surfMatName);
	auto *surfMat = hasSurfaceMaterial ? get_cgame()->GetSurfaceMaterial(surfMatName) : nullptr;

	auto fSetMaterialValue = [&matTraditional]<typename T>(material::CMaterial &mat, const std::string &path, const T &value) {
		if(mat.GetPropertyType(path) == material::PropertyType::None)
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

	auto *shaderComposeRMA = static_cast<ShaderComposeRMA *>(get_cengine()->GetShader("compose_rma").get());
	if(shaderComposeRMA == nullptr)
		return false;

	auto fGetTexture = [&matTraditional](const std::string &name) -> prosper::Texture * {
		auto *map = matTraditional.GetTextureInfo(name);
		if(map && map->texture && std::static_pointer_cast<material::Texture>(map->texture)->HasValidVkTexture())
			return std::static_pointer_cast<material::Texture>(map->texture)->GetVkTexture().get();
		return nullptr;
	};

	prosper::Texture *roughnessMap = fGetTexture("roughness_map");
	prosper::Texture *metalnessMap = fGetTexture("metalness_map");
	prosper::Texture *aoMap = fGetTexture("ao_map");

	auto flags = ShaderComposeRMA::Flags::None;
	if(roughnessMap == nullptr) {
		roughnessMap = fGetTexture("specular_map");
		if(roughnessMap)
			flags |= ShaderComposeRMA::Flags::UseSpecularWorkflow;
	}

	std::string rmaMapName = "";
	if(roughnessMap || metalnessMap || aoMap) {
		auto rmaMap = shaderComposeRMA->ComposeRMA(get_cengine()->GetRenderContext(), roughnessMap, metalnessMap, aoMap, flags);

		rmaMapName = matName + "_rma";
		image::TextureInfo imgWriteInfo {};
		imgWriteInfo.alphaMode = image::TextureInfo::AlphaMode::None;
		imgWriteInfo.containerFormat = image::TextureInfo::ContainerFormat::DDS;
		imgWriteInfo.flags = image::TextureInfo::Flags::GenerateMipmaps;
		imgWriteInfo.inputFormat = image::TextureInfo::InputFormat::R8G8B8A8_UInt;
		imgWriteInfo.outputFormat = image::TextureInfo::OutputFormat::ColorMap;
		get_cgame()->SaveImage(*rmaMap, "addons/converted/materials/" + rmaMapName, imgWriteInfo);

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
		if(matTraditional.GetPropertyType("metalness_factor") == material::PropertyType::None)
			matTraditional.SetProperty("metalness_factor", 0.f);
		if(matTraditional.GetPropertyType("roughness_factor") == material::PropertyType::None)
			matTraditional.SetProperty("roughness_factor", 0.5f);
	}
	matTraditional.SetTextureProperty(material::ematerial::RMA_MAP_IDENTIFIER, rmaMapName);

	// Note: If no surface material could be found in the material,
	// the model's surface material will be checked as well in 'GenerateGeometryBasedTextures'.
	//

	matTraditional.UpdateTextures();

	// Overwrite old material with new PBR settings
	std::string err;
	auto savePath = pragma::asset::relative_path_to_absolute_path(matTraditional.GetName(), asset::Type::Material, util::CONVERT_PATH);
	auto *client = get_client_state();
	if(matTraditional.Save(savePath.GetString(), err, true))
		client->LoadMaterial(matName, nullptr, true, true); // Reload material immediately
	static_cast<material::CMaterialManager &>(client->GetMaterialManager()).GetTextureManager().ClearUnused();
	// Con::COUT<<"Conversion complete!"<<Con::endl;
	return true;
}
std::shared_ptr<prosper::Texture> CPBRConverterComponent::ConvertSpecularMapToRoughness(prosper::Texture &specularMap)
{
	auto *shaderSpecularToRoughness = static_cast<ShaderSpecularToRoughness *>(get_cengine()->GetShader("specular_to_roughness").get());
	if(shaderSpecularToRoughness == nullptr)
		return nullptr;
	auto &setupCmd = get_cengine()->GetSetupCommandBuffer();
	// Specular descriptor set
	auto dsgSpecular = get_cengine()->GetRenderContext().CreateDescriptorSetGroup(shaderSpecularToRoughness::get_descriptor_set_texture());
	dsgSpecular->GetDescriptorSet()->SetBindingTexture(specularMap, 0u);

	// Initialize roughness image
	auto createInfoRoughness = specularMap.GetImage().GetCreateInfo();
	createInfoRoughness.format = prosper::Format::R8G8B8A8_UNorm;
	createInfoRoughness.postCreateLayout = prosper::ImageLayout::ColorAttachmentOptimal;
	createInfoRoughness.usage = prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::ColorAttachmentBit;
	auto roughnessMap = get_cengine()->GetRenderContext().CreateImage(createInfoRoughness);
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	auto roughnessTex = get_cengine()->GetRenderContext().CreateTexture({}, *roughnessMap, imgViewCreateInfo, samplerCreateInfo);
	auto roughnessRt = get_cengine()->GetRenderContext().CreateRenderTarget({roughnessTex}, shaderSpecularToRoughness->GetRenderPass());

	// Specular to roughness
	if(setupCmd->RecordBeginRenderPass(*roughnessRt) == true) {
		prosper::ShaderBindState bindState {*setupCmd};
		if(shaderSpecularToRoughness->RecordBeginDraw(bindState) == true) {
			shaderSpecularToRoughness->RecordDraw(bindState, *dsgSpecular->GetDescriptorSet());
			shaderSpecularToRoughness->RecordEndDraw(bindState);
		}
		setupCmd->RecordEndRenderPass();
	}
	get_cengine()->FlushSetupCommandBuffer();
	return roughnessTex;
}

////////

void CUtilPBRConverter::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CPBRConverterComponent>();
}
