// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:util.image;

export import pragma.image;
export import pragma.prosper;

export namespace pragma::util {
	struct ToImageBufferInfo {
		bool includeLayers = false;
		bool includeMipmaps = false;
		std::optional<image::Format> targetFormat {};
		prosper::ImageLayout inputImageLayout = prosper::ImageLayout::ShaderReadOnlyOptimal;
		prosper::ImageLayout finalImageLayout = prosper::ImageLayout::TransferSrcOptimal;
		prosper::IImage *stagingImage = nullptr;
	};
	DLLCLIENT bool to_image_buffer(prosper::IImage &image, const ToImageBufferInfo &info, std::vector<std::vector<std::shared_ptr<image::ImageBuffer>>> &outImageBuffers);
};
