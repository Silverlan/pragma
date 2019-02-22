#include "stdafx_client.h"
#include "pragma/entities/components/c_light_map_component.hpp"
#include "pragma/model/c_model.h"
#include <pragma/util/util_tga.hpp>
#include <pragma/level/mapgeometry.h>
#include <GuillotineBinPack.h>
#include <util_bsp.hpp>
#include <prosper_util.hpp>
#include <buffers/prosper_dynamic_resizable_buffer.hpp>
#include <prosper_command_buffer.hpp>
#include <image/prosper_sampler.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;
extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

luabind::object CLightMapComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CLightMapComponentHandleWrapper>(l);}
void CLightMapComponent::Initialize()
{
	BaseEntityComponent::Initialize();
}

void CLightMapComponent::InitializeLightMapData(
	const std::shared_ptr<prosper::Texture> &lightMap,
	const std::shared_ptr<prosper::DynamicResizableBuffer> &lightMapUvBuffer,
	const std::vector<std::shared_ptr<prosper::Buffer>> &meshUvBuffers
)
{
	m_lightMapAtlas = lightMap;
	m_meshLightMapUvBuffer = lightMapUvBuffer;
	m_meshLightMapUvBuffers = meshUvBuffers;
}

const std::shared_ptr<prosper::Texture> &CLightMapComponent::GetLightMap() const {return m_lightMapAtlas;}

prosper::Buffer *CLightMapComponent::GetMeshLightMapUvBuffer(uint32_t meshIdx) const
{
	if(meshIdx >= m_meshLightMapUvBuffers.size())
		return nullptr;
	return m_meshLightMapUvBuffers.at(meshIdx).get();
}

const std::vector<std::shared_ptr<prosper::Buffer>> &CLightMapComponent::GetMeshLightMapUvBuffers() const {return const_cast<CLightMapComponent*>(this)->GetMeshLightMapUvBuffers();}
std::vector<std::shared_ptr<prosper::Buffer>> &CLightMapComponent::GetMeshLightMapUvBuffers() {return m_meshLightMapUvBuffers;}

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
	if(lightMapUvBuffer->Map(0ull,lightMapUvBuffer->GetSize()) == false)
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

std::shared_ptr<prosper::Texture> CLightMapComponent::LoadLightMap(pragma::level::BSPInputData &bspInputData)
{
	auto &lightMapInfo = bspInputData.lightMapInfo;
	const auto widthLightmapAtlas = lightMapInfo.atlasSize;
	const auto heightLightmapAtlas = lightMapInfo.atlasSize;
	auto borderSize = lightMapInfo.borderSize;

	auto &lightMapData = bspInputData.lightMapInfo.luxelData;
	if(lightMapData.empty())
		return nullptr;
	std::vector<Vector4> lightMapImageData(widthLightmapAtlas *heightLightmapAtlas);
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
				auto rgbR = luxel.r *exp;
				auto rgbG = luxel.g *exp;
				auto rgbB = luxel.b *exp;

				if(lmInfo.lightMapSize.at(1) != 0)
				{
					auto pxIdx = fCalcPixelIndex(x,y,0,0);

					Vector4 pxCol {rgbR,rgbG,rgbB,1.f};
					lightMapImageData.at(pxIdx) = pxCol;

					auto borderCol = pxCol;
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
	auto &dev = c_engine->GetDevice();
	prosper::util::ImageCreateInfo lightMapCreateInfo {};
	lightMapCreateInfo.width = widthLightmapAtlas;
	lightMapCreateInfo.height = heightLightmapAtlas;
	lightMapCreateInfo.format = Anvil::Format::R32G32B32A32_SFLOAT;
	lightMapCreateInfo.postCreateLayout = Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL;
	lightMapCreateInfo.usage = Anvil::ImageUsageFlagBits::SAMPLED_BIT;
	lightMapCreateInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::CPUToGPU;
	lightMapCreateInfo.tiling = Anvil::ImageTiling::LINEAR;
	auto imgStaging = prosper::util::create_image(dev,lightMapCreateInfo,reinterpret_cast<uint8_t*>(lightMapImageData.data()));

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

	// Write lightmap atlas to file; For debugging purposes only
	static auto writeLightMap = false;
	if(writeLightMap)
	{
		std::vector<uint8_t> rgbData {};
		rgbData.reserve(lightMapImageData.size() *3);
		for(auto &v : lightMapImageData)
		{
			rgbData.push_back(umath::clamp(v.x,0.f,255.f));
			rgbData.push_back(umath::clamp(v.y,0.f,255.f));
			rgbData.push_back(umath::clamp(v.z,0.f,255.f));
		}
		util::tga::write_tga("lightmap.tga",lightMapCreateInfo.width,lightMapCreateInfo.height,rgbData);
	}
	return prosper::util::create_texture(dev,{},img,&imgViewCreateInfo,&samplerCreateInfo);
}
