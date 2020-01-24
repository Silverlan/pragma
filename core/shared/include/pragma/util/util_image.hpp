#ifndef __PRAGMA_UTIL_IMAGE_HPP__
#define __PRAGMA_UTIL_IMAGE_HPP__

#include "pragma/networkdefinitions.h"
#include <sharedutils/util_image_buffer.hpp>
#include <cinttypes>
#include <optional>
#include <string>
#include <memory>

class VFilePtrInternalReal;
class VFilePtrInternal;
namespace pragma::image
{
	enum class ImageFormat : uint8_t
	{
		PNG = 0,
		BMP,
		TGA,
		JPG,
		HDR,
		Count
	};
	enum class PixelFormat : uint8_t
	{
		LDR = 0,
		HDR,
		Float
	};
	DLLNETWORK std::string get_file_extension(ImageFormat format);
	DLLNETWORK std::shared_ptr<::util::ImageBuffer> load_image(std::shared_ptr<VFilePtrInternal> f,PixelFormat pixelFormat=PixelFormat::LDR);
	DLLNETWORK bool save_image(std::shared_ptr<VFilePtrInternalReal> f,::util::ImageBuffer &imgBuffer,ImageFormat format,float quality=1.f);
	DLLNETWORK std::optional<ImageFormat> string_to_image_output_format(const std::string &str);
	DLLNETWORK std::string get_image_output_format_extension(ImageFormat format);

	DLLNETWORK std::optional<util::ImageBuffer::ToneMapping> string_to_tone_mapping(const std::string &str);
};

#endif
