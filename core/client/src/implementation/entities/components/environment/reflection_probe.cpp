// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/console/helper.hpp"

module pragma.client;

import :entities.components.env_reflection_probe;
import :client_state;
import :engine;
import :entities.components.transform;
import :game;
import :gui;
import :math;
import :rendering.shaders;

using namespace pragma;

rendering::IBLData::IBLData(const std::shared_ptr<prosper::Texture> &irradianceMap, const std::shared_ptr<prosper::Texture> &prefilterMap, const std::shared_ptr<prosper::Texture> &brdfMap) : irradianceMap {irradianceMap}, prefilterMap {prefilterMap}, brdfMap {brdfMap} {}

struct RenderSettings {
	std::string renderer = "luxcorerender";
	std::string sky = "skies/dusk379.hdr";
	EulerAngles skyAngles = {0.f, 160.f, 0.f};
	float skyStrength = 0.3f;
	float exposure = 50.f;
} static g_renderSettings;
static void map_build_reflection_probes(NetworkState *state, BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(get_cgame() == nullptr)
		return;
	std::unordered_map<std::string, console::CommandOption> commandOptions {};
	pragma::console::parse_command_options(argv, commandOptions);
	auto rebuild = commandOptions.contains("rebuild");
	auto closest = commandOptions.contains("closest");
	g_renderSettings.renderer = pragma::console::get_command_option_parameter_value(commandOptions, "renderer", util::declvalue(&::RenderSettings::renderer));
	g_renderSettings.sky = pragma::console::get_command_option_parameter_value(commandOptions, "sky", util::declvalue(&::RenderSettings::sky));
	g_renderSettings.skyStrength = util::to_float(pragma::console::get_command_option_parameter_value(commandOptions, "sky_strength", std::to_string(util::declvalue(&::RenderSettings::skyStrength))));
	g_renderSettings.exposure = util::to_float(pragma::console::get_command_option_parameter_value(commandOptions, "exposure", std::to_string(util::declvalue(&::RenderSettings::exposure))));
	auto defAngles = util::declvalue(&::RenderSettings::skyAngles);
	g_renderSettings.skyAngles = EulerAngles {pragma::console::get_command_option_parameter_value(commandOptions, "sky_angles", std::to_string(defAngles.p) + ' ' + std::to_string(defAngles.y) + ' ' + std::to_string(defAngles.r))};
	if(closest) {
		ecs::EntityIterator entIt {*get_cgame(), ecs::EntityIterator::FilterFlags::Default | ecs::EntityIterator::FilterFlags::Pending};
		entIt.AttachFilter<TEntityIteratorFilterComponent<CReflectionProbeComponent>>();
		CReflectionProbeComponent *probeClosest = nullptr;
		auto dClosest = std::numeric_limits<float>::max();
		Vector3 origin {};
		if(auto *cam = get_cgame()->GetRenderCamera<CCameraComponent>())
			origin = cam->GetEntity().GetPosition();
		for(const auto *entProbe : entIt) {
			auto d = uvec::distance_sqr(origin, entProbe->GetPosition());
			if(d > dClosest)
				continue;
			dClosest = d;
			probeClosest = entProbe->GetComponent<CReflectionProbeComponent>().get();
		}
		if(probeClosest == nullptr) {
			CReflectionProbeComponent::get_logger<CReflectionProbeComponent>().warn("No reflection probe found!");
			return;
		}
		std::vector<CReflectionProbeComponent *> probes {probeClosest};
		CReflectionProbeComponent::BuildReflectionProbes(*get_cgame(), probes, rebuild);
		return;
	}
	CReflectionProbeComponent::BuildAllReflectionProbes(*get_cgame(), rebuild);
}
namespace {
	auto UVN = console::client::register_command("map_build_reflection_probes", &map_build_reflection_probes, console::ConVarFlags::None,
	  "Build all reflection probes in the map. Use the '-rebuild' argument to clear all current IBL textures first. Use 'debug_pbr_ibl' to check the probes after they have been built.");
}
static void print_status(const uint32_t i, const uint32_t count)
{
	auto percent = math::ceil((count > 0u) ? (i / static_cast<float>(count) * 100.f) : 100.f);
	CReflectionProbeComponent::get_logger<CReflectionProbeComponent>().info("Reflection probe update at %{}", percent);
}

////////////////

CReflectionProbeComponent::CReflectionProbeComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
CReflectionProbeComponent::~CReflectionProbeComponent() { m_raytracingJobManager = nullptr; }
CReflectionProbeComponent::RaytracingJobManager::RaytracingJobManager(CReflectionProbeComponent &probe) : probe {probe} {}
CReflectionProbeComponent::RaytracingJobManager::~RaytracingJobManager()
{
	job.Cancel();
	job.Wait();
}
void CReflectionProbeComponent::RaytracingJobManager::StartNextJob()
{
	auto preprocessCompletionHandler = job.GetCompletionHandler();
	job.SetCompletionHandler([this, preprocessCompletionHandler](util::ParallelWorker<image::ImageLayerSet> &worker) {
		if(worker.IsSuccessful() == false) {
			get_logger<CReflectionProbeComponent>().warn("Raytracing scene for reflection probe has failed: {}", worker.GetResultMessage());
			probe.m_raytracingJobManager = nullptr;
			return;
		}
		if(preprocessCompletionHandler)
			preprocessCompletionHandler(worker);
		auto imgBuffer = worker.GetResult().images.begin()->second;
		m_equirectImageBuffer = imgBuffer;
		auto pThis = std::move(probe.m_raytracingJobManager); // Keep alive until end of scope
		Finalize();
	});
	job.Start();
	get_cengine()->AddParallelJob(job, "Reflection probe");
}
void CReflectionProbeComponent::RaytracingJobManager::Finalize()
{
	// Initialize cubemap image data from individual cubemap side buffers.
	// Since the cubemap image is in optimal layout with device memory,
	// we'll have to copy the data to a temporary buffer first and then
	// to the image via the command buffer.

	//auto cubemapImage = probe.CreateCubemapImage();
	//auto extents = cubemapImage->GetExtents();

	probe.GenerateFromEquirectangularImage(*m_equirectImageBuffer);

	// TODO: Equirect to cubemap
	//auto *memBlock = cubemapImage->GetAnvilImage().get_memory_block();
	//auto totalSize = memBlock->get_create_info_ptr()->get_size();
#if 0
	for(auto layerIndex=decltype(m_layerImageBuffers.size()){0u};layerIndex<m_layerImageBuffers.size();++layerIndex)
	{
		auto &imgBuffer = m_layerImageBuffers.at(layerIndex);
		auto imgDataSize = imgBuffer->GetSize();

		auto tmpBuf = pragma::get_cengine()->GetRenderContext().AllocateTemporaryBuffer(imgDataSize,0u /* alignment */,imgBuffer->GetData());

		auto &setupCmd = pragma::get_cengine()->GetSetupCommandBuffer();
		prosper::util::BufferImageCopyInfo copyInfo {};
		copyInfo.baseArrayLayer = layerIndex;
		copyInfo.dstImageLayout = prosper::ImageLayout::TransferDstOptimal;
		setupCmd->RecordCopyBufferToImage(copyInfo,*tmpBuf,*cubemapImage);

		pragma::get_cengine()->FlushSetupCommandBuffer();

		// Don't need the image buffer anymore
		imgBuffer = nullptr;
	}
	probe.FinalizeCubemap(*cubemapImage);
#endif
}

////////////////

//static auto *GUI_EL_NAME = "cubemap_generation_image";
static std::queue<ComponentHandle<CReflectionProbeComponent>> g_reflectionProbeQueue = {};
static std::vector<CReflectionProbeComponent *> get_probes()
{
	if(get_cgame() == nullptr)
		return {};
	ecs::EntityIterator entIt {*get_cgame(), ecs::EntityIterator::FilterFlags::Default | ecs::EntityIterator::FilterFlags::Pending};
	entIt.AttachFilter<TEntityIteratorFilterComponent<CReflectionProbeComponent>>();
	auto numProbes = entIt.GetCount();
	std::vector<CReflectionProbeComponent *> probes {};
	probes.reserve(numProbes);
	for(auto *ent : entIt) {
		auto reflProbeC = ent->GetComponent<CReflectionProbeComponent>();
		probes.push_back(reflProbeC.get());
	}
	return probes;
}
static void build_next_reflection_probe()
{
	if(get_cgame() == nullptr)
		return;
	while(g_reflectionProbeQueue.empty() == false) {
		auto hProbe = g_reflectionProbeQueue.front();
		g_reflectionProbeQueue.pop();
		if(hProbe.expired())
			continue;
		auto &probe = *hProbe;
		if(probe.RequiresRebuild() == false) {
			probe.LoadIBLReflectionsFromFile();
			continue;
		}
		auto &ent = probe.GetEntity();
		auto pos = ent.GetPosition();
		CReflectionProbeComponent::get_logger<CReflectionProbeComponent>().info("Updating reflection probe at position ({},{},{})...", pos.x, pos.y, pos.z);
		auto status = probe.UpdateIBLData(false);
		if(status == CReflectionProbeComponent::UpdateStatus::Pending)
			break; // Next reflection probe will automatically be generated once this one has completed rendering!
		if(status == CReflectionProbeComponent::UpdateStatus::Failed)
			CReflectionProbeComponent::get_logger<CReflectionProbeComponent>().warn("Unable to update reflection probe data for probe at position ({},{},{}). Probe will be unavailable!", pos.x, pos.y, pos.z);
	}

	/*auto &wgui = pragma::gui::WGUI::GetInstance();
	auto *p = dynamic_cast<WITexturedCubemap*>(wgui.GetBaseElement()->FindDescendantByName(GUI_EL_NAME));
	if(p)
	{
	auto hEl = p->GetHandle();
	pragma::get_cgame()->CreateTimer(5.f,0,FunctionCallback<void>::Create([hEl]() {
	if(hEl.IsValid())
	hEl.get()->Remove();
	}),TimerType::RealTime);
	}*/
}

void CReflectionProbeComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = CReflectionProbeComponent;

	{
		using TIblStrength = float;
		registerMember(create_component_member_info<T, TIblStrength, static_cast<void (T::*)(TIblStrength)>(&T::SetIBLStrength), static_cast<TIblStrength (T::*)() const>(&T::GetIBLStrength)>("iblStrength", 1.f));
	}

	{
		using TMaterial = std::string;
		auto memberInfo = create_component_member_info<T, TMaterial,
		  [](const ComponentMemberInfo &info, T &component, const TMaterial &value) {
			  component.SetCubemapIBLMaterialFilePath(value);
			  component.LoadIBLReflectionsFromFile();
		  },
		  [](const ComponentMemberInfo &info, T &component, TMaterial &value) {
			  auto path = util::Path::CreateFile(component.GetCubemapIBLMaterialFilePath());
			  path.PopFront();
			  value = path.GetString();
		  }>("iblMaterial", "", AttributeSpecializationType::File);
		auto &metaData = memberInfo.AddMetaData();
		metaData["assetType"] = "material";
		metaData["rootPath"] = util::Path::CreatePath(pragma::asset::get_asset_root_directory(asset::Type::Material)).GetString();
		metaData["extensions"] = pragma::asset::get_supported_extensions(asset::Type::Material, asset::FormatType::All);
		metaData["stripRootPath"] = true;
		metaData["stripExtension"] = true;
		registerMember(std::move(memberInfo));
	}
}

// These have been determined through experimentation and produce good results
// while not taking excessive time to render.
static uint32_t CUBEMAP_LAYER_WIDTH = 512;
static uint32_t CUBEMAP_LAYER_HEIGHT = 512;
static uint32_t RAYTRACING_SAMPLE_COUNT = 32;
void CReflectionProbeComponent::BuildReflectionProbes(Game &game, std::vector<CReflectionProbeComponent *> &probes, bool rebuild)
{
	g_reflectionProbeQueue = {};
	if(rebuild) {
		auto filePostfixes = pragma::asset::get_supported_extensions(asset::Type::Material);
		for(auto &ext : filePostfixes)
			ext = '.' + ext;
		filePostfixes.push_back("_irradiance.ktx");
		filePostfixes.push_back("_prefilter.ktx");
		// Clear existing IBL files
		for(auto *probe : probes) {
			probe->m_stateFlags |= StateFlags::RequiresRebuild;
			if(probe->m_iblMat.empty() == false)
				continue;
			auto path = util::Path {probe->GetCubemapIBLMaterialFilePath()};
			path.PopFront();
			path.RemoveFileExtension(pragma::asset::get_supported_extensions(asset::Type::Material));
			auto identifier = probe->GetCubemapIdentifier();
			for(auto &postfix : filePostfixes) {
				auto fpath = path + postfix;
				auto fileName = pragma::asset::find_file(path.GetString(), asset::Type::Material);
				if(fileName.has_value() == false)
					continue;
				get_logger<CReflectionProbeComponent>().info("Removing probe IBL file '{}'...", fpath.GetString());
				if(fs::remove_file(("materials/" + *fileName)))
					continue;
				get_logger<CReflectionProbeComponent>().warn("Unable to remove IBL file '{}'! This reflection probe may not be rebuilt!", fpath.GetString());
			}
		}
	}
	uint32_t numProbes = 0u;
	for(auto *probe : probes) {
		if(probe->RequiresRebuild()) {
			++numProbes;
			g_reflectionProbeQueue.push(probe->GetHandle<CReflectionProbeComponent>());
		}
	}
	get_logger<CReflectionProbeComponent>().info("Updating {} reflection probes... This may take a while!", numProbes);
	build_next_reflection_probe();
}
void CReflectionProbeComponent::BuildAllReflectionProbes(Game &game, bool rebuild)
{
	auto probes = get_probes();
	BuildReflectionProbes(game, probes, rebuild);
}

prosper::IDescriptorSet *CReflectionProbeComponent::FindDescriptorSetForClosestProbe(const CSceneComponent &scene, const Vector3 &origin, float &outIntensity)
{
	if(get_cgame() == nullptr)
		return nullptr;
	// Find closest reflection probe to camera position
	ecs::EntityIterator entIt {*get_cgame()};
	entIt.AttachFilter<TEntityIteratorFilterComponent<CReflectionProbeComponent>>();
	auto dClosest = std::numeric_limits<float>::max();
	ecs::BaseEntity *entClosest = nullptr;
	for(auto *ent : entIt) {
		if(static_cast<ecs::CBaseEntity *>(ent)->IsInScene(scene) == false)
			continue;
		auto posEnt = ent->GetPosition();
		auto d = uvec::distance_sqr(origin, posEnt);
		if(d >= dClosest)
			continue;
		dClosest = d;
		entClosest = ent;
	}
	if(entClosest) {
		auto &reflectionProbeC = *entClosest->GetComponent<CReflectionProbeComponent>();
		outIntensity = reflectionProbeC.GetIBLStrength();
		return reflectionProbeC.GetIBLDescriptorSet();
	}
	return nullptr;
}

bool CReflectionProbeComponent::GenerateFromEquirectangularImage(image::ImageBuffer &imgBuf)
{
	auto *shaderEquiRectToCubemap = static_cast<ShaderEquirectangularToCubemap *>(get_cengine()->GetShader("equirectangular_to_cubemap").get());
	if(shaderEquiRectToCubemap == nullptr)
		return false;
	auto imgEquirect = get_cengine()->GetRenderContext().CreateImage(imgBuf);
	auto texEquirect = get_cengine()->GetRenderContext().CreateTexture({}, *imgEquirect, prosper::util::ImageViewCreateInfo {}, prosper::util::SamplerCreateInfo {});
	auto cubemapTex = shaderEquiRectToCubemap->EquirectangularTextureToCubemap(*texEquirect, 256); // TODO: What resolution?
	return FinalizeCubemap(cubemapTex->GetImage());
}

void CReflectionProbeComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	GetEntity().AddComponent<CTransformComponent>();

	BindEvent(ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "env_map", false))
			m_srcEnvMap = kvData.value;
		else if(pragma::string::compare<std::string>(kvData.key, "ibl_material", false))
			m_iblMat = kvData.value;
		else if(pragma::string::compare<std::string>(kvData.key, "ibl_strength", false))
			m_strength = kvData.value.empty() ? std::optional<float> {} : util::to_float(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
}

void CReflectionProbeComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	auto &logger = InitLogger();
	if(LoadIBLReflectionsFromFile() == false)
		LogWarn("Invalid/missing IBL reflection resources for cubemap {}! Please run 'map_build_reflection_probes' to build all reflection probes!", GetCubemapIdentifier());
}

void CReflectionProbeComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	ClearDescriptorSet();
	ClearIblData();
}

std::string CReflectionProbeComponent::GetCubemapIBLMaterialFilePath() const
{
	if(m_iblMat.empty() == false)
		return "materials/" + m_iblMat;
	return "materials/" + GetCubemapIBLMaterialPath() + GetCubemapIdentifier() + "." + asset::FORMAT_MATERIAL_ASCII;
}

void CReflectionProbeComponent::SetCubemapIBLMaterialFilePath(const std::string &path) { m_iblMat = path; }

bool CReflectionProbeComponent::RequiresRebuild() const
{
	if(math::is_flag_set(m_stateFlags, StateFlags::BakingFailed))
		return false; // We've already tried baking the probe and it failed; don't try again!
	return math::is_flag_set(m_stateFlags, StateFlags::RequiresRebuild);
}

CReflectionProbeComponent::UpdateStatus CReflectionProbeComponent::UpdateIBLData(bool rebuild)
{
	if(rebuild == false && RequiresRebuild() == false)
		return UpdateStatus::Complete;
	if(m_srcEnvMap.empty() == false) {
		if(GenerateIBLReflectionsFromEnvMap("materials/" + m_srcEnvMap) == false)
			return UpdateStatus::Failed;
	}
	else
		return CaptureIBLReflectionsFromScene() ? UpdateStatus::Pending : UpdateStatus::Failed;
	return UpdateStatus::Complete;
}

void CReflectionProbeComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

bool CReflectionProbeComponent::SaveIBLReflectionsToFile()
{
	if(m_iblData == nullptr)
		return false;
	auto *client = get_client_state();
	if(m_iblMat.empty() == false) {
		if(pragma::asset::exists(m_iblMat, asset::Type::Material)) {
			auto *curMat = client->LoadMaterial(m_iblMat);
			if(curMat) {
				if(curMat->GetProperty("generated", false) == false)
					return false; // Don't overwrite non-generated material
			}
		}
	}

	auto relPath = GetCubemapIBLMaterialPath();
	auto absPath = "materials/" + relPath;
	fs::create_path(absPath);
	auto identifier = GetCubemapIdentifier();

	auto &imgPrefilter = m_iblData->prefilterMap->GetImage();
	auto &imgBrdf = m_iblData->brdfMap->GetImage();
	auto &imgIrradiance = m_iblData->irradianceMap->GetImage();

	auto fErrorHandler = [this](const std::string &errMsg) { LogWarn("Unable to create IBL reflection files: {}", errMsg); };
	const std::string pathBrdf = "materials/env/brdf.ktx";
	if(fs::exists(pathBrdf) == false) {
		image::TextureInfo imgWriteInfo {};
		imgWriteInfo.inputFormat = image::TextureInfo::InputFormat::R16G16B16A16_Float;
		imgWriteInfo.outputFormat = image::TextureInfo::OutputFormat::HDRColorMap;
		if(get_cgame()->SaveImage(imgBrdf, "materials/env/brdf", imgWriteInfo) == false) {
			fErrorHandler("Unable to save BRDF map!");
			return false;
		}
	}

	image::TextureInfo imgWriteInfo {};
	imgWriteInfo.inputFormat = image::TextureInfo::InputFormat::R16G16B16A16_Float;
	imgWriteInfo.outputFormat = image::TextureInfo::OutputFormat::HDRColorMap;
	auto prefix = identifier + "_";
	if(get_cgame()->SaveImage(imgPrefilter, absPath + prefix + "prefilter", imgWriteInfo) == false) {
		fErrorHandler("Unable to save prefilter map!");
		return false;
	}
	if(get_cgame()->SaveImage(imgIrradiance, absPath + prefix + "irradiance", imgWriteInfo) == false) {
		fErrorHandler("Unable to save irradiance map!");
		return false;
	}

	auto mat = client->CreateMaterial("ibl");
	if(mat == nullptr)
		return false;
	mat->SetTextureProperty("prefilter", relPath + prefix + "prefilter");
	mat->SetTextureProperty("irradiance", relPath + prefix + "irradiance");
	mat->SetTextureProperty("brdf", "env/brdf");
	mat->SetProperty("generated", true);
	auto rpath = util::Path::CreateFile(relPath + identifier + "." + asset::FORMAT_MATERIAL_ASCII);
	auto apath = pragma::asset::relative_path_to_absolute_path(rpath, asset::Type::Material);
	std::string err;
	apath.PopFront();
	auto result = mat->Save(apath.GetString(), err);
	if(result)
		client->LoadMaterial(rpath.GetString(), nullptr, true, true);
	return result;
}

util::ParallelJob<image::ImageLayerSet> CReflectionProbeComponent::CaptureRaytracedIBLReflectionsFromScene(uint32_t width, uint32_t height, const Vector3 &camPos, const Quat &camRot, float nearZ, float farZ, math::Degree fov, float exposure,
  const std::vector<ecs::BaseEntity *> *optEntityList, bool renderJob)
{
	rendering::cycles::SceneInfo sceneInfo {};
	sceneInfo.width = width;
	sceneInfo.height = height;
	sceneInfo.exposure = exposure;
	sceneInfo.device = rendering::cycles::SceneInfo::DeviceType::GPU;
	sceneInfo.colorTransform = rendering::cycles::SceneInfo::ColorTransform {};
	sceneInfo.colorTransform->config = "filmic-blender";
	sceneInfo.colorTransform->look = "Medium Contrast";
	sceneInfo.renderJob = renderJob;

	rendering::cycles::RenderImageInfo renderImgInfo {};
	renderImgInfo.camPose.SetOrigin(camPos);
	renderImgInfo.camPose.SetRotation(camRot);
	renderImgInfo.nearZ = nearZ;
	renderImgInfo.farZ = farZ;
	renderImgInfo.fov = fov;
	renderImgInfo.equirectPanorama = true;

	sceneInfo.sky = g_renderSettings.sky;
	sceneInfo.skyAngles = g_renderSettings.skyAngles;
	sceneInfo.skyStrength = g_renderSettings.skyStrength;
	sceneInfo.renderer = g_renderSettings.renderer;
	static auto useCycles = true;
	if(useCycles)
		sceneInfo.renderer = "cycles";
	else
		sceneInfo.renderer = "luxcorerender";

	sceneInfo.samples = RAYTRACING_SAMPLE_COUNT;
	sceneInfo.denoise = true;
	sceneInfo.hdrOutput = true;
	math::set_flag(sceneInfo.sceneFlags, rendering::cycles::SceneInfo::SceneFlags::CullObjectsOutsideCameraFrustum, false);

	std::shared_ptr<image::ImageBuffer> imgBuffer = nullptr;
	if(optEntityList)
		renderImgInfo.entityList = optEntityList;
	else {
		renderImgInfo.entityFilter = [](ecs::BaseEntity &ent) -> bool { return ent.IsMapEntity(); };
	}
	auto job = rendering::cycles::render_image(*get_client_state(), sceneInfo, renderImgInfo);
	if(job.IsValid() == false)
		return {};
	job.SetCompletionHandler([](util::ParallelWorker<image::ImageLayerSet> &worker) {
		if(worker.IsSuccessful() == false) {
			get_logger<CReflectionProbeComponent>().warn("Raytracing scene for IBL reflections has failed: {}", worker.GetResultMessage());
			return;
		}
	});
	return job;
}

std::shared_ptr<prosper::IImage> CReflectionProbeComponent::CreateCubemapImage()
{
	prosper::util::ImageCreateInfo createInfo {};
	createInfo.format = prosper::Format::R16G16B16A16_SFloat; // We need HDR colors for the cubemap
	createInfo.flags = prosper::util::ImageCreateInfo::Flags::Cubemap | prosper::util::ImageCreateInfo::Flags::FullMipmapChain;
	// The rendered cubemap itself will be discarded, so we render it at a high resolution
	// to get the best results for the subsequent stages.
	createInfo.width = CUBEMAP_LAYER_WIDTH;
	createInfo.height = CUBEMAP_LAYER_HEIGHT;
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	createInfo.postCreateLayout = prosper::ImageLayout::TransferDstOptimal;
	createInfo.tiling = prosper::ImageTiling::Optimal;
	createInfo.usage = prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::TransferSrcBit | prosper::ImageUsageFlags::TransferDstBit;
	return get_cengine()->GetRenderContext().CreateImage(createInfo);
}

bool CReflectionProbeComponent::CaptureIBLReflectionsFromScene(const std::vector<ecs::BaseEntity *> *optEntityList, bool renderJob)
{
	math::set_flag(m_stateFlags, StateFlags::BakingFailed, true); // Mark as failed until complete
	auto pos = GetEntity().GetPosition();
	LogInfo("Capturing reflection probe IBL reflections for probe at position ({},{},{})...", pos.x, pos.y, pos.z);

	auto *scene = get_cgame()->GetScene<CSceneComponent>();
	if(scene == nullptr)
		return false;
	auto hCam = scene->GetActiveCamera();
	if(hCam.expired()) {
		LogWarn("Unable to capture scene: Game scene camera is invalid!");
		return false;
	}

	auto hShaderPbr = get_cengine()->GetShader("pbr");
	if(hShaderPbr.expired()) {
		LogWarn("Unable to capture scene: PBR shader is not valid!");
		return false;
	}

	m_raytracingJobManager = std::unique_ptr<RaytracingJobManager> {new RaytracingJobManager {*this}};
	uint32_t width = 512;
	uint32_t height = 256;
	float exposure = g_renderSettings.exposure;
	auto job = CaptureRaytracedIBLReflectionsFromScene(width, height, pos, uquat::identity(), hCam->GetNearZ(), hCam->GetFarZ(), 90.f /* fov */, exposure, optEntityList, renderJob);
	if(job.IsValid() == false) {
		LogWarn("Unable to set scene up for reflection probe raytracing!");
		m_raytracingJobManager = nullptr;
		return false;
	}
	m_raytracingJobManager->job = job;
	m_raytracingJobManager->StartNextJob();
	return true;
#if 0
	constexpr auto useRaytracing = true;

	static const std::array<std::pair<Vector3,Vector3>,6> forwardUpDirs = {
		std::pair<Vector3,Vector3>{Vector3{1.0f,0.0f,0.0f},Vector3{0.0f,1.0f,0.0f}}, // Left
		std::pair<Vector3,Vector3>{Vector3{-1.0f,0.0f,0.0f},Vector3{0.0f,1.0f,0.0f}}, // Right
		std::pair<Vector3,Vector3>{Vector3{0.0f,1.0f,0.0f},Vector3{0.0f,0.0f,1.0f}}, // Top
		std::pair<Vector3,Vector3>{Vector3{0.0f,-1.0f,0.0f},Vector3{0.0f,0.0f,-1.0f}}, // Bottom
		std::pair<Vector3,Vector3>{Vector3{0.0f,0.0f,1.0f},Vector3{0.0f,1.0f,0.0f}}, // Front
		std::pair<Vector3,Vector3>{Vector3{0.0f,0.0f,-1.0f},Vector3{0.0f,1.0f,0.0f}} // Back
	};
	static const std::array<Mat4,6> cubemapViewMatrices = {
		glm::gtc::lookAtRH(Vector3{0.0f,0.0f,0.0f},forwardUpDirs.at(0).first,forwardUpDirs.at(0).second),
		glm::gtc::lookAtRH(Vector3{0.0f,0.0f,0.0f},forwardUpDirs.at(1).first,forwardUpDirs.at(1).second),
		glm::gtc::lookAtRH(Vector3{0.0f,0.0f,0.0f},forwardUpDirs.at(2).first,forwardUpDirs.at(2).second),
		glm::gtc::lookAtRH(Vector3{0.0f,0.0f,0.0f},forwardUpDirs.at(3).first,forwardUpDirs.at(3).second),
		glm::gtc::lookAtRH(Vector3{0.0f,0.0f,0.0f},forwardUpDirs.at(4).first,forwardUpDirs.at(4).second),
		glm::gtc::lookAtRH(Vector3{0.0f,0.0f,0.0f},forwardUpDirs.at(5).first,forwardUpDirs.at(5).second)
	};

	if(useRaytracing)
	{
		m_raytracingJobManager = std::unique_ptr<RaytracingJobManager>{new RaytracingJobManager{*this}};
		uint32_t numJobs = 0u;
		for(uint8_t iLayer=0;iLayer<6u;++iLayer)
		{
			auto &forward = forwardUpDirs.at(iLayer).first;
			auto &up = forwardUpDirs.at(iLayer).second;
			auto right = uvec::cross(forward,up);
			uvec::normalize(&right);
			auto rot = uquat::create(forward,right,up);
			auto job = CaptureRaytracedIBLReflectionsFromScene(CUBEMAP_LAYER_WIDTH,CUBEMAP_LAYER_HEIGHT,iLayer,pos,rot,hCam->GetNearZ(),hCam->GetFarZ(),90.f /* fov */);
			if(job.IsValid() == false)
				break;
			m_raytracingJobManager->jobs.at(iLayer) = job;
			++numJobs;
		}
		if(numJobs < 6)
		{
			LogWarn("Unable to set scene up for reflection probe raytracing!");
			m_raytracingJobManager = nullptr;
			return false;
		}
		m_raytracingJobManager->StartNextJob();
		return true;
	}

	// We're generating the IBL textures now, so we have to fall back to non-ibl mode.
	static_cast<pragma::ShaderPBR*>(hShaderPbr.get())->SetForceNonIBLMode(true);
	ScopeGuard sgIblMode {[hShaderPbr]() {
		static_cast<pragma::ShaderPBR*>(hShaderPbr.get())->SetForceNonIBLMode(false);
	}};

	auto oldRenderResolution = pragma::get_cengine()->GetRenderResolution();
	if(useRaytracing == false)
	{
		LogErr("Custom render resolutions currently not supported for reflection probes!");
		pragma::get_cengine()->SetRenderResolution(Vector2i{CUBEMAP_LAYER_WIDTH,CUBEMAP_LAYER_HEIGHT});
	}

	auto oldProjMat = hCam->GetProjectionMatrix();
	auto oldViewMat = hCam->GetViewMatrix();

	auto mProj = glm::gtc::perspectiveRH<float>(glm::radians(90.0f),1.f,hCam->GetNearZ(),hCam->GetFarZ());
	mProj = glm::gtc::scale(mProj,Vector3(-1.f,-1.f,1.f));
	hCam->SetProjectionMatrix(mProj);

	auto img = CreateCubemapImage();
	for(uint8_t iLayer=0;iLayer<6;++iLayer)
	{
		hCam->SetViewMatrix(cubemapViewMatrices.at(iLayer));
		hCam->GetEntity().SetPosition(pos);
		hCam->UpdateViewMatrix(); // TODO: Remove this?

		auto drawCmd = pragma::get_cengine()->GetSetupCommandBuffer();
		scene->UpdateBuffers(drawCmd); // TODO: Remove this?

		// TODO: FRender::Reflection is required to flip the winding order, but why is this needed in the first place?
		pragma::rendering::DrawSceneInfo drawSceneInfo {};
		drawSceneInfo.commandBuffer = drawCmd;
		drawSceneInfo.outputImage = img;
		drawSceneInfo.renderFlags = (FRender::All | FRender::HDR | FRender::Reflection) &~(FRender::View | FRender::Dynamic);
		drawSceneInfo.outputLayerId = iLayer;
		pragma::get_cgame()->RenderScene(drawSceneInfo);

		// We're flushing the command buffer for each layer
		// individually to make sure we're not gonna hit the TDR
		pragma::get_cengine()->FlushSetupCommandBuffer();
	}

	hCam->SetProjectionMatrix(oldProjMat);
	hCam->SetViewMatrix(oldViewMat);

	if(useRaytracing == false)
	{
		// Restore old render resolution TODO: Do this only once when capturing all cubemaps
		pragma::get_cengine()->SetRenderResolution(oldRenderResolution);
	}
	return FinalizeCubemap(*img);
#endif
}

bool CReflectionProbeComponent::FinalizeCubemap(prosper::IImage &imgCubemap)
{
	auto drawCmd = get_cengine()->GetSetupCommandBuffer();
	// Generate cubemap mipmaps
	drawCmd->RecordImageBarrier(imgCubemap, prosper::ImageLayout::TransferDstOptimal, prosper::ImageLayout::TransferSrcOptimal);
	drawCmd->RecordGenerateMipmaps(imgCubemap, prosper::ImageLayout::TransferSrcOptimal, prosper::AccessFlags::TransferReadBit | prosper::AccessFlags::TransferWriteBit, prosper::PipelineStageFlags::TransferBit);
	get_cengine()->FlushSetupCommandBuffer();
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	samplerCreateInfo.addressModeU = prosper::SamplerAddressMode::ClampToEdge;
	samplerCreateInfo.addressModeV = prosper::SamplerAddressMode::ClampToEdge;
	samplerCreateInfo.addressModeW = prosper::SamplerAddressMode::ClampToEdge;
	samplerCreateInfo.minFilter = prosper::Filter::Linear;
	samplerCreateInfo.magFilter = prosper::Filter::Linear;
	auto tex = get_cengine()->GetRenderContext().CreateTexture({}, imgCubemap, imgViewCreateInfo, samplerCreateInfo);

	LogInfo("Generating IBL reflection textures from reflection probe...");
	auto result = GenerateIBLReflectionsFromCubemap(*tex);
	if(result == false) {
		LogWarn("Generating IBL reflection textures has failed! Reflection probe will be unavailable.");
		build_next_reflection_probe();
		return result;
	}

	auto success = (m_iblData && SaveIBLReflectionsToFile());
	math::set_flag(m_stateFlags, StateFlags::BakingFailed, !success);
	build_next_reflection_probe();
	/*auto &wgui = pragma::gui::WGUI::GetInstance();
	auto *p = dynamic_cast<WITexturedCubemap*>(wgui.GetBaseElement()->FindDescendantByName(GUI_EL_NAME));
	if(p == nullptr)
	{
	p = wgui.Create<WITexturedCubemap>();
	p->SetName(GUI_EL_NAME);
	}
	p->SetTexture(*tex);
	p->SetSize(512,384);*/
	return success;
}

bool CReflectionProbeComponent::GenerateIBLReflectionsFromCubemap(prosper::Texture &cubemap)
{
	auto *shaderConvolute = static_cast<ShaderConvoluteCubemapLighting *>(get_cengine()->GetShader("convolute_cubemap_lighting").get());
	auto *shaderRoughness = static_cast<ShaderComputeIrradianceMapRoughness *>(get_cengine()->GetShader("compute_irradiance_map_roughness").get());
	auto *shaderBRDF = static_cast<ShaderBRDFConvolution *>(get_cengine()->GetShader("brdf_convolution").get());
	if(shaderConvolute == nullptr || shaderRoughness == nullptr || shaderBRDF == nullptr)
		return false;
	auto irradianceMap = shaderConvolute->ConvoluteCubemapLighting(cubemap, 32);
	auto prefilterMap = shaderRoughness->ComputeRoughness(cubemap, 512);

	std::shared_ptr<void> texPtr = nullptr;

	// Load BRDF texture from disk, if it already exists
	auto loadInfo = std::make_unique<material::TextureLoadInfo>();
	loadInfo->mipmapMode = material::TextureMipmapMode::Ignore;
	std::shared_ptr<prosper::Texture> brdfTex = nullptr;
	auto texInfo = static_cast<material::CMaterialManager &>(get_client_state()->GetMaterialManager()).GetTextureManager().LoadAsset("env/brdf.ktx", std::move(loadInfo));
	if(texInfo)
		brdfTex = texInfo->GetVkTexture();
	// Otherwise generate it
	if(brdfTex == nullptr)
		brdfTex = shaderBRDF->CreateBRDFConvolutionMap(512);

	if(irradianceMap == nullptr || prefilterMap == nullptr)
		return false;
	ClearDescriptorSet();
	ClearIblData();
	m_iblData = std::make_unique<rendering::IBLData>(irradianceMap, prefilterMap, brdfTex);
	InitializeDescriptorSet();

	// Debug test: Apply texture to skybox
	/*{
		for(auto &pair : pragma::get_client_state()->GetMaterialManager().GetMaterials())
		{
			if(pair.first.find("skybox") == std::string::npos)
				continue;
			auto *texInfo = pair.second.get()->GetTextureInfo("skybox");
			std::static_pointer_cast<material::Texture>(texInfo->texture)->texture = m_iblData->prefilterMap;
			pragma::get_cgame()->ReloadMaterialShader(static_cast<material::CMaterial*>(pair.second.get()));
			break;
		}
	}*/
	return SaveIBLReflectionsToFile();
}

bool CReflectionProbeComponent::GenerateIBLReflectionsFromEnvMap(const std::string &envMapFileName)
{
	auto *shaderEquiRectToCubemap = static_cast<ShaderEquirectangularToCubemap *>(get_cengine()->GetShader("equirectangular_to_cubemap").get());
	if(shaderEquiRectToCubemap == nullptr)
		return false;
	auto pos = GetEntity().GetPosition();
	LogInfo("Generating reflection probe IBL reflections for probe at position ({},{},{}) using environment map '{}'...", pos.x, pos.y, pos.z, envMapFileName);
	auto cubemapTex = shaderEquiRectToCubemap->LoadEquirectangularImage(envMapFileName, 512);
	if(cubemapTex == nullptr)
		return false;
	return GenerateIBLReflectionsFromCubemap(*cubemapTex);
}
material::Material *CReflectionProbeComponent::LoadMaterial(bool &outIsDefault)
{
	outIsDefault = false;
	auto matPath = util::Path {GetCubemapIBLMaterialFilePath()};
	matPath.PopFront();
	if(pragma::asset::exists(matPath.GetString(), asset::Type::Material) == false) {
		outIsDefault = true;
		matPath = "maps/default_ibl." + std::string {asset::FORMAT_MATERIAL_ASCII};
	}
	auto *mat = get_client_state()->LoadMaterial(matPath.GetString(), nullptr, false, true);
	if(mat && !mat->IsError())
		return mat;
	LogWarn("Failed to load material '{}'!", matPath.GetString());
	return nullptr;
}
void CReflectionProbeComponent::ClearIblData()
{
	if(m_iblData) {
		get_cengine()->GetRenderContext().KeepResourceAliveUntilPresentationComplete(m_iblData->brdfMap);
		get_cengine()->GetRenderContext().KeepResourceAliveUntilPresentationComplete(m_iblData->irradianceMap);
		get_cengine()->GetRenderContext().KeepResourceAliveUntilPresentationComplete(m_iblData->prefilterMap);
	}
	m_iblData = nullptr;
}
bool CReflectionProbeComponent::LoadIBLReflectionsFromFile()
{
	auto isDefaultMaterial = false;
	auto *mat = LoadMaterial(isDefaultMaterial);
	if(mat == nullptr)
		return false;
	auto *pPrefilter = mat->GetTextureInfo("prefilter");
	auto *pIrradiance = mat->GetTextureInfo("irradiance");
	auto *pBrdf = mat->GetTextureInfo("brdf");
	if(!pPrefilter) {
		LogWarn("Material has no prefilter texture!");
		return false;
	}
	if(!pIrradiance) {
		LogWarn("Material has no irradiance texture!");
		return false;
	}
	if(!pBrdf) {
		LogWarn("Material has no brdf texture!");
		return false;
	}
	auto texPrefilter = std::static_pointer_cast<material::Texture>(pPrefilter->texture);
	auto texIrradiance = std::static_pointer_cast<material::Texture>(pIrradiance->texture);
	auto texBrdf = std::static_pointer_cast<material::Texture>(pBrdf->texture);
	if(!texPrefilter || !texPrefilter->HasValidVkTexture()) {
		LogWarn("Prefilter texture is invalid!");
		return false;
	}
	if(!texIrradiance || !texIrradiance->HasValidVkTexture()) {
		LogWarn("Irradiance texture is invalid!");
		return false;
	}
	if(!texBrdf || !texBrdf->HasValidVkTexture()) {
		LogWarn("Brdf texture is invalid!");
		return false;
	}
	ClearDescriptorSet();
	ClearIblData();
	m_iblData = std::make_unique<rendering::IBLData>(texIrradiance->GetVkTexture(), texPrefilter->GetVkTexture(), texBrdf->GetVkTexture());
	if(m_strength.has_value())
		m_iblData->strength = *m_strength;
	else
		mat->GetProperty("ibl_strength", &m_iblData->strength);

	// TODO: Do this properly (e.g. via material attributes)
	//static auto brdfSamplerInitialized = false;
	//if(brdfSamplerInitialized == false)
	{
		//brdfSamplerInitialized = true;
		prosper::util::SamplerCreateInfo samplerCreateInfo {};
		samplerCreateInfo.addressModeU = prosper::SamplerAddressMode::ClampToEdge;
		samplerCreateInfo.addressModeV = prosper::SamplerAddressMode::ClampToEdge;
		samplerCreateInfo.addressModeW = prosper::SamplerAddressMode::ClampToEdge;
		samplerCreateInfo.minFilter = prosper::Filter::Linear;
		samplerCreateInfo.magFilter = prosper::Filter::Linear;
		auto sampler = get_cengine()->GetRenderContext().CreateSampler(samplerCreateInfo);
		texIrradiance->GetVkTexture()->SetSampler(*sampler);
		texBrdf->GetVkTexture()->SetSampler(*sampler);

		samplerCreateInfo.mipmapMode = prosper::SamplerMipmapMode::Linear;
		sampler = get_cengine()->GetRenderContext().CreateSampler(samplerCreateInfo);
		texPrefilter->GetVkTexture()->SetSampler(*sampler);
	}

	InitializeDescriptorSet();
	if(isDefaultMaterial == false)
		math::set_flag(m_stateFlags, StateFlags::RequiresRebuild, false);
	LogInfo("Loaded IBL reflection resources for cubemap {}!", GetCubemapIdentifier());
	return true;
}
void CReflectionProbeComponent::ClearDescriptorSet()
{
	if(m_iblDsg)
		get_cengine()->GetRenderContext().KeepResourceAliveUntilPresentationComplete(m_iblDsg);
	m_iblDsg = nullptr;
}
void CReflectionProbeComponent::InitializeDescriptorSet()
{
	ClearDescriptorSet();
	if(m_iblData == nullptr)
		return;
	auto &context = get_cengine()->GetRenderContext();
	m_iblDsg = context.CreateDescriptorSetGroup(ShaderPBR::DESCRIPTOR_SET_PBR);
	auto &ds = *m_iblDsg->GetDescriptorSet();
	ds.SetBindingTexture(*m_iblData->irradianceMap, math::to_integral(ShaderPBR::PBRBinding::IrradianceMap));
	ds.SetBindingTexture(*m_iblData->prefilterMap, math::to_integral(ShaderPBR::PBRBinding::PrefilterMap));
	ds.SetBindingTexture(*m_iblData->brdfMap, math::to_integral(ShaderPBR::PBRBinding::BRDFMap));

	m_iblDsg->GetDescriptorSet()->Update();
}
std::string CReflectionProbeComponent::GetCubemapIBLMaterialPath() const
{
	if(m_iblMat.empty() == false)
		return ufile::get_path_from_filename(m_iblMat);
	return "maps/" + get_cgame()->GetMapName() + "/ibl/";
}
std::string CReflectionProbeComponent::GetLocationIdentifier() const
{
	auto pos = GetEntity().GetPosition();
	auto identifier = std::to_string(pos.x) + std::to_string(pos.y) + std::to_string(pos.z);
	return std::to_string(std::hash<std::string> {}(identifier));
}
std::string CReflectionProbeComponent::GetCubemapIdentifier() const
{
	if(m_srcEnvMap.empty() == false)
		return std::to_string(std::hash<std::string> {}(m_srcEnvMap));
	return GetLocationIdentifier();
}
prosper::IDescriptorSet *CReflectionProbeComponent::GetIBLDescriptorSet() { return m_iblDsg ? m_iblDsg->GetDescriptorSet() : nullptr; }

float CReflectionProbeComponent::GetIBLStrength() const { return m_iblData ? m_iblData->strength : 0.f; }

void CReflectionProbeComponent::SetIBLStrength(float iblStrength)
{
	if(m_iblData == nullptr)
		return;
	m_iblData->strength = iblStrength;
}

const rendering::IBLData *CReflectionProbeComponent::GetIBLData() const { return m_iblData.get(); }

////////

void CEnvReflectionProbe::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CReflectionProbeComponent>();
}

////////

static void debug_pbr_ibl(NetworkState *state, BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(get_cgame() == nullptr)
		return;
	const std::string name = "pbr_ibl_brdf";
	auto &wgui = gui::WGUI::GetInstance();
	auto *pRoot = wgui.GetBaseElement();
	auto *p = pRoot->FindDescendantByName(name);
	if(p != nullptr) {
		p->Remove();
		return;
	}

	if(get_cgame() == nullptr)
		return;

	ecs::EntityIterator entIt {*get_cgame()};
	entIt.AttachFilter<TEntityIteratorFilterComponent<CReflectionProbeComponent>>();

	auto origin = pl->GetEntity().GetPosition();
	auto dClosest = std::numeric_limits<float>::max();
	ecs::BaseEntity *entClosest = nullptr;
	for(auto *ent : entIt) {
		auto trC = ent->GetTransformComponent();
		if(!trC)
			continue;
		auto d = uvec::distance_sqr(origin, trC->GetOrigin());
		if(d > dClosest)
			continue;
		dClosest = d;
		entClosest = ent;
	}

	if(entClosest == nullptr) {
		CReflectionProbeComponent::get_logger<CReflectionProbeComponent>().info("No reflection probe found!");
		return;
	}

	auto *cam = get_cgame()->GetRenderCamera<CCameraComponent>();
	if(cam)
		get_cgame()->DrawLine(cam->GetEntity().GetPosition(), entClosest->GetPosition(), colors::Red, 30.f);

	auto reflProbeC = entClosest->GetComponent<CReflectionProbeComponent>();
	if(reflProbeC.expired())
		return;
	auto *iblData = reflProbeC->GetIBLData();
	if(iblData == nullptr) {
		CReflectionProbeComponent::get_logger<CReflectionProbeComponent>().info("No IBL textures available for reflection probe!");
		return;
	}
	auto &brdfMap = iblData->brdfMap;
	auto &irradianceMap = iblData->irradianceMap;
	auto &prefilterMap = iblData->prefilterMap;

	auto *pElContainer = wgui.Create<gui::types::WIBase>();
	pElContainer->SetAutoAlignToParent(true);
	pElContainer->SetName(name);
	pElContainer->TrapFocus(true);
	pElContainer->RequestFocus();

	auto *pFrameBrdf = wgui.Create<gui::types::WIFrame>(pElContainer);
	pFrameBrdf->SetTitle("BRDF");
	auto *pBrdf = wgui.Create<gui::types::WITexturedRect>(pFrameBrdf);
	pBrdf->SetSize(256, 256);
	pBrdf->SetY(24);
	pBrdf->SetTexture(*brdfMap);
	pFrameBrdf->SizeToContents();
	pBrdf->SetAnchor(0.f, 0.f, 1.f, 1.f);

	auto maxLod = brdfMap->GetImage().GetMipmapCount();
	if(maxLod > 1) {
		auto *pSlider = wgui.Create<gui::types::WISlider>(pBrdf);
		pSlider->SetSize(pSlider->GetParent()->GetWidth(), 24);
		pSlider->SetRange(0.f, maxLod, 0.01f);
		pSlider->SetPostFix(" LOD");
		auto hBrdf = pBrdf->GetHandle();
		pSlider->AddCallback("OnChange", FunctionCallback<void, float, float>::Create([hBrdf](float oldVal, float newVal) mutable {
			if(hBrdf.IsValid() == false)
				return;
			static_cast<gui::types::WITexturedRect *>(hBrdf.get())->SetLOD(newVal);
		}));
		pSlider->SetAnchor(0.f, 0.f, 1.f, 0.f);
	}

	///

	auto *pFrameIrradiance = wgui.Create<gui::types::WIFrame>(pElContainer);
	pFrameIrradiance->SetTitle("Irradiance");
	pFrameIrradiance->SetX(pFrameBrdf->GetRight());
	auto *pIrradiance = wgui.Create<gui::types::WITexturedCubemap>(pFrameIrradiance);
	pIrradiance->SetY(24);
	pIrradiance->SetTexture(*irradianceMap);
	pFrameIrradiance->SizeToContents();
	pIrradiance->SetAnchor(0.f, 0.f, 1.f, 1.f);

	maxLod = irradianceMap->GetImage().GetMipmapCount();
	if(maxLod > 1) {
		auto *pSlider = wgui.Create<gui::types::WISlider>(pIrradiance);
		pSlider->SetSize(pSlider->GetParent()->GetWidth(), 24);
		pSlider->SetRange(0.f, maxLod, 0.01f);
		pSlider->SetPostFix(" LOD");
		auto hIrradiance = pIrradiance->GetHandle();
		pSlider->AddCallback("OnChange", FunctionCallback<void, float, float>::Create([hIrradiance](float oldVal, float newVal) mutable {
			if(hIrradiance.IsValid() == false)
				return;
			static_cast<gui::types::WITexturedCubemap *>(hIrradiance.get())->SetLOD(newVal);
		}));
		pSlider->SetAnchor(0.f, 0.f, 1.f, 0.f);
	}

	///

	auto *pFramePrefilter = wgui.Create<gui::types::WIFrame>(pElContainer);
	pFramePrefilter->SetTitle("Prefilter");
	pFramePrefilter->SetY(pFrameIrradiance->GetBottom());
	auto *pPrefilter = wgui.Create<gui::types::WITexturedCubemap>(pFramePrefilter);
	pPrefilter->SetY(24);
	pPrefilter->SetTexture(*prefilterMap);
	pFramePrefilter->SizeToContents();
	pPrefilter->SetAnchor(0.f, 0.f, 1.f, 1.f);

	maxLod = prefilterMap->GetImage().GetMipmapCount();
	if(maxLod > 1) {
		auto *pSlider = wgui.Create<gui::types::WISlider>(pPrefilter);
		pSlider->SetSize(pSlider->GetParent()->GetWidth(), 24);
		pSlider->SetRange(0.f, maxLod, 0.01f);
		pSlider->SetPostFix(" LOD");
		auto hPrefilter = pPrefilter->GetHandle();
		pSlider->AddCallback("OnChange", FunctionCallback<void, float, float>::Create([hPrefilter](float oldVal, float newVal) mutable {
			if(hPrefilter.IsValid() == false)
				return;
			static_cast<gui::types::WITexturedCubemap *>(hPrefilter.get())->SetLOD(newVal);
		}));
		pSlider->SetAnchor(0.f, 0.f, 1.f, 0.f);
	}
}
namespace {
	auto UVN = console::client::register_command("debug_pbr_ibl", &debug_pbr_ibl, console::ConVarFlags::None, "Displays the irradiance, prefilter and brdf map for the closest cubemap.");
}
