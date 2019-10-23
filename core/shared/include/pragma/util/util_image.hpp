#ifndef __PRAGMA_UTIL_IMAGE_HPP__
#define __PRAGMA_UTIL_IMAGE_HPP__

#include "pragma/networkdefinitions.h"
#include <sharedutils/util_image_buffer.hpp>
#include <cinttypes>
#include <optional>
#include <string>
#include <memory>

class VFilePtrInternalReal;
namespace pragma::image
{
	enum class ImageOutputFormat : uint8_t
	{
		PNG = 0,
		BMP,
		TGA,
		JPG,
		HDR
	};
	DLLNETWORK bool save_image(std::shared_ptr<VFilePtrInternalReal> f,::util::ImageBuffer &imgBuffer,ImageOutputFormat format,float quality=1.f);
	DLLNETWORK std::optional<ImageOutputFormat> string_to_image_output_format(const std::string &str);
	DLLNETWORK std::string get_image_output_format_extension(ImageOutputFormat format);

	DLLNETWORK std::optional<util::ImageBuffer::ToneMapping> string_to_tone_mapping(const std::string &str);
};

#endif
