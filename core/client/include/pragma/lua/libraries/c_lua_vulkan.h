// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_LUA_VULKAN_H__
#define __C_LUA_VULKAN_H__

#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>
#include <pragma/lua/types/udm.hpp>
#include <prosper_util.hpp>
#include <prosper_prepared_command_buffer.hpp>
#include <prosper_command_buffer.hpp>
#include <image/prosper_render_target.hpp>
#include <image/prosper_sampler.hpp>

namespace prosper {
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
	class SwapBuffer;
	class ISwapCommandBufferGroup;
};

namespace vk {
	union ClearValue;
};

namespace Lua {
	namespace Vulkan {
		struct DLLCLIENT PreparedCommandLuaArg {
			template<typename T>
			static PreparedCommandLuaArg CreateValue(lua_State *l, T &&value)
			{
				PreparedCommandLuaArg r {};
				r.o = luabind::object {l, value};
				return r;
			}
			luabind::object o;
		};
		struct DLLCLIENT PreparedCommandLuaDynamicArg {
			PreparedCommandLuaDynamicArg(std::string argName) : argName {std::move(argName)} {}
			std::string argName;
		};
		DLLCLIENT prosper::util::PreparedCommand::Argument make_pcb_arg(const Lua::Vulkan::PreparedCommandLuaArg &larg, udm::Type type);
		template<typename T>
		prosper::util::PreparedCommand::Argument make_pcb_arg(const Lua::Vulkan::PreparedCommandLuaArg &larg)
		{
			if constexpr(std::is_enum_v<T>)
				return make_pcb_arg(larg, udm::type_to_enum<std::underlying_type_t<T>>());
			else
				return make_pcb_arg(larg, udm::type_to_enum<T>());
		}
		struct DLLCLIENT ClearValue {
		  public:
			ClearValue(const Color &color) : clearValue {prosper::ClearColorValue {std::array<float, 4> {color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f}}} {}
			ClearValue(float depth, uint32_t stencil) : clearValue {prosper::ClearDepthStencilValue {depth, stencil}} {}
			ClearValue(float depth) : clearValue {prosper::ClearDepthStencilValue {depth}} {}
			ClearValue() : ClearValue(0.f) {}
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
		using SwapBuffer = prosper::SwapBuffer;
		using DescriptorSet = prosper::IDescriptorSetGroup;
		using RenderTarget = prosper::RenderTarget;
		using TimestampQuery = prosper::TimestampQuery;
		using TimerQuery = prosper::TimerQuery;
		using CommandBufferRecorder = prosper::ISwapCommandBufferGroup;
		struct DLLCLIENT RenderPassInfo {
			RenderPassInfo(const std::shared_ptr<RenderTarget> &renderTarget) : renderTarget {renderTarget} {}
			std::shared_ptr<RenderTarget> renderTarget;
			std::vector<ClearValue> clearValues = {};
			std::optional<uint32_t> layerId = {};
			std::shared_ptr<RenderPass> renderPass = nullptr;
			prosper::IPrimaryCommandBuffer::RenderPassFlags renderPassFlags = prosper::IPrimaryCommandBuffer::RenderPassFlags::None;
		};
	};
};

namespace prosper {
	namespace util {
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

// PCB Argument converter
namespace luabind {
	template<>
	struct DLLNETWORK default_converter<Lua::Vulkan::PreparedCommandLuaArg> : native_converter_base<Lua::Vulkan::PreparedCommandLuaArg> {
		enum { consumed_args = 1 };

		template<typename U>
		Lua::Vulkan::PreparedCommandLuaArg to_cpp(lua_State *L, U u, int index);

		template<class U>
		static int match(lua_State *l, U u, int index);

		template<class U>
		void converter_postcall(lua_State *, U u, int)
		{
		}

		void to_lua(lua_State *L, Lua::Vulkan::PreparedCommandLuaArg const &x);
		void to_lua(lua_State *L, Lua::Vulkan::PreparedCommandLuaArg *x);
	  public:
		static value_type to_cpp_deferred(lua_State *, int) { return {}; }
		static void to_lua_deferred(lua_State *, param_type) {}
		static int compute_score(lua_State *, int) { return no_match; }
	};

	template<>
	struct DLLNETWORK default_converter<const Lua::Vulkan::PreparedCommandLuaArg> : default_converter<Lua::Vulkan::PreparedCommandLuaArg> {};

	template<>
	struct DLLNETWORK default_converter<Lua::Vulkan::PreparedCommandLuaArg const &> : default_converter<Lua::Vulkan::PreparedCommandLuaArg> {};

	template<>
	struct DLLNETWORK default_converter<Lua::Vulkan::PreparedCommandLuaArg &&> : default_converter<Lua::Vulkan::PreparedCommandLuaArg> {};
}

template<typename U>
Lua::Vulkan::PreparedCommandLuaArg luabind::default_converter<Lua::Vulkan::PreparedCommandLuaArg>::to_cpp(lua_State *L, U u, int index)
{
	return Lua::Vulkan::PreparedCommandLuaArg {luabind::object {luabind::from_stack(L, index)}};
}

template<class U>
int luabind::default_converter<Lua::Vulkan::PreparedCommandLuaArg>::match(lua_State *l, U u, int index)
{
	return (luabind::check_udm<true, true, true>(l, index) || Lua::IsType<Lua::Vulkan::PreparedCommandLuaDynamicArg>(l, index)) ? 1 : no_match;
}
//

#endif
