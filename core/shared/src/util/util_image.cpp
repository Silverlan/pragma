#include "pragma/util/util_image.hpp"
#include "pragma/util/stb_image_write.h"
#include "pragma/util/stb_image.h"
#include <sharedutils/util_image_buffer.hpp>
#include <sharedutils/util_string.h>
#include <fsys/filesystem.h>

#pragma optimize("",off)
std::optional<pragma::image::ImageFormat> pragma::image::string_to_image_output_format(const std::string &str)
{
	if(ustring::compare(str,"PNG",false))
		return ImageFormat::PNG;
	else if(ustring::compare(str,"BMP",false))
		return ImageFormat::BMP;
	else if(ustring::compare(str,"TGA",false))
		return ImageFormat::TGA;
	else if(ustring::compare(str,"JPG",false))
		return ImageFormat::JPG;
	else if(ustring::compare(str,"HDR",false))
		return ImageFormat::HDR;
	return {};
}

std::string pragma::image::get_image_output_format_extension(ImageFormat format)
{
	switch(format)
	{
	case ImageFormat::PNG:
		return "png";
	case ImageFormat::BMP:
		return "bmp";
	case ImageFormat::TGA:
		return "tga";
	case ImageFormat::JPG:
		return "jpg";
	case ImageFormat::HDR:
		return "hdr";
	}
	return "";
}

std::optional<util::ImageBuffer::ToneMapping> pragma::image::string_to_tone_mapping(const std::string &str)
{
	if(ustring::compare(str,"gamma_correction",false))
		return util::ImageBuffer::ToneMapping::GammaCorrection;
	else if(ustring::compare(str,"reinhard",false))
		return util::ImageBuffer::ToneMapping::Reinhard;
	else if(ustring::compare(str,"hejil_richard",false))
		return util::ImageBuffer::ToneMapping::HejilRichard;
	else if(ustring::compare(str,"uncharted",false))
		return util::ImageBuffer::ToneMapping::Uncharted;
	else if(ustring::compare(str,"aces",false))
		return util::ImageBuffer::ToneMapping::Aces;
	else if(ustring::compare(str,"gran_turismo",false))
		return util::ImageBuffer::ToneMapping::GranTurismo;
	return {};
}

std::string pragma::image::get_file_extension(ImageFormat format)
{
	switch(format)
	{
	case ImageFormat::PNG:
		return "png";
	case ImageFormat::BMP:
		return "bmp";
	case ImageFormat::TGA:
		return "tga";
	case ImageFormat::JPG:
		return "jpg";
	case ImageFormat::HDR:
		return "hdr";
	}
	static_assert(umath::to_integral(ImageFormat::Count) == 5);
	return "";
}

std::shared_ptr<::util::ImageBuffer> pragma::image::load_image(std::shared_ptr<VFilePtrInternal> f,PixelFormat pixelFormat)
{
	int width,height,nrComponents;
	stbi_io_callbacks ioCallbacks {};
	ioCallbacks.read = [](void *user,char *data,int size) -> int {
		return static_cast<VFilePtrInternal*>(user)->Read(data,size);
	};
	ioCallbacks.skip = [](void *user,int n) -> void {
		auto *f = static_cast<VFilePtrInternal*>(user);
		f->Seek(f->Tell() +n);
	};
	ioCallbacks.eof = [](void *user) -> int {
		return static_cast<VFilePtrInternal*>(user)->Eof();
	};
	std::shared_ptr<::util::ImageBuffer> imgBuffer = nullptr;
	switch(pixelFormat)
	{
	case PixelFormat::LDR:
	{
		auto *data = stbi_load_from_callbacks(&ioCallbacks,f.get(),&width,&height,&nrComponents,4);
		imgBuffer = data ? util::ImageBuffer::CreateWithCustomDeleter(data,width,height,util::ImageBuffer::Format::RGBA8,[](void *data) {stbi_image_free(data);}) : nullptr;
		break;
	}
	case PixelFormat::HDR:
	{
		auto *data = stbi_load_16_from_callbacks(&ioCallbacks,f.get(),&width,&height,&nrComponents,4);
		imgBuffer = data ? util::ImageBuffer::CreateWithCustomDeleter(data,width,height,util::ImageBuffer::Format::RGBA16,[](void *data) {stbi_image_free(data);}) : nullptr;
		break;
	}
	case PixelFormat::Float:
	{
		auto *data = stbi_loadf_from_callbacks(&ioCallbacks,f.get(),&width,&height,&nrComponents,4);
		imgBuffer = data ? util::ImageBuffer::CreateWithCustomDeleter(data,width,height,util::ImageBuffer::Format::RGBA32,[](void *data) {stbi_image_free(data);}) : nullptr;
		break;
	}
	}
	return imgBuffer;
}

bool pragma::image::save_image(std::shared_ptr<VFilePtrInternalReal> f,::util::ImageBuffer &imgBuffer,ImageFormat format,float quality)
{
	auto *fptr = f.get();

	auto imgFormat = imgBuffer.GetFormat();
	imgFormat = ::util::ImageBuffer::ToRGBFormat(imgFormat);
	if(format != ImageFormat::HDR)
		imgFormat = ::util::ImageBuffer::ToLDRFormat(imgFormat);
	else
		imgFormat = ::util::ImageBuffer::ToFloatFormat(imgFormat);
	imgBuffer.Convert(imgFormat);
	auto w = imgBuffer.GetWidth();
	auto h = imgBuffer.GetHeight();
	auto *data = imgBuffer.GetData();
	auto numChannels = imgBuffer.GetChannelCount();
	int result = 0;
	switch(format)
	{
	case ImageFormat::PNG:
		if(quality >= 0.9f)
			stbi_write_png_compression_level = 0;
		else if(quality >= 0.75f)
			stbi_write_png_compression_level = 6;
		else if(quality >= 0.5f)
			stbi_write_png_compression_level = 8;
		else if(quality >= 0.25f)
			stbi_write_png_compression_level = 10;
		else
			stbi_write_png_compression_level = 12;
		result = stbi_write_png_to_func([](void *context,void *data,int size) {
			static_cast<VFilePtrInternalReal*>(context)->Write(data,size);
		},fptr,w,h,numChannels,data,imgBuffer.GetPixelSize() *w);
		break;
	case ImageFormat::BMP:
		result = stbi_write_bmp_to_func([](void *context,void *data,int size) {
			static_cast<VFilePtrInternalReal*>(context)->Write(data,size);
		},fptr,w,h,numChannels,data);
		break;
	case ImageFormat::TGA:
		result = stbi_write_tga_to_func([](void *context,void *data,int size) {
			static_cast<VFilePtrInternalReal*>(context)->Write(data,size);
		},fptr,w,h,numChannels,data);
		break;
	case ImageFormat::JPG:
		result = stbi_write_jpg_to_func([](void *context,void *data,int size) {
			static_cast<VFilePtrInternalReal*>(context)->Write(data,size);
		},fptr,w,h,numChannels,data,static_cast<int32_t>(quality *100.f));
		break;
	case ImageFormat::HDR:
		result = stbi_write_hdr_to_func([](void *context,void *data,int size) {
			static_cast<VFilePtrInternalReal*>(context)->Write(data,size);
		},fptr,w,h,numChannels,reinterpret_cast<float*>(data));
		break;
	}
	return result != 0;
}
#pragma optimize("",on)
