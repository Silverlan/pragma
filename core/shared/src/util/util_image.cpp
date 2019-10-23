#include "pragma/util/util_image.hpp"
#include "pragma/util/stb_image_write.h"
#include <sharedutils/util_image_buffer.hpp>
#include <sharedutils/util_string.h>
#include <fsys/filesystem.h>

#pragma optimize("",off)
std::optional<pragma::image::ImageOutputFormat> pragma::image::string_to_image_output_format(const std::string &str)
{
	if(ustring::compare(str,"PNG",false))
		return ImageOutputFormat::PNG;
	else if(ustring::compare(str,"BMP",false))
		return ImageOutputFormat::BMP;
	else if(ustring::compare(str,"TGA",false))
		return ImageOutputFormat::TGA;
	else if(ustring::compare(str,"JPG",false))
		return ImageOutputFormat::JPG;
	else if(ustring::compare(str,"HDR",false))
		return ImageOutputFormat::HDR;
	return {};
}

std::string pragma::image::get_image_output_format_extension(ImageOutputFormat format)
{
	switch(format)
	{
	case ImageOutputFormat::PNG:
		return "png";
	case ImageOutputFormat::BMP:
		return "bmp";
	case ImageOutputFormat::TGA:
		return "tga";
	case ImageOutputFormat::JPG:
		return "jpg";
	case ImageOutputFormat::HDR:
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

bool pragma::image::save_image(std::shared_ptr<VFilePtrInternalReal> f,::util::ImageBuffer &imgBuffer,ImageOutputFormat format,float quality)
{
	auto *fptr = f.get();

	auto imgFormat = imgBuffer.GetFormat();
	imgFormat = ::util::ImageBuffer::ToRGBFormat(imgFormat);
	if(format != ImageOutputFormat::HDR)
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
	case ImageOutputFormat::PNG:
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
	case ImageOutputFormat::BMP:
		result = stbi_write_bmp_to_func([](void *context,void *data,int size) {
			static_cast<VFilePtrInternalReal*>(context)->Write(data,size);
		},fptr,w,h,numChannels,data);
		break;
	case ImageOutputFormat::TGA:
		result = stbi_write_tga_to_func([](void *context,void *data,int size) {
			static_cast<VFilePtrInternalReal*>(context)->Write(data,size);
		},fptr,w,h,numChannels,data);
		break;
	case ImageOutputFormat::JPG:
		result = stbi_write_jpg_to_func([](void *context,void *data,int size) {
			static_cast<VFilePtrInternalReal*>(context)->Write(data,size);
		},fptr,w,h,numChannels,data,static_cast<int32_t>(quality *100.f));
		break;
	case ImageOutputFormat::HDR:
		result = stbi_write_hdr_to_func([](void *context,void *data,int size) {
			static_cast<VFilePtrInternalReal*>(context)->Write(data,size);
		},fptr,w,h,numChannels,reinterpret_cast<float*>(data));
		break;
	}
	return result != 0;
}
#pragma optimize("",on)
