/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2025 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/lua/libraries/c_lua_vulkan.h"
#include "pragma/lua/classes/c_lua_vulkan.hpp"
#include <pragma/lua/converters/vector_converter_t.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <prosper_event.hpp>

extern DLLCLIENT CEngine *c_engine;

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

void Lua::Vulkan::VKContextObject::SetDebugName(lua_State *l, prosper::ContextObject &o, const std::string &name) { o.SetDebugName(name); }
std::string Lua::Vulkan::VKContextObject::GetDebugName(lua_State *l, const prosper::ContextObject &o) { return o.GetDebugName(); }

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

prosper::Texture *Lua::Vulkan::VKDescriptorSet::GetBindingTexture(lua_State *l, DescriptorSet &hDescSet, uint32_t bindingIdx)
{
	auto *binding = hDescSet.GetDescriptorSet()->GetBinding(bindingIdx);
	if(!binding)
		return nullptr;
	if(binding->GetType() != prosper::DescriptorSetBinding::Type::Texture)
		return nullptr;
	auto &tex = static_cast<prosper::DescriptorSetBindingTexture *>(binding)->GetTexture();
	if(tex.expired())
		return nullptr;
	return tex.lock().get();
}
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
