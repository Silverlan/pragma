// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:scripting.lua.classes.vulkan;

export import :rendering.shaders;
export import :scripting.lua.libraries.vulkan;
export import :util;

export namespace Lua {
	namespace Vulkan {
		std::vector<pragma::ShaderGradient::Node> get_gradient_nodes(lua::State *l, const luabind::tableT<void> &tNodes);
		namespace VKFramebuffer {
			bool IsValid(lua::State *l, Framebuffer &hFramebuffer);
#if 0
			void GetAttachment(lua::State *l,Framebuffer &hFramebuffer,uint32_t attId);
			void GetAttachmentCount(lua::State *l,Framebuffer &hFramebuffer);
#endif
		};
		namespace VKRenderPass {
			bool IsValid(lua::State *l, RenderPass &hRenderPass);
#if 0
			void GetInfo(lua::State *l,RenderPass &hRenderPass);
			void GetSwapchain(lua::State *l,RenderPass &hRenderPass);
#endif
		};
		namespace VKEvent {
			bool IsValid(lua::State *l, Event &hEvent);
			prosper::Result GetStatus(lua::State *l, Event &hEvent);
		};
		namespace VKFence {
			bool IsValid(lua::State *l, Fence &hFence);
		};
		namespace VKCommandBuffer {
			bool IsValid(lua::State *l, CommandBuffer &hCommandBuffer);
			bool RecordClearImage(lua::State *l, CommandBuffer &hCommandBuffer, Image &img, const ::Color &col, const prosper::util::ClearImageInfo &clearImageInfo = {});
			bool RecordClearImage(lua::State *l, CommandBuffer &hCommandBuffer, Image &img, std::optional<float> clearDepth, std::optional<uint32_t> clearStencil, const prosper::util::ClearImageInfo &clearImageInfo = {});
			bool RecordClearAttachment(lua::State *l, CommandBuffer &hCommandBuffer, Image &img, const ::Color &col, uint32_t attId = 0u);
			bool RecordClearAttachment(lua::State *l, CommandBuffer &hCommandBuffer, Image &img, std::optional<float> clearDepth, std::optional<uint32_t> clearStencil);
			bool RecordCopyImage(lua::State *l, CommandBuffer &hCommandBuffer, Image &imgSrc, Image &imgDst, const prosper::util::CopyInfo &copyInfo);
			bool RecordCopyBufferToImage(lua::State *l, CommandBuffer &hCommandBuffer, Buffer &bufSrc, Image &imgDst, const prosper::util::BufferImageCopyInfo &copyInfo);
			bool RecordCopyBuffer(lua::State *l, CommandBuffer &hCommandBuffer, Buffer &bufSrc, Buffer &bufDst, const prosper::util::BufferCopy &copyInfo);
			bool RecordUpdateBuffer(lua::State *l, CommandBuffer &hCommandBuffer, Buffer &buf, uint32_t offset, pragma::util::DataStream &ds);
			bool RecordUpdateBuffer(lua::State *l, CommandBuffer &hCommandBuffer, Buffer &buf, uint32_t offset, ::udm::Type type, udm_ng value);
			bool RecordBlitImage(lua::State *l, CommandBuffer &hCommandBuffer, Image &imgSrc, Image &imgDst, const prosper::util::BlitInfo &blitInfo);
			bool RecordResolveImage(lua::State *l, CommandBuffer &hCommandBuffer, Image &imgSrc, Image &imgDst);
			bool RecordBlitTexture(lua::State *l, CommandBuffer &hCommandBuffer, Texture &texSrc, Image &imgDst);
			bool RecordGenerateMipmaps(lua::State *l, CommandBuffer &hCommandBuffer, Image &img, uint32_t currentLayout, uint32_t srcAccessMask, uint32_t srcStage);
			bool RecordPipelineBarrier(lua::State *l, CommandBuffer &hCommandBuffer, const prosper::util::PipelineBarrierInfo &barrierInfo);
			bool RecordImageBarrier(lua::State *l, CommandBuffer &hCommandBuffer, Image &img, uint32_t srcStageMask, uint32_t dstStageMask, uint32_t oldLayout, uint32_t newLayout, uint32_t srcAccessMask, uint32_t dstAccessMask, uint32_t baseLayer);
			bool RecordImageBarrier(lua::State *l, CommandBuffer &hCommandBuffer, Image &img, uint32_t oldLayout, uint32_t newLayout, const prosper::util::ImageSubresourceRange &subresourceRange);
			bool RecordBufferBarrier(lua::State *l, CommandBuffer &hCommandBuffer, Buffer &buf, uint32_t srcStageMask, uint32_t dstStageMask, uint32_t srcAccessMask, uint32_t dstAccessMask, uint32_t offset = 0ull, uint32_t size = std::numeric_limits<uint32_t>::max());
			bool RecordSetViewport(lua::State *l, CommandBuffer &hCommandBuffer, uint32_t width, uint32_t height, uint32_t x = 0u, uint32_t y = 0u);
			bool RecordSetScissor(lua::State *l, CommandBuffer &hCommandBuffer, uint32_t width, uint32_t height, uint32_t x = 0u, uint32_t y = 0u);
			bool RecordBeginRenderPass(lua::State *l, CommandBuffer &hCommandBuffer, RenderPassInfo &rpInfo);
			bool RecordEndRenderPass(lua::State *l, CommandBuffer &hCommandBuffer);
			bool RecordBindIndexBuffer(lua::State *l, CommandBuffer &hCommandBuffer, Buffer &indexBuffer, uint32_t indexType, uint32_t offset);
			bool RecordBindVertexBuffer(lua::State *l, CommandBuffer &hCommandBuffer, prosper::ShaderGraphics &, Buffer &vertexBuffer, uint32_t startBinding, uint32_t offset);
			bool RecordBindVertexBuffers(lua::State *l, CommandBuffer &hCommandBuffer, prosper::ShaderGraphics &, const std::vector<prosper::IBuffer *> &buffers, uint32_t startBinding, const std::vector<uint64_t> &voffsets);
			bool RecordBindVertexBuffers(lua::State *l, CommandBuffer &hCommandBuffer, prosper::ShaderGraphics &, const std::vector<prosper::IBuffer *> &buffers, uint32_t startBinding);
			bool RecordBindVertexBuffers(lua::State *l, CommandBuffer &hCommandBuffer, prosper::ShaderGraphics &, const std::vector<prosper::IBuffer *> &buffers);
			bool RecordCopyImageToBuffer(lua::State *l, CommandBuffer &hCommandBuffer, Image &imgSrc, uint32_t srcImageLayout, Buffer &bufDst, const prosper::util::BufferImageCopyInfo &copyInfo);
			//void RecordDispatch(lua::State *l,CommandBuffer &hCommandBuffer,uint32_t x,uint32_t y,uint32_t z);
			//void RecordDispatchIndirect(lua::State *l,CommandBuffer &hCommandBuffer,Buffer &buffer,uint32_t offset);
			bool RecordDraw(lua::State *l, CommandBuffer &hCommandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
			bool RecordDrawIndexed(lua::State *l, CommandBuffer &hCommandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t firstInstance);
			bool RecordDrawIndexedIndirect(lua::State *l, CommandBuffer &hCommandBuffer, Buffer &buffer, uint32_t offset, uint32_t drawCount, uint32_t stride);
			bool RecordDrawIndirect(lua::State *l, CommandBuffer &hCommandBuffer, Buffer &buffer, uint32_t offset, uint32_t drawCount, uint32_t stride);
			bool RecordFillBuffer(lua::State *l, CommandBuffer &hCommandBuffer, Buffer &buffer, uint32_t offset, uint32_t size, uint32_t data);
			//void RecordResetEvent(lua::State *l,CommandBuffer &hCommandBuffer,Event &ev,uint32_t stageMask);
			bool RecordSetBlendConstants(lua::State *l, CommandBuffer &hCommandBuffer, const ::Vector4 &blendConstants);
			bool RecordSetDepthBias(lua::State *l, CommandBuffer &hCommandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float slopeScaledDepthBias);
			bool RecordSetDepthBounds(lua::State *l, CommandBuffer &hCommandBuffer, float minDepthBounds, float maxDepthBounds);
			//void RecordSetEvent(lua::State *l,CommandBuffer &hCommandBuffer,Event &ev,uint32_t stageMask);
			bool RecordSetLineWidth(lua::State *l, CommandBuffer &hCommandBuffer, float lineWidt);
			bool RecordSetStencilCompareMask(lua::State *l, CommandBuffer &hCommandBuffer, uint32_t faceMask, uint32_t stencilCompareMask);
			bool RecordSetStencilReference(lua::State *l, CommandBuffer &hCommandBuffer, uint32_t faceMask, uint32_t stencilReference);
			bool RecordSetStencilWriteMask(lua::State *l, CommandBuffer &hCommandBuffer, uint32_t faceMask, uint32_t stencilWriteMask);
			//void RecordWaitEvents(lua::State *l,CommandBuffer &hCommandBuffer); // TODO
			//void RecordWriteTimestamp(lua::State *l,CommandBuffer &hCommandBuffer); // TODO
			//void RecordBeginQuery(lua::State *l,CommandBuffer &hCommandBuffer); // TODO
			//void RecordEndQuery(lua::State *l,CommandBuffer &hCommandBuffer); // TODO
			bool RecordDrawGradient(lua::State *l, CommandBuffer &hCommandBuffer, RenderTarget &rt, const ::Vector2 &dir, luabind::object lnodes);
			bool StartRecording(lua::State *l, CommandBuffer &hCommandBuffer, bool oneTimeSubmit, bool simultaneousUseAllowed);
		};
		namespace VKContextObject {
			void SetDebugName(lua::State *l, prosper::ContextObject &o, const std::string &name);
			std::string GetDebugName(lua::State *l, const prosper::ContextObject &o);
		};
		namespace VKBuffer {
			bool IsValid(lua::State *l, Buffer &hBuffer);
			bool Write(lua::State *l, Buffer &hBuffer, uint32_t offset, pragma::util::DataStream &ds, uint32_t dsOffset, uint32_t dsSize);
			bool Write(lua::State *l, Buffer &hBuffer, uint32_t offset, ::udm::Type type, udm_ng value);
			opt<pragma::util::DataStream> Read(lua::State *l, Buffer &hBuffer, uint32_t offset, uint32_t size);
			bool Read(lua::State *l, Buffer &hBuffer, uint32_t offset, uint32_t size, pragma::util::DataStream &ds, uint32_t dsOffset);
			bool Read(lua::State *l, Buffer &hBuffer, uint32_t offset, ::udm::Type type, udm_ng value);
			bool Map(lua::State *l, Buffer &hBuffer, uint32_t offset, uint32_t size, Buffer::MapFlags mapFlags);
		};
		namespace VKDescriptorSet {
			bool IsValid(lua::State *l, DescriptorSet &hDescSet);
#if 0
			void GetBindingInfo(lua::State *l,DescriptorSet &hDescSet);
			void GetBindingInfo(lua::State *l,DescriptorSet &hDescSet,uint32_t bindingIdx);
#endif
			bool SetBindingTexture(lua::State *l, DescriptorSet &hDescSet, uint32_t bindingIdx, Texture &texture, uint32_t layerId);
			bool SetBindingTexture(lua::State *l, DescriptorSet &hDescSet, uint32_t bindingIdx, Texture &texture);
			prosper::Texture *GetBindingTexture(lua::State *l, DescriptorSet &hDescSet, uint32_t bindingIdx);
			bool SetBindingArrayTexture(lua::State *l, DescriptorSet &hDescSet, uint32_t bindingIdx, Texture &texture, uint32_t arrayIdx, uint32_t layerId);
			bool SetBindingArrayTexture(lua::State *l, DescriptorSet &hDescSet, uint32_t bindingIdx, Texture &texture, uint32_t arrayIdx);
			bool SetBindingStorageBuffer(lua::State *l, DescriptorSet &hDescSet, uint32_t bindingIdx, Buffer &buffer, uint32_t startOffset, uint32_t size);
			bool SetBindingUniformBuffer(lua::State *l, DescriptorSet &hDescSet, uint32_t bindingIdx, Buffer &buffer, uint32_t startOffset, uint32_t size);
			bool SetBindingUniformBufferDynamic(lua::State *l, DescriptorSet &hDescSet, uint32_t bindingIdx, Buffer &buffer, uint32_t startOffset, uint32_t size);
			//void GetLayout(lua::State *l,DescriptorSet &hDescSet); // TODO
			//void GetBindingProperties(lua::State *l,DescriptorSet &hDescSet); // TODO
		};
	};
};
