#ifndef __PRAGMA_UTIL_IMAGE_HPP__
#define __PRAGMA_UTIL_IMAGE_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/util/resource_watcher.h>
#include <util_image_buffer.hpp>

namespace prosper {class Image;};

namespace util
{
	DLLCLIENT bool to_image_buffer(
		prosper::Image &image,uimg::ImageBuffer::Format targetFormat,std::vector<std::vector<std::shared_ptr<uimg::ImageBuffer>>> &outImageBuffers,
		bool includeLayers=false,bool includeMipmaps=false
	);
	DLLCLIENT bool to_image_buffer(
		prosper::Image &image,std::vector<std::vector<std::shared_ptr<uimg::ImageBuffer>>> &outImageBuffers,
		bool includeLayers=false,bool includeMipmaps=false
	);
};

#endif
