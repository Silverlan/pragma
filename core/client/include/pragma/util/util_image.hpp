/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __PRAGMA_UTIL_IMAGE_HPP__
#define __PRAGMA_UTIL_IMAGE_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/util/resource_watcher.h>
#include <util_image_buffer.hpp>

namespace prosper {class Image;};

namespace util
{
	DLLCLIENT bool to_image_buffer(
		prosper::IImage &image,uimg::ImageBuffer::Format targetFormat,std::vector<std::vector<std::shared_ptr<uimg::ImageBuffer>>> &outImageBuffers,
		bool includeLayers=false,bool includeMipmaps=false
	);
	DLLCLIENT bool to_image_buffer(
		prosper::IImage &image,std::vector<std::vector<std::shared_ptr<uimg::ImageBuffer>>> &outImageBuffers,
		bool includeLayers=false,bool includeMipmaps=false
	);
};

#endif
