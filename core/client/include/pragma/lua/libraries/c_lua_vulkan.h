/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_LUA_VULKAN_H__
#define __C_LUA_VULKAN_H__

#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>
#include <prosper_util.hpp>
#include <image/prosper_render_target.hpp>
#include <image/prosper_sampler.hpp>

namespace prosper
{
	class Texture;
	class IImage;
	class IImageView;
	class IBuffer;
	class RenderTarget;
	class TimestampQuery;
	class TimerQuery;
	class ISampler;
	class IFramebuffer;
	class RenderPass;
	class ICommandBuffer;
	class IDescriptorSetGroup;
};

namespace vk
{
	union ClearValue;
};

namespace Lua
{
	namespace Vulkan
	{
		struct DLLCLIENT ClearValue
		{
		public:
			ClearValue(const Color &color)
				: clearValue{prosper::ClearColorValue{std::array<float,4>{color.r /255.f,color.g /255.f,color.b /255.f,color.a /255.f}}}
			{}
			ClearValue(float depth,uint32_t stencil)
				: clearValue{prosper::ClearDepthStencilValue{depth,stencil}}
			{}
			ClearValue(float depth)
				: clearValue{prosper::ClearDepthStencilValue{depth}}
			{}
			ClearValue()
				: ClearValue(0.f)
			{}
			prosper::ClearValue clearValue {};
		};
		using Texture = prosper::Texture;
		using Image = prosper::IImage;
		using ImageView = prosper::IImageView;
		using Sampler = prosper::ISampler;
		using Framebuffer = prosper::IFramebuffer;
		using RenderPass = prosper::IRenderPass;
		using Event = prosper::IEvent;
		using Fence = prosper::IFence;
		//using Semaphore = prosper::Semaphore;
		//using Memory = prosper::MemoryBlock;
		using CommandBuffer = prosper::ICommandBuffer;
		using Buffer = prosper::IBuffer;
		using DescriptorSet = prosper::IDescriptorSetGroup;
		using RenderTarget = prosper::RenderTarget;
		using TimestampQuery = prosper::TimestampQuery;
		using TimerQuery = prosper::TimerQuery;
		struct DLLCLIENT RenderPassInfo
		{
			RenderPassInfo(const std::shared_ptr<RenderTarget> &renderTarget)
				: renderTarget{renderTarget}
			{}
			std::shared_ptr<RenderTarget> renderTarget;
			std::vector<ClearValue> clearValues = {};
			std::optional<uint32_t> layerId = {};
			std::shared_ptr<RenderPass> renderPass = nullptr;
		};
	};
};

namespace prosper
{
	namespace util
	{
		struct TextureCreateInfo;
		struct SamplerCreateInfo;
		struct RenderTargetCreateInfo;
		struct BufferCreateInfo;
		struct ImageCreateInfo;
		struct ImageViewCreateInfo;
		struct RenderPassCreateInfo;
		struct BlitInfo;
		struct CopyInfo;
		struct PipelineBarrierInfo;
		struct ClearImageInfo;
	};
};

#endif
