/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "pragma/c_engine.h"
#include "pragma/game/c_game.h"
#include "pragma/game/game_resources.hpp"
#include "pragma/clientstate/clientstate.h"
#include "pragma/entities/components/c_light_map_component.hpp"
#include <pragma/entities/entity_component_system_t.hpp>
#include <util_bsp.hpp>
#include <buffers/prosper_buffer.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_util.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;
extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

// These are hardcoded limits in the source engine which mustn't be exceeded when we're writing luxel data to the BSP
constexpr uint32_t BSP_MAX_BRUSH_LIGHTMAP_DIM_WITHOUT_BORDER = 32;
constexpr uint32_t BSP_MAX_BRUSH_LIGHTMAP_DIM_INCLUDING_BORDER = 35;
constexpr uint32_t BSP_MAX_DISP_LIGHTMAP_DIM_WITHOUT_BORDER = 128;
constexpr uint32_t BSP_MAX_DISP_LIGHTMAP_DIM_INCLUDING_BORDER = 131;

#define BSP_ASSERT(condition,msg) \
	if(!(condition)) \
	{ \
		throw std::logic_error{#msg}; \
	}

//#define TEST_SCALE_LIGHTMAP_ATLAS
#include <util_bsp.hpp>
void CLightMapComponent::ConvertLightmapToBSPLuxelData() const
{
	Con::cwar<<"WARNING: BSP lightmap conversion is currently disabled due to changes to the level format. Please contact the Engine developer if you need this feature!"<<Con::endl;

	// TODO: Pragma's map format does not contain all of the lightmap information we need anymore.
	// Instead, we need to read the information from the original Source Engine BSP-file.
#if 0
	auto &lightMapInfo = *m_lightmapInfo;
	bsp::ColorRGBExp32 *luxelData = nullptr;
	uint32_t luxelDataSize = 0u;
	auto &lightmap = GetLightMap();
	if(lightmap == nullptr || lightMapInfo.luxelData.empty())
		return;
	auto &img = lightmap->GetImage();
	auto extents = img->GetExtents();
#ifdef TEST_SCALE_LIGHTMAP_ATLAS
	extents.width *= 2;
	extents.height *= 2;
#endif
	// If the lightmap resolution has changed, we will have to make some additional changes to the BSP file (i.e. updating offsets)
	auto widthLightmapAtlas = lightMapInfo.atlasSize.x;
	auto heightLightmapAtlas = lightMapInfo.atlasSize.y;
	auto scaleFactorW = extents.width /static_cast<float>(widthLightmapAtlas);
	auto scaleFactorH = extents.height /static_cast<float>(heightLightmapAtlas);
	auto newResolution = (extents.width != widthLightmapAtlas || extents.height != heightLightmapAtlas);

	std::vector<bsp::ColorRGBExp32> newLuxels; // Only used if resolution has changed
	auto borderSize = lightMapInfo.borderSize;
	if(newResolution == false)
	{
		luxelData = reinterpret_cast<bsp::ColorRGBExp32*>(lightMapInfo.luxelData.data());
		luxelDataSize = lightMapInfo.luxelData.size() /sizeof(bsp::ColorRGBExp32);
	}
	else
	{
		uint32_t maxLuxelCount = 0;
		for(auto faceIndex=decltype(lightMapInfo.faceInfos.size()){0u};faceIndex<lightMapInfo.faceInfos.size();++faceIndex)
		{
			auto &lmInfo = lightMapInfo.faceInfos.at(faceIndex);
			if(lmInfo.valid() == false)
				continue;
			auto &face = lightMapInfo.faceInfos.at(faceIndex);
			if(face.luxelDataOffset == std::numeric_limits<uint32_t>::max())
				continue; // No light map available for this face (e.g. nodraw or skybox)
			auto offset = face.luxelDataOffset;
#ifdef TEST_SCALE_LIGHTMAP_ATLAS
			offset *= 2 *2;
#endif
			BSP_ASSERT((offset %sizeof(bsp::ColorRGBExp32)) == 0,"Unexpected offset alignment for luxel data!");
			auto index = offset /sizeof(bsp::ColorRGBExp32);
			auto widthLightmap = umath::ceil(face.lightMapSize.at(0) *scaleFactorW);
			auto heightLightmap = umath::ceil(face.lightMapSize.at(1) *scaleFactorH);
			maxLuxelCount = umath::max<uint32_t>(index +widthLightmap *heightLightmap,maxLuxelCount);
		}
		newLuxels.resize(maxLuxelCount);
		luxelData = newLuxels.data();
		luxelDataSize = newLuxels.size() *sizeof(newLuxels.front());
	}

#ifdef TEST_SCALE_LIGHTMAP_ATLAS
	auto numPixels = extents.width /2 *extents.height /2;
#else
	auto numPixels = extents.width *extents.height;
#endif
	std::vector<uint16_t> lightmapColors;
	lightmapColors.resize(numPixels *4);
	auto szLightmaps = lightmapColors.size() *sizeof(lightmapColors.front());

	// We can't read the image data directly, so we'll need a temporary buffer to copy it into
	prosper::util::BufferCreateInfo createInfo {};
	createInfo.size = szLightmaps;
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUToCPU;
	createInfo.usageFlags = prosper::BufferUsageFlags::TransferDstBit;
	auto buf = .CreateBuffer(c_engine->GetDevice(),createInfo);

	auto &setupCmd = c_engine->GetSetupCommandBuffer();
	.RecordImageBarrier(**setupCmd,**img,prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::TransferDstOptimal);
	.RecordCopyImageToBuffer(**setupCmd,{},**img,prosper::ImageLayout::TransferDstOptimal,*buf);
	.RecordImageBarrier(**setupCmd,**img,prosper::ImageLayout::TransferDstOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal);
	c_engine->FlushSetupCommandBuffer();

	if(buf->Map(0,szLightmaps) == false)
		return;
	buf->Read(0,szLightmaps,lightmapColors.data());
	buf->Unmap();
#ifdef TEST_SCALE_LIGHTMAP_ATLAS
	std::vector<uint16_t> scaledLightmapColors {};
	scaledLightmapColors.resize(extents.width *extents.height *4);
	for(auto i=0;i<scaledLightmapColors.size();++i)
	{
		auto iSrc = i /2 /2;
		scaledLightmapColors.at(i) = lightmapColors.at(iSrc);
	}
	lightmapColors = std::move(scaledLightmapColors);
#endif

	// Convert lightmap image data to BSP luxels
	for(auto faceIndex=decltype(lightMapInfo.faceInfos.size()){0u};faceIndex<lightMapInfo.faceInfos.size();++faceIndex)
	{
		auto &lmInfo = lightMapInfo.faceInfos.at(faceIndex);
		if(lmInfo.valid() == false)
			continue;
		auto &face = lightMapInfo.faceInfos.at(faceIndex);
		if(face.luxelDataOffset == std::numeric_limits<uint32_t>::max())
			continue; // No light map available for this face (e.g. nodraw or skybox)
		auto widthLightmap = umath::ceil(face.lightMapSize.at(0) *scaleFactorW);
		auto heightLightmap = umath::ceil(face.lightMapSize.at(1) *scaleFactorH);
		auto bRotated = (lmInfo.flags &util::bsp::FaceLightMapInfo::Flags::Rotated) != util::bsp::FaceLightMapInfo::Flags::None;
		const auto fCalcPixelIndex = [&lmInfo,extents,bRotated,borderSize,scaleFactorW,scaleFactorH](int x,int y,int offsetX,int offsetY) {
			x += lmInfo.x *scaleFactorW +borderSize +offsetX; // TODO
			y += lmInfo.y *scaleFactorH +borderSize +offsetY;
			return y *extents.width +x;
		};
		auto luxelDataOffset = face.luxelDataOffset;
#ifdef TEST_SCALE_LIGHTMAP_ATLAS
		luxelDataOffset *= 2 *2;
#endif
		auto *lightmapData = reinterpret_cast<bsp::ColorRGBExp32*>(reinterpret_cast<uint8_t*>(luxelData) +luxelDataOffset);
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
				auto &luxel = *(lightmapData +(yLuxel *(bRotated ? heightLightmap : widthLightmap) +xLuxel));

				if(lmInfo.lightMapSize.at(1) != 0)
				{
					auto pxIdx = fCalcPixelIndex(x,y,0,0);

					auto *pxCol = &lightmapColors.at(pxIdx *4);

					// TODO: How to calculate luxel exponent from arbitrary pixel color data?
					auto exp = umath::pow(2.0,static_cast<double>(luxel.exponent));
					const auto fToCompressedLuxel = [exp](uint16_t pxVal) {
						// TODO
						auto floatVal = 255.f;//umath::float16_to_float32_glm(pxVal) *255.f;
						return static_cast<uint8_t>(umath::clamp<double>(static_cast<double>(floatVal) /exp,0,std::numeric_limits<uint8_t>::max()));
					};
					luxel.r = fToCompressedLuxel(pxCol[0]);
					luxel.g = fToCompressedLuxel(pxCol[1]);
					luxel.b = fToCompressedLuxel(pxCol[2]);
				}
			}
		}
	}

	// Write new luxel data back into BSP
	std::string mapPath = "maps/" +c_game->GetMapName() +".bsp";
	auto f = FileManager::OpenFile<VFilePtrReal>(mapPath.c_str(),"r+b");
	if(f == nullptr)
		return;
	bsp::ResultCode code;
	auto bsp = bsp::File::Open(VFilePtr{f},code);
	//auto *pLumpInfoLDR = bsp ? bsp->GetLumpHeaderInfo(bsp::LUMP_ID_LIGHTING) : nullptr;
	auto *pLumpInfoHDR = bsp ? bsp->GetLumpHeaderInfo(bsp::LUMP_ID_LIGHTING_HDR) : nullptr;
	if(pLumpInfoHDR == nullptr)
		return;
	if(newResolution)
	{
		std::vector<uint8_t> originalData {};
		originalData.resize(f->GetSize());
		f->Seek(0);
		f->Read(originalData.data(),f->GetSize());

		// If the lightmap atlas resolution has changed, we will need to move all data after the lightmap lump back,
		// as well as update all of the offsets in the header and the game lumps.
		auto oldLumpSize = pLumpInfoHDR->filelen;
		auto newLumpSize = luxelDataSize;
		auto deltaSize = static_cast<int32_t>(newLumpSize) -static_cast<int32_t>(oldLumpSize);

		auto lightingLumpOffset = pLumpInfoHDR->fileofs;
		auto *pGameLump = bsp ? bsp->GetLumpHeaderInfo(bsp::LUMP_ID_GAME) : nullptr;
		if(pGameLump)
		{
			// Offsets in game lump are relative to the beginning of the file instead of relative to the lump, so we have to update them
			// (exception being the BSP-files of the console version of Portal 2, but we'll ignore that here)
			auto gameLumps = bsp->GetGameLumps();
			for(auto &lump : gameLumps)
			{
				if(lump.fileofs <= lightingLumpOffset)
					continue;
				lump.fileofs += deltaSize;
			}
			f->Seek(pGameLump->fileofs +sizeof(int32_t)); // We can skip the number of lumps, since it has remained the same

														  // BSP_ASSERT(gameLumps.size() *sizeof(gameLumps.front()) == pGameLump->filelen,"Game lump size mismatch!");
			f->Write(gameLumps.data(),gameLumps.size() *sizeof(gameLumps.front()));
		}

		// Update face infos
		auto faces = bsp->GetHDRFaces();
		for(auto &face : faces)
		{
			if(face.lightofs == -1)
				continue;
			face.LightmapTextureSizeInLuxels.at(0) = (face.LightmapTextureSizeInLuxels.at(0) +1) *scaleFactorW -1;
			face.LightmapTextureSizeInLuxels.at(1) = (face.LightmapTextureSizeInLuxels.at(1) +1) *scaleFactorH -1;

			//if(face.LightmapTextureSizeInLuxels.at(0) >= BSP_MAX_BRUSH_LIGHTMAP_DIM_INCLUDING_BORDER ||
			//	face.LightmapTextureSizeInLuxels.at(1) >= BSP_MAX_BRUSH_LIGHTMAP_DIM_INCLUDING_BORDER)
			//	Con::cwar<<"WARNING!"<<Con::endl;
			BSP_ASSERT(face.LightmapTextureSizeInLuxels.at(0) < BSP_MAX_BRUSH_LIGHTMAP_DIM_INCLUDING_BORDER &&
				face.LightmapTextureSizeInLuxels.at(1) < BSP_MAX_BRUSH_LIGHTMAP_DIM_INCLUDING_BORDER,"Lightmap bounds exceed max bounds supported by source engine");
		}
		auto &hdrFaceLump = *bsp->GetLumpHeaderInfo(bsp::LUMP_ID_FACES_HDR);
		f->Seek(hdrFaceLump.fileofs);
		f->Write(faces.data(),faces.size() *sizeof(faces.front()));


		// Update header lumps
		auto headerData = bsp->GetHeaderData();
		for(auto &lumpData : headerData.lumps)
		{
			if(lumpData.fileofs <= lightingLumpOffset)
				continue;
			lumpData.fileofs += deltaSize;
		}

		f->Seek(0);
		f->Write(&headerData,sizeof(headerData));

		// Temporarily store remaining data after luxel data
		f->Seek(lightingLumpOffset +pLumpInfoHDR->filelen);
		auto offset = f->Tell();
		auto bspSize = f->GetSize();
		auto sizeRemaining = bspSize -offset;
		std::vector<uint8_t> remainingData {};
		remainingData.resize(sizeRemaining);
		f->Read(remainingData.data(),remainingData.size() *sizeof(remainingData.front()));

		// Write new luxel data
		f->Seek(lightingLumpOffset);
		f->Write(luxelData,luxelDataSize);

		// Write remaining data back to file
		f->Write(remainingData.data(),remainingData.size() *sizeof(remainingData.front()));
	}
	else
	{
		f->Seek(pLumpInfoHDR->fileofs);
		f->Write(luxelData,luxelDataSize);
	}




	//fOut->Seek(lumpInfoLDR.fileofs);
	//fOut->Write(luxelData.data(),luxelData.size() *sizeof(luxelData.front()));
	f = nullptr;
#endif
}
