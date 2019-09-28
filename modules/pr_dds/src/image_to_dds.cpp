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
#include <sharedutils/util_image_buffer.hpp>
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

static nvtt::Container to_nvtt_enum(ImageWriteInfo::ContainerFormat format,ImageWriteInfo::OutputFormat imgFormat)
{
	switch(format)
	{
	case ImageWriteInfo::ContainerFormat::DDS:
	{
		if(imgFormat == ImageWriteInfo::OutputFormat::BC6 || imgFormat == ImageWriteInfo::OutputFormat::BC7)
			return nvtt::Container_DDS10; // These formats are only supported by DDS10
		return nvtt::Container_DDS;
	}
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

static std::string get_full_name(const std::string &fileName,ImageWriteInfo::ContainerFormat containerFormat)
{
	auto path = ufile::get_path_from_filename(fileName);
	FileManager::CreatePath(path.c_str());
	auto fileNameWithExt = fileName;
	ufile::remove_extension_from_filename(fileNameWithExt);
	switch(containerFormat)
	{
	case ImageWriteInfo::ContainerFormat::DDS:
		fileNameWithExt += ".dds";
		break;
	case ImageWriteInfo::ContainerFormat::KTX:
		fileNameWithExt += ".ktx";
		break;
	}
	return FileManager::GetProgramPath() +'/' +fileNameWithExt;
}

static Anvil::Format get_anvil_format(ImageWriteInfo::InputFormat format)
{
	Anvil::Format anvFormat;
	switch(format)
	{
	case ImageWriteInfo::InputFormat::R8G8B8A8_UInt:
		anvFormat = Anvil::Format::B8G8R8A8_UNORM;
		break;
	case ImageWriteInfo::InputFormat::R16G16B16A16_Float:
		anvFormat = Anvil::Format::R16G16B16A16_SFLOAT;
		break;
	case ImageWriteInfo::InputFormat::R32G32B32A32_Float:
		anvFormat = Anvil::Format::R32G32B32A32_SFLOAT;
		break;
	case ImageWriteInfo::InputFormat::R32_Float:
		anvFormat = Anvil::Format::R32_SFLOAT;
		break;
	case ImageWriteInfo::InputFormat::KeepInputImageFormat:
		break;
	}
	return anvFormat;
}

static nvtt::InputFormat get_nvtt_format(ImageWriteInfo::InputFormat format)
{
	nvtt::InputFormat nvttFormat;
	switch(format)
	{
	case ImageWriteInfo::InputFormat::R8G8B8A8_UInt:
		nvttFormat = nvtt::InputFormat_BGRA_8UB;
		break;
	case ImageWriteInfo::InputFormat::R16G16B16A16_Float:
		nvttFormat = nvtt::InputFormat_RGBA_16F;
		break;
	case ImageWriteInfo::InputFormat::R32G32B32A32_Float:
		nvttFormat = nvtt::InputFormat_RGBA_32F;
		break;
	case ImageWriteInfo::InputFormat::R32_Float:
		nvttFormat = nvtt::InputFormat_R_32F;
		break;
	case ImageWriteInfo::InputFormat::KeepInputImageFormat:
		break;
	}
	return nvttFormat;
}

static bool save_image(
	const std::function<const uint8_t*(uint32_t,uint32_t,std::function<void()>&)> &fGetImgData,uint32_t width,uint32_t height,Anvil::Format format,
	uint32_t numLayers,uint32_t numMipmaps,bool cubemap,const std::string &fileName,
	const ImageWriteInfo &ktxCreateInfo,const std::function<void(const std::string&)> &errorHandler
)
{
	auto szPerPixel = prosper::util::get_byte_size(format);
	auto size = width *height *szPerPixel;

	auto nvttFormat = get_nvtt_format(ktxCreateInfo.inputFormat);
	nvtt::InputOptions inputOptions {};
	inputOptions.reset();
	inputOptions.setTextureLayout(nvtt::TextureType_2D,width,height);
	inputOptions.setFormat(nvttFormat);
	inputOptions.setWrapMode(to_nvtt_enum(ktxCreateInfo.wrapMode));
	inputOptions.setMipmapFilter(to_nvtt_enum(ktxCreateInfo.mipMapFilter));

	if(umath::is_flag_set(ktxCreateInfo.flags,ImageWriteInfo::Flags::GenerateMipmaps))
		inputOptions.setMipmapGeneration(true);
	else
		inputOptions.setMipmapGeneration(numMipmaps > 1,numMipmaps -1u);

	auto texType = cubemap ? nvtt::TextureType_Cube : nvtt::TextureType_2D;
	auto alphaMode = ktxCreateInfo.alphaMode;
	if(ktxCreateInfo.outputFormat == ImageWriteInfo::OutputFormat::BC6)
		alphaMode = ImageWriteInfo::AlphaMode::Transparency;
	inputOptions.setTextureLayout(texType,width,height);
	for(auto iLayer=decltype(numLayers){0u};iLayer<numLayers;++iLayer)
	{
		for(auto iMipmap=decltype(numMipmaps){0u};iMipmap<numMipmaps;++iMipmap)
		{
			std::function<void(void)> deleter = nullptr;
			auto *data = fGetImgData(iLayer,iMipmap,deleter);
			if(data == nullptr)
				continue;
			uint32_t wMipmap,hMipmap;
			prosper::util::calculate_mipmap_size(width,height,&wMipmap,&hMipmap,iMipmap);
			inputOptions.setMipmapData(data,wMipmap,hMipmap,1,iLayer,iMipmap);

			if(alphaMode == ImageWriteInfo::AlphaMode::Auto)
			{
				// Determine whether there are any alpha values < 1
				auto numPixels = wMipmap *hMipmap;
				for(auto i=decltype(numPixels){0u};i<numPixels;++i)
				{
					float alpha = 1.f;
					switch(nvttFormat)
					{
					case nvtt::InputFormat_BGRA_8UB:
						alpha = (data +i *4)[3] /static_cast<float>(std::numeric_limits<uint8_t>::max());
						break;
					case nvtt::InputFormat_RGBA_16F:
						alpha = umath::float16_to_float32_glm((reinterpret_cast<const uint16_t*>(data) +i *4)[3]);
						break;
					case nvtt::InputFormat_RGBA_32F:
						alpha = (reinterpret_cast<const float*>(data) +i *4)[3];
						break;
					case nvtt::InputFormat_R_32F:
						break;
					}
					if(alpha < 0.999f)
					{
						alphaMode = ImageWriteInfo::AlphaMode::Transparency;
						break;
					}
				}
			}
			if(deleter)
				deleter();
		}
	}
	inputOptions.setAlphaMode((alphaMode == ImageWriteInfo::AlphaMode::Transparency) ? nvtt::AlphaMode::AlphaMode_Transparency : nvtt::AlphaMode::AlphaMode_None);

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
	outputOptions.setContainer(to_nvtt_enum(ktxCreateInfo.containerFormat,ktxCreateInfo.outputFormat));
	outputOptions.setSrgbFlag(umath::is_flag_set(ktxCreateInfo.flags,ImageWriteInfo::Flags::SRGB));
	//outputOptions.setOutputHandler(&outputHandler); // Does not seem to work? TODO: FIXME
	outputOptions.setFileName(get_full_name(fileName,ktxCreateInfo.containerFormat).c_str());
	outputOptions.setErrorHandler(&errHandler);

	auto nvttOutputFormat = to_nvtt_enum(ktxCreateInfo.outputFormat);
	nvtt::CompressionOptions compressionOptions {};
	compressionOptions.reset();
	compressionOptions.setFormat(nvttOutputFormat);
	compressionOptions.setQuality(nvtt::Quality_Production);

	// These settings are from the standalone nvtt nvcompress application
	switch(nvttOutputFormat)
	{
	case nvtt::Format_BC2:
		// Dither alpha when using BC2.
		compressionOptions.setQuantization(/*color dithering*/false, /*alpha dithering*/true, /*binary alpha*/false);
		break;
	case nvtt::Format_BC1a:
		// Binary alpha when using BC1a.
		compressionOptions.setQuantization(/*color dithering*/false, /*alpha dithering*/true, /*binary alpha*/true, 127);
		break;
	case nvtt::Format_BC6:
		compressionOptions.setPixelType(nvtt::PixelType_UnsignedFloat);
		break;
	case nvtt::Format_DXT1n:
		compressionOptions.setColorWeights(1, 1, 0);
		break;
	}

	if(umath::is_flag_set(ktxCreateInfo.flags,ImageWriteInfo::Flags::NormalMap))
	{
		inputOptions.setNormalMap(true);
		inputOptions.setConvertToNormalMap(false);
		inputOptions.setGamma(1.0f, 1.0f);
		inputOptions.setNormalizeMipmaps(true);
	}
	else if(umath::is_flag_set(ktxCreateInfo.flags,ImageWriteInfo::Flags::ConvertToNormalMap))
	{
		inputOptions.setNormalMap(false);
		inputOptions.setConvertToNormalMap(true);
		inputOptions.setHeightEvaluation(1.0f/3.0f, 1.0f/3.0f, 1.0f/3.0f, 0.0f);
		//inputOptions.setNormalFilter(1.0f, 0, 0, 0);
		//inputOptions.setNormalFilter(0.0f, 0, 0, 1);
		inputOptions.setGamma(1.0f, 1.0f);
		inputOptions.setNormalizeMipmaps(true);
	}
	else
	{
		inputOptions.setNormalMap(false);
		inputOptions.setConvertToNormalMap(false);
		inputOptions.setGamma(2.2f, 2.2f);
		inputOptions.setNormalizeMipmaps(false);
	}

	nvtt::Compressor compressor {};
	compressor.enableCudaAcceleration(true);
	return compressor.process(inputOptions,compressionOptions,outputOptions);
}

bool Lua::dds::save_image(
	util::ImageBuffer &imgBuffer,const std::string &fileName,const ImageWriteInfo &ktxCreateInfo,const std::function<void(const std::string&)> &errorHandler
)
{
	constexpr auto numLayers = 1u;
	constexpr auto numMipmaps = 1u;
	return ::save_image([&imgBuffer](uint32_t iLayer,uint32_t iMipmap,std::function<void(void)> &outDeleter) -> const uint8_t* {
		return static_cast<uint8_t*>(imgBuffer.GetData());
	},imgBuffer.GetWidth(),imgBuffer.GetHeight(),get_anvil_format(ktxCreateInfo.inputFormat),numLayers,numMipmaps,false /* cubemap */,fileName,ktxCreateInfo,errorHandler);
}

bool Lua::dds::save_image(
	const std::vector<std::vector<const void*>> &imgLayerMipmapData,uint32_t width,uint32_t height,const std::string &fileName,
	const ImageWriteInfo &ktxCreateInfo,const std::function<void(const std::string&)> &errorHandler
)
{
	auto numLayers = imgLayerMipmapData.size();
	auto numMipmaps = imgLayerMipmapData.empty() ? 1 : imgLayerMipmapData.front().size();
	return ::save_image([&imgLayerMipmapData](uint32_t iLayer,uint32_t iMipmap,std::function<void(void)> &outDeleter) -> const uint8_t* {
		if(iLayer >= imgLayerMipmapData.size())
			return nullptr;
		auto &mipmapData = imgLayerMipmapData.at(iLayer);
		if(iMipmap >= mipmapData.size())
			return nullptr;
		return static_cast<const uint8_t*>(mipmapData.at(iMipmap));
	},width,height,get_anvil_format(ktxCreateInfo.inputFormat),numLayers,numMipmaps,false /* cubemap */,fileName,ktxCreateInfo,errorHandler);
}

bool Lua::dds::save_image(prosper::Image &image,const std::string &fileName,const ImageWriteInfo &ktxCreateInfo,const std::function<void(const std::string&)> &errorHandler)
{
	std::shared_ptr<prosper::Image> imgRead = image.shared_from_this();
	auto srcFormat = image.GetFormat();
	auto dstFormat = srcFormat;
	if(ktxCreateInfo.inputFormat != ImageWriteInfo::InputFormat::KeepInputImageFormat)
		dstFormat = get_anvil_format(ktxCreateInfo.inputFormat);
	auto nvttFormat = get_nvtt_format(ktxCreateInfo.inputFormat);
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
		auto fullFileName = get_full_name(fileName,ktxCreateInfo.containerFormat);
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
	auto extents = imgRead->GetExtents();
	return ::save_image([imgRead](uint32_t iLayer,uint32_t iMipmap,std::function<void(void)> &outDeleter) -> const uint8_t* {
		auto subresourceLayout = imgRead->GetSubresourceLayout(iLayer,iMipmap);
		if(subresourceLayout.has_value() == false)
			return nullptr;
		void *data;
		auto *memBlock = (*imgRead)->get_memory_block();
		if(memBlock->map(subresourceLayout->offset,subresourceLayout->size,&data) == false)
			return nullptr;
		outDeleter = [memBlock]() {
			memBlock->unmap(); // Note: setMipmapData copies the data, so we don't need to keep it mapped
		};
		return static_cast<uint8_t*>(data);
	},extents.width,extents.height,dstFormat,imgRead->GetLayerCount(),imgRead->GetMipmapCount(),imgRead->IsCubemap(),fileName,ktxCreateInfo,errorHandler);
}
#pragma optimize("",on)
