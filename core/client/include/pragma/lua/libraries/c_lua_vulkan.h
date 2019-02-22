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
		using Texture = std::shared_ptr<prosper::Texture>;
		using Image = std::shared_ptr<prosper::Image>;
		using ImageView = std::shared_ptr<prosper::ImageView>;
		using Sampler = std::shared_ptr<prosper::Sampler>;
		using Framebuffer = std::shared_ptr<prosper::Framebuffer>;
		using RenderPass = std::shared_ptr<prosper::RenderPass>;
		using Event = std::shared_ptr<Anvil::Event>;
		using Fence = std::shared_ptr<Anvil::Fence>;
		using Semaphore = std::shared_ptr<Anvil::Semaphore>;
		using Memory = Anvil::MemoryBlock;
		using CommandBuffer = std::shared_ptr<prosper::CommandBuffer>;
		using Buffer = std::shared_ptr<prosper::Buffer>;
		using DescriptorSet = std::shared_ptr<prosper::DescriptorSetGroup>;
		using RenderTarget = std::shared_ptr<prosper::RenderTarget>;
		using TimestampQuery = std::shared_ptr<prosper::TimestampQuery>;
		using TimerQuery = std::shared_ptr<prosper::TimerQuery>;
		struct DLLCLIENT RenderPassInfo
		{
			RenderPassInfo(const RenderTarget &renderTarget)
				: renderTarget{renderTarget}
			{}
			RenderTarget renderTarget;
			std::vector<ClearValue> clearValues = {};
			std::optional<uint32_t> layerId = {};
			RenderPass renderPass = nullptr;
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

lua_registercheck(VKTextureCreateInfo,prosper::util::TextureCreateInfo);
lua_registercheck(VKSamplerCreateInfo,prosper::util::SamplerCreateInfo);
lua_registercheck(VKRenderTargetCreateInfo,prosper::util::RenderTargetCreateInfo);
lua_registercheck(VKBufferCreateInfo,prosper::util::BufferCreateInfo);
lua_registercheck(VKImageCreateInfo,prosper::util::ImageCreateInfo);
lua_registercheck(VKImageViewCreateInfo,prosper::util::ImageViewCreateInfo);
lua_registercheck(VKRenderPassCreateInfo,prosper::util::RenderPassCreateInfo);
lua_registercheck(VKBlitInfo,prosper::util::BlitInfo);
lua_registercheck(VKCopyInfo,prosper::util::CopyInfo);
lua_registercheck(VKPipelineBarrierInfo,prosper::util::PipelineBarrierInfo);
lua_registercheck(VKClearImageInfo,prosper::util::ClearImageInfo);

lua_registercheck(VKClearValue,Lua::Vulkan::ClearValue);
lua_registercheck(VKTexture,Lua::Vulkan::Texture);
lua_registercheck(VKImage,Lua::Vulkan::Image);
lua_registercheck(VKImageView,Lua::Vulkan::ImageView);
lua_registercheck(VKSampler,Lua::Vulkan::Sampler);
lua_registercheck(VKFramebuffer,Lua::Vulkan::Framebuffer);
lua_registercheck(VKRenderPass,Lua::Vulkan::RenderPass);
lua_registercheck(VKEvent,Lua::Vulkan::Event);
lua_registercheck(VKFence,Lua::Vulkan::Fence);
lua_registercheck(VKSemaphore,Lua::Vulkan::Semaphore);
lua_registercheck(VKMemory,Lua::Vulkan::Memory);
lua_registercheck(VKCommandBuffer,Lua::Vulkan::CommandBuffer);
lua_registercheck(VKBuffer,Lua::Vulkan::Buffer);
lua_registercheck(VKDescriptorSet,Lua::Vulkan::DescriptorSet);
lua_registercheck(VKRenderTarget,Lua::Vulkan::RenderTarget);
lua_registercheck(VKTimestampQuery,Lua::Vulkan::TimestampQuery);
lua_registercheck(VKTimerQuery,Lua::Vulkan::TimerQuery);

#endif
