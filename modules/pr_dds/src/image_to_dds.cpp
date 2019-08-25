#include "wv_dds.hpp"
#include "pr_dds.hpp"
#include <image/prosper_image.hpp>
#include <prosper_context.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_util.hpp>
#include <wrappers/memory_block.h>
#include <nvtt/nvtt.h>
#include <sharedutils/scope_guard.h>
#include <sharedutils/util_file.h>
#include <fsys/filesystem.h>

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
	}
	static_assert(umath::to_integral(ImageWriteInfo::OutputFormat::Count) == 26);
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
	Anvil::Format dstFormat;
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
		copyCreateInfo.postCreateLayout = Anvil::ImageLayout::GENERAL;
		copyCreateInfo.tiling = Anvil::ImageTiling::LINEAR;
		imgRead = image.Copy(*setupCmd,copyCreateInfo);
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
	auto fullFileName = FileManager::GetProgramPath() +'/' +fileNameWithExt;
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
