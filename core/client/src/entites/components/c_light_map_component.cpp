#include "stdafx_client.h"
#include "pragma/entities/components/c_light_map_component.hpp"
#include "pragma/console/c_cvar_global_functions.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
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
#include <sharedutils/util_image_buffer.hpp>
#include <pragma/console/command_options.hpp>
#include <pragma/util/util_game.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;
extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

#pragma optimize("",off)
luabind::object CLightMapComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CLightMapComponentHandleWrapper>(l);}
void CLightMapComponent::Initialize()
{
	BaseEntityComponent::Initialize();
}

void CLightMapComponent::InitializeLightMapData(
	const std::shared_ptr<util::bsp::LightMapInfo> &lightmapInfo,
	const std::shared_ptr<prosper::Texture> &lightMap,
	const std::shared_ptr<prosper::DynamicResizableBuffer> &lightMapUvBuffer,
	const std::vector<std::shared_ptr<prosper::Buffer>> &meshUvBuffers,
	const std::vector<std::vector<Vector2>> &lightmapUvs
)
{
	m_lightmapInfo = lightmapInfo;
	m_lightMapAtlas = lightMap;
	m_meshLightMapUvBuffer = lightMapUvBuffer;
	m_meshLightMapUvBuffers = meshUvBuffers;
	m_lightmapUvs = std::move(lightmapUvs);
}

const std::shared_ptr<prosper::Texture> &CLightMapComponent::GetLightMap() const {return m_lightMapAtlas;}

prosper::Buffer *CLightMapComponent::GetMeshLightMapUvBuffer(uint32_t meshIdx) const
{
	if(meshIdx >= m_meshLightMapUvBuffers.size())
		return nullptr;
	return m_meshLightMapUvBuffers.at(meshIdx).get();
}
std::shared_ptr<prosper::DynamicResizableBuffer> CLightMapComponent::GetGlobalLightMapUvBuffer() const {return m_meshLightMapUvBuffer;}

const std::vector<std::shared_ptr<prosper::Buffer>> &CLightMapComponent::GetMeshLightMapUvBuffers() const {return const_cast<CLightMapComponent*>(this)->GetMeshLightMapUvBuffers();}
std::vector<std::shared_ptr<prosper::Buffer>> &CLightMapComponent::GetMeshLightMapUvBuffers() {return m_meshLightMapUvBuffers;}

const std::vector<std::vector<Vector2>> &CLightMapComponent::GetLightmapUvs() const {return const_cast<CLightMapComponent*>(this)->GetLightmapUvs();}
std::vector<std::vector<Vector2>> &CLightMapComponent::GetLightmapUvs() {return m_lightmapUvs;}
/* Obsolete
void CLightMapComponent::ReadLightmapUvCoordinates(std::vector<std::vector<Vector2>> &uvs) const
{
	// Note: We need the lightmap uv coordinates, however these are not available on the CPU after the map has been loaded.
	// Therefore we will have to map the lightmap uv buffer instead and copy the values from the GPU.
	auto uvBuffer = GetGlobalLightMapUvBuffer();
	auto mdl = GetEntity().GetModel();
	auto meshGroup = mdl ? mdl->GetMeshGroup(0) : nullptr;
	if(uvBuffer == nullptr || meshGroup == nullptr || uvBuffer->Map(0ull,uvBuffer->GetSize()) == false)
		return;

	// Note: There is one uv buffer per sub-mesh of the model, which contains one uv-set per vertex.
	auto uvBuffers = GetMeshLightMapUvBuffers();
	uvs.reserve(uvBuffers.size());
	for(auto &mesh : meshGroup->GetMeshes())
	{
		for(auto &subMesh : mesh->GetSubMeshes())
		{
			auto refId = subMesh->GetReferenceId();
			if(refId >= uvBuffers.size())
				continue;
			auto &subBuffer = uvBuffers.at(refId);
			if(refId >= uvs.size())
				uvs.resize(refId +1);
			auto &meshUvs = uvs.at(refId);
			auto numVerts = subMesh->GetVertexCount();
			meshUvs.resize(numVerts);
			uvBuffer->Read(subBuffer->GetStartOffset(),numVerts *sizeof(Vector2),meshUvs.data());
		}
	}
	uvBuffer->Unmap();
}
*/
void CLightMapComponent::UpdateLightmapUvBuffers()
{
	auto uvBuffer = GetGlobalLightMapUvBuffer();
	auto mdl = GetEntity().GetModel();
	auto meshGroup = mdl ? mdl->GetMeshGroup(0) : nullptr;
	if(uvBuffer == nullptr || meshGroup == nullptr || m_lightmapUvs.empty() || uvBuffer->Map(0ull,uvBuffer->GetSize()) == false)
		return;
	auto &uvBuffers = GetMeshLightMapUvBuffers();
	for(auto &mesh : meshGroup->GetMeshes())
	{
		for(auto &subMesh : mesh->GetSubMeshes())
		{
			auto refId = subMesh->GetReferenceId();
			if(refId >= m_lightmapUvs.size() || refId >= uvBuffers.size())
				continue;
			auto &uvs = m_lightmapUvs.at(refId);
			uvBuffers.at(refId)->Write(0,uvs.size() *sizeof(uvs.front()),uvs.data());
		}
	}
	uvBuffer->Unmap();
}

std::shared_ptr<prosper::DynamicResizableBuffer> CLightMapComponent::LoadLightMapUvBuffers(const std::vector<std::vector<Vector2>> &meshUvData,std::vector<std::shared_ptr<prosper::Buffer>> &outMeshLightMapUvBuffers)
{
	auto &dev = c_engine->GetDevice();
	prosper::util::BufferCreateInfo bufCreateInfo {};
	bufCreateInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
	bufCreateInfo.usageFlags = Anvil::BufferUsageFlagBits::VERTEX_BUFFER_BIT | Anvil::BufferUsageFlagBits::TRANSFER_SRC_BIT | Anvil::BufferUsageFlagBits::TRANSFER_DST_BIT; // Transfer flags are required for mapping GPUBulk buffers
	auto requiredBufferSize = 0ull;
	auto alignment = prosper::util::calculate_buffer_alignment(dev,bufCreateInfo.usageFlags);
	for(auto &data : meshUvData)
		requiredBufferSize += prosper::util::get_aligned_size(data.size() *sizeof(data.front()),alignment);
	bufCreateInfo.size = requiredBufferSize;
	auto lightMapUvBuffer = prosper::util::create_dynamic_resizable_buffer(*c_engine,bufCreateInfo,bufCreateInfo.size,0.2f);
	if(lightMapUvBuffer == nullptr || lightMapUvBuffer->Map(0ull,lightMapUvBuffer->GetSize()) == false)
		return nullptr;
	outMeshLightMapUvBuffers.reserve(meshUvData.size());
	for(auto &data : meshUvData)
	{
		auto subBuf = lightMapUvBuffer->AllocateBuffer(data.size() *sizeof(data.front()));
		outMeshLightMapUvBuffers.push_back(subBuf);

		lightMapUvBuffer->Write(subBuf->GetStartOffset(),data.size() *sizeof(data.front()),data.data());
	}
	lightMapUvBuffer->Unmap();
	return lightMapUvBuffer;
}

const std::shared_ptr<util::bsp::LightMapInfo> &CLightMapComponent::GetLightmapInfo() const {return m_lightmapInfo;}

std::shared_ptr<prosper::Texture> CLightMapComponent::LoadLightMap(pragma::level::BSPInputData &bspInputData)
{
	auto &lightMapInfo = bspInputData.lightMapInfo;
	const auto widthLightmapAtlas = lightMapInfo.atlasSize;
	const auto heightLightmapAtlas = lightMapInfo.atlasSize;
	auto borderSize = lightMapInfo.borderSize;

	auto &lightMapData = bspInputData.lightMapInfo.luxelData;
	if(lightMapData.empty())
		return nullptr;
	std::vector<std::array<uint16_t,4>> lightMapImageData(widthLightmapAtlas *heightLightmapAtlas);
	auto &rects = lightMapInfo.lightmapAtlas;
	if(rects.size() != lightMapInfo.faceInfos.size())
		; // TODO: Print Warning: LIGHT MAP ATLAS TO SMALL TO ENCOMPASS ALL LIGHTMAPS
	auto rectIdx = 0u;
	for(auto lmIdx=decltype(lightMapInfo.faceInfos.size()){0u};lmIdx<lightMapInfo.faceInfos.size();++lmIdx)
	{
		auto &info = lightMapInfo.faceInfos.at(lmIdx);
		if(info.valid() == false)
			continue;
		auto &rect = rects.at(rectIdx++);
		info.x = rect.x;
		info.y = rect.y;
		if(info.lightMapSize.at(0) +borderSize *2u != rect.w)
		{
			// This shouldn't happen (The bounds should already have been put in the proper rotation before the map-file was created)
			throw std::runtime_error("Illegal light map bounds!");
			info.flags |= util::bsp::FaceLightMapInfo::Flags::Rotated;
		}
		info.lightMapSize.at(0) = rect.w -borderSize *2u;
		info.lightMapSize.at(1) = rect.h -borderSize *2u;
	}

	for(auto faceIndex=decltype(lightMapInfo.faceInfos.size()){0u};faceIndex<lightMapInfo.faceInfos.size();++faceIndex)
	{
		auto &lmInfo = lightMapInfo.faceInfos.at(faceIndex);
		if(lmInfo.valid() == false)
			continue;
		auto &face = lightMapInfo.faceInfos.at(faceIndex);
		if(face.luxelDataOffset == std::numeric_limits<uint32_t>::max())
			continue; // No light map available for this face (e.g. nodraw or skybox)
		auto widthLightmap = face.lightMapSize.at(0);
		auto heightLightmap = face.lightMapSize.at(1);
		auto bRotated = (lmInfo.flags &util::bsp::FaceLightMapInfo::Flags::Rotated) != util::bsp::FaceLightMapInfo::Flags::None;
		const auto fCalcPixelIndex = [&lmInfo,widthLightmapAtlas,heightLightmap,bRotated,borderSize](int x,int y,int offsetX,int offsetY) {
			x += lmInfo.x +borderSize +offsetX;
			y += lmInfo.y +borderSize +offsetY;
			return y *widthLightmapAtlas +x;
		};
		auto *lightmapData = lightMapData.data() +face.luxelDataOffset;
		for(auto y=decltype(heightLightmap){0u};y<heightLightmap;++y)
		{
			for(auto x=decltype(widthLightmap){0u};x<widthLightmap;++x)
			{
				auto xLuxel = x;
				auto yLuxel = y;
				if(bRotated)
				{
					auto tmp = yLuxel;
					yLuxel = widthLightmap -xLuxel -1;
					xLuxel = tmp;
				}
				auto &luxel = *reinterpret_cast<const bsp::ColorRGBExp32*>(lightmapData +(yLuxel *(bRotated ? heightLightmap : widthLightmap) +xLuxel) *sizeof(bsp::ColorRGBExp32));

				auto exp = umath::pow(2.0,static_cast<double>(luxel.exponent));
				auto rgbR = luxel.r *exp /255.f;
				auto rgbG = luxel.g *exp /255.f;
				auto rgbB = luxel.b *exp /255.f;

				if(lmInfo.lightMapSize.at(1) != 0)
				{
					auto pxIdx = fCalcPixelIndex(x,y,0,0);

					std::array<uint16_t,4> hdrPxCol = {
						umath::float32_to_float16_glm(rgbR),
						umath::float32_to_float16_glm(rgbG),
						umath::float32_to_float16_glm(rgbB),
						1.f
					};
					lightMapImageData.at(pxIdx) = hdrPxCol;

					auto borderCol = hdrPxCol;
					//borderCol = bRotated ? Vector4{255.f,0.f,255.f,255.f} : Vector4{0.f,255.f,255.f,255.f};
					// Vertical border
					if(x == 0u)
					{
						for(auto i=decltype(borderSize){1u};i<=borderSize;++i)
							lightMapImageData.at(fCalcPixelIndex(x,y,-i,0)) = borderCol;
						if(y == 0u)
						{
							// Fill out top left corner
							for(auto xOff=decltype(borderSize){1u};xOff<=borderSize;++xOff)
							{
								for(auto yOff=decltype(borderSize){1u};yOff<=borderSize;++yOff)
									lightMapImageData.at(fCalcPixelIndex(x,y,-xOff,-yOff)) = borderCol;
							}
						}
					}
					else if(x == (widthLightmap -1u))
					{
						for(auto i=decltype(borderSize){1u};i<=borderSize;++i)
							lightMapImageData.at(fCalcPixelIndex(x,y,i,0)) = borderCol;
						if(y == (heightLightmap -1u))
						{
							// Fill out bottom right corner
							for(auto xOff=decltype(borderSize){1u};xOff<=borderSize;++xOff)
							{
								for(auto yOff=decltype(borderSize){1u};yOff<=borderSize;++yOff)
									lightMapImageData.at(fCalcPixelIndex(x,y,xOff,yOff)) = borderCol;
							}
						}
					}

					// Horizontal border
					if(y == 0u)
					{
						for(auto i=decltype(borderSize){1u};i<=borderSize;++i)
							lightMapImageData.at(fCalcPixelIndex(x,y,0,-i)) = borderCol;
						if(x == (widthLightmap -1u))
						{
							// Fill out top right corner
							for(auto xOff=decltype(borderSize){1u};xOff<=borderSize;++xOff)
							{
								for(auto yOff=decltype(borderSize){1u};yOff<=borderSize;++yOff)
									lightMapImageData.at(fCalcPixelIndex(x,y,xOff,-yOff)) = borderCol;
							}
						}
					}
					else if(y == (heightLightmap -1u))
					{
						for(auto i=decltype(borderSize){1u};i<=borderSize;++i)
							lightMapImageData.at(fCalcPixelIndex(x,y,0,i)) = borderCol;
						if(x == 0u)
						{
							// Fill out bottom left corner
							for(auto xOff=decltype(borderSize){1u};xOff<=borderSize;++xOff)
							{
								for(auto yOff=decltype(borderSize){1u};yOff<=borderSize;++yOff)
									lightMapImageData.at(fCalcPixelIndex(x,y,-xOff,yOff)) = borderCol;
							}
						}
					}
				}
			}
		}
	}

	// Build lightmap texture
	return CreateLightmapTexture(widthLightmapAtlas,heightLightmapAtlas,reinterpret_cast<uint16_t*>(lightMapImageData.data()));
}

std::shared_ptr<prosper::Texture> CLightMapComponent::CreateLightmapTexture(uint32_t width,uint32_t height,const uint16_t *hdrPixelData)
{
	auto &dev = c_engine->GetDevice();
	prosper::util::ImageCreateInfo lightMapCreateInfo {};
	lightMapCreateInfo.width = width;
	lightMapCreateInfo.height = height;
	lightMapCreateInfo.format = Anvil::Format::R16G16B16A16_SFLOAT;
	lightMapCreateInfo.postCreateLayout = Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL;
	lightMapCreateInfo.usage = Anvil::ImageUsageFlagBits::SAMPLED_BIT | Anvil::ImageUsageFlagBits::TRANSFER_SRC_BIT;
	lightMapCreateInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::CPUToGPU;
	lightMapCreateInfo.tiling = Anvil::ImageTiling::LINEAR;
	auto imgStaging = prosper::util::create_image(dev,lightMapCreateInfo,reinterpret_cast<const uint8_t*>(hdrPixelData));

	lightMapCreateInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
	lightMapCreateInfo.tiling = Anvil::ImageTiling::OPTIMAL;
	auto img = prosper::util::create_image(dev,lightMapCreateInfo);

	auto &setupCmd = c_engine->GetSetupCommandBuffer();
	if(prosper::util::record_blit_image(**setupCmd,{},**imgStaging,**img) == false)
		; // TODO: Print warning
	c_engine->FlushSetupCommandBuffer();

	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	samplerCreateInfo.minFilter = Anvil::Filter::LINEAR;
	samplerCreateInfo.magFilter = Anvil::Filter::LINEAR;
	samplerCreateInfo.addressModeU = Anvil::SamplerAddressMode::CLAMP_TO_EDGE; // Doesn't really matter since lightmaps have their own border either way
	samplerCreateInfo.addressModeV = Anvil::SamplerAddressMode::CLAMP_TO_EDGE;
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {}; // TODO: Is this needed?
	return prosper::util::create_texture(dev,{},img,&imgViewCreateInfo,&samplerCreateInfo);
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
	auto &dstLightmapUvs = lightmapC->GetLightmapUvs();
	uint32_t numVerts = 0;
	uint32_t numTris = 0;
	// Count the number of primitives so we can pre-allocate the data properly
	for(auto &mesh : meshGroup->GetMeshes())
	{
		for(auto &subMesh : mesh->GetSubMeshes())
		{
			auto refId = subMesh->GetReferenceId();
			if(refId >= dstLightmapUvs.size())
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
			auto refId = subMesh->GetReferenceId();
			if(refId >= dstLightmapUvs.size())
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

		auto &dstLightmapUvs = lightmapC->GetLightmapUvs();
		auto &newLightmapUvs = worker.GetResult();
		uint32_t vertexOffset = 0u;
		for(auto &mesh : meshGroup->GetMeshes())
		{
			for(auto &subMesh : mesh->GetSubMeshes())
			{
				auto refId = subMesh->GetReferenceId();
				if(refId >= dstLightmapUvs.size())
					continue;
				auto &dstMeshLightmapUvs = dstLightmapUvs.at(refId);
				auto numVerts = subMesh->GetVertexCount();
				assert(newLightmapUvs.size() >= vertexOffset +numVerts);
				for(auto i=vertexOffset;i<(vertexOffset +numVerts);++i)
					dstMeshLightmapUvs.at(i -vertexOffset) = newLightmapUvs.at(i);
				vertexOffset += numVerts;
			}
		}
		lightmapC->UpdateLightmapUvBuffers();
		callback(true);
	});
	job.Start();
	c_engine->AddParallelJob(job,"Lightmap UV Atlas");
}

static void generate_lightmaps(BaseEntity &ent,uint32_t width,uint32_t height,uint32_t sampleCount,bool denoise)
{
	Con::cout<<"Baking lightmaps... This may take a few minutes!"<<Con::endl;
	constexpr auto hdrOutput = true;
	pragma::rendering::cycles::SceneInfo sceneInfo {};
	//sceneInfo.width = 256;//1024;//lightmapC->GetLightmapInfo()->atlasSize;//1'024; // TODO: Use original lightmap size
	//sceneInfo.height = 256;//1024;//lightmapC->GetLightmapInfo()->atlasSize;//1'024; // TODO: Use original lightmap size
	sceneInfo.width = width;
	sceneInfo.height = height;
	sceneInfo.samples = sampleCount;//16;//8192 *4; // TODO
	sceneInfo.denoise = denoise;
	sceneInfo.hdrOutput = hdrOutput;

	auto job = pragma::rendering::cycles::bake_lightmaps(*client,sceneInfo,ent);
	if(job.IsValid() == false)
	{
		Con::cwar<<"WARNING: Unable to initialize cycles scene for lightmap baking!"<<Con::endl;
		return;
	}
	job.SetCompletionHandler([hdrOutput](util::ParallelWorker<std::shared_ptr<util::ImageBuffer>> &worker) {
		if(worker.IsSuccessful() == false)
		{
			Con::cwar<<"WARNING: Unable to bake lightmaps: "<<worker.GetResultMessage()<<Con::endl;
			return;
		}
		auto &dev = c_engine->GetDevice();

		auto imgBuffer = worker.GetResult();
		if(hdrOutput == false)
		{
			// No HDR output, but we'll still use HDR data
			imgBuffer->Convert(util::ImageBuffer::Format::RGBA16);
		}
		auto tex = CLightMapComponent::CreateLightmapTexture(imgBuffer->GetWidth(),imgBuffer->GetHeight(),reinterpret_cast<uint16_t*>(imgBuffer->GetData()));
		/*{
		TextureManager::LoadInfo loadInfo {};
		loadInfo.flags = TextureLoadFlags::LoadInstantly;
		std::shared_ptr<void> ptrTex;
		static_cast<CMaterialManager&>(client->GetMaterialManager()).GetTextureManager().Load(*c_engine,"lightmaps_medium.dds",loadInfo,&ptrTex);
		tex = std::static_pointer_cast<Texture>(ptrTex)->texture;
		}*/

		auto &ent = c_game->GetWorld()->GetEntity();
		auto lightmapC = ent.GetComponent<pragma::CLightMapComponent>();
		if(lightmapC.valid())
			const_cast<std::shared_ptr<prosper::Texture>&>(lightmapC->GetLightMap()) = tex; // TODO

		auto &scene = c_game->GetRenderScene();
		static_cast<pragma::rendering::RasterizationRenderer*>(scene->GetRenderer())->SetLightMap(tex);

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

void Console::commands::map_rebuild_lightmaps(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv)
{
	if(c_game == nullptr)
	{
		Con::cwar<<"WARNING: No map loaded!"<<Con::endl;
		return;
	}
	auto *world = c_game->GetWorld();
	if(world == nullptr)
	{
		Con::cwar<<"WARNING: World is invalid!"<<Con::endl;
		return;
	}
	auto &ent = world->GetEntity();
	auto lightmapC = ent.GetComponent<pragma::CLightMapComponent>();
	if(lightmapC.expired())
	{
		Con::cwar<<"WARNING: World has no lightmap component! Lightmaps cannot be generated!"<<Con::endl;
		return;
	}

	std::unordered_map<std::string,pragma::console::CommandOption> commandOptions {};
	pragma::console::parse_command_options(argv,commandOptions);

	//auto resolution = c_engine->GetRenderResolution();
	Vector2i resolution {
		lightmapC->GetLightmapInfo()->atlasSize,
		lightmapC->GetLightmapInfo()->atlasSize
	};
	auto width = util::to_uint(pragma::console::get_command_option_parameter_value(commandOptions,"width",std::to_string(resolution.x)));
	auto height = util::to_uint(pragma::console::get_command_option_parameter_value(commandOptions,"height",std::to_string(resolution.y)));
	auto sampleCount = util::to_uint(pragma::console::get_command_option_parameter_value(commandOptions,"samples","1225"));
	auto denoise = false;
	auto itDenoise = commandOptions.find("denoise");
	if(itDenoise != commandOptions.end())
		denoise = true;
	auto hEnt = ent.GetHandle();
	auto itRebuildUvAtlas = commandOptions.find("rebuild_uv_atlas");
	if(itRebuildUvAtlas != commandOptions.end())
	{
		generate_lightmap_uv_atlas(ent,width,height,[hEnt,width,height,sampleCount,denoise](bool success) {
			if(success == false || hEnt.IsValid() == false)
				return;
			generate_lightmaps(*hEnt.get(),width,height,sampleCount,denoise);
		});
		return;
	}
	generate_lightmaps(*hEnt.get(),width,height,sampleCount,denoise);
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

	EntityIterator entIt {*c_game,};
	entIt.AttachFilter<TEntityIteratorFilterComponent<CLightMapComponent>>();
	auto it = entIt.begin();
	if(it == entIt.end())
		return;
	auto *ent = *it;
	auto lightmapC = ent->GetComponent<CLightMapComponent>();
	auto &lightmap = lightmapC->GetLightMap();
	if(lightmap == nullptr)
		return;

	auto *pElContainer = wgui.Create<WIBase>();
	pElContainer->SetAutoAlignToParent(true);
	pElContainer->SetName(name);
	pElContainer->TrapFocus(true);
	pElContainer->RequestFocus();

	auto *pFrame = wgui.Create<WIFrame>(pElContainer);
	pFrame->SetTitle("BRDF");
	auto *pLightmaps = wgui.Create<WITexturedRect>(pFrame);
	pLightmaps->SetSize(256,256);
	pLightmaps->SetY(24);
	pLightmaps->SetTexture(*lightmap);
	pFrame->SizeToContents();
	pLightmaps->SetAnchor(0.f,0.f,1.f,1.f);
}
#pragma optimize("",on)
