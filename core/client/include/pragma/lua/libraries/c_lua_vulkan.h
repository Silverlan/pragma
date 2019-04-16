#ifndef __C_LUA_VULKAN_H__
#define __C_LUA_VULKAN_H__

#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>
#include <prosper_util.hpp>
#include <image/prosper_render_target.hpp>
#include <image/prosper_sampler.hpp>
#include <wrappers/memory_block.h>

namespace Anvil
{
	class Image;
	class ImageView;
	class Framebuffer;
	class RenderPass;
	class Event;
	class Fence;
	class Semaphore;
	class MemoryBlock;
	class CommandBufferBase;
	class DescriptorSetGroup;
};

namespace prosper
{
	class Texture;
	class Image;
	class ImageView;
	class Buffer;
	class RenderTarget;
	class TimestampQuery;
	class TimerQuery;
	class Sampler;
	class Framebuffer;
	class RenderPass;
	class CommandBuffer;
	class DescriptorSetGroup;
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
				: clearValue{vk::ClearColorValue{std::array<float,4>{color.r /255.f,color.g /255.f,color.b /255.f,color.a /255.f}}}
			{}
			ClearValue(float depth,uint32_t stencil)
				: clearValue{vk::ClearDepthStencilValue{depth,stencil}}
			{}
			ClearValue(float depth)
				: clearValue{vk::ClearDepthStencilValue{depth}}
			{}
			ClearValue()
				: ClearValue(0.f)
			{}
			vk::ClearValue clearValue {};
		};
		using Texture = prosper::Texture;
		using Image = prosper::Image;
		using ImageView = prosper::ImageView;
		using Sampler = prosper::Sampler;
		using Framebuffer = prosper::Framebuffer;
		using RenderPass = prosper::RenderPass;
		using Event = Anvil::Event;
		using Fence = Anvil::Fence;
		using Semaphore = Anvil::Semaphore;
		using Memory = Anvil::MemoryBlock;
		using CommandBuffer = prosper::CommandBuffer;
		using Buffer = prosper::Buffer;
		using DescriptorSet = prosper::DescriptorSetGroup;
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
