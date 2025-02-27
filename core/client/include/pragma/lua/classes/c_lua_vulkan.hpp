/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2025 Silverlan
 */

#ifndef __C_LUA_VULKAN_HPP__
#define __C_LUA_VULKAN_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/shaders/image/c_shader_gradient.hpp"
#include <buffers/prosper_buffer.hpp>

namespace Lua {
	namespace Vulkan {
		std::vector<pragma::ShaderGradient::Node> get_gradient_nodes(lua_State *l, const luabind::tableT<void> &tNodes);
		namespace VKFramebuffer {
			bool IsValid(lua_State *l, Framebuffer &hFramebuffer);
#if 0
			void GetAttachment(lua_State *l,Framebuffer &hFramebuffer,uint32_t attId);
			void GetAttachmentCount(lua_State *l,Framebuffer &hFramebuffer);
#endif
		};
		namespace VKRenderPass {
			bool IsValid(lua_State *l, RenderPass &hRenderPass);
#if 0
			void GetInfo(lua_State *l,RenderPass &hRenderPass);
			void GetSwapchain(lua_State *l,RenderPass &hRenderPass);
#endif
		};
		namespace VKEvent {
			bool IsValid(lua_State *l, Event &hEvent);
			prosper::Result GetStatus(lua_State *l, Event &hEvent);
		};
		namespace VKFence {
			bool IsValid(lua_State *l, Fence &hFence);
		};
		namespace VKCommandBuffer {
			bool IsValid(lua_State *l, CommandBuffer &hCommandBuffer);
			bool RecordClearImage(lua_State *l, CommandBuffer &hCommandBuffer, Image &img, const Color &col, const prosper::util::ClearImageInfo &clearImageInfo = {});
			bool RecordClearImage(lua_State *l, CommandBuffer &hCommandBuffer, Image &img, std::optional<float> clearDepth, std::optional<uint32_t> clearStencil, const prosper::util::ClearImageInfo &clearImageInfo = {});
			bool RecordClearAttachment(lua_State *l, CommandBuffer &hCommandBuffer, Image &img, const Color &col, uint32_t attId = 0u);
			bool RecordClearAttachment(lua_State *l, CommandBuffer &hCommandBuffer, Image &img, std::optional<float> clearDepth, std::optional<uint32_t> clearStencil);
			bool RecordCopyImage(lua_State *l, CommandBuffer &hCommandBuffer, Image &imgSrc, Image &imgDst, const prosper::util::CopyInfo &copyInfo);
			bool RecordCopyBufferToImage(lua_State *l, CommandBuffer &hCommandBuffer, Buffer &bufSrc, Image &imgDst, const prosper::util::BufferImageCopyInfo &copyInfo);
			bool RecordCopyBuffer(lua_State *l, CommandBuffer &hCommandBuffer, Buffer &bufSrc, Buffer &bufDst, const prosper::util::BufferCopy &copyInfo);
			bool RecordUpdateBuffer(lua_State *l, CommandBuffer &hCommandBuffer, Buffer &buf, uint32_t offset, ::DataStream &ds);
			bool RecordUpdateBuffer(lua_State *l, CommandBuffer &hCommandBuffer, Buffer &buf, uint32_t offset, ::udm::Type type, Lua::udm_ng value);
			bool RecordBlitImage(lua_State *l, CommandBuffer &hCommandBuffer, Image &imgSrc, Image &imgDst, const prosper::util::BlitInfo &blitInfo);
			bool RecordResolveImage(lua_State *l, CommandBuffer &hCommandBuffer, Image &imgSrc, Image &imgDst);
			bool RecordBlitTexture(lua_State *l, CommandBuffer &hCommandBuffer, Texture &texSrc, Image &imgDst);
			bool RecordGenerateMipmaps(lua_State *l, CommandBuffer &hCommandBuffer, Image &img, uint32_t currentLayout, uint32_t srcAccessMask, uint32_t srcStage);
			bool RecordPipelineBarrier(lua_State *l, CommandBuffer &hCommandBuffer, const prosper::util::PipelineBarrierInfo &barrierInfo);
			bool RecordImageBarrier(lua_State *l, CommandBuffer &hCommandBuffer, Image &img, uint32_t srcStageMask, uint32_t dstStageMask, uint32_t oldLayout, uint32_t newLayout, uint32_t srcAccessMask, uint32_t dstAccessMask, uint32_t baseLayer);
			bool RecordImageBarrier(lua_State *l, CommandBuffer &hCommandBuffer, Image &img, uint32_t oldLayout, uint32_t newLayout, const prosper::util::ImageSubresourceRange &subresourceRange);
			bool RecordBufferBarrier(lua_State *l, CommandBuffer &hCommandBuffer, Buffer &buf, uint32_t srcStageMask, uint32_t dstStageMask, uint32_t srcAccessMask, uint32_t dstAccessMask, uint32_t offset = 0ull, uint32_t size = std::numeric_limits<uint32_t>::max());
			bool RecordSetViewport(lua_State *l, CommandBuffer &hCommandBuffer, uint32_t width, uint32_t height, uint32_t x = 0u, uint32_t y = 0u);
			bool RecordSetScissor(lua_State *l, CommandBuffer &hCommandBuffer, uint32_t width, uint32_t height, uint32_t x = 0u, uint32_t y = 0u);
			bool RecordBeginRenderPass(lua_State *l, CommandBuffer &hCommandBuffer, Lua::Vulkan::RenderPassInfo &rpInfo);
			bool RecordEndRenderPass(lua_State *l, CommandBuffer &hCommandBuffer);
			bool RecordBindIndexBuffer(lua_State *l, CommandBuffer &hCommandBuffer, Buffer &indexBuffer, uint32_t indexType, uint32_t offset);
			bool RecordBindVertexBuffer(lua_State *l, CommandBuffer &hCommandBuffer, prosper::ShaderGraphics &, Buffer &vertexBuffer, uint32_t startBinding, uint32_t offset);
			bool RecordBindVertexBuffers(lua_State *l, CommandBuffer &hCommandBuffer, prosper::ShaderGraphics &, const std::vector<prosper::IBuffer *> &buffers, uint32_t startBinding, const std::vector<uint64_t> &voffsets);
			bool RecordBindVertexBuffers(lua_State *l, CommandBuffer &hCommandBuffer, prosper::ShaderGraphics &, const std::vector<prosper::IBuffer *> &buffers, uint32_t startBinding);
			bool RecordBindVertexBuffers(lua_State *l, CommandBuffer &hCommandBuffer, prosper::ShaderGraphics &, const std::vector<prosper::IBuffer *> &buffers);
			bool RecordCopyImageToBuffer(lua_State *l, CommandBuffer &hCommandBuffer, Image &imgSrc, uint32_t srcImageLayout, Buffer &bufDst, const prosper::util::BufferImageCopyInfo &copyInfo);
			//void RecordDispatch(lua_State *l,CommandBuffer &hCommandBuffer,uint32_t x,uint32_t y,uint32_t z);
			//void RecordDispatchIndirect(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &buffer,uint32_t offset);
			bool RecordDraw(lua_State *l, CommandBuffer &hCommandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
			bool RecordDrawIndexed(lua_State *l, CommandBuffer &hCommandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t firstInstance);
			bool RecordDrawIndexedIndirect(lua_State *l, CommandBuffer &hCommandBuffer, Buffer &buffer, uint32_t offset, uint32_t drawCount, uint32_t stride);
			bool RecordDrawIndirect(lua_State *l, CommandBuffer &hCommandBuffer, Buffer &buffer, uint32_t offset, uint32_t drawCount, uint32_t stride);
			bool RecordFillBuffer(lua_State *l, CommandBuffer &hCommandBuffer, Buffer &buffer, uint32_t offset, uint32_t size, uint32_t data);
			//void RecordResetEvent(lua_State *l,CommandBuffer &hCommandBuffer,Event &ev,uint32_t stageMask);
			bool RecordSetBlendConstants(lua_State *l, CommandBuffer &hCommandBuffer, const Vector4 &blendConstants);
			bool RecordSetDepthBias(lua_State *l, CommandBuffer &hCommandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float slopeScaledDepthBias);
			bool RecordSetDepthBounds(lua_State *l, CommandBuffer &hCommandBuffer, float minDepthBounds, float maxDepthBounds);
			//void RecordSetEvent(lua_State *l,CommandBuffer &hCommandBuffer,Event &ev,uint32_t stageMask);
			bool RecordSetLineWidth(lua_State *l, CommandBuffer &hCommandBuffer, float lineWidt);
			bool RecordSetStencilCompareMask(lua_State *l, CommandBuffer &hCommandBuffer, uint32_t faceMask, uint32_t stencilCompareMask);
			bool RecordSetStencilReference(lua_State *l, CommandBuffer &hCommandBuffer, uint32_t faceMask, uint32_t stencilReference);
			bool RecordSetStencilWriteMask(lua_State *l, CommandBuffer &hCommandBuffer, uint32_t faceMask, uint32_t stencilWriteMask);
			//void RecordWaitEvents(lua_State *l,CommandBuffer &hCommandBuffer); // TODO
			//void RecordWriteTimestamp(lua_State *l,CommandBuffer &hCommandBuffer); // TODO
			//void RecordBeginQuery(lua_State *l,CommandBuffer &hCommandBuffer); // TODO
			//void RecordEndQuery(lua_State *l,CommandBuffer &hCommandBuffer); // TODO
			bool RecordDrawGradient(lua_State *l, CommandBuffer &hCommandBuffer, RenderTarget &rt, const Vector2 &dir, luabind::object lnodes);
			bool StartRecording(lua_State *l, CommandBuffer &hCommandBuffer, bool oneTimeSubmit, bool simultaneousUseAllowed);
		};
		namespace VKContextObject {
			void SetDebugName(lua_State *l, prosper::ContextObject &o, const std::string &name);
			std::string GetDebugName(lua_State *l, const prosper::ContextObject &o);
		};
		namespace VKBuffer {
			bool IsValid(lua_State *l, Buffer &hBuffer);
			bool Write(lua_State *l, Buffer &hBuffer, uint32_t offset, ::DataStream &ds, uint32_t dsOffset, uint32_t dsSize);
			bool Write(lua_State *l, Buffer &hBuffer, uint32_t offset, ::udm::Type type, Lua::udm_ng value);
			Lua::opt<::DataStream> Read(lua_State *l, Buffer &hBuffer, uint32_t offset, uint32_t size);
			bool Read(lua_State *l, Buffer &hBuffer, uint32_t offset, uint32_t size, ::DataStream &ds, uint32_t dsOffset);
			bool Read(lua_State *l, Buffer &hBuffer, uint32_t offset, ::udm::Type type, Lua::udm_ng value);
			bool Map(lua_State *l, Buffer &hBuffer, uint32_t offset, uint32_t size, Buffer::MapFlags mapFlags);
		};
		namespace VKDescriptorSet {
			bool IsValid(lua_State *l, DescriptorSet &hDescSet);
#if 0
			void GetBindingInfo(lua_State *l,DescriptorSet &hDescSet);
			void GetBindingInfo(lua_State *l,DescriptorSet &hDescSet,uint32_t bindingIdx);
#endif
			bool SetBindingTexture(lua_State *l, DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Texture &texture, uint32_t layerId);
			bool SetBindingTexture(lua_State *l, DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Texture &texture);
			prosper::Texture *GetBindingTexture(lua_State *l, DescriptorSet &hDescSet, uint32_t bindingIdx);
			bool SetBindingArrayTexture(lua_State *l, DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Texture &texture, uint32_t arrayIdx, uint32_t layerId);
			bool SetBindingArrayTexture(lua_State *l, DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Texture &texture, uint32_t arrayIdx);
			bool SetBindingStorageBuffer(lua_State *l, DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Buffer &buffer, uint32_t startOffset, uint32_t size);
			bool SetBindingUniformBuffer(lua_State *l, DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Buffer &buffer, uint32_t startOffset, uint32_t size);
			bool SetBindingUniformBufferDynamic(lua_State *l, DescriptorSet &hDescSet, uint32_t bindingIdx, Lua::Vulkan::Buffer &buffer, uint32_t startOffset, uint32_t size);
			//void GetLayout(lua_State *l,DescriptorSet &hDescSet); // TODO
			//void GetBindingProperties(lua_State *l,DescriptorSet &hDescSet); // TODO
		};
	};
};

#endif
