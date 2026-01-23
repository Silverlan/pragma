// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :util.image;
import :engine;

static pragma::image::Format determine_target_format(prosper::Format format)
{
	auto targetFormat = pragma::image::Format::RGBA8;
	switch(format) {
	case prosper::Format::R8_UNorm:
	case prosper::Format::R8_SNorm:
	case prosper::Format::R8_UScaled_PoorCoverage:
	case prosper::Format::R8_SScaled_PoorCoverage:
	case prosper::Format::R8_UInt:
	case prosper::Format::R8_SInt:
	case prosper::Format::R8_SRGB:
	case prosper::Format::R8G8_UNorm:
	case prosper::Format::R8G8_SNorm:
	case prosper::Format::R8G8_UScaled_PoorCoverage:
	case prosper::Format::R8G8_SScaled_PoorCoverage:
	case prosper::Format::R8G8_UInt:
	case prosper::Format::R8G8_SInt:
	case prosper::Format::R8G8_SRGB_PoorCoverage:
	case prosper::Format::R8G8B8_UNorm_PoorCoverage:
	case prosper::Format::R8G8B8_SNorm_PoorCoverage:
	case prosper::Format::R8G8B8_UScaled_PoorCoverage:
	case prosper::Format::R8G8B8_SScaled_PoorCoverage:
	case prosper::Format::R8G8B8_UInt_PoorCoverage:
	case prosper::Format::R8G8B8_SInt_PoorCoverage:
	case prosper::Format::R8G8B8_SRGB_PoorCoverage:
	case prosper::Format::B8G8R8_UNorm_PoorCoverage:
	case prosper::Format::B8G8R8_SNorm_PoorCoverage:
	case prosper::Format::B8G8R8_UScaled_PoorCoverage:
	case prosper::Format::B8G8R8_SScaled_PoorCoverage:
	case prosper::Format::B8G8R8_UInt_PoorCoverage:
	case prosper::Format::B8G8R8_SInt_PoorCoverage:
	case prosper::Format::B8G8R8_SRGB_PoorCoverage:
	case prosper::Format::R8G8B8A8_UNorm:
	case prosper::Format::R8G8B8A8_SNorm:
	case prosper::Format::R8G8B8A8_UScaled_PoorCoverage:
	case prosper::Format::R8G8B8A8_SScaled_PoorCoverage:
	case prosper::Format::R8G8B8A8_UInt:
	case prosper::Format::R8G8B8A8_SInt:
	case prosper::Format::R8G8B8A8_SRGB:
	case prosper::Format::B8G8R8A8_UNorm:
	case prosper::Format::B8G8R8A8_SNorm:
	case prosper::Format::B8G8R8A8_UScaled_PoorCoverage:
	case prosper::Format::B8G8R8A8_SScaled_PoorCoverage:
	case prosper::Format::B8G8R8A8_UInt:
	case prosper::Format::B8G8R8A8_SInt:
	case prosper::Format::B8G8R8A8_SRGB:
	case prosper::Format::A8B8G8R8_UNorm_Pack32:
	case prosper::Format::A8B8G8R8_SNorm_Pack32:
	case prosper::Format::A8B8G8R8_UScaled_Pack32_PoorCoverage:
	case prosper::Format::A8B8G8R8_SScaled_Pack32_PoorCoverage:
	case prosper::Format::A8B8G8R8_UInt_Pack32:
	case prosper::Format::A8B8G8R8_SInt_Pack32:
	case prosper::Format::A8B8G8R8_SRGB_Pack32:
	case prosper::Format::A2R10G10B10_UNorm_Pack32:
	case prosper::Format::A2R10G10B10_SNorm_Pack32_PoorCoverage:
	case prosper::Format::A2R10G10B10_UScaled_Pack32_PoorCoverage:
	case prosper::Format::A2R10G10B10_SScaled_Pack32_PoorCoverage:
	case prosper::Format::A2R10G10B10_UInt_Pack32:
	case prosper::Format::A2R10G10B10_SInt_Pack32_PoorCoverage:
	case prosper::Format::A2B10G10R10_UNorm_Pack32:
	case prosper::Format::A2B10G10R10_SNorm_Pack32_PoorCoverage:
	case prosper::Format::A2B10G10R10_UScaled_Pack32_PoorCoverage:
	case prosper::Format::A2B10G10R10_SScaled_Pack32_PoorCoverage:
	case prosper::Format::A2B10G10R10_UInt_Pack32:
	case prosper::Format::A2B10G10R10_SInt_Pack32_PoorCoverage:
	case prosper::Format::S8_UInt_PoorCoverage:
	case prosper::Format::BC1_RGB_UNorm_Block:
	case prosper::Format::BC1_RGB_SRGB_Block:
	case prosper::Format::BC1_RGBA_UNorm_Block:
	case prosper::Format::BC1_RGBA_SRGB_Block:
	case prosper::Format::BC2_UNorm_Block:
	case prosper::Format::BC2_SRGB_Block:
	case prosper::Format::BC3_UNorm_Block:
	case prosper::Format::BC3_SRGB_Block:
	case prosper::Format::BC4_UNorm_Block:
	case prosper::Format::BC4_SNorm_Block:
		targetFormat = pragma::image::Format::RGBA8;
		break;
	case prosper::Format::R16_UNorm:
	case prosper::Format::R16_SNorm:
	case prosper::Format::R16_UScaled_PoorCoverage:
	case prosper::Format::R16_SScaled_PoorCoverage:
	case prosper::Format::R16_UInt:
	case prosper::Format::R16_SInt:
	case prosper::Format::R16_SFloat:
	case prosper::Format::R16G16_UNorm:
	case prosper::Format::R16G16_SNorm:
	case prosper::Format::R16G16_UScaled_PoorCoverage:
	case prosper::Format::R16G16_SScaled_PoorCoverage:
	case prosper::Format::R16G16_UInt:
	case prosper::Format::R16G16_SInt:
	case prosper::Format::R16G16_SFloat:
	case prosper::Format::R16G16B16_UNorm_PoorCoverage:
	case prosper::Format::R16G16B16_SNorm_PoorCoverage:
	case prosper::Format::R16G16B16_UScaled_PoorCoverage:
	case prosper::Format::R16G16B16_SScaled_PoorCoverage:
	case prosper::Format::R16G16B16_UInt_PoorCoverage:
	case prosper::Format::R16G16B16_SInt_PoorCoverage:
	case prosper::Format::R16G16B16_SFloat_PoorCoverage:
	case prosper::Format::R16G16B16A16_UNorm:
	case prosper::Format::R16G16B16A16_SNorm:
	case prosper::Format::R16G16B16A16_UScaled_PoorCoverage:
	case prosper::Format::R16G16B16A16_SScaled_PoorCoverage:
	case prosper::Format::R16G16B16A16_UInt:
	case prosper::Format::R16G16B16A16_SInt:
	case prosper::Format::R16G16B16A16_SFloat:
		targetFormat = pragma::image::Format::RGBA16;
		break;
	case prosper::Format::R32_UInt:
	case prosper::Format::R32_SInt:
	case prosper::Format::R32_SFloat:
	case prosper::Format::R32G32_UInt:
	case prosper::Format::R32G32_SInt:
	case prosper::Format::R32G32_SFloat:
	case prosper::Format::R32G32B32_UInt:
	case prosper::Format::R32G32B32_SInt:
	case prosper::Format::R32G32B32_SFloat:
	case prosper::Format::R32G32B32A32_UInt:
	case prosper::Format::R32G32B32A32_SInt:
	case prosper::Format::R32G32B32A32_SFloat:
	case prosper::Format::R64_UInt_PoorCoverage:
	case prosper::Format::R64_SInt_PoorCoverage:
	case prosper::Format::R64_SFloat_PoorCoverage:
	case prosper::Format::R64G64_UInt_PoorCoverage:
	case prosper::Format::R64G64_SInt_PoorCoverage:
	case prosper::Format::R64G64_SFloat_PoorCoverage:
	case prosper::Format::R64G64B64_UInt_PoorCoverage:
	case prosper::Format::R64G64B64_SInt_PoorCoverage:
	case prosper::Format::R64G64B64_SFloat_PoorCoverage:
	case prosper::Format::R64G64B64A64_UInt_PoorCoverage:
	case prosper::Format::R64G64B64A64_SInt_PoorCoverage:
	case prosper::Format::R64G64B64A64_SFloat_PoorCoverage:
	case prosper::Format::B10G11R11_UFloat_Pack32:
	case prosper::Format::E5B9G9R9_UFloat_Pack32:
	case prosper::Format::D16_UNorm:
	case prosper::Format::X8_D24_UNorm_Pack32_PoorCoverage:
	case prosper::Format::D32_SFloat:
	case prosper::Format::D16_UNorm_S8_UInt_PoorCoverage:
	case prosper::Format::D24_UNorm_S8_UInt_PoorCoverage:
	case prosper::Format::D32_SFloat_S8_UInt:
	case prosper::Format::BC5_UNorm_Block:
	case prosper::Format::BC5_SNorm_Block:
	case prosper::Format::BC6H_UFloat_Block:
	case prosper::Format::BC6H_SFloat_Block:
	case prosper::Format::BC7_UNorm_Block:
	case prosper::Format::BC7_SRGB_Block:
		targetFormat = pragma::image::Format::RGBA32;
		break;
	}
	return targetFormat;
}

bool pragma::util::to_image_buffer(prosper::IImage &image, const ToImageBufferInfo &info, std::vector<std::vector<std::shared_ptr<image::ImageBuffer>>> &outImageBuffers)
{
	auto targetFormat = info.targetFormat.has_value() ? *info.targetFormat : determine_target_format(image.GetFormat());
	auto outputFormat = targetFormat;
	prosper::Format dstFormat;
	switch(targetFormat) {
	case image::Format::RGB8:
	case image::Format::RGBA8:
		dstFormat = prosper::Format::R8G8B8A8_UNorm;
		targetFormat = image::Format::RGBA8;
		break;
	case image::Format::RGB16:
	case image::Format::RGBA16:
		dstFormat = prosper::Format::R16G16B16A16_SFloat;
		targetFormat = image::Format::RGBA16;
		break;
	case image::Format::RGB32:
	case image::Format::RGBA32:
		dstFormat = prosper::Format::R32G32B32A32_SFloat;
		targetFormat = image::Format::RGBA32;
		break;
	default:
		return false;
	}

	// Convert the image into the target format
	auto &context = image.GetContext();
	auto &setupCmd = context.GetSetupCommandBuffer();

	auto copyCreateInfo = image.GetCreateInfo();
	auto imgRead = image.shared_from_this();
	auto isOrigImg = false;
	auto srcLayout = prosper::ImageLayout::TransferDstOptimal;
	if(info.stagingImage) {
		imgRead = info.stagingImage->shared_from_this();
		setupCmd->RecordImageBarrier(*imgRead, prosper::ImageLayout::TransferSrcOptimal, prosper::ImageLayout::TransferDstOptimal);
		setupCmd->RecordImageBarrier(image, info.inputImageLayout, prosper::ImageLayout::TransferSrcOptimal);
		image.Copy(*setupCmd, *imgRead);
		setupCmd->RecordImageBarrier(image, prosper::ImageLayout::TransferSrcOptimal, info.inputImageLayout);
	}
	else if(copyCreateInfo.format != dstFormat) {
		copyCreateInfo.format = dstFormat;
		copyCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::DeviceLocal;
		copyCreateInfo.postCreateLayout = prosper::ImageLayout::TransferDstOptimal;
		copyCreateInfo.tiling = prosper::ImageTiling::Optimal; // Needs to be in optimal tiling because some GPUs do not support linear tiling with mipmaps
		setupCmd->RecordImageBarrier(image, info.inputImageLayout, prosper::ImageLayout::TransferSrcOptimal);
		imgRead = image.Copy(*setupCmd, copyCreateInfo);
		setupCmd->RecordImageBarrier(image, prosper::ImageLayout::TransferSrcOptimal, info.inputImageLayout);
	}
	else {
		isOrigImg = true;
		srcLayout = info.inputImageLayout;
	}

	// Copy the image data to a buffer
	uint64_t size = 0;
	uint64_t offset = 0;
	auto numLayers = info.includeLayers ? image.GetLayerCount() : 1;
	auto numMipmaps = info.includeMipmaps ? image.GetMipmapCount() : 1;
	auto sizePerPixel = prosper::util::get_byte_size(dstFormat);

	std::vector<std::vector<size_t>> layerMipmapOffsets {};
	layerMipmapOffsets.resize(numLayers);
	outImageBuffers.resize(numLayers);
	for(auto iLayer = decltype(numLayers) {0u}; iLayer < numLayers; ++iLayer) {
		outImageBuffers.at(iLayer).resize(numMipmaps);
		auto &mipmapOffsets = layerMipmapOffsets.at(iLayer);
		mipmapOffsets.resize(numMipmaps);
		for(auto iMipmap = decltype(numMipmaps) {0u}; iMipmap < numMipmaps; ++iMipmap) {
			mipmapOffsets.at(iMipmap) = size;

			auto extents = image.GetExtents(iMipmap);
			size += extents.width * extents.height * sizePerPixel;
		}
	}
	auto buf = context.AllocateTemporaryBuffer(size);
	if(buf == nullptr) {
		context.FlushSetupCommandBuffer();
		return false; // Buffer allocation failed; Requested size too large?
	}
	setupCmd->RecordImageBarrier(*imgRead, srcLayout, prosper::ImageLayout::TransferSrcOptimal);

	prosper::util::BufferImageCopyInfo copyInfo {};
	copyInfo.dstImageLayout = prosper::ImageLayout::TransferSrcOptimal;

	struct ImageMipmapData {
		uint32_t mipmapIndex = 0u;
		uint64_t bufferOffset = 0ull;
		uint64_t bufferSize = 0ull;
		prosper::Extent2D extents = {};
	};
	struct ImageLayerData {
		std::vector<ImageMipmapData> mipmaps = {};
		uint32_t layerIndex = 0u;
	};
	std::vector<ImageLayerData> layers = {};
	for(auto iLayer = decltype(numLayers) {0u}; iLayer < numLayers; ++iLayer) {
		layers.push_back({});
		auto &layerData = layers.back();
		layerData.layerIndex = iLayer;
		for(auto iMipmap = decltype(numMipmaps) {0u}; iMipmap < numMipmaps; ++iMipmap) {
			layerData.mipmaps.push_back({});
			auto &mipmapData = layerData.mipmaps.back();
			mipmapData.mipmapIndex = iMipmap;
			mipmapData.bufferOffset = layerMipmapOffsets.at(iLayer).at(iMipmap);

			auto extents = image.GetExtents(iMipmap);
			mipmapData.extents = extents;
			mipmapData.bufferSize = extents.width * extents.height * sizePerPixel;
		}
	}
	for(auto &layerData : layers) {
		for(auto &mipmapData : layerData.mipmaps) {
			copyInfo.mipLevel = mipmapData.mipmapIndex;
			copyInfo.baseArrayLayer = layerData.layerIndex;
			copyInfo.bufferOffset = mipmapData.bufferOffset;
			setupCmd->RecordCopyImageToBuffer(copyInfo, *imgRead, prosper::ImageLayout::TransferSrcOptimal, *buf);
		}
	}
	if(isOrigImg && info.finalImageLayout != prosper::ImageLayout::TransferSrcOptimal)
		setupCmd->RecordImageBarrier(*imgRead, prosper::ImageLayout::TransferSrcOptimal, info.finalImageLayout);
	context.FlushSetupCommandBuffer();

	for(auto iLayer = decltype(layers.size()) {0u}; iLayer < layers.size(); ++iLayer) {
		auto &layerData = layers.at(iLayer);
		auto &layerImages = outImageBuffers.at(iLayer);
		for(auto iMipmap = decltype(layerData.mipmaps.size()) {0u}; iMipmap < layerData.mipmaps.size(); ++iMipmap) {
			auto &mipmapData = layerData.mipmaps.at(iMipmap);
			auto &mipmapImg = layerImages.at(iMipmap);
			mipmapImg = image::ImageBuffer::Create(mipmapData.extents.width, mipmapData.extents.height, targetFormat);
			if(buf->Map(mipmapData.bufferOffset, mipmapData.bufferSize, prosper::IBuffer::MapFlags::ReadBit)) {
				buf->Read(0, mipmapData.bufferSize, mipmapImg->GetData());
				buf->Unmap();
			}
			mipmapImg->Convert(outputFormat);
		}
	}
	return true;
}
