/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_light_map_component.hpp"
#include "pragma/console/c_cvar_global_functions.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/vk_mesh.h"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/raytracing/cycles.hpp"
#include "pragma/gui/wiframe.h"
#include <pragma/util/util_tga.hpp>
#include <pragma/level/mapgeometry.h>
#include <pragma/entities/entity_iterator.hpp>
#include <GuillotineBinPack.h>
#include <pragma/entities/entity_component_system_t.hpp>
#include <prosper_util.hpp>
#include <buffers/prosper_dynamic_resizable_buffer.hpp>
#include <prosper_command_buffer.hpp>
#include <image/prosper_sampler.hpp>
#include <util_image_buffer.hpp>
#include <util_image.hpp>
#include <util_texture_info.hpp>
#include <pragma/console/command_options.hpp>
#include <pragma/util/util_game.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;
extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

CLightMapComponent::CLightMapComponent(BaseEntity &ent)
	: BaseEntityComponent(ent),m_lightMapExposure{util::FloatProperty::Create(0.f)}
{}

luabind::object CLightMapComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CLightMapComponentHandleWrapper>(l);}
void CLightMapComponent::Initialize()
{
	BaseEntityComponent::Initialize();
}

void CLightMapComponent::InitializeLightMapData(
	const std::shared_ptr<prosper::Texture> &lightMap,
	const std::shared_ptr<prosper::IDynamicResizableBuffer> &lightMapUvBuffer,
	const std::vector<std::shared_ptr<prosper::IBuffer>> &meshUvBuffers
)
{
	m_lightMapAtlas = lightMap;
	m_meshLightMapUvBuffer = lightMapUvBuffer;
	m_meshLightMapUvBuffers = meshUvBuffers;

	EntityIterator entIt {*c_game,EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CLightMapReceiverComponent>>();
	for(auto *ent : entIt)
	{
		auto lightMapReceiverC = ent->GetComponent<pragma::CLightMapReceiverComponent>();
		lightMapReceiverC->UpdateMeshLightmapUvBuffers(*this);
	}
}

void CLightMapComponent::SetLightMapAtlas(const std::shared_ptr<prosper::Texture> &lightMap)
{
	m_lightMapAtlas = lightMap;

	// TODO: This method only allows one lightmap atlas globally; Implement this in a way that allows multiple (maybe add to entity descriptor set?)!
	pragma::rendering::RasterizationRenderer::UpdateLightmap(lightMap);
}

void CLightMapComponent::ReloadLightMapData()
{
	std::vector<std::shared_ptr<prosper::IBuffer>> buffers {};
	auto globalLightmapUvBuffer = pragma::CLightMapComponent::GenerateLightmapUVBuffers(buffers);
	InitializeLightMapData(m_lightMapAtlas,globalLightmapUvBuffer,buffers);
	UpdateLightmapUvBuffers();
}

const std::shared_ptr<prosper::Texture> &CLightMapComponent::GetLightMap() const {return m_lightMapAtlas;}

prosper::IBuffer *CLightMapComponent::GetMeshLightMapUvBuffer(uint32_t meshIdx) const
{
	if(meshIdx >= m_meshLightMapUvBuffers.size())
		return nullptr;
	return m_meshLightMapUvBuffers.at(meshIdx).get();
}
std::shared_ptr<prosper::IDynamicResizableBuffer> CLightMapComponent::GetGlobalLightMapUvBuffer() const {return m_meshLightMapUvBuffer;}

const std::vector<std::shared_ptr<prosper::IBuffer>> &CLightMapComponent::GetMeshLightMapUvBuffers() const {return const_cast<CLightMapComponent*>(this)->GetMeshLightMapUvBuffers();}
std::vector<std::shared_ptr<prosper::IBuffer>> &CLightMapComponent::GetMeshLightMapUvBuffers() {return m_meshLightMapUvBuffers;}

void CLightMapComponent::SetLightMapExposure(float exp) {*m_lightMapExposure = exp;}
float CLightMapComponent::GetLightMapExposure() const {return *m_lightMapExposure;}

void CLightMapComponent::UpdateLightmapUvBuffers()
{
	// TODO: Move this function to light map receiver component?
	auto uvBuffer = GetGlobalLightMapUvBuffer();
	auto mdl = GetEntity().GetModel();
	auto meshGroup = mdl ? mdl->GetMeshGroup(0) : nullptr;
	if(uvBuffer == nullptr || meshGroup == nullptr || uvBuffer->Map(0ull,uvBuffer->GetSize(),prosper::IBuffer::MapFlags::WriteBit) == false)
		return;
	auto &uvBuffers = GetMeshLightMapUvBuffers();
	EntityIterator entIt {*c_game,EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CLightMapReceiverComponent>>();
	for(auto *ent : entIt)
	{
		auto lightMapReceiverC = ent->GetComponent<pragma::CLightMapReceiverComponent>();
		for(auto &mesh : meshGroup->GetMeshes())
		{
			for(auto &subMesh : mesh->GetSubMeshes())
			{
				auto *uvSet = subMesh->GetUVSet("lightmap");
				auto bufIdx = lightMapReceiverC->FindBufferIndex(static_cast<CModelSubMesh&>(*subMesh));
				if(uvSet == nullptr || bufIdx.has_value() == false)
					continue;
				uvBuffers.at(*bufIdx)->Write(0,uvSet->size() *sizeof(uvSet->front()),uvSet->data());
			}
		}
	}
	uvBuffer->Unmap();
}

std::shared_ptr<prosper::IDynamicResizableBuffer> CLightMapComponent::GenerateLightmapUVBuffers(std::vector<std::shared_ptr<prosper::IBuffer>> &outMeshLightMapUvBuffers)
{
	prosper::util::BufferCreateInfo bufCreateInfo {};
	bufCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	bufCreateInfo.usageFlags = prosper::BufferUsageFlags::VertexBufferBit | prosper::BufferUsageFlags::TransferSrcBit | prosper::BufferUsageFlags::TransferDstBit; // Transfer flags are required for mapping GPUBulk buffers
	auto alignment = c_engine->GetRenderContext().CalcBufferAlignment(bufCreateInfo.usageFlags);
	auto requiredBufferSize = 0ull;

	// Collect all meshes that have lightmap uv coordinates
	EntityIterator entIt {*c_game,EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CLightMapReceiverComponent>>();

	// Calculate required buffer size
	uint32_t numMeshes = 0;
	for(auto *ent : entIt)
	{
		auto lightMapReceiverC = ent->GetComponent<pragma::CLightMapReceiverComponent>();
		auto &meshLightMapUvData = lightMapReceiverC->GetMeshLightMapUvData();
		for(auto &pair : meshLightMapUvData)
		{
			auto &uvSet = pair.second;
			requiredBufferSize += prosper::util::get_aligned_size(uvSet.size() *sizeof(uvSet.front()),alignment);
			++numMeshes;
		}
	}

	// Generate the lightmap uv buffer
	bufCreateInfo.size = requiredBufferSize;
	auto lightMapUvBuffer = c_engine->GetRenderContext().CreateDynamicResizableBuffer(bufCreateInfo,bufCreateInfo.size,0.2f);
	if(lightMapUvBuffer == nullptr || lightMapUvBuffer->Map(0ull,lightMapUvBuffer->GetSize(),prosper::IBuffer::MapFlags::WriteBit) == false)
		return nullptr;

	outMeshLightMapUvBuffers.reserve(numMeshes);
	uint32_t bufIdx = 0;
	for(auto *ent : entIt)
	{
		auto lightMapReceiverC = ent->GetComponent<pragma::CLightMapReceiverComponent>();
		auto &meshLightMapUvData = lightMapReceiverC->GetMeshLightMapUvData();
		for(auto &pair : meshLightMapUvData)
		{
			auto &uvSet = pair.second;
			auto subBuf = lightMapUvBuffer->AllocateBuffer(uvSet.size() *sizeof(uvSet.front()));
			outMeshLightMapUvBuffers.push_back(subBuf);

			lightMapUvBuffer->Write(subBuf->GetStartOffset(),uvSet.size() *sizeof(uvSet.front()),uvSet.data());
			lightMapReceiverC->AssignBufferIndex(pair.first,bufIdx++);
		}
	}
	lightMapUvBuffer->Unmap();
	return lightMapUvBuffer;
}

std::shared_ptr<prosper::Texture> CLightMapComponent::CreateLightmapTexture(uint32_t width,uint32_t height,const uint16_t *hdrPixelData)
{
	prosper::util::ImageCreateInfo lightMapCreateInfo {};
	lightMapCreateInfo.width = width;
	lightMapCreateInfo.height = height;
	lightMapCreateInfo.format = prosper::Format::R16G16B16A16_SFloat;
	lightMapCreateInfo.postCreateLayout = prosper::ImageLayout::TransferSrcOptimal;
	lightMapCreateInfo.usage = prosper::ImageUsageFlags::TransferSrcBit;
	lightMapCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::HostCoherent | prosper::MemoryFeatureFlags::HostAccessable;
	lightMapCreateInfo.tiling = prosper::ImageTiling::Linear;
	auto imgStaging = c_engine->GetRenderContext().CreateImage(lightMapCreateInfo,reinterpret_cast<const uint8_t*>(hdrPixelData));

	lightMapCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	lightMapCreateInfo.tiling = prosper::ImageTiling::Optimal;
	lightMapCreateInfo.postCreateLayout = prosper::ImageLayout::TransferDstOptimal;
	lightMapCreateInfo.usage = prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::TransferDstBit;
	auto img = c_engine->GetRenderContext().CreateImage(lightMapCreateInfo);

	auto &setupCmd = c_engine->GetSetupCommandBuffer();
	if(setupCmd->RecordBlitImage({},*imgStaging,*img) == false)
		; // TODO: Print warning
	setupCmd->RecordImageBarrier(*img,prosper::ImageLayout::TransferDstOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal);
	c_engine->FlushSetupCommandBuffer();

	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	samplerCreateInfo.minFilter = prosper::Filter::Linear;
	samplerCreateInfo.magFilter = prosper::Filter::Linear;
	samplerCreateInfo.addressModeU = prosper::SamplerAddressMode::ClampToEdge; // Doesn't really matter since lightmaps have their own border either way
	samplerCreateInfo.addressModeV = prosper::SamplerAddressMode::ClampToEdge;
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	imgViewCreateInfo.swizzleAlpha = prosper::ComponentSwizzle::One; // We don't use the alpha channel
	return c_engine->GetRenderContext().CreateTexture({},*img,imgViewCreateInfo,samplerCreateInfo);
}

#include "cmaterialmanager.h"
#include <wgui/types/wirect.h>
static void generate_lightmap_uv_atlas(BaseEntity &ent,uint32_t width,uint32_t height,const std::function<void(bool)> &callback)
{
	Con::cout<<"Generating lightmap uv atlas... This may take a few minutes!"<<Con::endl;
	auto lightmapC = ent.GetComponent<pragma::CLightMapComponent>();
	auto mdl = ent.GetModel();
	auto meshGroup = mdl ? mdl->GetMeshGroup(0) : nullptr;
	if(meshGroup == nullptr || lightmapC.expired())
	{
		callback(false);
		return;
	}
	uint32_t numVerts = 0;
	uint32_t numTris = 0;
	// Count the number of primitives so we can pre-allocate the data properly
	for(auto &mesh : meshGroup->GetMeshes())
	{
		for(auto &subMesh : mesh->GetSubMeshes())
		{
			if(subMesh->GetUVSet("lightmap") == nullptr)
				continue;
			numVerts += subMesh->GetVertexCount();
			numTris += subMesh->GetTriangleCount();
		}
	}

	// Populate data vectors
	std::vector<Vertex> verts {};
	std::vector<uint32_t> tris {};
	verts.reserve(numVerts);
	tris.reserve(numTris *3);
	for(auto &mesh : meshGroup->GetMeshes())
	{
		for(auto &subMesh : mesh->GetSubMeshes())
		{
			if(subMesh->GetUVSet("lightmap") == nullptr)
				continue;
			auto vertOffset = verts.size();
			for(auto &v : subMesh->GetVertices())
				verts.push_back(v);

			for(auto idx : subMesh->GetTriangles())
				tris.push_back(vertOffset +idx);
		}
	}

	std::vector<Vector2> newLightmapUvs {};
	newLightmapUvs.reserve(numVerts);
	auto job = util::generate_lightmap_uvs(*client,width,height,verts,tris);
	if(job.IsValid() == false)
	{
		callback(false);
		return;
	}
	auto hEnt = ent.GetHandle();
	job.SetCompletionHandler([hEnt,callback](util::ParallelWorker<std::vector<Vector2>&> &worker) {
		if(worker.IsSuccessful() == false)
		{
			Con::cwar<<"WARNING: Atlas generation failed: "<<worker.GetResultMessage()<<Con::endl;
			callback(false);
			return;
		}

		auto mdl = hEnt.IsValid() ? hEnt.get()->GetModel() : nullptr;
		auto meshGroup = mdl ? mdl->GetMeshGroup(0) : nullptr;
		auto lightmapC = hEnt.IsValid() ? hEnt.get()->GetComponent<pragma::CLightMapComponent>() : util::WeakHandle<pragma::CLightMapComponent>{};
		if(meshGroup == nullptr || lightmapC.expired())
		{
			Con::cwar<<"WARNING: Resources used for atlas generation are no longer valid!"<<Con::endl;
			callback(false);
			return;
		}
		Con::cout<<"Lightmap uvs generation successful!"<<Con::endl;

		// Apply new lightmap uvs

		auto &newLightmapUvs = worker.GetResult();
		uint32_t vertexOffset = 0u;
		for(auto &mesh : meshGroup->GetMeshes())
		{
			for(auto &subMesh : mesh->GetSubMeshes())
			{
				auto *uvSet = subMesh->GetUVSet("lightmap");
				if(uvSet == nullptr)
					continue;
				auto numVerts = subMesh->GetVertexCount();
				assert(newLightmapUvs.size() >= vertexOffset +numVerts);
				for(auto i=vertexOffset;i<(vertexOffset +numVerts);++i)
					uvSet->at(i -vertexOffset) = newLightmapUvs.at(i);
				vertexOffset += numVerts;
			}
		}
		lightmapC->UpdateLightmapUvBuffers();
		callback(true);
	});
	job.Start();
	c_engine->AddParallelJob(job,"Lightmap UV Atlas");
}

static void generate_lightmaps(uint32_t width,uint32_t height,uint32_t sampleCount,bool denoise,bool renderJob)
{
	Con::cout<<"Baking lightmaps... This may take a few minutes!"<<Con::endl;
	auto hdrOutput = true;
	pragma::rendering::cycles::SceneInfo sceneInfo {};
	sceneInfo.width = width;
	sceneInfo.height = height;
	sceneInfo.samples = sampleCount;
	sceneInfo.denoise = denoise;
	sceneInfo.hdrOutput = hdrOutput;
	sceneInfo.renderJob = renderJob;
	sceneInfo.device = pragma::rendering::cycles::SceneInfo::DeviceType::GPU;

	// TODO: Replace these with command arguments?
	sceneInfo.sky = "skies/dusk379.hdr";
	sceneInfo.skyAngles = {0.f,0.f,0.f};
	sceneInfo.skyStrength = 0.3f;

	auto job = pragma::rendering::cycles::bake_lightmaps(*client,sceneInfo);
	if(sceneInfo.renderJob)
		return;
	if(job.IsValid() == false)
	{
		Con::cwar<<"WARNING: Unable to initialize cycles scene for lightmap baking!"<<Con::endl;
		return;
	}
	job.SetCompletionHandler([hdrOutput](util::ParallelWorker<std::shared_ptr<uimg::ImageBuffer>> &worker) {
		if(worker.IsSuccessful() == false)
		{
			Con::cwar<<"WARNING: Unable to bake lightmaps: "<<worker.GetResultMessage()<<Con::endl;
			return;
		}

		auto imgBuffer = worker.GetResult();
		if(hdrOutput == false)
		{
			// No HDR output, but we'll still use HDR data
			imgBuffer->Convert(uimg::ImageBuffer::Format::RGBA16);
		}

		auto tex = CLightMapComponent::CreateLightmapTexture(imgBuffer->GetWidth(),imgBuffer->GetHeight(),reinterpret_cast<uint16_t*>(imgBuffer->GetData()));
		/*{
		TextureManager::LoadInfo loadInfo {};
		loadInfo.flags = TextureLoadFlags::LoadInstantly;
		std::shared_ptr<void> ptrTex;
		static_cast<CMaterialManager&>(client->GetMaterialManager()).GetTextureManager().Load(*c_engine,"lightmaps_medium.dds",loadInfo,&ptrTex);
		tex = std::static_pointer_cast<Texture>(ptrTex)->texture;
		}*/

		uimg::TextureInfo texInfo {};
		texInfo.containerFormat = uimg::TextureInfo::ContainerFormat::DDS;
		texInfo.inputFormat = uimg::TextureInfo::InputFormat::R16G16B16A16_Float;
		texInfo.outputFormat = uimg::TextureInfo::OutputFormat::BC6;
		texInfo.flags = uimg::TextureInfo::Flags::GenerateMipmaps;
	//	auto f = FileManager::OpenFile<VFilePtrReal>("materials/maps/sfm_gtav_mp_apa_06/lightmap_atlas.dds","wb");
	//	if(f)
		auto mapName = c_game->GetMapName();
		Con::cout<<"Lightmap atlas save result: "<<uimg::save_texture("materials/maps/" +mapName +"/lightmap_atlas.dds",*imgBuffer,texInfo,false)<<Con::endl;

		EntityIterator entIt {*c_game};
		entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CLightMapComponent>>();
		auto it = entIt.begin();
		if(it == entIt.end())
			return;
		auto *ent = *it;
		auto lightmapC = ent->GetComponent<pragma::CLightMapComponent>();

		if(lightmapC.valid())
			lightmapC->SetLightMapAtlas(tex);

		{
			auto &wgui = WGUI::GetInstance();
			auto *pRect = wgui.Create<WITexturedRect>();
			pRect->SetSize(512,512);
			pRect->SetTexture(*tex);
		}
	});
	job.Start();
	c_engine->AddParallelJob(job,"Baked lightmaps");
}

bool CLightMapComponent::BakeLightmaps(const LightmapBakeSettings &bakeSettings)
{
	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CLightMapComponent>>();
	auto it = entIt.begin();
	if(it == entIt.end())
	{
		Con::cwar<<"WARNING: No lightmap entity found!"<<Con::endl;
		return false;
	}
	auto *ent = *it;
	auto lightmapC = ent->GetComponent<pragma::CLightMapComponent>();

	//auto resolution = c_engine->GetRenderResolution();
	auto &lightMap = lightmapC->GetLightMap();
	Vector2i resolution {2'048,2'048};
	if(lightMap)
	{
		auto extents = lightMap->GetImage().GetExtents();
		resolution = {extents.width,extents.height};
	}
	if(bakeSettings.width.has_value())
		resolution.x = *bakeSettings.width;
	if(bakeSettings.height.has_value())
		resolution.y = *bakeSettings.height;
	auto hEnt = ent->GetHandle();
	if(bakeSettings.rebuildUvAtlas)
	{
		generate_lightmap_uv_atlas(*ent,resolution.x,resolution.y,[hEnt,resolution,bakeSettings](bool success) {
			if(success == false || hEnt.IsValid() == false)
				return;
			generate_lightmaps(resolution.x,resolution.y,bakeSettings.samples,bakeSettings.denoise,bakeSettings.createAsRenderJob);
		});
		return true;
	}
	generate_lightmaps(resolution.x,resolution.y,bakeSettings.samples,bakeSettings.denoise,bakeSettings.createAsRenderJob);
	return true;
}

void Console::commands::map_rebuild_lightmaps(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv)
{
	std::unordered_map<std::string,pragma::console::CommandOption> commandOptions {};
	pragma::console::parse_command_options(argv,commandOptions);

	auto width = pragma::console::get_command_option_parameter_value(commandOptions,"width","");
	auto height = pragma::console::get_command_option_parameter_value(commandOptions,"height","");
	CLightMapComponent::LightmapBakeSettings bakeSettings {};
	if(width.empty() == false)
		bakeSettings.width = util::to_uint(width);
	if(height.empty() == false)
		bakeSettings.height = util::to_uint(height);
	bakeSettings.samples = util::to_uint(pragma::console::get_command_option_parameter_value(commandOptions,"samples","1225"));
	bakeSettings.denoise = util::to_boolean(pragma::console::get_command_option_parameter_value(commandOptions,"denoise","1"));
	bakeSettings.createAsRenderJob = util::to_boolean(pragma::console::get_command_option_parameter_value(commandOptions,"render_job","0"));
	auto itRebuildUvAtlas = commandOptions.find("rebuild_uv_atlas");
	bakeSettings.rebuildUvAtlas = (itRebuildUvAtlas != commandOptions.end());
	CLightMapComponent::BakeLightmaps(bakeSettings);
}

void Console::commands::debug_lightmaps(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv)
{
	const std::string name = "dbg_lightmaps";
	auto &wgui = WGUI::GetInstance();
	auto *pRoot = wgui.GetBaseElement();
	auto *p = pRoot->FindDescendantByName(name);
	if(p != nullptr)
	{
		p->Remove();
		return;
	}

	if(c_game == nullptr)
		return;

	auto *scene = c_game->GetRenderScene();
	auto *renderer = scene ? scene->GetRenderer() : nullptr;
	if(renderer == nullptr || renderer->IsRasterizationRenderer() == false)
		return;
	auto &lightmap = static_cast<pragma::rendering::RasterizationRenderer*>(renderer)->GetLightMap();
	if(lightmap == nullptr)
		return;

	auto *pElContainer = wgui.Create<WIBase>();
	pElContainer->SetAutoAlignToParent(true);
	pElContainer->SetName(name);
	pElContainer->TrapFocus(true);
	pElContainer->RequestFocus();

	auto *pFrame = wgui.Create<WIFrame>(pElContainer);
	pFrame->SetTitle("Lightmap Atlas");
	auto *pLightmaps = wgui.Create<WITexturedRect>(pFrame);
	pLightmaps->SetSize(256,256);
	pLightmaps->SetY(24);
	pLightmaps->SetTexture(*lightmap);
	pFrame->SizeToContents();
	pLightmaps->SetAnchor(0.f,0.f,1.f,1.f);
}
