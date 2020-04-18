#include "stdafx_client.h"
#include "pragma/util/util_image.hpp"
#include <prosper_command_buffer.hpp>

extern DLLCENGINE CEngine *c_engine;


bool util::to_image_buffer(
	prosper::Image &image,uimg::ImageBuffer::Format targetFormat,std::vector<std::vector<std::shared_ptr<uimg::ImageBuffer>>> &outImageBuffers,
	bool includeLayers,bool includeMipmaps
)
{
	auto outputFormat = targetFormat;
	Anvil::Format dstFormat;
	switch(targetFormat)
	{
	case uimg::ImageBuffer::Format::RGB8:
	case uimg::ImageBuffer::Format::RGBA8:
		dstFormat = Anvil::Format::R8G8B8A8_UNORM;
		targetFormat = uimg::ImageBuffer::Format::RGBA8;
		break;
	case uimg::ImageBuffer::Format::RGB16:
	case uimg::ImageBuffer::Format::RGBA16:
		dstFormat = Anvil::Format::R16G16B16A16_SFLOAT;
		targetFormat = uimg::ImageBuffer::Format::RGBA16;
		break;
	case uimg::ImageBuffer::Format::RGB32:
	case uimg::ImageBuffer::Format::RGBA32:
		dstFormat = Anvil::Format::R32G32B32A32_SFLOAT;
		targetFormat = uimg::ImageBuffer::Format::RGBA32;
		break;
	default:
		return false;
	}

	// Convert the image into the target format
	auto &context = image.GetContext();
	auto &setupCmd = context.GetSetupCommandBuffer();

	prosper::util::ImageCreateInfo copyCreateInfo {};
	image.GetCreateInfo(copyCreateInfo);
	copyCreateInfo.format = dstFormat;
	copyCreateInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::DeviceLocal;
	copyCreateInfo.postCreateLayout = Anvil::ImageLayout::TRANSFER_DST_OPTIMAL;
	copyCreateInfo.tiling = Anvil::ImageTiling::OPTIMAL; // Needs to be in optimal tiling because some GPUs do not support linear tiling with mipmaps
	prosper::util::record_image_barrier(**setupCmd,*image,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL);
	auto imgRead = image.Copy(*setupCmd,copyCreateInfo);
	prosper::util::record_image_barrier(**setupCmd,*image,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);

	// Copy the image data to a buffer
	uint64_t size = 0;
	uint64_t offset = 0;
	auto numLayers = includeLayers ? image.GetLayerCount() : 1;
	auto numMipmaps = includeMipmaps ? image.GetMipmapCount() : 1;
	auto sizePerPixel = prosper::util::get_byte_size(dstFormat);

	std::vector<std::vector<size_t>> layerMipmapOffsets {};
	layerMipmapOffsets.resize(numLayers);
	outImageBuffers.resize(numLayers);
	for(auto iLayer=decltype(numLayers){0u};iLayer<numLayers;++iLayer)
	{
		outImageBuffers.at(iLayer).resize(numMipmaps);
		auto &mipmapOffsets = layerMipmapOffsets.at(iLayer);
		mipmapOffsets.resize(numMipmaps);
		for(auto iMipmap=decltype(numMipmaps){0u};iMipmap<numMipmaps;++iMipmap)
		{
			mipmapOffsets.at(iMipmap) = size;

			auto extents = image.GetExtents(iMipmap);
			size += extents.width *extents.height *sizePerPixel;
		}
	}
	auto buf = context.AllocateTemporaryBuffer(size);
	if(buf == nullptr)
	{
		context.FlushSetupCommandBuffer();
		return false; // Buffer allocation failed; Requested size too large?
	}
	prosper::util::record_image_barrier(**setupCmd,**imgRead,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL);

	prosper::util::BufferImageCopyInfo copyInfo {};
	copyInfo.dstImageLayout = Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL;

	struct ImageMipmapData
	{
		uint32_t mipmapIndex = 0u;
		uint64_t bufferOffset = 0ull;
		uint64_t bufferSize = 0ull;
		vk::Extent2D extents = {};
	};
	struct ImageLayerData
	{
		std::vector<ImageMipmapData> mipmaps = {};
		uint32_t layerIndex = 0u;
	};
	std::vector<ImageLayerData> layers = {};
	for(auto iLayer=decltype(numLayers){0u};iLayer<numLayers;++iLayer)
	{
		layers.push_back({});
		auto &layerData = layers.back();
		layerData.layerIndex = iLayer;
		for(auto iMipmap=decltype(numMipmaps){0u};iMipmap<numMipmaps;++iMipmap)
		{
			layerData.mipmaps.push_back({});
			auto &mipmapData = layerData.mipmaps.back();
			mipmapData.mipmapIndex = iMipmap;
			mipmapData.bufferOffset = layerMipmapOffsets.at(iLayer).at(iMipmap);

			auto extents = image.GetExtents(iMipmap);
			mipmapData.extents = extents;
			mipmapData.bufferSize = extents.width *extents.height *sizePerPixel;
		}
	}
	for(auto &layerData : layers)
	{
		for(auto &mipmapData : layerData.mipmaps)
		{
			copyInfo.mipLevel = mipmapData.mipmapIndex;
			copyInfo.baseArrayLayer = layerData.layerIndex;
			copyInfo.bufferOffset = mipmapData.bufferOffset;
			prosper::util::record_copy_image_to_buffer(**setupCmd,copyInfo,**imgRead,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL,*buf);
		}
	}
	context.FlushSetupCommandBuffer();

	for(auto iLayer=decltype(layers.size()){0u};iLayer<layers.size();++iLayer)
	{
		auto &layerData = layers.at(iLayer);
		auto &layerImages = outImageBuffers.at(iLayer);
		for(auto iMipmap=decltype(layerData.mipmaps.size()){0u};iMipmap<layerData.mipmaps.size();++iMipmap)
		{
			auto &mipmapData = layerData.mipmaps.at(iMipmap);
			auto &mipmapImg = layerImages.at(iMipmap);
			mipmapImg = uimg::ImageBuffer::Create(mipmapData.extents.width,mipmapData.extents.height,targetFormat);
			if(buf->Map(mipmapData.bufferOffset,mipmapData.bufferSize))
			{
				buf->Read(0,mipmapData.bufferSize,mipmapImg->GetData());
				buf->Unmap();
			}
			mipmapImg->Convert(outputFormat);
		}
	}
	return true;
}
bool util::to_image_buffer(
	prosper::Image &image,std::vector<std::vector<std::shared_ptr<uimg::ImageBuffer>>> &outImageBuffers,
	bool includeLayers,bool includeMipmaps
)
{
	auto format = image.GetFormat();
	auto targetFormat = uimg::ImageBuffer::Format::RGBA8;
	switch(format)
	{
	case Anvil::Format::R8_UNORM:
	case Anvil::Format::R8_SNORM:
	case Anvil::Format::R8_USCALED:
	case Anvil::Format::R8_SSCALED:
	case Anvil::Format::R8_UINT:
	case Anvil::Format::R8_SINT:
	case Anvil::Format::R8_SRGB:
	case Anvil::Format::R8G8_UNORM:
	case Anvil::Format::R8G8_SNORM:
	case Anvil::Format::R8G8_USCALED:
	case Anvil::Format::R8G8_SSCALED:
	case Anvil::Format::R8G8_UINT:
	case Anvil::Format::R8G8_SINT:
	case Anvil::Format::R8G8_SRGB:
	case Anvil::Format::R8G8B8_UNORM:
	case Anvil::Format::R8G8B8_SNORM:
	case Anvil::Format::R8G8B8_USCALED:
	case Anvil::Format::R8G8B8_SSCALED:
	case Anvil::Format::R8G8B8_UINT:
	case Anvil::Format::R8G8B8_SINT:
	case Anvil::Format::R8G8B8_SRGB:
	case Anvil::Format::B8G8R8_UNORM:
	case Anvil::Format::B8G8R8_SNORM:
	case Anvil::Format::B8G8R8_USCALED:
	case Anvil::Format::B8G8R8_SSCALED:
	case Anvil::Format::B8G8R8_UINT:
	case Anvil::Format::B8G8R8_SINT:
	case Anvil::Format::B8G8R8_SRGB:
	case Anvil::Format::R8G8B8A8_UNORM:
	case Anvil::Format::R8G8B8A8_SNORM:
	case Anvil::Format::R8G8B8A8_USCALED:
	case Anvil::Format::R8G8B8A8_SSCALED:
	case Anvil::Format::R8G8B8A8_UINT:
	case Anvil::Format::R8G8B8A8_SINT:
	case Anvil::Format::R8G8B8A8_SRGB:
	case Anvil::Format::B8G8R8A8_UNORM:
	case Anvil::Format::B8G8R8A8_SNORM:
	case Anvil::Format::B8G8R8A8_USCALED:
	case Anvil::Format::B8G8R8A8_SSCALED:
	case Anvil::Format::B8G8R8A8_UINT:
	case Anvil::Format::B8G8R8A8_SINT:
	case Anvil::Format::B8G8R8A8_SRGB:
	case Anvil::Format::A8B8G8R8_UNORM_PACK32:
	case Anvil::Format::A8B8G8R8_SNORM_PACK32:
	case Anvil::Format::A8B8G8R8_USCALED_PACK32:
	case Anvil::Format::A8B8G8R8_SSCALED_PACK32:
	case Anvil::Format::A8B8G8R8_UINT_PACK32:
	case Anvil::Format::A8B8G8R8_SINT_PACK32:
	case Anvil::Format::A8B8G8R8_SRGB_PACK32:
	case Anvil::Format::A2R10G10B10_UNORM_PACK32:
	case Anvil::Format::A2R10G10B10_SNORM_PACK32:
	case Anvil::Format::A2R10G10B10_USCALED_PACK32:
	case Anvil::Format::A2R10G10B10_SSCALED_PACK32:
	case Anvil::Format::A2R10G10B10_UINT_PACK32:
	case Anvil::Format::A2R10G10B10_SINT_PACK32:
	case Anvil::Format::A2B10G10R10_UNORM_PACK32:
	case Anvil::Format::A2B10G10R10_SNORM_PACK32:
	case Anvil::Format::A2B10G10R10_USCALED_PACK32:
	case Anvil::Format::A2B10G10R10_SSCALED_PACK32:
	case Anvil::Format::A2B10G10R10_UINT_PACK32:
	case Anvil::Format::A2B10G10R10_SINT_PACK32:
	case Anvil::Format::S8_UINT:
	case Anvil::Format::BC1_RGB_UNORM_BLOCK:
	case Anvil::Format::BC1_RGB_SRGB_BLOCK:
	case Anvil::Format::BC1_RGBA_UNORM_BLOCK:
	case Anvil::Format::BC1_RGBA_SRGB_BLOCK:
	case Anvil::Format::BC2_UNORM_BLOCK:
	case Anvil::Format::BC2_SRGB_BLOCK:
	case Anvil::Format::BC3_UNORM_BLOCK:
	case Anvil::Format::BC3_SRGB_BLOCK:
	case Anvil::Format::BC4_UNORM_BLOCK:
	case Anvil::Format::BC4_SNORM_BLOCK:
		targetFormat = uimg::ImageBuffer::Format::RGBA8;
		break;
	case Anvil::Format::R16_UNORM:
	case Anvil::Format::R16_SNORM:
	case Anvil::Format::R16_USCALED:
	case Anvil::Format::R16_SSCALED:
	case Anvil::Format::R16_UINT:
	case Anvil::Format::R16_SINT:
	case Anvil::Format::R16_SFLOAT:
	case Anvil::Format::R16G16_UNORM:
	case Anvil::Format::R16G16_SNORM:
	case Anvil::Format::R16G16_USCALED:
	case Anvil::Format::R16G16_SSCALED:
	case Anvil::Format::R16G16_UINT:
	case Anvil::Format::R16G16_SINT:
	case Anvil::Format::R16G16_SFLOAT:
	case Anvil::Format::R16G16B16_UNORM:
	case Anvil::Format::R16G16B16_SNORM:
	case Anvil::Format::R16G16B16_USCALED:
	case Anvil::Format::R16G16B16_SSCALED:
	case Anvil::Format::R16G16B16_UINT:
	case Anvil::Format::R16G16B16_SINT:
	case Anvil::Format::R16G16B16_SFLOAT:
	case Anvil::Format::R16G16B16A16_UNORM:
	case Anvil::Format::R16G16B16A16_SNORM:
	case Anvil::Format::R16G16B16A16_USCALED:
	case Anvil::Format::R16G16B16A16_SSCALED:
	case Anvil::Format::R16G16B16A16_UINT:
	case Anvil::Format::R16G16B16A16_SINT:
	case Anvil::Format::R16G16B16A16_SFLOAT:
		targetFormat = uimg::ImageBuffer::Format::RGBA16;
		break;
	case Anvil::Format::R32_UINT:
	case Anvil::Format::R32_SINT:
	case Anvil::Format::R32_SFLOAT:
	case Anvil::Format::R32G32_UINT:
	case Anvil::Format::R32G32_SINT:
	case Anvil::Format::R32G32_SFLOAT:
	case Anvil::Format::R32G32B32_UINT:
	case Anvil::Format::R32G32B32_SINT:
	case Anvil::Format::R32G32B32_SFLOAT:
	case Anvil::Format::R32G32B32A32_UINT:
	case Anvil::Format::R32G32B32A32_SINT:
	case Anvil::Format::R32G32B32A32_SFLOAT:
	case Anvil::Format::R64_UINT:
	case Anvil::Format::R64_SINT:
	case Anvil::Format::R64_SFLOAT:
	case Anvil::Format::R64G64_UINT:
	case Anvil::Format::R64G64_SINT:
	case Anvil::Format::R64G64_SFLOAT:
	case Anvil::Format::R64G64B64_UINT:
	case Anvil::Format::R64G64B64_SINT:
	case Anvil::Format::R64G64B64_SFLOAT:
	case Anvil::Format::R64G64B64A64_UINT:
	case Anvil::Format::R64G64B64A64_SINT:
	case Anvil::Format::R64G64B64A64_SFLOAT:
	case Anvil::Format::B10G11R11_UFLOAT_PACK32:
	case Anvil::Format::E5B9G9R9_UFLOAT_PACK32:
	case Anvil::Format::D16_UNORM:
	case Anvil::Format::X8_D24_UNORM_PACK32:
	case Anvil::Format::D32_SFLOAT:
	case Anvil::Format::D16_UNORM_S8_UINT:
	case Anvil::Format::D24_UNORM_S8_UINT:
	case Anvil::Format::D32_SFLOAT_S8_UINT:
	case Anvil::Format::BC5_UNORM_BLOCK:
	case Anvil::Format::BC5_SNORM_BLOCK:
	case Anvil::Format::BC6H_UFLOAT_BLOCK:
	case Anvil::Format::BC6H_SFLOAT_BLOCK:
	case Anvil::Format::BC7_UNORM_BLOCK:
	case Anvil::Format::BC7_SRGB_BLOCK:
		targetFormat = uimg::ImageBuffer::Format::RGBA32;
		break;
	}
	return to_image_buffer(image,targetFormat,outImageBuffers,includeLayers,includeMipmaps);
}

