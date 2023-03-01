/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/lua/libraries/c_lua_vulkan.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/shaders/c_shader_lua.hpp"
#include "pragma/rendering/shaders/image/c_shader_resize_image.hpp"
#include <pragma/lua/classes/ldef_color.h>
#include <pragma/lua/classes/ldef_vector.h>
#include <pragma/lua/classes/lerrorcode.h>
#include <pragma/lua/classes/ldatastream.h>
#include <pragma/lua/policies/optional_policy.hpp>
#include <pragma/lua/policies/shared_from_this_policy.hpp>
#include <pragma/lua/policies/pair_policy.hpp>
#include <pragma/lua/policies/vector_policy.hpp>
#include <pragma/lua/policies/default_parameter_policy.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/lua/converters/vector_converter_t.hpp>
#include <pragma/lua/converters/optional_converter_t.hpp>
#include <pragma/lua/converters/pair_converter_t.hpp>
#include <pragma/lua/util.hpp>
#include <pragma/lua/luaapi.h>
#include <luainterface.hpp>
#include <prosper_util.hpp>
#include <image/prosper_sampler.hpp>
#include <image/prosper_image_view.hpp>
#include <buffers/prosper_buffer.hpp>
#include <buffers/prosper_swap_buffer.hpp>
#include <buffers/prosper_render_buffer.hpp>
#include <queries/prosper_timer_query.hpp>
#include <queries/prosper_timestamp_query.hpp>
#include <image/prosper_render_target.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_framebuffer.hpp>
#include <prosper_render_pass.hpp>
#include <prosper_fence.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <util_image_buffer.hpp>
#include "pragma/util/util_image.hpp"
#include "pragma/model/vk_mesh.h"
#include <prosper_event.hpp>
#include <prosper_window.hpp>
#include <prosper_swap_command_buffer.hpp>
#include <pragma/rendering/shaders/image/c_shader_gradient.hpp>
#include <wgui/wgui.h>
#include <luabind/copy_policy.hpp>
#include <luabind/detail/meta.hpp>
#include <shader/prosper_shader_blur.hpp>

extern CEngine *c_engine;

namespace luabind {
	namespace detail {

		struct render_context_converter {
			enum { consumed_args = 0 };

			template<typename U>
			prosper::IPrContext &to_cpp(lua_State *L, U, int)
			{
				return c_engine->GetRenderContext();
			}

			static int match(...) { return 0; }
			template<typename U>
			void converter_postcall(lua_State *, by_reference<U>, int)
			{
			}
		};

		struct render_context_policy {
			template<class T, class Direction>
			struct specialize {
				using type = render_context_converter;
			};
		};

	}

	template<unsigned int N>
	using render_context_policy = meta::type_list<converter_policy_injector<N, detail::render_context_policy>>;

}

namespace Lua {
	namespace Vulkan {
		std::vector<pragma::ShaderGradient::Node> get_gradient_nodes(lua_State *l, const luabind::tableT<void> &tNodes);
		static std::shared_ptr<prosper::IBuffer> create_buffer(prosper::IPrContext &context, prosper::util::BufferCreateInfo &bufCreateInfo, ::DataStream &ds);
		static std::shared_ptr<prosper::IBuffer> create_buffer(prosper::IPrContext &context, prosper::util::BufferCreateInfo &bufCreateInfo);
		static std::shared_ptr<prosper::Texture> create_texture(prosper::IImage &img, const prosper::util::TextureCreateInfo &texCreateInfo, const prosper::util::ImageViewCreateInfo &imgViewCreateInfo, const prosper::util::SamplerCreateInfo &samplerCreateInfo);
		static std::shared_ptr<prosper::Texture> create_texture(prosper::IImage &img, const prosper::util::TextureCreateInfo &texCreateInfo, const prosper::util::ImageViewCreateInfo &imgViewCreateInfo);
		static std::shared_ptr<prosper::Texture> create_texture(prosper::IImage &img, const prosper::util::TextureCreateInfo &texCreateInfo);
		static std::shared_ptr<prosper::IDescriptorSetGroup> create_descriptor_set(const pragma::LuaDescriptorSetInfo &ldescSetInfo);
		static std::shared_ptr<prosper::IImage> create_image(const prosper::util::ImageCreateInfo &imgCreateInfo, ::DataStream &ds);
		static std::shared_ptr<prosper::IImage> create_image(const prosper::util::ImageCreateInfo &imgCreateInfo);
		static std::shared_ptr<prosper::IImage> create_image(const std::array<std::shared_ptr<uimg::ImageBuffer>, 6> &imgBuffers);
		static std::shared_ptr<prosper::IImage> create_image(const std::array<std::shared_ptr<uimg::ImageBuffer>, 6> &imgBuffers, const prosper::util::ImageCreateInfo &imgCreateInfo);
		static std::shared_ptr<prosper::IImage> create_image(uimg::ImageBuffer &imgBuffer);
		static std::shared_ptr<prosper::IImage> create_image(uimg::ImageBuffer &imgBuffer, const prosper::util::ImageCreateInfo &imgCreateInfo);
		static std::shared_ptr<prosper::IImageView> create_image_view(prosper::IPrContext &context, const prosper::util::ImageViewCreateInfo &imgViewCreateInfo, prosper::IImage &img);
		static prosper::util::ImageCreateInfo create_image_create_info(const uimg::ImageBuffer &imgBuf, bool cubemap);
		static prosper::util::ImageCreateInfo create_image_create_info(const uimg::ImageBuffer &imgBuf);
		static std::shared_ptr<prosper::Texture> create_gradient_texture(lua_State *l, uint32_t width, uint32_t height, prosper::Format format, const Vector2 &dir, const luabind::tableT<void> &tNodes);
		static Lua::var<prosper::Texture, Lua::mult<bool, std::string>> blur_texture(lua_State *l, prosper::Texture &srcTex, uint32_t blurStrength);
		static std::shared_ptr<prosper::IFence> create_fence(bool createSignalled);
		static std::shared_ptr<prosper::IFence> create_fence();
		static std::shared_ptr<prosper::IFramebuffer> create_framebuffer(uint32_t width, uint32_t height, const std::vector<prosper::IImageView *> &attachments, uint32_t layers);
		static std::shared_ptr<prosper::IFramebuffer> create_framebuffer(uint32_t width, uint32_t height, const std::vector<prosper::IImageView *> &attachments);
		static std::shared_ptr<prosper::IRenderPass> create_render_pass(const prosper::util::RenderPassCreateInfo &rpCreateInfo);

		static std::shared_ptr<prosper::RenderTarget> create_render_target(const prosper::util::RenderTargetCreateInfo &rtCreateInfo, Texture &texture);
		static std::shared_ptr<prosper::RenderTarget> create_render_target(const prosper::util::RenderTargetCreateInfo &rtCreateInfo, Texture &texture, Lua::Vulkan::RenderPass &renderPass);
		static std::shared_ptr<prosper::RenderTarget> create_render_target(const prosper::util::RenderTargetCreateInfo &rtCreateInfo, const std::vector<std::shared_ptr<Texture>> &textures, Lua::Vulkan::RenderPass &renderPass);
		static std::shared_ptr<prosper::RenderTarget> create_render_target(const prosper::util::RenderTargetCreateInfo &rtCreateInfo, const std::vector<std::shared_ptr<Texture>> &textures);

		static Vector2i calculate_mipmap_size(uint32_t w, uint32_t h, uint32_t level);
		static uint32_t calculate_mipmap_size(uint32_t v, uint32_t level);
		static std::shared_ptr<prosper::IBuffer> get_square_vertex_uv_buffer(lua_State *l);
		static std::shared_ptr<prosper::IBuffer> get_square_vertex_buffer(lua_State *l);
		static std::shared_ptr<prosper::IBuffer> get_square_uv_buffer(lua_State *l);
		static std::shared_ptr<prosper::IBuffer> allocate_temporary_buffer(lua_State *l, uint32_t size);
		static std::shared_ptr<prosper::IBuffer> allocate_temporary_buffer(lua_State *l, ::DataStream &ds);
		static std::shared_ptr<prosper::IBuffer> get_line_vertex_buffer(lua_State *l);
		namespace VKTexture {
			static bool IsValid(lua_State *l, Texture &hTex);
			static uint32_t GetWidth(lua_State *l, Texture &hTex);
			static uint32_t GetHeight(lua_State *l, Texture &hTex);
		};
		namespace VKImage {
			static bool IsValid(lua_State *l, Image &hImg);
			static std::pair<uint32_t, uint32_t> GetExtent2D(lua_State *l, Image &hImg, uint32_t mipmap = 0u);
#if 0
			static void GetExtent3D(lua_State *l,Image &hImg,uint32_t mipmap=0u);
			static void GetMemoryTypes(lua_State *l,Image &hImg);
			static void GetStorageSize(lua_State *l,Image &hImg);
			static void GetParentSwapchain(lua_State *l,Image &hImg);
			static void GetMemoryBlock(lua_State *l,Image &hImg);
			static void GetSubresourceRange(lua_State *l,Image &hImg);
#endif
			static std::pair<uint32_t, uint32_t> GetMipmapSize(lua_State *l, Image &hImg, uint32_t mipmap = 0u);
			static void WriteMemory(lua_State *l, Image &hImg, uint32_t x, uint32_t y, uimg::ImageBuffer &imgBuf, uint32_t layerIndex, uint32_t mipLevel);
			static void WriteMemory(lua_State *l, Image &hImg, uint32_t x, uint32_t y, uimg::ImageBuffer &imgBuf, uint32_t layerIndex);
			static void WriteMemory(lua_State *l, Image &hImg, uint32_t x, uint32_t y, uimg::ImageBuffer &imgBuf);
		};
		namespace VKImageView {
			static bool IsValid(lua_State *l, ImageView &hImgView);
#if 0
			static void GetBaseMipmapSize(lua_State *l,ImageView &hImgView);
			static void GetParentImage(lua_State *l,ImageView &hImgView);
			static void GetSubresourceRange(lua_State *l,ImageView &hImgView);
#endif
		};
		namespace VKSampler {
			static bool IsValid(lua_State *l, Sampler &hSampler);
			static bool GetAnisotropyEnabled(lua_State *l, Sampler &hSampler);
			// static void GetUseUnnormalizedCoordinates(lua_State *l,Sampler &hSampler);
			// static void SetUseUnnormalizedCoordinates(lua_State *l,Sampler &hSampler,bool bUnnormalizedCoordinates);
		};
		namespace VKContextObject {
			static void SetDebugName(lua_State *l, prosper::ContextObject &o, const std::string &name) { o.SetDebugName(name); }
			static std::string GetDebugName(lua_State *l, const prosper::ContextObject &o) { return o.GetDebugName(); }
		};
#if 0
		namespace VKSemaphore
		{
			static bool IsValid(lua_State *l,Semaphore &hFence);
		};
		namespace VKMemory
		{
			static bool IsValid(lua_State *l,Memory &hMemory);
			static void GetParentMemory(lua_State *l,Memory &hMemory);
			static void GetSize(lua_State *l,Memory &hMemory);
			static void GetStartOffset(lua_State *l,Memory &hMemory);
			static void Write(lua_State *l,Memory &hMemory,uint32_t offset,::DataStream &ds,uint32_t dsOffset,uint32_t dsSize);
			static void Read(lua_State *l,Memory &hMemory,uint32_t offset,uint32_t size);
			static void Read(lua_State *l,Memory &hMemory,uint32_t offset,uint32_t size,::DataStream &ds,uint32_t dsOffset);
			static void Map(lua_State *l,Memory &hMemory,uint32_t offset,uint32_t size);
			static void Unmap(lua_State *l,Memory &hMemory);
		};
#endif
		namespace VKRenderTarget {
			static bool IsValid(lua_State *l, RenderTarget &hRt);
			static uint32_t GetWidth(lua_State *l, RenderTarget &hRt);
			static uint32_t GetHeight(lua_State *l, RenderTarget &hRt);
			static prosper::Format GetFormat(lua_State *l, RenderTarget &hRt);
		};
		namespace VKTimestampQuery {
			static bool IsValid(lua_State *l, TimestampQuery &hTimestampQuery);
			static Lua::var<bool, int64_t> QueryResult(lua_State *l, TimestampQuery &hTimestampQuery);
		};
		namespace VKTimerQuery {
			static bool IsValid(lua_State *l, TimerQuery &hTimerQuery);
			static Lua::var<bool, int64_t> QueryResult(lua_State *l, TimerQuery &hTimerQuery);
		};
	};
};

namespace prosper { // For some reason these need to be in the same namespaces as the types, otherwise luabind can't locate them
	static bool operator==(const Lua::Vulkan::Texture &a, const Lua::Vulkan::Texture &b) { return &a == &b; }
	static bool operator==(const Lua::Vulkan::Image &a, const Lua::Vulkan::Image &b) { return &a == &b; }
	static bool operator==(const Lua::Vulkan::ImageView &a, const Lua::Vulkan::ImageView &b) { return &a == &b; }
	static bool operator==(const Lua::Vulkan::Sampler &a, const Lua::Vulkan::Sampler &b) { return &a == &b; }
	static bool operator==(const Lua::Vulkan::RenderTarget &a, const Lua::Vulkan::RenderTarget &b) { return &a == &b; }
	static bool operator==(const Lua::Vulkan::TimestampQuery &a, const Lua::Vulkan::TimestampQuery &b) { return &a == &b; }
	static bool operator==(const Lua::Vulkan::TimerQuery &a, const Lua::Vulkan::TimerQuery &b) { return &a == &b; }
	static bool operator==(const Lua::Vulkan::CommandBufferRecorder &a, const Lua::Vulkan::CommandBufferRecorder &b) { return &a == &b; }

	static std::ostream &operator<<(std::ostream &out, const Lua::Vulkan::ImageView &hImgView)
	{
		out << "VKImageView[";
		out << "]";
		return out;
	}

	static std::ostream &operator<<(std::ostream &out, const Lua::Vulkan::Sampler &hSampler)
	{
		out << "VKSampler[";
		out << "]";
		return out;
	}

	static std::ostream &operator<<(std::ostream &out, const Lua::Vulkan::Image &hImg)
	{
		out << "VKImage[";
		auto extents = hImg.GetExtents();
		out << extents.width << "x" << extents.height;
		out << "][";
		auto format = hImg.GetFormat();
		out << prosper::util::to_string(format);
		out << "]";
		return out;
	}

	static std::ostream &operator<<(std::ostream &out, const Lua::Vulkan::Framebuffer &hFramebuffer)
	{
		out << "VKFramebuffer[";
		out << "]";
		return out;
	}

	static std::ostream &operator<<(std::ostream &out, const Lua::Vulkan::RenderPass &hRenderPass)
	{
		out << "VKRenderPass[";
		out << "]";
		return out;
	}

	static std::ostream &operator<<(std::ostream &out, const Lua::Vulkan::Texture &hTex)
	{
		out << "VKTexture[";
		auto &img = hTex.GetImage();
		auto *imgView = hTex.GetImageView();
		auto *sampler = hTex.GetSampler();
		out << img << "][";
		if(imgView == nullptr)
			out << "NULL][";
		else
			out << imgView << "][";
		if(sampler == nullptr)
			out << "NULL";
		else
			out << sampler;
		out << "]";
		return out;
	}

	static std::ostream &operator<<(std::ostream &out, const Lua::Vulkan::RenderTarget &hRt)
	{
		out << "VKRenderTarget[";
		auto &tex = hRt.GetTexture();
		auto &framebuffer = hRt.GetFramebuffer();
		auto &renderPass = hRt.GetRenderPass();

		out << tex << "][";
		out << framebuffer << "][";
		out << renderPass << "][";
		out << "]";
		return out;
	}

	static std::ostream &operator<<(std::ostream &out, const Lua::Vulkan::TimestampQuery &hTimestampQuery)
	{
		out << "VKTimestampQuery[";
		out << hTimestampQuery.IsResultAvailable();
		out << "]";
		return out;
	}

	static std::ostream &operator<<(std::ostream &out, const Lua::Vulkan::TimerQuery &hTimerQuery)
	{
		out << "VKTimerQuery[";
		out << hTimerQuery.IsResultAvailable();
		out << "]";
		return out;
	}

	static std::ostream &operator<<(std::ostream &out, const Lua::Vulkan::CommandBufferRecorder &hCmdBufferRecorder)
	{
		out << "VKCommandBufferRecorder[";
		out << hCmdBufferRecorder.IsPending();
		out << "]";
		return out;
	}
};

std::shared_ptr<prosper::IBuffer> Lua::Vulkan::create_buffer(prosper::IPrContext &context, prosper::util::BufferCreateInfo &bufCreateInfo, ::DataStream &ds)
{
	auto buf = context.CreateBuffer(bufCreateInfo, ds->GetData());
	if(buf == nullptr)
		return 0;
	buf->SetDebugName("lua_buf");
	return buf;
}

std::shared_ptr<prosper::IBuffer> Lua::Vulkan::create_buffer(prosper::IPrContext &context, prosper::util::BufferCreateInfo &bufCreateInfo)
{
	auto buf = context.CreateBuffer(bufCreateInfo);
	if(buf == nullptr)
		return 0;
	buf->SetDebugName("lua_buf");
	return buf;
}

#if 0
void Lua::Vulkan::get_descriptor_set_layout_bindings(lua_State *l,std::vector<::Vulkan::DescriptorSetLayout::Binding> &bindings,int32_t tBindings)
{
	auto numBindings = Lua::GetObjectLength(l,tBindings);
	bindings.reserve(numBindings);
	for(auto j=decltype(numBindings){0};j<numBindings;++j)
	{
		Lua::PushInt(l,j +1); /* 1 */
		Lua::GetTableValue(l,tBindings);

		Lua::CheckTable(l,-1);
		auto tBinding = Lua::GetStackTop(l);

		auto type = prosper::DescriptorType::UniformBuffer;
		get_table_value<ptrdiff_t,decltype(type)>(l,"type",tBinding,type,Lua::CheckInt);

		auto shaderStages = prosper::ShaderStageFlagBits::eAllGraphics;
		get_table_value<ptrdiff_t,decltype(shaderStages)>(l,"stage",tBinding,shaderStages,Lua::CheckInt);

		uint32_t arrayCount = 1;
		get_table_value<ptrdiff_t,decltype(arrayCount)>(l,"arrayCount",tBinding,arrayCount,Lua::CheckInt);

		::Vulkan::DescriptorSetLayout::Binding binding {type,shaderStages,arrayCount};
		bindings.push_back(binding);

		Lua::Pop(l,1); /* 0 */
	}
}
#endif // TODO

std::shared_ptr<prosper::IDescriptorSetGroup> Lua::Vulkan::create_descriptor_set(const pragma::LuaDescriptorSetInfo &ldescSetInfo)
{
	auto shaderDescSetInfo = to_prosper_descriptor_set_info(ldescSetInfo);
	auto dsg = c_engine->GetRenderContext().CreateDescriptorSetGroup(shaderDescSetInfo);
	if(dsg == nullptr)
		return nullptr;
	dsg->SetDebugName("lua_dsg");
	return dsg;
}

std::shared_ptr<prosper::IImageView> Lua::Vulkan::create_image_view(prosper::IPrContext &context, const prosper::util::ImageViewCreateInfo &imgViewCreateInfo, prosper::IImage &img) { return context.CreateImageView(imgViewCreateInfo, img); }

prosper::util::ImageCreateInfo Lua::Vulkan::create_image_create_info(const uimg::ImageBuffer &imgBuf, bool cubemap) { return prosper::util::get_image_create_info(imgBuf, cubemap); }

prosper::util::ImageCreateInfo Lua::Vulkan::create_image_create_info(const uimg::ImageBuffer &imgBuf) { return prosper::util::get_image_create_info(imgBuf, false); }

std::shared_ptr<prosper::IImage> Lua::Vulkan::create_image(uimg::ImageBuffer &imgBuffer, const prosper::util::ImageCreateInfo &imgCreateInfo)
{
	auto img = c_engine->GetRenderContext().CreateImage(imgBuffer, imgCreateInfo);
	if(img)
		img->SetDebugName("lua_img");
	return img;
}

std::shared_ptr<prosper::IImage> Lua::Vulkan::create_image(uimg::ImageBuffer &imgBuffer)
{
	auto img = c_engine->GetRenderContext().CreateImage(imgBuffer);
	if(img)
		img->SetDebugName("lua_img");
	return img;
}

std::shared_ptr<prosper::IImage> Lua::Vulkan::create_image(const std::array<std::shared_ptr<uimg::ImageBuffer>, 6> &imgBuffers, const prosper::util::ImageCreateInfo &imgCreateInfo)
{
	// TODO: Parameter for CreateCubemap should be const
	auto img = c_engine->GetRenderContext().CreateCubemap(const_cast<std::array<std::shared_ptr<uimg::ImageBuffer>, 6> &>(imgBuffers), imgCreateInfo);
	if(img)
		img->SetDebugName("lua_img");
	return img;
}

std::shared_ptr<prosper::IImage> Lua::Vulkan::create_image(const std::array<std::shared_ptr<uimg::ImageBuffer>, 6> &imgBuffers)
{
	// TODO: Parameter for CreateCubemap should be const
	auto img = c_engine->GetRenderContext().CreateCubemap(const_cast<std::array<std::shared_ptr<uimg::ImageBuffer>, 6> &>(imgBuffers));
	if(img)
		img->SetDebugName("lua_img");
	return img;
}

std::shared_ptr<prosper::IImage> Lua::Vulkan::create_image(const prosper::util::ImageCreateInfo &imgCreateInfo)
{
	auto img = c_engine->GetRenderContext().CreateImage(imgCreateInfo);
	if(img)
		img->SetDebugName("lua_img");
	return img;
}

std::shared_ptr<prosper::IImage> Lua::Vulkan::create_image(const prosper::util::ImageCreateInfo &imgCreateInfo, ::DataStream &ds)
{
	auto img = c_engine->GetRenderContext().CreateImage(imgCreateInfo, ds->GetData());
	if(img)
		img->SetDebugName("lua_img");
	return img;
}

std::shared_ptr<prosper::Texture> Lua::Vulkan::create_texture(prosper::IImage &img, const prosper::util::TextureCreateInfo &texCreateInfo, const prosper::util::ImageViewCreateInfo &imgViewCreateInfo, const prosper::util::SamplerCreateInfo &samplerCreateInfo)
{
	auto tex = c_engine->GetRenderContext().CreateTexture(texCreateInfo, img, imgViewCreateInfo, samplerCreateInfo);
	if(tex == nullptr)
		return nullptr;
	tex->SetDebugName("lua_tex");
	return tex;
}

std::shared_ptr<prosper::Texture> Lua::Vulkan::create_texture(prosper::IImage &img, const prosper::util::TextureCreateInfo &texCreateInfo, const prosper::util::ImageViewCreateInfo &imgViewCreateInfo)
{
	std::optional<prosper::util::SamplerCreateInfo> samplerCreateInfo {};
	auto tex = c_engine->GetRenderContext().CreateTexture(texCreateInfo, img, imgViewCreateInfo, samplerCreateInfo);
	if(tex == nullptr)
		return nullptr;
	tex->SetDebugName("lua_tex");
	return tex;
}

std::shared_ptr<prosper::Texture> Lua::Vulkan::create_texture(prosper::IImage &img, const prosper::util::TextureCreateInfo &texCreateInfo)
{
	std::optional<prosper::util::ImageViewCreateInfo> imgViewCreateInfo {};
	std::optional<prosper::util::SamplerCreateInfo> samplerCreateInfo {};
	auto tex = c_engine->GetRenderContext().CreateTexture(texCreateInfo, img, imgViewCreateInfo, samplerCreateInfo);
	if(tex == nullptr)
		return nullptr;
	tex->SetDebugName("lua_tex");
	return tex;
}

std::shared_ptr<prosper::IFramebuffer> Lua::Vulkan::create_framebuffer(uint32_t width, uint32_t height, const std::vector<prosper::IImageView *> &attachments, uint32_t layers)
{
	auto fb = c_engine->GetRenderContext().CreateFramebuffer(width, height, layers, attachments);
	if(fb == nullptr)
		return nullptr;
	fb->SetDebugName("lua_fb");
	return fb;
}

std::shared_ptr<prosper::IFramebuffer> Lua::Vulkan::create_framebuffer(uint32_t width, uint32_t height, const std::vector<prosper::IImageView *> &attachments) { return create_framebuffer(width, height, attachments, 1); }
std::shared_ptr<prosper::IRenderPass> Lua::Vulkan::create_render_pass(const prosper::util::RenderPassCreateInfo &rpCreateInfo)
{
	auto rp = c_engine->GetRenderContext().CreateRenderPass(rpCreateInfo);
	if(rp == nullptr)
		return nullptr;
	rp->SetDebugName("lua_rp");
	return rp;
}

std::shared_ptr<prosper::RenderTarget> Lua::Vulkan::create_render_target(const prosper::util::RenderTargetCreateInfo &rtCreateInfo, Texture &texture)
{
	auto rt = c_engine->GetRenderContext().CreateRenderTarget({texture.shared_from_this()}, nullptr, rtCreateInfo);
	if(rt == nullptr)
		return nullptr;
	rt->SetDebugName("lua_rt");
	return rt;
}
std::shared_ptr<prosper::RenderTarget> Lua::Vulkan::create_render_target(const prosper::util::RenderTargetCreateInfo &rtCreateInfo, Texture &texture, Lua::Vulkan::RenderPass &renderPass)
{
	auto rt = c_engine->GetRenderContext().CreateRenderTarget({texture.shared_from_this()}, renderPass.shared_from_this(), rtCreateInfo);
	if(rt == nullptr)
		return nullptr;
	rt->SetDebugName("lua_rt");
	return rt;
}
std::shared_ptr<prosper::RenderTarget> Lua::Vulkan::create_render_target(const prosper::util::RenderTargetCreateInfo &rtCreateInfo, const std::vector<std::shared_ptr<Texture>> &textures, Lua::Vulkan::RenderPass &renderPass)
{
	auto rt = c_engine->GetRenderContext().CreateRenderTarget(textures, renderPass.shared_from_this(), rtCreateInfo);
	if(rt == nullptr)
		return nullptr;
	rt->SetDebugName("lua_rt");
	return rt;
}
std::shared_ptr<prosper::RenderTarget> Lua::Vulkan::create_render_target(const prosper::util::RenderTargetCreateInfo &rtCreateInfo, const std::vector<std::shared_ptr<Texture>> &textures)
{
	auto rt = c_engine->GetRenderContext().CreateRenderTarget(textures, nullptr, rtCreateInfo);
	if(rt == nullptr)
		return nullptr;
	rt->SetDebugName("lua_rt");
	return rt;
}
std::shared_ptr<prosper::IFence> Lua::Vulkan::create_fence(bool createSignalled) { return c_engine->GetRenderContext().CreateFence(createSignalled); }
std::shared_ptr<prosper::IFence> Lua::Vulkan::create_fence() { return create_fence(false); }

Vector2i Lua::Vulkan::calculate_mipmap_size(uint32_t w, uint32_t h, uint32_t level)
{
	uint32_t wMipmap;
	uint32_t hMipmap;
	::prosper::util::calculate_mipmap_size(w, h, &wMipmap, &hMipmap, level);
	return {wMipmap, hMipmap};
}
uint32_t Lua::Vulkan::calculate_mipmap_size(uint32_t v, uint32_t level) { return ::prosper::util::calculate_mipmap_size(v, level); }
std::shared_ptr<prosper::IBuffer> Lua::Vulkan::get_line_vertex_buffer(lua_State *l) { return c_engine->GetRenderContext().GetCommonBufferCache().GetLineVertexBuffer(); }
std::shared_ptr<prosper::IBuffer> Lua::Vulkan::get_square_vertex_uv_buffer(lua_State *l) { return c_engine->GetRenderContext().GetCommonBufferCache().GetSquareVertexUvBuffer(); }
std::shared_ptr<prosper::IBuffer> Lua::Vulkan::get_square_vertex_buffer(lua_State *l) { return c_engine->GetRenderContext().GetCommonBufferCache().GetSquareVertexBuffer(); }
std::shared_ptr<prosper::IBuffer> Lua::Vulkan::get_square_uv_buffer(lua_State *l) { return c_engine->GetRenderContext().GetCommonBufferCache().GetSquareUvBuffer(); }
std::shared_ptr<prosper::IBuffer> Lua::Vulkan::allocate_temporary_buffer(lua_State *l, uint32_t size) { return c_engine->GetRenderContext().AllocateTemporaryBuffer(size); }
std::shared_ptr<prosper::IBuffer> Lua::Vulkan::allocate_temporary_buffer(lua_State *l, ::DataStream &ds) { return c_engine->GetRenderContext().AllocateTemporaryBuffer(ds->GetSize(), 0u /* alignment */, ds->GetData()); }

std::vector<pragma::ShaderGradient::Node> Lua::Vulkan::get_gradient_nodes(lua_State *l, const luabind::tableT<void> &tNodes)
{
	std::vector<pragma::ShaderGradient::Node> nodes;

	auto n = Lua::GetObjectLength(l, tNodes);
	nodes.reserve(n);
	for(luabind::iterator i {tNodes}, end; i != end; ++i) {
		luabind::object o = *i;
		auto offset = luabind::object_cast<float>(o["offset"]);
		auto color = luabind::object_cast<Color>(o["color"]);
		nodes.push_back(pragma::ShaderGradient::Node(Vector4(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f), offset));
	}
	return nodes;
}
Lua::var<prosper::Texture, Lua::mult<bool, std::string>> Lua::Vulkan::blur_texture(lua_State *l, prosper::Texture &srcTex, uint32_t blurStrength)
{
	auto &context = c_engine->GetRenderContext();
	auto &srcImg = srcTex.GetImage();

	prosper::util::ImageCreateInfo createInfo {};
	createInfo.usage = prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::TransferSrcBit | prosper::ImageUsageFlags::TransferDstBit;
	createInfo.width = srcImg.GetWidth();
	createInfo.height = srcImg.GetHeight();
	createInfo.format = prosper::Format::R8G8B8A8_UNorm;
	createInfo.postCreateLayout = prosper::ImageLayout::ShaderReadOnlyOptimal;

	auto img = context.CreateImage(createInfo);
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	auto tex = context.CreateTexture({}, *img, imgViewCreateInfo, samplerCreateInfo);
	prosper::util::RenderPassCreateInfo rpInfo {{img->GetFormat()}};
	auto rp = context.CreateRenderPass(rpInfo);
	auto rt = context.CreateRenderTarget({tex}, rp);
	rt->SetDebugName("img_slideshow_rt");

	auto blurSet = prosper::BlurSet::Create(context, rt);
	if(!blurSet)
		return Lua::mult<bool, std::string> {l, false, "Unable to create blur set!"};
	auto &setupCmd = context.GetSetupCommandBuffer();

	setupCmd->RecordImageBarrier(srcImg, prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::TransferSrcOptimal);
	setupCmd->RecordImageBarrier(rt->GetTexture().GetImage(), prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::TransferDstOptimal);
	setupCmd->RecordBlitImage({}, srcImg, rt->GetTexture().GetImage());
	setupCmd->RecordImageBarrier(srcImg, prosper::ImageLayout::TransferSrcOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
	setupCmd->RecordImageBarrier(rt->GetTexture().GetImage(), prosper::ImageLayout::TransferDstOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);

	try {
		prosper::util::record_blur_image(context, setupCmd, *blurSet, {Vector4(1.f, 1.f, 1.f, 1.f), 1.75f, 9}, blurStrength);
	}
	catch(const std::logic_error &e) {
		return Lua::mult<bool, std::string> {l, false, e.what()};
	}
	context.FlushSetupCommandBuffer();
	return luabind::object {l, blurSet->GetFinalRenderTarget()->GetTexture().shared_from_this()};
}
std::shared_ptr<prosper::Texture> Lua::Vulkan::create_gradient_texture(lua_State *l, uint32_t width, uint32_t height, prosper::Format format, const Vector2 &dir, const luabind::tableT<void> &tNodes)
{
	auto whShader = c_engine->GetShader("gradient");
	if(whShader.expired())
		return nullptr;
	auto nodes = get_gradient_nodes(l, tNodes);
	prosper::util::ImageCreateInfo createInfo {};
	createInfo.width = width;
	createInfo.height = height;
	createInfo.format = static_cast<prosper::Format>(format);
	createInfo.usage = prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::ColorAttachmentBit;
	createInfo.postCreateLayout = prosper::ImageLayout::ColorAttachmentOptimal;
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	auto &context = c_engine->GetRenderContext();
	auto img = context.CreateImage(createInfo);
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	auto texture = context.CreateTexture({}, *img, imgViewCreateInfo, samplerCreateInfo);
	auto rt = context.CreateRenderTarget({texture}, static_cast<prosper::ShaderGraphics &>(*whShader.get()).GetRenderPass());
	rt->SetDebugName("lua_gradient");

	auto &setupCmd = context.GetSetupCommandBuffer();
	pragma::util::record_draw_gradient(context, setupCmd, *rt, dir, nodes);
	context.FlushSetupCommandBuffer();
	return texture;
}

static void push_image_buffers(lua_State *l, bool includeLayers, bool includeMipmaps, const std::vector<std::vector<std::shared_ptr<uimg::ImageBuffer>>> &imgBuffers)
{
	if(imgBuffers.empty())
		return;
	if(includeLayers == false) {
		auto &layer = imgBuffers.front();
		if(layer.empty())
			return;
		if(includeMipmaps == false) {
			auto &imgBuf = layer.front();
			Lua::Push(l, imgBuf);
			return;
		}
		auto t = Lua::CreateTable(l);
		for(auto i = decltype(layer.size()) {0u}; i < layer.size(); ++i) {
			Lua::PushInt(l, i + 1);
			Lua::Push(l, layer.at(i));
			Lua::SetTableValue(l, t);
		}
		return;
	}
	if(includeMipmaps == false) {
		auto t = Lua::CreateTable(l);
		for(auto i = decltype(imgBuffers.size()) {0u}; i < imgBuffers.size(); ++i) {
			auto &layer = imgBuffers.at(i);
			if(layer.empty())
				return;
			Lua::PushInt(l, i + 1);
			Lua::Push(l, layer.at(i));
			Lua::SetTableValue(l, t);
		}
		return;
	}
	auto t = Lua::CreateTable(l);
	for(auto i = decltype(imgBuffers.size()) {0u}; i < imgBuffers.size(); ++i) {
		auto &layer = imgBuffers.at(i);
		if(layer.empty())
			return;
		Lua::PushInt(l, i + 1);
		auto tMipmaps = Lua::CreateTable(l);
		for(auto j = decltype(layer.size()) {0u}; j < layer.size(); ++j) {
			Lua::PushInt(l, j + 1);
			Lua::Push(l, layer.at(i));
		}
		Lua::SetTableValue(l, t);
	}
}

static std::shared_ptr<prosper::Texture> get_color_attachment_texture(lua_State *l, Lua::Vulkan::RenderTarget &rt, uint32_t idx)
{
	auto n = rt.GetAttachmentCount();
	for(auto i = decltype(n) {0u}; i < n; ++i) {
		auto *tex = rt.GetTexture(i);
		if(!tex)
			return nullptr;
		if(prosper::util::is_depth_format(tex->GetImage().GetFormat()))
			continue;
		if(idx-- == 0)
			return tex->shared_from_this();
	}
	return nullptr;
}

void register_vulkan_lua_interface2(Lua::Interface &lua, luabind::module_ &prosperMod); // Registration is split up to avoid compiler errors
void ClientState::RegisterVulkanLuaInterface(Lua::Interface &lua)
{
	auto prosperMod = luabind::module_(lua.GetState(), "prosper");
	prosperMod[luabind::def("create_buffer", static_cast<std::shared_ptr<prosper::IBuffer> (*)(prosper::IPrContext &, prosper::util::BufferCreateInfo &, ::DataStream &)>(&Lua::Vulkan::create_buffer), luabind::render_context_policy<1> {}),
	  luabind::def("create_buffer", static_cast<std::shared_ptr<prosper::IBuffer> (*)(prosper::IPrContext &, prosper::util::BufferCreateInfo &)>(&Lua::Vulkan::create_buffer), luabind::render_context_policy<1> {}), luabind::def("create_descriptor_set", Lua::Vulkan::create_descriptor_set),
	  luabind::def("create_image", static_cast<std::shared_ptr<prosper::IImage> (*)(const prosper::util::ImageCreateInfo &, ::DataStream &)>(&Lua::Vulkan::create_image)),
	  luabind::def("create_image", static_cast<std::shared_ptr<prosper::IImage> (*)(const prosper::util::ImageCreateInfo &)>(&Lua::Vulkan::create_image)),
	  luabind::def("create_image", static_cast<std::shared_ptr<prosper::IImage> (*)(const std::array<std::shared_ptr<uimg::ImageBuffer>, 6> &)>(&Lua::Vulkan::create_image)),
	  luabind::def("create_image", static_cast<std::shared_ptr<prosper::IImage> (*)(const std::array<std::shared_ptr<uimg::ImageBuffer>, 6> &, const prosper::util::ImageCreateInfo &)>(&Lua::Vulkan::create_image)),
	  luabind::def("create_image", static_cast<std::shared_ptr<prosper::IImage> (*)(uimg::ImageBuffer &)>(&Lua::Vulkan::create_image)),
	  luabind::def("create_image", static_cast<std::shared_ptr<prosper::IImage> (*)(uimg::ImageBuffer &, const prosper::util::ImageCreateInfo &)>(&Lua::Vulkan::create_image)), luabind::def("create_image_view", Lua::Vulkan::create_image_view),
	  luabind::def("create_image_create_info", static_cast<prosper::util::ImageCreateInfo (*)(const uimg::ImageBuffer &, bool)>(&Lua::Vulkan::create_image_create_info)),
	  luabind::def("create_image_create_info", static_cast<prosper::util::ImageCreateInfo (*)(const uimg::ImageBuffer &)>(&Lua::Vulkan::create_image_create_info)),
	  luabind::def("create_texture", static_cast<std::shared_ptr<prosper::Texture> (*)(prosper::IImage &, const prosper::util::TextureCreateInfo &, const prosper::util::ImageViewCreateInfo &, const prosper::util::SamplerCreateInfo &)>(&Lua::Vulkan::create_texture)),
	  luabind::def("create_texture", static_cast<std::shared_ptr<prosper::Texture> (*)(prosper::IImage &, const prosper::util::TextureCreateInfo &, const prosper::util::ImageViewCreateInfo &)>(&Lua::Vulkan::create_texture)),
	  luabind::def("create_texture", static_cast<std::shared_ptr<prosper::Texture> (*)(prosper::IImage &, const prosper::util::TextureCreateInfo &)>(&Lua::Vulkan::create_texture)),
	  luabind::def("create_framebuffer", static_cast<std::shared_ptr<prosper::IFramebuffer> (*)(uint32_t, uint32_t, const std::vector<prosper::IImageView *> &, uint32_t)>(&Lua::Vulkan::create_framebuffer)),
	  luabind::def("create_framebuffer", static_cast<std::shared_ptr<prosper::IFramebuffer> (*)(uint32_t, uint32_t, const std::vector<prosper::IImageView *> &)>(&Lua::Vulkan::create_framebuffer)), luabind::def("create_render_pass", Lua::Vulkan::create_render_pass),

	  luabind::def("create_render_target", static_cast<std::shared_ptr<prosper::RenderTarget> (*)(const prosper::util::RenderTargetCreateInfo &, prosper::Texture &)>(&Lua::Vulkan::create_render_target)),
	  luabind::def("create_render_target", static_cast<std::shared_ptr<prosper::RenderTarget> (*)(const prosper::util::RenderTargetCreateInfo &, prosper::Texture &, Lua::Vulkan::RenderPass &)>(&Lua::Vulkan::create_render_target)),
	  luabind::def("create_render_target", static_cast<std::shared_ptr<prosper::RenderTarget> (*)(const prosper::util::RenderTargetCreateInfo &, const std::vector<std::shared_ptr<prosper::Texture>> &, Lua::Vulkan::RenderPass &)>(&Lua::Vulkan::create_render_target)),
	  luabind::def("create_render_target", static_cast<std::shared_ptr<prosper::RenderTarget> (*)(const prosper::util::RenderTargetCreateInfo &, const std::vector<std::shared_ptr<prosper::Texture>> &)>(&Lua::Vulkan::create_render_target)),

	  luabind::def("create_gradient_texture", &Lua::Vulkan::create_gradient_texture), luabind::def("blur_texture", &Lua::Vulkan::blur_texture), luabind::def("blur_texture", &Lua::Vulkan::blur_texture, luabind::default_parameter_policy<3, static_cast<uint32_t>(1)> {}),
	  luabind::def("create_event", &prosper::IPrContext::CreateEvent, luabind::render_context_policy<1> {}), luabind::def("create_fence", static_cast<std::shared_ptr<prosper::IFence> (*)(bool)>(&Lua::Vulkan::create_fence)),
	  luabind::def("create_fence", static_cast<std::shared_ptr<prosper::IFence> (*)()>(&Lua::Vulkan::create_fence)), luabind::def("calculate_mipmap_count", &prosper::util::calculate_mipmap_count),
	  luabind::def("calculate_mipmap_size", static_cast<Vector2i (*)(uint32_t, uint32_t, uint32_t)>(&Lua::Vulkan::calculate_mipmap_size)), luabind::def("calculate_mipmap_size", static_cast<uint32_t (*)(uint32_t, uint32_t)>(&Lua::Vulkan::calculate_mipmap_size)),
	  luabind::def("result_to_string", static_cast<std::string (*)(prosper::Result)>(&::prosper::util::to_string)), luabind::def("format_to_string", static_cast<std::string (*)(prosper::Format)>(&::prosper::util::to_string)),
	  luabind::def("shader_stage_to_string", static_cast<std::string (*)(prosper::ShaderStage)>(&::prosper::util::to_string)), luabind::def("is_depth_format", ::prosper::util::is_depth_format), luabind::def("is_compressed_format", ::prosper::util::is_compressed_format),
	  luabind::def("is_uncompressed_format", ::prosper::util::is_uncompressed_format), luabind::def("get_bit_size", ::prosper::util::get_bit_size), luabind::def("get_byte_size", ::prosper::util::get_byte_size),
	  luabind::def("wait_idle", &::prosper::IPrContext::WaitIdle, luabind::render_context_policy<1> {}), luabind::def("flush", &::prosper::IPrContext::Flush, luabind::render_context_policy<1> {}),
	  luabind::def("get_api_identifier", &::prosper::IPrContext::GetAPIIdentifier, luabind::render_context_policy<1> {}), luabind::def("get_api_abbreviation", &::prosper::IPrContext::GetAPIAbbreviation, luabind::render_context_policy<1> {}),
	  luabind::def("wait_for_current_swapchain_command_buffer_completion", static_cast<Lua::var<bool, std::string> (*)(lua_State *)>([](lua_State *l) -> Lua::var<bool, std::string> {
		  std::string err;
		  if(c_engine->GetRenderContext().WaitForCurrentSwapchainCommandBuffer(err))
			  return luabind::object {l, false};
		  return luabind::object {l, err};
	  })),
	  luabind::def("create_primary_command_buffer", static_cast<std::shared_ptr<prosper::ICommandBuffer> (*)(lua_State *)>([](lua_State *l) -> std::shared_ptr<prosper::ICommandBuffer> {
		  uint32_t universalQueueFamilyIndex;
		  return c_engine->GetRenderContext().AllocatePrimaryLevelCommandBuffer(prosper::QueueFamilyType::Universal, universalQueueFamilyIndex);
	  })),
	  luabind::def("create_secondary_command_buffer", static_cast<std::shared_ptr<prosper::ICommandBuffer> (*)(lua_State *)>([](lua_State *l) -> std::shared_ptr<prosper::ICommandBuffer> {
		  uint32_t universalQueueFamilyIndex;
		  return c_engine->GetRenderContext().AllocateSecondaryLevelCommandBuffer(prosper::QueueFamilyType::Universal, universalQueueFamilyIndex);
	  })),
	  luabind::def("create_window", &CEngine::CreateWindow)];

	prosperMod[luabind::namespace_("util")[luabind::def("get_square_vertex_uv_buffer", &Lua::Vulkan::get_square_vertex_uv_buffer), luabind::def("get_square_vertex_buffer", &Lua::Vulkan::get_square_vertex_buffer), luabind::def("get_square_uv_buffer", &Lua::Vulkan::get_square_uv_buffer),
	  luabind::def("get_square_vertices", &prosper::CommonBufferCache::GetSquareVertices), luabind::def("get_square_uv_coordinates", &prosper::CommonBufferCache::GetSquareUvCoordinates), luabind::def("get_square_vertex_count", &prosper::CommonBufferCache::GetSquareVertexCount),
	  luabind::def("get_square_vertex_format", &prosper::CommonBufferCache::GetSquareVertexFormat), luabind::def("get_square_uv_format", &prosper::CommonBufferCache::GetSquareUvFormat),
	  luabind::def("allocate_temporary_buffer", static_cast<std::shared_ptr<prosper::IBuffer> (*)(lua_State *, ::DataStream &)>(&Lua::Vulkan::allocate_temporary_buffer)),
	  luabind::def("allocate_temporary_buffer", static_cast<std::shared_ptr<prosper::IBuffer> (*)(lua_State *, uint32_t)>(&Lua::Vulkan::allocate_temporary_buffer)),

	  luabind::def("get_line_vertex_buffer", &Lua::Vulkan::get_line_vertex_buffer), luabind::def("get_line_vertices", &prosper::CommonBufferCache::GetLineVertices), luabind::def("get_line_vertex_count", &prosper::CommonBufferCache::GetLineVertexCount),
	  luabind::def("get_line_vertex_format", &prosper::CommonBufferCache::GetLineVertexFormat),
	  luabind::def(
	    "record_resize_image",
	    +[](Lua::Vulkan::CommandBuffer &cmd, Lua::Vulkan::DescriptorSet &dsg, Lua::Vulkan::RenderTarget &dstRt) {
		    if(!cmd.IsPrimary())
			    return;
		    auto &primaryCmdBuffer = dynamic_cast<prosper::IPrimaryCommandBuffer &>(cmd);
		    if(primaryCmdBuffer.RecordBeginRenderPass(dstRt)) {
			    auto *shader = static_cast<pragma::ShaderResizeImage *>(c_engine->GetShader("resize_image").get());
			    if(shader) {
				    auto *ds = dsg.GetDescriptorSet();
				    auto *imgSrc = ds ? ds->GetBoundImage(0) : nullptr;
				    auto &imgDst = dstRt.GetTexture().GetImage();
				    auto wSrc = imgSrc->GetWidth();
				    auto wDst = imgDst.GetWidth();
				    std::optional<pragma::ShaderResizeImage::LanczosFilter::Scale> scale {};
				    if(wSrc == wDst * 2)
					    scale = pragma::ShaderResizeImage::LanczosFilter::Scale::e2;
				    else if(wSrc == wDst * 4)
					    scale = pragma::ShaderResizeImage::LanczosFilter::Scale::e4;
				    if(scale.has_value()) {
					    pragma::ShaderResizeImage::LanczosFilter filter {};
					    filter.scale = *scale;
					    shader->RecordDraw(primaryCmdBuffer, *ds, filter);
				    }
				    else {
					    pragma::ShaderResizeImage::BicubicFilter filter {};
					    shader->RecordDraw(primaryCmdBuffer, *ds, filter);
				    }
			    }
		    }
	    }),
	  luabind::def(
	    "create_generic_image_descriptor_set", +[](Lua::Vulkan::Texture &tex) -> std::shared_ptr<Lua::Vulkan::DescriptorSet> {
		    auto *shader = static_cast<pragma::ShaderResizeImage *>(c_engine->GetShader("resize_image").get()); // TODO: use a generic shader
		    if(!shader)
			    return nullptr;
		    auto ds = shader->CreateDescriptorSetGroup(0);
		    ds->GetDescriptorSet()->SetBindingTexture(tex, 0u);
		    return ds;
	    })]];

	Lua::RegisterLibraryEnums(lua.GetState(), "prosper", {
		{"FORMAT_UNKNOWN", umath::to_integral(prosper::Format::Unknown)}, {"FORMAT_R4G4_UNORM_PACK8", umath::to_integral(prosper::Format::R4G4_UNorm_Pack8)}, {"FORMAT_R4G4B4A4_UNORM_PACK16", umath::to_integral(prosper::Format::R4G4B4A4_UNorm_Pack16)},
		  {"FORMAT_B4G4R4A4_UNORM_PACK16", umath::to_integral(prosper::Format::B4G4R4A4_UNorm_Pack16)}, {"FORMAT_R5G6B5_UNORM_PACK16", umath::to_integral(prosper::Format::R5G6B5_UNorm_Pack16)}, {"FORMAT_B5G6R5_UNORM_PACK16", umath::to_integral(prosper::Format::B5G6R5_UNorm_Pack16)},
		  {"FORMAT_R5G5B5A1_UNORM_PACK16", umath::to_integral(prosper::Format::R5G5B5A1_UNorm_Pack16)}, {"FORMAT_B5G5R5A1_UNORM_PACK16", umath::to_integral(prosper::Format::B5G5R5A1_UNorm_Pack16)},
		  {"FORMAT_A1R5G5B5_UNORM_PACK16", umath::to_integral(prosper::Format::A1R5G5B5_UNorm_Pack16)}, {"FORMAT_R8_UNORM", umath::to_integral(prosper::Format::R8_UNorm)}, {"FORMAT_R8_SNORM", umath::to_integral(prosper::Format::R8_SNorm)},
		  {"FORMAT_R8_USCALED_POOR_COVERAGE", umath::to_integral(prosper::Format::R8_UScaled_PoorCoverage)}, {"FORMAT_R8_SSCALED_POOR_COVERAGE", umath::to_integral(prosper::Format::R8_SScaled_PoorCoverage)}, {"FORMAT_R8_UINT", umath::to_integral(prosper::Format::R8_UInt)},
		  {"FORMAT_R8_SINT", umath::to_integral(prosper::Format::R8_SInt)}, {"FORMAT_R8_SRGB", umath::to_integral(prosper::Format::R8_SRGB)}, {"FORMAT_R8G8_UNORM", umath::to_integral(prosper::Format::R8G8_UNorm)}, {"FORMAT_R8G8_SNORM", umath::to_integral(prosper::Format::R8G8_SNorm)},
		  {"FORMAT_R8G8_USCALED_POOR_COVERAGE", umath::to_integral(prosper::Format::R8G8_UScaled_PoorCoverage)}, {"FORMAT_R8G8_SSCALED_POOR_COVERAGE", umath::to_integral(prosper::Format::R8G8_SScaled_PoorCoverage)}, {"FORMAT_R8G8_UINT", umath::to_integral(prosper::Format::R8G8_UInt)},
		  {"FORMAT_R8G8_SINT", umath::to_integral(prosper::Format::R8G8_SInt)}, {"FORMAT_R8G8_SRGB_POOR_COVERAGE", umath::to_integral(prosper::Format::R8G8_SRGB_PoorCoverage)}, {"FORMAT_R8G8B8_UNORM_POOR_COVERAGE", umath::to_integral(prosper::Format::R8G8B8_UNorm_PoorCoverage)},
		  {"FORMAT_R8G8B8_SNORM_POOR_COVERAGE", umath::to_integral(prosper::Format::R8G8B8_SNorm_PoorCoverage)}, {"FORMAT_R8G8B8_USCALED_POOR_COVERAGE", umath::to_integral(prosper::Format::R8G8B8_UScaled_PoorCoverage)},
		  {"FORMAT_R8G8B8_SSCALED_POOR_COVERAGE", umath::to_integral(prosper::Format::R8G8B8_SScaled_PoorCoverage)}, {"FORMAT_R8G8B8_UINT_POOR_COVERAGE", umath::to_integral(prosper::Format::R8G8B8_UInt_PoorCoverage)},
		  {"FORMAT_R8G8B8_SINT_POOR_COVERAGE", umath::to_integral(prosper::Format::R8G8B8_SInt_PoorCoverage)}, {"FORMAT_R8G8B8_SRGB_POOR_COVERAGE", umath::to_integral(prosper::Format::R8G8B8_SRGB_PoorCoverage)},
		  {"FORMAT_B8G8R8_UNORM_POOR_COVERAGE", umath::to_integral(prosper::Format::B8G8R8_UNorm_PoorCoverage)}, {"FORMAT_B8G8R8_SNORM_POOR_COVERAGE", umath::to_integral(prosper::Format::B8G8R8_SNorm_PoorCoverage)},
		  {"FORMAT_B8G8R8_USCALED_POOR_COVERAGE", umath::to_integral(prosper::Format::B8G8R8_UScaled_PoorCoverage)}, {"FORMAT_B8G8R8_SSCALED_POOR_COVERAGE", umath::to_integral(prosper::Format::B8G8R8_SScaled_PoorCoverage)},
		  {"FORMAT_B8G8R8_UINT_POOR_COVERAGE", umath::to_integral(prosper::Format::B8G8R8_UInt_PoorCoverage)}, {"FORMAT_B8G8R8_SINT_POOR_COVERAGE", umath::to_integral(prosper::Format::B8G8R8_SInt_PoorCoverage)},
		  {"FORMAT_B8G8R8_SRGB_POOR_COVERAGE", umath::to_integral(prosper::Format::B8G8R8_SRGB_PoorCoverage)}, {"FORMAT_R8G8B8A8_UNORM", umath::to_integral(prosper::Format::R8G8B8A8_UNorm)}, {"FORMAT_R8G8B8A8_SNORM", umath::to_integral(prosper::Format::R8G8B8A8_SNorm)},
		  {"FORMAT_R8G8B8A8_USCALED_POOR_COVERAGE", umath::to_integral(prosper::Format::R8G8B8A8_UScaled_PoorCoverage)}, {"FORMAT_R8G8B8A8_SSCALED_POOR_COVERAGE", umath::to_integral(prosper::Format::R8G8B8A8_SScaled_PoorCoverage)},
		  {"FORMAT_R8G8B8A8_UINT", umath::to_integral(prosper::Format::R8G8B8A8_UInt)}, {"FORMAT_R8G8B8A8_SINT", umath::to_integral(prosper::Format::R8G8B8A8_SInt)}, {"FORMAT_R8G8B8A8_SRGB", umath::to_integral(prosper::Format::R8G8B8A8_SRGB)},
		  {"FORMAT_B8G8R8A8_UNORM", umath::to_integral(prosper::Format::B8G8R8A8_UNorm)}, {"FORMAT_B8G8R8A8_SNORM", umath::to_integral(prosper::Format::B8G8R8A8_SNorm)}, {"FORMAT_B8G8R8A8_USCALED_POOR_COVERAGE", umath::to_integral(prosper::Format::B8G8R8A8_UScaled_PoorCoverage)},
		  {"FORMAT_B8G8R8A8_SSCALED_POOR_COVERAGE", umath::to_integral(prosper::Format::B8G8R8A8_SScaled_PoorCoverage)}, {"FORMAT_B8G8R8A8_UINT", umath::to_integral(prosper::Format::B8G8R8A8_UInt)}, {"FORMAT_B8G8R8A8_SINT", umath::to_integral(prosper::Format::B8G8R8A8_SInt)},
		  {"FORMAT_B8G8R8A8_SRGB", umath::to_integral(prosper::Format::B8G8R8A8_SRGB)}, {"FORMAT_A8B8G8R8_UNORM_PACK32", umath::to_integral(prosper::Format::A8B8G8R8_UNorm_Pack32)}, {"FORMAT_A8B8G8R8_SNORM_PACK32", umath::to_integral(prosper::Format::A8B8G8R8_SNorm_Pack32)},
		  {"FORMAT_A8B8G8R8_USCALED_PACK32_POOR_COVERAGE", umath::to_integral(prosper::Format::A8B8G8R8_UScaled_Pack32_PoorCoverage)}, {"FORMAT_A8B8G8R8_SSCALED_PACK32_POOR_COVERAGE", umath::to_integral(prosper::Format::A8B8G8R8_SScaled_Pack32_PoorCoverage)},
		  {"FORMAT_A8B8G8R8_UINT_PACK32", umath::to_integral(prosper::Format::A8B8G8R8_UInt_Pack32)}, {"FORMAT_A8B8G8R8_SINT_PACK32", umath::to_integral(prosper::Format::A8B8G8R8_SInt_Pack32)}, {"FORMAT_A8B8G8R8_SRGB_PACK32", umath::to_integral(prosper::Format::A8B8G8R8_SRGB_Pack32)},
		  {"FORMAT_A2R10G10B10_UNORM_PACK32", umath::to_integral(prosper::Format::A2R10G10B10_UNorm_Pack32)}, {"FORMAT_A2R10G10B10_SNORM_PACK32_POOR_COVERAGE", umath::to_integral(prosper::Format::A2R10G10B10_SNorm_Pack32_PoorCoverage)},
		  {"FORMAT_A2R10G10B10_USCALED_PACK32_POOR_COVERAGE", umath::to_integral(prosper::Format::A2R10G10B10_UScaled_Pack32_PoorCoverage)}, {"FORMAT_A2R10G10B10_SSCALED_PACK32_POOR_COVERAGE", umath::to_integral(prosper::Format::A2R10G10B10_SScaled_Pack32_PoorCoverage)},
		  {"FORMAT_A2R10G10B10_UINT_PACK32", umath::to_integral(prosper::Format::A2R10G10B10_UInt_Pack32)}, {"FORMAT_A2R10G10B10_SINT_PACK32_POOR_COVERAGE", umath::to_integral(prosper::Format::A2R10G10B10_SInt_Pack32_PoorCoverage)},
		  {"FORMAT_A2B10G10R10_UNORM_PACK32", umath::to_integral(prosper::Format::A2B10G10R10_UNorm_Pack32)}, {"FORMAT_A2B10G10R10_SNORM_PACK32_POOR_COVERAGE", umath::to_integral(prosper::Format::A2B10G10R10_SNorm_Pack32_PoorCoverage)},
		  {"FORMAT_A2B10G10R10_USCALED_PACK32_POOR_COVERAGE", umath::to_integral(prosper::Format::A2B10G10R10_UScaled_Pack32_PoorCoverage)}, {"FORMAT_A2B10G10R10_SSCALED_PACK32_POOR_COVERAGE", umath::to_integral(prosper::Format::A2B10G10R10_SScaled_Pack32_PoorCoverage)},
		  {"FORMAT_A2B10G10R10_UINT_PACK32", umath::to_integral(prosper::Format::A2B10G10R10_UInt_Pack32)}, {"FORMAT_A2B10G10R10_SINT_PACK32_POOR_COVERAGE", umath::to_integral(prosper::Format::A2B10G10R10_SInt_Pack32_PoorCoverage)},
		  {"FORMAT_R16_UNORM", umath::to_integral(prosper::Format::R16_UNorm)}, {"FORMAT_R16_SNORM", umath::to_integral(prosper::Format::R16_SNorm)}, {"FORMAT_R16_USCALED_POOR_COVERAGE", umath::to_integral(prosper::Format::R16_UScaled_PoorCoverage)},
		  {"FORMAT_R16_SSCALED_POOR_COVERAGE", umath::to_integral(prosper::Format::R16_SScaled_PoorCoverage)}, {"FORMAT_R16_UINT", umath::to_integral(prosper::Format::R16_UInt)}, {"FORMAT_R16_SINT", umath::to_integral(prosper::Format::R16_SInt)},
		  {"FORMAT_R16_SFLOAT", umath::to_integral(prosper::Format::R16_SFloat)}, {"FORMAT_R16G16_UNORM", umath::to_integral(prosper::Format::R16G16_UNorm)}, {"FORMAT_R16G16_SNORM", umath::to_integral(prosper::Format::R16G16_SNorm)},
		  {"FORMAT_R16G16_USCALED_POOR_COVERAGE", umath::to_integral(prosper::Format::R16G16_UScaled_PoorCoverage)}, {"FORMAT_R16G16_SSCALED_POOR_COVERAGE", umath::to_integral(prosper::Format::R16G16_SScaled_PoorCoverage)},
		  {"FORMAT_R16G16_UINT", umath::to_integral(prosper::Format::R16G16_UInt)}, {"FORMAT_R16G16_SINT", umath::to_integral(prosper::Format::R16G16_SInt)}, {"FORMAT_R16G16_SFLOAT", umath::to_integral(prosper::Format::R16G16_SFloat)},
		  {"FORMAT_R16G16B16_UNORM_POOR_COVERAGE", umath::to_integral(prosper::Format::R16G16B16_UNorm_PoorCoverage)}, {"FORMAT_R16G16B16_SNORM_POOR_COVERAGE", umath::to_integral(prosper::Format::R16G16B16_SNorm_PoorCoverage)},
		  {"FORMAT_R16G16B16_USCALED_POOR_COVERAGE", umath::to_integral(prosper::Format::R16G16B16_UScaled_PoorCoverage)}, {"FORMAT_R16G16B16_SSCALED_POOR_COVERAGE", umath::to_integral(prosper::Format::R16G16B16_SScaled_PoorCoverage)},
		  {"FORMAT_R16G16B16_UINT_POOR_COVERAGE", umath::to_integral(prosper::Format::R16G16B16_UInt_PoorCoverage)}, {"FORMAT_R16G16B16_SINT_POOR_COVERAGE", umath::to_integral(prosper::Format::R16G16B16_SInt_PoorCoverage)},
		  {"FORMAT_R16G16B16_SFLOAT_POOR_COVERAGE", umath::to_integral(prosper::Format::R16G16B16_SFloat_PoorCoverage)}, {"FORMAT_R16G16B16A16_UNORM", umath::to_integral(prosper::Format::R16G16B16A16_UNorm)},
		  {"FORMAT_R16G16B16A16_SNORM", umath::to_integral(prosper::Format::R16G16B16A16_SNorm)}, {"FORMAT_R16G16B16A16_USCALED_POOR_COVERAGE", umath::to_integral(prosper::Format::R16G16B16A16_UScaled_PoorCoverage)},
		  {"FORMAT_R16G16B16A16_SSCALED_POOR_COVERAGE", umath::to_integral(prosper::Format::R16G16B16A16_SScaled_PoorCoverage)}, {"FORMAT_R16G16B16A16_UINT", umath::to_integral(prosper::Format::R16G16B16A16_UInt)},
		  {"FORMAT_R16G16B16A16_SINT", umath::to_integral(prosper::Format::R16G16B16A16_SInt)}, {"FORMAT_R16G16B16A16_SFLOAT", umath::to_integral(prosper::Format::R16G16B16A16_SFloat)}, {"FORMAT_R32_UINT", umath::to_integral(prosper::Format::R32_UInt)},
		  {"FORMAT_R32_SINT", umath::to_integral(prosper::Format::R32_SInt)}, {"FORMAT_R32_SFLOAT", umath::to_integral(prosper::Format::R32_SFloat)}, {"FORMAT_R32G32_UINT", umath::to_integral(prosper::Format::R32G32_UInt)},
		  {"FORMAT_R32G32_SINT", umath::to_integral(prosper::Format::R32G32_SInt)}, {"FORMAT_R32G32_SFLOAT", umath::to_integral(prosper::Format::R32G32_SFloat)}, {"FORMAT_R32G32B32_UINT", umath::to_integral(prosper::Format::R32G32B32_UInt)},
		  {"FORMAT_R32G32B32_SINT", umath::to_integral(prosper::Format::R32G32B32_SInt)}, {"FORMAT_R32G32B32_SFLOAT", umath::to_integral(prosper::Format::R32G32B32_SFloat)}, {"FORMAT_R32G32B32A32_UINT", umath::to_integral(prosper::Format::R32G32B32A32_UInt)},
		  {"FORMAT_R32G32B32A32_SINT", umath::to_integral(prosper::Format::R32G32B32A32_SInt)}, {"FORMAT_R32G32B32A32_SFLOAT", umath::to_integral(prosper::Format::R32G32B32A32_SFloat)}, {"FORMAT_R64_UINT_POOR_COVERAGE", umath::to_integral(prosper::Format::R64_UInt_PoorCoverage)},
		  {"FORMAT_R64_SINT_POOR_COVERAGE", umath::to_integral(prosper::Format::R64_SInt_PoorCoverage)}, {"FORMAT_R64_SFLOAT_POOR_COVERAGE", umath::to_integral(prosper::Format::R64_SFloat_PoorCoverage)},
		  {"FORMAT_R64G64_UINT_POOR_COVERAGE", umath::to_integral(prosper::Format::R64G64_UInt_PoorCoverage)}, {"FORMAT_R64G64_SINT_POOR_COVERAGE", umath::to_integral(prosper::Format::R64G64_SInt_PoorCoverage)},
		  {"FORMAT_R64G64_SFLOAT_POOR_COVERAGE", umath::to_integral(prosper::Format::R64G64_SFloat_PoorCoverage)}, {"FORMAT_R64G64B64_UINT_POOR_COVERAGE", umath::to_integral(prosper::Format::R64G64B64_UInt_PoorCoverage)},
		  {"FORMAT_R64G64B64_SINT_POOR_COVERAGE", umath::to_integral(prosper::Format::R64G64B64_SInt_PoorCoverage)}, {"FORMAT_R64G64B64_SFLOAT_POOR_COVERAGE", umath::to_integral(prosper::Format::R64G64B64_SFloat_PoorCoverage)},
		  {"FORMAT_R64G64B64A64_UINT_POOR_COVERAGE", umath::to_integral(prosper::Format::R64G64B64A64_UInt_PoorCoverage)}, {"FORMAT_R64G64B64A64_SINT_POOR_COVERAGE", umath::to_integral(prosper::Format::R64G64B64A64_SInt_PoorCoverage)},
		  {"FORMAT_R64G64B64A64_SFLOAT_POOR_COVERAGE", umath::to_integral(prosper::Format::R64G64B64A64_SFloat_PoorCoverage)}, {"FORMAT_B10G11R11_UFLOAT_PACK32", umath::to_integral(prosper::Format::B10G11R11_UFloat_Pack32)},
		  {"FORMAT_E5B9G9R9_UFLOAT_PACK32", umath::to_integral(prosper::Format::E5B9G9R9_UFloat_Pack32)}, {"FORMAT_D16_UNORM", umath::to_integral(prosper::Format::D16_UNorm)},
		  {"FORMAT_X8_D24_UNORM_PACK32_POOR_COVERAGE", umath::to_integral(prosper::Format::X8_D24_UNorm_Pack32_PoorCoverage)}, {"FORMAT_D32_SFLOAT", umath::to_integral(prosper::Format::D32_SFloat)},
		  {"FORMAT_S8_UINT_POOR_COVERAGE", umath::to_integral(prosper::Format::S8_UInt_PoorCoverage)}, {"FORMAT_D16_UNORM_S8_UINT_POOR_COVERAGE", umath::to_integral(prosper::Format::D16_UNorm_S8_UInt_PoorCoverage)},
		  {"FORMAT_D24_UNORM_S8_UINT_POOR_COVERAGE", umath::to_integral(prosper::Format::D24_UNorm_S8_UInt_PoorCoverage)}, {"FORMAT_D32_SFLOAT_S8_UINT", umath::to_integral(prosper::Format::D32_SFloat_S8_UInt)},
		  {"FORMAT_BC1_RGB_UNORM_BLOCK", umath::to_integral(prosper::Format::BC1_RGB_UNorm_Block)}, {"FORMAT_BC1_RGB_SRGB_BLOCK", umath::to_integral(prosper::Format::BC1_RGB_SRGB_Block)}, {"FORMAT_BC1_RGBA_UNORM_BLOCK", umath::to_integral(prosper::Format::BC1_RGBA_UNorm_Block)},
		  {"FORMAT_BC1_RGBA_SRGB_BLOCK", umath::to_integral(prosper::Format::BC1_RGBA_SRGB_Block)}, {"FORMAT_BC2_UNORM_BLOCK", umath::to_integral(prosper::Format::BC2_UNorm_Block)}, {"FORMAT_BC2_SRGB_BLOCK", umath::to_integral(prosper::Format::BC2_SRGB_Block)},
		  {"FORMAT_BC3_UNORM_BLOCK", umath::to_integral(prosper::Format::BC3_UNorm_Block)}, {"FORMAT_BC3_SRGB_BLOCK", umath::to_integral(prosper::Format::BC3_SRGB_Block)}, {"FORMAT_BC4_UNORM_BLOCK", umath::to_integral(prosper::Format::BC4_UNorm_Block)},
		  {"FORMAT_BC4_SNORM_BLOCK", umath::to_integral(prosper::Format::BC4_SNorm_Block)}, {"FORMAT_BC5_UNORM_BLOCK", umath::to_integral(prosper::Format::BC5_UNorm_Block)}, {"FORMAT_BC5_SNORM_BLOCK", umath::to_integral(prosper::Format::BC5_SNorm_Block)},
		  {"FORMAT_BC6H_UFLOAT_BLOCK", umath::to_integral(prosper::Format::BC6H_UFloat_Block)}, {"FORMAT_BC6H_SFLOAT_BLOCK", umath::to_integral(prosper::Format::BC6H_SFloat_Block)}, {"FORMAT_BC7_UNORM_BLOCK", umath::to_integral(prosper::Format::BC7_UNorm_Block)},
		  {"FORMAT_BC7_SRGB_BLOCK", umath::to_integral(prosper::Format::BC7_SRGB_Block)}, {"FORMAT_ETC2_R8G8B8_UNORM_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ETC2_R8G8B8_UNorm_Block_PoorCoverage)},
		  {"FORMAT_ETC2_R8G8B8_SRGB_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ETC2_R8G8B8_SRGB_Block_PoorCoverage)}, {"FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ETC2_R8G8B8A1_UNorm_Block_PoorCoverage)},
		  {"FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ETC2_R8G8B8A1_SRGB_Block_PoorCoverage)}, {"FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ETC2_R8G8B8A8_UNorm_Block_PoorCoverage)},
		  {"FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ETC2_R8G8B8A8_SRGB_Block_PoorCoverage)}, {"FORMAT_EAC_R11_UNORM_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::EAC_R11_UNorm_Block_PoorCoverage)},
		  {"FORMAT_EAC_R11_SNORM_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::EAC_R11_SNorm_Block_PoorCoverage)}, {"FORMAT_EAC_R11G11_UNORM_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::EAC_R11G11_UNorm_Block_PoorCoverage)},
		  {"FORMAT_EAC_R11G11_SNORM_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::EAC_R11G11_SNorm_Block_PoorCoverage)}, {"FORMAT_ASTC_4x4_UNORM_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ASTC_4x4_UNorm_Block_PoorCoverage)},
		  {"FORMAT_ASTC_4x4_SRGB_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ASTC_4x4_SRGB_Block_PoorCoverage)}, {"FORMAT_ASTC_5x4_UNORM_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ASTC_5x4_UNorm_Block_PoorCoverage)},
		  {"FORMAT_ASTC_5x4_SRGB_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ASTC_5x4_SRGB_Block_PoorCoverage)}, {"FORMAT_ASTC_5x5_UNORM_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ASTC_5x5_UNorm_Block_PoorCoverage)},
		  {"FORMAT_ASTC_5x5_SRGB_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ASTC_5x5_SRGB_Block_PoorCoverage)}, {"FORMAT_ASTC_6x5_UNORM_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ASTC_6x5_UNorm_Block_PoorCoverage)},
		  {"FORMAT_ASTC_6x5_SRGB_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ASTC_6x5_SRGB_Block_PoorCoverage)}, {"FORMAT_ASTC_6x6_UNORM_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ASTC_6x6_UNorm_Block_PoorCoverage)},
		  {"FORMAT_ASTC_6x6_SRGB_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ASTC_6x6_SRGB_Block_PoorCoverage)}, {"FORMAT_ASTC_8x5_UNORM_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ASTC_8x5_UNorm_Block_PoorCoverage)},
		  {"FORMAT_ASTC_8x5_SRGB_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ASTC_8x5_SRGB_Block_PoorCoverage)}, {"FORMAT_ASTC_8x6_UNORM_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ASTC_8x6_UNorm_Block_PoorCoverage)},
		  {"FORMAT_ASTC_8x6_SRGB_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ASTC_8x6_SRGB_Block_PoorCoverage)}, {"FORMAT_ASTC_8x8_UNORM_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ASTC_8x8_UNorm_Block_PoorCoverage)},
		  {"FORMAT_ASTC_8x8_SRGB_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ASTC_8x8_SRGB_Block_PoorCoverage)}, {"FORMAT_ASTC_10x5_UNORM_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ASTC_10x5_UNorm_Block_PoorCoverage)},
		  {"FORMAT_ASTC_10x5_SRGB_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ASTC_10x5_SRGB_Block_PoorCoverage)}, {"FORMAT_ASTC_10x6_UNORM_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ASTC_10x6_UNorm_Block_PoorCoverage)},
		  {"FORMAT_ASTC_10x6_SRGB_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ASTC_10x6_SRGB_Block_PoorCoverage)}, {"FORMAT_ASTC_10x8_UNORM_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ASTC_10x8_UNorm_Block_PoorCoverage)},
		  {"FORMAT_ASTC_10x8_SRGB_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ASTC_10x8_SRGB_Block_PoorCoverage)}, {"FORMAT_ASTC_10x10_UNORM_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ASTC_10x10_UNorm_Block_PoorCoverage)},
		  {"FORMAT_ASTC_10x10_SRGB_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ASTC_10x10_SRGB_Block_PoorCoverage)}, {"FORMAT_ASTC_12x10_UNORM_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ASTC_12x10_UNorm_Block_PoorCoverage)},
		  {"FORMAT_ASTC_12x10_SRGB_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ASTC_12x10_SRGB_Block_PoorCoverage)}, {"FORMAT_ASTC_12x12_UNORM_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ASTC_12x12_UNorm_Block_PoorCoverage)},
		  {"FORMAT_ASTC_12x12_SRGB_BLOCK_POOR_COVERAGE", umath::to_integral(prosper::Format::ASTC_12x12_SRGB_Block_PoorCoverage)},

		  {"RESULT_SUCCESS", umath::to_integral(prosper::Result::Success)}, {"RESULT_NOT_READY", umath::to_integral(prosper::Result::NotReady)}, {"RESULT_TIMEOUT", umath::to_integral(prosper::Result::Timeout)}, {"RESULT_EVENT_SET", umath::to_integral(prosper::Result::EventSet)},
		  {"RESULT_EVENT_RESET", umath::to_integral(prosper::Result::EventReset)}, {"RESULT_INCOMPLETE", umath::to_integral(prosper::Result::Incomplete)}, {"RESULT_ERROR_OUT_OF_HOST_MEMORY", umath::to_integral(prosper::Result::ErrorOutOfHostMemory)},
		  {"RESULT_ERROR_OUT_OF_DEVICE_MEMORY", umath::to_integral(prosper::Result::ErrorOutOfDeviceMemory)}, {"RESULT_ERROR_INITIALIZATION_FAILED", umath::to_integral(prosper::Result::ErrorInitializationFailed)},
		  {"RESULT_ERROR_DEVICE_LOST", umath::to_integral(prosper::Result::ErrorDeviceLost)}, {"RESULT_ERROR_MEMORY_MAP_FAILED", umath::to_integral(prosper::Result::ErrorMemoryMapFailed)}, {"RESULT_ERROR_LAYER_NOT_PRESENT", umath::to_integral(prosper::Result::ErrorLayerNotPresent)},
		  {"RESULT_ERROR_EXTENSION_NOT_PRESENT", umath::to_integral(prosper::Result::ErrorExtensionNotPresent)}, {"RESULT_ERROR_FEATURE_NOT_PRESENT", umath::to_integral(prosper::Result::ErrorFeatureNotPresent)},
		  {"RESULT_ERROR_INCOMPATIBLE_DRIVER", umath::to_integral(prosper::Result::ErrorIncompatibleDriver)}, {"RESULT_ERROR_TOO_MANY_OBJECTS", umath::to_integral(prosper::Result::ErrorTooManyObjects)},
		  {"RESULT_ERROR_FORMAT_NOT_SUPPORTED", umath::to_integral(prosper::Result::ErrorFormatNotSupported)}, {"RESULT_ERROR_SURFACE_LOST_KHR", umath::to_integral(prosper::Result::ErrorSurfaceLostKHR)},
		  {"RESULT_ERROR_NATIVE_WINDOW_IN_USE_KHR", umath::to_integral(prosper::Result::ErrorNativeWindowInUseKHR)}, {"RESULT_SUBOPTIMAL_KHR", umath::to_integral(prosper::Result::SuboptimalKHR)}, {"RESULT_ERROR_OUT_OF_DATE_KHR", umath::to_integral(prosper::Result::ErrorOutOfDateKHR)},
		  {"RESULT_ERROR_INCOMPATIBLE_DISPLAY_KHR", umath::to_integral(prosper::Result::ErrorIncompatibleDisplayKHR)}, {"RESULT_ERROR_VALIDATION_FAILED_EXT", umath::to_integral(prosper::Result::ErrorValidationFailedEXT)},

		  {"SAMPLER_MIPMAP_MODE_LINEAR", umath::to_integral(prosper::SamplerMipmapMode::Linear)}, {"SAMPLER_MIPMAP_MODE_NEAREST", umath::to_integral(prosper::SamplerMipmapMode::Nearest)},

		  {"SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER", umath::to_integral(prosper::SamplerAddressMode::ClampToBorder)}, {"SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE", umath::to_integral(prosper::SamplerAddressMode::ClampToEdge)},
		  {"SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE", umath::to_integral(prosper::SamplerAddressMode::MirrorClampToEdge)}, {"SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT", umath::to_integral(prosper::SamplerAddressMode::MirroredRepeat)},
		  {"SAMPLER_ADDRESS_MODE_REPEAT", umath::to_integral(prosper::SamplerAddressMode::Repeat)},

		  {"COMPARE_OP_ALWAYS", umath::to_integral(prosper::CompareOp::Always)}, {"COMPARE_OP_EQUAL", umath::to_integral(prosper::CompareOp::Equal)}, {"COMPARE_OP_GREATER", umath::to_integral(prosper::CompareOp::Greater)},
		  {"COMPARE_OP_GREATER_OR_EQUAL", umath::to_integral(prosper::CompareOp::GreaterOrEqual)}, {"COMPARE_OP_LESS", umath::to_integral(prosper::CompareOp::Less)}, {"COMPARE_OP_LESS_OR_EQUAL", umath::to_integral(prosper::CompareOp::LessOrEqual)},
		  {"COMPARE_OP_NEVER", umath::to_integral(prosper::CompareOp::Never)}, {"COMPARE_OP_NOT_EQUAL", umath::to_integral(prosper::CompareOp::NotEqual)},

		  {"BORDER_COLOR_FLOAT_OPAQUE_BLACK", umath::to_integral(prosper::BorderColor::FloatOpaqueBlack)}, {"BORDER_COLOR_FLOAT_OPAQUE_WHITE", umath::to_integral(prosper::BorderColor::FloatOpaqueWhite)},
		  {"BORDER_COLOR_floatRANSPARENT_BLACK", umath::to_integral(prosper::BorderColor::FloatTransparentBlack)}, {"BORDER_COLOR_INT_OPAQUE_BLACK", umath::to_integral(prosper::BorderColor::IntOpaqueBlack)},
		  {"BORDER_COLOR_INT_OPAQUE_WHITE", umath::to_integral(prosper::BorderColor::IntOpaqueWhite)}, {"BORDER_COLOR_INT_TRANSPARENT_BLACK", umath::to_integral(prosper::BorderColor::IntTransparentBlack)},

		  {"IMAGE_LAYOUT_UNDEFINED", umath::to_integral(prosper::ImageLayout::Undefined)}, {"IMAGE_LAYOUT_GENERAL", umath::to_integral(prosper::ImageLayout::General)}, {"IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL", umath::to_integral(prosper::ImageLayout::ColorAttachmentOptimal)},
		  {"IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL", umath::to_integral(prosper::ImageLayout::DepthStencilAttachmentOptimal)}, {"IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL", umath::to_integral(prosper::ImageLayout::DepthStencilReadOnlyOptimal)},
		  {"IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL", umath::to_integral(prosper::ImageLayout::ShaderReadOnlyOptimal)}, {"IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL", umath::to_integral(prosper::ImageLayout::TransferSrcOptimal)},
		  {"IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL", umath::to_integral(prosper::ImageLayout::TransferDstOptimal)}, {"IMAGE_LAYOUT_PREINITIALIZED", umath::to_integral(prosper::ImageLayout::Preinitialized)},
		  {"IMAGE_LAYOUT_PRESENT_SRC_KHR", umath::to_integral(prosper::ImageLayout::PresentSrcKHR)},

		  {"ATTACHMENT_LOAD_OP_LOAD", umath::to_integral(prosper::AttachmentLoadOp::Load)}, {"ATTACHMENT_LOAD_OP_CLEAR", umath::to_integral(prosper::AttachmentLoadOp::Clear)}, {"ATTACHMENT_LOAD_OP_DONT_CARE", umath::to_integral(prosper::AttachmentLoadOp::DontCare)},

		  {"ATTACHMENT_STORE_OP_STORE", umath::to_integral(prosper::AttachmentStoreOp::Store)}, {"ATTACHMENT_STORE_OP_DONT_CARE", umath::to_integral(prosper::AttachmentStoreOp::DontCare)},

		  {"IMAGE_TYPE_1D", umath::to_integral(prosper::ImageType::e1D)}, {"IMAGE_TYPE_2D", umath::to_integral(prosper::ImageType::e2D)}, {"IMAGE_TYPE_3D", umath::to_integral(prosper::ImageType::e3D)},

		  {"IMAGE_TILING_OPTIMAL", umath::to_integral(prosper::ImageTiling::Optimal)}, {"IMAGE_TILING_LINEAR", umath::to_integral(prosper::ImageTiling::Linear)},

		  {"IMAGE_VIEW_TYPE_1D", umath::to_integral(prosper::ImageViewType::e1D)}, {"IMAGE_VIEW_TYPE_2D", umath::to_integral(prosper::ImageViewType::e2D)}, {"IMAGE_VIEW_TYPE_3D", umath::to_integral(prosper::ImageViewType::e3D)},
		  {"IMAGE_VIEW_TYPE_CUBE", umath::to_integral(prosper::ImageViewType::Cube)}, {"IMAGE_VIEW_TYPE_1D_ARRAY", umath::to_integral(prosper::ImageViewType::e1DArray)}, {"IMAGE_VIEW_TYPE_2D_ARRAY", umath::to_integral(prosper::ImageViewType::e2DArray)},
		  {"IMAGE_VIEW_TYPE_CUBE_ARRAY", umath::to_integral(prosper::ImageViewType::CubeArray)},

		  {"COMPONENT_SWIZZLE_IDENTITY", umath::to_integral(prosper::ComponentSwizzle::Identity)}, {"COMPONENT_SWIZZLE_ZERO", umath::to_integral(prosper::ComponentSwizzle::Zero)}, {"COMPONENT_SWIZZLE_ONE", umath::to_integral(prosper::ComponentSwizzle::One)},
		  {"COMPONENT_SWIZZLE_R", umath::to_integral(prosper::ComponentSwizzle::R)}, {"COMPONENT_SWIZZLE_G", umath::to_integral(prosper::ComponentSwizzle::G)}, {"COMPONENT_SWIZZLE_B", umath::to_integral(prosper::ComponentSwizzle::B)},
		  {"COMPONENT_SWIZZLE_A", umath::to_integral(prosper::ComponentSwizzle::A)},

		  {"DESCRIPTOR_TYPE_SAMPLER", umath::to_integral(prosper::DescriptorType::Sampler)}, {"DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER", umath::to_integral(prosper::DescriptorType::CombinedImageSampler)},
		  {"DESCRIPTOR_TYPE_SAMPLED_IMAGE", umath::to_integral(prosper::DescriptorType::SampledImage)}, {"DESCRIPTOR_TYPE_STORAGE_IMAGE", umath::to_integral(prosper::DescriptorType::StorageImage)},
		  {"DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER", umath::to_integral(prosper::DescriptorType::UniformTexelBuffer)}, {"DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER", umath::to_integral(prosper::DescriptorType::StorageTexelBuffer)},
		  {"DESCRIPTOR_TYPE_UNIFORM_BUFFER", umath::to_integral(prosper::DescriptorType::UniformBuffer)}, {"DESCRIPTOR_TYPE_STORAGE_BUFFER", umath::to_integral(prosper::DescriptorType::StorageBuffer)},
		  {"DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC", umath::to_integral(prosper::DescriptorType::UniformBufferDynamic)}, {"DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC", umath::to_integral(prosper::DescriptorType::StorageBufferDynamic)},
		  {"DESCRIPTOR_TYPE_INPUT_ATTACHMENT", umath::to_integral(prosper::DescriptorType::InputAttachment)},

		  {"QUERY_TYPE_OCCLUSION", umath::to_integral(prosper::QueryType::Occlusion)}, {"QUERY_TYPE_PIPELINE_STATISTICS", umath::to_integral(prosper::QueryType::PipelineStatistics)}, {"QUERY_TYPE_TIMESTAMP", umath::to_integral(prosper::QueryType::Timestamp)},

		  {"PIPELINE_BIND_POINT_GRAPHICS", umath::to_integral(prosper::PipelineBindPoint::Graphics)}, {"PIPELINE_BIND_POINT_COMPUTE", umath::to_integral(prosper::PipelineBindPoint::Compute)},

		  {"PRIMITIVE_TOPOLOGY_POINT_LIST", umath::to_integral(prosper::PrimitiveTopology::PointList)}, {"PRIMITIVE_TOPOLOGY_LINE_LIST", umath::to_integral(prosper::PrimitiveTopology::LineList)}, {"PRIMITIVE_TOPOLOGY_LINE_STRIP", umath::to_integral(prosper::PrimitiveTopology::LineStrip)},
		  {"PRIMITIVE_TOPOLOGY_TRIANGLE_LIST", umath::to_integral(prosper::PrimitiveTopology::TriangleList)}, {"PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP", umath::to_integral(prosper::PrimitiveTopology::TriangleStrip)},
		  {"PRIMITIVE_TOPOLOGY_TRIANGLE_FAN", umath::to_integral(prosper::PrimitiveTopology::TriangleFan)}, {"PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY", umath::to_integral(prosper::PrimitiveTopology::LineListWithAdjacency)},
		  {"PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY", umath::to_integral(prosper::PrimitiveTopology::LineStripWithAdjacency)}, {"PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY", umath::to_integral(prosper::PrimitiveTopology::TriangleListWithAdjacency)},
		  {"PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY", umath::to_integral(prosper::PrimitiveTopology::TriangleStripWithAdjacency)}, {"PRIMITIVE_TOPOLOGY_PATCH_LIST", umath::to_integral(prosper::PrimitiveTopology::PatchList)},

		  {"SHARING_MODE_EXCLUSIVE", umath::to_integral(prosper::SharingMode::Exclusive)}, {"SHARING_MODE_CONCURRENT", umath::to_integral(prosper::SharingMode::Concurrent)},

		  {"INDEX_TYPE_UINT16", umath::to_integral(prosper::IndexType::UInt16)}, {"INDEX_TYPE_UINT32", umath::to_integral(prosper::IndexType::UInt32)},

		  {"FILTER_NEAREST", umath::to_integral(prosper::Filter::Nearest)}, {"FILTER_LINEAR", umath::to_integral(prosper::Filter::Linear)},

		  {"POLYGON_MODE_FILL", umath::to_integral(prosper::PolygonMode::Fill)}, {"POLYGON_MODE_LINE", umath::to_integral(prosper::PolygonMode::Line)}, {"POLYGON_MODE_POINT", umath::to_integral(prosper::PolygonMode::Point)},

		  {"CULL_MODE_NONE", umath::to_integral(prosper::CullModeFlags::None)}, {"CULL_MODE_FRONT_BIT", umath::to_integral(prosper::CullModeFlags::FrontBit)}, {"CULL_MODE_BACK_BIT", umath::to_integral(prosper::CullModeFlags::BackBit)},
		  {"CULL_MODE_FRONT_AND_BACK", umath::to_integral(prosper::CullModeFlags::FrontAndBack)},

		  {"FRONT_FACE_COUNTER_CLOCKWISE", umath::to_integral(prosper::FrontFace::CounterClockwise)}, {"FRONT_FACE_CLOCKWISE", umath::to_integral(prosper::FrontFace::Clockwise)},

		  {"BLEND_FACTOR_ZERO", umath::to_integral(prosper::BlendFactor::Zero)}, {"BLEND_FACTOR_ONE", umath::to_integral(prosper::BlendFactor::One)}, {"BLEND_FACTOR_SRC_COLOR", umath::to_integral(prosper::BlendFactor::SrcColor)},
		  {"BLEND_FACTOR_ONE_MINUS_SRC_COLOR", umath::to_integral(prosper::BlendFactor::OneMinusSrcColor)}, {"BLEND_FACTOR_DST_COLOR", umath::to_integral(prosper::BlendFactor::DstColor)}, {"BLEND_FACTOR_ONE_MINUS_DST_COLOR", umath::to_integral(prosper::BlendFactor::OneMinusDstColor)},
		  {"BLEND_FACTOR_SRC_ALPHA", umath::to_integral(prosper::BlendFactor::SrcAlpha)}, {"BLEND_FACTOR_ONE_MINUS_SRC_ALPHA", umath::to_integral(prosper::BlendFactor::OneMinusSrcAlpha)}, {"BLEND_FACTOR_DST_ALPHA", umath::to_integral(prosper::BlendFactor::DstAlpha)},
		  {"BLEND_FACTOR_ONE_MINUS_DST_ALPHA", umath::to_integral(prosper::BlendFactor::OneMinusDstAlpha)}, {"BLEND_FACTOR_CONSTANT_COLOR", umath::to_integral(prosper::BlendFactor::ConstantColor)},
		  {"BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR", umath::to_integral(prosper::BlendFactor::OneMinusConstantColor)}, {"BLEND_FACTOR_CONSTANT_ALPHA", umath::to_integral(prosper::BlendFactor::ConstantAlpha)},
		  {"BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA", umath::to_integral(prosper::BlendFactor::OneMinusConstantAlpha)}, {"BLEND_FACTOR_SRC_ALPHA_SATURATE", umath::to_integral(prosper::BlendFactor::SrcAlphaSaturate)},
		  {"BLEND_FACTOR_SRC1_COLOR", umath::to_integral(prosper::BlendFactor::Src1Color)}, {"BLEND_FACTOR_ONE_MINUS_SRC1_COLOR", umath::to_integral(prosper::BlendFactor::OneMinusSrc1Color)}, {"BLEND_FACTOR_SRC1_ALPHA", umath::to_integral(prosper::BlendFactor::Src1Alpha)},
		  {"BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA", umath::to_integral(prosper::BlendFactor::OneMinusSrc1Alpha)},

		  {"BLEND_OP_ADD", umath::to_integral(prosper::BlendOp::Add)}, {"BLEND_OP_SUBTRACT", umath::to_integral(prosper::BlendOp::Subtract)}, {"BLEND_OP_REVERSE_SUBTRACT", umath::to_integral(prosper::BlendOp::ReverseSubtract)}, {"BLEND_OP_MIN", umath::to_integral(prosper::BlendOp::Min)},
		  {"BLEND_OP_MAX", umath::to_integral(prosper::BlendOp::Max)},

		  {"STENCIL_OP_KEEP", umath::to_integral(prosper::StencilOp::Keep)}, {"STENCIL_OP_ZERO", umath::to_integral(prosper::StencilOp::Zero)}, {"STENCIL_OP_REPLACE", umath::to_integral(prosper::StencilOp::Replace)},
		  {"STENCIL_OP_INCREMENT_AND_CLAMP", umath::to_integral(prosper::StencilOp::IncrementAndClamp)}, {"STENCIL_OP_DECREMENT_AND_CLAMP", umath::to_integral(prosper::StencilOp::DecrementAndClamp)}, {"STENCIL_OP_INVERT", umath::to_integral(prosper::StencilOp::Invert)},
		  {"STENCIL_OP_INCREMENT_AND_WRAP", umath::to_integral(prosper::StencilOp::IncrementAndWrap)}, {"STENCIL_OP_DECREMENT_AND_WRAP", umath::to_integral(prosper::StencilOp::DecrementAndWrap)},

		  {"LOGIC_OP_CLEAR", umath::to_integral(prosper::LogicOp::Clear)}, {"LOGIC_OP_AND", umath::to_integral(prosper::LogicOp::And)}, {"LOGIC_OP_AND_REVERSE", umath::to_integral(prosper::LogicOp::AndReverse)}, {"LOGIC_OP_COPY", umath::to_integral(prosper::LogicOp::Copy)},
		  {"LOGIC_OP_AND_INVERTED", umath::to_integral(prosper::LogicOp::AndInverted)}, {"LOGIC_OP_NO_OP", umath::to_integral(prosper::LogicOp::NoOp)}, {"LOGIC_OP_XOR", umath::to_integral(prosper::LogicOp::Xor)}, {"LOGIC_OP_OR", umath::to_integral(prosper::LogicOp::Or)},
		  {"LOGIC_OP_NOR", umath::to_integral(prosper::LogicOp::Nor)}, {"LOGIC_OP_EQUIVALENT", umath::to_integral(prosper::LogicOp::Equivalent)}, {"LOGIC_OP_INVERT", umath::to_integral(prosper::LogicOp::Invert)}, {"LOGIC_OP_OR_REVERSE", umath::to_integral(prosper::LogicOp::OrReverse)},
		  {"LOGIC_OP_COPY_INVERTED", umath::to_integral(prosper::LogicOp::CopyInverted)}, {"LOGIC_OP_OR_INVERTED", umath::to_integral(prosper::LogicOp::OrInverted)}, {"LOGIC_OP_NAND", umath::to_integral(prosper::LogicOp::Nand)}, {"LOGIC_OP_SET", umath::to_integral(prosper::LogicOp::Set)},

		  {"PHYSICAL_DEVICE_TYPE_OTHER", umath::to_integral(prosper::PhysicalDeviceType::Other)}, {"PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU", umath::to_integral(prosper::PhysicalDeviceType::IntegratedGPU)},
		  {"PHYSICAL_DEVICE_TYPE_DISCRETE_GPU", umath::to_integral(prosper::PhysicalDeviceType::DiscreteGPU)}, {"PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU", umath::to_integral(prosper::PhysicalDeviceType::VirtualGPU)},
		  {"PHYSICAL_DEVICE_TYPE_CPU", umath::to_integral(prosper::PhysicalDeviceType::CPU)},

		  {"VERTEX_INPUT_RATE_VERTEX", umath::to_integral(prosper::VertexInputRate::Vertex)}, {"VERTEX_INPUT_RATE_INSTANCE", umath::to_integral(prosper::VertexInputRate::Instance)},

		  {"DYNAMIC_STATE_NONE", umath::to_integral(prosper::util::DynamicStateFlags::None)}, {"DYNAMIC_STATE_VIEWPORT_BIT", umath::to_integral(prosper::util::DynamicStateFlags::Viewport)}, {"DYNAMIC_STATE_SCISSOR_BIT", umath::to_integral(prosper::util::DynamicStateFlags::Scissor)},
		  {"DYNAMIC_STATE_LINE_WIDTH_BIT", umath::to_integral(prosper::util::DynamicStateFlags::LineWidth)}, {"DYNAMIC_STATE_DEPTH_BIAS_BIT", umath::to_integral(prosper::util::DynamicStateFlags::DepthBias)},
		  {"DYNAMIC_STATE_BLEND_CONSTANTS_BIT", umath::to_integral(prosper::util::DynamicStateFlags::BlendConstants)}, {"DYNAMIC_STATE_DEPTH_BOUNDS_BIT", umath::to_integral(prosper::util::DynamicStateFlags::DepthBounds)},
		  {"DYNAMIC_STATE_STENCIL_COMPARE_MASK_BIT", umath::to_integral(prosper::util::DynamicStateFlags::StencilCompareMask)}, {"DYNAMIC_STATE_STENCIL_WRITE_MASK_BIT", umath::to_integral(prosper::util::DynamicStateFlags::StencilWriteMask)},
		  {"DYNAMIC_STATE_STENCIL_REFERENCE_BIT", umath::to_integral(prosper::util::DynamicStateFlags::StencilReference)},
#if 0
		{"DYNAMIC_STATE_VIEWPORT_W_SCALING_NV_BIT",umath::to_integral(prosper::util::DynamicStateFlags::ViewportWScalingNV)},
		{"DYNAMIC_STATE_DISCARD_RECTANGLE_EXT_BIT",umath::to_integral(prosper::util::DynamicStateFlags::DiscardRectangleEXT)},
		{"DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT_BIT",umath::to_integral(prosper::util::DynamicStateFlags::SampleLocationsEXT)},
		{"DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV_BIT",umath::to_integral(prosper::util::DynamicStateFlags::ViewportShadingRatePaletteNV)},
		{"DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV_BIT",umath::to_integral(prosper::util::DynamicStateFlags::ViewportCoarseSampleOrderNV)},
		{"DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV_BIT",umath::to_integral(prosper::util::DynamicStateFlags::ExclusiveScissorNV)},
#endif

		  {"MEMORY_PROPERTY_DEVICE_LOCAL_BIT", umath::to_integral(prosper::MemoryPropertyFlags::DeviceLocalBit)}, {"MEMORY_PROPERTY_HOST_VISIBLE_BIT", umath::to_integral(prosper::MemoryPropertyFlags::HostVisibleBit)},
		  {"MEMORY_PROPERTY_HOST_COHERENT_BIT", umath::to_integral(prosper::MemoryPropertyFlags::HostCoherentBit)}, {"MEMORY_PROPERTY_HOST_CACHED_BIT", umath::to_integral(prosper::MemoryPropertyFlags::HostCachedBit)},
		  {"MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT", umath::to_integral(prosper::MemoryPropertyFlags::LazilyAllocatedBit)},

		  {"ACCESS_INDIRECT_COMMAND_READ_BIT", umath::to_integral(prosper::AccessFlags::IndirectCommandReadBit)}, {"ACCESS_INDEX_READ_BIT", umath::to_integral(prosper::AccessFlags::IndexReadBit)},
		  {"ACCESS_VERTEX_ATTRIBUTE_READ_BIT", umath::to_integral(prosper::AccessFlags::VertexAttributeReadBit)}, {"ACCESS_UNIFORM_READ_BIT", umath::to_integral(prosper::AccessFlags::UniformReadBit)},
		  {"ACCESS_INPUT_ATTACHMENT_READ_BIT", umath::to_integral(prosper::AccessFlags::InputAttachmentReadBit)}, {"ACCESS_SHADER_READ_BIT", umath::to_integral(prosper::AccessFlags::ShaderReadBit)}, {"ACCESS_SHADER_WRITE_BIT", umath::to_integral(prosper::AccessFlags::ShaderWriteBit)},
		  {"ACCESS_COLOR_ATTACHMENT_READ_BIT", umath::to_integral(prosper::AccessFlags::ColorAttachmentReadBit)}, {"ACCESS_COLOR_ATTACHMENT_WRITE_BIT", umath::to_integral(prosper::AccessFlags::ColorAttachmentWriteBit)},
		  {"ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT", umath::to_integral(prosper::AccessFlags::DepthStencilAttachmentReadBit)}, {"ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT", umath::to_integral(prosper::AccessFlags::DepthStencilAttachmentWriteBit)},
		  {"ACCESS_TRANSFER_READ_BIT", umath::to_integral(prosper::AccessFlags::TransferReadBit)}, {"ACCESS_TRANSFER_WRITE_BIT", umath::to_integral(prosper::AccessFlags::TransferWriteBit)}, {"ACCESS_HOST_READ_BIT", umath::to_integral(prosper::AccessFlags::HostReadBit)},
		  {"ACCESS_HOST_WRITE_BIT", umath::to_integral(prosper::AccessFlags::HostWriteBit)}, {"ACCESS_MEMORY_READ_BIT", umath::to_integral(prosper::AccessFlags::MemoryReadBit)}, {"ACCESS_MEMORY_WRITE_BIT", umath::to_integral(prosper::AccessFlags::MemoryWriteBit)},

		  {"BUFFER_USAGE_TRANSFER_SRC_BIT", umath::to_integral(prosper::BufferUsageFlags::TransferSrcBit)}, {"BUFFER_USAGE_TRANSFER_DST_BIT", umath::to_integral(prosper::BufferUsageFlags::TransferDstBit)},
		  {"BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT", umath::to_integral(prosper::BufferUsageFlags::UniformTexelBufferBit)}, {"BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT", umath::to_integral(prosper::BufferUsageFlags::StorageTexelBufferBit)},
		  {"BUFFER_USAGE_UNIFORM_BUFFER_BIT", umath::to_integral(prosper::BufferUsageFlags::UniformBufferBit)}, {"BUFFER_USAGE_STORAGE_BUFFER_BIT", umath::to_integral(prosper::BufferUsageFlags::StorageBufferBit)},
		  {"BUFFER_USAGE_INDEX_BUFFER_BIT", umath::to_integral(prosper::BufferUsageFlags::IndexBufferBit)}, {"BUFFER_USAGE_VERTEX_BUFFER_BIT", umath::to_integral(prosper::BufferUsageFlags::VertexBufferBit)},
		  {"BUFFER_USAGE_INDIRECT_BUFFER_BIT", umath::to_integral(prosper::BufferUsageFlags::IndirectBufferBit)},

		  {"SHADER_STAGE_VERTEX_BIT", umath::to_integral(prosper::ShaderStageFlags::VertexBit)}, {"SHADER_STAGE_TESSELLATION_CONTROL_BIT", umath::to_integral(prosper::ShaderStageFlags::TessellationControlBit)},
		  {"SHADER_STAGE_TESSELLATION_EVALUATION_BIT", umath::to_integral(prosper::ShaderStageFlags::TessellationEvaluationBit)}, {"SHADER_STAGE_GEOMETRY_BIT", umath::to_integral(prosper::ShaderStageFlags::GeometryBit)},
		  {"SHADER_STAGE_FRAGMENT_BIT", umath::to_integral(prosper::ShaderStageFlags::FragmentBit)}, {"SHADER_STAGE_COMPUTE_BIT", umath::to_integral(prosper::ShaderStageFlags::ComputeBit)}, {"SHADER_STAGE_ALL_GRAPHICS", umath::to_integral(prosper::ShaderStageFlags::AllGraphics)},
		  {"SHADER_STAGE_ALL", umath::to_integral(prosper::ShaderStageFlags::All)},

		  {"IMAGE_USAGE_TRANSFER_SRC_BIT", umath::to_integral(prosper::ImageUsageFlags::TransferSrcBit)}, {"IMAGE_USAGE_TRANSFER_DST_BIT", umath::to_integral(prosper::ImageUsageFlags::TransferDstBit)}, {"IMAGE_USAGE_SAMPLED_BIT", umath::to_integral(prosper::ImageUsageFlags::SampledBit)},
		  {"IMAGE_USAGE_STORAGE_BIT", umath::to_integral(prosper::ImageUsageFlags::StorageBit)}, {"IMAGE_USAGE_COLOR_ATTACHMENT_BIT", umath::to_integral(prosper::ImageUsageFlags::ColorAttachmentBit)},
		  {"IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT", umath::to_integral(prosper::ImageUsageFlags::DepthStencilAttachmentBit)}, {"IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT", umath::to_integral(prosper::ImageUsageFlags::TransientAttachmentBit)},
		  {"IMAGE_USAGE_INPUT_ATTACHMENT_BIT", umath::to_integral(prosper::ImageUsageFlags::InputAttachmentBit)},

		  {"IMAGE_CREATE_SPARSE_BINDING_BIT", umath::to_integral(prosper::ImageCreateFlags::SparseBindingBit)}, {"IMAGE_CREATE_SPARSE_RESIDENCY_BIT", umath::to_integral(prosper::ImageCreateFlags::SparseResidencyBit)},
		  {"IMAGE_CREATE_SPARSE_ALIASED_BIT", umath::to_integral(prosper::ImageCreateFlags::SparseAliasedBit)}, {"IMAGE_CREATE_MUTABLE_FORMAT_BIT", umath::to_integral(prosper::ImageCreateFlags::MutableFormatBit)},
		  {"IMAGE_CREATE_CUBE_COMPATIBLE_BIT", umath::to_integral(prosper::ImageCreateFlags::CubeCompatibleBit)},

		  {"PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT", umath::to_integral(prosper::PipelineCreateFlags::DisableOptimizationBit)}, {"PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT", umath::to_integral(prosper::PipelineCreateFlags::AllowDerivativesBit)},
		  {"PIPELINE_CREATE_DERIVATIVE_BIT", umath::to_integral(prosper::PipelineCreateFlags::DerivativeBit)},

		  {"COLOR_COMPONENT_R_BIT", umath::to_integral(prosper::ColorComponentFlags::RBit)}, {"COLOR_COMPONENT_G_BIT", umath::to_integral(prosper::ColorComponentFlags::GBit)}, {"COLOR_COMPONENT_B_BIT", umath::to_integral(prosper::ColorComponentFlags::BBit)},
		  {"COLOR_COMPONENT_A_BIT", umath::to_integral(prosper::ColorComponentFlags::ABit)},

		  {"QUERY_RESULT_64_BIT", umath::to_integral(prosper::QueryResultFlags::e64Bit)}, {"QUERY_RESULT_WAIT_BIT", umath::to_integral(prosper::QueryResultFlags::WaitBit)}, {"QUERY_RESULT_WITH_AVAILABILITY_BIT", umath::to_integral(prosper::QueryResultFlags::WithAvailabilityBit)},
		  {"QUERY_RESULT_PARTIAL_BIT", umath::to_integral(prosper::QueryResultFlags::PartialBit)},

		  {"QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT", umath::to_integral(prosper::QueryPipelineStatisticFlags::InputAssemblyVerticesBit)},
		  {"QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT", umath::to_integral(prosper::QueryPipelineStatisticFlags::InputAssemblyPrimitivesBit)},
		  {"QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT", umath::to_integral(prosper::QueryPipelineStatisticFlags::VertexShaderInvocationsBit)},
		  {"QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT", umath::to_integral(prosper::QueryPipelineStatisticFlags::GeometryShaderInvocationsBit)},
		  {"QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT", umath::to_integral(prosper::QueryPipelineStatisticFlags::GeometryShaderPrimitivesBit)},
		  {"QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT", umath::to_integral(prosper::QueryPipelineStatisticFlags::ClippingInvocationsBit)}, {"QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT", umath::to_integral(prosper::QueryPipelineStatisticFlags::ClippingPrimitivesBit)},
		  {"QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT", umath::to_integral(prosper::QueryPipelineStatisticFlags::FragmentShaderInvocationsBit)},
		  {"QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT", umath::to_integral(prosper::QueryPipelineStatisticFlags::TessellationControlShaderPatchesBit)},
		  {"QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT", umath::to_integral(prosper::QueryPipelineStatisticFlags::TessellationEvaluationShaderInvocationsBit)},
		  {"QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT", umath::to_integral(prosper::QueryPipelineStatisticFlags::ComputeShaderInvocationsBit)},

		  {"IMAGE_ASPECT_COLOR_BIT", umath::to_integral(prosper::ImageAspectFlags::ColorBit)}, {"IMAGE_ASPECT_DEPTH_BIT", umath::to_integral(prosper::ImageAspectFlags::DepthBit)}, {"IMAGE_ASPECT_STENCIL_BIT", umath::to_integral(prosper::ImageAspectFlags::StencilBit)},
		  {"IMAGE_ASPECT_METADATA_BIT", umath::to_integral(prosper::ImageAspectFlags::MetadataBit)},

		  {"PIPELINE_STAGE_TOP_OF_PIPE_BIT", umath::to_integral(prosper::PipelineStageFlags::TopOfPipeBit)}, {"PIPELINE_STAGE_DRAW_INDIRECT_BIT", umath::to_integral(prosper::PipelineStageFlags::DrawIndirectBit)},
		  {"PIPELINE_STAGE_VERTEX_INPUT_BIT", umath::to_integral(prosper::PipelineStageFlags::VertexInputBit)}, {"PIPELINE_STAGE_VERTEX_SHADER_BIT", umath::to_integral(prosper::PipelineStageFlags::VertexShaderBit)},
		  {"PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT", umath::to_integral(prosper::PipelineStageFlags::TessellationControlShaderBit)}, {"PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT", umath::to_integral(prosper::PipelineStageFlags::TessellationEvaluationShaderBit)},
		  {"PIPELINE_STAGE_GEOMETRY_SHADER_BIT", umath::to_integral(prosper::PipelineStageFlags::GeometryShaderBit)}, {"PIPELINE_STAGE_FRAGMENT_SHADER_BIT", umath::to_integral(prosper::PipelineStageFlags::FragmentShaderBit)},
		  {"PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT", umath::to_integral(prosper::PipelineStageFlags::EarlyFragmentTestsBit)}, {"PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT", umath::to_integral(prosper::PipelineStageFlags::LateFragmentTestsBit)},
		  {"PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT", umath::to_integral(prosper::PipelineStageFlags::ColorAttachmentOutputBit)}, {"PIPELINE_STAGE_COMPUTE_SHADER_BIT", umath::to_integral(prosper::PipelineStageFlags::ComputeShaderBit)},
		  {"PIPELINE_STAGE_TRANSFER_BIT", umath::to_integral(prosper::PipelineStageFlags::TransferBit)}, {"PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT", umath::to_integral(prosper::PipelineStageFlags::BottomOfPipeBit)},
		  {"PIPELINE_STAGE_HOST_BIT", umath::to_integral(prosper::PipelineStageFlags::HostBit)}, {"PIPELINE_STAGE_ALL_GRAPHICS", umath::to_integral(prosper::PipelineStageFlags::AllGraphics)}, {"PIPELINE_STAGE_ALL_COMMANDS", umath::to_integral(prosper::PipelineStageFlags::AllCommands)},

		  {"SAMPLE_COUNT_1_BIT", umath::to_integral(prosper::SampleCountFlags::e1Bit)}, {"SAMPLE_COUNT_2_BIT", umath::to_integral(prosper::SampleCountFlags::e2Bit)}, {"SAMPLE_COUNT_4_BIT", umath::to_integral(prosper::SampleCountFlags::e4Bit)},
		  {"SAMPLE_COUNT_8_BIT", umath::to_integral(prosper::SampleCountFlags::e8Bit)}, {"SAMPLE_COUNT_16_BIT", umath::to_integral(prosper::SampleCountFlags::e16Bit)}, {"SAMPLE_COUNT_32_BIT", umath::to_integral(prosper::SampleCountFlags::e32Bit)},
		  {"SAMPLE_COUNT_64_BIT", umath::to_integral(prosper::SampleCountFlags::e64Bit)},

		  {"STENCIL_FACE_FRONT_BIT", umath::to_integral(prosper::StencilFaceFlags::FrontBit)}, {"STENCIL_FACE_BACK_BIT", umath::to_integral(prosper::StencilFaceFlags::BackBit)}, {"STENCIL_FRONT_AND_BACK", umath::to_integral(prosper::StencilFaceFlags::FrontAndBack)},

		  {"PRESENT_MODE_IMMEDIATE_KHR", umath::to_integral(prosper::PresentModeKHR::Immediate)}, {"PRESENT_MODE_MAILBOX_KHR", umath::to_integral(prosper::PresentModeKHR::Mailbox)}, {"PRESENT_MODE_FIFO_KHR", umath::to_integral(prosper::PresentModeKHR::Fifo)},
		  {"PRESENT_MODE_FIFO_RELAXED_KHR", umath::to_integral(prosper::PresentModeKHR::FifoRelaxed)},

#if 0
		{"COMMAND_BUFFER_LEVEL_PRIMARY",umath::to_integral(prosper::CommandBufferLevel::Primary)},
		{"COMMAND_BUFFER_LEVEL_SECONDARY",umath::to_integral(prosper::CommandBufferLevel::Secondary)},

		{"PIPELINE_CACHE_HEADER_VERSION_ONE",umath::to_integral(prosper::PipelineCacheHeaderVersion::One)},

		{"INTERNAL_ALLOCATION_TYPE_EXECUTABLE",umath::to_integral(prosper::InternalAllocationType::Executable)},

		{"SYSTEM_ALLOCATION_SCOPE_COMMAND",umath::to_integral(prosper::SystemAllocationScope::Command)},
		{"SYSTEM_ALLOCATION_SCOPE_OBJECT",umath::to_integral(prosper::SystemAllocationScope::Object)},
		{"SYSTEM_ALLOCATION_SCOPE_CACHE",umath::to_integral(prosper::SystemAllocationScope::Cache)},
		{"SYSTEM_ALLOCATION_SCOPE_DEVICE",umath::to_integral(prosper::SystemAllocationScope::Device)},
		{"SYSTEM_ALLOCATION_SCOPE_INSTANCE",umath::to_integral(prosper::SystemAllocationScope::Instance)},

		{"SUBPASS_CONTENTS_INLINE",umath::to_integral(prosper::SubpassContents::Inline)},
		{"SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS",umath::to_integral(prosper::SubpassContents::SecondaryCommandBuffers)},

		{"QUEUE_GRAPHICS_BIT",umath::to_integral(prosper::QueueFlags::Graphics)},
		{"QUEUE_COMPUTE_BIT",umath::to_integral(prosper::QueueFlags::Compute)},
		{"QUEUE_TRANSFER_BIT",umath::to_integral(prosper::QueueFlags::Transfer)},
		{"QUEUE_SPARSE_BINDING_BIT",umath::to_integral(prosper::QueueFlags::SparseBinding)},

		{"MEMORY_HEAP_DEVICE_LOCAL_BIT",umath::to_integral(prosper::MemoryHeapFlags::DeviceLocal)},

		{"BUFFER_CREATE_SPARSE_BINDING_BIT",umath::to_integral(prosper::BufferCreateFlags::SparseBinding)},
		{"BUFFER_CREATE_SPARSE_RESIDENCY_BIT",umath::to_integral(prosper::BufferCreateFlags::SparseResidency)},
		{"BUFFER_CREATE_SPARSE_ALIASED_BIT",umath::to_integral(prosper::BufferCreateFlags::SparseAliased)},

		{"FENCE_CREATE_SIGNALED_BIT",umath::to_integral(prosper::FenceCreateFlags::Signaled)},

		{"FORMAT_FEATURE_SAMPLED_IMAGE_BIT",umath::to_integral(prosper::FormatFeatureFlags::SampledImage)},
		{"FORMAT_FEATURE_STORAGE_IMAGE_BIT",umath::to_integral(prosper::FormatFeatureFlags::StorageImage)},
		{"FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT",umath::to_integral(prosper::FormatFeatureFlags::StorageImageAtomic)},
		{"FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT",umath::to_integral(prosper::FormatFeatureFlags::UniformTexelBuffer)},
		{"FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT",umath::to_integral(prosper::FormatFeatureFlags::StorageTexelBuffer)},
		{"FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT",umath::to_integral(prosper::FormatFeatureFlags::StorageTexelBufferAtomic)},
		{"FORMAT_FEATURE_VERTEX_BUFFER_BIT",umath::to_integral(prosper::FormatFeatureFlags::VertexBuffer)},
		{"FORMAT_FEATURE_COLOR_ATTACHMENT_BIT",umath::to_integral(prosper::FormatFeatureFlags::ColorAttachment)},
		{"FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT",umath::to_integral(prosper::FormatFeatureFlags::ColorAttachmentBlend)},
		{"FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT",umath::to_integral(prosper::FormatFeatureFlags::DepthStencilAttachment)},
		{"FORMAT_FEATURE_BLIT_SRC_BIT",umath::to_integral(prosper::FormatFeatureFlags::BlitSrc)},
		{"FORMAT_FEATURE_BLIT_DST_BIT",umath::to_integral(prosper::FormatFeatureFlags::BlitDst)},
		{"FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT",umath::to_integral(prosper::FormatFeatureFlags::SampledImageFilterLinear)},

		{"QUERY_CONTROL_PRECISE_BIT",umath::to_integral(prosper::QueryControlFlags::Precise)},

		{"COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT",umath::to_integral(prosper::CommandBufferUsageFlags::OneTimeSubmit)},
		{"COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT",umath::to_integral(prosper::CommandBufferUsageFlags::RenderPassContinue)},
		{"COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT",umath::to_integral(prosper::CommandBufferUsageFlags::SimultaneousUse)},

		{"SPARSE_IMAGE_FORMAT_SINGLE_MIPTAIL_BIT",umath::to_integral(prosper::SparseImageFormatFlags::SingleMiptail)},
		{"SPARSE_IMAGE_FORMAT_ALIGNED_MIP_SIZE_BIT",umath::to_integral(prosper::SparseImageFormatFlags::AlignedMipSize)},
		{"SPARSE_IMAGE_FORMAT_NONSTANDARD_BLOCK_SIZE_BIT",umath::to_integral(prosper::SparseImageFormatFlags::NonstandardBlockSize)},

		{"SPARSE_MEMORY_BIND_METADATA_BIT",umath::to_integral(prosper::SparseMemoryBindFlags::Metadata)},

		{"COMMAND_POOL_CREATE_TRANSIENT_BIT",umath::to_integral(prosper::CommandPoolCreateFlags::Transient)},
		{"COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT",umath::to_integral(prosper::CommandPoolCreateFlags::ResetCommandBuffer)},

		{"COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT",umath::to_integral(prosper::CommandPoolResetFlags::ReleaseResources)},

		{"COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT",umath::to_integral(prosper::CommandBufferResetFlags::ReleaseResources)},

		{"ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT",umath::to_integral(prosper::AttachmentDescriptionFlags::MayAlias)},

		{"DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT",umath::to_integral(prosper::DescriptorPoolCreateFlags::FreeDescriptorSet)},

		{"DEPENDENCY_BY_REGION_BIT",umath::to_integral(prosper::DependencyFlags::ByRegion)},

		{"COLORSPACE_SRGB_NONLINEAR_KHR",umath::to_integral(prosper::ColorSpaceKHR::SrgbNonlinear)},

		{"DISPLAY_PLANE_ALPHA_OPAQUE_BIT_KHR",umath::to_integral(prosper::DisplayPlaneAlphaFlagsKHR::Opaque)},
		{"DISPLAY_PLANE_ALPHA_GLOBAL_BIT_KHR",umath::to_integral(prosper::DisplayPlaneAlphaFlagsKHR::Global)},
		{"DISPLAY_PLANE_ALPHA_PER_PIXEL_BIT_KHR",umath::to_integral(prosper::DisplayPlaneAlphaFlagsKHR::PerPixel)},
		{"DISPLAY_PLANE_ALPHA_PER_PIXEL_PREMULTIPLIED_BIT_KHR",umath::to_integral(prosper::DisplayPlaneAlphaFlagsKHR::PerPixelPremultiplied)},

		{"COMPOSITE_ALPHA_OPAQUE_BIT_KHR",umath::to_integral(prosper::CompositeAlphaFlagsKHR::Opaque)},
		{"COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR",umath::to_integral(prosper::CompositeAlphaFlagsKHR::PreMultiplied)},
		{"COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR",umath::to_integral(prosper::CompositeAlphaFlagsKHR::PostMultiplied)},
		{"COMPOSITE_ALPHA_INHERIT_BIT_KHR",umath::to_integral(prosper::CompositeAlphaFlagsKHR::Inherit)},

		{"SURFACE_TRANSFORM_IDENTITY_BIT_KHR",umath::to_integral(prosper::SurfaceTransformFlagsKHR::Identity)},
		{"SURFACE_TRANSFORM_ROTATE_90_BIT_KHR",umath::to_integral(prosper::SurfaceTransformFlagsKHR::Rotate90)},
		{"SURFACE_TRANSFORM_ROTATE_180_BIT_KHR",umath::to_integral(prosper::SurfaceTransformFlagsKHR::Rotate180)},
		{"SURFACE_TRANSFORM_ROTATE_270_BIT_KHR",umath::to_integral(prosper::SurfaceTransformFlagsKHR::Rotate270)},
		{"SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR",umath::to_integral(prosper::SurfaceTransformFlagsKHR::HorizontalMirror)},
		{"SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR",umath::to_integral(prosper::SurfaceTransformFlagsKHR::HorizontalMirrorRotate90)},
		{"SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR",umath::to_integral(prosper::SurfaceTransformFlagsKHR::HorizontalMirrorRotate180)},
		{"SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR",umath::to_integral(prosper::SurfaceTransformFlagsKHR::HorizontalMirrorRotate270)},
		{"SURFACE_TRANSFORM_INHERIT_BIT_KHR",umath::to_integral(prosper::SurfaceTransformFlagsKHR::Inherit)},
#endif

		  {"DEBUG_REPORT_INFORMATION_BIT_EXT", umath::to_integral(prosper::DebugReportFlags::InformationBit)}, {"DEBUG_REPORT_WARNING_BIT_EXT", umath::to_integral(prosper::DebugReportFlags::WarningBit)},
		  {"DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT", umath::to_integral(prosper::DebugReportFlags::PerformanceWarningBit)}, {"DEBUG_REPORT_ERROR_BIT_EXT", umath::to_integral(prosper::DebugReportFlags::ErrorBit)},
		  {"DEBUG_REPORT_DEBUG_BIT_EXT", umath::to_integral(prosper::DebugReportFlags::DebugBit)},

		  {"DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT", umath::to_integral(prosper::DebugReportObjectTypeEXT::Unknown)}, {"DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT", umath::to_integral(prosper::DebugReportObjectTypeEXT::Instance)},
		  {"DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT", umath::to_integral(prosper::DebugReportObjectTypeEXT::PhysicalDevice)}, {"DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT", umath::to_integral(prosper::DebugReportObjectTypeEXT::Device)},
		  {"DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT", umath::to_integral(prosper::DebugReportObjectTypeEXT::Queue)}, {"DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT", umath::to_integral(prosper::DebugReportObjectTypeEXT::Semaphore)},
		  {"DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT", umath::to_integral(prosper::DebugReportObjectTypeEXT::CommandBuffer)}, {"DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT", umath::to_integral(prosper::DebugReportObjectTypeEXT::Fence)},
		  {"DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT", umath::to_integral(prosper::DebugReportObjectTypeEXT::DeviceMemory)}, {"DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT", umath::to_integral(prosper::DebugReportObjectTypeEXT::Buffer)},
		  {"DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT", umath::to_integral(prosper::DebugReportObjectTypeEXT::Image)}, {"DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT", umath::to_integral(prosper::DebugReportObjectTypeEXT::Event)},
		  {"DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT", umath::to_integral(prosper::DebugReportObjectTypeEXT::QueryPool)}, {"DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT", umath::to_integral(prosper::DebugReportObjectTypeEXT::BufferView)},
		  {"DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT", umath::to_integral(prosper::DebugReportObjectTypeEXT::ImageView)}, {"DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT", umath::to_integral(prosper::DebugReportObjectTypeEXT::ShaderModule)},
		  {"DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT", umath::to_integral(prosper::DebugReportObjectTypeEXT::PipelineCache)}, {"DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT", umath::to_integral(prosper::DebugReportObjectTypeEXT::PipelineLayout)},
		  {"DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT", umath::to_integral(prosper::DebugReportObjectTypeEXT::RenderPass)}, {"DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT", umath::to_integral(prosper::DebugReportObjectTypeEXT::Pipeline)},
		  {"DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT", umath::to_integral(prosper::DebugReportObjectTypeEXT::DescriptorSetLayout)}, {"DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT", umath::to_integral(prosper::DebugReportObjectTypeEXT::Sampler)},
		  {"DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT", umath::to_integral(prosper::DebugReportObjectTypeEXT::DescriptorPool)}, {"DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT", umath::to_integral(prosper::DebugReportObjectTypeEXT::DescriptorSet)},
		  {"DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT", umath::to_integral(prosper::DebugReportObjectTypeEXT::Framebuffer)}, {"DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT", umath::to_integral(prosper::DebugReportObjectTypeEXT::CommandPool)},
		  {"DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT", umath::to_integral(prosper::DebugReportObjectTypeEXT::SurfaceKHR)}, {"DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT", umath::to_integral(prosper::DebugReportObjectTypeEXT::SwapchainKHR)},

		  {"MEMORY_FEATURE_DEVICE_LOCAL_BIT", umath::to_integral(prosper::MemoryFeatureFlags::DeviceLocal)}, {"MEMORY_FEATURE_HOST_CACHED_BIT", umath::to_integral(prosper::MemoryFeatureFlags::HostCached)},
		  {"MEMORY_FEATURE_HOST_COHERENT_BIT", umath::to_integral(prosper::MemoryFeatureFlags::HostCoherent)}, {"MEMORY_FEATURE_LAZILY_ALLOCATED_BIT", umath::to_integral(prosper::MemoryFeatureFlags::LazilyAllocated)},
		  {"MEMORY_FEATURE_HOST_ACCESSABLE_BIT", umath::to_integral(prosper::MemoryFeatureFlags::HostAccessable)}, {"MEMORY_FEATURE_GPU_BULK_BIT", umath::to_integral(prosper::MemoryFeatureFlags::GPUBulk)},
		  {"MEMORY_FEATURE_CPU_TO_GPU", umath::to_integral(prosper::MemoryFeatureFlags::CPUToGPU)}, {"MEMORY_FEATURE_GPU_TO_CPU", umath::to_integral(prosper::MemoryFeatureFlags::GPUToCPU)},

		  {"QUEUE_FAMILY_GRAPHICS_BIT", umath::to_integral(prosper::QueueFamilyFlags::GraphicsBit)}, {"QUEUE_FAMILY_COMPUTE_BIT", umath::to_integral(prosper::QueueFamilyFlags::ComputeBit)},
		{
			"QUEUE_FAMILY_DMA_BIT", umath::to_integral(prosper::QueueFamilyFlags::DMABit)
		}
	});

#if 0
	auto defShaderStatisticsInfoAMD = luabind::class_<prosper::ShaderStatisticsInfoAMD>("ShaderStatisticsInfoAMD");
	defShaderStatisticsInfoAMD.def_readwrite("shaderStageMask",reinterpret_cast<uint32_t prosper::ShaderStatisticsInfoAMD::*>(&prosper::ShaderStatisticsInfoAMD::shaderStageMask));
	defShaderStatisticsInfoAMD.def_readwrite("resourceUsage",reinterpret_cast<uint32_t prosper::ShaderStatisticsInfoAMD::*>(&prosper::ShaderStatisticsInfoAMD::resourceUsage));
	defShaderStatisticsInfoAMD.def_readwrite("numPhysicalVgprs",&prosper::ShaderStatisticsInfoAMD::numPhysicalVgprs);
	defShaderStatisticsInfoAMD.def_readwrite("numPhysicalSgprs",&prosper::ShaderStatisticsInfoAMD::numPhysicalSgprs);
	defShaderStatisticsInfoAMD.def_readwrite("numAvailableVgprs",&prosper::ShaderStatisticsInfoAMD::numAvailableVgprs);
	defShaderStatisticsInfoAMD.def_readwrite("numAvailableSgprs",&prosper::ShaderStatisticsInfoAMD::numAvailableSgprs);
	defShaderStatisticsInfoAMD.property("computeWorkGroupSize",static_cast<Vector3i(*)(prosper::ShaderStatisticsInfoAMD&)>([](prosper::ShaderStatisticsInfoAMD &stats) -> Vector3i {
		return {stats.computeWorkGroupSize[0],stats.computeWorkGroupSize[1],stats.computeWorkGroupSize[2]};
	}),static_cast<void(*)(prosper::ShaderStatisticsInfoAMD&,const Vector3i&)>([](prosper::ShaderStatisticsInfoAMD &stats,const Vector3i &v) {
		stats.computeWorkGroupSize[0] = v.x;
		stats.computeWorkGroupSize[1] = v.y;
		stats.computeWorkGroupSize[2] = v.z;
	}));
	prosperMod[defShaderStatisticsInfoAMD];

	auto defShaderResourceUsageAMD = luabind::class_<prosper::ShaderResourceUsageAMD>("ShaderResourceUsageAMD");
	defShaderResourceUsageAMD.def_readwrite("numUsedVgprs",&prosper::ShaderResourceUsageAMD::numUsedVgprs);
	defShaderResourceUsageAMD.def_readwrite("numUsedSgprs",&prosper::ShaderResourceUsageAMD::numUsedSgprs);
	defShaderResourceUsageAMD.def_readwrite("ldsSizePerLocalWorkGroup",&prosper::ShaderResourceUsageAMD::ldsSizePerLocalWorkGroup);
	defShaderResourceUsageAMD.def_readwrite("ldsUsageSizeInBytes",&prosper::ShaderResourceUsageAMD::ldsUsageSizeInBytes);
	defShaderResourceUsageAMD.def_readwrite("scratchMemUsageInBytes",&prosper::ShaderResourceUsageAMD::scratchMemUsageInBytes);
	prosperMod[defShaderResourceUsageAMD];

	auto defPipelineColorBlendAttachmentState = luabind::class_<prosper::PipelineColorBlendAttachmentState>("PipelineColorBlendAttachmentState");
	defPipelineColorBlendAttachmentState.def(luabind::constructor<>());
	defPipelineColorBlendAttachmentState.def_readwrite("blendEnable",reinterpret_cast<bool prosper::PipelineColorBlendAttachmentState::*>(&prosper::PipelineColorBlendAttachmentState::blendEnable));
	defPipelineColorBlendAttachmentState.def_readwrite("srcColorBlendFactor",reinterpret_cast<uint32_t prosper::PipelineColorBlendAttachmentState::*>(&prosper::PipelineColorBlendAttachmentState::srcColorBlendFactor));
	defPipelineColorBlendAttachmentState.def_readwrite("dstColorBlendFactor",reinterpret_cast<uint32_t prosper::PipelineColorBlendAttachmentState::*>(&prosper::PipelineColorBlendAttachmentState::dstColorBlendFactor));
	defPipelineColorBlendAttachmentState.def_readwrite("colorBlendOp",reinterpret_cast<uint32_t prosper::PipelineColorBlendAttachmentState::*>(&prosper::PipelineColorBlendAttachmentState::colorBlendOp));
	defPipelineColorBlendAttachmentState.def_readwrite("srcAlphaBlendFactor",reinterpret_cast<uint32_t prosper::PipelineColorBlendAttachmentState::*>(&prosper::PipelineColorBlendAttachmentState::srcAlphaBlendFactor));
	defPipelineColorBlendAttachmentState.def_readwrite("dstAlphaBlendFactor",reinterpret_cast<uint32_t prosper::PipelineColorBlendAttachmentState::*>(&prosper::PipelineColorBlendAttachmentState::dstAlphaBlendFactor));
	defPipelineColorBlendAttachmentState.def_readwrite("alphaBlendOp",reinterpret_cast<uint32_t prosper::PipelineColorBlendAttachmentState::*>(&prosper::PipelineColorBlendAttachmentState::alphaBlendOp));
	defPipelineColorBlendAttachmentState.def_readwrite("colorWriteMask",reinterpret_cast<uint32_t prosper::PipelineColorBlendAttachmentState::*>(&prosper::PipelineColorBlendAttachmentState::colorWriteMask));
	prosperMod[defPipelineColorBlendAttachmentState];
#endif

	auto defBufferCreateInfo = luabind::class_<prosper::util::BufferCreateInfo>("BufferCreateInfo");
	defBufferCreateInfo.def(luabind::constructor<>());
	defBufferCreateInfo.def_readwrite("size", &prosper::util::BufferCreateInfo::size);
	defBufferCreateInfo.def_readwrite("queueFamilyMask", reinterpret_cast<uint32_t prosper::util::BufferCreateInfo::*>(&prosper::util::BufferCreateInfo::queueFamilyMask));
	defBufferCreateInfo.def_readwrite("flags", reinterpret_cast<uint32_t prosper::util::BufferCreateInfo::*>(&prosper::util::BufferCreateInfo::flags));
	defBufferCreateInfo.def_readwrite("usageFlags", reinterpret_cast<uint32_t prosper::util::BufferCreateInfo::*>(&prosper::util::BufferCreateInfo::usageFlags));
	defBufferCreateInfo.def_readwrite("memoryFeatures", reinterpret_cast<uint32_t prosper::util::BufferCreateInfo::*>(&prosper::util::BufferCreateInfo::memoryFeatures));
	defBufferCreateInfo.add_static_constant("FLAG_NONE", umath::to_integral(prosper::util::BufferCreateInfo::Flags::None));
	defBufferCreateInfo.add_static_constant("FLAG_CONCURRENT_SHARING_BIT", umath::to_integral(prosper::util::BufferCreateInfo::Flags::ConcurrentSharing));
	defBufferCreateInfo.add_static_constant("FLAG_DONT_ALLOCATE_MEMORY_BIT", umath::to_integral(prosper::util::BufferCreateInfo::Flags::DontAllocateMemory));
	defBufferCreateInfo.add_static_constant("FLAG_SPARSE_BIT", umath::to_integral(prosper::util::BufferCreateInfo::Flags::Sparse));
	defBufferCreateInfo.add_static_constant("FLAG_SPARSE_ALIASED_RESIDENCY_BIT", umath::to_integral(prosper::util::BufferCreateInfo::Flags::SparseAliasedResidency));
	prosperMod[defBufferCreateInfo];

	auto defRenderTargetCreateInfo = luabind::class_<prosper::util::RenderTargetCreateInfo>("RenderTargetCreateInfo");
	defRenderTargetCreateInfo.def(luabind::constructor<>());
	defRenderTargetCreateInfo.def_readwrite("useLayerFramebuffers", &prosper::util::RenderTargetCreateInfo::useLayerFramebuffers);
	prosperMod[defRenderTargetCreateInfo];

	auto defSamplerCreateInfo = luabind::class_<prosper::util::SamplerCreateInfo>("SamplerCreateInfo");
	defSamplerCreateInfo.def(luabind::constructor<>());
	defSamplerCreateInfo.def_readwrite("minFilter", reinterpret_cast<uint32_t prosper::util::SamplerCreateInfo::*>(&prosper::util::SamplerCreateInfo::minFilter));
	defSamplerCreateInfo.def_readwrite("magFilter", reinterpret_cast<uint32_t prosper::util::SamplerCreateInfo::*>(&prosper::util::SamplerCreateInfo::magFilter));
	defSamplerCreateInfo.def_readwrite("mipmapMode", reinterpret_cast<uint32_t prosper::util::SamplerCreateInfo::*>(&prosper::util::SamplerCreateInfo::mipmapMode));
	defSamplerCreateInfo.def_readwrite("addressModeU", reinterpret_cast<uint32_t prosper::util::SamplerCreateInfo::*>(&prosper::util::SamplerCreateInfo::addressModeU));
	defSamplerCreateInfo.def_readwrite("addressModeV", reinterpret_cast<uint32_t prosper::util::SamplerCreateInfo::*>(&prosper::util::SamplerCreateInfo::addressModeV));
	defSamplerCreateInfo.def_readwrite("addressModeW", reinterpret_cast<uint32_t prosper::util::SamplerCreateInfo::*>(&prosper::util::SamplerCreateInfo::addressModeW));
	defSamplerCreateInfo.def_readwrite("mipLodBias", &prosper::util::SamplerCreateInfo::mipLodBias);
	defSamplerCreateInfo.def_readwrite("maxAnisotropy", &prosper::util::SamplerCreateInfo::maxAnisotropy);
	defSamplerCreateInfo.def_readwrite("compareEnable", &prosper::util::SamplerCreateInfo::compareEnable);
	defSamplerCreateInfo.def_readwrite("compareOp", reinterpret_cast<uint32_t prosper::util::SamplerCreateInfo::*>(&prosper::util::SamplerCreateInfo::compareOp));
	defSamplerCreateInfo.def_readwrite("minLod", &prosper::util::SamplerCreateInfo::minLod);
	defSamplerCreateInfo.def_readwrite("maxLod", &prosper::util::SamplerCreateInfo::maxLod);
	defSamplerCreateInfo.def_readwrite("borderColor", reinterpret_cast<uint32_t prosper::util::SamplerCreateInfo::*>(&prosper::util::SamplerCreateInfo::borderColor));
	// defSamplerCreateInfo.def_readwrite("useUnnormalizedCoordinates",&prosper::util::SamplerCreateInfo::useUnnormalizedCoordinates);
	prosperMod[defSamplerCreateInfo];

	auto defTextureCreateInfo = luabind::class_<prosper::util::TextureCreateInfo>("TextureCreateInfo");
	defTextureCreateInfo.def(luabind::constructor<>());
	defTextureCreateInfo.def_readwrite("flags", reinterpret_cast<uint32_t prosper::util::TextureCreateInfo::*>(&prosper::util::TextureCreateInfo::flags));
	defTextureCreateInfo.def_readwrite("sampler", &prosper::util::TextureCreateInfo::sampler);
	defTextureCreateInfo.def_readwrite("imageView", &prosper::util::TextureCreateInfo::imageView);
	defTextureCreateInfo.add_static_constant("FLAG_NONE", umath::to_integral(prosper::util::TextureCreateInfo::Flags::None));
	defTextureCreateInfo.add_static_constant("FLAG_RESOLVABLE", umath::to_integral(prosper::util::TextureCreateInfo::Flags::Resolvable));
	defTextureCreateInfo.add_static_constant("FLAG_CREATE_IMAGE_VIEW_FOR_EACH_LAYER", umath::to_integral(prosper::util::TextureCreateInfo::Flags::CreateImageViewForEachLayer));
	prosperMod[defTextureCreateInfo];

	auto defImageCreateInfo = luabind::class_<prosper::util::ImageCreateInfo>("ImageCreateInfo");
	defImageCreateInfo.def(luabind::constructor<>());
	defImageCreateInfo.def_readwrite("type", reinterpret_cast<uint32_t prosper::util::ImageCreateInfo::*>(&prosper::util::ImageCreateInfo::type));
	defImageCreateInfo.def_readwrite("width", &prosper::util::ImageCreateInfo::width);
	defImageCreateInfo.def_readwrite("height", &prosper::util::ImageCreateInfo::height);
	defImageCreateInfo.def_readwrite("format", reinterpret_cast<uint32_t prosper::util::ImageCreateInfo::*>(&prosper::util::ImageCreateInfo::format));
	defImageCreateInfo.def_readwrite("layers", &prosper::util::ImageCreateInfo::layers);
	defImageCreateInfo.def_readwrite("usageFlags", reinterpret_cast<uint32_t prosper::util::ImageCreateInfo::*>(&prosper::util::ImageCreateInfo::usage));
	defImageCreateInfo.def_readwrite("samples", reinterpret_cast<uint32_t prosper::util::ImageCreateInfo::*>(&prosper::util::ImageCreateInfo::samples));
	defImageCreateInfo.def_readwrite("tiling", reinterpret_cast<uint32_t prosper::util::ImageCreateInfo::*>(&prosper::util::ImageCreateInfo::tiling));
	defImageCreateInfo.def_readwrite("postCreateLayout", reinterpret_cast<uint32_t prosper::util::ImageCreateInfo::*>(&prosper::util::ImageCreateInfo::postCreateLayout));
	defImageCreateInfo.def_readwrite("flags", reinterpret_cast<uint32_t prosper::util::ImageCreateInfo::*>(&prosper::util::ImageCreateInfo::flags));
	defImageCreateInfo.def_readwrite("queueFamilyMask", reinterpret_cast<uint32_t prosper::util::ImageCreateInfo::*>(&prosper::util::ImageCreateInfo::queueFamilyMask));
	defImageCreateInfo.def_readwrite("memoryFeatures", reinterpret_cast<uint32_t prosper::util::ImageCreateInfo::*>(&prosper::util::ImageCreateInfo::memoryFeatures));
	defImageCreateInfo.add_static_constant("FLAG_NONE", umath::to_integral(prosper::util::ImageCreateInfo::Flags::None));
	defImageCreateInfo.add_static_constant("FLAG_CUBEMAP_BIT", umath::to_integral(prosper::util::ImageCreateInfo::Flags::Cubemap));
	defImageCreateInfo.add_static_constant("FLAG_CONCURRENT_SHARING_BIT", umath::to_integral(prosper::util::ImageCreateInfo::Flags::ConcurrentSharing));
	defImageCreateInfo.add_static_constant("FLAG_FULL_MIPMAP_CHAIN_BIT", umath::to_integral(prosper::util::ImageCreateInfo::Flags::FullMipmapChain));
	defImageCreateInfo.add_static_constant("FLAG_SPARSE_BIT", umath::to_integral(prosper::util::ImageCreateInfo::Flags::Sparse));
	defImageCreateInfo.add_static_constant("FLAG_SPARSE_ALIASED_RESIDENCY_BIT", umath::to_integral(prosper::util::ImageCreateInfo::Flags::SparseAliasedResidency));

	defImageCreateInfo.add_static_constant("FLAG_ALLOCATE_DISCRETE_MEMORY_BIT", umath::to_integral(prosper::util::ImageCreateInfo::Flags::AllocateDiscreteMemory));
	defImageCreateInfo.add_static_constant("FLAG_DONT_ALLOCATE_MEMORY_BIT", umath::to_integral(prosper::util::ImageCreateInfo::Flags::DontAllocateMemory));
	defImageCreateInfo.add_static_constant("FLAG_SRGB_BIT", umath::to_integral(prosper::util::ImageCreateInfo::Flags::Srgb));
	defImageCreateInfo.add_static_constant("FLAG_NORMAL_MAP_BIT", umath::to_integral(prosper::util::ImageCreateInfo::Flags::NormalMap));
	prosperMod[defImageCreateInfo];

	auto defImageViewCreateInfo = luabind::class_<prosper::util::ImageViewCreateInfo>("ImageViewCreateInfo");
	defImageViewCreateInfo.def(luabind::constructor<>());
	defImageViewCreateInfo.def_readwrite("baseLayer", &prosper::util::ImageViewCreateInfo::baseLayer);
	defImageViewCreateInfo.def_readwrite("levelCount", &prosper::util::ImageViewCreateInfo::levelCount);
	defImageViewCreateInfo.def_readwrite("baseMipmap", &prosper::util::ImageViewCreateInfo::baseMipmap);
	defImageViewCreateInfo.def_readwrite("mipmapLevels", &prosper::util::ImageViewCreateInfo::mipmapLevels);
	defImageViewCreateInfo.def_readwrite("format", reinterpret_cast<uint32_t prosper::util::ImageViewCreateInfo::*>(&prosper::util::ImageViewCreateInfo::format));
	defImageViewCreateInfo.def_readwrite("swizzleRed", reinterpret_cast<uint32_t prosper::util::ImageViewCreateInfo::*>(&prosper::util::ImageViewCreateInfo::swizzleRed));
	defImageViewCreateInfo.def_readwrite("swizzleGreen", reinterpret_cast<uint32_t prosper::util::ImageViewCreateInfo::*>(&prosper::util::ImageViewCreateInfo::swizzleGreen));
	defImageViewCreateInfo.def_readwrite("swizzleBlue", reinterpret_cast<uint32_t prosper::util::ImageViewCreateInfo::*>(&prosper::util::ImageViewCreateInfo::swizzleBlue));
	defImageViewCreateInfo.def_readwrite("swizzleAlpha", reinterpret_cast<uint32_t prosper::util::ImageViewCreateInfo::*>(&prosper::util::ImageViewCreateInfo::swizzleAlpha));
	prosperMod[defImageViewCreateInfo];

	auto defRenderPassCreateInfo = luabind::class_<prosper::util::RenderPassCreateInfo>("RenderPassCreateInfo");
	defRenderPassCreateInfo.def(luabind::constructor<>());
	defRenderPassCreateInfo.def("AddAttachment",
	  static_cast<uint32_t (*)(lua_State *, prosper::util::RenderPassCreateInfo &, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t)>(
	    [](lua_State *l, prosper::util::RenderPassCreateInfo &createInfo, uint32_t format, uint32_t initialLayout, uint32_t finalLayout, uint32_t loadOp, uint32_t storeOp, uint32_t sampleCount) -> uint32_t {
		    createInfo.attachments.push_back({static_cast<prosper::Format>(format), static_cast<prosper::ImageLayout>(initialLayout), static_cast<prosper::AttachmentLoadOp>(loadOp), static_cast<prosper::AttachmentStoreOp>(storeOp), static_cast<prosper::SampleCountFlags>(sampleCount),
		      static_cast<prosper::ImageLayout>(finalLayout)});
		    return createInfo.attachments.size() - 1ull;
	    }));
	defRenderPassCreateInfo.def("AddAttachment",
	  static_cast<uint32_t (*)(lua_State *, prosper::util::RenderPassCreateInfo &, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t)>(
	    [](lua_State *l, prosper::util::RenderPassCreateInfo &createInfo, uint32_t format, uint32_t initialLayout, uint32_t finalLayout, uint32_t loadOp, uint32_t storeOp) -> uint32_t {
		    createInfo.attachments.push_back(
		      {static_cast<prosper::Format>(format), static_cast<prosper::ImageLayout>(initialLayout), static_cast<prosper::AttachmentLoadOp>(loadOp), static_cast<prosper::AttachmentStoreOp>(storeOp), prosper::SampleCountFlags::e1Bit, static_cast<prosper::ImageLayout>(finalLayout)});
		    return createInfo.attachments.size() - 1ull;
	    }));
	defRenderPassCreateInfo.def("AddSubPass", static_cast<uint32_t (*)(lua_State *, prosper::util::RenderPassCreateInfo &)>([](lua_State *l, prosper::util::RenderPassCreateInfo &createInfo) -> uint32_t {
		createInfo.subPasses.push_back({});
		return createInfo.subPasses.size() - 1ull;
	}));
	defRenderPassCreateInfo.def("AddSubPassColorAttachment", static_cast<void (*)(lua_State *, prosper::util::RenderPassCreateInfo &, uint32_t, uint32_t)>([](lua_State *l, prosper::util::RenderPassCreateInfo &createInfo, uint32_t subPassId, uint32_t colorAttId) {
		if(subPassId >= createInfo.subPasses.size() || colorAttId >= createInfo.attachments.size())
			return;
		createInfo.subPasses.at(subPassId).colorAttachments.push_back(colorAttId);
	}));
	defRenderPassCreateInfo.def("SetSubPassDepthStencilAttachmentEnabled", static_cast<void (*)(lua_State *, prosper::util::RenderPassCreateInfo &, uint32_t, bool)>([](lua_State *l, prosper::util::RenderPassCreateInfo &createInfo, uint32_t subPassId, bool bEnabled) {
		if(subPassId >= createInfo.subPasses.size())
			return;
		createInfo.subPasses.at(subPassId).useDepthStencilAttachment = bEnabled;
	}));
	defRenderPassCreateInfo.def("AddSubPassDependency",
	  static_cast<void (*)(lua_State *, prosper::util::RenderPassCreateInfo &, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t)>(
	    [](lua_State *l, prosper::util::RenderPassCreateInfo &createInfo, uint32_t subPassId, uint32_t sourceSubPassId, uint32_t destinationSubPassId, uint32_t sourceStageMask, uint32_t destinationStageMask, uint32_t sourceAccessMask, uint32_t destinationAccessMask) {
		    if(subPassId >= createInfo.subPasses.size())
			    return;
		    createInfo.subPasses.at(subPassId).dependencies.push_back(
		      {sourceSubPassId, destinationSubPassId, static_cast<prosper::PipelineStageFlags>(sourceStageMask), static_cast<prosper::PipelineStageFlags>(destinationStageMask), static_cast<prosper::AccessFlags>(sourceAccessMask), static_cast<prosper::AccessFlags>(destinationAccessMask)});
	    }));
	prosperMod[defRenderPassCreateInfo];

	auto defImageSubresourceLayers = luabind::class_<prosper::util::ImageSubresourceLayers>("ImageSubresourceLayers");
	defImageSubresourceLayers.def(luabind::constructor<>());
	defImageSubresourceLayers.def_readwrite("aspectMask", reinterpret_cast<std::underlying_type_t<decltype(prosper::util::ImageSubresourceLayers::aspectMask)> prosper::util::ImageSubresourceLayers::*>(&prosper::util::ImageSubresourceLayers::aspectMask));
	defImageSubresourceLayers.def_readwrite("mipLevel", &prosper::util::ImageSubresourceLayers::mipLevel);
	defImageSubresourceLayers.def_readwrite("baseArrayLayer", &prosper::util::ImageSubresourceLayers::baseArrayLayer);
	defImageSubresourceLayers.def_readwrite("layerCount", &prosper::util::ImageSubresourceLayers::layerCount);
	prosperMod[defImageSubresourceLayers];

	auto defBlitInfo = luabind::class_<prosper::util::BlitInfo>("BlitInfo");
	defBlitInfo.def(luabind::constructor<>());
	defBlitInfo.def_readwrite("srcSubresourceLayer", &prosper::util::BlitInfo::srcSubresourceLayer);
	defBlitInfo.def_readwrite("dstSubresourceLayer", &prosper::util::BlitInfo::dstSubresourceLayer);
	defBlitInfo.property(
	  "offsetSrc",
	  +[](const prosper::util::BlitInfo &blitInfo) -> Vector2i {
		  return {blitInfo.offsetSrc[0], blitInfo.offsetSrc[1]};
	  },
	  +[](lua_State *l, prosper::util::BlitInfo &blitInfo, const Vector2i &offsetSrc) {
		  blitInfo.offsetSrc = {offsetSrc.x, offsetSrc.y};
	  });
	defBlitInfo.property(
	  "offsetDst",
	  +[](const prosper::util::BlitInfo &blitInfo) -> Vector2i {
		  return {blitInfo.offsetDst[0], blitInfo.offsetDst[1]};
	  },
	  +[](lua_State *l, prosper::util::BlitInfo &blitInfo, const Vector2i &offsetDst) {
		  blitInfo.offsetDst = {offsetDst.x, offsetDst.y};
	  });
	defBlitInfo.property(
	  "extentsSrc",
	  +[](const prosper::util::BlitInfo &blitInfo) -> std::optional<Vector2i> {
		  if(!blitInfo.extentsSrc.has_value())
			  return {};
		  return Vector2i {blitInfo.extentsSrc->width, blitInfo.extentsSrc->height};
	  },
	  +[](lua_State *l, prosper::util::BlitInfo &blitInfo, const luabind::object &ext) {
		  if(luabind::type(ext) == LUA_TNIL) {
			  blitInfo.extentsSrc = {};
			  return;
		  }
		  auto v = luabind::object_cast<Vector2i>(ext);
		  blitInfo.extentsSrc = prosper::Extent2D {static_cast<uint32_t>(v.x), static_cast<uint32_t>(v.y)};
	  });
	defBlitInfo.property(
	  "extentsDst",
	  +[](const prosper::util::BlitInfo &blitInfo) -> std::optional<Vector2i> {
		  if(!blitInfo.extentsDst.has_value())
			  return {};
		  return Vector2i {blitInfo.extentsDst->width, blitInfo.extentsDst->height};
	  },
	  +[](lua_State *l, prosper::util::BlitInfo &blitInfo, const luabind::object &ext) {
		  if(luabind::type(ext) == LUA_TNIL) {
			  blitInfo.extentsDst = {};
			  return;
		  }
		  auto v = luabind::object_cast<Vector2i>(ext);
		  blitInfo.extentsDst = prosper::Extent2D {static_cast<uint32_t>(v.x), static_cast<uint32_t>(v.y)};
	  });
	prosperMod[defBlitInfo];

	static_assert(sizeof(prosper::Offset3D) == sizeof(Vector3i));
	auto defCopyInfo = luabind::class_<prosper::util::CopyInfo>("ImageCopyInfo");
	defCopyInfo.def(luabind::constructor<>());
	defCopyInfo.def_readwrite("width", &prosper::util::CopyInfo::width);
	defCopyInfo.def_readwrite("height", &prosper::util::CopyInfo::height);
	defCopyInfo.def_readwrite("srcSubresource", &prosper::util::CopyInfo::srcSubresource);
	defCopyInfo.def_readwrite("dstSubresource", &prosper::util::CopyInfo::dstSubresource);
	defCopyInfo.def_readwrite("srcOffset", reinterpret_cast<Vector3i prosper::util::CopyInfo::*>(&prosper::util::CopyInfo::srcOffset));
	defCopyInfo.def_readwrite("dstOffset", reinterpret_cast<Vector3i prosper::util::CopyInfo::*>(&prosper::util::CopyInfo::dstOffset));
	defCopyInfo.def_readwrite("srcImageLayout", reinterpret_cast<uint32_t prosper::util::CopyInfo::*>(&prosper::util::CopyInfo::srcImageLayout));
	defCopyInfo.def_readwrite("dstImageLayout", reinterpret_cast<uint32_t prosper::util::CopyInfo::*>(&prosper::util::CopyInfo::dstImageLayout));
	prosperMod[defCopyInfo];

	auto defBufferCopyInfo = luabind::class_<prosper::util::BufferCopy>("BufferCopyInfo");
	defBufferCopyInfo.def(luabind::constructor<>());
	defBufferCopyInfo.def_readwrite("srcOffset", &prosper::util::BufferCopy::srcOffset);
	defBufferCopyInfo.def_readwrite("dstOffset", &prosper::util::BufferCopy::dstOffset);
	defBufferCopyInfo.def_readwrite("size", &prosper::util::BufferCopy::size);
	prosperMod[defBufferCopyInfo];

	auto defBufferImageCopyInfo = luabind::class_<prosper::util::BufferImageCopyInfo>("BufferImageCopyInfo");
	defBufferImageCopyInfo.def(luabind::constructor<>());
	defBufferImageCopyInfo.def(
	  "SetImageSize", +[](lua_State *l, prosper::util::BufferImageCopyInfo &copyInfo, uint32_t width, uint32_t height) {
		  copyInfo.imageExtent = {width, height};
	  });
	defBufferImageCopyInfo.def(
	  "GetImageSize", +[](lua_State *l, prosper::util::BufferImageCopyInfo &copyInfo) -> std::optional<Vector2i> {
		  if(copyInfo.imageExtent.has_value() == false)
			  return {};
		  return *copyInfo.imageExtent;
	  });
	defBufferImageCopyInfo.def_readwrite("bufferOffset", &prosper::util::BufferImageCopyInfo::bufferOffset);
	defBufferImageCopyInfo.def_readwrite("imageSize", &prosper::util::BufferImageCopyInfo::imageExtent);
	defBufferImageCopyInfo.def_readwrite("mipLevel", &prosper::util::BufferImageCopyInfo::mipLevel);
	defBufferImageCopyInfo.def_readwrite("baseArrayLayer", &prosper::util::BufferImageCopyInfo::baseArrayLayer);
	defBufferImageCopyInfo.def_readwrite("layerCount", &prosper::util::BufferImageCopyInfo::layerCount);
	defBufferImageCopyInfo.def_readwrite("aspectMask", reinterpret_cast<uint32_t prosper::util::BufferImageCopyInfo::*>(&prosper::util::BufferImageCopyInfo::aspectMask));
	defBufferImageCopyInfo.def_readwrite("dstImageLayout", reinterpret_cast<uint32_t prosper::util::BufferImageCopyInfo::*>(&prosper::util::BufferImageCopyInfo::dstImageLayout));
	prosperMod[defBufferImageCopyInfo];

	auto defPipelineBarrierInfo = luabind::class_<prosper::util::PipelineBarrierInfo>("PipelineBarrierInfo");
	defPipelineBarrierInfo.def(luabind::constructor<>());
	defPipelineBarrierInfo.def_readwrite("srcStageMask", reinterpret_cast<uint32_t prosper::util::PipelineBarrierInfo::*>(&prosper::util::PipelineBarrierInfo::srcStageMask));
	defPipelineBarrierInfo.def_readwrite("dstStageMask", reinterpret_cast<uint32_t prosper::util::PipelineBarrierInfo::*>(&prosper::util::PipelineBarrierInfo::dstStageMask));
	//defPipelineBarrierInfo.def_readwrite("bufferBarriers",&prosper::util::PipelineBarrierInfo::bufferBarriers); // prosper TODO
	//defPipelineBarrierInfo.def_readwrite("imageBarriers",&prosper::util::PipelineBarrierInfo::imageBarriers); // prosper TODO
	prosperMod[defPipelineBarrierInfo];

	auto defImageSubresourceRange = luabind::class_<prosper::util::ImageSubresourceRange>("ImageSubresourceRange");
	defImageSubresourceRange.def(luabind::constructor<>());
	defImageSubresourceRange.def(luabind::constructor<uint32_t>());
	defImageSubresourceRange.def(luabind::constructor<uint32_t, uint32_t>());
	defImageSubresourceRange.def(luabind::constructor<uint32_t, uint32_t, uint32_t>());
	defImageSubresourceRange.def(luabind::constructor<uint32_t, uint32_t, uint32_t, uint32_t>());
	defImageSubresourceRange.def_readwrite("baseMipLevel", &prosper::util::ImageSubresourceRange::baseMipLevel);
	defImageSubresourceRange.def_readwrite("levelCount", &prosper::util::ImageSubresourceRange::levelCount);
	defImageSubresourceRange.def_readwrite("baseArrayLayer", &prosper::util::ImageSubresourceRange::baseArrayLayer);
	defImageSubresourceRange.def_readwrite("layerCount", &prosper::util::ImageSubresourceRange::layerCount);
	prosperMod[defImageSubresourceRange];

	auto defClearImageInfo = luabind::class_<prosper::util::ClearImageInfo>("ClearImageInfo");
	defClearImageInfo.def(luabind::constructor<>());
	defClearImageInfo.def_readwrite("subresourceRange", &prosper::util::ClearImageInfo::subresourceRange);
	prosperMod[defClearImageInfo];

	auto defVkTexture = luabind::class_<Lua::Vulkan::Texture>("Texture");
	defVkTexture.def(luabind::tostring(luabind::self));
	defVkTexture.def(luabind::const_self == luabind::const_self);
	defVkTexture.def("GetImage", static_cast<prosper::IImage &(Lua::Vulkan::Texture::*)()>(&Lua::Vulkan::Texture::GetImage), luabind::shared_from_this_policy<0> {});
	defVkTexture.def("GetImageView", static_cast<prosper::IImageView *(Lua::Vulkan::Texture::*)()>(&Lua::Vulkan::Texture::GetImageView), luabind::shared_from_this_policy<0> {});
	defVkTexture.def("GetImageView", static_cast<prosper::IImageView *(Lua::Vulkan::Texture::*)(uint32_t)>(&Lua::Vulkan::Texture::GetImageView), luabind::shared_from_this_policy<0> {});
	defVkTexture.def("GetSampler", static_cast<prosper::ISampler *(Lua::Vulkan::Texture::*)()>(&Lua::Vulkan::Texture::GetSampler), luabind::shared_from_this_policy<0> {});
	defVkTexture.def("GetWidth", &Lua::Vulkan::VKTexture::GetWidth);
	defVkTexture.def("GetHeight", &Lua::Vulkan::VKTexture::GetHeight);
	defVkTexture.def("GetFormat", static_cast<prosper::Format (*)(const Lua::Vulkan::Texture &)>([](const Lua::Vulkan::Texture &tex) { return tex.GetImage().GetFormat(); }));
	defVkTexture.def("IsValid", &Lua::Vulkan::VKTexture::IsValid);
	defVkTexture.def(
	  "SetDebugName", +[](lua_State *l, Lua::Vulkan::Texture &tex, const std::string &name) { Lua::Vulkan::VKContextObject::SetDebugName(l, tex, name); });
	defVkTexture.def(
	  "GetDebugName", +[](lua_State *l, Lua::Vulkan::Texture &tex) { return Lua::Vulkan::VKContextObject::GetDebugName(l, tex); });
	defVkTexture.def("SetImageView", &Lua::Vulkan::Texture::SetImageView);
	//
	prosperMod[defVkTexture];

	auto defVkImage = luabind::class_<Lua::Vulkan::Image>("Image");

	auto defToImageBufferInfo = luabind::class_<::util::ToImageBufferInfo>("ToImageBufferInfo");
	defToImageBufferInfo.def(luabind::constructor<>());
	defToImageBufferInfo.def_readwrite("includeLayers", &::util::ToImageBufferInfo::includeLayers);
	defToImageBufferInfo.def_readwrite("includeMipmaps", &::util::ToImageBufferInfo::includeMipmaps);
	defToImageBufferInfo.def_readwrite("inputImageLayout", &::util::ToImageBufferInfo::inputImageLayout);
	defToImageBufferInfo.def_readwrite("stagingImage", &::util::ToImageBufferInfo::stagingImage);
	defToImageBufferInfo.def_readwrite("targetFormat", &::util::ToImageBufferInfo::targetFormat);
	defVkImage.scope[defToImageBufferInfo];

	defVkImage.def(luabind::tostring(luabind::self));
	defVkImage.def(luabind::const_self == luabind::const_self);
	defVkImage.def("IsValid", &Lua::Vulkan::VKImage::IsValid);
	defVkImage.def("GetAspectSubresourceLayout", &prosper::IImage::GetSubresourceLayout);
	defVkImage.def("GetAspectSubresourceLayout", static_cast<std::optional<prosper::util::SubresourceLayout> (*)(Lua::Vulkan::Image &, uint32_t)>([](Lua::Vulkan::Image &img, uint32_t layer) -> std::optional<prosper::util::SubresourceLayout> { return img.GetSubresourceLayout(layer); }));
	defVkImage.def("GetAspectSubresourceLayout", static_cast<std::optional<prosper::util::SubresourceLayout> (*)(Lua::Vulkan::Image &)>([](Lua::Vulkan::Image &img) -> std::optional<prosper::util::SubresourceLayout> { return img.GetSubresourceLayout(); }));
	defVkImage.def("IsSrgb", &Lua::Vulkan::Image::IsSrgb);
	defVkImage.def("IsNormalMap", &Lua::Vulkan::Image::IsNormalMap);
	defVkImage.def("SetSrgb", &Lua::Vulkan::Image::SetSrgb);
	defVkImage.def("SetNormalMap", &Lua::Vulkan::Image::SetNormalMap);
	defVkImage.def("GetAlignment", &Lua::Vulkan::Image::GetAlignment);
	defVkImage.def("GetExtent2D", static_cast<std::pair<uint32_t, uint32_t> (*)(Lua::Vulkan::Image &, uint32_t)>([](Lua::Vulkan::Image &img, uint32_t mipmap) -> std::pair<uint32_t, uint32_t> {
		auto extents = img.GetExtents(mipmap);
		return {extents.width, extents.height};
	}));
	defVkImage.def("GetFormat", &Lua::Vulkan::Image::GetFormat);
#if 0
	defVkImage.def("GetExtent3D",&Lua::Vulkan::VKImage::GetExtent3D);
	defVkImage.def("GetMemoryTypes",&Lua::Vulkan::VKImage::GetMemoryTypes);
	defVkImage.def("GetStorageSize",&Lua::Vulkan::VKImage::GetStorageSize);
	//defVkImage.def("GetParentSwapchain",&Lua::Vulkan::VKImage::GetParentSwapchain);
	defVkImage.def("GetMemoryBlock",&Lua::Vulkan::VKImage::GetMemoryBlock);
	defVkImage.def("GetSubresourceRange",&Lua::Vulkan::VKImage::GetSubresourceRange);
#endif
	defVkImage.def("GetMipmapSize", static_cast<std::pair<uint32_t, uint32_t> (*)(Lua::Vulkan::Image &, uint32_t)>([](Lua::Vulkan::Image &img, uint32_t mipmap) -> std::pair<uint32_t, uint32_t> {
		auto extents = img.GetExtents(mipmap);
		return {extents.width, extents.height};
	}));
	defVkImage.def("GetLayerCount", &Lua::Vulkan::Image::GetLayerCount);
	defVkImage.def("GetMipmapCount", &Lua::Vulkan::Image::GetMipmapCount);
	defVkImage.def("GetSampleCount", &Lua::Vulkan::Image::GetSampleCount);
	defVkImage.def("GetSharingMode", &Lua::Vulkan::Image::GetSharingMode);
	defVkImage.def("GetTiling", &Lua::Vulkan::Image::GetTiling);
	defVkImage.def("GetType", &Lua::Vulkan::Image::GetType);
	defVkImage.def("GetUsage", &Lua::Vulkan::Image::GetUsageFlags);
	defVkImage.def("GetWidth", &Lua::Vulkan::Image::GetWidth);
	defVkImage.def("GetWidth", &Lua::Vulkan::Image::GetWidth, luabind::default_parameter_policy<2, uint32_t {0}> {});
	defVkImage.def("GetHeight", &Lua::Vulkan::Image::GetHeight);
	defVkImage.def("GetHeight", &Lua::Vulkan::Image::GetHeight, luabind::default_parameter_policy<2, uint32_t {0}> {});
	defVkImage.def("WriteMemory", static_cast<void (*)(lua_State *, Lua::Vulkan::Image &, uint32_t, uint32_t, uimg::ImageBuffer &, uint32_t, uint32_t)>(Lua::Vulkan::VKImage::WriteMemory));
	defVkImage.def("WriteMemory", static_cast<void (*)(lua_State *, Lua::Vulkan::Image &, uint32_t, uint32_t, uimg::ImageBuffer &, uint32_t)>(Lua::Vulkan::VKImage::WriteMemory));
	defVkImage.def("WriteMemory", static_cast<void (*)(lua_State *, Lua::Vulkan::Image &, uint32_t, uint32_t, uimg::ImageBuffer &)>(Lua::Vulkan::VKImage::WriteMemory));
	defVkImage.def(
	  "SetDebugName", +[](lua_State *l, Lua::Vulkan::Image &img, const std::string &name) { Lua::Vulkan::VKContextObject::SetDebugName(l, img, name); });
	defVkImage.def(
	  "GetDebugName", +[](lua_State *l, Lua::Vulkan::Image &img) { return Lua::Vulkan::VKContextObject::GetDebugName(l, img); });
	defVkImage.def("Convert",
	  static_cast<std::shared_ptr<prosper::IImage> (*)(lua_State *, Lua::Vulkan::Image &, Lua::Vulkan::CommandBuffer &, prosper::Format)>(
	    [](lua_State *l, Lua::Vulkan::Image &img, Lua::Vulkan::CommandBuffer &cmd, prosper::Format format) -> std::shared_ptr<prosper::IImage> { return img.Convert(cmd, format); }));
	defVkImage.def("Copy",
	  static_cast<std::shared_ptr<prosper::IImage> (*)(lua_State *, Lua::Vulkan::Image &, Lua::Vulkan::CommandBuffer &, prosper::util::ImageCreateInfo &)>(
	    [](lua_State *l, Lua::Vulkan::Image &img, Lua::Vulkan::CommandBuffer &cmd, prosper::util::ImageCreateInfo &imgCreateInfo) -> std::shared_ptr<prosper::IImage> { return img.Copy(cmd, imgCreateInfo); }));
	defVkImage.def("Copy",
	  static_cast<std::shared_ptr<prosper::IImage> (*)(lua_State *, Lua::Vulkan::Image &, Lua::Vulkan::CommandBuffer &)>([](lua_State *l, Lua::Vulkan::Image &img, Lua::Vulkan::CommandBuffer &cmd) -> std::shared_ptr<prosper::IImage> { return img.Copy(cmd, img.GetCreateInfo()); }));
	defVkImage.def("GetCreateInfo", static_cast<prosper::util::ImageCreateInfo &(prosper::IImage::*)()>(&prosper::IImage::GetCreateInfo), luabind::copy_policy<0> {});
	defVkImage.def(
	  "ToImageBuffer", +[](lua_State *l, Lua::Vulkan::Image &img, bool includeLayers, bool includeMipmaps, uint32_t targetFormat, prosper::ImageLayout inputImageLayout) {
		  std::vector<std::vector<std::shared_ptr<uimg::ImageBuffer>>> imgBuffers;
		  util::ToImageBufferInfo info {};
		  info.targetFormat = static_cast<uimg::Format>(targetFormat);
		  info.includeLayers = includeLayers;
		  info.includeMipmaps = includeMipmaps;
		  info.inputImageLayout = inputImageLayout;
		  auto result = util::to_image_buffer(img, info, imgBuffers);
		  if(result == false || imgBuffers.empty())
			  return;
		  push_image_buffers(l, includeLayers, includeMipmaps, imgBuffers);
	  });
	defVkImage.def(
	  "ToImageBuffer", +[](lua_State *l, Lua::Vulkan::Image &img, bool includeLayers, bool includeMipmaps, uint32_t targetFormat) {
		  std::vector<std::vector<std::shared_ptr<uimg::ImageBuffer>>> imgBuffers;
		  util::ToImageBufferInfo info {};
		  info.targetFormat = static_cast<uimg::Format>(targetFormat);
		  info.includeLayers = includeLayers;
		  info.includeMipmaps = includeMipmaps;
		  auto result = util::to_image_buffer(img, info, imgBuffers);
		  if(result == false || imgBuffers.empty())
			  return;
		  push_image_buffers(l, includeLayers, includeMipmaps, imgBuffers);
	  });
	defVkImage.def(
	  "ToImageBuffer", +[](lua_State *l, Lua::Vulkan::Image &img, bool includeLayers, bool includeMipmaps) {
		  std::vector<std::vector<std::shared_ptr<uimg::ImageBuffer>>> imgBuffers;
		  util::ToImageBufferInfo info {};
		  info.includeLayers = includeLayers;
		  info.includeMipmaps = includeMipmaps;
		  auto result = util::to_image_buffer(img, info, imgBuffers);
		  if(result == false || imgBuffers.empty())
			  return;
		  push_image_buffers(l, includeLayers, includeMipmaps, imgBuffers);
	  });
	defVkImage.def(
	  "ToImageBuffer", +[](lua_State *l, Lua::Vulkan::Image &img, const util::ToImageBufferInfo &info) {
		  std::vector<std::vector<std::shared_ptr<uimg::ImageBuffer>>> imgBuffers;
		  auto result = util::to_image_buffer(img, info, imgBuffers);
		  if(result == false || imgBuffers.empty())
			  return;
		  push_image_buffers(l, info.includeLayers, info.includeMipmaps, imgBuffers);
	  });
	defVkImage.def("GetMemoryBuffer", static_cast<prosper::IBuffer *(*)(lua_State *, Lua::Vulkan::Image &)>([](lua_State *l, Lua::Vulkan::Image &img) -> prosper::IBuffer * {
		auto *buf = img.GetMemoryBuffer();
		if(buf == nullptr)
			return nullptr;
		// The image should have ownership over the memory, so we'll just return a pointer. It's up to the user to make sure not to use the memory after the image has been destroyed!
		return buf;
		//Lua::Push(l,buf->shared_from_this());
	}));
	prosperMod[defVkImage];

	auto debSubresourceLayout = luabind::class_<prosper::util::SubresourceLayout>("SubresourceLayout");
	debSubresourceLayout.def(luabind::constructor<>());
	debSubresourceLayout.def_readwrite("offset", &prosper::util::SubresourceLayout::offset);
	debSubresourceLayout.def_readwrite("size", &prosper::util::SubresourceLayout::size);
	debSubresourceLayout.def_readwrite("rowPitch", &prosper::util::SubresourceLayout::row_pitch);
	debSubresourceLayout.def_readwrite("arrayPitch", &prosper::util::SubresourceLayout::array_pitch);
	debSubresourceLayout.def_readwrite("depthPitch", &prosper::util::SubresourceLayout::depth_pitch);
	prosperMod[debSubresourceLayout];

	auto defVkImageView = luabind::class_<Lua::Vulkan::ImageView>("ImageView");
	defVkImageView.def(luabind::tostring(luabind::self));
	defVkImageView.def(luabind::const_self == luabind::const_self);
	defVkImageView.def("IsValid", &Lua::Vulkan::VKImageView::IsValid);
	defVkImageView.def("GetAspectMask", &Lua::Vulkan::ImageView::GetAspectMask);
	defVkImageView.def("GetBaseLayer", &Lua::Vulkan::ImageView::GetBaseLayer);
	defVkImageView.def("GetBaseMipmapLevel", &Lua::Vulkan::ImageView::GetBaseMipmapLevel);
#if 0
	defVkImageView.def("GetBaseMipmapSize",&Lua::Vulkan::VKImageView::GetBaseMipmapSize);
	//defVkImageView.def("GetParentImage",&Lua::Vulkan::VKImageView::GetParentImage);
	defVkImageView.def("GetSubresourceRange",&Lua::Vulkan::VKImageView::GetSubresourceRange);
#endif
	defVkImageView.def("GetImageFormat", &Lua::Vulkan::ImageView::GetFormat);
	defVkImageView.def("GetLayerCount", &Lua::Vulkan::ImageView::GetLayerCount);
	defVkImageView.def("GetMipmapCount", &Lua::Vulkan::ImageView::GetMipmapCount);
	defVkImageView.def("GetSwizzleArray", &Lua::Vulkan::ImageView::GetSwizzleArray);
	defVkImageView.def("GetType", &Lua::Vulkan::ImageView::GetType);
	defVkImageView.def(
	  "SetDebugName", +[](lua_State *l, Lua::Vulkan::ImageView &imgView, const std::string &name) { Lua::Vulkan::VKContextObject::SetDebugName(l, imgView, name); });
	defVkImageView.def(
	  "GetDebugName", +[](lua_State *l, Lua::Vulkan::ImageView &imgView) { return Lua::Vulkan::VKContextObject::GetDebugName(l, imgView); });
	prosperMod[defVkImageView];

	auto defVkSampler = luabind::class_<Lua::Vulkan::Sampler>("Sampler");
	defVkSampler.def(luabind::tostring(luabind::self));
	defVkSampler.def(luabind::const_self == luabind::const_self);
	defVkSampler.def("IsValid", &Lua::Vulkan::VKSampler::IsValid);
	defVkSampler.def("Update", &Lua::Vulkan::Sampler::Update);
	defVkSampler.def("GetMagFilter", &Lua::Vulkan::Sampler::GetMagFilter);
	defVkSampler.def("GetMinFilter", &Lua::Vulkan::Sampler::GetMinFilter);
	defVkSampler.def("GetMipmapMode", &Lua::Vulkan::Sampler::GetMipmapMode);
	defVkSampler.def("GetAddressModeU", &Lua::Vulkan::Sampler::GetAddressModeU);
	defVkSampler.def("GetAddressModeV", &Lua::Vulkan::Sampler::GetAddressModeV);
	defVkSampler.def("GetAddressModeW", &Lua::Vulkan::Sampler::GetAddressModeW);
	defVkSampler.def("GetMipLodBias", &Lua::Vulkan::Sampler::GetLodBias);
	defVkSampler.def("GetAnisotropyEnabled", &Lua::Vulkan::VKSampler::GetAnisotropyEnabled);
	defVkSampler.def("GetMaxAnisotropy", &Lua::Vulkan::Sampler::GetMaxAnisotropy);
	defVkSampler.def("GetCompareEnabled", &Lua::Vulkan::Sampler::GetCompareEnabled);
	defVkSampler.def("GetCompareOperation", &Lua::Vulkan::Sampler::GetCompareOp);
	defVkSampler.def("GetMinLod", &Lua::Vulkan::Sampler::GetMinLod);
	defVkSampler.def("GetMaxLod", &Lua::Vulkan::Sampler::GetMaxLod);
	defVkSampler.def("GetBorderColor", &Lua::Vulkan::Sampler::GetBorderColor);
	// defVkSampler.def("GetUnnormalizedCoordinates",&Lua::Vulkan::VKSampler::GetUseUnnormalizedCoordinates);
	defVkSampler.def("SetMagFilter", &Lua::Vulkan::Sampler::SetMagFilter);
	defVkSampler.def("SetMinFilter", &Lua::Vulkan::Sampler::SetMinFilter);
	defVkSampler.def("SetMipmapMode", &Lua::Vulkan::Sampler::SetMipmapMode);
	defVkSampler.def("SetAddressModeU", &Lua::Vulkan::Sampler::SetAddressModeU);
	defVkSampler.def("SetAddressModeV", &Lua::Vulkan::Sampler::SetAddressModeV);
	defVkSampler.def("SetAddressModeW", &Lua::Vulkan::Sampler::SetAddressModeW);
	//defVkSampler.def("SetMipLodBias",&Lua::Vulkan::VKSampler::SetMipLodBias);
	//defVkSampler.def("SetAnisotropyEnabled",&Lua::Vulkan::VKSampler::SetAnisotropyEnabled);
	defVkSampler.def("SetMaxAnisotropy", &Lua::Vulkan::Sampler::SetMaxAnisotropy);
	defVkSampler.def("SetCompareEnabled", &Lua::Vulkan::Sampler::SetCompareEnable);
	defVkSampler.def("SetCompareOperation", &Lua::Vulkan::Sampler::SetCompareOp);
	defVkSampler.def("SetMinLod", &Lua::Vulkan::Sampler::SetMinLod);
	defVkSampler.def("SetMaxLod", &Lua::Vulkan::Sampler::SetMaxLod);
	defVkSampler.def("SetBorderColor", &Lua::Vulkan::Sampler::SetBorderColor);
	// defVkSampler.def("SetUnnormalizedCoordinates",&Lua::Vulkan::VKSampler::SetUseUnnormalizedCoordinates);
	defVkSampler.def(
	  "SetDebugName", +[](lua_State *l, Lua::Vulkan::Sampler &smp, const std::string &name) { Lua::Vulkan::VKContextObject::SetDebugName(l, smp, name); });
	defVkSampler.def(
	  "GetDebugName", +[](lua_State *l, Lua::Vulkan::Sampler &smp) { return Lua::Vulkan::VKContextObject::GetDebugName(l, smp); });
	prosperMod[defVkSampler];

	auto defVkRenderTarget = luabind::class_<Lua::Vulkan::RenderTarget>("RenderTarget");
	defVkRenderTarget.def(luabind::tostring(luabind::self));
	defVkRenderTarget.def(luabind::const_self == luabind::const_self);
	defVkRenderTarget.def("GetTexture", static_cast<prosper::Texture &(Lua::Vulkan::RenderTarget::*)()>(&Lua::Vulkan::RenderTarget::GetTexture), luabind::shared_from_this_policy<0> {});
	defVkRenderTarget.def("GetTexture", static_cast<prosper::Texture *(Lua::Vulkan::RenderTarget::*)(uint32_t)>(&Lua::Vulkan::RenderTarget::GetTexture), luabind::shared_from_this_policy<0> {});
	defVkRenderTarget.def("GetRenderPass", static_cast<prosper::IRenderPass &(Lua::Vulkan::RenderTarget::*)()>(&Lua::Vulkan::RenderTarget::GetRenderPass), luabind::shared_from_this_policy<0> {});
	defVkRenderTarget.def("GetFramebuffer", static_cast<prosper::IFramebuffer &(Lua::Vulkan::RenderTarget::*)()>(&Lua::Vulkan::RenderTarget::GetFramebuffer), luabind::shared_from_this_policy<0> {});
	defVkRenderTarget.def("GetWidth", &Lua::Vulkan::VKRenderTarget::GetWidth);
	defVkRenderTarget.def("GetHeight", &Lua::Vulkan::VKRenderTarget::GetHeight);
	defVkRenderTarget.def("GetFormat", &Lua::Vulkan::VKRenderTarget::GetFormat);
	defVkRenderTarget.def("GetColorAttachmentTexture", &get_color_attachment_texture);
	defVkRenderTarget.def("GetColorAttachmentTexture", &get_color_attachment_texture, luabind::default_parameter_policy<3, 0u> {});
	defVkRenderTarget.def(
	  "GetDepthStencilAttachmentTexture", +[](lua_State *l, Lua::Vulkan::RenderTarget &rt) -> std::shared_ptr<prosper::Texture> {
		  auto n = rt.GetAttachmentCount();
		  for(auto i = decltype(n) {0u}; i < n; ++i) {
			  auto *tex = rt.GetTexture(i);
			  if(!tex)
				  return nullptr;
			  if(!prosper::util::is_depth_format(tex->GetImage().GetFormat()))
				  continue;
			  return tex->shared_from_this();
		  }
		  return nullptr;
	  });
	defVkRenderTarget.def(
	  "SetDebugName", +[](lua_State *l, Lua::Vulkan::RenderTarget &rt, const std::string &name) { Lua::Vulkan::VKContextObject::SetDebugName(l, rt, name); });
	defVkRenderTarget.def(
	  "GetDebugName", +[](lua_State *l, Lua::Vulkan::RenderTarget &rt) { return Lua::Vulkan::VKContextObject::GetDebugName(l, rt); });
	prosperMod[defVkRenderTarget];

	auto defVkTimestampQuery = luabind::class_<Lua::Vulkan::TimestampQuery>("TimestampQuery");
	defVkTimestampQuery.def(luabind::tostring(luabind::self));
	defVkTimestampQuery.def(luabind::const_self == luabind::const_self);
	defVkTimestampQuery.def("IsValid", &Lua::Vulkan::VKTimestampQuery::IsValid);
	prosperMod[defVkTimestampQuery];

	auto defVkTimerQuery = luabind::class_<Lua::Vulkan::TimerQuery>("TimerQuery");
	defVkTimerQuery.def(luabind::tostring(luabind::self));
	defVkTimerQuery.def(luabind::const_self == luabind::const_self);
	defVkTimerQuery.def("IsValid", &Lua::Vulkan::VKTimerQuery::IsValid);
	prosperMod[defVkTimerQuery];

	auto defCommandBufferRecorder = luabind::class_<Lua::Vulkan::CommandBufferRecorder>("CommandBufferRecorder");
	defCommandBufferRecorder.def(luabind::tostring(luabind::self));
	defCommandBufferRecorder.def(luabind::const_self == luabind::const_self);
	defCommandBufferRecorder.def("IsValid", static_cast<bool (*)()>([]() -> bool { return true; }));
	defCommandBufferRecorder.def("IsPending", &Lua::Vulkan::CommandBufferRecorder::IsPending);
	defCommandBufferRecorder.def("ExecuteCommands", static_cast<bool (*)(Lua::Vulkan::CommandBufferRecorder &, Lua::Vulkan::CommandBuffer &)>([](Lua::Vulkan::CommandBufferRecorder &recorder, Lua::Vulkan::CommandBuffer &drawCmd) -> bool {
		return drawCmd.IsPrimary() && recorder.ExecuteCommands(dynamic_cast<prosper::IPrimaryCommandBuffer &>(drawCmd));
	}));
	prosperMod[defCommandBufferRecorder];

	auto defClearValue = luabind::class_<Lua::Vulkan::ClearValue>("ClearValue");
	defClearValue.def(luabind::constructor<>());
	defClearValue.def(luabind::constructor<const Color &>());
	defClearValue.def(luabind::constructor<float, uint32_t>());
	defClearValue.def(luabind::constructor<float>());
	defClearValue.def("SetColor", static_cast<void (*)(lua_State *, Lua::Vulkan::ClearValue &, const Color &)>([](lua_State *l, Lua::Vulkan::ClearValue &clearValue, const Color &clearColor) {
		clearValue.clearValue.setColor(prosper::ClearColorValue {std::array<float, 4> {clearColor.r / 255.f, clearColor.g / 255.f, clearColor.b / 255.f, clearColor.a / 255.f}});
	}));
	defClearValue.def("SetDepthStencil", static_cast<void (*)(lua_State *, Lua::Vulkan::ClearValue &, float)>([](lua_State *l, Lua::Vulkan::ClearValue &clearValue, float depth) { clearValue.clearValue.setDepthStencil(prosper::ClearDepthStencilValue {depth}); }));
	defClearValue.def("SetDepthStencil", static_cast<void (*)(lua_State *, Lua::Vulkan::ClearValue &, float, uint32_t)>([](lua_State *l, Lua::Vulkan::ClearValue &clearValue, float depth, uint32_t stencil) {
		clearValue.clearValue.setDepthStencil(prosper::ClearDepthStencilValue {depth, stencil});
	}));
	prosperMod[defClearValue];

	auto defRenderPassInfo = luabind::class_<Lua::Vulkan::RenderPassInfo>("RenderPassInfo");
	defRenderPassInfo.def(luabind::constructor<const std::shared_ptr<Lua::Vulkan::RenderTarget> &>());
	defRenderPassInfo.def("SetLayer", static_cast<void (*)(lua_State *, Lua::Vulkan::RenderPassInfo &)>([](lua_State *l, Lua::Vulkan::RenderPassInfo &rpInfo) { rpInfo.layerId = {}; }));
	defRenderPassInfo.def("SetLayer", static_cast<void (*)(lua_State *, Lua::Vulkan::RenderPassInfo &, uint32_t)>([](lua_State *l, Lua::Vulkan::RenderPassInfo &rpInfo, uint32_t layerId) { rpInfo.layerId = layerId; }));
	defRenderPassInfo.def("SetClearValues",
	  static_cast<void (*)(lua_State *, Lua::Vulkan::RenderPassInfo &, std::vector<Lua::Vulkan::ClearValue> &&)>([](lua_State *l, Lua::Vulkan::RenderPassInfo &rpInfo, std::vector<Lua::Vulkan::ClearValue> &&clearValues) { rpInfo.clearValues = std::move(clearValues); }));
	defRenderPassInfo.def("AddClearValue", static_cast<void (*)(lua_State *, Lua::Vulkan::RenderPassInfo &, Lua::Vulkan::ClearValue &)>([](lua_State *l, Lua::Vulkan::RenderPassInfo &rpInfo, Lua::Vulkan::ClearValue &clearValue) { rpInfo.clearValues.push_back(clearValue); }));
	defRenderPassInfo.def("SetRenderPass", static_cast<void (*)(lua_State *, Lua::Vulkan::RenderPassInfo &)>([](lua_State *l, Lua::Vulkan::RenderPassInfo &rpInfo) { rpInfo.renderPass = nullptr; }));
	defRenderPassInfo.def("SetRenderPass", static_cast<void (*)(lua_State *, Lua::Vulkan::RenderPassInfo &, Lua::Vulkan::RenderPass &)>([](lua_State *l, Lua::Vulkan::RenderPassInfo &rpInfo, Lua::Vulkan::RenderPass &rp) { rpInfo.renderPass = rp.shared_from_this(); }));
	prosperMod[defRenderPassInfo];

	auto defWindowCreateInfo = luabind::class_<prosper::WindowSettings>("WindowCreateInfo");
	defWindowCreateInfo.def(luabind::constructor<>());
	defWindowCreateInfo.def_readwrite("resizable", &prosper::WindowSettings::resizable);
	defWindowCreateInfo.def_readwrite("visible", &prosper::WindowSettings::visible);
	defWindowCreateInfo.def_readwrite("decorated", &prosper::WindowSettings::decorated);
	defWindowCreateInfo.def_readwrite("focused", &prosper::WindowSettings::focused);
	defWindowCreateInfo.def_readwrite("autoIconify", &prosper::WindowSettings::autoIconify);
	defWindowCreateInfo.def_readwrite("floating", &prosper::WindowSettings::floating);
	defWindowCreateInfo.def_readwrite("stereo", &prosper::WindowSettings::stereo);
	defWindowCreateInfo.def_readwrite("srgbCapable", &prosper::WindowSettings::srgbCapable);
	defWindowCreateInfo.def_readwrite("doublebuffer", &prosper::WindowSettings::doublebuffer);
	defWindowCreateInfo.def_readwrite("refreshRate", &prosper::WindowSettings::refreshRate);
	defWindowCreateInfo.def_readwrite("samples", &prosper::WindowSettings::samples);
	defWindowCreateInfo.def_readwrite("redBits", &prosper::WindowSettings::redBits);
	defWindowCreateInfo.def_readwrite("greenBits", &prosper::WindowSettings::greenBits);
	defWindowCreateInfo.def_readwrite("blueBits", &prosper::WindowSettings::blueBits);
	defWindowCreateInfo.def_readwrite("alphaBits", &prosper::WindowSettings::alphaBits);
	defWindowCreateInfo.def_readwrite("depthBits", &prosper::WindowSettings::depthBits);
	defWindowCreateInfo.def_readwrite("stencilBits", &prosper::WindowSettings::stencilBits);
	defWindowCreateInfo.def_readwrite("title", &prosper::WindowSettings::title);
	defWindowCreateInfo.def_readwrite("width", &prosper::WindowSettings::width);
	defWindowCreateInfo.def_readwrite("height", &prosper::WindowSettings::height);
	prosperMod[defWindowCreateInfo];

	auto defWindow = luabind::class_<prosper::Window>("Window");
	defWindow.def(
	  "GetMonitorBounds", +[](prosper::Window &window) -> std::optional<std::tuple<Vector2, Vector2, Vector2, Vector2>> {
		  auto bounds = window->GetMonitorBounds();
		  if(!bounds.has_value())
			  return {};
		  return std::tuple<Vector2, Vector2, Vector2, Vector2> {bounds->monitorPos, bounds->monitorSize, bounds->workPos, bounds->workSize};
	  });
	defWindow.def("SetWindowTitle", static_cast<void (*)(prosper::Window &, const std::string &)>([](prosper::Window &window, const std::string &title) { window->SetWindowTitle(title); }));
	defWindow.def("ShouldClose", static_cast<bool (*)(prosper::Window &)>([](prosper::Window &window) -> bool { return window->ShouldClose(); }));
	defWindow.def("SetShouldClose", static_cast<void (*)(prosper::Window &, bool)>([](prosper::Window &window, bool shouldClose) { window->SetShouldClose(shouldClose); }));
	defWindow.def("GetClipboardString", static_cast<std::string (*)(prosper::Window &)>([](prosper::Window &window) -> std::string { return window->GetClipboardString(); }));
	defWindow.def("SetClipboardString", static_cast<void (*)(prosper::Window &, const std::string &)>([](prosper::Window &window, const std::string &str) { window->SetClipboardString(str); }));
	defWindow.def("SetCursorPos", static_cast<void (*)(prosper::Window &, const Vector2 &)>([](prosper::Window &window, const Vector2 &pos) { window->SetCursorPos(pos); }));
	defWindow.def("SetStickyKeysEnabled", static_cast<void (*)(prosper::Window &, bool)>([](prosper::Window &window, bool enabled) { window->SetStickyKeysEnabled(enabled); }));
	defWindow.def("GetStickyKeysEnabled", static_cast<bool (*)(prosper::Window &)>([](prosper::Window &window) -> bool { return window->GetStickyKeysEnabled(); }));
	defWindow.def("SetStickyMouseButtonsEnabled", static_cast<void (*)(prosper::Window &, bool)>([](prosper::Window &window, bool enabled) { window->SetStickyMouseButtonsEnabled(enabled); }));
	defWindow.def("GetStickyMouseButtonsEnabled", static_cast<bool (*)(prosper::Window &)>([](prosper::Window &window) -> bool { return window->GetStickyMouseButtonsEnabled(); }));
	defWindow.def("SwapBuffers", static_cast<void (*)(prosper::Window &)>([](prosper::Window &window) { window->SwapBuffers(); }));
	defWindow.def("GetPos", static_cast<Vector2i (*)(prosper::Window &)>([](prosper::Window &window) -> Vector2i { return window->GetPos(); }));
	defWindow.def("SetPos", static_cast<void (*)(prosper::Window &, const Vector2i &)>([](prosper::Window &window, const Vector2i &pos) { window->SetPos(pos); }));
	defWindow.def("GetSize", static_cast<Vector2i (*)(prosper::Window &)>([](prosper::Window &window) -> Vector2i { return window->GetSize(); }));
	defWindow.def("SetSize", static_cast<void (*)(prosper::Window &, const Vector2i &)>([](prosper::Window &window, const Vector2i &size) { window->SetSize(size); }));
	defWindow.def("GetFramebufferSize", static_cast<Vector2i (*)(prosper::Window &)>([](prosper::Window &window) -> Vector2i { return window->GetFramebufferSize(); }));
	defWindow.def("GetFrameSize", static_cast<Vector4i (*)(prosper::Window &)>([](prosper::Window &window) -> Vector4i { return window->GetFrameSize(); }));
	defWindow.def("Iconify", static_cast<void (*)(prosper::Window &)>([](prosper::Window &window) { window->Iconify(); }));
	defWindow.def("Restore", static_cast<void (*)(prosper::Window &)>([](prosper::Window &window) { window->Restore(); }));
	defWindow.def("Maximize", static_cast<void (*)(prosper::Window &)>([](prosper::Window &window) { window->Maximize(); }));
	defWindow.def("Show", static_cast<void (*)(prosper::Window &)>([](prosper::Window &window) { window->Show(); }));
	defWindow.def("Hide", static_cast<void (*)(prosper::Window &)>([](prosper::Window &window) { window->Hide(); }));
	defWindow.def("GetCursorPos", static_cast<Vector2 (*)(prosper::Window &)>([](prosper::Window &window) -> Vector2 { return window->GetCursorPos(); }));
	defWindow.def("ClearCursorPosOverride", static_cast<void (*)(prosper::Window &)>([](prosper::Window &window) { window->ClearCursorPosOverride(); }));
	defWindow.def("SetCursorPosOverride", static_cast<void (*)(prosper::Window &, const Vector2 &)>([](prosper::Window &window, const Vector2 &pos) { window->SetCursorPosOverride(pos); }));
	defWindow.def("MakeContextCurrent", static_cast<void (*)(prosper::Window &)>([](prosper::Window &window) { window->MakeContextCurrent(); }));
	defWindow.def("IsMaximized", static_cast<bool (*)(prosper::Window &)>([](prosper::Window &window) -> bool { return window->IsMaximized(); }));
	defWindow.def("IsFocused", static_cast<bool (*)(prosper::Window &)>([](prosper::Window &window) -> bool { return window->IsFocused(); }));
	defWindow.def("IsIconified", static_cast<bool (*)(prosper::Window &)>([](prosper::Window &window) -> bool { return window->IsIconified(); }));
	defWindow.def("IsVisible", static_cast<bool (*)(prosper::Window &)>([](prosper::Window &window) -> bool { return window->IsVisible(); }));
	defWindow.def("IsResizable", static_cast<bool (*)(prosper::Window &)>([](prosper::Window &window) -> bool { return window->IsResizable(); }));
	defWindow.def("IsDecorated", static_cast<bool (*)(prosper::Window &)>([](prosper::Window &window) -> bool { return window->IsDecorated(); }));
	defWindow.def("IsFloating", static_cast<bool (*)(prosper::Window &)>([](prosper::Window &window) -> bool { return window->IsFloating(); }));
	defWindow.def("ClearCursor", static_cast<void (*)(prosper::Window &)>([](prosper::Window &window) { window->ClearCursor(); }));
	defWindow.def("GetKeyState", static_cast<GLFW::KeyState (*)(prosper::Window &, GLFW::Key)>([](prosper::Window &window, GLFW::Key key) -> GLFW::KeyState { return window->GetKeyState(key); }));
	defWindow.def("GetMouseButtonState", static_cast<GLFW::KeyState (*)(prosper::Window &, GLFW::MouseButton)>([](prosper::Window &window, GLFW::MouseButton mouseButton) -> GLFW::KeyState { return window->GetMouseButtonState(mouseButton); }));
	defWindow.def("SetCursorInputMode", static_cast<void (*)(prosper::Window &, GLFW::CursorMode)>([](prosper::Window &window, GLFW::CursorMode cursorMode) { window->SetCursorInputMode(cursorMode); }));
	defWindow.def("GetCursorInputMode", static_cast<GLFW::CursorMode (*)(prosper::Window &)>([](prosper::Window &window) -> GLFW::CursorMode { return window->GetCursorInputMode(); }));
	defWindow.def("SetCursor", static_cast<void (*)(prosper::Window &, GLFW::Cursor &)>([](prosper::Window &window, GLFW::Cursor &cursor) { window->SetCursor(cursor); }));
	defWindow.def("Close", &prosper::Window::Close);
	defWindow.def("GetSwapchainImageCount", &prosper::Window::GetSwapchainImageCount);
	defWindow.def("IsValid", static_cast<bool (*)(prosper::Window &)>([](prosper::Window &window) -> bool { return window.IsValid(); }));
	defWindow.def("AddCloseListener",
	  static_cast<void (*)(lua_State *, prosper::Window &, const Lua::func<void> &)>([](lua_State *l, prosper::Window &window, const Lua::func<void> &function) { window.AddCloseListener([function]() mutable { const_cast<Lua::func<void> &>(function)(); }); }));
	prosperMod[defWindow];

	auto defRenderBuffer = luabind::class_<prosper::IRenderBuffer>("RenderBuffer");
	prosperMod[defRenderBuffer];

	register_vulkan_lua_interface2(lua, prosperMod);
}

/////////////////////////////////

bool Lua::Vulkan::VKTexture::IsValid(lua_State *l, Texture &hTex) { return true; }
uint32_t Lua::Vulkan::VKTexture::GetWidth(lua_State *l, Texture &hTex)
{
	auto &img = hTex.GetImage();
	auto extents = img.GetExtents();
	return extents.width;
}
uint32_t Lua::Vulkan::VKTexture::GetHeight(lua_State *l, Texture &hTex)
{
	auto &img = hTex.GetImage();
	auto extents = img.GetExtents();
	return extents.height;
}

/////////////////////////////////

bool Lua::Vulkan::VKImage::IsValid(lua_State *l, Image &hImg) { return true; }
std::pair<uint32_t, uint32_t> Lua::Vulkan::VKImage::GetExtent2D(lua_State *l, Image &hImg, uint32_t mipmap)
{
	auto extents = hImg.GetExtents(mipmap);
	return {extents.width, extents.height};
}
#if 0
void Lua::Vulkan::VKImage::GetExtent3D(lua_State *l,Image &hImg,uint32_t mipmap)
{
	auto extents = hImg->get_image_extent_3D(mipmap);
	Lua::PushInt(l,extents.width);
	Lua::PushInt(l,extents.height);
	Lua::PushInt(l,extents.depth);
}
void Lua::Vulkan::VKImage::GetMemoryTypes(lua_State *l,Image &hImg)
{
	Lua::PushInt(l,hImg->get_image_memory_types(0u)); // TODO: Plane parameter
}
void Lua::Vulkan::VKImage::GetStorageSize(lua_State *l,Image &hImg)
{
	Lua::PushInt(l,hImg->get_image_storage_size(0u)); // TODO: Plane parameter
}
void Lua::Vulkan::VKImage::GetParentSwapchain(lua_State *l,Image &hImg)
{
	hImg->get_create_info_ptr()->get_swapchain(); // prosper TODO
}
void Lua::Vulkan::VKImage::GetMemoryBlock(lua_State *l,Image &hImg)
{
	Lua::Push<Memory*>(l,hImg->get_memory_block());
}
void Lua::Vulkan::VKImage::GetSubresourceRange(lua_State *l,Image &hImg)
{
	auto subresourceRange = hImg->get_subresource_range();
	Lua::Push(l,reinterpret_cast<prosper::ImageSubresourceRange&>(subresourceRange));
}
#endif
std::pair<uint32_t, uint32_t> Lua::Vulkan::VKImage::GetMipmapSize(lua_State *l, Image &hImg, uint32_t mipmap)
{
	uint32_t width;
	uint32_t height;
	auto extents = hImg.GetExtents(mipmap);
	return {extents.width, extents.height};
}
void Lua::Vulkan::VKImage::WriteMemory(lua_State *l, Image &hImg, uint32_t x, uint32_t y, uimg::ImageBuffer &imgBuf, uint32_t layerIndex, uint32_t mipLevel)
{
	hImg.WriteImageData(x, y, imgBuf.GetWidth(), imgBuf.GetHeight(), layerIndex, mipLevel, imgBuf.GetSize(), static_cast<uint8_t *>(imgBuf.GetData()));
}
void Lua::Vulkan::VKImage::WriteMemory(lua_State *l, Image &hImg, uint32_t x, uint32_t y, uimg::ImageBuffer &imgBuf, uint32_t layerIndex) { WriteMemory(l, hImg, x, y, imgBuf, layerIndex, 0); }
void Lua::Vulkan::VKImage::WriteMemory(lua_State *l, Image &hImg, uint32_t x, uint32_t y, uimg::ImageBuffer &imgBuf) { WriteMemory(l, hImg, x, y, imgBuf, 0u, 0); }

/////////////////////////////////

bool Lua::Vulkan::VKImageView::IsValid(lua_State *l, ImageView &hImgView) { return true; }
#if 0
void Lua::Vulkan::VKImageView::GetBaseMipmapSize(lua_State *l,ImageView &hImgView)
{
	auto width = 0u;
	auto height = 0u;
	hImgView->get_base_mipmap_size(&width,&height,&depth);
	Lua::PushInt(l,width);
	Lua::PushInt(l,height);
	Lua::PushInt(l,depth);
}
void Lua::Vulkan::VKImageView::GetParentImage(lua_State *l,ImageView &hImgView)
{
	auto *img = hImgView->get_create_info_ptr()->get_parent_image();
	if(img == nullptr)
		return;
	// prosper TODO: get_shared_from_this()?
}
void Lua::Vulkan::VKImageView::GetSubresourceRange(lua_State *l,ImageView &hImgView)
{
	auto subresourceRange = hImgView->get_subresource_range();
	Lua::Push(l,reinterpret_cast<prosper::ImageSubresourceRange&>(subresourceRange));
}
#endif

/////////////////////////////////

bool Lua::Vulkan::VKSampler::IsValid(lua_State *l, Sampler &hSampler) { return true; }
bool Lua::Vulkan::VKSampler::GetAnisotropyEnabled(lua_State *l, Sampler &hSampler) { return hSampler.GetMaxAnisotropy() != 0.f; }

/////////////////////////////////

bool Lua::Vulkan::VKRenderTarget::IsValid(lua_State *l, RenderTarget &hRt) { return true; }
uint32_t Lua::Vulkan::VKRenderTarget::GetWidth(lua_State *l, RenderTarget &hRt)
{
	auto extents = hRt.GetTexture().GetImage().GetExtents();
	return extents.width;
}
uint32_t Lua::Vulkan::VKRenderTarget::GetHeight(lua_State *l, RenderTarget &hRt)
{
	auto extents = hRt.GetTexture().GetImage().GetExtents();
	return extents.height;
}
prosper::Format Lua::Vulkan::VKRenderTarget::GetFormat(lua_State *l, RenderTarget &hRt) { return hRt.GetTexture().GetImage().GetFormat(); }

/////////////////////////////////

bool Lua::Vulkan::VKTimestampQuery::IsValid(lua_State *l, TimestampQuery &hTimestampQuery) { return true; }
Lua::var<bool, int64_t> Lua::Vulkan::VKTimestampQuery::QueryResult(lua_State *l, TimestampQuery &hTimestampQuery)
{
	std::chrono::nanoseconds r;
	if(hTimestampQuery.QueryResult(r) == false)
		return {luabind::object {l, false}};
	return {luabind::object {l, r.count()}};
}

/////////////////////////////////

bool Lua::Vulkan::VKTimerQuery::IsValid(lua_State *l, TimerQuery &hTimerQuery) { return true; }
Lua::var<bool, int64_t> Lua::Vulkan::VKTimerQuery::QueryResult(lua_State *l, TimerQuery &hTimerQuery)
{
	std::chrono::nanoseconds r;
	if(hTimerQuery.QueryResult(r) == false)
		return {luabind::object {l, false}};
	return {luabind::object {l, r.count()}};
}
