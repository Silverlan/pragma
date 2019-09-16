#include "wv_dds.hpp"
#include "pr_dds.hpp"
#include <image/prosper_image.hpp>
#include <buffers/prosper_buffer.hpp>
#include <prosper_context.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_util.hpp>
#include <wrappers/memory_block.h>
#include <nvtt/nvtt.h>
#include <sharedutils/scope_guard.h>
#include <sharedutils/util_file.h>
#include <fsys/filesystem.h>
#include <gli/save.hpp>
#include <gli/texture2d.hpp>

#undef small

#pragma optimize("",off)
static nvtt::Format to_nvtt_enum(ImageWriteInfo::OutputFormat format)
{
	switch(format)
	{
	case ImageWriteInfo::OutputFormat::RGB:
		return nvtt::Format_RGB;
	case ImageWriteInfo::OutputFormat::RGBA:
		return nvtt::Format_RGBA;
	case ImageWriteInfo::OutputFormat::DXT1:
		return nvtt::Format_DXT1;
	case ImageWriteInfo::OutputFormat::DXT1a:
		return nvtt::Format_DXT1a;
	case ImageWriteInfo::OutputFormat::DXT3:
		return nvtt::Format_DXT3;
	case ImageWriteInfo::OutputFormat::DXT5:
		return nvtt::Format_DXT5;
	case ImageWriteInfo::OutputFormat::DXT5n:
		return nvtt::Format_DXT5n;
	case ImageWriteInfo::OutputFormat::BC1:
		return nvtt::Format_BC1;
	case ImageWriteInfo::OutputFormat::BC1a:
		return nvtt::Format_BC1a;
	case ImageWriteInfo::OutputFormat::BC2:
		return nvtt::Format_BC2;
	case ImageWriteInfo::OutputFormat::BC3:
		return nvtt::Format_BC3;
	case ImageWriteInfo::OutputFormat::BC3n:
		return nvtt::Format_BC3n;
	case ImageWriteInfo::OutputFormat::BC4:
		return nvtt::Format_BC4;
	case ImageWriteInfo::OutputFormat::BC5:
		return nvtt::Format_BC5;
	case ImageWriteInfo::OutputFormat::DXT1n:
		return nvtt::Format_DXT1n;
	case ImageWriteInfo::OutputFormat::CTX1:
		return nvtt::Format_CTX1;
	case ImageWriteInfo::OutputFormat::BC6:
		return nvtt::Format_BC6;
	case ImageWriteInfo::OutputFormat::BC7:
		return nvtt::Format_BC7;
	case ImageWriteInfo::OutputFormat::BC3_RGBM:
		return nvtt::Format_BC3_RGBM;
	case ImageWriteInfo::OutputFormat::ETC1:
		return nvtt::Format_ETC1;
	case ImageWriteInfo::OutputFormat::ETC2_R:
		return nvtt::Format_ETC2_R;
	case ImageWriteInfo::OutputFormat::ETC2_RG:
		return nvtt::Format_ETC2_RG;
	case ImageWriteInfo::OutputFormat::ETC2_RGB:
		return nvtt::Format_ETC2_RGB;
	case ImageWriteInfo::OutputFormat::ETC2_RGBA:
		return nvtt::Format_ETC2_RGBA;
	case ImageWriteInfo::OutputFormat::ETC2_RGB_A1:
		return nvtt::Format_ETC2_RGB_A1;
	case ImageWriteInfo::OutputFormat::ETC2_RGBM:
		return nvtt::Format_ETC2_RGBM;
	case ImageWriteInfo::OutputFormat::KeepInputImageFormat:
		break;
	}
	static_assert(umath::to_integral(ImageWriteInfo::OutputFormat::Count) == 27);
	return {};
}

static nvtt::Container to_nvtt_enum(ImageWriteInfo::ContainerFormat format)
{
	switch(format)
	{
	case ImageWriteInfo::ContainerFormat::DDS:
		return nvtt::Container_DDS;
	case ImageWriteInfo::ContainerFormat::KTX:
		return nvtt::Container_KTX;
	}
	static_assert(umath::to_integral(ImageWriteInfo::ContainerFormat::Count) == 2);
	return {};
}

static nvtt::MipmapFilter to_nvtt_enum(ImageWriteInfo::MipmapFilter filter)
{
	switch(filter)
	{
	case ImageWriteInfo::MipmapFilter::Box:
		return nvtt::MipmapFilter_Box;
	case ImageWriteInfo::MipmapFilter::Kaiser:
		return nvtt::MipmapFilter_Kaiser;
	}
	static_assert(umath::to_integral(ImageWriteInfo::ContainerFormat::Count) == 2);
	return {};
}

static nvtt::WrapMode to_nvtt_enum(ImageWriteInfo::WrapMode wrapMode)
{
	switch(wrapMode)
	{
	case ImageWriteInfo::WrapMode::Clamp:
		return nvtt::WrapMode_Clamp;
	case ImageWriteInfo::WrapMode::Repeat:
		return nvtt::WrapMode_Repeat;
	case ImageWriteInfo::WrapMode::Mirror:
		return nvtt::WrapMode_Mirror;
	}
	static_assert(umath::to_integral(ImageWriteInfo::ContainerFormat::Count) == 2);
	return {};
}

struct OutputHandler
	: public nvtt::OutputHandler
{
	OutputHandler(VFilePtrReal f)
		: m_file{f}
	{}
	virtual ~OutputHandler() override {}

	// Indicate the start of a new compressed image that's part of the final texture.
	virtual void beginImage(int size, int width, int height, int depth, int face, int miplevel) override {}

	// Output data. Compressed data is output as soon as it's generated to minimize memory allocations.
	virtual bool writeData(const void * data, int size) override
	{
		return m_file->Write(data,size) == size;
	}

	// Indicate the end of the compressed image. (New in NVTT 2.1)
	virtual void endImage() override {}

private:
	VFilePtrReal m_file = nullptr;
};

struct ErrorHandler
	: public nvtt::ErrorHandler
{
	ErrorHandler(const std::function<void(const std::string&)> &errorHandler)
		: m_errorHandler{errorHandler}
	{}
	virtual ~ErrorHandler() override {};

	// Signal error.
	virtual void error(nvtt::Error e) override
	{
		if(m_errorHandler == nullptr)
			return;
		switch(e)
		{
		case nvtt::Error_Unknown:
			m_errorHandler("Unknown error");
			break;
		case nvtt::Error_InvalidInput:
			m_errorHandler("Invalid input");
			break;
		case nvtt::Error_UnsupportedFeature:
			m_errorHandler("Unsupported feature");
			break;
		case nvtt::Error_CudaError:
			m_errorHandler("Cuda error");
			break;
		case nvtt::Error_FileOpen:
			m_errorHandler("File open error");
			break;
		case nvtt::Error_FileWrite:
			m_errorHandler("File write error");
			break;
		case nvtt::Error_UnsupportedOutputFormat:
			m_errorHandler("Unsupported output format");
			break;
		}
	}
private:
	std::function<void(const std::string&)> m_errorHandler = nullptr;
};

bool save_prosper_image_as_ktx(prosper::Image &image,const std::string &fileName,const ImageWriteInfo &ktxCreateInfo,const std::function<void(const std::string&)> &errorHandler)
{
	std::shared_ptr<prosper::Image> imgRead = image.shared_from_this();
	auto srcFormat = image.GetFormat();
	auto dstFormat = srcFormat;
	nvtt::InputFormat nvttFormat;
	switch(ktxCreateInfo.inputFormat)
	{
	case ImageWriteInfo::InputFormat::R8G8B8A8_UInt:
		dstFormat = Anvil::Format::B8G8R8A8_UNORM;
		nvttFormat = nvtt::InputFormat_BGRA_8UB;
		break;
	case ImageWriteInfo::InputFormat::R16G16B16A16_Float:
		dstFormat = Anvil::Format::R16G16B16A16_SFLOAT;
		nvttFormat = nvtt::InputFormat_RGBA_16F;
		break;
	case ImageWriteInfo::InputFormat::R32G32B32A32_Float:
		dstFormat = Anvil::Format::R32G32B32A32_SFLOAT;
		nvttFormat = nvtt::InputFormat_RGBA_32F;
		break;
	case ImageWriteInfo::InputFormat::R32_Float:
		dstFormat = Anvil::Format::R32_SFLOAT;
		nvttFormat = nvtt::InputFormat_R_32F;
		break;
	case ImageWriteInfo::InputFormat::KeepInputImageFormat:
		break;
	}

	auto path = ufile::get_path_from_filename(fileName);
	FileManager::CreatePath(path.c_str());
	auto fileNameWithExt = fileName;
	ufile::remove_extension_from_filename(fileNameWithExt);
	switch(ktxCreateInfo.containerFormat)
	{
	case ImageWriteInfo::ContainerFormat::DDS:
		fileNameWithExt += ".dds";
		break;
	case ImageWriteInfo::ContainerFormat::KTX:
		fileNameWithExt += ".ktx";
		break;
	}
	auto fullFileName = FileManager::GetProgramPath() +'/' +fileNameWithExt;

	if(ktxCreateInfo.outputFormat == ImageWriteInfo::OutputFormat::KeepInputImageFormat || prosper::util::is_compressed_format(imgRead->GetFormat()))
	{
		// No compression needed, just copy the image data to a buffer and save it directly

		auto extents = imgRead->GetExtents();
		auto numLayers = imgRead->GetLayerCount();
		auto numLevels = imgRead->GetMipmapCount();
		auto &context = image.GetContext();
		auto &setupCmd = context.GetSetupCommandBuffer();

		gli::extent2d gliExtents {extents.width,extents.height};
		gli::texture2d gliTex {static_cast<gli::texture::format_type>(imgRead->GetFormat()),gliExtents,numLevels};

		prosper::util::BufferCreateInfo bufCreateInfo {};
		bufCreateInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUToCPU;
		bufCreateInfo.size = gliTex.size();
		bufCreateInfo.usageFlags = Anvil::BufferUsageFlagBits::TRANSFER_DST_BIT;
		auto buf = prosper::util::create_buffer(context.GetDevice(),bufCreateInfo);
		buf->SetPermanentlyMapped(true);

		// Initialize buffer with image data
		size_t bufferOffset = 0;
		for(auto iLayer=decltype(numLayers){0u};iLayer<numLayers;++iLayer)
		{
			for(auto iMipmap=decltype(numLevels){0u};iMipmap<numLevels;++iMipmap)
			{
				auto extents = imgRead->GetExtents(iMipmap);
				auto mipmapSize = gliTex.size(iMipmap);
				prosper::util::BufferImageCopyInfo copyInfo {};
				copyInfo.baseArrayLayer = iLayer;
				copyInfo.bufferOffset = bufferOffset;
				copyInfo.dstImageLayout = Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL;
				copyInfo.width = extents.width;
				copyInfo.height = extents.height;
				copyInfo.layerCount = 1;
				copyInfo.mipLevel = iMipmap;
				prosper::util::record_copy_image_to_buffer(**setupCmd,copyInfo,*image,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL,*buf);

				bufferOffset += mipmapSize;
			}
		}
		context.FlushSetupCommandBuffer();

		// Copy the data to a gli texture object
		bufferOffset = 0ull;
		for(auto iLayer=decltype(numLayers){0u};iLayer<numLayers;++iLayer)
		{
			for(auto iMipmap=decltype(numLevels){0u};iMipmap<numLevels;++iMipmap)
			{
				auto extents = imgRead->GetExtents(iMipmap);
				auto mipmapSize = gliTex.size(iMipmap);
				auto *dstData = gliTex.data(iLayer,0u /* face */,iMipmap);
				buf->Read(bufferOffset,mipmapSize,dstData);
				bufferOffset += mipmapSize;
			}
		}
		switch(ktxCreateInfo.containerFormat)
		{
		case ImageWriteInfo::ContainerFormat::DDS:
			return gli::save_dds(gliTex,fullFileName);
		case ImageWriteInfo::ContainerFormat::KTX:
			return gli::save_ktx(gliTex,fullFileName);
		}
		return false;
	}

	if(
		image.GetTiling() != Anvil::ImageTiling::LINEAR ||
		(image.GetAnvilImage().get_memory_block()->get_create_info_ptr()->get_memory_features() &Anvil::MemoryFeatureFlagBits::MAPPABLE_BIT) == Anvil::MemoryFeatureFlagBits::NONE ||
		image.GetFormat() != dstFormat
	)
	{
		// Convert the image into the target format
		auto &context = image.GetContext();
		auto &setupCmd = context.GetSetupCommandBuffer();
		prosper::util::ImageCreateInfo copyCreateInfo {};
		image.GetCreateInfo(copyCreateInfo);
		copyCreateInfo.format = dstFormat;
		copyCreateInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUToCPU;
		copyCreateInfo.postCreateLayout = Anvil::ImageLayout::TRANSFER_DST_OPTIMAL;
		copyCreateInfo.tiling = Anvil::ImageTiling::LINEAR;
		prosper::util::record_image_barrier(**setupCmd,*image,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL);
		imgRead = image.Copy(*setupCmd,copyCreateInfo);
		prosper::util::record_image_barrier(**setupCmd,*image,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
		context.FlushSetupCommandBuffer();
	}

	auto szPerPixel = prosper::util::get_byte_size(dstFormat);
	auto extents = image.GetExtents();
	auto size = extents.width *extents.height *szPerPixel;

	nvtt::InputOptions inputOptions {};
	inputOptions.reset();
	inputOptions.setTextureLayout(nvtt::TextureType_2D,extents.width,extents.height);
	inputOptions.setFormat(nvttFormat);
	inputOptions.setWrapMode(to_nvtt_enum(ktxCreateInfo.wrapMode));
	inputOptions.setMipmapFilter(to_nvtt_enum(ktxCreateInfo.mipMapFilter));
	inputOptions.setNormalMap(umath::is_flag_set(ktxCreateInfo.flags,ImageWriteInfo::Flags::NormalMap));
	inputOptions.setNormalizeMipmaps(true);
	inputOptions.setConvertToNormalMap(umath::is_flag_set(ktxCreateInfo.flags,ImageWriteInfo::Flags::ConvertToNormalMap));

	auto numMipmaps = imgRead->GetMipmapCount();
	if(umath::is_flag_set(ktxCreateInfo.flags,ImageWriteInfo::Flags::GenerateMipmaps))
		inputOptions.setMipmapGeneration(true);
	else
		inputOptions.setMipmapGeneration(numMipmaps > 1,numMipmaps -1u);

	auto bCubemap = imgRead->IsCubemap();
	auto texType = bCubemap ? nvtt::TextureType_Cube : nvtt::TextureType_2D;
	auto numLayers = imgRead->GetLayerCount();
	inputOptions.setTextureLayout(texType,extents.width,extents.height);
	for(auto iLayer=decltype(numLayers){0u};iLayer<numLayers;++iLayer)
	{
		for(auto iMipmap=decltype(numMipmaps){0u};iMipmap<numMipmaps;++iMipmap)
		{
			auto subresourceLayout = imgRead->GetSubresourceLayout(iLayer,iMipmap);
			if(subresourceLayout.has_value() == false)
				continue;
			void *data;
			auto *memBlock = (*imgRead)->get_memory_block();
			if(memBlock->map(subresourceLayout->offset,subresourceLayout->size,&data) == false)
				continue;
			uint32_t wMipmap,hMipmap;
			prosper::util::calculate_mipmap_size(extents.width,extents.height,&wMipmap,&hMipmap,iMipmap);
			inputOptions.setMipmapData(data,wMipmap,hMipmap,1,iLayer,iMipmap);
			memBlock->unmap(); // Note: setMipmapData copies the data, so we don't need to keep it mapped
		}
	}

	static_assert(umath::to_integral(ImageWriteInfo::ContainerFormat::Count) == 2);
	/*auto f = FileManager::OpenFile<VFilePtrReal>(fileNameWithExt.c_str(),"wb");
	if(f == nullptr)
	{
		if(errorHandler)
			errorHandler("Unable to write file!");
		return false;
	}*/

	ErrorHandler errHandler {errorHandler};
	//OutputHandler outputHandler {f};
	nvtt::OutputOptions outputOptions {};
	outputOptions.reset();
	outputOptions.setContainer(to_nvtt_enum(ktxCreateInfo.containerFormat));
	outputOptions.setSrgbFlag(umath::is_flag_set(ktxCreateInfo.flags,ImageWriteInfo::Flags::SRGB));
	//outputOptions.setOutputHandler(&outputHandler); // Does not seem to work? TODO: FIXME
	outputOptions.setFileName(fullFileName.c_str());
	outputOptions.setErrorHandler(&errHandler);

	nvtt::CompressionOptions compressionOptions {};
	compressionOptions.reset();
	compressionOptions.setFormat(to_nvtt_enum(ktxCreateInfo.outputFormat));
	compressionOptions.setQuality(nvtt::Quality_Production);

	nvtt::Compressor compressor {};
	auto result = compressor.process(inputOptions,compressionOptions,outputOptions);
	return result;
}
#pragma optimize("",on)
