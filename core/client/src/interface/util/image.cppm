// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"

export module pragma.client:util.image;

export import pragma.image;
export import pragma.prosper;

export namespace util {
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
