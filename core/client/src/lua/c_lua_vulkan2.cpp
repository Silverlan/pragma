/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include <luabind/class.hpp>
#include "pragma/lua/libraries/c_lua_vulkan.h"
#include "pragma/lua/policies/shared_from_this_policy.hpp"
#include "pragma/lua/converters/shader_converter_t.hpp"
#include "pragma/model/vk_mesh.h"
#include <pragma/lua/policies/vector_policy.hpp>
#include <pragma/lua/converters/vector_converter_t.hpp>
#include <pragma/lua/converters/optional_converter_t.hpp>
#include <pragma/lua/custom_constructor.hpp>
#include <prosper_framebuffer.hpp>
#include <prosper_fence.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <prosper_window.hpp>
#include <prosper_render_pass.hpp>
#include <prosper_prepared_command_buffer.hpp>
#include <image/prosper_render_target.hpp>
#include <queries/prosper_timestamp_query.hpp>
#include <queries/prosper_timer_query.hpp>
#include <buffers/prosper_buffer.hpp>
#include <buffers/prosper_swap_buffer.hpp>
#include <prosper_swap_command_buffer.hpp>
#include <prosper_event.hpp>
#include <sharedutils/datastream.h>
#include "pragma/c_engine.h"
#include <pragma/rendering/shaders/image/c_shader_gradient.hpp>
#include <luabind/copy_policy.hpp>
#include <pragma/lua/types/udm.hpp>
#include <luainterface.hpp>

extern DLLCLIENT CEngine *c_engine;

namespace Lua {
	namespace Vulkan {
		std::vector<pragma::ShaderGradient::Node> get_gradient_nodes(lua_State *l, const luabind::tableT<void> &tNodes);
		namespace VKFramebuffer {
			static bool IsValid(lua_State *l, Framebuffer &hFramebuffer);
#if 0
			static void GetAttachment(lua_State *l,Framebuffer &hFramebuffer,uint32_t attId);
			static void GetAttachmentCount(lua_State *l,Framebuffer &hFramebuffer);
#endif
		};
		namespace VKRenderPass {
			static bool IsValid(lua_State *l, RenderPass &hRenderPass);
#if 0
			static void GetInfo(lua_State *l,RenderPass &hRenderPass);
			static void GetSwapchain(lua_State *l,RenderPass &hRenderPass);
#endif
		};
		namespace VKEvent {
			static bool IsValid(lua_State *l, Event &hEvent);
			static prosper::Result GetStatus(lua_State *l, Event &hEvent);
		};
		namespace VKFence {
			static bool IsValid(lua_State *l, Fence &hFence);
		};
		namespace VKCommandBuffer {
			static bool IsValid(lua_State *l, CommandBuffer &hCommandBuffer);
			static bool RecordClearImage(lua_State *l, CommandBuffer &hCommandBuffer, Image &img, const Color &col, const prosper::util::ClearImageInfo &clearImageInfo = {});
			static bool RecordClearImage(lua_State *l, CommandBuffer &hCommandBuffer, Image &img, std::optional<float> clearDepth, std::optional<uint32_t> clearStencil, const prosper::util::ClearImageInfo &clearImageInfo = {});
			static bool RecordClearAttachment(lua_State *l, CommandBuffer &hCommandBuffer, Image &img, const Color &col, uint32_t attId = 0u);
			static bool RecordClearAttachment(lua_State *l, CommandBuffer &hCommandBuffer, Image &img, std::optional<float> clearDepth, std::optional<uint32_t> clearStencil);
			static bool RecordCopyImage(lua_State *l, CommandBuffer &hCommandBuffer, Image &imgSrc, Image &imgDst, const prosper::util::CopyInfo &copyInfo);
			static bool RecordCopyBufferToImage(lua_State *l, CommandBuffer &hCommandBuffer, Buffer &bufSrc, Image &imgDst, const prosper::util::BufferImageCopyInfo &copyInfo);
			static bool RecordCopyBuffer(lua_State *l, CommandBuffer &hCommandBuffer, Buffer &bufSrc, Buffer &bufDst, const prosper::util::BufferCopy &copyInfo);
			static bool RecordUpdateBuffer(lua_State *l, CommandBuffer &hCommandBuffer, Buffer &buf, uint32_t offset, ::DataStream &ds);
			static bool RecordUpdateBuffer(lua_State *l, CommandBuffer &hCommandBuffer, Buffer &buf, uint32_t offset, ::udm::Type type, Lua::udm_ng value);
			static bool RecordBlitImage(lua_State *l, CommandBuffer &hCommandBuffer, Image &imgSrc, Image &imgDst, const prosper::util::BlitInfo &blitInfo);
			static bool RecordResolveImage(lua_State *l, CommandBuffer &hCommandBuffer, Image &imgSrc, Image &imgDst);
			static bool RecordBlitTexture(lua_State *l, CommandBuffer &hCommandBuffer, Texture &texSrc, Image &imgDst);
			static bool RecordGenerateMipmaps(lua_State *l, CommandBuffer &hCommandBuffer, Image &img, uint32_t currentLayout, uint32_t srcAccessMask, uint32_t srcStage);
			static bool RecordPipelineBarrier(lua_State *l, CommandBuffer &hCommandBuffer, const prosper::util::PipelineBarrierInfo &barrierInfo);
			static bool RecordImageBarrier(lua_State *l, CommandBuffer &hCommandBuffer, Image &img, uint32_t srcStageMask, uint32_t dstStageMask, uint32_t oldLayout, uint32_t newLayout, uint32_t srcAccessMask, uint32_t dstAccessMask, uint32_t baseLayer);
			static bool RecordImageBarrier(lua_State *l, CommandBuffer &hCommandBuffer, Image &img, uint32_t oldLayout, uint32_t newLayout, const prosper::util::ImageSubresourceRange &subresourceRange);
			static bool RecordBufferBarrier(lua_State *l, CommandBuffer &hCommandBuffer, Buffer &buf, uint32_t srcStageMask, uint32_t dstStageMask, uint32_t srcAccessMask, uint32_t dstAccessMask, uint32_t offset = 0ull, uint32_t size = std::numeric_limits<uint32_t>::max());
			static bool RecordSetViewport(lua_State *l, CommandBuffer &hCommandBuffer, uint32_t width, uint32_t height, uint32_t x = 0u, uint32_t y = 0u);
			static bool RecordSetScissor(lua_State *l, CommandBuffer &hCommandBuffer, uint32_t width, uint32_t height, uint32_t x = 0u, uint32_t y = 0u);
			static bool RecordBeginRenderPass(lua_State *l, CommandBuffer &hCommandBuffer, Lua::Vulkan::RenderPassInfo &rpInfo);
			static bool RecordEndRenderPass(lua_State *l, CommandBuffer &hCommandBuffer);
			static bool RecordBindIndexBuffer(lua_State *l, CommandBuffer &hCommandBuffer, Buffer &indexBuffer, uint32_t indexType, uint32_t offset);
			static bool RecordBindVertexBuffer(lua_State *l, CommandBuffer &hCommandBuffer, prosper::ShaderGraphics &, Buffer &vertexBuffer, uint32_t startBinding, uint32_t offset);
			static bool RecordBindVertexBuffers(lua_State *l, CommandBuffer &hCommandBuffer, prosper::ShaderGraphics &, const std::vector<prosper::IBuffer *> &buffers, uint32_t startBinding, const std::vector<uint64_t> &voffsets);
			static bool RecordBindVertexBuffers(lua_State *l, CommandBuffer &hCommandBuffer, prosper::ShaderGraphics &, const std::vector<prosper::IBuffer *> &buffers, uint32_t startBinding);
			static bool RecordBindVertexBuffers(lua_State *l, CommandBuffer &hCommandBuffer, prosper::ShaderGraphics &, const std::vector<prosper::IBuffer *> &buffers);
			static bool RecordCopyImageToBuffer(lua_State *l, CommandBuffer &hCommandBuffer, Image &imgSrc, uint32_t srcImageLayout, Buffer &bufDst, const prosper::util::BufferImageCopyInfo &copyInfo);
			//static void RecordDispatch(lua_State *l,CommandBuffer &hCommandBuffer,uint32_t x,uint32_t y,uint32_t z);
			//static void RecordDispatchIndirect(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &buffer,uint32_t offset);
			static bool RecordDraw(lua_State *l, CommandBuffer &hCommandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
			static bool RecordDrawIndexed(lua_State *l, CommandBuffer &hCommandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t firstInstance);
			static bool RecordDrawIndexedIndirect(lua_State *l, CommandBuffer &hCommandBuffer, Buffer &buffer, uint32_t offset, uint32_t drawCount, uint32_t stride);
			static bool RecordDrawIndirect(lua_State *l, CommandBuffer &hCommandBuffer, Buffer &buffer, uint32_t offset, uint32_t drawCount, uint32_t stride);
			static bool RecordFillBuffer(lua_State *l, CommandBuffer &hCommandBuffer, Buffer &buffer, uint32_t offset, uint32_t size, uint32_t data);
			//static void RecordResetEvent(lua_State *l,CommandBuffer &hCommandBuffer,Event &ev,uint32_t stageMask);
			static bool RecordSetBlendConstants(lua_State *l, CommandBuffer &hCommandBuffer, const Vector4 &blendConstants);
			static bool RecordSetDepthBias(lua_State *l, CommandBuffer &hCommandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float slopeScaledDepthBias);
			static bool RecordSetDepthBounds(lua_State *l, CommandBuffer &hCommandBuffer, float minDepthBounds, float maxDepthBounds);
			//static void RecordSetEvent(lua_State *l,CommandBuffer &hCommandBuffer,Event &ev,uint32_t stageMask);
			static bool RecordSetLineWidth(lua_State *l, CommandBuffer &hCommandBuffer, float lineWidt);
			static bool RecordSetStencilCompareMask(lua_State *l, CommandBuffer &hCommandBuffer, uint32_t faceMask, uint32_t stencilCompareMask);
			static bool RecordSetStencilReference(lua_State *l, CommandBuffer &hCommandBuffer, uint32_t faceMask, uint32_t stencilReference);
			static bool RecordSetStencilWriteMask(lua_State *l, CommandBuffer &hCommandBuffer, uint32_t faceMask, uint32_t stencilWriteMask);
			//static void RecordWaitEvents(lua_State *l,CommandBuffer &hCommandBuffer); // TODO
			//static void RecordWriteTimestamp(lua_State *l,CommandBuffer &hCommandBuffer); // TODO
			//static void RecordBeginQuery(lua_State *l,CommandBuffer &hCommandBuffer); // TODO
			//static void RecordEndQuery(lua_State *l,CommandBuffer &hCommandBuffer); // TODO
			static bool RecordDrawGradient(lua_State *l, CommandBuffer &hCommandBuffer, RenderTarget &rt, const Vector2 &dir, luabind::object lnodes);
			static bool StartRecording(lua_State *l, CommandBuffer &hCommandBuffer, bool oneTimeSubmit, bool simultaneousUseAllowed);
		};
		namespace VKContextObject {
			static void SetDebugName(lua_State *l, prosper::ContextObject &o, const std::string &name) { o.SetDebugName(name); }
			static std::string GetDebugName(lua_State *l, const prosper::ContextObject &o) { return o.GetDebugName(); }
		};
		namespace VKBuffer {
			static bool IsValid(lua_State *l, Buffer &hBuffer);
			static bool Write(lua_State *l, Buffer &hBuffer, uint32_t offset, ::DataStream &ds, uint32_t dsOffset, uint32_t dsSize);
			static bool Write(lua_State *l, Buffer &hBuffer, uint32_t offset, ::udm::Type type, Lua::udm_ng value);
			static Lua::opt<::DataStream> Read(lua_State *l, Buffer &hBuffer, uint32_t offset, uint32_t size);
			static bool Read(lua_State *l, Buffer &hBuffer, uint32_t offset, uint32_t size, ::DataStream &ds, uint32_t dsOffset);
			static bool Read(lua_State *l, Buffer &hBuffer, uint32_t offset, ::udm::Type type, Lua::udm_ng value);
			static bool Map(lua_State *l, Buffer &hBuffer, uint32_t offset, uint32_t size, Buffer::MapFlags mapFlags);
		};
		namespace VKDescriptorSet {
			static bool IsValid(lua_State *l, DescriptorSet &hDescSet);
#if 0
			static void GetBindingInfo(lua_State *l,DescriptorSet &hDescSet);
			static void GetBindingInfo(lua_State *l,DescriptorSet &hDescSet,uint32_t bindingIdx);
#endif
			static bool SetBindingTexture(lua_State *l, DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Texture &texture, uint32_t layerId);
			static bool SetBindingTexture(lua_State *l, DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Texture &texture);
			static bool SetBindingArrayTexture(lua_State *l, DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Texture &texture, uint32_t arrayIdx, uint32_t layerId);
			static bool SetBindingArrayTexture(lua_State *l, DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Texture &texture, uint32_t arrayIdx);
			static bool SetBindingStorageBuffer(lua_State *l, DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Buffer &buffer, uint32_t startOffset, uint32_t size);
			static bool SetBindingUniformBuffer(lua_State *l, DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Buffer &buffer, uint32_t startOffset, uint32_t size);
			static bool SetBindingUniformBufferDynamic(lua_State *l, DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Buffer &buffer, uint32_t startOffset, uint32_t size);
			//static void GetLayout(lua_State *l,DescriptorSet &hDescSet); // TODO
			//static void GetBindingProperties(lua_State *l,DescriptorSet &hDescSet); // TODO
		};
	};
};

namespace prosper {
	static bool operator==(const Lua::Vulkan::Framebuffer &a, const Lua::Vulkan::Framebuffer &b) { return &a == &b; }
	static bool operator==(const Lua::Vulkan::RenderPass &a, const Lua::Vulkan::RenderPass &b) { return &a == &b; }
	static bool operator==(const Lua::Vulkan::Event &a, const Lua::Vulkan::Event &b) { return &a == &b; }
	static bool operator==(const Lua::Vulkan::Fence &a, const Lua::Vulkan::Fence &b) { return &a == &b; }
	static bool operator==(const Lua::Vulkan::CommandBuffer &a, const Lua::Vulkan::CommandBuffer &b) { return &a == &b; }
	static bool operator==(const Lua::Vulkan::Buffer &a, const Lua::Vulkan::Buffer &b) { return &a == &b; }
	static bool operator==(const Lua::Vulkan::DescriptorSet &a, const Lua::Vulkan::DescriptorSet &b) { return &a == &b; }

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

	static std::ostream &operator<<(std::ostream &out, const Lua::Vulkan::Event &hEvent)
	{
		out << "VKEvent[";
		auto r = prosper::Result::EventReset;
		if(hEvent.IsSet())
			r = prosper::Result::EventSet;
		out << prosper::util::to_string(r);
		out << "]";
		return out;
	}

	static std::ostream &operator<<(std::ostream &out, const Lua::Vulkan::Fence &hFence)
	{
		out << "VKFence[";
		out << "]";
		return out;
	}
#if 0
	static std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::Semaphore &hSemaphore)
	{
		out<<"VKSemaphore[";
		out<<"]";
		return out;
	}

	static std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::Memory &hMemory)
	{
		out<<"VKMemory[";
		out<<"]";
		return out;
	}
#endif
	static std::ostream &operator<<(std::ostream &out, const Lua::Vulkan::CommandBuffer &hCommandBuffer)
	{
		out << "VKCommandBuffer[";
		out << "]";
		return out;
	}

	static std::ostream &operator<<(std::ostream &out, const Lua::Vulkan::Buffer &hBuffer)
	{
		out << "VKBuffer[";
		out << hBuffer.GetSize();
		out << "]";
		return out;
	}

	static std::ostream &operator<<(std::ostream &out, const Lua::Vulkan::DescriptorSet &hDescSet)
	{
		out << "VKDescriptorSet[";
		out << "]";
		return out;
	}
};

namespace pragma {
	static bool operator==(const pragma::SceneMesh &a, const pragma::SceneMesh &b) { return &a == &b; }

	static std::ostream &operator<<(std::ostream &out, const pragma::SceneMesh &)
	{
		out << "SceneMesh";
		return out;
	}
};

prosper::util::PreparedCommand::Argument Lua::Vulkan::make_pcb_arg(const Lua::Vulkan::PreparedCommandLuaArg &larg, udm::Type type)
{
	auto &o = larg.o;
	prosper::util::PreparedCommand::Argument arg {};
	auto *dynArg = luabind::object_cast_nothrow<PreparedCommandLuaDynamicArg *>(larg.o, static_cast<PreparedCommandLuaDynamicArg *>(nullptr));
	if(dynArg) {
		arg.SetDynamicValue(dynArg->argName);
		return arg;
	}
	udm::visit(type, [&o, &arg](auto tag) {
		using T = typename decltype(tag)::type;
		constexpr auto type = udm::type_to_enum<T>();
		if constexpr(udm::is_trivial_type(type))
			arg.SetStaticValue<T>(luabind::object_cast<T>(o));
		else
			throw std::runtime_error {"Expected non-trivial UDM type!"};
	});
	return arg;
}

using PcbArg = prosper::util::PreparedCommand::Argument;
using PcbLuaArg = Lua::Vulkan::PreparedCommandLuaArg;
static void pcb_recordBufferBarrier(prosper::util::PreparedCommandBuffer &pcb, Lua::Vulkan::Buffer &buf, const PcbLuaArg &srcStageMask, const PcbLuaArg &dstStageMask, const PcbLuaArg &srcAccessMask, const PcbLuaArg &dstAccessMask, const PcbLuaArg &offset, const PcbLuaArg &size)
{
	auto pbuf = buf.shared_from_this();
	pcb.PushCommand(
	  [pbuf](const prosper::util::PreparedCommandBufferRecordState &recordState) -> bool {
		  auto &cmdBuf = recordState.commandBuffer;
		  c_engine->GetRenderContext().KeepResourceAliveUntilPresentationComplete(pbuf);
		  auto size = recordState.GetArgument<uint32_t>(5);
		  auto lsize = (size != std::numeric_limits<uint32_t>::max()) ? static_cast<uint64_t>(size) : std::numeric_limits<uint64_t>::max();
		  return cmdBuf.RecordBufferBarrier(*pbuf, recordState.GetArgument<prosper::PipelineStageFlags>(0), recordState.GetArgument<prosper::PipelineStageFlags>(1), recordState.GetArgument<prosper::AccessFlags>(2), recordState.GetArgument<prosper::AccessFlags>(3),
		    recordState.GetArgument<uint32_t>(4), lsize);
	  },
	  std::move(util::make_vector<PcbArg>(Lua::Vulkan::make_pcb_arg<prosper::PipelineStageFlags>(srcStageMask), Lua::Vulkan::make_pcb_arg<prosper::PipelineStageFlags>(dstStageMask), Lua::Vulkan::make_pcb_arg<prosper::AccessFlags>(srcAccessMask),
	    Lua::Vulkan::make_pcb_arg<prosper::AccessFlags>(dstAccessMask), Lua::Vulkan::make_pcb_arg<uint32_t>(offset), Lua::Vulkan::make_pcb_arg<uint32_t>(size))));
};

static bool pcb_record_bind_descriptor_set(prosper::util::PreparedCommandBuffer &pcb, const std::shared_ptr<prosper::IDescriptorSetGroup> &descSet, const PcbLuaArg &firstSet, const PcbLuaArg &dynamicOffset)
{
	pcb.PushCommand(
	  [descSet](const prosper::util::PreparedCommandBufferRecordState &recordState) -> bool {
		  constexpr auto BIND_PIPELINE_LAYOUT = ustring::string_switch::hash("pipelineLayout");
		  auto &pipelineLayout = recordState.userData.Get<prosper::IShaderPipelineLayout>(BIND_PIPELINE_LAYOUT);

		  auto dynOffset = recordState.GetArgument<uint32_t>(1);
		  return recordState.commandBuffer.RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics, pipelineLayout, recordState.GetArgument<uint32_t>(0), *descSet->GetDescriptorSet(), &dynOffset);
	  },
	  std::move(util::make_vector<PcbArg>(Lua::Vulkan::make_pcb_arg<uint32_t>(firstSet), Lua::Vulkan::make_pcb_arg<uint32_t>(dynamicOffset))));
	return true;
}

void register_vulkan_lua_interface2(Lua::Interface &lua, luabind::module_ &prosperMod)
{
	auto defVkFramebuffer = luabind::class_<Lua::Vulkan::Framebuffer>("Framebuffer");
	defVkFramebuffer.def(luabind::tostring(luabind::self));
	defVkFramebuffer.def(luabind::const_self == luabind::const_self);
	defVkFramebuffer.def("IsValid", &Lua::Vulkan::VKFramebuffer::IsValid);
	defVkFramebuffer.def("GetWidth", &Lua::Vulkan::Framebuffer::GetWidth);
	defVkFramebuffer.def("GetHeight", &Lua::Vulkan::Framebuffer::GetHeight);
	prosperMod[defVkFramebuffer];

	auto defVkRenderPass = luabind::class_<Lua::Vulkan::RenderPass>("RenderPass");
	defVkRenderPass.def(luabind::tostring(luabind::self));
	defVkRenderPass.def(luabind::const_self == luabind::const_self);
	defVkRenderPass.def("IsValid", &Lua::Vulkan::VKRenderPass::IsValid);
	prosperMod[defVkRenderPass];

	auto defVkEvent = luabind::class_<Lua::Vulkan::Event>("Event");
	defVkEvent.def(luabind::tostring(luabind::self));
	defVkEvent.def(luabind::const_self == luabind::const_self);
	defVkEvent.def("IsValid", &Lua::Vulkan::VKEvent::IsValid);
	prosperMod[defVkEvent];

	auto defVkFence = luabind::class_<Lua::Vulkan::Fence>("Fence");
	defVkFence.def(luabind::tostring(luabind::self));
	defVkFence.def(luabind::const_self == luabind::const_self);
	defVkFence.def("IsValid", &Lua::Vulkan::VKFence::IsValid);
	prosperMod[defVkFence];

#if 0
	auto defVkSemaphore = luabind::class_<Lua::Vulkan::Semaphore>("Semaphore");
	defVkSemaphore.def(luabind::tostring(luabind::self));
	defVkSemaphore.def("IsValid",&Lua::Vulkan::VKSemaphore::IsValid);
	prosperMod[defVkSemaphore];
	
	auto defVkMemory = luabind::class_<Lua::Vulkan::Memory>("Memory");
	defVkMemory.def(luabind::tostring(luabind::self));
	defVkMemory.def("IsValid",&Lua::Vulkan::VKMemory::IsValid);
	defVkMemory.def("GetParentMemory",&Lua::Vulkan::VKMemory::GetParentMemory);
	defVkMemory.def("GetSize",&Lua::Vulkan::VKMemory::GetSize);
	defVkMemory.def("GetStartOffset",&Lua::Vulkan::VKMemory::GetStartOffset);
	defVkMemory.def("Write",&Lua::Vulkan::VKMemory::Write);
	defVkMemory.def("Write",static_cast<void(*)(lua_State*,Lua::Vulkan::Memory&,uint32_t,::DataStream&)>([](lua_State *l,Lua::Vulkan::Memory &hMemory,uint32_t offset,::DataStream &ds) {
		Lua::Vulkan::VKMemory::Write(l,hMemory,offset,ds,0u,ds->GetSize());
	}));
	defVkMemory.def("Read",static_cast<void(*)(lua_State*,Lua::Vulkan::Memory&,uint32_t,uint32_t,::DataStream&,uint32_t)>(&Lua::Vulkan::VKMemory::Read));
	defVkMemory.def("Read",static_cast<void(*)(lua_State*,Lua::Vulkan::Memory&,uint32_t,uint32_t,::DataStream&)>([](lua_State *l,Lua::Vulkan::Memory &hMemory,uint32_t offset,uint32_t size,::DataStream &ds) {
		Lua::Vulkan::VKMemory::Read(l,hMemory,offset,size,ds,0u);
	}));
	defVkMemory.def("Read",static_cast<void(*)(lua_State*,Lua::Vulkan::Memory&,uint32_t,uint32_t)>(&Lua::Vulkan::VKMemory::Read));
	defVkMemory.def("Read",static_cast<void(*)(lua_State*,Lua::Vulkan::Memory&)>([](lua_State *l,Lua::Vulkan::Memory &hMemory) {
		Lua::Vulkan::VKMemory::Read(l,hMemory,0u,hMemory.get_create_info_ptr()->get_size());
	}));
	defVkMemory.def("Map",&Lua::Vulkan::VKMemory::Map);
	defVkMemory.def("Map",static_cast<void(*)(lua_State*,Lua::Vulkan::Memory&)>([](lua_State *l,Lua::Vulkan::Memory &hMemory) {
		Lua::Vulkan::VKMemory::Map(l,hMemory,0u,hMemory.get_create_info_ptr()->get_size());
	}));
	defVkMemory.def("Unmap",&Lua::Vulkan::VKMemory::Unmap);
	prosperMod[defVkMemory];
#endif
	auto defVkCommandBuffer = luabind::class_<Lua::Vulkan::CommandBuffer>("CommandBuffer");
	defVkCommandBuffer.add_static_constant("RENDER_PASS_FLAG_NONE", umath::to_integral(prosper::IPrimaryCommandBuffer::RenderPassFlags::None));
	defVkCommandBuffer.add_static_constant("RENDER_PASS_FLAG_SECONDARY_COMMAND_BUFFERS_BIT", umath::to_integral(prosper::IPrimaryCommandBuffer::RenderPassFlags::SecondaryCommandBuffers));
	defVkCommandBuffer.def(luabind::tostring(luabind::self));
	defVkCommandBuffer.def(luabind::const_self == luabind::const_self);
	defVkCommandBuffer.def("IsRecording", &Lua::Vulkan::CommandBuffer::IsRecording);
	defVkCommandBuffer.def("RecordClearImage", static_cast<bool (*)(lua_State *, Lua::Vulkan::CommandBuffer &, Lua::Vulkan::Image &, const Color &, const prosper::util::ClearImageInfo &)>(&Lua::Vulkan::VKCommandBuffer::RecordClearImage));
	defVkCommandBuffer.def("RecordClearImage", static_cast<bool (*)(lua_State *, Lua::Vulkan::CommandBuffer &, Lua::Vulkan::Image &, const Color &)>([](lua_State *l, Lua::Vulkan::CommandBuffer &cmdBuffer, Lua::Vulkan::Image &img, const Color &col) {
		return Lua::Vulkan::VKCommandBuffer::RecordClearImage(l, cmdBuffer, img, col);
	}));
	defVkCommandBuffer.def("RecordClearImage", static_cast<bool (*)(lua_State *, Lua::Vulkan::CommandBuffer &, Lua::Vulkan::Image &, std::optional<float>, std::optional<uint32_t>, const prosper::util::ClearImageInfo &)>(&Lua::Vulkan::VKCommandBuffer::RecordClearImage));
	defVkCommandBuffer.def(
	  "RecordClearImage", +[](lua_State *l, Lua::Vulkan::CommandBuffer &cmdBuffer, Lua::Vulkan::Image &img, std::optional<float> clearDepth, std::optional<uint32_t> clearStencil) { return Lua::Vulkan::VKCommandBuffer::RecordClearImage(l, cmdBuffer, img, clearDepth, clearStencil); });
	defVkCommandBuffer.def("RecordClearAttachment", static_cast<bool (*)(lua_State *, Lua::Vulkan::CommandBuffer &, Lua::Vulkan::Image &, const Color &, uint32_t)>(&Lua::Vulkan::VKCommandBuffer::RecordClearAttachment));
	defVkCommandBuffer.def("RecordClearAttachment", static_cast<bool (*)(lua_State *, Lua::Vulkan::CommandBuffer &, Lua::Vulkan::Image &, std::optional<float>, std::optional<uint32_t>)>(&Lua::Vulkan::VKCommandBuffer::RecordClearAttachment));
	defVkCommandBuffer.def(
	  "RecordClearAttachment", +[](lua_State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, Lua::Vulkan::Image &img, std::optional<float> clearDepth) { return Lua::Vulkan::VKCommandBuffer::RecordClearAttachment(l, hCommandBuffer, img, clearDepth, std::optional<uint32_t> {}); });
	defVkCommandBuffer.def("RecordCopyImage", &Lua::Vulkan::VKCommandBuffer::RecordCopyImage);
	defVkCommandBuffer.def("RecordCopyBufferToImage", &Lua::Vulkan::VKCommandBuffer::RecordCopyBufferToImage);
	defVkCommandBuffer.def("RecordCopyBufferToImage", static_cast<bool (*)(lua_State *, Lua::Vulkan::CommandBuffer &, Lua::Vulkan::Buffer &, Lua::Vulkan::Image &)>([](lua_State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, Lua::Vulkan::Buffer &bufSrc, Lua::Vulkan::Image &imgDst) {
		return Lua::Vulkan::VKCommandBuffer::RecordCopyBufferToImage(l, hCommandBuffer, bufSrc, imgDst, {});
	}));
	defVkCommandBuffer.def("RecordCopyBuffer", &Lua::Vulkan::VKCommandBuffer::RecordCopyBuffer);
	defVkCommandBuffer.def("RecordUpdateBuffer", static_cast<bool (*)(lua_State *, Lua::Vulkan::CommandBuffer &, Lua::Vulkan::Buffer &, uint32_t, ::DataStream &)>(&Lua::Vulkan::VKCommandBuffer::RecordUpdateBuffer));
	defVkCommandBuffer.def("RecordUpdateBuffer", static_cast<bool (*)(lua_State *, Lua::Vulkan::CommandBuffer &, Lua::Vulkan::Buffer &, uint32_t, ::udm::Type, Lua::udm_ng)>(&Lua::Vulkan::VKCommandBuffer::RecordUpdateBuffer));
	defVkCommandBuffer.def("RecordBlitImage", &Lua::Vulkan::VKCommandBuffer::RecordBlitImage);
	defVkCommandBuffer.def("RecordResolveImage", &Lua::Vulkan::VKCommandBuffer::RecordResolveImage);
	defVkCommandBuffer.def("RecordBlitTexture", &Lua::Vulkan::VKCommandBuffer::RecordBlitTexture);
	defVkCommandBuffer.def("RecordGenerateMipmaps", &Lua::Vulkan::VKCommandBuffer::RecordGenerateMipmaps);
	defVkCommandBuffer.def("RecordPipelineBarrier", &Lua::Vulkan::VKCommandBuffer::RecordPipelineBarrier);
	defVkCommandBuffer.def("RecordImageBarrier", static_cast<bool (*)(lua_State *, Lua::Vulkan::CommandBuffer &, Lua::Vulkan::Image &, uint32_t, uint32_t, const prosper::util::ImageSubresourceRange &)>(&Lua::Vulkan::VKCommandBuffer::RecordImageBarrier));
	defVkCommandBuffer.def("RecordImageBarrier", static_cast<bool (*)(lua_State *, Lua::Vulkan::CommandBuffer &, Lua::Vulkan::Image &, uint32_t, uint32_t)>([](lua_State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, Lua::Vulkan::Image &img, uint32_t oldLayout, uint32_t newLayout) {
		return Lua::Vulkan::VKCommandBuffer::RecordImageBarrier(l, hCommandBuffer, img, oldLayout, newLayout, prosper::util::ImageSubresourceRange {});
	}));
	defVkCommandBuffer.def("RecordImageBarrier", static_cast<bool (*)(lua_State *, Lua::Vulkan::CommandBuffer &, Lua::Vulkan::Image &, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t)>(&Lua::Vulkan::VKCommandBuffer::RecordImageBarrier));
	defVkCommandBuffer.def("RecordImageBarrier",
	  static_cast<bool (*)(lua_State *, Lua::Vulkan::CommandBuffer &, Lua::Vulkan::Image &, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t)>(
	    [](lua_State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, Lua::Vulkan::Image &img, uint32_t srcStageMask, uint32_t dstStageMask, uint32_t oldLayout, uint32_t newLayout, uint32_t srcAccessMask, uint32_t dstAccessMask) {
		    return Lua::Vulkan::VKCommandBuffer::RecordImageBarrier(l, hCommandBuffer, img, srcStageMask, dstStageMask, oldLayout, newLayout, srcAccessMask, dstAccessMask, 0u);
	    }));
	defVkCommandBuffer.def("RecordBufferBarrier", &Lua::Vulkan::VKCommandBuffer::RecordBufferBarrier);
	defVkCommandBuffer.def("RecordBufferBarrier",
	  static_cast<bool (*)(lua_State *, Lua::Vulkan::CommandBuffer &, Lua::Vulkan::Buffer &, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t)>(
	    [](lua_State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, Lua::Vulkan::Buffer &buf, uint32_t srcStageMask, uint32_t dstStageMask, uint32_t srcAccessMask, uint32_t dstAccessMask, uint32_t offset) {
		    return Lua::Vulkan::VKCommandBuffer::RecordBufferBarrier(l, hCommandBuffer, buf, srcStageMask, dstStageMask, srcAccessMask, dstAccessMask, offset, std::numeric_limits<uint32_t>::max());
	    }));
	defVkCommandBuffer.def("RecordBufferBarrier",
	  static_cast<bool (*)(lua_State *, Lua::Vulkan::CommandBuffer &, Lua::Vulkan::Buffer &, uint32_t, uint32_t, uint32_t, uint32_t)>(
	    [](lua_State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, Lua::Vulkan::Buffer &buf, uint32_t srcStageMask, uint32_t dstStageMask, uint32_t srcAccessMask, uint32_t dstAccessMask) {
		    return Lua::Vulkan::VKCommandBuffer::RecordBufferBarrier(l, hCommandBuffer, buf, srcStageMask, dstStageMask, srcAccessMask, dstAccessMask, 0u, std::numeric_limits<uint32_t>::max());
	    }));
	defVkCommandBuffer.def("RecordSetViewport", &Lua::Vulkan::VKCommandBuffer::RecordSetViewport);
	defVkCommandBuffer.def("RecordSetScissor", &Lua::Vulkan::VKCommandBuffer::RecordSetScissor);
	defVkCommandBuffer.def("RecordBeginRenderPass", static_cast<bool (*)(lua_State *, Lua::Vulkan::CommandBuffer &, Lua::Vulkan::RenderPassInfo &)>([](lua_State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, Lua::Vulkan::RenderPassInfo &rpInfo) {
		return Lua::Vulkan::VKCommandBuffer::RecordBeginRenderPass(l, hCommandBuffer, rpInfo);
	}));
	defVkCommandBuffer.def("RecordEndRenderPass", &Lua::Vulkan::VKCommandBuffer::RecordEndRenderPass);
	defVkCommandBuffer.def("RecordBindIndexBuffer", &Lua::Vulkan::VKCommandBuffer::RecordBindIndexBuffer);
	defVkCommandBuffer.def("RecordBindIndexBuffer", static_cast<bool (*)(lua_State *, Lua::Vulkan::CommandBuffer &, Lua::Vulkan::Buffer &, uint32_t)>([](lua_State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, Lua::Vulkan::Buffer &indexBuffer, uint32_t indexType) {
		return Lua::Vulkan::VKCommandBuffer::RecordBindIndexBuffer(l, hCommandBuffer, indexBuffer, indexType, 0u);
	}));
	defVkCommandBuffer.def("RecordBindVertexBuffer", &Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffer);
	defVkCommandBuffer.def("RecordBindVertexBuffer",
	  static_cast<bool (*)(lua_State *, Lua::Vulkan::CommandBuffer &, prosper::ShaderGraphics &, Lua::Vulkan::Buffer &, uint32_t)>(
	    [](lua_State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, prosper::ShaderGraphics &graphics, Lua::Vulkan::Buffer &vertexBuffer, uint32_t startBinding) { return Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffer(l, hCommandBuffer, graphics, vertexBuffer, startBinding, 0u); }));
	defVkCommandBuffer.def("RecordBindVertexBuffer",
	  static_cast<bool (*)(lua_State *, Lua::Vulkan::CommandBuffer &, prosper::ShaderGraphics &, Lua::Vulkan::Buffer &)>(
	    [](lua_State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, prosper::ShaderGraphics &graphics, Lua::Vulkan::Buffer &vertexBuffer) { return Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffer(l, hCommandBuffer, graphics, vertexBuffer, 0u, 0u); }));
	defVkCommandBuffer.def("RecordBindVertexBuffers", static_cast<bool (*)(lua_State *, Lua::Vulkan::CommandBuffer &, prosper::ShaderGraphics &, const std::vector<prosper::IBuffer *> &, uint32_t, const std::vector<uint64_t> &)>(&Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffers));
	defVkCommandBuffer.def("RecordBindVertexBuffers", static_cast<bool (*)(lua_State *, Lua::Vulkan::CommandBuffer &, prosper::ShaderGraphics &, const std::vector<prosper::IBuffer *> &, uint32_t)>(&Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffers));
	defVkCommandBuffer.def("RecordBindVertexBuffers", static_cast<bool (*)(lua_State *, Lua::Vulkan::CommandBuffer &, prosper::ShaderGraphics &, const std::vector<prosper::IBuffer *> &)>(&Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffers));
#if 0
bool Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffers(
	lua_State *l,CommandBuffer &hCommandBuffer,prosper::ShaderGraphics &graphics,const std::vector<prosper::IBuffer*> &buffers,uint32_t startBinding
)
bool Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffers(
	lua_State *l,CommandBuffer &hCommandBuffer,prosper::ShaderGraphics &graphics,const std::vector<prosper::IBuffer*> &buffers,uint32_t startBinding,const std::vector<uint64_t> &voffsets
)
#endif
	defVkCommandBuffer.def("RecordCopyImageToBuffer", &Lua::Vulkan::VKCommandBuffer::RecordCopyImageToBuffer);
#if 0
	defVkCommandBuffer.def("RecordDispatch",&Lua::Vulkan::VKCommandBuffer::RecordDispatch);
	defVkCommandBuffer.def("RecordDispatch",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,uint32_t,uint32_t)>([](lua_State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer,uint32_t x,uint32_t y) {
		Lua::Vulkan::VKCommandBuffer::RecordDispatch(l,hCommandBuffer,x,y,0u);
	}));
	defVkCommandBuffer.def("RecordDispatch",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,uint32_t)>([](lua_State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer,uint32_t x) {
		Lua::Vulkan::VKCommandBuffer::RecordDispatch(l,hCommandBuffer,x,0u,0u);
	}));
	defVkCommandBuffer.def("RecordDispatch",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&)>([](lua_State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer) {
		Lua::Vulkan::VKCommandBuffer::RecordDispatch(l,hCommandBuffer,0u,0u,0u);
	}));
	defVkCommandBuffer.def("RecordDispatchIndirect",&Lua::Vulkan::VKCommandBuffer::RecordDispatchIndirect);
	defVkCommandBuffer.def("RecordDispatchIndirect",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,Lua::Vulkan::Buffer&)>([](lua_State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer,Lua::Vulkan::Buffer &buffer) {
		Lua::Vulkan::VKCommandBuffer::RecordDispatchIndirect(l,hCommandBuffer,buffer,0u);
	}));
#endif
	defVkCommandBuffer.def("RecordDraw",
	  static_cast<bool (*)(lua_State *, Lua::Vulkan::CommandBuffer &, uint32_t, uint32_t, uint32_t, uint32_t)>([](lua_State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
		  return Lua::Vulkan::VKCommandBuffer::RecordDraw(l, hCommandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
	  }));
	defVkCommandBuffer.def("RecordDraw", static_cast<bool (*)(lua_State *, Lua::Vulkan::CommandBuffer &, uint32_t, uint32_t, uint32_t)>([](lua_State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex) {
		return Lua::Vulkan::VKCommandBuffer::RecordDraw(l, hCommandBuffer, vertexCount, instanceCount, firstVertex, 0u);
	}));
	defVkCommandBuffer.def("RecordDraw", static_cast<bool (*)(lua_State *, Lua::Vulkan::CommandBuffer &, uint32_t, uint32_t)>([](lua_State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, uint32_t vertexCount, uint32_t instanceCount) {
		return Lua::Vulkan::VKCommandBuffer::RecordDraw(l, hCommandBuffer, vertexCount, instanceCount, 0u, 0u);
	}));
	defVkCommandBuffer.def("RecordDraw",
	  static_cast<bool (*)(lua_State *, Lua::Vulkan::CommandBuffer &, uint32_t)>([](lua_State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, uint32_t vertexCount) { return Lua::Vulkan::VKCommandBuffer::RecordDraw(l, hCommandBuffer, vertexCount, 1u, 0u, 0u); }));
	defVkCommandBuffer.def("RecordDrawIndexed", &Lua::Vulkan::VKCommandBuffer::RecordDrawIndexed);
	defVkCommandBuffer.def("RecordDrawIndexed", static_cast<bool (*)(lua_State *, Lua::Vulkan::CommandBuffer &, uint32_t, uint32_t, uint32_t)>([](lua_State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex) {
		return Lua::Vulkan::VKCommandBuffer::RecordDrawIndexed(l, hCommandBuffer, indexCount, instanceCount, firstIndex, 0u);
	}));
	defVkCommandBuffer.def("RecordDrawIndexed", static_cast<bool (*)(lua_State *, Lua::Vulkan::CommandBuffer &, uint32_t, uint32_t)>([](lua_State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, uint32_t indexCount, uint32_t instanceCount) {
		return Lua::Vulkan::VKCommandBuffer::RecordDrawIndexed(l, hCommandBuffer, indexCount, instanceCount, 0u, 0u);
	}));
	defVkCommandBuffer.def("RecordDrawIndexed",
	  static_cast<bool (*)(lua_State *, Lua::Vulkan::CommandBuffer &, uint32_t)>([](lua_State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, uint32_t indexCount) { return Lua::Vulkan::VKCommandBuffer::RecordDrawIndexed(l, hCommandBuffer, indexCount, 1u, 0u, 0u); }));
	defVkCommandBuffer.def("RecordDrawIndexedIndirect", &Lua::Vulkan::VKCommandBuffer::RecordDrawIndexedIndirect);
	defVkCommandBuffer.def("RecordDrawIndirect", &Lua::Vulkan::VKCommandBuffer::RecordDrawIndirect);
	defVkCommandBuffer.def("RecordFillBuffer", &Lua::Vulkan::VKCommandBuffer::RecordFillBuffer);
	//defVkCommandBuffer.def("RecordResetEvent",&Lua::Vulkan::VKCommandBuffer::RecordResetEvent);
	defVkCommandBuffer.def("RecordSetBlendConstants", &Lua::Vulkan::VKCommandBuffer::RecordSetBlendConstants);
	defVkCommandBuffer.def("RecordSetDepthBias", &Lua::Vulkan::VKCommandBuffer::RecordSetDepthBias);
	defVkCommandBuffer.def("RecordSetDepthBounds", &Lua::Vulkan::VKCommandBuffer::RecordSetDepthBounds);
	//defVkCommandBuffer.def("RecordSetEvent",&Lua::Vulkan::VKCommandBuffer::RecordSetEvent);
	defVkCommandBuffer.def("RecordSetLineWidth", &Lua::Vulkan::VKCommandBuffer::RecordSetLineWidth);
	defVkCommandBuffer.def("RecordSetStencilCompareMask", &Lua::Vulkan::VKCommandBuffer::RecordSetStencilCompareMask);
	defVkCommandBuffer.def("RecordSetStencilReference", &Lua::Vulkan::VKCommandBuffer::RecordSetStencilReference);
	defVkCommandBuffer.def("RecordSetStencilWriteMask", &Lua::Vulkan::VKCommandBuffer::RecordSetStencilWriteMask);
	//defVkCommandBuffer.def("RecordWaitEvents",&Lua::Vulkan::VKCommandBuffer::RecordWaitEvents); // TODO
	//defVkCommandBuffer.def("RecordWriteTimestamp",&Lua::Vulkan::VKCommandBuffer::RecordWriteTimestamp); // TODO
	//defVkCommandBuffer.def("RecordBeginQuery",&Lua::Vulkan::VKCommandBuffer::RecordBeginQuery); // TODO
	//defVkCommandBuffer.def("RecordEndQuery",&Lua::Vulkan::VKCommandBuffer::RecordEndQuery); // TODO
	defVkCommandBuffer.def("RecordDrawGradient", &Lua::Vulkan::VKCommandBuffer::RecordDrawGradient);
	defVkCommandBuffer.def("StopRecording", &Lua::Vulkan::CommandBuffer::StopRecording);
	defVkCommandBuffer.def("StartRecording", &Lua::Vulkan::VKCommandBuffer::StartRecording);
	defVkCommandBuffer.def("SetDebugName", static_cast<void (*)(lua_State *, Lua::Vulkan::CommandBuffer &, const std::string &)>([](lua_State *l, Lua::Vulkan::CommandBuffer &cb, const std::string &name) { Lua::Vulkan::VKContextObject::SetDebugName(l, cb, name); }));
	defVkCommandBuffer.def("GetDebugName", static_cast<std::string (*)(lua_State *, Lua::Vulkan::CommandBuffer &)>([](lua_State *l, Lua::Vulkan::CommandBuffer &cb) { return Lua::Vulkan::VKContextObject::GetDebugName(l, cb); }));
	defVkCommandBuffer.def("Flush", static_cast<void (*)(lua_State *, Lua::Vulkan::CommandBuffer &)>([](lua_State *l, Lua::Vulkan::CommandBuffer &cb) { c_engine->GetRenderContext().FlushCommandBuffer(cb); }));
	prosperMod[defVkCommandBuffer];

	auto devVkBuffer = luabind::class_<Lua::Vulkan::Buffer>("Buffer");
	devVkBuffer.def(luabind::tostring(luabind::self));
	devVkBuffer.def(luabind::const_self == luabind::const_self);
	devVkBuffer.def("IsValid", &Lua::Vulkan::VKBuffer::IsValid);
	devVkBuffer.def("GetStartOffset", &Lua::Vulkan::Buffer::GetStartOffset);
	devVkBuffer.def("GetBaseIndex", &Lua::Vulkan::Buffer::GetBaseIndex);
	devVkBuffer.def("GetSize", &Lua::Vulkan::Buffer::GetSize);
	devVkBuffer.def("GetUsageFlags", &Lua::Vulkan::Buffer::GetUsageFlags);
	devVkBuffer.def("SetDebugName", static_cast<void (*)(lua_State *, Lua::Vulkan::Buffer &, const std::string &)>([](lua_State *l, Lua::Vulkan::Buffer &buf, const std::string &name) { Lua::Vulkan::VKContextObject::SetDebugName(l, buf, name); }));
	devVkBuffer.def("GetDebugName", static_cast<std::string (*)(lua_State *, Lua::Vulkan::Buffer &)>([](lua_State *l, Lua::Vulkan::Buffer &buf) { return Lua::Vulkan::VKContextObject::GetDebugName(l, buf); }));
	devVkBuffer.def("GetCreateInfo", &Lua::Vulkan::Buffer::GetCreateInfo, luabind::copy_policy<0> {});
	devVkBuffer.def("SetPermanentlyMapped", &Lua::Vulkan::Buffer::SetPermanentlyMapped);
	devVkBuffer.def("GetParent", static_cast<std::shared_ptr<prosper::IBuffer> (prosper::IBuffer::*)()>(&Lua::Vulkan::Buffer::GetParent));
	devVkBuffer.def("WriteMemory", static_cast<bool (*)(lua_State *, Lua::Vulkan::Buffer &, uint32_t, ::DataStream &, uint32_t, uint32_t)>(&Lua::Vulkan::VKBuffer::Write));
	devVkBuffer.def("WriteMemory", static_cast<bool (*)(lua_State *, Lua::Vulkan::Buffer &, uint32_t, ::udm::Type, Lua::udm_ng)>(&Lua::Vulkan::VKBuffer::Write));
	devVkBuffer.def("WriteMemory", static_cast<bool (*)(lua_State *, Lua::Vulkan::Buffer &, uint32_t, ::DataStream &)>([](lua_State *l, Lua::Vulkan::Buffer &hBuffer, uint32_t offset, ::DataStream &ds) { return Lua::Vulkan::VKBuffer::Write(l, hBuffer, offset, ds, 0u, ds->GetSize()); }));
	devVkBuffer.def("ReadMemory", static_cast<bool (*)(lua_State *, Lua::Vulkan::Buffer &, uint32_t, uint32_t, ::DataStream &, uint32_t)>(&Lua::Vulkan::VKBuffer::Read));
	devVkBuffer.def("ReadMemory",
	  static_cast<bool (*)(lua_State *, Lua::Vulkan::Buffer &, uint32_t, uint32_t, ::DataStream &)>([](lua_State *l, Lua::Vulkan::Buffer &hBuffer, uint32_t offset, uint32_t size, ::DataStream &ds) { return Lua::Vulkan::VKBuffer::Read(l, hBuffer, offset, size, ds, 0u); }));
	devVkBuffer.def("ReadMemory", static_cast<bool (*)(lua_State *, Lua::Vulkan::Buffer &, uint32_t, ::udm::Type, Lua::udm_ng)>(&Lua::Vulkan::VKBuffer::Read));
	devVkBuffer.def("ReadMemory", static_cast<Lua::opt<::DataStream> (*)(lua_State *, Lua::Vulkan::Buffer &, uint32_t, uint32_t)>(&Lua::Vulkan::VKBuffer::Read));
	devVkBuffer.def("ReadMemory", static_cast<Lua::opt<::DataStream> (*)(lua_State *, Lua::Vulkan::Buffer &)>([](lua_State *l, Lua::Vulkan::Buffer &hBuffer) { return Lua::Vulkan::VKBuffer::Read(l, hBuffer, 0u, hBuffer.GetSize()); }));
	devVkBuffer.def("MapMemory", &Lua::Vulkan::VKBuffer::Map);
	devVkBuffer.def("MapMemory",
	  static_cast<bool (*)(lua_State *, Lua::Vulkan::Buffer &, Lua::Vulkan::Buffer::MapFlags)>([](lua_State *l, Lua::Vulkan::Buffer &hBuffer, Lua::Vulkan::Buffer::MapFlags mapFlags) { return Lua::Vulkan::VKBuffer::Map(l, hBuffer, 0u, hBuffer.GetSize(), mapFlags); }));
	devVkBuffer.def("UnmapMemory", &Lua::Vulkan::Buffer::Unmap);
	prosperMod[devVkBuffer];

	auto devVkSwapBuffer = luabind::class_<Lua::Vulkan::SwapBuffer>("SwapBuffer");
	prosperMod[devVkSwapBuffer];

	auto defVkDescriptorSet = luabind::class_<Lua::Vulkan::DescriptorSet>("DescriptorSet");
	defVkDescriptorSet.def(luabind::tostring(luabind::self));
	defVkDescriptorSet.def(luabind::const_self == luabind::const_self);
#if 0
	defVkDescriptorSet.def("GetBindingInfo",static_cast<void(*)(lua_State*,Lua::Vulkan::DescriptorSet&,uint32_t)>(&Lua::Vulkan::VKDescriptorSet::GetBindingInfo));
	defVkDescriptorSet.def("GetBindingInfo",static_cast<void(*)(lua_State*,Lua::Vulkan::DescriptorSet&)>(&Lua::Vulkan::VKDescriptorSet::GetBindingInfo));
#endif
	defVkDescriptorSet.def("GetBindingCount", &Lua::Vulkan::DescriptorSet::GetBindingCount);
	defVkDescriptorSet.def("SetBindingTexture", static_cast<bool (*)(lua_State *, Lua::Vulkan::DescriptorSet &, uint32_t, Lua::Vulkan::Texture &, uint32_t)>([](lua_State *l, Lua::Vulkan::DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Texture &texture, uint32_t layerId) {
		return Lua::Vulkan::VKDescriptorSet::SetBindingTexture(l, hDescSet, bindingIdx, texture, layerId);
	}));
	defVkDescriptorSet.def("SetBindingTexture", static_cast<bool (*)(lua_State *, Lua::Vulkan::DescriptorSet &, uint32_t, Lua::Vulkan::Texture &)>([](lua_State *l, Lua::Vulkan::DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Texture &texture) {
		return Lua::Vulkan::VKDescriptorSet::SetBindingTexture(l, hDescSet, bindingIdx, texture);
	}));
	defVkDescriptorSet.def("SetBindingArrayTexture", static_cast<bool (*)(lua_State *, Lua::Vulkan::DescriptorSet &, uint32_t, Lua::Vulkan::Texture &, uint32_t, uint32_t)>(&Lua::Vulkan::VKDescriptorSet::SetBindingArrayTexture));
	defVkDescriptorSet.def("SetBindingArrayTexture", static_cast<bool (*)(lua_State *, Lua::Vulkan::DescriptorSet &, uint32_t, Lua::Vulkan::Texture &, uint32_t)>([](lua_State *l, Lua::Vulkan::DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Texture &texture, uint32_t arrayIdx) {
		return Lua::Vulkan::VKDescriptorSet::SetBindingArrayTexture(l, hDescSet, bindingIdx, texture, arrayIdx);
	}));
	defVkDescriptorSet.def("SetBindingUniformBuffer", &Lua::Vulkan::VKDescriptorSet::SetBindingUniformBuffer);
	defVkDescriptorSet.def("SetBindingUniformBuffer", static_cast<bool (*)(lua_State *, Lua::Vulkan::DescriptorSet &, uint32_t, Lua::Vulkan::Buffer &, uint32_t)>([](lua_State *l, Lua::Vulkan::DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Buffer &buffer, uint32_t startOffset) {
		return Lua::Vulkan::VKDescriptorSet::SetBindingUniformBuffer(l, hDescSet, bindingIdx, buffer, startOffset, std::numeric_limits<uint32_t>::max());
	}));
	defVkDescriptorSet.def("SetBindingUniformBuffer", static_cast<bool (*)(lua_State *, Lua::Vulkan::DescriptorSet &, uint32_t, Lua::Vulkan::Buffer &)>([](lua_State *l, Lua::Vulkan::DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Buffer &buffer) {
		return Lua::Vulkan::VKDescriptorSet::SetBindingUniformBuffer(l, hDescSet, bindingIdx, buffer, 0u, std::numeric_limits<uint32_t>::max());
	}));
	defVkDescriptorSet.def("SetBindingUniformBufferDynamic", &Lua::Vulkan::VKDescriptorSet::SetBindingUniformBufferDynamic);
	defVkDescriptorSet.def("SetBindingUniformBufferDynamic",
	  static_cast<bool (*)(lua_State *, Lua::Vulkan::DescriptorSet &, uint32_t, Lua::Vulkan::Buffer &, uint32_t)>([](lua_State *l, Lua::Vulkan::DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Buffer &buffer, uint32_t startOffset) {
		  return Lua::Vulkan::VKDescriptorSet::SetBindingUniformBufferDynamic(l, hDescSet, bindingIdx, buffer, startOffset, std::numeric_limits<uint32_t>::max());
	  }));
	defVkDescriptorSet.def("SetBindingUniformBufferDynamic", static_cast<bool (*)(lua_State *, Lua::Vulkan::DescriptorSet &, uint32_t, Lua::Vulkan::Buffer &)>([](lua_State *l, Lua::Vulkan::DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Buffer &buffer) {
		return Lua::Vulkan::VKDescriptorSet::SetBindingUniformBufferDynamic(l, hDescSet, bindingIdx, buffer, 0u, std::numeric_limits<uint32_t>::max());
	}));
	defVkDescriptorSet.def("SetBindingStorageBuffer", &Lua::Vulkan::VKDescriptorSet::SetBindingStorageBuffer);
	defVkDescriptorSet.def("SetBindingStorageBuffer", static_cast<bool (*)(lua_State *, Lua::Vulkan::DescriptorSet &, uint32_t, Lua::Vulkan::Buffer &, uint32_t)>([](lua_State *l, Lua::Vulkan::DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Buffer &buffer, uint32_t startOffset) {
		return Lua::Vulkan::VKDescriptorSet::SetBindingStorageBuffer(l, hDescSet, bindingIdx, buffer, startOffset, std::numeric_limits<uint32_t>::max());
	}));
	defVkDescriptorSet.def("SetBindingStorageBuffer", static_cast<bool (*)(lua_State *, Lua::Vulkan::DescriptorSet &, uint32_t, Lua::Vulkan::Buffer &)>([](lua_State *l, Lua::Vulkan::DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Buffer &buffer) {
		return Lua::Vulkan::VKDescriptorSet::SetBindingStorageBuffer(l, hDescSet, bindingIdx, buffer, 0u, std::numeric_limits<uint32_t>::max());
	}));
	defVkDescriptorSet.def("SetDebugName", static_cast<void (*)(lua_State *, Lua::Vulkan::DescriptorSet &, const std::string &)>([](lua_State *l, Lua::Vulkan::DescriptorSet &ds, const std::string &name) { Lua::Vulkan::VKContextObject::SetDebugName(l, ds, name); }));
	defVkDescriptorSet.def("GetDebugName", static_cast<std::string (*)(lua_State *, Lua::Vulkan::DescriptorSet &)>([](lua_State *l, Lua::Vulkan::DescriptorSet &ds) { return Lua::Vulkan::VKContextObject::GetDebugName(l, ds); }));
	defVkDescriptorSet.def(
	  "Update", +[](lua_State *l, Lua::Vulkan::DescriptorSet &ds) { return ds.GetDescriptorSet()->Update(); });
	prosperMod[defVkDescriptorSet];

	auto defVkMesh = luabind::class_<pragma::SceneMesh>("Mesh");
	defVkMesh.def(luabind::tostring(luabind::self));
	defVkMesh.def(luabind::const_self == luabind::const_self);
	defVkMesh.def("GetVertexBuffer", &pragma::SceneMesh::GetVertexBuffer);
	defVkMesh.def("GetVertexWeightBuffer", &pragma::SceneMesh::GetVertexWeightBuffer);
	defVkMesh.def("GetAlphaBuffer", &pragma::SceneMesh::GetAlphaBuffer);
	defVkMesh.def("GetIndexBuffer", &pragma::SceneMesh::GetIndexBuffer);
	defVkMesh.def("SetVertexBuffer", &pragma::SceneMesh::SetVertexBuffer);
	defVkMesh.def("SetVertexWeightBuffer", &pragma::SceneMesh::SetVertexWeightBuffer);
	defVkMesh.def("SetAlphaBuffer", &pragma::SceneMesh::SetAlphaBuffer);
	defVkMesh.def("SetIndexBuffer", &pragma::SceneMesh::SetIndexBuffer);
	defVkMesh.def("ClearBuffers", static_cast<void (*)(lua_State *, pragma::SceneMesh &)>([](lua_State *l, pragma::SceneMesh &mesh) { mesh.ClearBuffers(); }));
	prosperMod[defVkMesh];

	auto defPcb = luabind::class_<prosper::util::PreparedCommandBuffer>("PreparedCommandBuffer");
	defPcb.def(
	  "RecordSetScissor", +[](prosper::util::PreparedCommandBuffer &pcb, const PcbLuaArg &width, const PcbLuaArg &height, const PcbLuaArg &x, const PcbLuaArg &y) {
		  pcb.PushCommand(
		    [](const prosper::util::PreparedCommandBufferRecordState &recordState) -> bool {
			    auto &cmdBuf = recordState.commandBuffer;
			    return cmdBuf.RecordSetScissor(recordState.GetArgument<uint32_t>(0), recordState.GetArgument<uint32_t>(1), recordState.GetArgument<uint32_t>(2), recordState.GetArgument<uint32_t>(3));
		    },
		    std::move(util::make_vector<PcbArg>(make_pcb_arg(width, udm::Type::UInt32), make_pcb_arg(height, udm::Type::UInt32), make_pcb_arg(x, udm::Type::UInt32), make_pcb_arg(y, udm::Type::UInt32))));
	  });
	defPcb.def("RecordBufferBarrier", &pcb_recordBufferBarrier);
	defPcb.def(
	  "RecordBufferBarrier", +[](lua_State *l, prosper::util::PreparedCommandBuffer &pcb, Lua::Vulkan::Buffer &buf, const PcbLuaArg &srcStageMask, const PcbLuaArg &dstStageMask, const PcbLuaArg &srcAccessMask, const PcbLuaArg &dstAccessMask) {
		  pcb_recordBufferBarrier(pcb, buf, srcStageMask, dstStageMask, srcAccessMask, dstAccessMask, PcbLuaArg::CreateValue<uint32_t>(l, 0), PcbLuaArg::CreateValue<uint32_t>(l, buf.GetSize()));
	  });
	defPcb.def(
	  "RecordSetStencilReference", +[](prosper::util::PreparedCommandBuffer &pcb, const Lua::Vulkan::PreparedCommandLuaArg &faceMask, const Lua::Vulkan::PreparedCommandLuaArg &stencilReference) {
		  pcb.PushCommand(
		    [](const prosper::util::PreparedCommandBufferRecordState &recordState) -> bool {
			    auto &cmdBuf = recordState.commandBuffer;
			    return cmdBuf.RecordSetStencilReference(static_cast<prosper::StencilFaceFlags>(recordState.GetArgument<uint8_t>(0)), recordState.GetArgument<uint32_t>(1));
		    },
		    std::move(util::make_vector<PcbArg>(make_pcb_arg(faceMask, udm::Type::UInt8), make_pcb_arg(stencilReference, udm::Type::UInt32))));
	  });
	defPcb.def(
	  "RecordSetLineWidth", +[](prosper::util::PreparedCommandBuffer &pcb, const Lua::Vulkan::PreparedCommandLuaArg &lineWidth) {
		  pcb.PushCommand(
		    [](const prosper::util::PreparedCommandBufferRecordState &recordState) -> bool {
			    auto &cmdBuf = recordState.commandBuffer;
			    return cmdBuf.RecordSetLineWidth(recordState.GetArgument<float>(0));
		    },
		    std::move(util::make_vector<PcbArg>(make_pcb_arg(lineWidth, udm::Type::Float))));
	  });
	defPcb.def(
	  "RecordClearImage", +[](prosper::util::PreparedCommandBuffer &pcb, prosper::IImage &img, const Color &color) -> bool {
		  pcb.PushCommand([&img, color](const prosper::util::PreparedCommandBufferRecordState &recordState) -> bool {
			  auto &cmdBuf = recordState.commandBuffer;
			  auto vCol = color.ToVector4();
			  return cmdBuf.RecordClearImage(img, prosper::ImageLayout::TransferDstOptimal, std::array<float, 4> {vCol[0], vCol[1], vCol[2], vCol[3]});
		  });
		  return true;
	  });
	defPcb.def("RecordBindDescriptorSet", &pcb_record_bind_descriptor_set);
	defPcb.def(
	  "RecordBindDescriptorSet",
	  +[](lua_State *l, prosper::util::PreparedCommandBuffer &pcb, const std::shared_ptr<prosper::IDescriptorSetGroup> &descSet, const PcbLuaArg &firstSet) -> bool { return pcb_record_bind_descriptor_set(pcb, descSet, firstSet, PcbLuaArg::CreateValue<uint32_t>(l, 0)); });
	defPcb.def(
	  "RecordBindDescriptorSet",
	  +[](lua_State *l, prosper::util::PreparedCommandBuffer &pcb, const std::shared_ptr<prosper::IDescriptorSetGroup> &descSet) -> bool { return pcb_record_bind_descriptor_set(pcb, descSet, PcbLuaArg::CreateValue<uint32_t>(l, 0), PcbLuaArg::CreateValue<uint32_t>(l, 0)); });
	defPcb.def(
	  "RecordCommands", +[](prosper::util::PreparedCommandBuffer &pcb, prosper::ICommandBuffer &cmd) -> bool { return pcb.RecordCommands(cmd, {}, {}); });
	defPcb.def_readonly("enableDrawArgs", &prosper::util::PreparedCommandBuffer::enableDrawArgs);

	auto defPcbDa = luabind::class_<Lua::Vulkan::PreparedCommandLuaDynamicArg>("DynArg");
	defPcbDa.def(luabind::constructor<std::string>());
	defPcb.scope[defPcbDa];

	prosperMod[defPcb];

	pragma::lua::define_custom_constructor<prosper::util::PreparedCommandBuffer, []() -> std::shared_ptr<prosper::util::PreparedCommandBuffer> { return std::make_shared<prosper::util::PreparedCommandBuffer>(); }>(lua.GetState());
}

/////////////////////////////////

bool Lua::Vulkan::VKFramebuffer::IsValid(lua_State *l, Framebuffer &hFramebuffer) { return true; }
#if 0
void Lua::Vulkan::VKFramebuffer::GetAttachment(lua_State *l,Framebuffer &hFramebuffer,uint32_t attId)
{
	Anvil::ImageView *imgView;
	if(hFramebuffer->get_create_info_ptr()->get_attachment_at_index(attId,&imgView) == false || imgView == nullptr)
		return;
	// prosper TODO
}
void Lua::Vulkan::VKFramebuffer::GetAttachmentCount(lua_State *l,Framebuffer &hFramebuffer)
{
	Lua::PushInt(l,hFramebuffer.GetAttachmentCount());
}
#endif

/////////////////////////////////

bool Lua::Vulkan::VKRenderPass::IsValid(lua_State *l, RenderPass &hRenderPass) { return true; }
#if 0
void Lua::Vulkan::VKRenderPass::GetInfo(lua_State *l,RenderPass &hRenderPass)
{
	auto *info = hRenderPass->get_render_pass_create_info(); // prosper TODO

}
void Lua::Vulkan::VKRenderPass::GetSwapchain(lua_State *l,RenderPass &hRenderPass)
{
	auto *swapchain = hRenderPass->get_swapchain(); // prosper TODO
}
#endif

/////////////////////////////////

bool Lua::Vulkan::VKEvent::IsValid(lua_State *l, Event &hEvent) { return true; }
prosper::Result Lua::Vulkan::VKEvent::GetStatus(lua_State *l, Event &hEvent)
{
	auto b = hEvent.IsSet();
	return b ? prosper::Result::EventSet : prosper::Result::EventReset;
}

/////////////////////////////////

bool Lua::Vulkan::VKFence::IsValid(lua_State *l, Fence &hFence) { return true; }

/////////////////////////////////

#if 0
void Lua::Vulkan::VKSemaphore::IsValid(lua_State *l,Semaphore &hSemaphore)
{
	Lua::PushBool(l,true);
}

/////////////////////////////////

void Lua::Vulkan::VKMemory::IsValid(lua_State *l,Memory &hMemory)
{
	Lua::PushBool(l,&hMemory != nullptr);
}
void Lua::Vulkan::VKMemory::GetParentMemory(lua_State *l,Memory &hMemory)
{
	auto *pParentMemory = hMemory.get_create_info_ptr()->get_parent_memory_block();
	if(pParentMemory == nullptr)
		return;
	Lua::Push<Memory*>(l,pParentMemory);
}
void Lua::Vulkan::VKMemory::GetSize(lua_State *l,Memory &hMemory)
{
	Lua::PushInt(l,hMemory.get_create_info_ptr()->get_size());
}
void Lua::Vulkan::VKMemory::GetStartOffset(lua_State *l,Memory &hMemory)
{
	Lua::PushInt(l,hMemory.get_start_offset());
}
void Lua::Vulkan::VKMemory::Write(lua_State *l,Memory &hMemory,uint32_t offset,::DataStream &ds,uint32_t dsOffset,uint32_t dsSize)
{
	Lua::PushBool(l,hMemory.write(offset,dsSize,ds->GetData() +dsOffset));
}
void Lua::Vulkan::VKMemory::Read(lua_State *l,Memory &hMemory,uint32_t offset,uint32_t size)
{
	auto ds = ::DataStream(size);
	if(hMemory.read(offset,size,ds->GetData()) == false)
		return;
	Lua::Push(l,ds);
}
void Lua::Vulkan::VKMemory::Read(lua_State *l,Memory &hMemory,uint32_t offset,uint32_t size,::DataStream &ds,uint32_t dsOffset)
{
	auto reqSize = dsOffset +size;
	if(ds->GetSize() < reqSize)
		ds->Resize(reqSize);
	Lua::PushBool(l,hMemory.read(offset,size,ds->GetData() +dsOffset));
}
void Lua::Vulkan::VKMemory::Map(lua_State *l,Memory &hMemory,uint32_t offset,uint32_t size)
{
	Lua::PushBool(l,hMemory.map(offset,size));
}
void Lua::Vulkan::VKMemory::Unmap(lua_State *l,Memory &hMemory)
{
	Lua::PushBool(l,hMemory.unmap());
}
#endif

/////////////////////////////////

bool Lua::Vulkan::VKCommandBuffer::IsValid(lua_State *l, CommandBuffer &hCommandBuffer) { return true; }
bool Lua::Vulkan::VKCommandBuffer::RecordClearImage(lua_State *l, CommandBuffer &hCommandBuffer, Image &img, const Color &col, const prosper::util::ClearImageInfo &clearImageInfo)
{
	auto vcol = col.ToVector4();
	return hCommandBuffer.RecordClearImage(img, prosper::ImageLayout::TransferDstOptimal, {vcol.r, vcol.g, vcol.b, vcol.a}, clearImageInfo);
}
bool Lua::Vulkan::VKCommandBuffer::RecordClearImage(lua_State *l, CommandBuffer &hCommandBuffer, Image &img, std::optional<float> clearDepth, std::optional<uint32_t> clearStencil, const prosper::util::ClearImageInfo &clearImageInfo)
{
	return hCommandBuffer.RecordClearImage(img, prosper::ImageLayout::TransferDstOptimal, clearDepth, clearStencil, clearImageInfo);
}
bool Lua::Vulkan::VKCommandBuffer::RecordClearAttachment(lua_State *l, CommandBuffer &hCommandBuffer, Image &img, const Color &col, uint32_t attId)
{
	auto vcol = col.ToVector4();
	return hCommandBuffer.RecordClearAttachment(img, {vcol.r, vcol.g, vcol.b, vcol.a}, attId);
}
bool Lua::Vulkan::VKCommandBuffer::RecordClearAttachment(lua_State *l, CommandBuffer &hCommandBuffer, Image &img, std::optional<float> clearDepth, std::optional<uint32_t> clearStencil) { return hCommandBuffer.RecordClearAttachment(img, clearDepth, clearStencil); }
bool Lua::Vulkan::VKCommandBuffer::RecordCopyImage(lua_State *l, CommandBuffer &hCommandBuffer, Image &imgSrc, Image &imgDst, const prosper::util::CopyInfo &copyInfo) { return hCommandBuffer.RecordCopyImage(copyInfo, imgSrc, imgDst); }
bool Lua::Vulkan::VKCommandBuffer::RecordCopyBufferToImage(lua_State *l, CommandBuffer &hCommandBuffer, Buffer &bufSrc, Image &imgDst, const prosper::util::BufferImageCopyInfo &copyInfo) { return hCommandBuffer.RecordCopyBufferToImage(copyInfo, bufSrc, imgDst); }
bool Lua::Vulkan::VKCommandBuffer::RecordCopyBuffer(lua_State *l, CommandBuffer &hCommandBuffer, Buffer &bufSrc, Buffer &bufDst, const prosper::util::BufferCopy &copyInfo) { return hCommandBuffer.RecordCopyBuffer(copyInfo, bufSrc, bufDst); }
bool Lua::Vulkan::VKCommandBuffer::RecordUpdateBuffer(lua_State *l, CommandBuffer &hCommandBuffer, Buffer &buf, uint32_t offset, ::DataStream &ds) { return hCommandBuffer.RecordUpdateBuffer(buf, offset, ds->GetSize(), ds->GetData()); }
bool Lua::Vulkan::VKCommandBuffer::RecordUpdateBuffer(lua_State *l, CommandBuffer &hCommandBuffer, Buffer &buf, uint32_t offset, ::udm::Type type, Lua::udm_ng value)
{
	return udm::visit_ng(type, [&buf, &value, &hCommandBuffer, offset](auto tag) {
		using T = typename decltype(tag)::type;
		auto val = luabind::object_cast<T>(value);
		return hCommandBuffer.RecordUpdateBuffer(buf, offset, sizeof(val), &val);
	});
}
bool Lua::Vulkan::VKCommandBuffer::RecordBlitImage(lua_State *l, CommandBuffer &hCommandBuffer, Image &imgSrc, Image &imgDst, const prosper::util::BlitInfo &blitInfo) { return hCommandBuffer.RecordBlitImage(blitInfo, imgSrc, imgDst); }
bool Lua::Vulkan::VKCommandBuffer::RecordResolveImage(lua_State *l, CommandBuffer &hCommandBuffer, Image &imgSrc, Image &imgDst) { return hCommandBuffer.RecordResolveImage(imgSrc, imgDst); }
bool Lua::Vulkan::VKCommandBuffer::RecordBlitTexture(lua_State *l, CommandBuffer &hCommandBuffer, Texture &texSrc, Image &imgDst) { return hCommandBuffer.RecordBlitTexture(texSrc, imgDst); }
bool Lua::Vulkan::VKCommandBuffer::RecordGenerateMipmaps(lua_State *l, CommandBuffer &hCommandBuffer, Image &img, uint32_t currentLayout, uint32_t srcAccessMask, uint32_t srcStage)
{
	return hCommandBuffer.RecordGenerateMipmaps(img, static_cast<prosper::ImageLayout>(currentLayout), static_cast<prosper::AccessFlags>(srcAccessMask), static_cast<prosper::PipelineStageFlags>(srcStage));
}
bool Lua::Vulkan::VKCommandBuffer::RecordPipelineBarrier(lua_State *l, CommandBuffer &hCommandBuffer, const prosper::util::PipelineBarrierInfo &barrierInfo) { return hCommandBuffer.RecordPipelineBarrier(barrierInfo); }
bool Lua::Vulkan::VKCommandBuffer::RecordImageBarrier(lua_State *l, CommandBuffer &hCommandBuffer, Image &img, uint32_t oldLayout, uint32_t newLayout, const prosper::util::ImageSubresourceRange &subresourceRange)
{
	return hCommandBuffer.RecordImageBarrier(img, static_cast<prosper::ImageLayout>(oldLayout), static_cast<prosper::ImageLayout>(newLayout), subresourceRange);
}
bool Lua::Vulkan::VKCommandBuffer::RecordImageBarrier(lua_State *l, CommandBuffer &hCommandBuffer, Image &img, uint32_t srcStageMask, uint32_t dstStageMask, uint32_t oldLayout, uint32_t newLayout, uint32_t srcAccessMask, uint32_t dstAccessMask, uint32_t baseLayer)
{
	return hCommandBuffer.RecordImageBarrier(img, static_cast<prosper::PipelineStageFlags>(srcStageMask), static_cast<prosper::PipelineStageFlags>(dstStageMask), static_cast<prosper::ImageLayout>(oldLayout), static_cast<prosper::ImageLayout>(newLayout),
	  static_cast<prosper::AccessFlags>(srcAccessMask), static_cast<prosper::AccessFlags>(dstAccessMask), baseLayer);
}
bool Lua::Vulkan::VKCommandBuffer::RecordBufferBarrier(lua_State *l, CommandBuffer &hCommandBuffer, Buffer &buf, uint32_t srcStageMask, uint32_t dstStageMask, uint32_t srcAccessMask, uint32_t dstAccessMask, uint32_t offset, uint32_t size)
{
	auto lsize = (size != std::numeric_limits<uint32_t>::max()) ? static_cast<uint64_t>(size) : std::numeric_limits<uint64_t>::max();
	return hCommandBuffer.RecordBufferBarrier(buf, static_cast<prosper::PipelineStageFlags>(srcStageMask), static_cast<prosper::PipelineStageFlags>(dstStageMask), static_cast<prosper::AccessFlags>(srcAccessMask), static_cast<prosper::AccessFlags>(dstAccessMask), offset, lsize);
}
bool Lua::Vulkan::VKCommandBuffer::RecordSetViewport(lua_State *l, CommandBuffer &hCommandBuffer, uint32_t width, uint32_t height, uint32_t x, uint32_t y) { return hCommandBuffer.RecordSetViewport(width, height, x, y); }
bool Lua::Vulkan::VKCommandBuffer::RecordSetScissor(lua_State *l, CommandBuffer &hCommandBuffer, uint32_t width, uint32_t height, uint32_t x, uint32_t y) { return hCommandBuffer.RecordSetScissor(width, height, x, y); }
bool Lua::Vulkan::VKCommandBuffer::RecordBeginRenderPass(lua_State *l, CommandBuffer &hCommandBuffer, Lua::Vulkan::RenderPassInfo &rpInfo)
{
	if(hCommandBuffer.IsPrimary() == false)
		return false;
	static_assert(sizeof(Lua::Vulkan::ClearValue) == sizeof(prosper::ClearValue));
	auto &primaryCmdBuffer = dynamic_cast<prosper::IPrimaryCommandBuffer &>(hCommandBuffer);
	if(rpInfo.layerId.has_value()) {
		auto r = primaryCmdBuffer.RecordBeginRenderPass(*rpInfo.renderTarget, *rpInfo.layerId, reinterpret_cast<std::vector<prosper::ClearValue> &>(rpInfo.clearValues), rpInfo.renderPassFlags, rpInfo.renderPass.get());
		return r;
	}
	auto r = primaryCmdBuffer.RecordBeginRenderPass(*rpInfo.renderTarget, reinterpret_cast<std::vector<prosper::ClearValue> &>(rpInfo.clearValues), rpInfo.renderPassFlags, rpInfo.renderPass.get());
	return r;
}
bool Lua::Vulkan::VKCommandBuffer::RecordEndRenderPass(lua_State *l, CommandBuffer &hCommandBuffer)
{
	if(hCommandBuffer.IsPrimary() == false)
		return false;
	auto &primaryCmdBuffer = dynamic_cast<prosper::IPrimaryCommandBuffer &>(hCommandBuffer);
	;
	return primaryCmdBuffer.RecordEndRenderPass();
}
bool Lua::Vulkan::VKCommandBuffer::RecordBindIndexBuffer(lua_State *l, CommandBuffer &hCommandBuffer, Buffer &indexBuffer, uint32_t indexType, uint32_t offset) { return hCommandBuffer.RecordBindIndexBuffer(indexBuffer, static_cast<prosper::IndexType>(indexType), offset); }
bool Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffer(lua_State *l, CommandBuffer &hCommandBuffer, prosper::ShaderGraphics &graphics, Buffer &vertexBuffer, uint32_t startBinding, uint32_t offset)
{
	return hCommandBuffer.RecordBindVertexBuffers(graphics, {&vertexBuffer}, startBinding, {static_cast<uint64_t>(offset)});
}
bool Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffers(lua_State *l, CommandBuffer &hCommandBuffer, prosper::ShaderGraphics &graphics, const std::vector<prosper::IBuffer *> &buffers, uint32_t startBinding)
{
	static std::vector<uint64_t> voffsets {0ull};
	return RecordBindVertexBuffers(l, hCommandBuffer, graphics, buffers, startBinding, voffsets);
}
bool Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffers(lua_State *l, CommandBuffer &hCommandBuffer, prosper::ShaderGraphics &graphics, const std::vector<prosper::IBuffer *> &buffers)
{
	static std::vector<uint64_t> voffsets {0ull};
	return RecordBindVertexBuffers(l, hCommandBuffer, graphics, buffers, 0u, voffsets);
}
bool Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffers(lua_State *l, CommandBuffer &hCommandBuffer, prosper::ShaderGraphics &graphics, const std::vector<prosper::IBuffer *> &buffers, uint32_t startBinding, const std::vector<uint64_t> &voffsets)
{
	return hCommandBuffer.RecordBindVertexBuffers(graphics, buffers, startBinding, voffsets);
}
bool Lua::Vulkan::VKCommandBuffer::RecordCopyImageToBuffer(lua_State *l, CommandBuffer &hCommandBuffer, Image &imgSrc, uint32_t srcImageLayout, Buffer &bufDst, const prosper::util::BufferImageCopyInfo &copyInfo)
{
	return hCommandBuffer.RecordCopyImageToBuffer(copyInfo, imgSrc, static_cast<prosper::ImageLayout>(srcImageLayout), bufDst);
}
#if 0
void Lua::Vulkan::VKCommandBuffer::RecordDispatch(lua_State *l,CommandBuffer &hCommandBuffer,uint32_t x,uint32_t y,uint32_t z)
{
	Lua::PushBool(l,hCommandBuffer.RecordDispatch(x,y,z));
}
void Lua::Vulkan::VKCommandBuffer::RecordDispatchIndirect(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &buffer,uint32_t offset)
{
	Lua::PushBool(l,hCommandBuffer.RecordDispatchIndirect(buffer,size);
}
void Lua::Vulkan::VKCommandBuffer::RecordResetEvent(lua_State *l,CommandBuffer &hCommandBuffer,Event &ev,uint32_t stageMask)
{
	Lua::PushBool(l,hCommandBuffer.RecordResetEvent(&ev,static_cast<Anvil::PipelineStageFlagBits>(stageMask)));
}
void Lua::Vulkan::VKCommandBuffer::RecordSetEvent(lua_State *l,CommandBuffer &hCommandBuffer,Event &ev,uint32_t stageMask)
{
	Lua::PushBool(l,hCommandBuffer.RecordSetEvent(&ev,static_cast<Anvil::PipelineStageFlagBits>(stageMask)));
}
#endif
bool Lua::Vulkan::VKCommandBuffer::RecordDraw(lua_State *l, CommandBuffer &hCommandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) { return hCommandBuffer.RecordDraw(vertexCount, instanceCount, firstVertex, firstInstance); }
bool Lua::Vulkan::VKCommandBuffer::RecordDrawIndexed(lua_State *l, CommandBuffer &hCommandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t firstInstance) { return hCommandBuffer.RecordDrawIndexed(indexCount, instanceCount, firstIndex, firstInstance); }
bool Lua::Vulkan::VKCommandBuffer::RecordDrawIndexedIndirect(lua_State *l, CommandBuffer &hCommandBuffer, Buffer &buffer, uint32_t offset, uint32_t drawCount, uint32_t stride) { return hCommandBuffer.RecordDrawIndexedIndirect(buffer, offset, drawCount, stride); }
bool Lua::Vulkan::VKCommandBuffer::RecordDrawIndirect(lua_State *l, CommandBuffer &hCommandBuffer, Buffer &buffer, uint32_t offset, uint32_t drawCount, uint32_t stride) { return hCommandBuffer.RecordDrawIndirect(buffer, offset, drawCount, stride); }
bool Lua::Vulkan::VKCommandBuffer::RecordFillBuffer(lua_State *l, CommandBuffer &hCommandBuffer, Buffer &buffer, uint32_t offset, uint32_t size, uint32_t data) { return hCommandBuffer.RecordFillBuffer(buffer, offset, size, data); }
bool Lua::Vulkan::VKCommandBuffer::RecordSetBlendConstants(lua_State *l, CommandBuffer &hCommandBuffer, const Vector4 &blendConstants) { return hCommandBuffer.RecordSetBlendConstants(std::array<float, 4> {blendConstants[0], blendConstants[1], blendConstants[2], blendConstants[3]}); }
bool Lua::Vulkan::VKCommandBuffer::RecordSetDepthBias(lua_State *l, CommandBuffer &hCommandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float slopeScaledDepthBias) { return hCommandBuffer.RecordSetDepthBias(depthBiasConstantFactor, depthBiasClamp, slopeScaledDepthBias); }
bool Lua::Vulkan::VKCommandBuffer::RecordSetDepthBounds(lua_State *l, CommandBuffer &hCommandBuffer, float minDepthBounds, float maxDepthBounds) { return hCommandBuffer.RecordSetDepthBounds(minDepthBounds, maxDepthBounds); }
bool Lua::Vulkan::VKCommandBuffer::RecordSetLineWidth(lua_State *l, CommandBuffer &hCommandBuffer, float lineWidth) { return hCommandBuffer.RecordSetLineWidth(lineWidth); }
bool Lua::Vulkan::VKCommandBuffer::RecordSetStencilCompareMask(lua_State *l, CommandBuffer &hCommandBuffer, uint32_t faceMask, uint32_t stencilCompareMask) { return hCommandBuffer.RecordSetStencilCompareMask(static_cast<prosper::StencilFaceFlags>(faceMask), stencilCompareMask); }
bool Lua::Vulkan::VKCommandBuffer::RecordSetStencilReference(lua_State *l, CommandBuffer &hCommandBuffer, uint32_t faceMask, uint32_t stencilReference) { return hCommandBuffer.RecordSetStencilReference(static_cast<prosper::StencilFaceFlags>(faceMask), stencilReference); }
bool Lua::Vulkan::VKCommandBuffer::RecordSetStencilWriteMask(lua_State *l, CommandBuffer &hCommandBuffer, uint32_t faceMask, uint32_t stencilWriteMask) { return hCommandBuffer.RecordSetStencilWriteMask(static_cast<prosper::StencilFaceFlags>(faceMask), stencilWriteMask); }
/*void Lua::Vulkan::VKCommandBuffer::RecordWaitEvents(lua_State *l,CommandBuffer &hCommandBuffer)
{
	Lua::PushBool(l,hCommandBuffer.record_wait_events(event,));
}
void Lua::Vulkan::VKCommandBuffer::RecordWriteTimestamp(lua_State *l,CommandBuffer &hCommandBuffer)
{
	Lua::PushBool(l,hCommandBuffer.record_write_timestamp(stage,queryPool,queryIndex));
}
void Lua::Vulkan::VKCommandBuffer::RecordBeginQuery(lua_State *l,CommandBuffer &hCommandBuffer)
{
	Lua::PushBool(l,hCommandBuffer.record_begin_query(queryPool,entry,flags));
}
void Lua::Vulkan::VKCommandBuffer::RecordEndQuery(lua_State *l,CommandBuffer &hCommandBuffer)
{
	Lua::PushBool(l,hCommandBuffer.record_end_query(queryPool,queryIndex));
}*/ // TODO
bool Lua::Vulkan::VKCommandBuffer::RecordDrawGradient(lua_State *l, CommandBuffer &hCommandBuffer, RenderTarget &rt, const Vector2 &dir, luabind::object lnodes)
{
	if(hCommandBuffer.IsPrimary() == false)
		return false;
	auto nodes = get_gradient_nodes(l, lnodes);
	auto primCmd = std::dynamic_pointer_cast<prosper::IPrimaryCommandBuffer>(hCommandBuffer.shared_from_this());
	return pragma::util::record_draw_gradient(c_engine->GetRenderContext(), primCmd, rt, dir, nodes);
}
bool Lua::Vulkan::VKCommandBuffer::StartRecording(lua_State *l, CommandBuffer &hCommandBuffer, bool oneTimeSubmit, bool simultaneousUseAllowed)
{
	if(hCommandBuffer.IsPrimary() == false)
		return false;
	return dynamic_cast<prosper::IPrimaryCommandBuffer &>(hCommandBuffer).StartRecording(oneTimeSubmit, simultaneousUseAllowed);
}

/////////////////////////////////

bool Lua::Vulkan::VKBuffer::IsValid(lua_State *l, Buffer &hBuffer) { return true; }
bool Lua::Vulkan::VKBuffer::Write(lua_State *l, Buffer &hBuffer, uint32_t offset, ::DataStream &ds, uint32_t dsOffset, uint32_t dsSize) { return hBuffer.Write(offset, dsSize, ds->GetData() + dsOffset); }
bool Lua::Vulkan::VKBuffer::Write(lua_State *l, Buffer &hBuffer, uint32_t offset, ::udm::Type type, Lua::udm_ng value)
{
	return udm::visit_ng(type, [&hBuffer, &value, offset](auto tag) {
		using T = typename decltype(tag)::type;
		return hBuffer.Write(offset, luabind::object_cast<T>(value));
	});
}
Lua::opt<::DataStream> Lua::Vulkan::VKBuffer::Read(lua_State *l, Buffer &hBuffer, uint32_t offset, uint32_t size)
{
	auto ds = ::DataStream(size);
	auto r = hBuffer.Read(offset, size, ds->GetData());
	if(r == false)
		return Lua::nil;
	return {l, ds};
}
bool Lua::Vulkan::VKBuffer::Read(lua_State *l, Buffer &hBuffer, uint32_t offset, ::udm::Type type, Lua::udm_ng value)
{
	return udm::visit_ng(type, [&hBuffer, &value, offset](auto tag) {
		using T = typename decltype(tag)::type;
		auto val = luabind::object_cast<T>(value);
		return hBuffer.Read(offset, sizeof(val), &val);
	});
}
bool Lua::Vulkan::VKBuffer::Read(lua_State *l, Buffer &hBuffer, uint32_t offset, uint32_t size, ::DataStream &ds, uint32_t dsOffset)
{
	auto reqSize = size + dsOffset;
	if(ds->GetSize() < reqSize)
		ds->Resize(reqSize);
	return hBuffer.Read(offset, size, ds->GetData() + dsOffset);
}
bool Lua::Vulkan::VKBuffer::Map(lua_State *l, Buffer &hBuffer, uint32_t offset, uint32_t size, Buffer::MapFlags mapFlags) { return hBuffer.Map(offset, size, mapFlags); }

/////////////////////////////////

bool Lua::Vulkan::VKDescriptorSet::IsValid(lua_State *l, DescriptorSet &hDescSet) { return true; }
#if 0
void Lua::Vulkan::VKDescriptorSet::GetBindingInfo(lua_State *l,DescriptorSet &hDescSet)
{
	auto *infos = hDescSet->get_descriptor_set_create_info();
	if(infos == nullptr || infos->empty())
		return;
	auto &info = *infos->at(0);
	auto numBindings = info.get_n_bindings();
	auto t = Lua::CreateTable(l);
	auto idx = 1u;
	for(auto i=decltype(numBindings){0};i<numBindings;++i)
	{
		pragma::LuaDescriptorSetBinding binding {};
		if(infos->at(0)->get_binding_properties_by_index_number(
				i,&binding.bindingIndex,reinterpret_cast<Anvil::DescriptorType*>(&binding.type),&binding.descriptorArraySize,reinterpret_cast<Anvil::ShaderStageFlags*>(&binding.shaderStages)
			) == false)
			continue;
		Lua::PushInt(l,idx++);
		Lua::Push(l,binding);
		Lua::SetTableValue(l,t);
	}
}
void Lua::Vulkan::VKDescriptorSet::GetBindingInfo(lua_State *l,DescriptorSet &hDescSet,uint32_t bindingIdx)
{
	auto *infos = hDescSet->get_descriptor_set_create_info();
	if(infos == nullptr || infos->empty())
		return;
	pragma::LuaDescriptorSetBinding binding {};
	if(infos->at(0)->get_binding_properties_by_index_number(
			bindingIdx,&binding.bindingIndex,reinterpret_cast<Anvil::DescriptorType*>(&binding.type),&binding.descriptorArraySize,reinterpret_cast<Anvil::ShaderStageFlags*>(&binding.shaderStages)
		) == false)
		return;
	Lua::Push(l,binding);
}
#endif
bool Lua::Vulkan::VKDescriptorSet::SetBindingTexture(lua_State *l, Lua::Vulkan::DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Texture &texture) { return hDescSet.GetDescriptorSet()->SetBindingTexture(texture, bindingIdx); }
bool Lua::Vulkan::VKDescriptorSet::SetBindingTexture(lua_State *l, Lua::Vulkan::DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Texture &texture, uint32_t layerId) { return hDescSet.GetDescriptorSet()->SetBindingTexture(texture, bindingIdx, layerId); }
bool Lua::Vulkan::VKDescriptorSet::SetBindingArrayTexture(lua_State *l, DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Texture &texture, uint32_t arrayIdx, uint32_t layerId) { return hDescSet.GetDescriptorSet()->SetBindingArrayTexture(texture, bindingIdx, arrayIdx, layerId); }
bool Lua::Vulkan::VKDescriptorSet::SetBindingArrayTexture(lua_State *l, DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Texture &texture, uint32_t arrayIdx) { return hDescSet.GetDescriptorSet()->SetBindingArrayTexture(texture, bindingIdx, arrayIdx); }
bool Lua::Vulkan::VKDescriptorSet::SetBindingStorageBuffer(lua_State *l, DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Buffer &buffer, uint32_t startOffset, uint32_t size)
{
	return hDescSet.GetDescriptorSet()->SetBindingStorageBuffer(buffer, bindingIdx, startOffset, (size != std::numeric_limits<uint32_t>::max()) ? static_cast<uint64_t>(size) : std::numeric_limits<uint64_t>::max());
}
bool Lua::Vulkan::VKDescriptorSet::SetBindingUniformBuffer(lua_State *l, DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Buffer &buffer, uint32_t startOffset, uint32_t size)
{
	return hDescSet.GetDescriptorSet()->SetBindingUniformBuffer(buffer, bindingIdx, startOffset, (size != std::numeric_limits<uint32_t>::max()) ? static_cast<uint64_t>(size) : std::numeric_limits<uint64_t>::max());
}
bool Lua::Vulkan::VKDescriptorSet::SetBindingUniformBufferDynamic(lua_State *l, DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Buffer &buffer, uint32_t startOffset, uint32_t size)
{
	return hDescSet.GetDescriptorSet()->SetBindingDynamicUniformBuffer(buffer, bindingIdx, startOffset, (size != std::numeric_limits<uint32_t>::max()) ? static_cast<uint64_t>(size) : std::numeric_limits<uint64_t>::max());
}
