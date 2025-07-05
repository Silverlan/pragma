// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PRAGMA_UTIL_IMAGE_HPP__
#define __PRAGMA_UTIL_IMAGE_HPP__

#include "pragma/clientdefinitions.h"
#include <prosper_enums.hpp>
#include <pragma/util/resource_watcher.h>
#include <util_image_buffer.hpp>

namespace prosper {
	class Image;
};

namespace util {
	struct ToImageBufferInfo {
		bool includeLayers = false;
		bool includeMipmaps = false;
		std::optional<uimg::Format> targetFormat {};
		prosper::ImageLayout inputImageLayout = prosper::ImageLayout::ShaderReadOnlyOptimal;
		prosper::ImageLayout finalImageLayout = prosper::ImageLayout::TransferSrcOptimal;
		prosper::IImage *stagingImage = nullptr;
	};
	DLLCLIENT bool to_image_buffer(prosper::IImage &image, const ToImageBufferInfo &info, std::vector<std::vector<std::shared_ptr<uimg::ImageBuffer>>> &outImageBuffers);
};

#endif
