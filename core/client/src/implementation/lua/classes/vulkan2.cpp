// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :scripting.lua.classes.vulkan;
import :engine;
import :model;

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
		out << util::to_string(format);
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
		auto r = Result::EventReset;
		if(hEvent.IsSet())
			r = Result::EventSet;
		out << util::to_string(r);
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

namespace pragma::rendering {
	static bool operator==(const SceneMesh &a, const SceneMesh &b) { return &a == &b; }

	static std::ostream &operator<<(std::ostream &out, const SceneMesh &)
	{
		out << "SceneMesh";
		return out;
	}
};

prosper::util::PreparedCommand::Argument Lua::Vulkan::make_pcb_arg(const PreparedCommandLuaArg &larg, ::udm::Type type)
{
	auto &o = larg.o;
	prosper::util::PreparedCommand::Argument arg {};
	auto *dynArg = luabind::object_cast_nothrow<PreparedCommandLuaDynamicArg *>(larg.o, static_cast<PreparedCommandLuaDynamicArg *>(nullptr));
	if(dynArg) {
		arg.SetDynamicValue(dynArg->argName);
		return arg;
	}
	::udm::visit(type, [&o, &arg](auto tag) {
		using T = typename decltype(tag)::type;
		constexpr auto type = ::udm::type_to_enum<T>();
		if constexpr(::udm::is_trivial_type(type))
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
		  pragma::get_cengine()->GetRenderContext().KeepResourceAliveUntilPresentationComplete(pbuf);
		  auto size = recordState.GetArgument<uint32_t>(5);
		  auto lsize = (size != std::numeric_limits<uint32_t>::max()) ? static_cast<uint64_t>(size) : std::numeric_limits<uint64_t>::max();
		  return cmdBuf.RecordBufferBarrier(*pbuf, recordState.GetArgument<prosper::PipelineStageFlags>(0), recordState.GetArgument<prosper::PipelineStageFlags>(1), recordState.GetArgument<prosper::AccessFlags>(2), recordState.GetArgument<prosper::AccessFlags>(3),
		    recordState.GetArgument<uint32_t>(4), lsize);
	  },
	  std::move(pragma::util::make_vector<PcbArg>(Lua::Vulkan::make_pcb_arg<prosper::PipelineStageFlags>(srcStageMask), Lua::Vulkan::make_pcb_arg<prosper::PipelineStageFlags>(dstStageMask), Lua::Vulkan::make_pcb_arg<prosper::AccessFlags>(srcAccessMask),
	    Lua::Vulkan::make_pcb_arg<prosper::AccessFlags>(dstAccessMask), Lua::Vulkan::make_pcb_arg<uint32_t>(offset), Lua::Vulkan::make_pcb_arg<uint32_t>(size))));
};

static bool pcb_record_bind_descriptor_set(prosper::util::PreparedCommandBuffer &pcb, const std::shared_ptr<prosper::IDescriptorSetGroup> &descSet, const PcbLuaArg &firstSet, const PcbLuaArg &dynamicOffset)
{
	pcb.PushCommand(
	  [descSet](const prosper::util::PreparedCommandBufferRecordState &recordState) -> bool {
		  constexpr auto BIND_PIPELINE_LAYOUT = pragma::string::string_switch::hash("pipelineLayout");
		  auto &pipelineLayout = recordState.userData.Get<prosper::IShaderPipelineLayout>(BIND_PIPELINE_LAYOUT);

		  auto dynOffset = recordState.GetArgument<uint32_t>(1);
		  return recordState.commandBuffer.RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics, pipelineLayout, recordState.GetArgument<uint32_t>(0), *descSet->GetDescriptorSet(), &dynOffset);
	  },
	  std::move(pragma::util::make_vector<PcbArg>(Lua::Vulkan::make_pcb_arg<uint32_t>(firstSet), Lua::Vulkan::make_pcb_arg<uint32_t>(dynamicOffset))));
	return true;
}

static std::shared_ptr<prosper::util::PreparedCommandBuffer> create_pcb() { return pragma::util::make_shared<prosper::util::PreparedCommandBuffer>(); }

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
	defVkRenderPass.def(
	  "GetFinalLayout", +[](const Lua::Vulkan::RenderPass &rp, uint32_t attIdx) -> std::optional<prosper::ImageLayout> {
		  auto &createInfo = rp.GetCreateInfo();
		  if(attIdx >= createInfo.attachments.size())
			  return {};
		  return createInfo.attachments[attIdx].finalLayout;
	  });
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
	defVkMemory.def("Write",static_cast<void(*)(lua::State*,Lua::Vulkan::Memory&,uint32_t,::util::DataStream&)>([](lua::State *l,Lua::Vulkan::Memory &hMemory,uint32_t offset,::util::DataStream &ds) {
		Lua::Vulkan::VKMemory::Write(l,hMemory,offset,ds,0u,ds->GetSize());
	}));
	defVkMemory.def("Read",static_cast<void(*)(lua::State*,Lua::Vulkan::Memory&,uint32_t,uint32_t,::util::DataStream&,uint32_t)>(&Lua::Vulkan::VKMemory::Read));
	defVkMemory.def("Read",static_cast<void(*)(lua::State*,Lua::Vulkan::Memory&,uint32_t,uint32_t,::util::DataStream&)>([](lua::State *l,Lua::Vulkan::Memory &hMemory,uint32_t offset,uint32_t size,::util::DataStream &ds) {
		Lua::Vulkan::VKMemory::Read(l,hMemory,offset,size,ds,0u);
	}));
	defVkMemory.def("Read",static_cast<void(*)(lua::State*,Lua::Vulkan::Memory&,uint32_t,uint32_t)>(&Lua::Vulkan::VKMemory::Read));
	defVkMemory.def("Read",static_cast<void(*)(lua::State*,Lua::Vulkan::Memory&)>([](lua::State *l,Lua::Vulkan::Memory &hMemory) {
		Lua::Vulkan::VKMemory::Read(l,hMemory,0u,hMemory.get_create_info_ptr()->get_size());
	}));
	defVkMemory.def("Map",&Lua::Vulkan::VKMemory::Map);
	defVkMemory.def("Map",static_cast<void(*)(lua::State*,Lua::Vulkan::Memory&)>([](lua::State *l,Lua::Vulkan::Memory &hMemory) {
		Lua::Vulkan::VKMemory::Map(l,hMemory,0u,hMemory.get_create_info_ptr()->get_size());
	}));
	defVkMemory.def("Unmap",&Lua::Vulkan::VKMemory::Unmap);
	prosperMod[defVkMemory];
#endif
	auto defVkCommandBuffer = luabind::class_<Lua::Vulkan::CommandBuffer>("CommandBuffer");
	defVkCommandBuffer.add_static_constant("RENDER_PASS_FLAG_NONE", pragma::math::to_integral(prosper::IPrimaryCommandBuffer::RenderPassFlags::None));
	defVkCommandBuffer.add_static_constant("RENDER_PASS_FLAG_SECONDARY_COMMAND_BUFFERS_BIT", pragma::math::to_integral(prosper::IPrimaryCommandBuffer::RenderPassFlags::SecondaryCommandBuffers));
	defVkCommandBuffer.def(luabind::tostring(luabind::self));
	defVkCommandBuffer.def(luabind::const_self == luabind::const_self);
	defVkCommandBuffer.def("IsRecording", &Lua::Vulkan::CommandBuffer::IsRecording);
	defVkCommandBuffer.def("RecordClearImage", static_cast<bool (*)(lua::State *, Lua::Vulkan::CommandBuffer &, Lua::Vulkan::Image &, const Color &, const prosper::util::ClearImageInfo &)>(&Lua::Vulkan::VKCommandBuffer::RecordClearImage));
	defVkCommandBuffer.def("RecordClearImage", static_cast<bool (*)(lua::State *, Lua::Vulkan::CommandBuffer &, Lua::Vulkan::Image &, const Color &)>([](lua::State *l, Lua::Vulkan::CommandBuffer &cmdBuffer, Lua::Vulkan::Image &img, const Color &col) {
		return Lua::Vulkan::VKCommandBuffer::RecordClearImage(l, cmdBuffer, img, col);
	}));
	defVkCommandBuffer.def("RecordClearImage", static_cast<bool (*)(lua::State *, Lua::Vulkan::CommandBuffer &, Lua::Vulkan::Image &, std::optional<float>, std::optional<uint32_t>, const prosper::util::ClearImageInfo &)>(&Lua::Vulkan::VKCommandBuffer::RecordClearImage));
	defVkCommandBuffer.def(
	  "RecordClearImage", +[](lua::State *l, Lua::Vulkan::CommandBuffer &cmdBuffer, Lua::Vulkan::Image &img, std::optional<float> clearDepth, std::optional<uint32_t> clearStencil) { return Lua::Vulkan::VKCommandBuffer::RecordClearImage(l, cmdBuffer, img, clearDepth, clearStencil); });
	defVkCommandBuffer.def("RecordClearAttachment", static_cast<bool (*)(lua::State *, Lua::Vulkan::CommandBuffer &, Lua::Vulkan::Image &, const Color &, uint32_t)>(&Lua::Vulkan::VKCommandBuffer::RecordClearAttachment));
	defVkCommandBuffer.def("RecordClearAttachment", static_cast<bool (*)(lua::State *, Lua::Vulkan::CommandBuffer &, Lua::Vulkan::Image &, std::optional<float>, std::optional<uint32_t>)>(&Lua::Vulkan::VKCommandBuffer::RecordClearAttachment));
	defVkCommandBuffer.def(
	  "RecordClearAttachment", +[](lua::State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, Lua::Vulkan::Image &img, std::optional<float> clearDepth) { return Lua::Vulkan::VKCommandBuffer::RecordClearAttachment(l, hCommandBuffer, img, clearDepth, std::optional<uint32_t> {}); });
	defVkCommandBuffer.def("RecordCopyImage", &Lua::Vulkan::VKCommandBuffer::RecordCopyImage);
	defVkCommandBuffer.def("RecordCopyBufferToImage", &Lua::Vulkan::VKCommandBuffer::RecordCopyBufferToImage);
	defVkCommandBuffer.def("RecordCopyBufferToImage", static_cast<bool (*)(lua::State *, Lua::Vulkan::CommandBuffer &, Lua::Vulkan::Buffer &, Lua::Vulkan::Image &)>([](lua::State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, Lua::Vulkan::Buffer &bufSrc, Lua::Vulkan::Image &imgDst) {
		return Lua::Vulkan::VKCommandBuffer::RecordCopyBufferToImage(l, hCommandBuffer, bufSrc, imgDst, {});
	}));
	defVkCommandBuffer.def("RecordCopyBuffer", &Lua::Vulkan::VKCommandBuffer::RecordCopyBuffer);
	defVkCommandBuffer.def("RecordUpdateBuffer", static_cast<bool (*)(lua::State *, Lua::Vulkan::CommandBuffer &, Lua::Vulkan::Buffer &, uint32_t, pragma::util::DataStream &)>(&Lua::Vulkan::VKCommandBuffer::RecordUpdateBuffer));
	defVkCommandBuffer.def("RecordUpdateBuffer", static_cast<bool (*)(lua::State *, Lua::Vulkan::CommandBuffer &, Lua::Vulkan::Buffer &, uint32_t, udm::Type, Lua::udm_ng)>(&Lua::Vulkan::VKCommandBuffer::RecordUpdateBuffer));
	defVkCommandBuffer.def("RecordBlitImage", &Lua::Vulkan::VKCommandBuffer::RecordBlitImage);
	defVkCommandBuffer.def("RecordResolveImage", &Lua::Vulkan::VKCommandBuffer::RecordResolveImage);
	defVkCommandBuffer.def("RecordBlitTexture", &Lua::Vulkan::VKCommandBuffer::RecordBlitTexture);
	defVkCommandBuffer.def("RecordGenerateMipmaps", &Lua::Vulkan::VKCommandBuffer::RecordGenerateMipmaps);
	defVkCommandBuffer.def("RecordPipelineBarrier", &Lua::Vulkan::VKCommandBuffer::RecordPipelineBarrier);
	defVkCommandBuffer.def("RecordImageBarrier", static_cast<bool (*)(lua::State *, Lua::Vulkan::CommandBuffer &, Lua::Vulkan::Image &, uint32_t, uint32_t, const prosper::util::ImageSubresourceRange &)>(&Lua::Vulkan::VKCommandBuffer::RecordImageBarrier));
	defVkCommandBuffer.def("RecordImageBarrier", static_cast<bool (*)(lua::State *, Lua::Vulkan::CommandBuffer &, Lua::Vulkan::Image &, uint32_t, uint32_t)>([](lua::State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, Lua::Vulkan::Image &img, uint32_t oldLayout, uint32_t newLayout) {
		return Lua::Vulkan::VKCommandBuffer::RecordImageBarrier(l, hCommandBuffer, img, oldLayout, newLayout, prosper::util::ImageSubresourceRange {});
	}));
	defVkCommandBuffer.def("RecordImageBarrier", static_cast<bool (*)(lua::State *, Lua::Vulkan::CommandBuffer &, Lua::Vulkan::Image &, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t)>(&Lua::Vulkan::VKCommandBuffer::RecordImageBarrier));
	defVkCommandBuffer.def("RecordImageBarrier",
	  static_cast<bool (*)(lua::State *, Lua::Vulkan::CommandBuffer &, Lua::Vulkan::Image &, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t)>(
	    [](lua::State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, Lua::Vulkan::Image &img, uint32_t srcStageMask, uint32_t dstStageMask, uint32_t oldLayout, uint32_t newLayout, uint32_t srcAccessMask, uint32_t dstAccessMask) {
		    return Lua::Vulkan::VKCommandBuffer::RecordImageBarrier(l, hCommandBuffer, img, srcStageMask, dstStageMask, oldLayout, newLayout, srcAccessMask, dstAccessMask, 0u);
	    }));
	defVkCommandBuffer.def("RecordBufferBarrier", &Lua::Vulkan::VKCommandBuffer::RecordBufferBarrier);
	defVkCommandBuffer.def("RecordBufferBarrier",
	  static_cast<bool (*)(lua::State *, Lua::Vulkan::CommandBuffer &, Lua::Vulkan::Buffer &, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t)>(
	    [](lua::State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, Lua::Vulkan::Buffer &buf, uint32_t srcStageMask, uint32_t dstStageMask, uint32_t srcAccessMask, uint32_t dstAccessMask, uint32_t offset) {
		    return Lua::Vulkan::VKCommandBuffer::RecordBufferBarrier(l, hCommandBuffer, buf, srcStageMask, dstStageMask, srcAccessMask, dstAccessMask, offset, std::numeric_limits<uint32_t>::max());
	    }));
	defVkCommandBuffer.def("RecordBufferBarrier",
	  static_cast<bool (*)(lua::State *, Lua::Vulkan::CommandBuffer &, Lua::Vulkan::Buffer &, uint32_t, uint32_t, uint32_t, uint32_t)>(
	    [](lua::State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, Lua::Vulkan::Buffer &buf, uint32_t srcStageMask, uint32_t dstStageMask, uint32_t srcAccessMask, uint32_t dstAccessMask) {
		    return Lua::Vulkan::VKCommandBuffer::RecordBufferBarrier(l, hCommandBuffer, buf, srcStageMask, dstStageMask, srcAccessMask, dstAccessMask, 0u, std::numeric_limits<uint32_t>::max());
	    }));
	defVkCommandBuffer.def("RecordSetViewport", &Lua::Vulkan::VKCommandBuffer::RecordSetViewport);
	defVkCommandBuffer.def("RecordSetScissor", &Lua::Vulkan::VKCommandBuffer::RecordSetScissor);
	defVkCommandBuffer.def("RecordBeginRenderPass", static_cast<bool (*)(lua::State *, Lua::Vulkan::CommandBuffer &, Lua::Vulkan::RenderPassInfo &)>([](lua::State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, Lua::Vulkan::RenderPassInfo &rpInfo) {
		return Lua::Vulkan::VKCommandBuffer::RecordBeginRenderPass(l, hCommandBuffer, rpInfo);
	}));
	defVkCommandBuffer.def("RecordEndRenderPass", &Lua::Vulkan::VKCommandBuffer::RecordEndRenderPass);
	defVkCommandBuffer.def("RecordBindIndexBuffer", &Lua::Vulkan::VKCommandBuffer::RecordBindIndexBuffer);
	defVkCommandBuffer.def("RecordBindIndexBuffer", static_cast<bool (*)(lua::State *, Lua::Vulkan::CommandBuffer &, Lua::Vulkan::Buffer &, uint32_t)>([](lua::State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, Lua::Vulkan::Buffer &indexBuffer, uint32_t indexType) {
		return Lua::Vulkan::VKCommandBuffer::RecordBindIndexBuffer(l, hCommandBuffer, indexBuffer, indexType, 0u);
	}));
	defVkCommandBuffer.def("RecordBindVertexBuffer", &Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffer);
	defVkCommandBuffer.def("RecordBindVertexBuffer",
	  static_cast<bool (*)(lua::State *, Lua::Vulkan::CommandBuffer &, prosper::ShaderGraphics &, Lua::Vulkan::Buffer &, uint32_t)>([](lua::State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, prosper::ShaderGraphics &graphics, Lua::Vulkan::Buffer &vertexBuffer, uint32_t startBinding) {
		  return Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffer(l, hCommandBuffer, graphics, vertexBuffer, startBinding, 0u);
	  }));
	defVkCommandBuffer.def("RecordBindVertexBuffer",
	  static_cast<bool (*)(lua::State *, Lua::Vulkan::CommandBuffer &, prosper::ShaderGraphics &, Lua::Vulkan::Buffer &)>(
	    [](lua::State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, prosper::ShaderGraphics &graphics, Lua::Vulkan::Buffer &vertexBuffer) { return Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffer(l, hCommandBuffer, graphics, vertexBuffer, 0u, 0u); }));
	defVkCommandBuffer.def("RecordBindVertexBuffers", static_cast<bool (*)(lua::State *, Lua::Vulkan::CommandBuffer &, prosper::ShaderGraphics &, const std::vector<prosper::IBuffer *> &, uint32_t, const std::vector<uint64_t> &)>(&Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffers));
	defVkCommandBuffer.def("RecordBindVertexBuffers", static_cast<bool (*)(lua::State *, Lua::Vulkan::CommandBuffer &, prosper::ShaderGraphics &, const std::vector<prosper::IBuffer *> &, uint32_t)>(&Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffers));
	defVkCommandBuffer.def("RecordBindVertexBuffers", static_cast<bool (*)(lua::State *, Lua::Vulkan::CommandBuffer &, prosper::ShaderGraphics &, const std::vector<prosper::IBuffer *> &)>(&Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffers));
#if 0
bool Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffers(
	lua::State *l,CommandBuffer &hCommandBuffer,prosper::ShaderGraphics &graphics,const std::vector<prosper::IBuffer*> &buffers,uint32_t startBinding
)
bool Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffers(
	lua::State *l,CommandBuffer &hCommandBuffer,prosper::ShaderGraphics &graphics,const std::vector<prosper::IBuffer*> &buffers,uint32_t startBinding,const std::vector<uint64_t> &voffsets
)
#endif
	defVkCommandBuffer.def("RecordCopyImageToBuffer", &Lua::Vulkan::VKCommandBuffer::RecordCopyImageToBuffer);
#if 0
	defVkCommandBuffer.def("RecordDispatch",&Lua::Vulkan::VKCommandBuffer::RecordDispatch);
	defVkCommandBuffer.def("RecordDispatch",static_cast<void(*)(lua::State*,Lua::Vulkan::CommandBuffer&,uint32_t,uint32_t)>([](lua::State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer,uint32_t x,uint32_t y) {
		Lua::Vulkan::VKCommandBuffer::RecordDispatch(l,hCommandBuffer,x,y,0u);
	}));
	defVkCommandBuffer.def("RecordDispatch",static_cast<void(*)(lua::State*,Lua::Vulkan::CommandBuffer&,uint32_t)>([](lua::State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer,uint32_t x) {
		Lua::Vulkan::VKCommandBuffer::RecordDispatch(l,hCommandBuffer,x,0u,0u);
	}));
	defVkCommandBuffer.def("RecordDispatch",static_cast<void(*)(lua::State*,Lua::Vulkan::CommandBuffer&)>([](lua::State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer) {
		Lua::Vulkan::VKCommandBuffer::RecordDispatch(l,hCommandBuffer,0u,0u,0u);
	}));
	defVkCommandBuffer.def("RecordDispatchIndirect",&Lua::Vulkan::VKCommandBuffer::RecordDispatchIndirect);
	defVkCommandBuffer.def("RecordDispatchIndirect",static_cast<void(*)(lua::State*,Lua::Vulkan::CommandBuffer&,Lua::Vulkan::Buffer&)>([](lua::State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer,Lua::Vulkan::Buffer &buffer) {
		Lua::Vulkan::VKCommandBuffer::RecordDispatchIndirect(l,hCommandBuffer,buffer,0u);
	}));
#endif
	defVkCommandBuffer.def("RecordDraw",
	  static_cast<bool (*)(lua::State *, Lua::Vulkan::CommandBuffer &, uint32_t, uint32_t, uint32_t, uint32_t)>([](lua::State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
		  return Lua::Vulkan::VKCommandBuffer::RecordDraw(l, hCommandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
	  }));
	defVkCommandBuffer.def("RecordDraw", static_cast<bool (*)(lua::State *, Lua::Vulkan::CommandBuffer &, uint32_t, uint32_t, uint32_t)>([](lua::State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex) {
		return Lua::Vulkan::VKCommandBuffer::RecordDraw(l, hCommandBuffer, vertexCount, instanceCount, firstVertex, 0u);
	}));
	defVkCommandBuffer.def("RecordDraw", static_cast<bool (*)(lua::State *, Lua::Vulkan::CommandBuffer &, uint32_t, uint32_t)>([](lua::State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, uint32_t vertexCount, uint32_t instanceCount) {
		return Lua::Vulkan::VKCommandBuffer::RecordDraw(l, hCommandBuffer, vertexCount, instanceCount, 0u, 0u);
	}));
	defVkCommandBuffer.def("RecordDraw",
	  static_cast<bool (*)(lua::State *, Lua::Vulkan::CommandBuffer &, uint32_t)>([](lua::State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, uint32_t vertexCount) { return Lua::Vulkan::VKCommandBuffer::RecordDraw(l, hCommandBuffer, vertexCount, 1u, 0u, 0u); }));
	defVkCommandBuffer.def("RecordDrawIndexed", &Lua::Vulkan::VKCommandBuffer::RecordDrawIndexed);
	defVkCommandBuffer.def("RecordDrawIndexed", static_cast<bool (*)(lua::State *, Lua::Vulkan::CommandBuffer &, uint32_t, uint32_t, uint32_t)>([](lua::State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex) {
		return Lua::Vulkan::VKCommandBuffer::RecordDrawIndexed(l, hCommandBuffer, indexCount, instanceCount, firstIndex, 0u);
	}));
	defVkCommandBuffer.def("RecordDrawIndexed", static_cast<bool (*)(lua::State *, Lua::Vulkan::CommandBuffer &, uint32_t, uint32_t)>([](lua::State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, uint32_t indexCount, uint32_t instanceCount) {
		return Lua::Vulkan::VKCommandBuffer::RecordDrawIndexed(l, hCommandBuffer, indexCount, instanceCount, 0u, 0u);
	}));
	defVkCommandBuffer.def("RecordDrawIndexed",
	  static_cast<bool (*)(lua::State *, Lua::Vulkan::CommandBuffer &, uint32_t)>([](lua::State *l, Lua::Vulkan::CommandBuffer &hCommandBuffer, uint32_t indexCount) { return Lua::Vulkan::VKCommandBuffer::RecordDrawIndexed(l, hCommandBuffer, indexCount, 1u, 0u, 0u); }));
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
	defVkCommandBuffer.def("SetDebugName", static_cast<void (*)(lua::State *, Lua::Vulkan::CommandBuffer &, const std::string &)>([](lua::State *l, Lua::Vulkan::CommandBuffer &cb, const std::string &name) { Lua::Vulkan::VKContextObject::SetDebugName(l, cb, name); }));
	defVkCommandBuffer.def("GetDebugName", static_cast<std::string (*)(lua::State *, Lua::Vulkan::CommandBuffer &)>([](lua::State *l, Lua::Vulkan::CommandBuffer &cb) { return Lua::Vulkan::VKContextObject::GetDebugName(l, cb); }));
	defVkCommandBuffer.def("Flush", static_cast<void (*)(lua::State *, Lua::Vulkan::CommandBuffer &)>([](lua::State *l, Lua::Vulkan::CommandBuffer &cb) { pragma::get_cengine()->GetRenderContext().FlushCommandBuffer(cb); }));
	prosperMod[defVkCommandBuffer];

	auto devVkBuffer = luabind::class_<Lua::Vulkan::Buffer>("Buffer");
	devVkBuffer.def(luabind::tostring(luabind::self));
	devVkBuffer.def(luabind::const_self == luabind::const_self);
	devVkBuffer.def("IsValid", &Lua::Vulkan::VKBuffer::IsValid);
	devVkBuffer.def("GetStartOffset", &Lua::Vulkan::Buffer::GetStartOffset);
	devVkBuffer.def("GetBaseIndex", &Lua::Vulkan::Buffer::GetBaseIndex);
	devVkBuffer.def("GetSize", &Lua::Vulkan::Buffer::GetSize);
	devVkBuffer.def("GetUsageFlags", &Lua::Vulkan::Buffer::GetUsageFlags);
	devVkBuffer.def("SetDebugName", static_cast<void (*)(lua::State *, Lua::Vulkan::Buffer &, const std::string &)>([](lua::State *l, Lua::Vulkan::Buffer &buf, const std::string &name) { Lua::Vulkan::VKContextObject::SetDebugName(l, buf, name); }));
	devVkBuffer.def("GetDebugName", static_cast<std::string (*)(lua::State *, Lua::Vulkan::Buffer &)>([](lua::State *l, Lua::Vulkan::Buffer &buf) { return Lua::Vulkan::VKContextObject::GetDebugName(l, buf); }));
	devVkBuffer.def("GetCreateInfo", &Lua::Vulkan::Buffer::GetCreateInfo, luabind::copy_policy<0> {});
	devVkBuffer.def("SetPermanentlyMapped", &Lua::Vulkan::Buffer::SetPermanentlyMapped);
	devVkBuffer.def("GetParent", static_cast<std::shared_ptr<prosper::IBuffer> (prosper::IBuffer::*)()>(&Lua::Vulkan::Buffer::GetParent));
	devVkBuffer.def("WriteMemory", static_cast<bool (*)(lua::State *, Lua::Vulkan::Buffer &, uint32_t, pragma::util::DataStream &, uint32_t, uint32_t)>(&Lua::Vulkan::VKBuffer::Write));
	devVkBuffer.def("WriteMemory", static_cast<bool (*)(lua::State *, Lua::Vulkan::Buffer &, uint32_t, udm::Type, Lua::udm_ng)>(&Lua::Vulkan::VKBuffer::Write));
	devVkBuffer.def("WriteMemory",
	  static_cast<bool (*)(lua::State *, Lua::Vulkan::Buffer &, uint32_t, pragma::util::DataStream &)>([](lua::State *l, Lua::Vulkan::Buffer &hBuffer, uint32_t offset, pragma::util::DataStream &ds) { return Lua::Vulkan::VKBuffer::Write(l, hBuffer, offset, ds, 0u, ds->GetSize()); }));
	devVkBuffer.def("ReadMemory", static_cast<bool (*)(lua::State *, Lua::Vulkan::Buffer &, uint32_t, uint32_t, pragma::util::DataStream &, uint32_t)>(&Lua::Vulkan::VKBuffer::Read));
	devVkBuffer.def("ReadMemory", static_cast<bool (*)(lua::State *, Lua::Vulkan::Buffer &, uint32_t, uint32_t, pragma::util::DataStream &)>([](lua::State *l, Lua::Vulkan::Buffer &hBuffer, uint32_t offset, uint32_t size, pragma::util::DataStream &ds) {
		return Lua::Vulkan::VKBuffer::Read(l, hBuffer, offset, size, ds, 0u);
	}));
	devVkBuffer.def("ReadMemory", static_cast<bool (*)(lua::State *, Lua::Vulkan::Buffer &, uint32_t, udm::Type, Lua::udm_ng)>(&Lua::Vulkan::VKBuffer::Read));
	devVkBuffer.def("ReadMemory", static_cast<Lua::opt<pragma::util::DataStream> (*)(lua::State *, Lua::Vulkan::Buffer &, uint32_t, uint32_t)>(&Lua::Vulkan::VKBuffer::Read));
	devVkBuffer.def("ReadMemory", static_cast<Lua::opt<pragma::util::DataStream> (*)(lua::State *, Lua::Vulkan::Buffer &)>([](lua::State *l, Lua::Vulkan::Buffer &hBuffer) { return Lua::Vulkan::VKBuffer::Read(l, hBuffer, 0u, hBuffer.GetSize()); }));
	devVkBuffer.def("MapMemory", &Lua::Vulkan::VKBuffer::Map);
	devVkBuffer.def("MapMemory",
	  static_cast<bool (*)(lua::State *, Lua::Vulkan::Buffer &, Lua::Vulkan::Buffer::MapFlags)>([](lua::State *l, Lua::Vulkan::Buffer &hBuffer, Lua::Vulkan::Buffer::MapFlags mapFlags) { return Lua::Vulkan::VKBuffer::Map(l, hBuffer, 0u, hBuffer.GetSize(), mapFlags); }));
	devVkBuffer.def("UnmapMemory", &Lua::Vulkan::Buffer::Unmap);
	prosperMod[devVkBuffer];

	auto devVkSwapBuffer = luabind::class_<Lua::Vulkan::SwapBuffer>("SwapBuffer");
	prosperMod[devVkSwapBuffer];

	auto defVkDescriptorSet = luabind::class_<Lua::Vulkan::DescriptorSet>("DescriptorSet");
	defVkDescriptorSet.def(luabind::tostring(luabind::self));
	defVkDescriptorSet.def(luabind::const_self == luabind::const_self);
#if 0
	defVkDescriptorSet.def("GetBindingInfo",static_cast<void(*)(lua::State*,Lua::Vulkan::DescriptorSet&,uint32_t)>(&Lua::Vulkan::VKDescriptorSet::GetBindingInfo));
	defVkDescriptorSet.def("GetBindingInfo",static_cast<void(*)(lua::State*,Lua::Vulkan::DescriptorSet&)>(&Lua::Vulkan::VKDescriptorSet::GetBindingInfo));
#endif
	defVkDescriptorSet.def("GetBindingCount", &Lua::Vulkan::DescriptorSet::GetBindingCount);
	defVkDescriptorSet.def("GetBindingTexture", &Lua::Vulkan::VKDescriptorSet::GetBindingTexture);
	defVkDescriptorSet.def("SetBindingTexture", static_cast<bool (*)(lua::State *, Lua::Vulkan::DescriptorSet &, uint32_t, Lua::Vulkan::Texture &, uint32_t)>([](lua::State *l, Lua::Vulkan::DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Texture &texture, uint32_t layerId) {
		return Lua::Vulkan::VKDescriptorSet::SetBindingTexture(l, hDescSet, bindingIdx, texture, layerId);
	}));
	defVkDescriptorSet.def("SetBindingTexture", static_cast<bool (*)(lua::State *, Lua::Vulkan::DescriptorSet &, uint32_t, Lua::Vulkan::Texture &)>([](lua::State *l, Lua::Vulkan::DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Texture &texture) {
		return Lua::Vulkan::VKDescriptorSet::SetBindingTexture(l, hDescSet, bindingIdx, texture);
	}));
	defVkDescriptorSet.def("SetBindingArrayTexture", static_cast<bool (*)(lua::State *, Lua::Vulkan::DescriptorSet &, uint32_t, Lua::Vulkan::Texture &, uint32_t, uint32_t)>(&Lua::Vulkan::VKDescriptorSet::SetBindingArrayTexture));
	defVkDescriptorSet.def("SetBindingArrayTexture",
	  static_cast<bool (*)(lua::State *, Lua::Vulkan::DescriptorSet &, uint32_t, Lua::Vulkan::Texture &, uint32_t)>(
	    [](lua::State *l, Lua::Vulkan::DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Texture &texture, uint32_t arrayIdx) { return Lua::Vulkan::VKDescriptorSet::SetBindingArrayTexture(l, hDescSet, bindingIdx, texture, arrayIdx); }));
	defVkDescriptorSet.def("SetBindingUniformBuffer", &Lua::Vulkan::VKDescriptorSet::SetBindingUniformBuffer);
	defVkDescriptorSet.def("SetBindingUniformBuffer",
	  static_cast<bool (*)(lua::State *, Lua::Vulkan::DescriptorSet &, uint32_t, Lua::Vulkan::Buffer &, uint32_t)>(
	    [](lua::State *l, Lua::Vulkan::DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Buffer &buffer, uint32_t startOffset) { return Lua::Vulkan::VKDescriptorSet::SetBindingUniformBuffer(l, hDescSet, bindingIdx, buffer, startOffset, std::numeric_limits<uint32_t>::max()); }));
	defVkDescriptorSet.def("SetBindingUniformBuffer", static_cast<bool (*)(lua::State *, Lua::Vulkan::DescriptorSet &, uint32_t, Lua::Vulkan::Buffer &)>([](lua::State *l, Lua::Vulkan::DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Buffer &buffer) {
		return Lua::Vulkan::VKDescriptorSet::SetBindingUniformBuffer(l, hDescSet, bindingIdx, buffer, 0u, std::numeric_limits<uint32_t>::max());
	}));
	defVkDescriptorSet.def("SetBindingUniformBufferDynamic", &Lua::Vulkan::VKDescriptorSet::SetBindingUniformBufferDynamic);
	defVkDescriptorSet.def("SetBindingUniformBufferDynamic",
	  static_cast<bool (*)(lua::State *, Lua::Vulkan::DescriptorSet &, uint32_t, Lua::Vulkan::Buffer &, uint32_t)>([](lua::State *l, Lua::Vulkan::DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Buffer &buffer, uint32_t startOffset) {
		  return Lua::Vulkan::VKDescriptorSet::SetBindingUniformBufferDynamic(l, hDescSet, bindingIdx, buffer, startOffset, std::numeric_limits<uint32_t>::max());
	  }));
	defVkDescriptorSet.def("SetBindingUniformBufferDynamic", static_cast<bool (*)(lua::State *, Lua::Vulkan::DescriptorSet &, uint32_t, Lua::Vulkan::Buffer &)>([](lua::State *l, Lua::Vulkan::DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Buffer &buffer) {
		return Lua::Vulkan::VKDescriptorSet::SetBindingUniformBufferDynamic(l, hDescSet, bindingIdx, buffer, 0u, std::numeric_limits<uint32_t>::max());
	}));
	defVkDescriptorSet.def("SetBindingStorageBuffer", &Lua::Vulkan::VKDescriptorSet::SetBindingStorageBuffer);
	defVkDescriptorSet.def("SetBindingStorageBuffer",
	  static_cast<bool (*)(lua::State *, Lua::Vulkan::DescriptorSet &, uint32_t, Lua::Vulkan::Buffer &, uint32_t)>(
	    [](lua::State *l, Lua::Vulkan::DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Buffer &buffer, uint32_t startOffset) { return Lua::Vulkan::VKDescriptorSet::SetBindingStorageBuffer(l, hDescSet, bindingIdx, buffer, startOffset, std::numeric_limits<uint32_t>::max()); }));
	defVkDescriptorSet.def("SetBindingStorageBuffer", static_cast<bool (*)(lua::State *, Lua::Vulkan::DescriptorSet &, uint32_t, Lua::Vulkan::Buffer &)>([](lua::State *l, Lua::Vulkan::DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Buffer &buffer) {
		return Lua::Vulkan::VKDescriptorSet::SetBindingStorageBuffer(l, hDescSet, bindingIdx, buffer, 0u, std::numeric_limits<uint32_t>::max());
	}));
	defVkDescriptorSet.def("SetDebugName", static_cast<void (*)(lua::State *, Lua::Vulkan::DescriptorSet &, const std::string &)>([](lua::State *l, Lua::Vulkan::DescriptorSet &ds, const std::string &name) { Lua::Vulkan::VKContextObject::SetDebugName(l, ds, name); }));
	defVkDescriptorSet.def("GetDebugName", static_cast<std::string (*)(lua::State *, Lua::Vulkan::DescriptorSet &)>([](lua::State *l, Lua::Vulkan::DescriptorSet &ds) { return Lua::Vulkan::VKContextObject::GetDebugName(l, ds); }));
	defVkDescriptorSet.def("Update", +[](lua::State *l, Lua::Vulkan::DescriptorSet &ds) { return ds.GetDescriptorSet()->Update(); });
	prosperMod[defVkDescriptorSet];

	auto defVkMesh = luabind::class_<pragma::rendering::SceneMesh>("Mesh");
	defVkMesh.def(luabind::tostring(luabind::self));
	defVkMesh.def(luabind::const_self == luabind::const_self);
	defVkMesh.def("GetVertexBuffer", &pragma::rendering::SceneMesh::GetVertexBuffer);
	defVkMesh.def("GetVertexWeightBuffer", &pragma::rendering::SceneMesh::GetVertexWeightBuffer);
	defVkMesh.def("GetAlphaBuffer", &pragma::rendering::SceneMesh::GetAlphaBuffer);
	defVkMesh.def("GetIndexBuffer", &pragma::rendering::SceneMesh::GetIndexBuffer);
	defVkMesh.def("SetVertexBuffer", &pragma::rendering::SceneMesh::SetVertexBuffer);
	defVkMesh.def("SetVertexWeightBuffer", &pragma::rendering::SceneMesh::SetVertexWeightBuffer);
	defVkMesh.def("SetAlphaBuffer", &pragma::rendering::SceneMesh::SetAlphaBuffer);
	defVkMesh.def("SetIndexBuffer", &pragma::rendering::SceneMesh::SetIndexBuffer);
	defVkMesh.def("ClearBuffers", static_cast<void (*)(lua::State *, pragma::rendering::SceneMesh &)>([](lua::State *l, pragma::rendering::SceneMesh &mesh) { mesh.ClearBuffers(); }));
	prosperMod[defVkMesh];

	auto defPcb = luabind::class_<prosper::util::PreparedCommandBuffer>("PreparedCommandBuffer");
	defPcb.def(
	  "RecordSetScissor", +[](prosper::util::PreparedCommandBuffer &pcb, const PcbLuaArg &width, const PcbLuaArg &height, const PcbLuaArg &x, const PcbLuaArg &y) {
		  pcb.PushCommand(
		    [](const prosper::util::PreparedCommandBufferRecordState &recordState) -> bool {
			    auto &cmdBuf = recordState.commandBuffer;
			    return cmdBuf.RecordSetScissor(recordState.GetArgument<uint32_t>(0), recordState.GetArgument<uint32_t>(1), recordState.GetArgument<uint32_t>(2), recordState.GetArgument<uint32_t>(3));
		    },
		    std::move(pragma::util::make_vector<PcbArg>(make_pcb_arg(width, udm::Type::UInt32), make_pcb_arg(height, udm::Type::UInt32), make_pcb_arg(x, udm::Type::UInt32), make_pcb_arg(y, udm::Type::UInt32))));
	  });
	defPcb.def("RecordBufferBarrier", &pcb_recordBufferBarrier);
	defPcb.def(
	  "RecordBufferBarrier", +[](lua::State *l, prosper::util::PreparedCommandBuffer &pcb, Lua::Vulkan::Buffer &buf, const PcbLuaArg &srcStageMask, const PcbLuaArg &dstStageMask, const PcbLuaArg &srcAccessMask, const PcbLuaArg &dstAccessMask) {
		  pcb_recordBufferBarrier(pcb, buf, srcStageMask, dstStageMask, srcAccessMask, dstAccessMask, PcbLuaArg::CreateValue<uint32_t>(l, 0), PcbLuaArg::CreateValue<uint32_t>(l, buf.GetSize()));
	  });
	defPcb.def(
	  "RecordSetStencilReference", +[](prosper::util::PreparedCommandBuffer &pcb, const Lua::Vulkan::PreparedCommandLuaArg &faceMask, const Lua::Vulkan::PreparedCommandLuaArg &stencilReference) {
		  pcb.PushCommand(
		    [](const prosper::util::PreparedCommandBufferRecordState &recordState) -> bool {
			    auto &cmdBuf = recordState.commandBuffer;
			    return cmdBuf.RecordSetStencilReference(static_cast<prosper::StencilFaceFlags>(recordState.GetArgument<uint8_t>(0)), recordState.GetArgument<uint32_t>(1));
		    },
		    std::move(pragma::util::make_vector<PcbArg>(make_pcb_arg(faceMask, udm::Type::UInt8), make_pcb_arg(stencilReference, udm::Type::UInt32))));
	  });
	defPcb.def(
	  "RecordSetLineWidth", +[](prosper::util::PreparedCommandBuffer &pcb, const Lua::Vulkan::PreparedCommandLuaArg &lineWidth) {
		  pcb.PushCommand(
		    [](const prosper::util::PreparedCommandBufferRecordState &recordState) -> bool {
			    auto &cmdBuf = recordState.commandBuffer;
			    return cmdBuf.RecordSetLineWidth(recordState.GetArgument<float>(0));
		    },
		    std::move(pragma::util::make_vector<PcbArg>(make_pcb_arg(lineWidth, udm::Type::Float))));
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
	  +[](lua::State *l, prosper::util::PreparedCommandBuffer &pcb, const std::shared_ptr<prosper::IDescriptorSetGroup> &descSet, const PcbLuaArg &firstSet) -> bool { return pcb_record_bind_descriptor_set(pcb, descSet, firstSet, PcbLuaArg::CreateValue<uint32_t>(l, 0)); });
	defPcb.def(
	  "RecordBindDescriptorSet",
	  +[](lua::State *l, prosper::util::PreparedCommandBuffer &pcb, const std::shared_ptr<prosper::IDescriptorSetGroup> &descSet) -> bool { return pcb_record_bind_descriptor_set(pcb, descSet, PcbLuaArg::CreateValue<uint32_t>(l, 0), PcbLuaArg::CreateValue<uint32_t>(l, 0)); });
	defPcb.def("RecordCommands", +[](prosper::util::PreparedCommandBuffer &pcb, prosper::ICommandBuffer &cmd) -> bool { return pcb.RecordCommands(cmd, {}, {}); });
	defPcb.def_readonly("enableDrawArgs", &prosper::util::PreparedCommandBuffer::enableDrawArgs);

	auto defPcbDa = luabind::class_<Lua::Vulkan::PreparedCommandLuaDynamicArg>("DynArg");
	defPcbDa.def(luabind::constructor<std::string>());
	defPcb.scope[defPcbDa];

	prosperMod[defPcb];

	pragma::LuaCore::define_custom_constructor<prosper::util::PreparedCommandBuffer, +[]() -> std::shared_ptr<prosper::util::PreparedCommandBuffer> { return pragma::util::make_shared<prosper::util::PreparedCommandBuffer>(); }>(lua.GetState());
}
