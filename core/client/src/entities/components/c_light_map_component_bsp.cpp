/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "pragma/c_engine.h"
#include "pragma/game/c_game.h"
#include "pragma/game/game_resources.hpp"
#include "pragma/clientstate/clientstate.h"
#include "pragma/entities/components/c_light_map_component.hpp"
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/game/game_resources.hpp>
#include <util_image_buffer.hpp>
#include <buffers/prosper_buffer.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_util.hpp>
#include <image/prosper_texture.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>

import source_engine.bsp;

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

//#define TEST_SCALE_LIGHTMAP_ATLAS
void CLightMapComponent::ConvertLightmapToBSPLuxelData() const
{
	auto &lightmap = GetLightMap();
	if(lightmap == nullptr)
		return;
	auto &img = lightmap->GetImage();
	auto extents = img.GetExtents();
#ifdef TEST_SCALE_LIGHTMAP_ATLAS
	extents.width *= 2;
	extents.height *= 2;
#endif
#ifdef TEST_SCALE_LIGHTMAP_ATLAS
	auto numPixels = extents.width / 2 * extents.height / 2;
#else
	auto numPixels = extents.width * extents.height;
#endif
	auto imgBuf = uimg::ImageBuffer::Create(extents.width, extents.height, uimg::Format::RGBA16);

	// We can't read the image data directly, so we'll need a temporary buffer to copy it into
	auto &context = c_engine->GetRenderContext();
	prosper::util::BufferCreateInfo createInfo {};
	createInfo.size = imgBuf->GetSize();
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUToCPU;
	createInfo.usageFlags = prosper::BufferUsageFlags::TransferDstBit;
	auto buf = context.CreateBuffer(createInfo);

	auto &setupCmd = c_engine->GetSetupCommandBuffer();
	setupCmd->RecordImageBarrier(img, prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::TransferDstOptimal);
	setupCmd->RecordCopyImageToBuffer({}, img, prosper::ImageLayout::TransferDstOptimal, *buf);
	setupCmd->RecordImageBarrier(img, prosper::ImageLayout::TransferDstOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
	c_engine->FlushSetupCommandBuffer();

	if(buf->Map(0, createInfo.size, prosper::IBuffer::MapFlags::ReadBit) == false)
		return;
	buf->Read(0, createInfo.size, imgBuf->GetData());
	buf->Unmap();
#ifdef TEST_SCALE_LIGHTMAP_ATLAS
	std::vector<uint16_t> scaledLightmapColors {};
	scaledLightmapColors.resize(extents.width * extents.height * 4);
	for(auto i = 0; i < scaledLightmapColors.size(); ++i) {
		auto iSrc = i / 2 / 2;
		scaledLightmapColors.at(i) = lightmapColors.at(iSrc);
	}
	lightmapColors = std::move(scaledLightmapColors);
#endif
	// imgBuf->Clear(Color::Red);

	auto mapPath = "maps/" + c_game->GetMapName() + ".bsp";
	auto *convertLightmapDataToBspLuxelData = reinterpret_cast<bool (*)(NetworkState &, const std::string &, const uimg::ImageBuffer &, uint32_t, uint32_t, std::string &)>(::util::impl::get_module_func(client, "convert_lightmap_data_to_bsp_luxel_data"));
	if(convertLightmapDataToBspLuxelData == nullptr)
		return;
	std::string errMsg;
	if(convertLightmapDataToBspLuxelData(*client, mapPath, *imgBuf, extents.width, extents.height, errMsg) == false) {
		Con::cwar << "Unable to convert lightmap data to BSP luxel data: " << errMsg << Con::endl;
		return;
	}
	Con::cout << "Successfully written lightmap luxel data!" << Con::endl;
}
