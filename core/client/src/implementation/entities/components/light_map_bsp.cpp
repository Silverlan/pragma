// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.light_map;
import :client_state;
import :engine;
import :game;
import source_engine.bsp;

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
	auto imgBuf = image::ImageBuffer::Create(extents.width, extents.height, image::Format::RGBA16);

	// We can't read the image data directly, so we'll need a temporary buffer to copy it into
	auto &context = get_cengine()->GetRenderContext();
	prosper::util::BufferCreateInfo createInfo {};
	createInfo.size = imgBuf->GetSize();
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUToCPU;
	createInfo.usageFlags = prosper::BufferUsageFlags::TransferDstBit;
	auto buf = context.CreateBuffer(createInfo);

	auto &setupCmd = get_cengine()->GetSetupCommandBuffer();
	setupCmd->RecordImageBarrier(img, prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::TransferDstOptimal);
	setupCmd->RecordCopyImageToBuffer({}, img, prosper::ImageLayout::TransferDstOptimal, *buf);
	setupCmd->RecordImageBarrier(img, prosper::ImageLayout::TransferDstOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
	get_cengine()->FlushSetupCommandBuffer();

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
	// imgBuf->Clear(colors::Red);

	auto mapPath = "maps/" + get_cgame()->GetMapName() + ".bsp";
	auto *convertLightmapDataToBspLuxelData = reinterpret_cast<bool (*)(NetworkState &, const std::string &, const image::ImageBuffer &, uint32_t, uint32_t, std::string &)>(util::impl::get_module_func(get_client_state(), "convert_lightmap_data_to_bsp_luxel_data"));
	if(convertLightmapDataToBspLuxelData == nullptr)
		return;
	std::string errMsg;
	if(convertLightmapDataToBspLuxelData(*get_client_state(), mapPath, *imgBuf, extents.width, extents.height, errMsg) == false) {
		Con::CWAR << "Unable to convert lightmap data to BSP luxel data: " << errMsg << Con::endl;
		return;
	}
	Con::COUT << "Successfully written lightmap luxel data!" << Con::endl;
}
