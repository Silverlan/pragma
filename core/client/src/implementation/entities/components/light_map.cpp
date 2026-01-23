// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <GuillotineBinPack.h>

module pragma.client;

import :entities.components.light_map;
import :client_state;
import :engine;
import :game;
import :gui;
import :model;

using namespace pragma;
spdlog::logger &CLightMapComponent::LOGGER = register_logger("lightmap");

void CLightMapComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = CLightMapComponent;

	using TExposure = float;
	{
		auto memberInfo = create_component_member_info<T, TExposure, static_cast<void (T::*)(TExposure)>(&T::SetLightMapExposure), static_cast<TExposure (T::*)() const>(&T::GetLightMapExposure)>("exposure", 1.f);
		memberInfo.SetMin(-10.f);
		memberInfo.SetMax(10.f);
		registerMember(std::move(memberInfo));
	}

	{
		using TMaterial = std::string;
		auto memberInfo
		  = create_component_member_info<T, TMaterial, [](const ComponentMemberInfo &info, T &component, const TMaterial &value) { component.SetLightMapMaterial(value); }, [](const ComponentMemberInfo &info, T &component, TMaterial &value) { value = component.GetLightMapMaterialName(); }>(
		    "lightmapMaterial", "", AttributeSpecializationType::File);
		auto &metaData = memberInfo.AddMetaData();
		metaData["assetType"] = "material";
		metaData["rootPath"] = util::Path::CreatePath(pragma::asset::get_asset_root_directory(asset::Type::Material)).GetString();
		metaData["extensions"] = pragma::asset::get_supported_extensions(asset::Type::Material, asset::FormatType::All);
		metaData["stripRootPath"] = true;
		metaData["stripExtension"] = true;
		registerMember(std::move(memberInfo));
	}
}
CLightMapComponent::CLightMapComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent), m_lightMapExposure {util::FloatProperty::Create(0.f)} {}

void CLightMapComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CLightMapComponent::Initialize() { BaseEntityComponent::Initialize(); }

void CLightMapComponent::InitializeLightMapData(const std::shared_ptr<prosper::Texture> &lightMap, const std::shared_ptr<prosper::IDynamicResizableBuffer> &lightMapUvBuffer, const std::vector<std::shared_ptr<prosper::IBuffer>> &meshUvBuffers,
  const std::shared_ptr<prosper::Texture> &directionalLightmap, bool keepCurrentTextures)
{
	if(!keepCurrentTextures) {
		m_textures[math::to_integral(Texture::DiffuseMap)] = lightMap;
		m_textures[math::to_integral(Texture::DominantDirectionMap)] = directionalLightmap;
	}
	m_meshLightMapUvBuffer = lightMapUvBuffer;
	m_meshLightMapUvBuffers = meshUvBuffers;

	ecs::EntityIterator entIt {*get_cgame(), ecs::EntityIterator::FilterFlags::Default | ecs::EntityIterator::FilterFlags::Pending};
	entIt.AttachFilter<TEntityIteratorFilterComponent<CLightMapReceiverComponent>>();
	for(auto *ent : entIt) {
		auto lightMapReceiverC = ent->GetComponent<CLightMapReceiverComponent>();
		lightMapReceiverC->UpdateMeshLightmapUvBuffers(*this);

		auto mdlC = ent->GetComponent<CModelComponent>();
		if(mdlC.valid()) {
			mdlC->SetRenderMeshesDirty();
			mdlC->UpdateLOD(0);
		}
	}
	CRasterizationRendererComponent::UpdateLightmap(*this);
}

const std::shared_ptr<prosper::Texture> &CLightMapComponent::GetTexture(Texture tex) const { return m_textures[math::to_integral(tex)]; }

void CLightMapComponent::InitializeFromMaterial()
{
	LOGGER.info("Initializing lightmap from material '{}'...", m_lightMapMaterialName);
	for(auto &tex : m_textures)
		tex = nullptr;
	m_lightMapMaterial = {};
	if(m_lightMapMaterialName.empty()) {
		LOGGER.warn("No lightmap material specified!");
		return;
	}
	auto *mat = get_client_state()->LoadMaterial(m_lightMapMaterialName);
	if(!mat) {
		LOGGER.error("Unable to load lightmap material '{}'!", m_lightMapMaterialName);
		return;
	}
	m_lightMapMaterial = mat->GetHandle();
	auto getTexture = [this](const std::string &identifier) -> std::shared_ptr<prosper::Texture> {
		if(!m_lightMapMaterial)
			return nullptr;
		auto *map = m_lightMapMaterial->GetTextureInfo(identifier);
		if(!map || !map->texture)
			return nullptr;
		auto *tex = static_cast<material::Texture *>(map->texture.get());
		return tex->GetVkTexture();
	};
	m_textures[math::to_integral(Texture::DiffuseMap)] = getTexture("diffuse_map");
	m_textures[math::to_integral(Texture::DiffuseDirectMap)] = getTexture("diffuse_direct_map");
	m_textures[math::to_integral(Texture::DiffuseIndirectMap)] = getTexture("diffuse_indirect_map");
	m_textures[math::to_integral(Texture::DominantDirectionMap)] = getTexture("dominant_direction_map");

	for(auto i = decltype(m_textures.size()) {0u}; i < m_textures.size(); ++i) {
		auto e = static_cast<Texture>(i);
		auto &tex = m_textures[i];
		LOGGER.info("Texture state for '{}': {}", magic_enum::enum_name(e), tex ? "loaded" : "not loaded");
	}

	CRasterizationRendererComponent::UpdateLightmap(*this);
}

void CLightMapComponent::SetLightMapAtlas(const std::shared_ptr<prosper::Texture> &lightMap)
{
	m_textures[math::to_integral(Texture::DiffuseMap)] = lightMap;

	// TODO: This method only allows one lightmap atlas globally; Implement this in a way that allows multiple (maybe add to entity descriptor set?)!
	CRasterizationRendererComponent::UpdateLightmap(*this);
}
const std::shared_ptr<prosper::Texture> &CLightMapComponent::GetLightMapAtlas() const { return m_textures[math::to_integral(Texture::DiffuseMap)]; }
void CLightMapComponent::SetDirectionalLightMapAtlas(const std::shared_ptr<prosper::Texture> &lightMap)
{
	m_textures[math::to_integral(Texture::DominantDirectionMap)] = lightMap;

	// TODO: This method only allows one lightmap atlas globally; Implement this in a way that allows multiple (maybe add to entity descriptor set?)!
	CRasterizationRendererComponent::UpdateLightmap(*this);
}
const std::shared_ptr<prosper::Texture> &CLightMapComponent::GetDirectionalLightMapAtlas() const { return m_textures[math::to_integral(Texture::DominantDirectionMap)]; }

bool CLightMapComponent::HasValidLightMap() const { return m_textures[math::to_integral(Texture::DiffuseMap)] != nullptr || m_textures[math::to_integral(Texture::DiffuseDirectMap)]; }

void CLightMapComponent::SetLightMapMaterial(const std::string &matName)
{
	m_lightMapMaterialName = matName;
	get_client_state()->PrecacheMaterial(m_lightMapMaterialName);
	if(GetEntity().IsSpawned())
		InitializeFromMaterial();
}

const std::string &CLightMapComponent::GetLightMapMaterialName() const { return m_lightMapMaterialName; }

void CLightMapComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	InitializeFromMaterial();
}

void CLightMapComponent::ReloadLightMapData()
{
	std::vector<std::shared_ptr<prosper::IBuffer>> buffers {};
	auto globalLightmapUvBuffer = GenerateLightmapUVBuffers(buffers);
	InitializeLightMapData(nullptr, globalLightmapUvBuffer, buffers, nullptr, true);
	UpdateLightmapUvBuffers();
}

const std::shared_ptr<prosper::Texture> &CLightMapComponent::GetLightMap() const { return m_textures[math::to_integral(Texture::DiffuseMap)]; }
const std::shared_ptr<prosper::Texture> &CLightMapComponent::GetDirectionalLightMap() const { return m_textures[math::to_integral(Texture::DominantDirectionMap)]; }

prosper::IBuffer *CLightMapComponent::GetMeshLightMapUvBuffer(uint32_t meshIdx) const
{
	if(meshIdx >= m_meshLightMapUvBuffers.size())
		return nullptr;
	return m_meshLightMapUvBuffers.at(meshIdx).get();
}
std::shared_ptr<prosper::IDynamicResizableBuffer> CLightMapComponent::GetGlobalLightMapUvBuffer() const { return m_meshLightMapUvBuffer; }

const std::vector<std::shared_ptr<prosper::IBuffer>> &CLightMapComponent::GetMeshLightMapUvBuffers() const { return const_cast<CLightMapComponent *>(this)->GetMeshLightMapUvBuffers(); }
std::vector<std::shared_ptr<prosper::IBuffer>> &CLightMapComponent::GetMeshLightMapUvBuffers() { return m_meshLightMapUvBuffers; }

void CLightMapComponent::SetLightMapExposure(float exp) { *m_lightMapExposure = exp; }
float CLightMapComponent::GetLightMapExposure() const { return *m_lightMapExposure; }
float CLightMapComponent::CalcLightMapPowExposurePow() const { return math::pow(2.0, static_cast<double>(GetLightMapExposure())); }
void CLightMapComponent::UpdateLightmapUvBuffers()
{
	// TODO: Move this function to light map receiver component?
	auto uvBuffer = GetGlobalLightMapUvBuffer();
	auto mdl = GetEntity().GetModel();
	auto meshGroup = mdl ? mdl->GetMeshGroup(0) : nullptr;
	if(uvBuffer == nullptr || meshGroup == nullptr || uvBuffer->Map(0ull, uvBuffer->GetSize(), prosper::IBuffer::MapFlags::WriteBit) == false)
		return;
	auto *cache = GetLightmapDataCache();

	auto &uvBuffers = GetMeshLightMapUvBuffers();
	ecs::EntityIterator entIt {*get_cgame(), ecs::EntityIterator::FilterFlags::Default | ecs::EntityIterator::FilterFlags::Pending};
	entIt.AttachFilter<TEntityIteratorFilterComponent<CLightMapReceiverComponent>>();
	for(auto *ent : entIt) {
		auto lightMapReceiverC = ent->GetComponent<CLightMapReceiverComponent>();
		for(auto &mesh : meshGroup->GetMeshes()) {
			for(auto &subMesh : mesh->GetSubMeshes()) {
				const std::vector<Vector2> *uvSet = nullptr;
				if(cache)
					uvSet = cache->FindLightmapUvs(ent->GetUuid(), subMesh->GetUuid());
				else
					uvSet = subMesh->GetUVSet("lightmap");
				auto bufIdx = lightMapReceiverC->FindBufferIndex(static_cast<geometry::CModelSubMesh &>(*subMesh));
				if(uvSet == nullptr || bufIdx.has_value() == false)
					continue;
				uvBuffers.at(*bufIdx)->Write(0, uvSet->size() * sizeof(uvSet->front()), uvSet->data());
			}
		}
	}
	uvBuffer->Unmap();
}

const rendering::LightmapDataCache *CLightMapComponent::GetLightmapDataCache() const { return m_lightmapDataCache.get(); }
void CLightMapComponent::SetLightmapDataCache(rendering::LightmapDataCache *cache) { m_lightmapDataCache = cache ? cache->shared_from_this() : nullptr; }

std::shared_ptr<prosper::IDynamicResizableBuffer> CLightMapComponent::GenerateLightmapUVBuffers(std::vector<std::shared_ptr<prosper::IBuffer>> &outMeshLightMapUvBuffers)
{
	LOGGER.info("Generating lightmap uv buffers...");
	prosper::util::BufferCreateInfo bufCreateInfo {};
	bufCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	bufCreateInfo.usageFlags = prosper::BufferUsageFlags::VertexBufferBit | prosper::BufferUsageFlags::TransferSrcBit | prosper::BufferUsageFlags::TransferDstBit; // Transfer flags are required for mapping GPUBulk buffers
	auto alignment = get_cengine()->GetRenderContext().CalcBufferAlignment(bufCreateInfo.usageFlags);
	auto requiredBufferSize = 0ull;

	// Collect all meshes that have lightmap uv coordinates
	ecs::EntityIterator entIt {*get_cgame(), ecs::EntityIterator::FilterFlags::Default | ecs::EntityIterator::FilterFlags::Pending};
	entIt.AttachFilter<TEntityIteratorFilterComponent<CLightMapReceiverComponent>>();

	// Calculate required buffer size
	uint32_t numMeshes = 0;
	for(auto *ent : entIt) {
		auto lightMapReceiverC = ent->GetComponent<CLightMapReceiverComponent>();
		auto &meshLightMapUvData = lightMapReceiverC->GetMeshLightMapUvData();
		for(auto &pair : meshLightMapUvData) {
			auto &uvSet = pair.second;
			requiredBufferSize += prosper::util::get_aligned_size(uvSet.size() * sizeof(uvSet.front()), alignment);
			++numMeshes;
		}
	}

	if(numMeshes == 0) {
		LOGGER.warn("No meshes with lightmap uv coordinates found!");
		return nullptr;
	}

	// Generate the lightmap uv buffer
	bufCreateInfo.size = requiredBufferSize;
	auto lightMapUvBuffer = get_cengine()->GetRenderContext().CreateDynamicResizableBuffer(bufCreateInfo, bufCreateInfo.size, 0.2f);
	if(!lightMapUvBuffer) {
		LOGGER.error("Unable to create lightmap uv buffer!");
		return nullptr;
	}
	if(lightMapUvBuffer->Map(0ull, lightMapUvBuffer->GetSize(), prosper::IBuffer::MapFlags::WriteBit) == false) {
		LOGGER.error("Unable to map lightmap uv buffer!");
		return nullptr;
	}

	outMeshLightMapUvBuffers.reserve(numMeshes);
	uint32_t bufIdx = 0;
	for(auto *ent : entIt) {
		auto lightMapReceiverC = ent->GetComponent<CLightMapReceiverComponent>();
		auto &meshLightMapUvData = lightMapReceiverC->GetMeshLightMapUvData();
		for(auto &pair : meshLightMapUvData) {
			auto &uvSet = pair.second;
			auto subBuf = lightMapUvBuffer->AllocateBuffer(uvSet.size() * sizeof(uvSet.front()));
			outMeshLightMapUvBuffers.push_back(subBuf);

			if(subBuf)
				lightMapUvBuffer->Write(subBuf->GetStartOffset(), uvSet.size() * sizeof(uvSet.front()), uvSet.data());
			lightMapReceiverC->AssignBufferIndex(pair.first, bufIdx++);
		}
	}
	lightMapUvBuffer->Unmap();
	return lightMapUvBuffer;
}

std::shared_ptr<prosper::Texture> CLightMapComponent::CreateLightmapTexture(image::ImageBuffer &imgBuf)
{
	imgBuf.Convert(image::Format::RGBA16);
	prosper::util::ImageCreateInfo lightMapCreateInfo {};
	lightMapCreateInfo.width = imgBuf.GetWidth();
	lightMapCreateInfo.height = imgBuf.GetHeight();
	lightMapCreateInfo.format = prosper::Format::R16G16B16A16_SFloat;
	lightMapCreateInfo.postCreateLayout = prosper::ImageLayout::TransferSrcOptimal;
	lightMapCreateInfo.usage = prosper::ImageUsageFlags::TransferSrcBit;
	lightMapCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::HostCoherent | prosper::MemoryFeatureFlags::HostAccessable;
	lightMapCreateInfo.tiling = prosper::ImageTiling::Linear;
	auto imgStaging = get_cengine()->GetRenderContext().CreateImage(lightMapCreateInfo, static_cast<const uint8_t *>(imgBuf.GetData()));

	lightMapCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	lightMapCreateInfo.tiling = prosper::ImageTiling::Optimal;
	lightMapCreateInfo.postCreateLayout = prosper::ImageLayout::TransferDstOptimal;
	lightMapCreateInfo.usage = prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::TransferDstBit;
	auto img = get_cengine()->GetRenderContext().CreateImage(lightMapCreateInfo);

	auto &setupCmd = get_cengine()->GetSetupCommandBuffer();
	if(setupCmd->RecordBlitImage({}, *imgStaging, *img) == false)
		; // TODO: Print warning
	setupCmd->RecordImageBarrier(*img, prosper::ImageLayout::TransferDstOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
	get_cengine()->FlushSetupCommandBuffer();

	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	samplerCreateInfo.minFilter = prosper::Filter::Linear;
	samplerCreateInfo.magFilter = prosper::Filter::Linear;
	samplerCreateInfo.addressModeU = prosper::SamplerAddressMode::ClampToEdge; // Doesn't really matter since lightmaps have their own border either way
	samplerCreateInfo.addressModeV = prosper::SamplerAddressMode::ClampToEdge;
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	imgViewCreateInfo.swizzleAlpha = prosper::ComponentSwizzle::One; // We don't use the alpha channel
	return get_cengine()->GetRenderContext().CreateTexture({}, *img, imgViewCreateInfo, samplerCreateInfo);
}

static void generate_lightmap_uv_atlas(ecs::BaseEntity &ent, uint32_t width, uint32_t height, const std::function<void(bool)> &callback)
{
	Con::COUT << "Generating lightmap uv atlas... This may take a few minutes!" << Con::endl;
	auto lightmapC = ent.GetComponent<CLightMapComponent>();
	auto mdl = ent.GetModel();
	auto meshGroup = mdl ? mdl->GetMeshGroup(0) : nullptr;
	if(meshGroup == nullptr || lightmapC.expired()) {
		callback(false);
		return;
	}
	uint32_t numVerts = 0;
	uint32_t numTris = 0;
	// Count the number of primitives so we can pre-allocate the data properly
	for(auto &mesh : meshGroup->GetMeshes()) {
		for(auto &subMesh : mesh->GetSubMeshes()) {
			if(subMesh->GetUVSet("lightmap") == nullptr)
				continue;
			numVerts += subMesh->GetVertexCount();
			numTris += subMesh->GetTriangleCount();
		}
	}

	// Populate data vectors
	std::vector<math::Vertex> verts {};
	std::vector<uint32_t> tris {};
	verts.reserve(numVerts);
	tris.reserve(numTris * 3);
	for(auto &mesh : meshGroup->GetMeshes()) {
		for(auto &subMesh : mesh->GetSubMeshes()) {
			if(subMesh->GetUVSet("lightmap") == nullptr)
				continue;
			auto vertOffset = verts.size();
			for(auto &v : subMesh->GetVertices())
				verts.push_back(v);

			auto offset = subMesh->GetIndexCount();
			subMesh->GetIndices(tris);
			for(auto i = offset; i < subMesh->GetIndexCount(); ++i)
				subMesh->SetIndex(i, *subMesh->GetIndex(i) + vertOffset);
		}
	}

	std::vector<Vector2> newLightmapUvs {};
	newLightmapUvs.reserve(numVerts);
	auto job = pragma::util::generate_lightmap_uvs(*get_client_state(), width, height, verts, tris);
	if(job.IsValid() == false) {
		callback(false);
		return;
	}
	auto hEnt = ent.GetHandle();
	job.SetCompletionHandler([hEnt, callback](util::ParallelWorker<std::vector<Vector2> &> &worker) {
		if(worker.IsSuccessful() == false) {
			Con::CWAR << "Atlas generation failed: " << worker.GetResultMessage() << Con::endl;
			callback(false);
			return;
		}

		auto mdl = hEnt.valid() ? hEnt.get()->GetModel() : nullptr;
		auto meshGroup = mdl ? mdl->GetMeshGroup(0) : nullptr;
		auto lightmapC = hEnt.valid() ? hEnt.get()->GetComponent<CLightMapComponent>() : pragma::ComponentHandle<CLightMapComponent> {};
		if(meshGroup == nullptr || lightmapC.expired()) {
			Con::CWAR << "Resources used for atlas generation are no longer valid!" << Con::endl;
			callback(false);
			return;
		}
		Con::COUT << "Lightmap uvs generation successful!" << Con::endl;

		// Apply new lightmap uvs

		auto &newLightmapUvs = worker.GetResult();
		uint32_t vertexOffset = 0u;
		for(auto &mesh : meshGroup->GetMeshes()) {
			for(auto &subMesh : mesh->GetSubMeshes()) {
				auto *uvSet = subMesh->GetUVSet("lightmap");
				if(uvSet == nullptr)
					continue;
				auto numVerts = subMesh->GetVertexCount();
				assert(newLightmapUvs.size() >= vertexOffset + numVerts);
				for(auto i = vertexOffset; i < (vertexOffset + numVerts); ++i)
					uvSet->at(i - vertexOffset) = newLightmapUvs.at(i);
				vertexOffset += numVerts;
			}
		}
		lightmapC->UpdateLightmapUvBuffers();
		callback(true);
	});
	job.Start();
	get_cengine()->AddParallelJob(job, "Lightmap UV Atlas");
}

bool CLightMapComponent::ImportLightmapAtlas(image::ImageBuffer &imgBuffer)
{
	auto tex = CreateLightmapTexture(imgBuffer);
	/*{
	TextureManager::LoadInfo loadInfo {};
	loadInfo.flags = TextureLoadFlags::LoadInstantly;
	std::shared_ptr<void> ptrTex;
	static_cast<CMaterialManager&>(client->GetMaterialManager()).GetTextureManager().Load(*c_engine,"lightmaps_medium.dds",loadInfo,&ptrTex);
	tex = std::static_pointer_cast<material::Texture>(ptrTex)->texture;
	}*/

	image::TextureSaveInfo texSaveInfo {};
	auto &texInfo = texSaveInfo.texInfo;
	texInfo.containerFormat = image::TextureInfo::ContainerFormat::DDS;
	texInfo.inputFormat = image::TextureInfo::InputFormat::R16G16B16A16_Float;
	texInfo.outputFormat = image::TextureInfo::OutputFormat::BC6;
	texInfo.flags = image::TextureInfo::Flags::GenerateMipmaps;

	auto mapName = get_cgame()->GetMapName();
	Con::COUT << "Lightmap atlas save result: " << image::save_texture("materials/maps/" + mapName + "/lightmap_atlas.dds", imgBuffer, texSaveInfo) << Con::endl;

	ecs::EntityIterator entIt {*get_cgame()};
	entIt.AttachFilter<TEntityIteratorFilterComponent<CLightMapComponent>>();
	auto it = entIt.begin();
	if(it == entIt.end())
		return false;
	auto *ent = *it;
	auto lightmapC = ent->GetComponent<CLightMapComponent>();

	if(lightmapC.valid())
		lightmapC->SetLightMapAtlas(tex);

	// std::vector<std::string> argv {};
	// Console::commands::debug_lightmaps(client, nullptr, argv);
	return true;
}

static void generate_lightmaps(uint32_t width, uint32_t height, uint32_t sampleCount, bool denoise, bool renderJob, float exposure, float skyStrength, float globalLightIntensityFactor, const std::string &skyTex,
  const std::optional<rendering::cycles::SceneInfo::ColorTransform> &colorTransform)
{
	Con::COUT << "Baking lightmaps... This may take a few minutes!" << Con::endl;
	auto hdrOutput = true;
	rendering::cycles::SceneInfo sceneInfo {};
	sceneInfo.width = width;
	sceneInfo.height = height;
	sceneInfo.samples = sampleCount;
	sceneInfo.denoise = denoise;
	sceneInfo.hdrOutput = hdrOutput;
	sceneInfo.renderJob = renderJob;
	sceneInfo.exposure = exposure;
	sceneInfo.colorTransform = colorTransform;
	sceneInfo.device = rendering::cycles::SceneInfo::DeviceType::GPU;
	sceneInfo.globalLightIntensityFactor = globalLightIntensityFactor;

	// TODO: Replace these with command arguments?
	sceneInfo.sky = skyTex;
	sceneInfo.skyAngles = {0.f, 0.f, 0.f};
	sceneInfo.skyStrength = skyStrength;
	// sceneInfo.renderer = "luxcorerender";
	sceneInfo.renderer = "cycles";

	auto job = pragma::rendering::cycles::bake_lightmaps(*get_client_state(), sceneInfo);
	if(sceneInfo.renderJob)
		return;
	if(job.IsValid() == false) {
		Con::CWAR << "Unable to initialize cycles scene for lightmap baking!" << Con::endl;
		return;
	}
	job.SetCompletionHandler([hdrOutput](util::ParallelWorker<image::ImageLayerSet> &worker) {
		if(worker.IsSuccessful() == false) {
			Con::CWAR << "Unable to bake lightmaps: " << worker.GetResultMessage() << Con::endl;
			return;
		}

		auto imgBuffer = worker.GetResult().images.begin()->second;
		if(hdrOutput == false) {
			// No HDR output, but we'll still use HDR data
			imgBuffer->Convert(image::Format::RGBA16);
		}

		CLightMapComponent::ImportLightmapAtlas(*imgBuffer);
	});
	job.Start();
	get_cengine()->AddParallelJob(job, "Baked lightmaps");
}

bool CLightMapComponent::ImportLightmapAtlas(fs::VFilePtr fp)
{
	fs::File f {fp};
	auto imgBuf = image::load_image(f, image::PixelFormat::Float);
	if(imgBuf == nullptr)
		return false;
	return ImportLightmapAtlas(*imgBuf);
}
bool CLightMapComponent::ImportLightmapAtlas(const std::string &path)
{
	auto f = fs::open_system_file(path, fs::FileMode::Read | fs::FileMode::Binary);
	if(f == nullptr)
		return false;
	return ImportLightmapAtlas(f);
}

bool CLightMapComponent::BakeLightmaps(const LightmapBakeSettings &bakeSettings)
{
	ecs::EntityIterator entIt {*get_cgame()};
	entIt.AttachFilter<TEntityIteratorFilterComponent<CLightMapComponent>>();
	auto it = entIt.begin();
	if(it == entIt.end()) {
		Con::CWAR << "No lightmap entity found!" << Con::endl;
		return false;
	}
	auto *ent = *it;
	auto lightmapC = ent->GetComponent<CLightMapComponent>();

	//auto resolution = pragma::get_cengine()->GetRenderResolution();
	auto &lightMap = lightmapC->GetLightMap();
	Vector2i resolution {2'048, 2'048};
	if(lightMap) {
		auto extents = lightMap->GetImage().GetExtents();
		resolution = {extents.width, extents.height};
	}
	if(bakeSettings.width.has_value())
		resolution.x = *bakeSettings.width;
	if(bakeSettings.height.has_value())
		resolution.y = *bakeSettings.height;
	auto hEnt = ent->GetHandle();
	if(bakeSettings.rebuildUvAtlas) {
		generate_lightmap_uv_atlas(*ent, resolution.x, resolution.y, [hEnt, resolution, bakeSettings](bool success) {
			if(success == false || hEnt.valid() == false)
				return;
			generate_lightmaps(resolution.x, resolution.y, bakeSettings.samples, bakeSettings.denoise, bakeSettings.createAsRenderJob, bakeSettings.exposure, bakeSettings.skyStrength, bakeSettings.globalLightIntensityFactor, bakeSettings.sky, bakeSettings.colorTransform);
		});
		return true;
	}
	generate_lightmaps(resolution.x, resolution.y, bakeSettings.samples, bakeSettings.denoise, bakeSettings.createAsRenderJob, bakeSettings.exposure, bakeSettings.skyStrength, bakeSettings.globalLightIntensityFactor, bakeSettings.sky, bakeSettings.colorTransform);
	return true;
}

static void map_rebuild_lightmaps(NetworkState *state, BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	std::unordered_map<std::string, console::CommandOption> commandOptions {};
	pragma::console::parse_command_options(argv, commandOptions);

	auto width = pragma::console::get_command_option_parameter_value(commandOptions, "width", "");
	auto height = pragma::console::get_command_option_parameter_value(commandOptions, "height", "");
	LightmapBakeSettings bakeSettings {};
	if(width.empty() == false)
		bakeSettings.width = util::to_uint(width);
	if(height.empty() == false)
		bakeSettings.height = util::to_uint(height);
	bakeSettings.exposure = util::to_float(pragma::console::get_command_option_parameter_value(commandOptions, "exposure", "50"));
	bakeSettings.skyStrength = util::to_float(pragma::console::get_command_option_parameter_value(commandOptions, "sky_strength", "0.3"));
	bakeSettings.samples = util::to_uint(pragma::console::get_command_option_parameter_value(commandOptions, "samples", "1225"));
	bakeSettings.denoise = util::to_boolean(pragma::console::get_command_option_parameter_value(commandOptions, "denoise", "1"));
	bakeSettings.createAsRenderJob = util::to_boolean(pragma::console::get_command_option_parameter_value(commandOptions, "render_job", "0"));
	bakeSettings.globalLightIntensityFactor = util::to_float(pragma::console::get_command_option_parameter_value(commandOptions, "light_intensity_factor", "1"));
	auto itRebuildUvAtlas = commandOptions.find("rebuild_uv_atlas");
	bakeSettings.rebuildUvAtlas = (itRebuildUvAtlas != commandOptions.end());
	bakeSettings.colorTransform = rendering::cycles::SceneInfo::ColorTransform {};
	bakeSettings.colorTransform->config = "filmic-blender";
	bakeSettings.colorTransform->look = "Medium Contrast";
	CLightMapComponent::BakeLightmaps(bakeSettings);
}
namespace {
	auto UVN = console::client::register_command("map_rebuild_lightmaps", &map_rebuild_lightmaps, console::ConVarFlags::None, "Rebuilds the lightmaps for the current map. Note that this will only work if the map was compiled with lightmap uvs.");
}

static void set_lightmap_texture(lua::State *l, CLightMapComponent &hLightMapC, const std::string &path, bool directional)
{
	auto *nw = get_cengine()->GetNetworkState(l);

	auto &texManager = static_cast<material::CMaterialManager &>(static_cast<ClientState *>(nw)->GetMaterialManager()).GetTextureManager();
	auto texture = texManager.LoadAsset(path);
	if(texture == nullptr)
		return;
	auto &vkTex = std::static_pointer_cast<material::Texture>(texture)->GetVkTexture();
	if(vkTex == nullptr)
		return;
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	auto sampler = get_cengine()->GetRenderContext().CreateSampler(samplerCreateInfo);
	vkTex->SetSampler(*sampler);
	if(directional)
		hLightMapC.SetDirectionalLightMapAtlas(vkTex);
	else
		hLightMapC.SetLightMapAtlas(vkTex);
}

void CLightMapComponent::RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts)
{
	auto defCLightMap = pragma::LuaCore::create_entity_component_class<CLightMapComponent, BaseEntityComponent>("LightMapComponent");
	defCLightMap.add_static_constant("TEXTURE_DIFFUSE", math::to_integral(Texture::DiffuseMap));
	defCLightMap.add_static_constant("TEXTURE_DIFFUSE_DIRECT", math::to_integral(Texture::DiffuseDirectMap));
	defCLightMap.add_static_constant("TEXTURE_DIFFUSE_INDIRECT", math::to_integral(Texture::DiffuseIndirectMap));
	defCLightMap.add_static_constant("TEXTURE_DOMINANT_DIRECTION", math::to_integral(Texture::DominantDirectionMap));
	defCLightMap.add_static_constant("TEXTURE_COUNT", math::to_integral(Texture::Count));

	defCLightMap.scope[luabind::def("bake_lightmaps", &CLightMapComponent::BakeLightmaps)];
	defCLightMap.scope[luabind::def("import_lightmap_atlas", static_cast<bool (*)(const std::string &)>(&CLightMapComponent::ImportLightmapAtlas))];
	defCLightMap.scope[luabind::def("import_lightmap_atlas", static_cast<bool (*)(image::ImageBuffer &)>(&CLightMapComponent::ImportLightmapAtlas))];
	//defCLightMap.scope[luabind::def("import_lightmap_atlas",static_cast<bool(*)(fs::VFilePtr)>(&pragma::CLightMapComponent::ImportLightmapAtlas),luabind::file_policy<1>{})];
	defCLightMap.def("GetLightmapTexture", static_cast<std::optional<std::shared_ptr<prosper::Texture>> (*)(lua::State *, CLightMapComponent &)>([](lua::State *l, CLightMapComponent &hLightMapC) -> std::optional<std::shared_ptr<prosper::Texture>> {
		auto lightMap = hLightMapC.GetLightMap();
		if(lightMap == nullptr)
			return {};
		return lightMap;
	}));
	defCLightMap.def(
	  "GetDirectionalLightmapTexture", +[](lua::State *l, CLightMapComponent &hLightMapC) -> std::optional<std::shared_ptr<prosper::Texture>> {
		  auto lightMap = hLightMapC.GetDirectionalLightMap();
		  if(lightMap == nullptr)
			  return {};
		  return lightMap;
	  });
	defCLightMap.def("GetLightmapMaterialName", &CLightMapComponent::GetLightMapMaterialName);
	defCLightMap.def("SetLightmapMaterial", &CLightMapComponent::SetLightMapMaterial);
	defCLightMap.def("ConvertLightmapToBSPLuxelData", &CLightMapComponent::ConvertLightmapToBSPLuxelData);
	defCLightMap.def("UpdateLightmapUvBuffers", &CLightMapComponent::UpdateLightmapUvBuffers);
	defCLightMap.def("ReloadLightmapData", &CLightMapComponent::ReloadLightMapData);
	defCLightMap.def("GetLightmapAtlas", &CLightMapComponent::GetLightMapAtlas);
	defCLightMap.def("GetDirectionalLightmapAtlas", &CLightMapComponent::GetDirectionalLightMapAtlas);
	defCLightMap.def("GetLightmapTexture", &CLightMapComponent::GetTexture, luabind::copy_policy<0> {});
	defCLightMap.def("SetLightmapAtlas", &CLightMapComponent::SetLightMapAtlas);
	defCLightMap.def("SetLightmapAtlas", +[](lua::State *l, CLightMapComponent &hLightMapC, const std::string &path) { set_lightmap_texture(l, hLightMapC, path, false); });
	defCLightMap.def("SetDirectionalLightmapAtlas", &CLightMapComponent::SetDirectionalLightMapAtlas);
	defCLightMap.def("SetDirectionalLightmapAtlas", +[](lua::State *l, CLightMapComponent &hLightMapC, const std::string &path) { set_lightmap_texture(l, hLightMapC, path, true); });
	defCLightMap.def("SetExposure", &CLightMapComponent::SetLightMapExposure);
	defCLightMap.def("GetExposure", &CLightMapComponent::GetLightMapExposure);
	defCLightMap.def("GetExposureProperty", &CLightMapComponent::GetLightMapExposureProperty);

	auto defLightmapBakeSettings = luabind::class_<LightmapBakeSettings>("BakeSettings");
	defLightmapBakeSettings.def(luabind::constructor<>());
	defLightmapBakeSettings.property("width",
	  static_cast<luabind::object (*)(lua::State *, LightmapBakeSettings &)>([](lua::State *l, LightmapBakeSettings &bakeSettings) -> luabind::object { return bakeSettings.width.has_value() ? luabind::object {l, *bakeSettings.width} : luabind::object {}; }),
	  static_cast<void (*)(lua::State *, LightmapBakeSettings &, luabind::object)>([](lua::State *l, LightmapBakeSettings &bakeSettings, luabind::object o) {
		  if(Lua::IsSet(l, 2) == false) {
			  bakeSettings.width = {};
			  return;
		  }
		  bakeSettings.width = Lua::CheckNumber(l, 2);
	  }));
	defLightmapBakeSettings.property("height",
	  static_cast<luabind::object (*)(lua::State *, LightmapBakeSettings &)>([](lua::State *l, LightmapBakeSettings &bakeSettings) -> luabind::object { return bakeSettings.height.has_value() ? luabind::object {l, *bakeSettings.height} : luabind::object {}; }),
	  static_cast<void (*)(lua::State *, LightmapBakeSettings &, luabind::object)>([](lua::State *l, LightmapBakeSettings &bakeSettings, luabind::object o) {
		  if(Lua::IsSet(l, 2) == false) {
			  bakeSettings.height = {};
			  return;
		  }
		  bakeSettings.height = Lua::CheckNumber(l, 2);
	  }));
	defLightmapBakeSettings.def_readwrite("samples", &LightmapBakeSettings::samples);
	defLightmapBakeSettings.def_readwrite("globalLightIntensityFactor", &LightmapBakeSettings::globalLightIntensityFactor);
	defLightmapBakeSettings.def_readwrite("denoise", &LightmapBakeSettings::denoise);
	defLightmapBakeSettings.def_readwrite("createAsRenderJob", &LightmapBakeSettings::createAsRenderJob);
	defLightmapBakeSettings.def_readwrite("rebuildUvAtlas", &LightmapBakeSettings::rebuildUvAtlas);
	defLightmapBakeSettings.def_readwrite("exposure", &LightmapBakeSettings::exposure);
	defLightmapBakeSettings.def("SetColorTransform", static_cast<void (*)(lua::State *, LightmapBakeSettings &, const std::string &, const std::string &)>([](lua::State *l, LightmapBakeSettings &bakeSettings, const std::string &config, const std::string &look) {
		bakeSettings.colorTransform = rendering::cycles::SceneInfo::ColorTransform {};
		bakeSettings.colorTransform->config = config;
		bakeSettings.colorTransform->look = look;
	}));
	defLightmapBakeSettings.def("ResetColorTransform", static_cast<void (*)(lua::State *, LightmapBakeSettings &)>([](lua::State *l, LightmapBakeSettings &bakeSettings) { bakeSettings.colorTransform = {}; }));
	defCLightMap.scope[defLightmapBakeSettings];

	auto defCache = luabind::class_<rendering::LightmapDataCache>("DataCache");
	defCache.scope[luabind::def(
	  "load", +[](lua::State *l, const std::string &path) -> Lua::var<rendering::LightmapDataCache, std::pair<bool, std::string>> {
		  auto cache = pragma::util::make_shared<rendering::LightmapDataCache>();
		  std::string err;
		  if(!rendering::LightmapDataCache::Load(path, *cache, err))
			  return luabind::object {l, std::pair<bool, std::string> {false, err}};
		  return luabind::object {l, cache};
	  })];
	defCache.def(
	  "AddInstanceData", +[](rendering::LightmapDataCache &cache, const std::string &entUuid, const std::string &model, const math::Transform &pose, const std::string &meshUuid, const std::vector<Vector2> &uvs) {
		  auto tmpUvs = uvs;
		  cache.AddInstanceData(util::uuid_string_to_bytes(entUuid), model, pose, util::uuid_string_to_bytes(meshUuid), std::move(tmpUvs));
	  });
	defCache.def(
	  "GetInstanceIds", +[](rendering::LightmapDataCache &cache) -> std::vector<std::string> {
		  std::vector<std::string> uuids;
		  uuids.reserve(cache.cacheData.size());
		  for(auto &pair : cache.cacheData)
			  uuids.push_back(util::uuid_to_string(pair.first.uuid));
		  return uuids;
	  });
	defCache.def(
	  "GetInstancePose", +[](rendering::LightmapDataCache &cache, const std::string &uuid) -> std::optional<math::Transform> {
		  auto it = cache.cacheData.find(rendering::LmUuid {util::uuid_string_to_bytes(uuid)});
		  if(it == cache.cacheData.end())
			  return {};
		  return it->second.pose;
	  });
	defCache.def(
	  "FindLightmapUvs", +[](rendering::LightmapDataCache &cache, const std::string &entUuid, const std::string &meshUuid) -> std::optional<std::vector<Vector2>> {
		  auto *uvs = cache.FindLightmapUvs(util::uuid_string_to_bytes(entUuid), util::uuid_string_to_bytes(meshUuid));
		  if(!uvs)
			  return {};
		  return *uvs;
	  });
	defCache.def(
	  "SaveAs", +[](lua::State *l, rendering::LightmapDataCache &cache, const std::string &path) -> Lua::var<bool, std::pair<bool, std::string>> {
		  std::string err;
		  auto res = cache.SaveAs(path, err);
		  if(res)
			  return luabind::object {l, res};
		  return luabind::object {l, std::pair<bool, std::string> {res, err}};
	  });
	defCache.def("SetLightmapEntity", +[](lua::State *l, rendering::LightmapDataCache &cache, const std::string &uuid) { cache.lightmapEntityId = util::uuid_string_to_bytes(uuid); });
	defCache.def("GetLightmapEntity", +[](lua::State *l, rendering::LightmapDataCache &cache) -> std::string { return util::uuid_to_string(cache.lightmapEntityId); });
	defCLightMap.scope[defCache];
	modEnts[defCLightMap];
	pragma::LuaCore::define_custom_constructor<rendering::LightmapDataCache, +[]() -> std::shared_ptr<rendering::LightmapDataCache> { return pragma::util::make_shared<rendering::LightmapDataCache>(); }>(l);

	auto defCLightMapReceiver = pragma::LuaCore::create_entity_component_class<CLightMapReceiverComponent, BaseEntityComponent>("LightMapReceiverComponent");
	defCLightMapReceiver.def("UpdateLightmapUvData", &CLightMapReceiverComponent::UpdateLightMapUvData);
	modEnts[defCLightMapReceiver];

	auto defCLmCache = pragma::LuaCore::create_entity_component_class<CLightMapDataCacheComponent, BaseEntityComponent>("LightMapDataCacheComponent");
	defCLmCache.def("SetLightMapDataCachePath", &CLightMapDataCacheComponent::SetLightMapDataCachePath);
	defCLmCache.def("GetLightMapDataCachePath", &CLightMapDataCacheComponent::GetLightMapDataCachePath);
	defCLmCache.def("GetLightMapDataCacheFilePath", +[](const CLightMapDataCacheComponent &component) -> std::string { return rendering::LightmapDataCache::GetCacheFileName(component.GetLightMapDataCachePath()); });
	defCLmCache.def("GetLightMapDataCache", &CLightMapDataCacheComponent::GetLightMapDataCache);
	defCLmCache.def("ReloadCache", &CLightMapDataCacheComponent::ReloadCache);
	modEnts[defCLmCache];
}
