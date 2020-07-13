/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/lua/libraries/c_lua_vulkan.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/shaders/image/c_shader_gradient.hpp"
#include "pragma/rendering/shaders/c_shader_lua.hpp"
#include <pragma/lua/classes/ldef_color.h>
#include <pragma/lua/classes/ldef_vector.h>
#include <pragma/lua/classes/lerrorcode.h>
#include <pragma/lua/classes/ldatastream.h>
#include <pragma/lua/luaapi.h>
#include <luainterface.hpp>
#include <prosper_util.hpp>
#include <image/prosper_sampler.hpp>
#include <image/prosper_image_view.hpp>
#include <buffers/prosper_buffer.hpp>
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
#pragma optimize("",off)
extern DLLCENGINE CEngine *c_engine;
namespace Lua
{
	namespace Vulkan
	{
		DLLCLIENT int create_buffer(lua_State *l);
		DLLCLIENT int create_texture(lua_State *l);
		DLLCLIENT int create_descriptor_set(lua_State *l);
		DLLCLIENT int create_image(lua_State *l);
		DLLCLIENT int create_image_view(lua_State *l);
		DLLCLIENT int create_image_create_info(lua_State *l);
		DLLCLIENT int create_gradient_texture(lua_State *l);
		DLLCLIENT int create_event(lua_State *l);
		DLLCLIENT int create_fence(lua_State *l);
		DLLCLIENT int create_framebuffer(lua_State *l);
		DLLCLIENT int create_render_pass(lua_State *l);
		DLLCLIENT int create_render_target(lua_State *l);
		DLLCLIENT int calculate_mipmap_size(lua_State *l);
		DLLCLIENT int calculate_mipmap_count(lua_State *l);
		DLLCLIENT int result_to_string(lua_State *l);
		DLLCLIENT int format_to_string(lua_State *l);
		DLLCLIENT int shader_stage_to_string(lua_State *l);
		DLLCLIENT int is_depth_format(lua_State *l);
		DLLCLIENT int is_compressed_format(lua_State *l);
		DLLCLIENT int is_uncompressed_format(lua_State *l);
		DLLCLIENT int get_bit_size(lua_State *l);
		DLLCLIENT int get_byte_size(lua_State *l);
		DLLCLIENT int get_swapchain_image_count(lua_State *l);
		DLLCLIENT int wait_idle(lua_State *l);
		DLLCLIENT int get_square_vertex_uv_buffer(lua_State *l);
		DLLCLIENT int get_square_vertex_buffer(lua_State *l);
		DLLCLIENT int get_square_uv_buffer(lua_State *l);
		DLLCLIENT int get_square_vertices(lua_State *l);
		DLLCLIENT int get_square_uv_coordinates(lua_State *l);
		DLLCLIENT int get_square_vertex_count(lua_State *l);
		DLLCLIENT int get_square_vertex_format(lua_State *l);
		DLLCLIENT int get_square_uv_format(lua_State *l);
		DLLCLIENT int allocate_temporary_buffer(lua_State *l,uint32_t size);
		DLLCLIENT int allocate_temporary_buffer(lua_State *l,::DataStream &ds);
		DLLCLIENT int get_line_vertex_buffer(lua_State *l);
		DLLCLIENT int get_line_vertices(lua_State *l);
		DLLCLIENT int get_line_vertex_count(lua_State *l);
		DLLCLIENT int get_line_vertex_format(lua_State *l);
		namespace VKContextObject
		{
			template<class T>
				void SetDebugName(lua_State *l,T &o,const std::string &name,T&(*fCheck)(lua_State*,int))
			{
				fCheck(l,1);
				o.SetDebugName(name);
			}
			template<class T>
				void GetDebugName(lua_State *l,T &o,T&(*fCheck)(lua_State*,int))
			{
				fCheck(l,1);
				Lua::PushString(l,o.GetDebugName());
			}
		};
		namespace VKTexture
		{
			DLLCLIENT void IsValid(lua_State *l,Texture &hTex);
			DLLCLIENT void GetImage(lua_State *l,Texture &hTex);
			DLLCLIENT void GetImageView(lua_State *l,Texture &hTex,uint32_t layerId);
			DLLCLIENT void GetImageView(lua_State *l,Texture &hTex);
			DLLCLIENT void GetSampler(lua_State *l,Texture &hTex);
			DLLCLIENT void GetWidth(lua_State *l,Texture &hTex);
			DLLCLIENT void GetHeight(lua_State *l,Texture &hTex);
			DLLCLIENT void GetFormat(lua_State *l,Texture &hTex);
			DLLCLIENT void IsMSAATexture(lua_State *l,Texture &hTex);
		};
		namespace VKImage
		{
			DLLCLIENT void IsValid(lua_State *l,Image &hImg);
			DLLCLIENT void GetAspectSubresourceLayout(lua_State *l,Image &hImg,uint32_t layer=0u,uint32_t mipmap=0u);
			DLLCLIENT void GetAlignment(lua_State *l,Image &hImg);
			DLLCLIENT void GetExtent2D(lua_State *l,Image &hImg,uint32_t mipmap=0u);
#if 0
			DLLCLIENT void GetExtent3D(lua_State *l,Image &hImg,uint32_t mipmap=0u);
			DLLCLIENT void GetMemoryTypes(lua_State *l,Image &hImg);
			DLLCLIENT void GetStorageSize(lua_State *l,Image &hImg);
			DLLCLIENT void GetParentSwapchain(lua_State *l,Image &hImg);
			DLLCLIENT void GetMemoryBlock(lua_State *l,Image &hImg);
			DLLCLIENT void GetSubresourceRange(lua_State *l,Image &hImg);
#endif
			DLLCLIENT void GetFormat(lua_State *l,Image &hImg);
			DLLCLIENT void GetMipmapSize(lua_State *l,Image &hImg,uint32_t mipmap=0u);
			DLLCLIENT void GetLayerCount(lua_State *l,Image &hImg);
			DLLCLIENT void GetMipmapCount(lua_State *l,Image &hImg);
			DLLCLIENT void GetSampleCount(lua_State *l,Image &hImg);
			DLLCLIENT void GetSharingMode(lua_State *l,Image &hImg);
			DLLCLIENT void GetTiling(lua_State *l,Image &hImg);
			DLLCLIENT void GetType(lua_State *l,Image &hImg);
			DLLCLIENT void GetUsage(lua_State *l,Image &hImg);
			DLLCLIENT void GetWidth(lua_State *l,Image &hImg);
			DLLCLIENT void GetHeight(lua_State *l,Image &hImg);
		};
		namespace VKImageView
		{
			DLLCLIENT void IsValid(lua_State *l,ImageView &hImgView);
			DLLCLIENT void GetAspectMask(lua_State *l,ImageView &hImgView);
			DLLCLIENT void GetBaseLayer(lua_State *l,ImageView &hImgView);
			DLLCLIENT void GetBaseMipmapLevel(lua_State *l,ImageView &hImgView);
#if 0
			DLLCLIENT void GetBaseMipmapSize(lua_State *l,ImageView &hImgView);
			DLLCLIENT void GetParentImage(lua_State *l,ImageView &hImgView);
			DLLCLIENT void GetSubresourceRange(lua_State *l,ImageView &hImgView);
#endif
			DLLCLIENT void GetImageFormat(lua_State *l,ImageView &hImgView);
			DLLCLIENT void GetLayerCount(lua_State *l,ImageView &hImgView);
			DLLCLIENT void GetMipmapCount(lua_State *l,ImageView &hImgView);
			DLLCLIENT void GetSwizzleArray(lua_State *l,ImageView &hImgView);
			DLLCLIENT void GetType(lua_State *l,ImageView &hImgView);
		};
		namespace VKSampler
		{
			DLLCLIENT void IsValid(lua_State *l,Sampler &hSampler);
			DLLCLIENT void Update(lua_State *l,Sampler &hSampler);
			DLLCLIENT void GetMagFilter(lua_State *l,Sampler &hSampler);
			DLLCLIENT void GetMinFilter(lua_State *l,Sampler &hSampler);
			DLLCLIENT void GetMipmapMode(lua_State *l,Sampler &hSampler);
			DLLCLIENT void GetAddressModeU(lua_State *l,Sampler &hSampler);
			DLLCLIENT void GetAddressModeV(lua_State *l,Sampler &hSampler);
			DLLCLIENT void GetAddressModeW(lua_State *l,Sampler &hSampler);
			DLLCLIENT void GetMipLodBias(lua_State *l,Sampler &hSampler);
			DLLCLIENT void GetAnisotropyEnabled(lua_State *l,Sampler &hSampler);
			DLLCLIENT void GetMaxAnisotropy(lua_State *l,Sampler &hSampler);
			DLLCLIENT void GetCompareEnabled(lua_State *l,Sampler &hSampler);
			DLLCLIENT void GetCompareOperation(lua_State *l,Sampler &hSampler);
			DLLCLIENT void GetMinLod(lua_State *l,Sampler &hSampler);
			DLLCLIENT void GetMaxLod(lua_State *l,Sampler &hSampler);
			DLLCLIENT void GetBorderColor(lua_State *l,Sampler &hSampler);
			// DLLCLIENT void GetUseUnnormalizedCoordinates(lua_State *l,Sampler &hSampler);
			DLLCLIENT void SetMagFilter(lua_State *l,Sampler &hSampler,int32_t magFilter);
			DLLCLIENT void SetMinFilter(lua_State *l,Sampler &hSampler,int32_t minFilter);
			DLLCLIENT void SetMipmapMode(lua_State *l,Sampler &hSampler,int32_t mipmapMode);
			DLLCLIENT void SetAddressModeU(lua_State *l,Sampler &hSampler,int32_t addressModeU);
			DLLCLIENT void SetAddressModeV(lua_State *l,Sampler &hSampler,int32_t addressModeV);
			DLLCLIENT void SetAddressModeW(lua_State *l,Sampler &hSampler,int32_t addressModeW);
			DLLCLIENT void SetMipLodBias(lua_State *l,Sampler &hSampler,float mipLodBias);
			DLLCLIENT void SetAnisotropyEnabled(lua_State *l,Sampler &hSampler,bool anisotropyEnabled);
			DLLCLIENT void SetMaxAnisotropy(lua_State *l,Sampler &hSampler,float maxAnisotropy);
			DLLCLIENT void SetCompareEnabled(lua_State *l,Sampler &hSampler,bool compareEnabled);
			DLLCLIENT void SetCompareOperation(lua_State *l,Sampler &hSampler,int32_t compareOp);
			DLLCLIENT void SetMinLod(lua_State *l,Sampler &hSampler,float minLod);
			DLLCLIENT void SetMaxLod(lua_State *l,Sampler &hSampler,float maxLod);
			DLLCLIENT void SetBorderColor(lua_State *l,Sampler &hSampler,int32_t borderColor);
			// DLLCLIENT void SetUseUnnormalizedCoordinates(lua_State *l,Sampler &hSampler,bool bUnnormalizedCoordinates);
		};
		namespace VKFramebuffer
		{
			DLLCLIENT void IsValid(lua_State *l,Framebuffer &hFramebuffer);
#if 0
			DLLCLIENT void GetAttachment(lua_State *l,Framebuffer &hFramebuffer,uint32_t attId);
			DLLCLIENT void GetAttachmentCount(lua_State *l,Framebuffer &hFramebuffer);
#endif
			DLLCLIENT void GetSize(lua_State *l,Framebuffer &hFramebuffer);
		};
		namespace VKRenderPass
		{
			DLLCLIENT void IsValid(lua_State *l,RenderPass &hRenderPass);
#if 0
			DLLCLIENT void GetInfo(lua_State *l,RenderPass &hRenderPass);
			DLLCLIENT void GetSwapchain(lua_State *l,RenderPass &hRenderPass);
#endif
		};
		namespace VKEvent
		{
			DLLCLIENT void IsValid(lua_State *l,Event &hEvent);
			DLLCLIENT void GetStatus(lua_State *l,Event &hEvent);
			DLLCLIENT void IsSet(lua_State *l,Event &hEvent);
		};
		namespace VKFence
		{
			DLLCLIENT void IsValid(lua_State *l,Fence &hFence);

		};
#if 0
		namespace VKSemaphore
		{
			DLLCLIENT void IsValid(lua_State *l,Semaphore &hFence);
		};
		namespace VKMemory
		{
			DLLCLIENT void IsValid(lua_State *l,Memory &hMemory);
			DLLCLIENT void GetParentMemory(lua_State *l,Memory &hMemory);
			DLLCLIENT void GetSize(lua_State *l,Memory &hMemory);
			DLLCLIENT void GetStartOffset(lua_State *l,Memory &hMemory);
			DLLCLIENT void Write(lua_State *l,Memory &hMemory,uint32_t offset,::DataStream &ds,uint32_t dsOffset,uint32_t dsSize);
			DLLCLIENT void Read(lua_State *l,Memory &hMemory,uint32_t offset,uint32_t size);
			DLLCLIENT void Read(lua_State *l,Memory &hMemory,uint32_t offset,uint32_t size,::DataStream &ds,uint32_t dsOffset);
			DLLCLIENT void Map(lua_State *l,Memory &hMemory,uint32_t offset,uint32_t size);
			DLLCLIENT void Unmap(lua_State *l,Memory &hMemory);
		};
#endif
		namespace VKCommandBuffer
		{
			DLLCLIENT void IsValid(lua_State *l,CommandBuffer &hCommandBuffer);
			DLLCLIENT void RecordClearImage(lua_State *l,CommandBuffer &hCommandBuffer,Image &img,const Color &col,const prosper::util::ClearImageInfo &clearImageInfo={});
			DLLCLIENT void RecordClearImage(lua_State *l,CommandBuffer &hCommandBuffer,Image &img,float clearDepth,const prosper::util::ClearImageInfo &clearImageInfo={});
			DLLCLIENT void RecordClearAttachment(lua_State *l,CommandBuffer &hCommandBuffer,Image &img,const Color &col,uint32_t attId=0u);
			DLLCLIENT void RecordClearAttachment(lua_State *l,CommandBuffer &hCommandBuffer,Image &img,float clearDepth);
			DLLCLIENT void RecordCopyImage(lua_State *l,CommandBuffer &hCommandBuffer,Image &imgSrc,Image &imgDst,const prosper::util::CopyInfo &copyInfo);
			DLLCLIENT void RecordCopyBufferToImage(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &bufSrc,Image &imgDst,const prosper::util::BufferImageCopyInfo &copyInfo);
			DLLCLIENT void RecordCopyBuffer(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &bufSrc,Buffer &bufDst,const prosper::util::BufferCopy &copyInfo);
			DLLCLIENT void RecordUpdateBuffer(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &buf,uint32_t offset,::DataStream &ds);
			DLLCLIENT void RecordBlitImage(lua_State *l,CommandBuffer &hCommandBuffer,Image &imgSrc,Image &imgDst,const prosper::util::BlitInfo &blitInfo);
			DLLCLIENT void RecordResolveImage(lua_State *l,CommandBuffer &hCommandBuffer,Image &imgSrc,Image &imgDst);
			DLLCLIENT void RecordBlitTexture(lua_State *l,CommandBuffer &hCommandBuffer,Texture &texSrc,Image &imgDst);
			DLLCLIENT void RecordGenerateMipmaps(lua_State *l,CommandBuffer &hCommandBuffer,Image &img,uint32_t currentLayout,uint32_t srcAccessMask,uint32_t srcStage);
			DLLCLIENT void RecordPipelineBarrier(lua_State *l,CommandBuffer &hCommandBuffer,const prosper::util::PipelineBarrierInfo &barrierInfo);
			DLLCLIENT void RecordImageBarrier(
				lua_State *l,CommandBuffer &hCommandBuffer,Image &img,
				uint32_t srcStageMask,uint32_t dstStageMask,
				uint32_t oldLayout,uint32_t newLayout,
				uint32_t srcAccessMask,uint32_t dstAccessMask,
				uint32_t baseLayer
			);
			DLLCLIENT void RecordImageBarrier(
				lua_State *l,CommandBuffer &hCommandBuffer,Image &img,
				uint32_t oldLayout,uint32_t newLayout,const prosper::util::ImageSubresourceRange &subresourceRange
			);
			DLLCLIENT void RecordBufferBarrier(
				lua_State *l,CommandBuffer &hCommandBuffer,Buffer &buf,
				uint32_t srcStageMask,uint32_t dstStageMask,
				uint32_t srcAccessMask,uint32_t dstAccessMask,
				uint32_t offset=0ull,uint32_t size=std::numeric_limits<uint32_t>::max()
			);
			DLLCLIENT void RecordSetViewport(lua_State *l,CommandBuffer &hCommandBuffer,uint32_t width,uint32_t height,uint32_t x=0u,uint32_t y=0u);
			DLLCLIENT void RecordSetScissor(lua_State *l,CommandBuffer &hCommandBuffer,uint32_t width,uint32_t height,uint32_t x=0u,uint32_t y=0u);
			DLLCLIENT void RecordBeginRenderPass(lua_State *l,CommandBuffer &hCommandBuffer,Lua::Vulkan::RenderPassInfo &rpInfo);
			DLLCLIENT void RecordEndRenderPass(lua_State *l,CommandBuffer &hCommandBuffer);
			DLLCLIENT void RecordBindIndexBuffer(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &indexBuffer,uint32_t indexType,uint32_t offset);
			DLLCLIENT void RecordBindVertexBuffer(lua_State *l,CommandBuffer &hCommandBuffer,prosper::ShaderGraphics&,Buffer &vertexBuffer,uint32_t startBinding,uint32_t offset);
			DLLCLIENT void RecordBindVertexBuffers(lua_State *l,CommandBuffer &hCommandBuffer,prosper::ShaderGraphics&,luabind::object vertexBuffers,uint32_t startBinding,luabind::object offsets);
			DLLCLIENT void RecordCopyImageToBuffer(lua_State *l,CommandBuffer &hCommandBuffer,Image &imgSrc,uint32_t srcImageLayout,Buffer &bufDst,const prosper::util::BufferImageCopyInfo &copyInfo);
			//DLLCLIENT void RecordDispatch(lua_State *l,CommandBuffer &hCommandBuffer,uint32_t x,uint32_t y,uint32_t z);
			//DLLCLIENT void RecordDispatchIndirect(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &buffer,uint32_t offset);
			DLLCLIENT void RecordDraw(lua_State *l,CommandBuffer &hCommandBuffer,uint32_t vertexCount,uint32_t instanceCount,uint32_t firstVertex,uint32_t firstInstance);
			DLLCLIENT void RecordDrawIndexed(lua_State *l,CommandBuffer &hCommandBuffer,uint32_t indexCount,uint32_t instanceCount,uint32_t firstIndex,uint32_t firstInstance);
			DLLCLIENT void RecordDrawIndexedIndirect(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &buffer,uint32_t offset,uint32_t drawCount,uint32_t stride);
			DLLCLIENT void RecordDrawIndirect(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &buffer,uint32_t offset,uint32_t drawCount,uint32_t stride);
			DLLCLIENT void RecordFillBuffer(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &buffer,uint32_t offset,uint32_t size,uint32_t data);
			//DLLCLIENT void RecordResetEvent(lua_State *l,CommandBuffer &hCommandBuffer,Event &ev,uint32_t stageMask);
			DLLCLIENT void RecordSetBlendConstants(lua_State *l,CommandBuffer &hCommandBuffer,const Vector4 &blendConstants);
			DLLCLIENT void RecordSetDepthBias(lua_State *l,CommandBuffer &hCommandBuffer,float depthBiasConstantFactor,float depthBiasClamp,float slopeScaledDepthBias);
			DLLCLIENT void RecordSetDepthBounds(lua_State *l,CommandBuffer &hCommandBuffer,float minDepthBounds,float maxDepthBounds);
			//DLLCLIENT void RecordSetEvent(lua_State *l,CommandBuffer &hCommandBuffer,Event &ev,uint32_t stageMask);
			DLLCLIENT void RecordSetLineWidth(lua_State *l,CommandBuffer &hCommandBuffer,float lineWidt);
			DLLCLIENT void RecordSetStencilCompareMask(lua_State *l,CommandBuffer &hCommandBuffer,uint32_t faceMask,uint32_t stencilCompareMask);
			DLLCLIENT void RecordSetStencilReference(lua_State *l,CommandBuffer &hCommandBuffer,uint32_t faceMask,uint32_t stencilReference);
			DLLCLIENT void RecordSetStencilWriteMask(lua_State *l,CommandBuffer &hCommandBuffer,uint32_t faceMask,uint32_t stencilWriteMask);
			//DLLCLIENT void RecordWaitEvents(lua_State *l,CommandBuffer &hCommandBuffer); // TODO
			//DLLCLIENT void RecordWriteTimestamp(lua_State *l,CommandBuffer &hCommandBuffer); // TODO
			//DLLCLIENT void RecordBeginQuery(lua_State *l,CommandBuffer &hCommandBuffer); // TODO
			//DLLCLIENT void RecordEndQuery(lua_State *l,CommandBuffer &hCommandBuffer); // TODO
			DLLCLIENT void RecordDrawGradient(lua_State *l,CommandBuffer &hCommandBuffer,RenderTarget &rt,const Vector2 &dir,luabind::object lnodes);
			DLLCLIENT void StopRecording(lua_State *l,CommandBuffer &hCommandBuffer);
			DLLCLIENT void StartRecording(lua_State *l,CommandBuffer &hCommandBuffer,bool oneTimeSubmit,bool simultaneousUseAllowed);
		};
		namespace VKBuffer
		{
			DLLCLIENT void IsValid(lua_State *l,Buffer &hBuffer);
			DLLCLIENT void GetStartOffset(lua_State *l,Buffer &hBuffer);
			DLLCLIENT void GetBaseIndex(lua_State *l,Buffer &hBuffer);
			DLLCLIENT void GetSize(lua_State *l,Buffer &hBuffer);
			DLLCLIENT void GetUsageFlags(lua_State *l,Buffer &hBuffer);
			DLLCLIENT void SetPermanentlyMapped(lua_State *l,Buffer &hBuffer,bool b);
			DLLCLIENT void GetParent(lua_State *l,Buffer &hBuffer);
			DLLCLIENT void Write(lua_State *l,Buffer &hBuffer,uint32_t offset,::DataStream &ds,uint32_t dsOffset,uint32_t dsSize);
			DLLCLIENT void Read(lua_State *l,Buffer &hBuffer,uint32_t offset,uint32_t size);
			DLLCLIENT void Read(lua_State *l,Buffer &hBuffer,uint32_t offset,uint32_t size,::DataStream &ds,uint32_t dsOffset);
			DLLCLIENT void Map(lua_State *l,Buffer &hBuffer,uint32_t offset,uint32_t size);
			DLLCLIENT void Unmap(lua_State *l,Buffer &hBuffer);
		};
		namespace VKDescriptorSet
		{
			DLLCLIENT void IsValid(lua_State *l,DescriptorSet &hDescSet);
#if 0
			DLLCLIENT void GetBindingInfo(lua_State *l,DescriptorSet &hDescSet);
			DLLCLIENT void GetBindingInfo(lua_State *l,DescriptorSet &hDescSet,uint32_t bindingIdx);
#endif
			DLLCLIENT void GetBindingCount(lua_State *l,DescriptorSet &hDescSet);
			DLLCLIENT void SetBindingTexture(lua_State *l,DescriptorSet &hDescSet,uint32_t bindingIdx,Lua::Vulkan::Texture &texture,uint32_t layerId);
			DLLCLIENT void SetBindingTexture(lua_State *l,DescriptorSet &hDescSet,uint32_t bindingIdx,Lua::Vulkan::Texture &texture);
			DLLCLIENT void SetBindingArrayTexture(lua_State *l,DescriptorSet &hDescSet,uint32_t bindingIdx,Lua::Vulkan::Texture &texture,uint32_t arrayIdx,uint32_t layerId);
			DLLCLIENT void SetBindingArrayTexture(lua_State *l,DescriptorSet &hDescSet,uint32_t bindingIdx,Lua::Vulkan::Texture &texture,uint32_t arrayIdx);
			DLLCLIENT void SetBindingStorageBuffer(lua_State *l,DescriptorSet &hDescSet,uint32_t bindingIdx,Lua::Vulkan::Buffer &buffer,uint32_t startOffset,uint32_t size);
			DLLCLIENT void SetBindingUniformBuffer(lua_State *l,DescriptorSet &hDescSet,uint32_t bindingIdx,Lua::Vulkan::Buffer &buffer,uint32_t startOffset,uint32_t size);
			DLLCLIENT void SetBindingUniformBufferDynamic(lua_State *l,DescriptorSet &hDescSet,uint32_t bindingIdx,Lua::Vulkan::Buffer &buffer,uint32_t startOffset,uint32_t size);
			//DLLCLIENT void GetLayout(lua_State *l,DescriptorSet &hDescSet); // TODO
			//DLLCLIENT void GetBindingProperties(lua_State *l,DescriptorSet &hDescSet); // TODO
		};
		namespace VKMesh
		{
			DLLCLIENT void GetVertexBuffer(lua_State *l,pragma::VkMesh &mesh);
			DLLCLIENT void GetVertexWeightBuffer(lua_State *l,pragma::VkMesh &mesh);
			DLLCLIENT void GetAlphaBuffer(lua_State *l,pragma::VkMesh &mesh);
			DLLCLIENT void GetIndexBuffer(lua_State *l,pragma::VkMesh &mesh);
			DLLCLIENT void SetVertexBuffer(lua_State *l,pragma::VkMesh &mesh,Buffer &hImg);
			DLLCLIENT void SetVertexWeightBuffer(lua_State *l,pragma::VkMesh &mesh,Buffer &hImg);
			DLLCLIENT void SetAlphaBuffer(lua_State *l,pragma::VkMesh &mesh,Buffer &hImg);
			DLLCLIENT void SetIndexBuffer(lua_State *l,pragma::VkMesh &mesh,Buffer &hImg);
		};
		namespace VKRenderTarget
		{
			DLLCLIENT void IsValid(lua_State *l,RenderTarget &hRt);
			DLLCLIENT void GetWidth(lua_State *l,RenderTarget &hRt);
			DLLCLIENT void GetHeight(lua_State *l,RenderTarget &hRt);
			DLLCLIENT void GetFormat(lua_State *l,RenderTarget &hRt);
			DLLCLIENT void GetTexture(lua_State *l,RenderTarget &hRt,uint32_t idx);
			DLLCLIENT void GetRenderPass(lua_State *l,RenderTarget &hRt);
			DLLCLIENT void GetFramebuffer(lua_State *l,RenderTarget &hRt);
		};
		namespace VKTimestampQuery
		{
			DLLCLIENT void IsValid(lua_State *l,TimestampQuery &hTimestampQuery);
			DLLCLIENT void IsResultAvailable(lua_State *l,TimestampQuery &hTimestampQuery);
			DLLCLIENT void GetPipelineStage(lua_State *l,TimestampQuery &hTimestampQuery);
			DLLCLIENT void Write(lua_State *l,TimestampQuery &hTimestampQuery,CommandBuffer &cmdBuffer);
			DLLCLIENT void QueryResult(lua_State *l,TimestampQuery &hTimestampQuery);
		};
		namespace VKTimerQuery
		{
			DLLCLIENT void IsValid(lua_State *l,TimerQuery &hTimerQuery);
			DLLCLIENT void Begin(lua_State *l,TimerQuery &hTimerQuery,CommandBuffer &cmdBuffer);
			DLLCLIENT void End(lua_State *l,TimerQuery &hTimerQuery,CommandBuffer &cmdBuffer);
			DLLCLIENT void IsResultAvailable(lua_State *l,TimerQuery &hTimerQuery);
			DLLCLIENT void QueryResult(lua_State *l,TimerQuery &hTimerQuery);
		};
	};
};

DLLCLIENT std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::Image &hImg)
{
	out<<"VKImage[";
	auto extents = hImg.GetExtents();
	out<<extents.width<<"x"<<extents.height;
	out<<"][";
	auto format = hImg.GetFormat();
	out<<prosper::util::to_string(format);
	out<<"]";
	return out;
}

DLLCLIENT std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::ImageView &hImgView)
{
	out<<"VKImageView[";
	out<<"]";
	return out;
}

DLLCLIENT std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::Sampler &hSampler)
{
	out<<"VKSampler[";
	out<<"]";
	return out;
}

DLLCLIENT std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::Texture &hTex)
{
	out<<"VKTexture[";
	auto &img = hTex.GetImage();
	auto *imgView = hTex.GetImageView();
	auto *sampler = hTex.GetSampler();
	out<<img<<"][";
	if(imgView == nullptr)
		out<<"NULL][";
	else
		out<<imgView<<"][";
	if(sampler == nullptr)
		out<<"NULL";
	else
		out<<sampler;
	out<<"]";
	return out;
}

DLLCLIENT std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::Framebuffer &hFramebuffer)
{
	out<<"VKFramebuffer[";
	out<<"]";
	return out;
}

DLLCLIENT std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::RenderPass &hRenderPass)
{
	out<<"VKRenderPass[";
	out<<"]";
	return out;
}

DLLCLIENT std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::Event &hEvent)
{
	out<<"VKEvent[";
	auto r = prosper::Result::EventReset;
	if(hEvent.IsSet())
		r = prosper::Result::EventSet;
	out<<prosper::util::to_string(r);
	out<<"]";
	return out;
}

DLLCLIENT std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::Fence &hFence)
{
	out<<"VKFence[";
	out<<"]";
	return out;
}
#if 0
DLLCLIENT std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::Semaphore &hSemaphore)
{
	out<<"VKSemaphore[";
	out<<"]";
	return out;
}

DLLCLIENT std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::Memory &hMemory)
{
	out<<"VKMemory[";
	out<<"]";
	return out;
}
#endif
DLLCLIENT std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::CommandBuffer &hCommandBuffer)
{
	out<<"VKCommandBuffer[";
	out<<"]";
	return out;
}

DLLCLIENT std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::Buffer &hBuffer)
{
	out<<"VKBuffer[";
	out<<hBuffer.GetSize();
	out<<"]";
	return out;
}

DLLCLIENT std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::DescriptorSet &hDescSet)
{
	out<<"VKDescriptorSet[";
	out<<"]";
	return out;
}

DLLCLIENT std::ostream &operator<<(std::ostream &out,const pragma::VkMesh&)
{
	out<<"VKMesh";
	return out;
}

DLLCLIENT std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::RenderTarget &hRt)
{
	out<<"VKRenderTarget[";
	auto &tex = hRt.GetTexture();
	auto &framebuffer = hRt.GetFramebuffer();
	auto &renderPass = hRt.GetRenderPass();

	out<<tex<<"][";
	out<<framebuffer<<"][";
	out<<renderPass<<"][";
	out<<"]";
	return out;
}

DLLCLIENT std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::TimestampQuery &hTimestampQuery)
{
	out<<"VKTimestampQuery[";
	out<<hTimestampQuery.IsResultAvailable();
	out<<"]";
	return out;
}

DLLCLIENT std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::TimerQuery &hTimerQuery)
{
	out<<"VKTimerQuery[";
	out<<hTimerQuery.IsResultAvailable();
	out<<"]";
	return out;
}

namespace luabind { // For some reason these need to be in the luabind namespace, otherwise luabind can't locate them
static bool operator==(const Lua::Vulkan::Texture &a,const Lua::Vulkan::Texture &b) {return &a == &b;}
static bool operator==(const Lua::Vulkan::Image &a,const Lua::Vulkan::Image &b) {return &a == &b;}
static bool operator==(const Lua::Vulkan::ImageView &a,const Lua::Vulkan::ImageView &b) {return &a == &b;}
static bool operator==(const Lua::Vulkan::Sampler &a,const Lua::Vulkan::Sampler &b) {return &a == &b;}
static bool operator==(const Lua::Vulkan::Framebuffer &a,const Lua::Vulkan::Framebuffer &b) {return &a == &b;}
static bool operator==(const Lua::Vulkan::RenderPass &a,const Lua::Vulkan::RenderPass &b) {return &a == &b;}
static bool operator==(const Lua::Vulkan::Event &a,const Lua::Vulkan::Event &b) {return &a == &b;}
static bool operator==(const Lua::Vulkan::Fence &a,const Lua::Vulkan::Fence &b) {return &a == &b;}
static bool operator==(const Lua::Vulkan::CommandBuffer &a,const Lua::Vulkan::CommandBuffer &b) {return &a == &b;}
static bool operator==(const Lua::Vulkan::Buffer &a,const Lua::Vulkan::Buffer &b) {return &a == &b;}
static bool operator==(const Lua::Vulkan::DescriptorSet &a,const Lua::Vulkan::DescriptorSet &b) {return &a == &b;}
static bool operator==(const pragma::VkMesh &a,const pragma::VkMesh &b) {return &a == &b;}
static bool operator==(const Lua::Vulkan::RenderTarget &a,const Lua::Vulkan::RenderTarget &b) {return &a == &b;}
static bool operator==(const Lua::Vulkan::TimestampQuery &a,const Lua::Vulkan::TimestampQuery &b) {return &a == &b;}
static bool operator==(const Lua::Vulkan::TimerQuery &a,const Lua::Vulkan::TimerQuery &b) {return &a == &b;}
};

int Lua::Vulkan::create_buffer(lua_State *l)
{
	auto arg = 1;
	auto &bufCreateInfo = Lua::Check<prosper::util::BufferCreateInfo>(l,arg++);
	std::shared_ptr<Buffer> buf = nullptr;
	if(Lua::IsSet(l,arg) == false)
		buf = c_engine->GetRenderContext().CreateBuffer(bufCreateInfo);
	else
	{
		auto &ds = *Lua::CheckDataStream(l,arg++);
		buf = c_engine->GetRenderContext().CreateBuffer(bufCreateInfo,ds->GetData());
	}
	if(buf == nullptr)
		return 0;
	buf->SetDebugName("lua_buf");
	Lua::Push(l,buf);
	return 1;
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

int Lua::Vulkan::create_descriptor_set(lua_State *l)
{
	auto &ldescSetInfo = *Lua::CheckDescriptorSetInfo(l,1);
	auto shaderDescSetInfo = to_prosper_descriptor_set_info(ldescSetInfo);
	auto dsg = c_engine->GetRenderContext().CreateDescriptorSetGroup(shaderDescSetInfo);
	if(dsg == nullptr)
		return 0;
	dsg->SetDebugName("lua_dsg");
	Lua::Push(l,dsg);
	return 1;
}

int Lua::Vulkan::create_image_view(lua_State *l)
{
	auto &imgViewCreateInfo = Lua::Check<prosper::util::ImageViewCreateInfo>(l,1);
	auto &img = Lua::Check<prosper::IImage>(l,2);
	auto imgView = c_engine->GetRenderContext().CreateImageView(imgViewCreateInfo,img);
	Lua::Push(l,imgView);
	return 1;
}

int Lua::Vulkan::create_image_create_info(lua_State *l)
{
	auto &imgBuf = Lua::Check<uimg::ImageBuffer>(l,1);
	auto cubemap = false;
	if(Lua::IsSet(l,2))
		cubemap = Lua::CheckBool(l,2);
	Lua::Push(l,prosper::util::get_image_create_info(imgBuf,cubemap));
	return 1;
}

int Lua::Vulkan::create_image(lua_State *l)
{
	auto arg = 1;
	if(Lua::IsType<uimg::ImageBuffer>(l,arg))
	{
		if(Lua::IsTable(l,arg) == false)
		{
			auto &imgBuffer = Lua::Check<uimg::ImageBuffer>(l,arg++);
			std::shared_ptr<prosper::IImage> img = nullptr;
			if(Lua::IsSet(l,arg))
			{
				auto &imgCreateInfo = Lua::Check<prosper::util::ImageCreateInfo>(l,arg++);
				img = c_engine->GetRenderContext().CreateImage(imgBuffer,imgCreateInfo);
			}
			else
				img = c_engine->GetRenderContext().CreateImage(imgBuffer);
			if(img == nullptr)
				return 0;
			img->SetDebugName("lua_img");
			Lua::Push(l,img);
			return 1;
		}
	}
	else if(Lua::IsTable(l,arg))
	{
		std::array<std::shared_ptr<uimg::ImageBuffer>,6> imgBuffers {};
		auto t = arg;
		for(auto i=decltype(imgBuffers.size()){0u};i<imgBuffers.size();++i)
		{
			Lua::PushInt(l,i +1);
			Lua::GetTableValue(l,t);
			auto &imgBuf = Lua::Check<uimg::ImageBuffer>(l,-1);
			imgBuffers.at(i) = imgBuf.shared_from_this();
			Lua::Pop(l,1);
		}
		std::shared_ptr<prosper::IImage> img = nullptr;
		if(Lua::IsSet(l,arg))
		{
			auto &imgCreateInfo = Lua::Check<prosper::util::ImageCreateInfo>(l,arg++);
			img = c_engine->GetRenderContext().CreateCubemap(imgBuffers,imgCreateInfo);
		}
		else
			img = c_engine->GetRenderContext().CreateCubemap(imgBuffers);
		if(img == nullptr)
			return 0;
		img->SetDebugName("lua_img");
		Lua::Push(l,img);
		return 1;
	}
	auto &imgCreateInfo = Lua::Check<prosper::util::ImageCreateInfo>(l,arg++);
	std::shared_ptr<Image> img = nullptr;
	if(Lua::IsSet(l,arg) == false)
		img = c_engine->GetRenderContext().CreateImage(imgCreateInfo);
	else
	{
		auto &ds = *Lua::CheckDataStream(l,arg++);
		img = c_engine->GetRenderContext().CreateImage(imgCreateInfo,ds->GetData());
	}
	if(img == nullptr)
		return 0;
	img->SetDebugName("lua_img");
	Lua::Push(l,img);
	return 1;
}

int Lua::Vulkan::create_texture(lua_State *l)
{
	auto arg = 1;
	auto &img = Lua::Check<Image>(l,arg++);
	auto &texCreateInfo = Lua::Check<prosper::util::TextureCreateInfo>(l,arg++);
	std::optional<prosper::util::ImageViewCreateInfo> imgViewCreateInfo {};
	std::optional<prosper::util::SamplerCreateInfo> samplerCreateInfo {};
	if(Lua::IsSet(l,arg))
	{
		imgViewCreateInfo = Lua::Check<prosper::util::ImageViewCreateInfo>(l,arg++);
		if(Lua::IsSet(l,arg))
			samplerCreateInfo = Lua::Check<prosper::util::SamplerCreateInfo>(l,arg++);
	}
	auto tex = c_engine->GetRenderContext().CreateTexture(texCreateInfo,img,imgViewCreateInfo,samplerCreateInfo);
	if(tex == nullptr)
		return 0;
	tex->SetDebugName("lua_tex");
	Lua::Push(l,tex);
	return 1;
}

int Lua::Vulkan::create_framebuffer(lua_State *l)
{
	auto arg = 1;
	auto width = Lua::CheckInt(l,arg++);
	auto height = Lua::CheckInt(l,arg++);
	std::vector<prosper::IImageView*> attachments;
	auto tAttachments = arg++;
	Lua::CheckTable(l,tAttachments);
	auto numAttachments = Lua::GetObjectLength(l,tAttachments);
	attachments.reserve(numAttachments);
	for(auto i=decltype(numAttachments){0};i<numAttachments;++i)
	{
		Lua::PushInt(l,i +1); /* 1 */
		Lua::GetTableValue(l,tAttachments);

		attachments.push_back(&Lua::Check<ImageView>(l,-1));

		Lua::Pop(l,1); /* 0 */
	}
	auto layers = 1u;
	if(Lua::IsSet(l,arg))
		layers = Lua::CheckInt(l,arg++);
	auto fb = c_engine->GetRenderContext().CreateFramebuffer(width,height,layers,attachments);
	if(fb == nullptr)
		return 0;
	fb->SetDebugName("lua_fb");
	Lua::Push(l,fb);
	return 1;
}
int Lua::Vulkan::create_render_pass(lua_State *l)
{
	auto &rpCreateInfo = Lua::Check<prosper::util::RenderPassCreateInfo>(l,1);
	auto rp = c_engine->GetRenderContext().CreateRenderPass(rpCreateInfo);
	if(rp == nullptr)
		return 0;
	rp->SetDebugName("lua_rp");
	Lua::Push(l,rp);
	return 1;
}

int Lua::Vulkan::create_render_target(lua_State *l)
{
	auto arg = 1;
	auto &rtCreateInfo = Lua::Check<prosper::util::RenderTargetCreateInfo>(l,arg++);
	std::vector<std::shared_ptr<Texture>> textures;
	if(Lua::IsType<Texture>(l,arg))
		textures.push_back(Lua::Check<Texture>(l,arg++).shared_from_this());
	else
	{
		Lua::CheckTable(l,arg);
		auto t = arg;
		auto numTextures = Lua::GetObjectLength(l,arg);
		textures.reserve(numTextures);
		for(auto i=decltype(numTextures){0};i<numTextures;++i)
		{
			Lua::PushInt(l,i +1); /* 1 */
			Lua::GetTableValue(l,t);

			textures.push_back(Lua::Check<Texture>(l,-1).shared_from_this());

			Lua::Pop(l,1); /* 0 */
		}
		++arg;
	}
	std::shared_ptr<Lua::Vulkan::RenderPass> rp = nullptr;
	if(Lua::IsSet(l,arg))
		rp = Lua::Check<RenderPass>(l,arg++).shared_from_this();
	auto rt = c_engine->GetRenderContext().CreateRenderTarget(textures,rp,rtCreateInfo);
	if(rt == nullptr)
		return 0;
	rt->SetDebugName("lua_rt");
	Lua::Push(l,rt);
	return 1;
}

int Lua::Vulkan::create_event(lua_State *l)
{
	auto ev = c_engine->GetRenderContext().CreateEvent();
	if(ev == nullptr)
		return 0;
	Lua::Push(l,ev);
	return 1;
}

int Lua::Vulkan::create_fence(lua_State *l)
{
	auto bCreateSignalled = false;
	if(Lua::IsSet(l,1))
		bCreateSignalled = Lua::CheckBool(l,1);
	auto fence = c_engine->GetRenderContext().CreateFence(bCreateSignalled);
	if(fence == nullptr)
		return 0;
	Lua::Push(l,fence);
	return 1;
}

int Lua::Vulkan::calculate_mipmap_count(lua_State *l)
{
	auto w = Lua::CheckInt(l,1);
	auto h = Lua::CheckInt(l,2);
	auto mipmapCount = ::prosper::util::calculate_mipmap_count(static_cast<uint32_t>(w),static_cast<uint32_t>(h));
	Lua::PushInt(l,mipmapCount);
	return 1;
}

int Lua::Vulkan::calculate_mipmap_size(lua_State *l)
{
	if(Lua::IsSet(l,3))
	{
		auto w = Lua::CheckInt(l,1);
		auto h = Lua::CheckInt(l,2);
		auto level = Lua::CheckInt(l,3);
		uint32_t wMipmap;
		uint32_t hMipmap;
		::prosper::util::calculate_mipmap_size(static_cast<uint32_t>(w),static_cast<uint32_t>(h),&wMipmap,&hMipmap,static_cast<uint32_t>(level));
		Lua::Push<Vector2i>(l,Vector2{wMipmap,hMipmap});
		return 1;
	}
	auto v = Lua::CheckInt(l,1);
	auto level = Lua::CheckInt(l,1);
	Lua::PushInt(l,::prosper::util::calculate_mipmap_size(static_cast<uint32_t>(v),static_cast<uint32_t>(level)));
	return 1;
}
int Lua::Vulkan::result_to_string(lua_State *l)
{
	auto result = Lua::CheckInt(l,1);
	Lua::PushString(l,::prosper::util::to_string(static_cast<prosper::Result>(result)));
	return 1;
}
int Lua::Vulkan::format_to_string(lua_State *l)
{
	auto format = Lua::CheckInt(l,1);
	Lua::PushString(l,::prosper::util::to_string(static_cast<prosper::Format>(format)));
	return 1;
}
int Lua::Vulkan::shader_stage_to_string(lua_State *l)
{
	auto shaderStage = Lua::CheckInt(l,1);
	Lua::PushString(l,::prosper::util::to_string(static_cast<prosper::ShaderStageFlags>(shaderStage)));
	return 1;
}
int Lua::Vulkan::is_depth_format(lua_State *l)
{
	auto format = Lua::CheckInt(l,1);
	Lua::PushBool(l,::prosper::util::is_depth_format(static_cast<prosper::Format>(format)));
	return 1;
}
int Lua::Vulkan::is_compressed_format(lua_State *l)
{
	auto format = Lua::CheckInt(l,1);
	Lua::PushBool(l,::prosper::util::is_compressed_format(static_cast<prosper::Format>(format)));
	return 1;
}
int Lua::Vulkan::is_uncompressed_format(lua_State *l)
{
	auto format = Lua::CheckInt(l,1);
	Lua::PushBool(l,::prosper::util::is_uncompressed_format(static_cast<prosper::Format>(format)));
	return 1;
}
int Lua::Vulkan::get_bit_size(lua_State *l)
{
	auto format = Lua::CheckInt(l,1);
	Lua::PushInt(l,::prosper::util::get_bit_size(static_cast<prosper::Format>(format)));
	return 1;
}
int Lua::Vulkan::get_byte_size(lua_State *l)
{
	auto format = Lua::CheckInt(l,1);
	Lua::PushInt(l,::prosper::util::get_byte_size(static_cast<prosper::Format>(format)));
	return 1;
}
int Lua::Vulkan::get_swapchain_image_count(lua_State *l)
{
	Lua::PushInt(l,c_engine->GetRenderContext().GetSwapchainImageCount());
	return 1;
}
int Lua::Vulkan::wait_idle(lua_State *l)
{
	c_engine->GetRenderContext().WaitIdle();
	return 0;
}
int Lua::Vulkan::get_line_vertex_buffer(lua_State *l)
{
	auto buf = c_engine->GetRenderContext().GetCommonBufferCache().GetLineVertexBuffer();
	Lua::Push(l,buf);
	return 1;
}
int Lua::Vulkan::get_line_vertices(lua_State *l)
{
	auto &verts = prosper::CommonBufferCache::GetLineVertices();
	auto t = Lua::CreateTable(l);
	auto idx = 1u;
	for(auto &v : verts)
	{
		Lua::PushInt(l,idx++);
		Lua::Push<Vector2>(l,v);
		Lua::SetTableValue(l,t);
	}
	return 1;
}
int Lua::Vulkan::get_line_vertex_count(lua_State *l)
{
	auto numVerts = prosper::CommonBufferCache::GetLineVertexCount();
	Lua::PushInt(l,numVerts);
	return 1;
}
int Lua::Vulkan::get_line_vertex_format(lua_State *l)
{
	auto format = prosper::CommonBufferCache::GetLineVertexFormat();
	Lua::PushInt(l,umath::to_integral(format));
	return 1;
}
int Lua::Vulkan::get_square_vertex_uv_buffer(lua_State *l)
{
	auto uvBuffer = c_engine->GetRenderContext().GetCommonBufferCache().GetSquareVertexUvBuffer();
	Lua::Push(l,uvBuffer);
	return 1;
}
int Lua::Vulkan::get_square_vertex_buffer(lua_State *l)
{
	auto vertexBuffer = c_engine->GetRenderContext().GetCommonBufferCache().GetSquareVertexBuffer();
	Lua::Push(l,vertexBuffer);
	return 1;
}
int Lua::Vulkan::get_square_uv_buffer(lua_State *l)
{
	auto uvBuffer = c_engine->GetRenderContext().GetCommonBufferCache().GetSquareUvBuffer();
	Lua::Push(l,uvBuffer);
	return 1;
}
int Lua::Vulkan::get_square_vertices(lua_State *l)
{
	auto &squareVerts = prosper::CommonBufferCache::GetSquareVertices();
	auto t = Lua::CreateTable(l);
	auto idx = 1u;
	for(auto &v : squareVerts)
	{
		Lua::PushInt(l,idx++);
		Lua::Push<Vector2>(l,v);
		Lua::SetTableValue(l,t);
	}
	return 1;
}
int Lua::Vulkan::get_square_uv_coordinates(lua_State *l)
{
	auto &squareUvs = prosper::CommonBufferCache::GetSquareUvCoordinates();
	auto t = Lua::CreateTable(l);
	auto idx = 1u;
	for(auto &v : squareUvs)
	{
		Lua::PushInt(l,idx++);
		Lua::Push<Vector2>(l,v);
		Lua::SetTableValue(l,t);
	}
	return 1;
}
int Lua::Vulkan::get_square_vertex_count(lua_State *l)
{
	auto numVerts = prosper::CommonBufferCache::GetSquareVertexCount();
	Lua::PushInt(l,numVerts);
	return 1;
}
int Lua::Vulkan::get_square_vertex_format(lua_State *l)
{
	auto format = prosper::CommonBufferCache::GetSquareVertexFormat();
	Lua::PushInt(l,umath::to_integral(format));
	return 1;
}
int Lua::Vulkan::get_square_uv_format(lua_State *l)
{
	auto uvFormat = prosper::CommonBufferCache::GetSquareUvFormat();
	Lua::PushInt(l,umath::to_integral(uvFormat));
	return 1;
}
int Lua::Vulkan::allocate_temporary_buffer(lua_State *l,uint32_t size)
{
	auto buf = c_engine->GetRenderContext().AllocateTemporaryBuffer(size);
	if(buf == nullptr)
		return 0;
	Lua::Push(l,buf);
	return 1;
}
int Lua::Vulkan::allocate_temporary_buffer(lua_State *l,::DataStream &ds)
{
	auto buf = c_engine->GetRenderContext().AllocateTemporaryBuffer(ds->GetSize(),0u /* alignment */,ds->GetData());
	if(buf == nullptr)
		return 0;
	Lua::Push(l,buf);
	return 1;
}

static std::vector<pragma::ShaderGradient::Node> get_gradient_nodes(lua_State *l,uint32_t argIdx)
{
	Lua::CheckTable(l,argIdx);
	std::vector<pragma::ShaderGradient::Node> nodes;
	Lua::PushNil(l); /* 2 */
	while(Lua::GetNextPair(l,argIdx) != 0) /* 3 */
	{
		if(Lua::IsTable(l,-1))
		{
			Lua::PushString(l,"offset"); /* 4 */
			Lua::GetTableValue(l,-2); /* 4 */
			float offset = Lua::CheckNumber<float>(l,-1); /* 4 */
			Lua::Pop(l,1); /* 3 */

			Lua::PushString(l,"color"); /* 4 */
			Lua::GetTableValue(l,-2); /* 4 */
			auto *col = Lua::CheckColor(l,-1); /* 4 */
			Lua::Pop(l,1); /* 3 */
			nodes.push_back(pragma::ShaderGradient::Node(Vector4(col->r /255.f,col->g /255.f,col->b /255.f,col->a /255.f),offset));
		}
		Lua::Pop(l,1); /* 2 */
	} /* 1 */

	Lua::Pop(l); /* 0 */
	return nodes;
}
int Lua::Vulkan::create_gradient_texture(lua_State *l)
{
	auto whShader = c_engine->GetShader("gradient");
	if(whShader.expired())
		return 0;
	auto arg = 1;
	auto width = Lua::CheckInt(l,arg++);
	auto height = Lua::CheckInt(l,arg++);
	auto format = Lua::CheckInt(l,arg++);
	auto &dir = *Lua::CheckVector2(l,arg++);
	auto nodes = get_gradient_nodes(l,arg++);

	prosper::util::ImageCreateInfo createInfo {};
	createInfo.width = width;
	createInfo.height = height;
	createInfo.format = static_cast<prosper::Format>(format);
	createInfo.usage = prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::ColorAttachmentBit;
	createInfo.postCreateLayout = prosper::ImageLayout::ColorAttachmentOptimal;
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	auto img = c_engine->GetRenderContext().CreateImage(createInfo);
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	auto texture = c_engine->GetRenderContext().CreateTexture({},*img,imgViewCreateInfo,samplerCreateInfo);
	auto rt = c_engine->GetRenderContext().CreateRenderTarget({texture},static_cast<prosper::ShaderGraphics&>(*whShader.get()).GetRenderPass());
	rt->SetDebugName("lua_gradient");
	auto cb = FunctionCallback<void,std::reference_wrapper<std::shared_ptr<prosper::IPrimaryCommandBuffer>>>::Create(nullptr);
	static_cast<Callback<void,std::reference_wrapper<std::shared_ptr<prosper::IPrimaryCommandBuffer>>>*>(cb.get())->SetFunction([cb,rt,dir,nodes](std::reference_wrapper<std::shared_ptr<prosper::IPrimaryCommandBuffer>> drawCmd) mutable {
		c_engine->GetRenderContext().KeepResourceAliveUntilPresentationComplete(rt);
		pragma::util::record_draw_gradient(
			c_engine->GetRenderContext(),drawCmd.get(),*rt,dir,nodes
		);
		if(cb.IsValid())
			cb.Remove();
	});
	c_engine->AddCallback("DrawFrame",cb);
	Lua::Push(l,texture);
	return 1;
}

static void push_image_buffers(lua_State *l,uint32_t includeLayers,uint32_t includeMipmaps,const std::vector<std::vector<std::shared_ptr<uimg::ImageBuffer>>> &imgBuffers)
{
	if(imgBuffers.empty())
		return;
	if(includeLayers == false)
	{
		auto &layer = imgBuffers.front();
		if(layer.empty())
			return;
		if(includeMipmaps == false)
		{
			auto &imgBuf = layer.front();
			Lua::Push(l,imgBuf);
			return;
		}
		auto t = Lua::CreateTable(l);
		for(auto i=decltype(layer.size()){0u};i<layer.size();++i)
		{
			Lua::PushInt(l,i +1);
			Lua::Push(l,layer.at(i));
			Lua::SetTableValue(l,t);
		}
		return;
	}
	if(includeMipmaps == false)
	{
		auto t = Lua::CreateTable(l);
		for(auto i=decltype(imgBuffers.size()){0u};i<imgBuffers.size();++i)
		{
			auto &layer = imgBuffers.at(i);
			if(layer.empty())
				return;
			Lua::PushInt(l,i +1);
			Lua::Push(l,layer.at(i));
			Lua::SetTableValue(l,t);
		}
		return;
	}
	auto t = Lua::CreateTable(l);
	for(auto i=decltype(imgBuffers.size()){0u};i<imgBuffers.size();++i)
	{
		auto &layer = imgBuffers.at(i);
		if(layer.empty())
			return;
		Lua::PushInt(l,i +1);
		auto tMipmaps = Lua::CreateTable(l);
		for(auto j=decltype(layer.size()){0u};j<layer.size();++j)
		{
			Lua::PushInt(l,j +1);
			Lua::Push(l,layer.at(i));
		}
		Lua::SetTableValue(l,t);
	}
}

void ClientState::RegisterVulkanLuaInterface(Lua::Interface &lua)
{
	auto &vulkanMod = lua.RegisterLibrary("prosper",{
		{"create_descriptor_set",Lua::Vulkan::create_descriptor_set},
		{"create_buffer",Lua::Vulkan::create_buffer},
		{"create_image",Lua::Vulkan::create_image},
		{"create_image_view",Lua::Vulkan::create_image_view},
		{"create_image_create_info",Lua::Vulkan::create_image_create_info},
		{"create_texture",Lua::Vulkan::create_texture},
		{"create_framebuffer",Lua::Vulkan::create_framebuffer},
		{"create_render_pass",Lua::Vulkan::create_render_pass},
		{"create_render_target",Lua::Vulkan::create_render_target},
		{"create_gradient_texture",Lua::Vulkan::create_gradient_texture},
		{"create_event",Lua::Vulkan::create_event},
		{"create_fence",Lua::Vulkan::create_fence},
		{"calculate_mipmap_count",Lua::Vulkan::calculate_mipmap_count},
		{"calculate_mipmap_size",Lua::Vulkan::calculate_mipmap_size},
		{"result_to_string",Lua::Vulkan::result_to_string},
		{"format_to_string",Lua::Vulkan::format_to_string},
		{"shader_stage_to_string",Lua::Vulkan::shader_stage_to_string},
		{"is_depth_format",Lua::Vulkan::is_depth_format},
		{"is_compressed_format",Lua::Vulkan::is_compressed_format},
		{"is_uncompressed_format",Lua::Vulkan::is_uncompressed_format},
		{"get_bit_size",Lua::Vulkan::get_bit_size},
		{"get_byte_size",Lua::Vulkan::get_byte_size},
		{"get_swapchain_image_count",Lua::Vulkan::get_swapchain_image_count},
		{"wait_idle",Lua::Vulkan::wait_idle}
	});
	vulkanMod[
		luabind::namespace_("util")
		[
			luabind::def("get_square_vertex_uv_buffer",&Lua::Vulkan::get_square_vertex_uv_buffer),
			luabind::def("get_square_vertex_buffer",&Lua::Vulkan::get_square_vertex_buffer),
			luabind::def("get_square_uv_buffer",&Lua::Vulkan::get_square_uv_buffer),
			luabind::def("get_square_vertices",&Lua::Vulkan::get_square_vertices),
			luabind::def("get_square_uv_coordinates",&Lua::Vulkan::get_square_uv_coordinates),
			luabind::def("get_square_vertex_count",&Lua::Vulkan::get_square_vertex_count),
			luabind::def("get_square_vertex_format",&Lua::Vulkan::get_square_vertex_format),
			luabind::def("get_square_uv_format",&Lua::Vulkan::get_square_uv_format),
			luabind::def("allocate_temporary_buffer",static_cast<int(*)(lua_State*,::DataStream&)>(&Lua::Vulkan::allocate_temporary_buffer)),
			luabind::def("allocate_temporary_buffer",static_cast<int(*)(lua_State*,uint32_t)>(&Lua::Vulkan::allocate_temporary_buffer)),

			luabind::def("get_line_vertex_buffer",&Lua::Vulkan::get_line_vertex_buffer),
			luabind::def("get_line_vertices",&Lua::Vulkan::get_line_vertices),
			luabind::def("get_line_vertex_count",&Lua::Vulkan::get_line_vertex_count),
			luabind::def("get_line_vertex_format",&Lua::Vulkan::get_line_vertex_format)
		]
	];

	Lua::RegisterLibraryEnums(lua.GetState(),"prosper",{
		{"FORMAT_UNKNOWN",umath::to_integral(prosper::Format::Unknown)},
		{"FORMAT_R4G4_UNORM_PACK8",umath::to_integral(prosper::Format::R4G4_UNorm_Pack8)},
		{"FORMAT_R4G4B4A4_UNORM_PACK16",umath::to_integral(prosper::Format::R4G4B4A4_UNorm_Pack16)},
		{"FORMAT_B4G4R4A4_UNORM_PACK16",umath::to_integral(prosper::Format::B4G4R4A4_UNorm_Pack16)},
		{"FORMAT_R5G6B5_UNORM_PACK16",umath::to_integral(prosper::Format::R5G6B5_UNorm_Pack16)},
		{"FORMAT_B5G6R5_UNORM_PACK16",umath::to_integral(prosper::Format::B5G6R5_UNorm_Pack16)},
		{"FORMAT_R5G5B5A1_UNORM_PACK16",umath::to_integral(prosper::Format::R5G5B5A1_UNorm_Pack16)},
		{"FORMAT_B5G5R5A1_UNORM_PACK16",umath::to_integral(prosper::Format::B5G5R5A1_UNorm_Pack16)},
		{"FORMAT_A1R5G5B5_UNORM_PACK16",umath::to_integral(prosper::Format::A1R5G5B5_UNorm_Pack16)},
		{"FORMAT_R8_UNORM",umath::to_integral(prosper::Format::R8_UNorm)},
		{"FORMAT_R8_SNORM",umath::to_integral(prosper::Format::R8_SNorm)},
		{"FORMAT_R8_USCALED_POOR_COVERAGE",umath::to_integral(prosper::Format::R8_UScaled_PoorCoverage)},
		{"FORMAT_R8_SSCALED_POOR_COVERAGE",umath::to_integral(prosper::Format::R8_SScaled_PoorCoverage)},
		{"FORMAT_R8_UINT",umath::to_integral(prosper::Format::R8_UInt)},
		{"FORMAT_R8_SINT",umath::to_integral(prosper::Format::R8_SInt)},
		{"FORMAT_R8_SRGB",umath::to_integral(prosper::Format::R8_SRGB)},
		{"FORMAT_R8G8_UNORM",umath::to_integral(prosper::Format::R8G8_UNorm)},
		{"FORMAT_R8G8_SNORM",umath::to_integral(prosper::Format::R8G8_SNorm)},
		{"FORMAT_R8G8_USCALED_POOR_COVERAGE",umath::to_integral(prosper::Format::R8G8_UScaled_PoorCoverage)},
		{"FORMAT_R8G8_SSCALED_POOR_COVERAGE",umath::to_integral(prosper::Format::R8G8_SScaled_PoorCoverage)},
		{"FORMAT_R8G8_UINT",umath::to_integral(prosper::Format::R8G8_UInt)},
		{"FORMAT_R8G8_SINT",umath::to_integral(prosper::Format::R8G8_SInt)},
		{"FORMAT_R8G8_SRGB_POOR_COVERAGE",umath::to_integral(prosper::Format::R8G8_SRGB_PoorCoverage)},
		{"FORMAT_R8G8B8_UNORM_POOR_COVERAGE",umath::to_integral(prosper::Format::R8G8B8_UNorm_PoorCoverage)},
		{"FORMAT_R8G8B8_SNORM_POOR_COVERAGE",umath::to_integral(prosper::Format::R8G8B8_SNorm_PoorCoverage)},
		{"FORMAT_R8G8B8_USCALED_POOR_COVERAGE",umath::to_integral(prosper::Format::R8G8B8_UScaled_PoorCoverage)},
		{"FORMAT_R8G8B8_SSCALED_POOR_COVERAGE",umath::to_integral(prosper::Format::R8G8B8_SScaled_PoorCoverage)},
		{"FORMAT_R8G8B8_UINT_POOR_COVERAGE",umath::to_integral(prosper::Format::R8G8B8_UInt_PoorCoverage)},
		{"FORMAT_R8G8B8_SINT_POOR_COVERAGE",umath::to_integral(prosper::Format::R8G8B8_SInt_PoorCoverage)},
		{"FORMAT_R8G8B8_SRGB_POOR_COVERAGE",umath::to_integral(prosper::Format::R8G8B8_SRGB_PoorCoverage)},
		{"FORMAT_B8G8R8_UNORM_POOR_COVERAGE",umath::to_integral(prosper::Format::B8G8R8_UNorm_PoorCoverage)},
		{"FORMAT_B8G8R8_SNORM_POOR_COVERAGE",umath::to_integral(prosper::Format::B8G8R8_SNorm_PoorCoverage)},
		{"FORMAT_B8G8R8_USCALED_POOR_COVERAGE",umath::to_integral(prosper::Format::B8G8R8_UScaled_PoorCoverage)},
		{"FORMAT_B8G8R8_SSCALED_POOR_COVERAGE",umath::to_integral(prosper::Format::B8G8R8_SScaled_PoorCoverage)},
		{"FORMAT_B8G8R8_UINT_POOR_COVERAGE",umath::to_integral(prosper::Format::B8G8R8_UInt_PoorCoverage)},
		{"FORMAT_B8G8R8_SINT_POOR_COVERAGE",umath::to_integral(prosper::Format::B8G8R8_SInt_PoorCoverage)},
		{"FORMAT_B8G8R8_SRGB_POOR_COVERAGE",umath::to_integral(prosper::Format::B8G8R8_SRGB_PoorCoverage)},
		{"FORMAT_R8G8B8A8_UNORM",umath::to_integral(prosper::Format::R8G8B8A8_UNorm)},
		{"FORMAT_R8G8B8A8_SNORM",umath::to_integral(prosper::Format::R8G8B8A8_SNorm)},
		{"FORMAT_R8G8B8A8_USCALED_POOR_COVERAGE",umath::to_integral(prosper::Format::R8G8B8A8_UScaled_PoorCoverage)},
		{"FORMAT_R8G8B8A8_SSCALED_POOR_COVERAGE",umath::to_integral(prosper::Format::R8G8B8A8_SScaled_PoorCoverage)},
		{"FORMAT_R8G8B8A8_UINT",umath::to_integral(prosper::Format::R8G8B8A8_UInt)},
		{"FORMAT_R8G8B8A8_SINT",umath::to_integral(prosper::Format::R8G8B8A8_SInt)},
		{"FORMAT_R8G8B8A8_SRGB",umath::to_integral(prosper::Format::R8G8B8A8_SRGB)},
		{"FORMAT_B8G8R8A8_UNORM",umath::to_integral(prosper::Format::B8G8R8A8_UNorm)},
		{"FORMAT_B8G8R8A8_SNORM",umath::to_integral(prosper::Format::B8G8R8A8_SNorm)},
		{"FORMAT_B8G8R8A8_USCALED_POOR_COVERAGE",umath::to_integral(prosper::Format::B8G8R8A8_UScaled_PoorCoverage)},
		{"FORMAT_B8G8R8A8_SSCALED_POOR_COVERAGE",umath::to_integral(prosper::Format::B8G8R8A8_SScaled_PoorCoverage)},
		{"FORMAT_B8G8R8A8_UINT",umath::to_integral(prosper::Format::B8G8R8A8_UInt)},
		{"FORMAT_B8G8R8A8_SINT",umath::to_integral(prosper::Format::B8G8R8A8_SInt)},
		{"FORMAT_B8G8R8A8_SRGB",umath::to_integral(prosper::Format::B8G8R8A8_SRGB)},
		{"FORMAT_A8B8G8R8_UNORM_PACK32",umath::to_integral(prosper::Format::A8B8G8R8_UNorm_Pack32)},
		{"FORMAT_A8B8G8R8_SNORM_PACK32",umath::to_integral(prosper::Format::A8B8G8R8_SNorm_Pack32)},
		{"FORMAT_A8B8G8R8_USCALED_PACK32_POOR_COVERAGE",umath::to_integral(prosper::Format::A8B8G8R8_UScaled_Pack32_PoorCoverage)},
		{"FORMAT_A8B8G8R8_SSCALED_PACK32_POOR_COVERAGE",umath::to_integral(prosper::Format::A8B8G8R8_SScaled_Pack32_PoorCoverage)},
		{"FORMAT_A8B8G8R8_UINT_PACK32",umath::to_integral(prosper::Format::A8B8G8R8_UInt_Pack32)},
		{"FORMAT_A8B8G8R8_SINT_PACK32",umath::to_integral(prosper::Format::A8B8G8R8_SInt_Pack32)},
		{"FORMAT_A8B8G8R8_SRGB_PACK32",umath::to_integral(prosper::Format::A8B8G8R8_SRGB_Pack32)},
		{"FORMAT_A2R10G10B10_UNORM_PACK32",umath::to_integral(prosper::Format::A2R10G10B10_UNorm_Pack32)},
		{"FORMAT_A2R10G10B10_SNORM_PACK32_POOR_COVERAGE",umath::to_integral(prosper::Format::A2R10G10B10_SNorm_Pack32_PoorCoverage)},
		{"FORMAT_A2R10G10B10_USCALED_PACK32_POOR_COVERAGE",umath::to_integral(prosper::Format::A2R10G10B10_UScaled_Pack32_PoorCoverage)},
		{"FORMAT_A2R10G10B10_SSCALED_PACK32_POOR_COVERAGE",umath::to_integral(prosper::Format::A2R10G10B10_SScaled_Pack32_PoorCoverage)},
		{"FORMAT_A2R10G10B10_UINT_PACK32",umath::to_integral(prosper::Format::A2R10G10B10_UInt_Pack32)},
		{"FORMAT_A2R10G10B10_SINT_PACK32_POOR_COVERAGE",umath::to_integral(prosper::Format::A2R10G10B10_SInt_Pack32_PoorCoverage)},
		{"FORMAT_A2B10G10R10_UNORM_PACK32",umath::to_integral(prosper::Format::A2B10G10R10_UNorm_Pack32)},
		{"FORMAT_A2B10G10R10_SNORM_PACK32_POOR_COVERAGE",umath::to_integral(prosper::Format::A2B10G10R10_SNorm_Pack32_PoorCoverage)},
		{"FORMAT_A2B10G10R10_USCALED_PACK32_POOR_COVERAGE",umath::to_integral(prosper::Format::A2B10G10R10_UScaled_Pack32_PoorCoverage)},
		{"FORMAT_A2B10G10R10_SSCALED_PACK32_POOR_COVERAGE",umath::to_integral(prosper::Format::A2B10G10R10_SScaled_Pack32_PoorCoverage)},
		{"FORMAT_A2B10G10R10_UINT_PACK32",umath::to_integral(prosper::Format::A2B10G10R10_UInt_Pack32)},
		{"FORMAT_A2B10G10R10_SINT_PACK32_POOR_COVERAGE",umath::to_integral(prosper::Format::A2B10G10R10_SInt_Pack32_PoorCoverage)},
		{"FORMAT_R16_UNORM",umath::to_integral(prosper::Format::R16_UNorm)},
		{"FORMAT_R16_SNORM",umath::to_integral(prosper::Format::R16_SNorm)},
		{"FORMAT_R16_USCALED_POOR_COVERAGE",umath::to_integral(prosper::Format::R16_UScaled_PoorCoverage)},
		{"FORMAT_R16_SSCALED_POOR_COVERAGE",umath::to_integral(prosper::Format::R16_SScaled_PoorCoverage)},
		{"FORMAT_R16_UINT",umath::to_integral(prosper::Format::R16_UInt)},
		{"FORMAT_R16_SINT",umath::to_integral(prosper::Format::R16_SInt)},
		{"FORMAT_R16_SFLOAT",umath::to_integral(prosper::Format::R16_SFloat)},
		{"FORMAT_R16G16_UNORM",umath::to_integral(prosper::Format::R16G16_UNorm)},
		{"FORMAT_R16G16_SNORM",umath::to_integral(prosper::Format::R16G16_SNorm)},
		{"FORMAT_R16G16_USCALED_POOR_COVERAGE",umath::to_integral(prosper::Format::R16G16_UScaled_PoorCoverage)},
		{"FORMAT_R16G16_SSCALED_POOR_COVERAGE",umath::to_integral(prosper::Format::R16G16_SScaled_PoorCoverage)},
		{"FORMAT_R16G16_UINT",umath::to_integral(prosper::Format::R16G16_UInt)},
		{"FORMAT_R16G16_SINT",umath::to_integral(prosper::Format::R16G16_SInt)},
		{"FORMAT_R16G16_SFLOAT",umath::to_integral(prosper::Format::R16G16_SFloat)},
		{"FORMAT_R16G16B16_UNORM_POOR_COVERAGE",umath::to_integral(prosper::Format::R16G16B16_UNorm_PoorCoverage)},
		{"FORMAT_R16G16B16_SNORM_POOR_COVERAGE",umath::to_integral(prosper::Format::R16G16B16_SNorm_PoorCoverage)},
		{"FORMAT_R16G16B16_USCALED_POOR_COVERAGE",umath::to_integral(prosper::Format::R16G16B16_UScaled_PoorCoverage)},
		{"FORMAT_R16G16B16_SSCALED_POOR_COVERAGE",umath::to_integral(prosper::Format::R16G16B16_SScaled_PoorCoverage)},
		{"FORMAT_R16G16B16_UINT_POOR_COVERAGE",umath::to_integral(prosper::Format::R16G16B16_UInt_PoorCoverage)},
		{"FORMAT_R16G16B16_SINT_POOR_COVERAGE",umath::to_integral(prosper::Format::R16G16B16_SInt_PoorCoverage)},
		{"FORMAT_R16G16B16_SFLOAT_POOR_COVERAGE",umath::to_integral(prosper::Format::R16G16B16_SFloat_PoorCoverage)},
		{"FORMAT_R16G16B16A16_UNORM",umath::to_integral(prosper::Format::R16G16B16A16_UNorm)},
		{"FORMAT_R16G16B16A16_SNORM",umath::to_integral(prosper::Format::R16G16B16A16_SNorm)},
		{"FORMAT_R16G16B16A16_USCALED_POOR_COVERAGE",umath::to_integral(prosper::Format::R16G16B16A16_UScaled_PoorCoverage)},
		{"FORMAT_R16G16B16A16_SSCALED_POOR_COVERAGE",umath::to_integral(prosper::Format::R16G16B16A16_SScaled_PoorCoverage)},
		{"FORMAT_R16G16B16A16_UINT",umath::to_integral(prosper::Format::R16G16B16A16_UInt)},
		{"FORMAT_R16G16B16A16_SINT",umath::to_integral(prosper::Format::R16G16B16A16_SInt)},
		{"FORMAT_R16G16B16A16_SFLOAT",umath::to_integral(prosper::Format::R16G16B16A16_SFloat)},
		{"FORMAT_R32_UINT",umath::to_integral(prosper::Format::R32_UInt)},
		{"FORMAT_R32_SINT",umath::to_integral(prosper::Format::R32_SInt)},
		{"FORMAT_R32_SFLOAT",umath::to_integral(prosper::Format::R32_SFloat)},
		{"FORMAT_R32G32_UINT",umath::to_integral(prosper::Format::R32G32_UInt)},
		{"FORMAT_R32G32_SINT",umath::to_integral(prosper::Format::R32G32_SInt)},
		{"FORMAT_R32G32_SFLOAT",umath::to_integral(prosper::Format::R32G32_SFloat)},
		{"FORMAT_R32G32B32_UINT",umath::to_integral(prosper::Format::R32G32B32_UInt)},
		{"FORMAT_R32G32B32_SINT",umath::to_integral(prosper::Format::R32G32B32_SInt)},
		{"FORMAT_R32G32B32_SFLOAT",umath::to_integral(prosper::Format::R32G32B32_SFloat)},
		{"FORMAT_R32G32B32A32_UINT",umath::to_integral(prosper::Format::R32G32B32A32_UInt)},
		{"FORMAT_R32G32B32A32_SINT",umath::to_integral(prosper::Format::R32G32B32A32_SInt)},
		{"FORMAT_R32G32B32A32_SFLOAT",umath::to_integral(prosper::Format::R32G32B32A32_SFloat)},
		{"FORMAT_R64_UINT_POOR_COVERAGE",umath::to_integral(prosper::Format::R64_UInt_PoorCoverage)},
		{"FORMAT_R64_SINT_POOR_COVERAGE",umath::to_integral(prosper::Format::R64_SInt_PoorCoverage)},
		{"FORMAT_R64_SFLOAT_POOR_COVERAGE",umath::to_integral(prosper::Format::R64_SFloat_PoorCoverage)},
		{"FORMAT_R64G64_UINT_POOR_COVERAGE",umath::to_integral(prosper::Format::R64G64_UInt_PoorCoverage)},
		{"FORMAT_R64G64_SINT_POOR_COVERAGE",umath::to_integral(prosper::Format::R64G64_SInt_PoorCoverage)},
		{"FORMAT_R64G64_SFLOAT_POOR_COVERAGE",umath::to_integral(prosper::Format::R64G64_SFloat_PoorCoverage)},
		{"FORMAT_R64G64B64_UINT_POOR_COVERAGE",umath::to_integral(prosper::Format::R64G64B64_UInt_PoorCoverage)},
		{"FORMAT_R64G64B64_SINT_POOR_COVERAGE",umath::to_integral(prosper::Format::R64G64B64_SInt_PoorCoverage)},
		{"FORMAT_R64G64B64_SFLOAT_POOR_COVERAGE",umath::to_integral(prosper::Format::R64G64B64_SFloat_PoorCoverage)},
		{"FORMAT_R64G64B64A64_UINT_POOR_COVERAGE",umath::to_integral(prosper::Format::R64G64B64A64_UInt_PoorCoverage)},
		{"FORMAT_R64G64B64A64_SINT_POOR_COVERAGE",umath::to_integral(prosper::Format::R64G64B64A64_SInt_PoorCoverage)},
		{"FORMAT_R64G64B64A64_SFLOAT_POOR_COVERAGE",umath::to_integral(prosper::Format::R64G64B64A64_SFloat_PoorCoverage)},
		{"FORMAT_B10G11R11_UFLOAT_PACK32",umath::to_integral(prosper::Format::B10G11R11_UFloat_Pack32)},
		{"FORMAT_E5B9G9R9_UFLOAT_PACK32",umath::to_integral(prosper::Format::E5B9G9R9_UFloat_Pack32)},
		{"FORMAT_D16_UNORM",umath::to_integral(prosper::Format::D16_UNorm)},
		{"FORMAT_X8_D24_UNORM_PACK32_POOR_COVERAGE",umath::to_integral(prosper::Format::X8_D24_UNorm_Pack32_PoorCoverage)},
		{"FORMAT_D32_SFLOAT",umath::to_integral(prosper::Format::D32_SFloat)},
		{"FORMAT_S8_UINT_POOR_COVERAGE",umath::to_integral(prosper::Format::S8_UInt_PoorCoverage)},
		{"FORMAT_D16_UNORM_S8_UINT_POOR_COVERAGE",umath::to_integral(prosper::Format::D16_UNorm_S8_UInt_PoorCoverage)},
		{"FORMAT_D24_UNORM_S8_UINT_POOR_COVERAGE",umath::to_integral(prosper::Format::D24_UNorm_S8_UInt_PoorCoverage)},
		{"FORMAT_D32_SFLOAT_S8_UINT",umath::to_integral(prosper::Format::D32_SFloat_S8_UInt)},
		{"FORMAT_BC1_RGB_UNORM_BLOCK",umath::to_integral(prosper::Format::BC1_RGB_UNorm_Block)},
		{"FORMAT_BC1_RGB_SRGB_BLOCK",umath::to_integral(prosper::Format::BC1_RGB_SRGB_Block)},
		{"FORMAT_BC1_RGBA_UNORM_BLOCK",umath::to_integral(prosper::Format::BC1_RGBA_UNorm_Block)},
		{"FORMAT_BC1_RGBA_SRGB_BLOCK",umath::to_integral(prosper::Format::BC1_RGBA_SRGB_Block)},
		{"FORMAT_BC2_UNORM_BLOCK",umath::to_integral(prosper::Format::BC2_UNorm_Block)},
		{"FORMAT_BC2_SRGB_BLOCK",umath::to_integral(prosper::Format::BC2_SRGB_Block)},
		{"FORMAT_BC3_UNORM_BLOCK",umath::to_integral(prosper::Format::BC3_UNorm_Block)},
		{"FORMAT_BC3_SRGB_BLOCK",umath::to_integral(prosper::Format::BC3_SRGB_Block)},
		{"FORMAT_BC4_UNORM_BLOCK",umath::to_integral(prosper::Format::BC4_UNorm_Block)},
		{"FORMAT_BC4_SNORM_BLOCK",umath::to_integral(prosper::Format::BC4_SNorm_Block)},
		{"FORMAT_BC5_UNORM_BLOCK",umath::to_integral(prosper::Format::BC5_UNorm_Block)},
		{"FORMAT_BC5_SNORM_BLOCK",umath::to_integral(prosper::Format::BC5_SNorm_Block)},
		{"FORMAT_BC6H_UFLOAT_BLOCK",umath::to_integral(prosper::Format::BC6H_UFloat_Block)},
		{"FORMAT_BC6H_SFLOAT_BLOCK",umath::to_integral(prosper::Format::BC6H_SFloat_Block)},
		{"FORMAT_BC7_UNORM_BLOCK",umath::to_integral(prosper::Format::BC7_UNorm_Block)},
		{"FORMAT_BC7_SRGB_BLOCK",umath::to_integral(prosper::Format::BC7_SRGB_Block)},
		{"FORMAT_ETC2_R8G8B8_UNORM_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ETC2_R8G8B8_UNorm_Block_PoorCoverage)},
		{"FORMAT_ETC2_R8G8B8_SRGB_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ETC2_R8G8B8_SRGB_Block_PoorCoverage)},
		{"FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ETC2_R8G8B8A1_UNorm_Block_PoorCoverage)},
		{"FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ETC2_R8G8B8A1_SRGB_Block_PoorCoverage)},
		{"FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ETC2_R8G8B8A8_UNorm_Block_PoorCoverage)},
		{"FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ETC2_R8G8B8A8_SRGB_Block_PoorCoverage)},
		{"FORMAT_EAC_R11_UNORM_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::EAC_R11_UNorm_Block_PoorCoverage)},
		{"FORMAT_EAC_R11_SNORM_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::EAC_R11_SNorm_Block_PoorCoverage)},
		{"FORMAT_EAC_R11G11_UNORM_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::EAC_R11G11_UNorm_Block_PoorCoverage)},
		{"FORMAT_EAC_R11G11_SNORM_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::EAC_R11G11_SNorm_Block_PoorCoverage)},
		{"FORMAT_ASTC_4x4_UNORM_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ASTC_4x4_UNorm_Block_PoorCoverage)},
		{"FORMAT_ASTC_4x4_SRGB_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ASTC_4x4_SRGB_Block_PoorCoverage)},
		{"FORMAT_ASTC_5x4_UNORM_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ASTC_5x4_UNorm_Block_PoorCoverage)},
		{"FORMAT_ASTC_5x4_SRGB_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ASTC_5x4_SRGB_Block_PoorCoverage)},
		{"FORMAT_ASTC_5x5_UNORM_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ASTC_5x5_UNorm_Block_PoorCoverage)},
		{"FORMAT_ASTC_5x5_SRGB_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ASTC_5x5_SRGB_Block_PoorCoverage)},
		{"FORMAT_ASTC_6x5_UNORM_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ASTC_6x5_UNorm_Block_PoorCoverage)},
		{"FORMAT_ASTC_6x5_SRGB_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ASTC_6x5_SRGB_Block_PoorCoverage)},
		{"FORMAT_ASTC_6x6_UNORM_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ASTC_6x6_UNorm_Block_PoorCoverage)},
		{"FORMAT_ASTC_6x6_SRGB_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ASTC_6x6_SRGB_Block_PoorCoverage)},
		{"FORMAT_ASTC_8x5_UNORM_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ASTC_8x5_UNorm_Block_PoorCoverage)},
		{"FORMAT_ASTC_8x5_SRGB_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ASTC_8x5_SRGB_Block_PoorCoverage)},
		{"FORMAT_ASTC_8x6_UNORM_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ASTC_8x6_UNorm_Block_PoorCoverage)},
		{"FORMAT_ASTC_8x6_SRGB_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ASTC_8x6_SRGB_Block_PoorCoverage)},
		{"FORMAT_ASTC_8x8_UNORM_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ASTC_8x8_UNorm_Block_PoorCoverage)},
		{"FORMAT_ASTC_8x8_SRGB_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ASTC_8x8_SRGB_Block_PoorCoverage)},
		{"FORMAT_ASTC_10x5_UNORM_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ASTC_10x5_UNorm_Block_PoorCoverage)},
		{"FORMAT_ASTC_10x5_SRGB_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ASTC_10x5_SRGB_Block_PoorCoverage)},
		{"FORMAT_ASTC_10x6_UNORM_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ASTC_10x6_UNorm_Block_PoorCoverage)},
		{"FORMAT_ASTC_10x6_SRGB_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ASTC_10x6_SRGB_Block_PoorCoverage)},
		{"FORMAT_ASTC_10x8_UNORM_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ASTC_10x8_UNorm_Block_PoorCoverage)},
		{"FORMAT_ASTC_10x8_SRGB_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ASTC_10x8_SRGB_Block_PoorCoverage)},
		{"FORMAT_ASTC_10x10_UNORM_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ASTC_10x10_UNorm_Block_PoorCoverage)},
		{"FORMAT_ASTC_10x10_SRGB_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ASTC_10x10_SRGB_Block_PoorCoverage)},
		{"FORMAT_ASTC_12x10_UNORM_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ASTC_12x10_UNorm_Block_PoorCoverage)},
		{"FORMAT_ASTC_12x10_SRGB_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ASTC_12x10_SRGB_Block_PoorCoverage)},
		{"FORMAT_ASTC_12x12_UNORM_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ASTC_12x12_UNorm_Block_PoorCoverage)},
		{"FORMAT_ASTC_12x12_SRGB_BLOCK_POOR_COVERAGE",umath::to_integral(prosper::Format::ASTC_12x12_SRGB_Block_PoorCoverage)},

		{"RESULT_SUCCESS",umath::to_integral(prosper::Result::Success)},
		{"RESULT_NOT_READY",umath::to_integral(prosper::Result::NotReady)},
		{"RESULT_TIMEOUT",umath::to_integral(prosper::Result::Timeout)},
		{"RESULT_EVENT_SET",umath::to_integral(prosper::Result::EventSet)},
		{"RESULT_EVENT_RESET",umath::to_integral(prosper::Result::EventReset)},
		{"RESULT_INCOMPLETE",umath::to_integral(prosper::Result::Incomplete)},
		{"RESULT_ERROR_OUT_OF_HOST_MEMORY",umath::to_integral(prosper::Result::ErrorOutOfHostMemory)},
		{"RESULT_ERROR_OUT_OF_DEVICE_MEMORY",umath::to_integral(prosper::Result::ErrorOutOfDeviceMemory)},
		{"RESULT_ERROR_INITIALIZATION_FAILED",umath::to_integral(prosper::Result::ErrorInitializationFailed)},
		{"RESULT_ERROR_DEVICE_LOST",umath::to_integral(prosper::Result::ErrorDeviceLost)},
		{"RESULT_ERROR_MEMORY_MAP_FAILED",umath::to_integral(prosper::Result::ErrorMemoryMapFailed)},
		{"RESULT_ERROR_LAYER_NOT_PRESENT",umath::to_integral(prosper::Result::ErrorLayerNotPresent)},
		{"RESULT_ERROR_EXTENSION_NOT_PRESENT",umath::to_integral(prosper::Result::ErrorExtensionNotPresent)},
		{"RESULT_ERROR_FEATURE_NOT_PRESENT",umath::to_integral(prosper::Result::ErrorFeatureNotPresent)},
		{"RESULT_ERROR_INCOMPATIBLE_DRIVER",umath::to_integral(prosper::Result::ErrorIncompatibleDriver)},
		{"RESULT_ERROR_TOO_MANY_OBJECTS",umath::to_integral(prosper::Result::ErrorTooManyObjects)},
		{"RESULT_ERROR_FORMAT_NOT_SUPPORTED",umath::to_integral(prosper::Result::ErrorFormatNotSupported)},
		{"RESULT_ERROR_SURFACE_LOST_KHR",umath::to_integral(prosper::Result::ErrorSurfaceLostKHR)},
		{"RESULT_ERROR_NATIVE_WINDOW_IN_USE_KHR",umath::to_integral(prosper::Result::ErrorNativeWindowInUseKHR)},
		{"RESULT_SUBOPTIMAL_KHR",umath::to_integral(prosper::Result::SuboptimalKHR)},
		{"RESULT_ERROR_OUT_OF_DATE_KHR",umath::to_integral(prosper::Result::ErrorOutOfDateKHR)},
		{"RESULT_ERROR_INCOMPATIBLE_DISPLAY_KHR",umath::to_integral(prosper::Result::ErrorIncompatibleDisplayKHR)},
		{"RESULT_ERROR_VALIDATION_FAILED_EXT",umath::to_integral(prosper::Result::ErrorValidationFailedEXT)},

		{"SAMPLER_MIPMAP_MODE_LINEAR",umath::to_integral(prosper::SamplerMipmapMode::Linear)},
		{"SAMPLER_MIPMAP_MODE_NEAREST",umath::to_integral(prosper::SamplerMipmapMode::Nearest)},

		{"SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER",umath::to_integral(prosper::SamplerAddressMode::ClampToBorder)},
		{"SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE",umath::to_integral(prosper::SamplerAddressMode::ClampToEdge)},
		{"SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE",umath::to_integral(prosper::SamplerAddressMode::MirrorClampToEdge)},
		{"SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT",umath::to_integral(prosper::SamplerAddressMode::MirroredRepeat)},
		{"SAMPLER_ADDRESS_MODE_REPEAT",umath::to_integral(prosper::SamplerAddressMode::Repeat)},

		{"COMPARE_OP_ALWAYS",umath::to_integral(prosper::CompareOp::Always)},
		{"COMPARE_OP_EQUAL",umath::to_integral(prosper::CompareOp::Equal)},
		{"COMPARE_OP_GREATER",umath::to_integral(prosper::CompareOp::Greater)},
		{"COMPARE_OP_GREATER_OR_EQUAL",umath::to_integral(prosper::CompareOp::GreaterOrEqual)},
		{"COMPARE_OP_LESS",umath::to_integral(prosper::CompareOp::Less)},
		{"COMPARE_OP_LESS_OR_EQUAL",umath::to_integral(prosper::CompareOp::LessOrEqual)},
		{"COMPARE_OP_NEVER",umath::to_integral(prosper::CompareOp::Never)},
		{"COMPARE_OP_NOT_EQUAL",umath::to_integral(prosper::CompareOp::NotEqual)},

		{"BORDER_COLOR_FLOAT_OPAQUE_BLACK",umath::to_integral(prosper::BorderColor::FloatOpaqueBlack)},
		{"BORDER_COLOR_FLOAT_OPAQUE_WHITE",umath::to_integral(prosper::BorderColor::FloatOpaqueWhite)},
		{"BORDER_COLOR_floatRANSPARENT_BLACK",umath::to_integral(prosper::BorderColor::FloatTransparentBlack)},
		{"BORDER_COLOR_INT_OPAQUE_BLACK",umath::to_integral(prosper::BorderColor::IntOpaqueBlack)},
		{"BORDER_COLOR_INT_OPAQUE_WHITE",umath::to_integral(prosper::BorderColor::IntOpaqueWhite)},
		{"BORDER_COLOR_INT_TRANSPARENT_BLACK",umath::to_integral(prosper::BorderColor::IntTransparentBlack)},

		{"IMAGE_LAYOUT_UNDEFINED",umath::to_integral(prosper::ImageLayout::Undefined)},
		{"IMAGE_LAYOUT_GENERAL",umath::to_integral(prosper::ImageLayout::General)},
		{"IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL",umath::to_integral(prosper::ImageLayout::ColorAttachmentOptimal)},
		{"IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL",umath::to_integral(prosper::ImageLayout::DepthStencilAttachmentOptimal)},
		{"IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL",umath::to_integral(prosper::ImageLayout::DepthStencilReadOnlyOptimal)},
		{"IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL",umath::to_integral(prosper::ImageLayout::ShaderReadOnlyOptimal)},
		{"IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL",umath::to_integral(prosper::ImageLayout::TransferSrcOptimal)},
		{"IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL",umath::to_integral(prosper::ImageLayout::TransferDstOptimal)},
		{"IMAGE_LAYOUT_PREINITIALIZED",umath::to_integral(prosper::ImageLayout::Preinitialized)},
		{"IMAGE_LAYOUT_PRESENT_SRC_KHR",umath::to_integral(prosper::ImageLayout::PresentSrcKHR)},

		{"ATTACHMENT_LOAD_OP_LOAD",umath::to_integral(prosper::AttachmentLoadOp::Load)},
		{"ATTACHMENT_LOAD_OP_CLEAR",umath::to_integral(prosper::AttachmentLoadOp::Clear)},
		{"ATTACHMENT_LOAD_OP_DONT_CARE",umath::to_integral(prosper::AttachmentLoadOp::DontCare)},

		{"ATTACHMENT_STORE_OP_STORE",umath::to_integral(prosper::AttachmentStoreOp::Store)},
		{"ATTACHMENT_STORE_OP_DONT_CARE",umath::to_integral(prosper::AttachmentStoreOp::DontCare)},

		{"IMAGE_TYPE_1D",umath::to_integral(prosper::ImageType::e1D)},
		{"IMAGE_TYPE_2D",umath::to_integral(prosper::ImageType::e2D)},
		{"IMAGE_TYPE_3D",umath::to_integral(prosper::ImageType::e3D)},

		{"IMAGE_TILING_OPTIMAL",umath::to_integral(prosper::ImageTiling::Optimal)},
		{"IMAGE_TILING_LINEAR",umath::to_integral(prosper::ImageTiling::Linear)},

		{"IMAGE_VIEW_TYPE_1D",umath::to_integral(prosper::ImageViewType::e1D)},
		{"IMAGE_VIEW_TYPE_2D",umath::to_integral(prosper::ImageViewType::e2D)},
		{"IMAGE_VIEW_TYPE_3D",umath::to_integral(prosper::ImageViewType::e3D)},
		{"IMAGE_VIEW_TYPE_CUBE",umath::to_integral(prosper::ImageViewType::Cube)},
		{"IMAGE_VIEW_TYPE_1D_ARRAY",umath::to_integral(prosper::ImageViewType::e1DArray)},
		{"IMAGE_VIEW_TYPE_2D_ARRAY",umath::to_integral(prosper::ImageViewType::e2DArray)},
		{"IMAGE_VIEW_TYPE_CUBE_ARRAY",umath::to_integral(prosper::ImageViewType::CubeArray)},

		{"COMPONENT_SWIZZLE_IDENTITY",umath::to_integral(prosper::ComponentSwizzle::Identity)},
		{"COMPONENT_SWIZZLE_ZERO",umath::to_integral(prosper::ComponentSwizzle::Zero)},
		{"COMPONENT_SWIZZLE_ONE",umath::to_integral(prosper::ComponentSwizzle::One)},
		{"COMPONENT_SWIZZLE_R",umath::to_integral(prosper::ComponentSwizzle::R)},
		{"COMPONENT_SWIZZLE_G",umath::to_integral(prosper::ComponentSwizzle::G)},
		{"COMPONENT_SWIZZLE_B",umath::to_integral(prosper::ComponentSwizzle::B)},
		{"COMPONENT_SWIZZLE_A",umath::to_integral(prosper::ComponentSwizzle::A)},

		{"DESCRIPTOR_TYPE_SAMPLER",umath::to_integral(prosper::DescriptorType::Sampler)},
		{"DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",umath::to_integral(prosper::DescriptorType::CombinedImageSampler)},
		{"DESCRIPTOR_TYPE_SAMPLED_IMAGE",umath::to_integral(prosper::DescriptorType::SampledImage)},
		{"DESCRIPTOR_TYPE_STORAGE_IMAGE",umath::to_integral(prosper::DescriptorType::StorageImage)},
		{"DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER",umath::to_integral(prosper::DescriptorType::UniformTexelBuffer)},
		{"DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER",umath::to_integral(prosper::DescriptorType::StorageTexelBuffer)},
		{"DESCRIPTOR_TYPE_UNIFORM_BUFFER",umath::to_integral(prosper::DescriptorType::UniformBuffer)},
		{"DESCRIPTOR_TYPE_STORAGE_BUFFER",umath::to_integral(prosper::DescriptorType::StorageBuffer)},
		{"DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC",umath::to_integral(prosper::DescriptorType::UniformBufferDynamic)},
		{"DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC",umath::to_integral(prosper::DescriptorType::StorageBufferDynamic)},
		{"DESCRIPTOR_TYPE_INPUT_ATTACHMENT",umath::to_integral(prosper::DescriptorType::InputAttachment)},

		{"QUERY_TYPE_OCCLUSION",umath::to_integral(prosper::QueryType::Occlusion)},
		{"QUERY_TYPE_PIPELINE_STATISTICS",umath::to_integral(prosper::QueryType::PipelineStatistics)},
		{"QUERY_TYPE_TIMESTAMP",umath::to_integral(prosper::QueryType::Timestamp)},

		{"PIPELINE_BIND_POINT_GRAPHICS",umath::to_integral(prosper::PipelineBindPoint::Graphics)},
		{"PIPELINE_BIND_POINT_COMPUTE",umath::to_integral(prosper::PipelineBindPoint::Compute)},

		{"PRIMITIVE_TOPOLOGY_POINT_LIST",umath::to_integral(prosper::PrimitiveTopology::PointList)},
		{"PRIMITIVE_TOPOLOGY_LINE_LIST",umath::to_integral(prosper::PrimitiveTopology::LineList)},
		{"PRIMITIVE_TOPOLOGY_LINE_STRIP",umath::to_integral(prosper::PrimitiveTopology::LineStrip)},
		{"PRIMITIVE_TOPOLOGY_TRIANGLE_LIST",umath::to_integral(prosper::PrimitiveTopology::TriangleList)},
		{"PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP",umath::to_integral(prosper::PrimitiveTopology::TriangleStrip)},
		{"PRIMITIVE_TOPOLOGY_TRIANGLE_FAN",umath::to_integral(prosper::PrimitiveTopology::TriangleFan)},
		{"PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY",umath::to_integral(prosper::PrimitiveTopology::LineListWithAdjacency)},
		{"PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY",umath::to_integral(prosper::PrimitiveTopology::LineStripWithAdjacency)},
		{"PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY",umath::to_integral(prosper::PrimitiveTopology::TriangleListWithAdjacency)},
		{"PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY",umath::to_integral(prosper::PrimitiveTopology::TriangleStripWithAdjacency)},
		{"PRIMITIVE_TOPOLOGY_PATCH_LIST",umath::to_integral(prosper::PrimitiveTopology::PatchList)},

		{"SHARING_MODE_EXCLUSIVE",umath::to_integral(prosper::SharingMode::Exclusive)},
		{"SHARING_MODE_CONCURRENT",umath::to_integral(prosper::SharingMode::Concurrent)},

		{"INDEX_TYPE_UINT16",umath::to_integral(prosper::IndexType::UInt16)},
		{"INDEX_TYPE_UINT32",umath::to_integral(prosper::IndexType::UInt32)},

		{"FILTER_NEAREST",umath::to_integral(prosper::Filter::Nearest)},
		{"FILTER_LINEAR",umath::to_integral(prosper::Filter::Linear)},

		{"POLYGON_MODE_FILL",umath::to_integral(prosper::PolygonMode::Fill)},
		{"POLYGON_MODE_LINE",umath::to_integral(prosper::PolygonMode::Line)},
		{"POLYGON_MODE_POINT",umath::to_integral(prosper::PolygonMode::Point)},

		{"CULL_MODE_NONE",umath::to_integral(prosper::CullModeFlags::None)},
		{"CULL_MODE_FRONT_BIT",umath::to_integral(prosper::CullModeFlags::FrontBit)},
		{"CULL_MODE_BACK_BIT",umath::to_integral(prosper::CullModeFlags::BackBit)},
		{"CULL_MODE_FRONT_AND_BACK",umath::to_integral(prosper::CullModeFlags::FrontAndBack)},

		{"FRONT_FACE_COUNTER_CLOCKWISE",umath::to_integral(prosper::FrontFace::CounterClockwise)},
		{"FRONT_FACE_CLOCKWISE",umath::to_integral(prosper::FrontFace::Clockwise)},

		{"BLEND_FACTOR_ZERO",umath::to_integral(prosper::BlendFactor::Zero)},
		{"BLEND_FACTOR_ONE",umath::to_integral(prosper::BlendFactor::One)},
		{"BLEND_FACTOR_SRC_COLOR",umath::to_integral(prosper::BlendFactor::SrcColor)},
		{"BLEND_FACTOR_ONE_MINUS_SRC_COLOR",umath::to_integral(prosper::BlendFactor::OneMinusSrcColor)},
		{"BLEND_FACTOR_DST_COLOR",umath::to_integral(prosper::BlendFactor::DstColor)},
		{"BLEND_FACTOR_ONE_MINUS_DST_COLOR",umath::to_integral(prosper::BlendFactor::OneMinusDstColor)},
		{"BLEND_FACTOR_SRC_ALPHA",umath::to_integral(prosper::BlendFactor::SrcAlpha)},
		{"BLEND_FACTOR_ONE_MINUS_SRC_ALPHA",umath::to_integral(prosper::BlendFactor::OneMinusSrcAlpha)},
		{"BLEND_FACTOR_DST_ALPHA",umath::to_integral(prosper::BlendFactor::DstAlpha)},
		{"BLEND_FACTOR_ONE_MINUS_DST_ALPHA",umath::to_integral(prosper::BlendFactor::OneMinusDstAlpha)},
		{"BLEND_FACTOR_CONSTANT_COLOR",umath::to_integral(prosper::BlendFactor::ConstantColor)},
		{"BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR",umath::to_integral(prosper::BlendFactor::OneMinusConstantColor)},
		{"BLEND_FACTOR_CONSTANT_ALPHA",umath::to_integral(prosper::BlendFactor::ConstantAlpha)},
		{"BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA",umath::to_integral(prosper::BlendFactor::OneMinusConstantAlpha)},
		{"BLEND_FACTOR_SRC_ALPHA_SATURATE",umath::to_integral(prosper::BlendFactor::SrcAlphaSaturate)},
		{"BLEND_FACTOR_SRC1_COLOR",umath::to_integral(prosper::BlendFactor::Src1Color)},
		{"BLEND_FACTOR_ONE_MINUS_SRC1_COLOR",umath::to_integral(prosper::BlendFactor::OneMinusSrc1Color)},
		{"BLEND_FACTOR_SRC1_ALPHA",umath::to_integral(prosper::BlendFactor::Src1Alpha)},
		{"BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA",umath::to_integral(prosper::BlendFactor::OneMinusSrc1Alpha)},

		{"BLEND_OP_ADD",umath::to_integral(prosper::BlendOp::Add)},
		{"BLEND_OP_SUBTRACT",umath::to_integral(prosper::BlendOp::Subtract)},
		{"BLEND_OP_REVERSE_SUBTRACT",umath::to_integral(prosper::BlendOp::ReverseSubtract)},
		{"BLEND_OP_MIN",umath::to_integral(prosper::BlendOp::Min)},
		{"BLEND_OP_MAX",umath::to_integral(prosper::BlendOp::Max)},

		{"STENCIL_OP_KEEP",umath::to_integral(prosper::StencilOp::Keep)},
		{"STENCIL_OP_ZERO",umath::to_integral(prosper::StencilOp::Zero)},
		{"STENCIL_OP_REPLACE",umath::to_integral(prosper::StencilOp::Replace)},
		{"STENCIL_OP_INCREMENT_AND_CLAMP",umath::to_integral(prosper::StencilOp::IncrementAndClamp)},
		{"STENCIL_OP_DECREMENT_AND_CLAMP",umath::to_integral(prosper::StencilOp::DecrementAndClamp)},
		{"STENCIL_OP_INVERT",umath::to_integral(prosper::StencilOp::Invert)},
		{"STENCIL_OP_INCREMENT_AND_WRAP",umath::to_integral(prosper::StencilOp::IncrementAndWrap)},
		{"STENCIL_OP_DECREMENT_AND_WRAP",umath::to_integral(prosper::StencilOp::DecrementAndWrap)},

		{"LOGIC_OP_CLEAR",umath::to_integral(prosper::LogicOp::Clear)},
		{"LOGIC_OP_AND",umath::to_integral(prosper::LogicOp::And)},
		{"LOGIC_OP_AND_REVERSE",umath::to_integral(prosper::LogicOp::AndReverse)},
		{"LOGIC_OP_COPY",umath::to_integral(prosper::LogicOp::Copy)},
		{"LOGIC_OP_AND_INVERTED",umath::to_integral(prosper::LogicOp::AndInverted)},
		{"LOGIC_OP_NO_OP",umath::to_integral(prosper::LogicOp::NoOp)},
		{"LOGIC_OP_XOR",umath::to_integral(prosper::LogicOp::Xor)},
		{"LOGIC_OP_OR",umath::to_integral(prosper::LogicOp::Or)},
		{"LOGIC_OP_NOR",umath::to_integral(prosper::LogicOp::Nor)},
		{"LOGIC_OP_EQUIVALENT",umath::to_integral(prosper::LogicOp::Equivalent)},
		{"LOGIC_OP_INVERT",umath::to_integral(prosper::LogicOp::Invert)},
		{"LOGIC_OP_OR_REVERSE",umath::to_integral(prosper::LogicOp::OrReverse)},
		{"LOGIC_OP_COPY_INVERTED",umath::to_integral(prosper::LogicOp::CopyInverted)},
		{"LOGIC_OP_OR_INVERTED",umath::to_integral(prosper::LogicOp::OrInverted)},
		{"LOGIC_OP_NAND",umath::to_integral(prosper::LogicOp::Nand)},
		{"LOGIC_OP_SET",umath::to_integral(prosper::LogicOp::Set)},

		{"PHYSICAL_DEVICE_TYPE_OTHER",umath::to_integral(prosper::PhysicalDeviceType::Other)},
		{"PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU",umath::to_integral(prosper::PhysicalDeviceType::IntegratedGPU)},
		{"PHYSICAL_DEVICE_TYPE_DISCRETE_GPU",umath::to_integral(prosper::PhysicalDeviceType::DiscreteGPU)},
		{"PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU",umath::to_integral(prosper::PhysicalDeviceType::VirtualGPU)},
		{"PHYSICAL_DEVICE_TYPE_CPU",umath::to_integral(prosper::PhysicalDeviceType::CPU)},

		{"VERTEX_INPUT_RATE_VERTEX",umath::to_integral(prosper::VertexInputRate::Vertex)},
		{"VERTEX_INPUT_RATE_INSTANCE",umath::to_integral(prosper::VertexInputRate::Instance)},

		{"DYNAMIC_STATE_NONE",umath::to_integral(prosper::util::DynamicStateFlags::None)},
		{"DYNAMIC_STATE_VIEWPORT_BIT",umath::to_integral(prosper::util::DynamicStateFlags::Viewport)},
		{"DYNAMIC_STATE_SCISSOR_BIT",umath::to_integral(prosper::util::DynamicStateFlags::Scissor)},
		{"DYNAMIC_STATE_LINE_WIDTH_BIT",umath::to_integral(prosper::util::DynamicStateFlags::LineWidth)},
		{"DYNAMIC_STATE_DEPTH_BIAS_BIT",umath::to_integral(prosper::util::DynamicStateFlags::DepthBias)},
		{"DYNAMIC_STATE_BLEND_CONSTANTS_BIT",umath::to_integral(prosper::util::DynamicStateFlags::BlendConstants)},
		{"DYNAMIC_STATE_DEPTH_BOUNDS_BIT",umath::to_integral(prosper::util::DynamicStateFlags::DepthBounds)},
		{"DYNAMIC_STATE_STENCIL_COMPARE_MASK_BIT",umath::to_integral(prosper::util::DynamicStateFlags::StencilCompareMask)},
		{"DYNAMIC_STATE_STENCIL_WRITE_MASK_BIT",umath::to_integral(prosper::util::DynamicStateFlags::StencilWriteMask)},
		{"DYNAMIC_STATE_STENCIL_REFERENCE_BIT",umath::to_integral(prosper::util::DynamicStateFlags::StencilReference)},
#if 0
		{"DYNAMIC_STATE_VIEWPORT_W_SCALING_NV_BIT",umath::to_integral(prosper::util::DynamicStateFlags::ViewportWScalingNV)},
		{"DYNAMIC_STATE_DISCARD_RECTANGLE_EXT_BIT",umath::to_integral(prosper::util::DynamicStateFlags::DiscardRectangleEXT)},
		{"DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT_BIT",umath::to_integral(prosper::util::DynamicStateFlags::SampleLocationsEXT)},
		{"DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV_BIT",umath::to_integral(prosper::util::DynamicStateFlags::ViewportShadingRatePaletteNV)},
		{"DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV_BIT",umath::to_integral(prosper::util::DynamicStateFlags::ViewportCoarseSampleOrderNV)},
		{"DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV_BIT",umath::to_integral(prosper::util::DynamicStateFlags::ExclusiveScissorNV)},
#endif

		{"MEMORY_PROPERTY_DEVICE_LOCAL_BIT",umath::to_integral(prosper::MemoryPropertyFlags::DeviceLocalBit)},
		{"MEMORY_PROPERTY_HOST_VISIBLE_BIT",umath::to_integral(prosper::MemoryPropertyFlags::HostVisibleBit)},
		{"MEMORY_PROPERTY_HOST_COHERENT_BIT",umath::to_integral(prosper::MemoryPropertyFlags::HostCoherentBit)},
		{"MEMORY_PROPERTY_HOST_CACHED_BIT",umath::to_integral(prosper::MemoryPropertyFlags::HostCachedBit)},
		{"MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT",umath::to_integral(prosper::MemoryPropertyFlags::LazilyAllocatedBit)},

	
		{"ACCESS_INDIRECT_COMMAND_READ_BIT",umath::to_integral(prosper::AccessFlags::IndirectCommandReadBit)},
		{"ACCESS_INDEX_READ_BIT",umath::to_integral(prosper::AccessFlags::IndexReadBit)},
		{"ACCESS_VERTEX_ATTRIBUTE_READ_BIT",umath::to_integral(prosper::AccessFlags::VertexAttributeReadBit)},
		{"ACCESS_UNIFORM_READ_BIT",umath::to_integral(prosper::AccessFlags::UniformReadBit)},
		{"ACCESS_INPUT_ATTACHMENT_READ_BIT",umath::to_integral(prosper::AccessFlags::InputAttachmentReadBit)},
		{"ACCESS_SHADER_READ_BIT",umath::to_integral(prosper::AccessFlags::ShaderReadBit)},
		{"ACCESS_SHADER_WRITE_BIT",umath::to_integral(prosper::AccessFlags::ShaderWriteBit)},
		{"ACCESS_COLOR_ATTACHMENT_READ_BIT",umath::to_integral(prosper::AccessFlags::ColorAttachmentReadBit)},
		{"ACCESS_COLOR_ATTACHMENT_WRITE_BIT",umath::to_integral(prosper::AccessFlags::ColorAttachmentWriteBit)},
		{"ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT",umath::to_integral(prosper::AccessFlags::DepthStencilAttachmentReadBit)},
		{"ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT",umath::to_integral(prosper::AccessFlags::DepthStencilAttachmentWriteBit)},
		{"ACCESS_TRANSFER_READ_BIT",umath::to_integral(prosper::AccessFlags::TransferReadBit)},
		{"ACCESS_TRANSFER_WRITE_BIT",umath::to_integral(prosper::AccessFlags::TransferWriteBit)},
		{"ACCESS_HOST_READ_BIT",umath::to_integral(prosper::AccessFlags::HostReadBit)},
		{"ACCESS_HOST_WRITE_BIT",umath::to_integral(prosper::AccessFlags::HostWriteBit)},
		{"ACCESS_MEMORY_READ_BIT",umath::to_integral(prosper::AccessFlags::MemoryReadBit)},
		{"ACCESS_MEMORY_WRITE_BIT",umath::to_integral(prosper::AccessFlags::MemoryWriteBit)},

		{"BUFFER_USAGE_TRANSFER_SRC_BIT",umath::to_integral(prosper::BufferUsageFlags::TransferSrcBit)},
		{"BUFFER_USAGE_TRANSFER_DST_BIT",umath::to_integral(prosper::BufferUsageFlags::TransferDstBit)},
		{"BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT",umath::to_integral(prosper::BufferUsageFlags::UniformTexelBufferBit)},
		{"BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT",umath::to_integral(prosper::BufferUsageFlags::StorageTexelBufferBit)},
		{"BUFFER_USAGE_UNIFORM_BUFFER_BIT",umath::to_integral(prosper::BufferUsageFlags::UniformBufferBit)},
		{"BUFFER_USAGE_STORAGE_BUFFER_BIT",umath::to_integral(prosper::BufferUsageFlags::StorageBufferBit)},
		{"BUFFER_USAGE_INDEX_BUFFER_BIT",umath::to_integral(prosper::BufferUsageFlags::IndexBufferBit)},
		{"BUFFER_USAGE_VERTEX_BUFFER_BIT",umath::to_integral(prosper::BufferUsageFlags::VertexBufferBit)},
		{"BUFFER_USAGE_INDIRECT_BUFFER_BIT",umath::to_integral(prosper::BufferUsageFlags::IndirectBufferBit)},

		{"SHADER_STAGE_VERTEX_BIT",umath::to_integral(prosper::ShaderStageFlags::VertexBit)},
		{"SHADER_STAGE_TESSELLATION_CONTROL_BIT",umath::to_integral(prosper::ShaderStageFlags::TessellationControlBit)},
		{"SHADER_STAGE_TESSELLATION_EVALUATION_BIT",umath::to_integral(prosper::ShaderStageFlags::TessellationEvaluationBit)},
		{"SHADER_STAGE_GEOMETRY_BIT",umath::to_integral(prosper::ShaderStageFlags::GeometryBit)},
		{"SHADER_STAGE_FRAGMENT_BIT",umath::to_integral(prosper::ShaderStageFlags::FragmentBit)},
		{"SHADER_STAGE_COMPUTE_BIT",umath::to_integral(prosper::ShaderStageFlags::ComputeBit)},
		{"SHADER_STAGE_ALL_GRAPHICS",umath::to_integral(prosper::ShaderStageFlags::AllGraphics)},
		{"SHADER_STAGE_ALL",umath::to_integral(prosper::ShaderStageFlags::All)},

		{"IMAGE_USAGE_TRANSFER_SRC_BIT",umath::to_integral(prosper::ImageUsageFlags::TransferSrcBit)},
		{"IMAGE_USAGE_TRANSFER_DST_BIT",umath::to_integral(prosper::ImageUsageFlags::TransferDstBit)},
		{"IMAGE_USAGE_SAMPLED_BIT",umath::to_integral(prosper::ImageUsageFlags::SampledBit)},
		{"IMAGE_USAGE_STORAGE_BIT",umath::to_integral(prosper::ImageUsageFlags::StorageBit)},
		{"IMAGE_USAGE_COLOR_ATTACHMENT_BIT",umath::to_integral(prosper::ImageUsageFlags::ColorAttachmentBit)},
		{"IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT",umath::to_integral(prosper::ImageUsageFlags::DepthStencilAttachmentBit)},
		{"IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT",umath::to_integral(prosper::ImageUsageFlags::TransientAttachmentBit)},
		{"IMAGE_USAGE_INPUT_ATTACHMENT_BIT",umath::to_integral(prosper::ImageUsageFlags::InputAttachmentBit)},

		{"IMAGE_CREATE_SPARSE_BINDING_BIT",umath::to_integral(prosper::ImageCreateFlags::SparseBindingBit)},
		{"IMAGE_CREATE_SPARSE_RESIDENCY_BIT",umath::to_integral(prosper::ImageCreateFlags::SparseResidencyBit)},
		{"IMAGE_CREATE_SPARSE_ALIASED_BIT",umath::to_integral(prosper::ImageCreateFlags::SparseAliasedBit)},
		{"IMAGE_CREATE_MUTABLE_FORMAT_BIT",umath::to_integral(prosper::ImageCreateFlags::MutableFormatBit)},
		{"IMAGE_CREATE_CUBE_COMPATIBLE_BIT",umath::to_integral(prosper::ImageCreateFlags::CubeCompatibleBit)},

		{"PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT",umath::to_integral(prosper::PipelineCreateFlags::DisableOptimizationBit)},
		{"PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT",umath::to_integral(prosper::PipelineCreateFlags::AllowDerivativesBit)},
		{"PIPELINE_CREATE_DERIVATIVE_BIT",umath::to_integral(prosper::PipelineCreateFlags::DerivativeBit)},

		{"COLOR_COMPONENT_R_BIT",umath::to_integral(prosper::ColorComponentFlags::RBit)},
		{"COLOR_COMPONENT_G_BIT",umath::to_integral(prosper::ColorComponentFlags::GBit)},
		{"COLOR_COMPONENT_B_BIT",umath::to_integral(prosper::ColorComponentFlags::BBit)},
		{"COLOR_COMPONENT_A_BIT",umath::to_integral(prosper::ColorComponentFlags::ABit)},

		{"QUERY_RESULT_64_BIT",umath::to_integral(prosper::QueryResultFlags::e64Bit)},
		{"QUERY_RESULT_WAIT_BIT",umath::to_integral(prosper::QueryResultFlags::WaitBit)},
		{"QUERY_RESULT_WITH_AVAILABILITY_BIT",umath::to_integral(prosper::QueryResultFlags::WithAvailabilityBit)},
		{"QUERY_RESULT_PARTIAL_BIT",umath::to_integral(prosper::QueryResultFlags::PartialBit)},

		{"QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT",umath::to_integral(prosper::QueryPipelineStatisticFlags::InputAssemblyVerticesBit)},
		{"QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT",umath::to_integral(prosper::QueryPipelineStatisticFlags::InputAssemblyPrimitivesBit)},
		{"QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT",umath::to_integral(prosper::QueryPipelineStatisticFlags::VertexShaderInvocationsBit)},
		{"QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT",umath::to_integral(prosper::QueryPipelineStatisticFlags::GeometryShaderInvocationsBit)},
		{"QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT",umath::to_integral(prosper::QueryPipelineStatisticFlags::GeometryShaderPrimitivesBit)},
		{"QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT",umath::to_integral(prosper::QueryPipelineStatisticFlags::ClippingInvocationsBit)},
		{"QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT",umath::to_integral(prosper::QueryPipelineStatisticFlags::ClippingPrimitivesBit)},
		{"QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT",umath::to_integral(prosper::QueryPipelineStatisticFlags::FragmentShaderInvocationsBit)},
		{"QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT",umath::to_integral(prosper::QueryPipelineStatisticFlags::TessellationControlShaderPatchesBit)},
		{"QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT",umath::to_integral(prosper::QueryPipelineStatisticFlags::TessellationEvaluationShaderInvocationsBit)},
		{"QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT",umath::to_integral(prosper::QueryPipelineStatisticFlags::ComputeShaderInvocationsBit)},

		{"IMAGE_ASPECT_COLOR_BIT",umath::to_integral(prosper::ImageAspectFlags::ColorBit)},
		{"IMAGE_ASPECT_DEPTH_BIT",umath::to_integral(prosper::ImageAspectFlags::DepthBit)},
		{"IMAGE_ASPECT_STENCIL_BIT",umath::to_integral(prosper::ImageAspectFlags::StencilBit)},
		{"IMAGE_ASPECT_METADATA_BIT",umath::to_integral(prosper::ImageAspectFlags::MetadataBit)},

		{"PIPELINE_STAGE_TOP_OF_PIPE_BIT",umath::to_integral(prosper::PipelineStageFlags::TopOfPipeBit)},
		{"PIPELINE_STAGE_DRAW_INDIRECT_BIT",umath::to_integral(prosper::PipelineStageFlags::DrawIndirectBit)},
		{"PIPELINE_STAGE_VERTEX_INPUT_BIT",umath::to_integral(prosper::PipelineStageFlags::VertexInputBit)},
		{"PIPELINE_STAGE_VERTEX_SHADER_BIT",umath::to_integral(prosper::PipelineStageFlags::VertexShaderBit)},
		{"PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT",umath::to_integral(prosper::PipelineStageFlags::TessellationControlShaderBit)},
		{"PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT",umath::to_integral(prosper::PipelineStageFlags::TessellationEvaluationShaderBit)},
		{"PIPELINE_STAGE_GEOMETRY_SHADER_BIT",umath::to_integral(prosper::PipelineStageFlags::GeometryShaderBit)},
		{"PIPELINE_STAGE_FRAGMENT_SHADER_BIT",umath::to_integral(prosper::PipelineStageFlags::FragmentShaderBit)},
		{"PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT",umath::to_integral(prosper::PipelineStageFlags::EarlyFragmentTestsBit)},
		{"PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT",umath::to_integral(prosper::PipelineStageFlags::LateFragmentTestsBit)},
		{"PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT",umath::to_integral(prosper::PipelineStageFlags::ColorAttachmentOutputBit)},
		{"PIPELINE_STAGE_COMPUTE_SHADER_BIT",umath::to_integral(prosper::PipelineStageFlags::ComputeShaderBit)},
		{"PIPELINE_STAGE_TRANSFER_BIT",umath::to_integral(prosper::PipelineStageFlags::TransferBit)},
		{"PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT",umath::to_integral(prosper::PipelineStageFlags::BottomOfPipeBit)},
		{"PIPELINE_STAGE_HOST_BIT",umath::to_integral(prosper::PipelineStageFlags::HostBit)},
		{"PIPELINE_STAGE_ALL_GRAPHICS",umath::to_integral(prosper::PipelineStageFlags::AllGraphics)},
		{"PIPELINE_STAGE_ALL_COMMANDS",umath::to_integral(prosper::PipelineStageFlags::AllCommands)},

		{"SAMPLE_COUNT_1_BIT",umath::to_integral(prosper::SampleCountFlags::e1Bit)},
		{"SAMPLE_COUNT_2_BIT",umath::to_integral(prosper::SampleCountFlags::e2Bit)},
		{"SAMPLE_COUNT_4_BIT",umath::to_integral(prosper::SampleCountFlags::e4Bit)},
		{"SAMPLE_COUNT_8_BIT",umath::to_integral(prosper::SampleCountFlags::e8Bit)},
		{"SAMPLE_COUNT_16_BIT",umath::to_integral(prosper::SampleCountFlags::e16Bit)},
		{"SAMPLE_COUNT_32_BIT",umath::to_integral(prosper::SampleCountFlags::e32Bit)},
		{"SAMPLE_COUNT_64_BIT",umath::to_integral(prosper::SampleCountFlags::e64Bit)},

		{"STENCIL_FACE_FRONT_BIT",umath::to_integral(prosper::StencilFaceFlags::FrontBit)},
		{"STENCIL_FACE_BACK_BIT",umath::to_integral(prosper::StencilFaceFlags::BackBit)},
		{"STENCIL_FRONT_AND_BACK",umath::to_integral(prosper::StencilFaceFlags::FrontAndBack)},

		{"PRESENT_MODE_IMMEDIATE_KHR",umath::to_integral(prosper::PresentModeKHR::Immediate)},
		{"PRESENT_MODE_MAILBOX_KHR",umath::to_integral(prosper::PresentModeKHR::Mailbox)},
		{"PRESENT_MODE_FIFO_KHR",umath::to_integral(prosper::PresentModeKHR::Fifo)},
		{"PRESENT_MODE_FIFO_RELAXED_KHR",umath::to_integral(prosper::PresentModeKHR::FifoRelaxed)},

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

		{"DEBUG_REPORT_INFORMATION_BIT_EXT",umath::to_integral(prosper::DebugReportFlags::InformationBit)},
		{"DEBUG_REPORT_WARNING_BIT_EXT",umath::to_integral(prosper::DebugReportFlags::WarningBit)},
		{"DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT",umath::to_integral(prosper::DebugReportFlags::PerformanceWarningBit)},
		{"DEBUG_REPORT_ERROR_BIT_EXT",umath::to_integral(prosper::DebugReportFlags::ErrorBit)},
		{"DEBUG_REPORT_DEBUG_BIT_EXT",umath::to_integral(prosper::DebugReportFlags::DebugBit)},

		{"DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT",umath::to_integral(prosper::DebugReportObjectTypeEXT::Unknown)},
		{"DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT",umath::to_integral(prosper::DebugReportObjectTypeEXT::Instance)},
		{"DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT",umath::to_integral(prosper::DebugReportObjectTypeEXT::PhysicalDevice)},
		{"DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT",umath::to_integral(prosper::DebugReportObjectTypeEXT::Device)},
		{"DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT",umath::to_integral(prosper::DebugReportObjectTypeEXT::Queue)},
		{"DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT",umath::to_integral(prosper::DebugReportObjectTypeEXT::Semaphore)},
		{"DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT",umath::to_integral(prosper::DebugReportObjectTypeEXT::CommandBuffer)},
		{"DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT",umath::to_integral(prosper::DebugReportObjectTypeEXT::Fence)},
		{"DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT",umath::to_integral(prosper::DebugReportObjectTypeEXT::DeviceMemory)},
		{"DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT",umath::to_integral(prosper::DebugReportObjectTypeEXT::Buffer)},
		{"DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT",umath::to_integral(prosper::DebugReportObjectTypeEXT::Image)},
		{"DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT",umath::to_integral(prosper::DebugReportObjectTypeEXT::Event)},
		{"DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT",umath::to_integral(prosper::DebugReportObjectTypeEXT::QueryPool)},
		{"DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT",umath::to_integral(prosper::DebugReportObjectTypeEXT::BufferView)},
		{"DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT",umath::to_integral(prosper::DebugReportObjectTypeEXT::ImageView)},
		{"DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT",umath::to_integral(prosper::DebugReportObjectTypeEXT::ShaderModule)},
		{"DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT",umath::to_integral(prosper::DebugReportObjectTypeEXT::PipelineCache)},
		{"DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT",umath::to_integral(prosper::DebugReportObjectTypeEXT::PipelineLayout)},
		{"DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT",umath::to_integral(prosper::DebugReportObjectTypeEXT::RenderPass)},
		{"DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT",umath::to_integral(prosper::DebugReportObjectTypeEXT::Pipeline)},
		{"DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT",umath::to_integral(prosper::DebugReportObjectTypeEXT::DescriptorSetLayout)},
		{"DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT",umath::to_integral(prosper::DebugReportObjectTypeEXT::Sampler)},
		{"DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT",umath::to_integral(prosper::DebugReportObjectTypeEXT::DescriptorPool)},
		{"DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT",umath::to_integral(prosper::DebugReportObjectTypeEXT::DescriptorSet)},
		{"DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT",umath::to_integral(prosper::DebugReportObjectTypeEXT::Framebuffer)},
		{"DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT",umath::to_integral(prosper::DebugReportObjectTypeEXT::CommandPool)},
		{"DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT",umath::to_integral(prosper::DebugReportObjectTypeEXT::SurfaceKHR)},
		{"DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT",umath::to_integral(prosper::DebugReportObjectTypeEXT::SwapchainKHR)},

		{"MEMORY_FEATURE_DEVICE_LOCAL_BIT",umath::to_integral(prosper::MemoryFeatureFlags::DeviceLocal)},
		{"MEMORY_FEATURE_HOST_CACHED_BIT",umath::to_integral(prosper::MemoryFeatureFlags::HostCached)},
		{"MEMORY_FEATURE_HOST_COHERENT_BIT",umath::to_integral(prosper::MemoryFeatureFlags::HostCoherent)},
		{"MEMORY_FEATURE_LAZILY_ALLOCATED_BIT",umath::to_integral(prosper::MemoryFeatureFlags::LazilyAllocated)},
		{"MEMORY_FEATURE_HOST_ACCESSABLE_BIT",umath::to_integral(prosper::MemoryFeatureFlags::HostAccessable)},
		{"MEMORY_FEATURE_GPU_BULK_BIT",umath::to_integral(prosper::MemoryFeatureFlags::GPUBulk)},
		{"MEMORY_FEATURE_CPU_TO_GPU",umath::to_integral(prosper::MemoryFeatureFlags::CPUToGPU)},
		{"MEMORY_FEATURE_GPU_TO_CPU",umath::to_integral(prosper::MemoryFeatureFlags::GPUToCPU)},

		{"QUEUE_FAMILY_GRAPHICS_BIT",umath::to_integral(prosper::QueueFamilyFlags::GraphicsBit)},
		{"QUEUE_FAMILY_COMPUTE_BIT",umath::to_integral(prosper::QueueFamilyFlags::ComputeBit)},
		{"QUEUE_FAMILY_DMA_BIT",umath::to_integral(prosper::QueueFamilyFlags::DMABit)}
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
	vulkanMod[defShaderStatisticsInfoAMD];

	auto defShaderResourceUsageAMD = luabind::class_<prosper::ShaderResourceUsageAMD>("ShaderResourceUsageAMD");
	defShaderResourceUsageAMD.def_readwrite("numUsedVgprs",&prosper::ShaderResourceUsageAMD::numUsedVgprs);
	defShaderResourceUsageAMD.def_readwrite("numUsedSgprs",&prosper::ShaderResourceUsageAMD::numUsedSgprs);
	defShaderResourceUsageAMD.def_readwrite("ldsSizePerLocalWorkGroup",&prosper::ShaderResourceUsageAMD::ldsSizePerLocalWorkGroup);
	defShaderResourceUsageAMD.def_readwrite("ldsUsageSizeInBytes",&prosper::ShaderResourceUsageAMD::ldsUsageSizeInBytes);
	defShaderResourceUsageAMD.def_readwrite("scratchMemUsageInBytes",&prosper::ShaderResourceUsageAMD::scratchMemUsageInBytes);
	vulkanMod[defShaderResourceUsageAMD];

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
	vulkanMod[defPipelineColorBlendAttachmentState];
#endif

	auto defBufferCreateInfo = luabind::class_<prosper::util::BufferCreateInfo>("BufferCreateInfo");
	defBufferCreateInfo.def(luabind::constructor<>());
	defBufferCreateInfo.def_readwrite("size",&prosper::util::BufferCreateInfo::size);
	defBufferCreateInfo.def_readwrite("queueFamilyMask",reinterpret_cast<uint32_t prosper::util::BufferCreateInfo::*>(&prosper::util::BufferCreateInfo::queueFamilyMask));
	defBufferCreateInfo.def_readwrite("flags",reinterpret_cast<uint32_t prosper::util::BufferCreateInfo::*>(&prosper::util::BufferCreateInfo::flags));
	defBufferCreateInfo.def_readwrite("usageFlags",reinterpret_cast<uint32_t prosper::util::BufferCreateInfo::*>(&prosper::util::BufferCreateInfo::usageFlags));
	defBufferCreateInfo.def_readwrite("memoryFeatures",reinterpret_cast<uint32_t prosper::util::BufferCreateInfo::*>(&prosper::util::BufferCreateInfo::memoryFeatures));
	defBufferCreateInfo.add_static_constant("FLAG_NONE",umath::to_integral(prosper::util::BufferCreateInfo::Flags::None));
	defBufferCreateInfo.add_static_constant("FLAG_CONCURRENT_SHARING_BIT",umath::to_integral(prosper::util::BufferCreateInfo::Flags::ConcurrentSharing));
	defBufferCreateInfo.add_static_constant("FLAG_DONT_ALLOCATE_MEMORY_BIT",umath::to_integral(prosper::util::BufferCreateInfo::Flags::DontAllocateMemory));
	defBufferCreateInfo.add_static_constant("FLAG_SPARSE_BIT",umath::to_integral(prosper::util::BufferCreateInfo::Flags::Sparse));
	defBufferCreateInfo.add_static_constant("FLAG_SPARSE_ALIASED_RESIDENCY_BIT",umath::to_integral(prosper::util::BufferCreateInfo::Flags::SparseAliasedResidency));
	vulkanMod[defBufferCreateInfo];

	auto defRenderTargetCreateInfo = luabind::class_<prosper::util::RenderTargetCreateInfo>("RenderTargetCreateInfo");
	defRenderTargetCreateInfo.def(luabind::constructor<>());
	defRenderTargetCreateInfo.def_readwrite("useLayerFramebuffers",&prosper::util::RenderTargetCreateInfo::useLayerFramebuffers);
	vulkanMod[defRenderTargetCreateInfo];

	auto defSamplerCreateInfo = luabind::class_<prosper::util::SamplerCreateInfo>("SamplerCreateInfo");
	defSamplerCreateInfo.def(luabind::constructor<>());
	defSamplerCreateInfo.def_readwrite("minFilter",reinterpret_cast<uint32_t prosper::util::SamplerCreateInfo::*>(&prosper::util::SamplerCreateInfo::minFilter));
	defSamplerCreateInfo.def_readwrite("magFilter",reinterpret_cast<uint32_t prosper::util::SamplerCreateInfo::*>(&prosper::util::SamplerCreateInfo::magFilter));
	defSamplerCreateInfo.def_readwrite("mipmapMode",reinterpret_cast<uint32_t prosper::util::SamplerCreateInfo::*>(&prosper::util::SamplerCreateInfo::mipmapMode));
	defSamplerCreateInfo.def_readwrite("addressModeU",reinterpret_cast<uint32_t prosper::util::SamplerCreateInfo::*>(&prosper::util::SamplerCreateInfo::addressModeU));
	defSamplerCreateInfo.def_readwrite("addressModeV",reinterpret_cast<uint32_t prosper::util::SamplerCreateInfo::*>(&prosper::util::SamplerCreateInfo::addressModeV));
	defSamplerCreateInfo.def_readwrite("addressModeW",reinterpret_cast<uint32_t prosper::util::SamplerCreateInfo::*>(&prosper::util::SamplerCreateInfo::addressModeW));
	defSamplerCreateInfo.def_readwrite("mipLodBias",&prosper::util::SamplerCreateInfo::mipLodBias);
	defSamplerCreateInfo.def_readwrite("maxAnisotropy",&prosper::util::SamplerCreateInfo::maxAnisotropy);
	defSamplerCreateInfo.def_readwrite("compareEnable",&prosper::util::SamplerCreateInfo::compareEnable);
	defSamplerCreateInfo.def_readwrite("compareOp",reinterpret_cast<uint32_t prosper::util::SamplerCreateInfo::*>(&prosper::util::SamplerCreateInfo::compareOp));
	defSamplerCreateInfo.def_readwrite("minLod",&prosper::util::SamplerCreateInfo::minLod);
	defSamplerCreateInfo.def_readwrite("maxLod",&prosper::util::SamplerCreateInfo::maxLod);
	defSamplerCreateInfo.def_readwrite("borderColor",reinterpret_cast<uint32_t prosper::util::SamplerCreateInfo::*>(&prosper::util::SamplerCreateInfo::borderColor));
	// defSamplerCreateInfo.def_readwrite("useUnnormalizedCoordinates",&prosper::util::SamplerCreateInfo::useUnnormalizedCoordinates);
	vulkanMod[defSamplerCreateInfo];

	auto defTextureCreateInfo = luabind::class_<prosper::util::TextureCreateInfo>("TextureCreateInfo");
	defTextureCreateInfo.def(luabind::constructor<>());
	defTextureCreateInfo.def_readwrite("flags",reinterpret_cast<uint32_t prosper::util::TextureCreateInfo::*>(&prosper::util::TextureCreateInfo::flags));
	defTextureCreateInfo.def_readwrite("sampler",&prosper::util::TextureCreateInfo::sampler);
	defTextureCreateInfo.def_readwrite("imageView",&prosper::util::TextureCreateInfo::imageView);
	defTextureCreateInfo.add_static_constant("FLAG_NONE",umath::to_integral(prosper::util::TextureCreateInfo::Flags::None));
	defTextureCreateInfo.add_static_constant("FLAG_RESOLVABLE",umath::to_integral(prosper::util::TextureCreateInfo::Flags::Resolvable));
	defTextureCreateInfo.add_static_constant("FLAG_CREATE_IMAGE_VIEW_FOR_EACH_LAYER",umath::to_integral(prosper::util::TextureCreateInfo::Flags::CreateImageViewForEachLayer));
	vulkanMod[defTextureCreateInfo];

	auto defImageCreateInfo = luabind::class_<prosper::util::ImageCreateInfo>("ImageCreateInfo");
	defImageCreateInfo.def(luabind::constructor<>());
	defImageCreateInfo.def_readwrite("type",reinterpret_cast<uint32_t prosper::util::ImageCreateInfo::*>(&prosper::util::ImageCreateInfo::type));
	defImageCreateInfo.def_readwrite("width",&prosper::util::ImageCreateInfo::width);
	defImageCreateInfo.def_readwrite("height",&prosper::util::ImageCreateInfo::height);
	defImageCreateInfo.def_readwrite("format",reinterpret_cast<uint32_t prosper::util::ImageCreateInfo::*>(&prosper::util::ImageCreateInfo::format));
	defImageCreateInfo.def_readwrite("layers",&prosper::util::ImageCreateInfo::layers);
	defImageCreateInfo.def_readwrite("usageFlags",reinterpret_cast<uint32_t prosper::util::ImageCreateInfo::*>(&prosper::util::ImageCreateInfo::usage));
	defImageCreateInfo.def_readwrite("samples",reinterpret_cast<uint32_t prosper::util::ImageCreateInfo::*>(&prosper::util::ImageCreateInfo::samples));
	defImageCreateInfo.def_readwrite("tiling",reinterpret_cast<uint32_t prosper::util::ImageCreateInfo::*>(&prosper::util::ImageCreateInfo::tiling));
	defImageCreateInfo.def_readwrite("postCreateLayout",reinterpret_cast<uint32_t prosper::util::ImageCreateInfo::*>(&prosper::util::ImageCreateInfo::postCreateLayout));
	defImageCreateInfo.def_readwrite("flags",reinterpret_cast<uint32_t prosper::util::ImageCreateInfo::*>(&prosper::util::ImageCreateInfo::flags));
	defImageCreateInfo.def_readwrite("queueFamilyMask",reinterpret_cast<uint32_t prosper::util::ImageCreateInfo::*>(&prosper::util::ImageCreateInfo::queueFamilyMask));
	defImageCreateInfo.def_readwrite("memoryFeatures",reinterpret_cast<uint32_t prosper::util::ImageCreateInfo::*>(&prosper::util::ImageCreateInfo::memoryFeatures));
	defImageCreateInfo.add_static_constant("FLAG_NONE",umath::to_integral(prosper::util::ImageCreateInfo::Flags::None));
	defImageCreateInfo.add_static_constant("FLAG_CUBEMAP_BIT",umath::to_integral(prosper::util::ImageCreateInfo::Flags::Cubemap));
	defImageCreateInfo.add_static_constant("FLAG_CONCURRENT_SHARING_BIT",umath::to_integral(prosper::util::ImageCreateInfo::Flags::ConcurrentSharing));
	defImageCreateInfo.add_static_constant("FLAG_FULL_MIPMAP_CHAIN_BIT",umath::to_integral(prosper::util::ImageCreateInfo::Flags::FullMipmapChain));
	defImageCreateInfo.add_static_constant("FLAG_SPARSE_BIT",umath::to_integral(prosper::util::ImageCreateInfo::Flags::Sparse));
	defImageCreateInfo.add_static_constant("FLAG_SPARSE_ALIASED_RESIDENCY_BIT",umath::to_integral(prosper::util::ImageCreateInfo::Flags::SparseAliasedResidency));
	vulkanMod[defImageCreateInfo];

	auto defImageViewCreateInfo = luabind::class_<prosper::util::ImageViewCreateInfo>("ImageViewCreateInfo");
	defImageViewCreateInfo.def(luabind::constructor<>());
	defImageViewCreateInfo.def_readwrite("baseLayer",&prosper::util::ImageViewCreateInfo::baseLayer);
	defImageViewCreateInfo.def_readwrite("levelCount",&prosper::util::ImageViewCreateInfo::levelCount);
	defImageViewCreateInfo.def_readwrite("baseMipmap",&prosper::util::ImageViewCreateInfo::baseMipmap);
	defImageViewCreateInfo.def_readwrite("mipmapLevels",&prosper::util::ImageViewCreateInfo::mipmapLevels);
	defImageViewCreateInfo.def_readwrite("format",reinterpret_cast<uint32_t prosper::util::ImageViewCreateInfo::*>(&prosper::util::ImageViewCreateInfo::format));
	defImageViewCreateInfo.def_readwrite("swizzleRed",reinterpret_cast<uint32_t prosper::util::ImageViewCreateInfo::*>(&prosper::util::ImageViewCreateInfo::swizzleRed));
	defImageViewCreateInfo.def_readwrite("swizzleGreen",reinterpret_cast<uint32_t prosper::util::ImageViewCreateInfo::*>(&prosper::util::ImageViewCreateInfo::swizzleGreen));
	defImageViewCreateInfo.def_readwrite("swizzleBlue",reinterpret_cast<uint32_t prosper::util::ImageViewCreateInfo::*>(&prosper::util::ImageViewCreateInfo::swizzleBlue));
	defImageViewCreateInfo.def_readwrite("swizzleAlpha",reinterpret_cast<uint32_t prosper::util::ImageViewCreateInfo::*>(&prosper::util::ImageViewCreateInfo::swizzleAlpha));
	vulkanMod[defImageViewCreateInfo];

	auto defRenderPassCreateInfo = luabind::class_<prosper::util::RenderPassCreateInfo>("RenderPassCreateInfo");
	defRenderPassCreateInfo.def(luabind::constructor<>());
	defRenderPassCreateInfo.def("AddAttachment",static_cast<void(*)(lua_State*,prosper::util::RenderPassCreateInfo&,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t)>([](lua_State *l,prosper::util::RenderPassCreateInfo &createInfo,uint32_t format,uint32_t initialLayout,uint32_t finalLayout,uint32_t loadOp,uint32_t storeOp,uint32_t sampleCount) {
		createInfo.attachments.push_back({static_cast<prosper::Format>(format),static_cast<prosper::ImageLayout>(initialLayout),static_cast<prosper::AttachmentLoadOp>(loadOp),static_cast<prosper::AttachmentStoreOp>(storeOp),static_cast<prosper::SampleCountFlags>(sampleCount),static_cast<prosper::ImageLayout>(finalLayout)});
		Lua::PushInt(l,createInfo.attachments.size() -1ull);
	}));
	defRenderPassCreateInfo.def("AddAttachment",static_cast<void(*)(lua_State*,prosper::util::RenderPassCreateInfo&,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t)>([](lua_State *l,prosper::util::RenderPassCreateInfo &createInfo,uint32_t format,uint32_t initialLayout,uint32_t finalLayout,uint32_t loadOp,uint32_t storeOp) {
		createInfo.attachments.push_back({static_cast<prosper::Format>(format),static_cast<prosper::ImageLayout>(initialLayout),static_cast<prosper::AttachmentLoadOp>(loadOp),static_cast<prosper::AttachmentStoreOp>(storeOp),prosper::SampleCountFlags::e1Bit,static_cast<prosper::ImageLayout>(finalLayout)});
		Lua::PushInt(l,createInfo.attachments.size() -1ull);
	}));
	defRenderPassCreateInfo.def("AddSubPass",static_cast<void(*)(lua_State*,prosper::util::RenderPassCreateInfo&)>([](lua_State *l,prosper::util::RenderPassCreateInfo &createInfo) {
		createInfo.subPasses.push_back({});
		Lua::PushInt(l,createInfo.subPasses.size() -1ull);
	}));
	defRenderPassCreateInfo.def("AddSubPassColorAttachment",static_cast<void(*)(lua_State*,prosper::util::RenderPassCreateInfo&,uint32_t,uint32_t)>([](lua_State *l,prosper::util::RenderPassCreateInfo &createInfo,uint32_t subPassId,uint32_t colorAttId) {
		if(subPassId >= createInfo.subPasses.size() || colorAttId >= createInfo.attachments.size())
			return;
		createInfo.subPasses.at(subPassId).colorAttachments.push_back(colorAttId);
	}));
	defRenderPassCreateInfo.def("SetSubPassDepthStencilAttachmentEnabled",static_cast<void(*)(lua_State*,prosper::util::RenderPassCreateInfo&,uint32_t,bool)>([](lua_State *l,prosper::util::RenderPassCreateInfo &createInfo,uint32_t subPassId,bool bEnabled) {
		if(subPassId >= createInfo.subPasses.size())
			return;
		createInfo.subPasses.at(subPassId).useDepthStencilAttachment = bEnabled;
	}));
	defRenderPassCreateInfo.def("AddSubPassDependency",static_cast<void(*)(lua_State*,prosper::util::RenderPassCreateInfo&,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t)>([](lua_State *l,prosper::util::RenderPassCreateInfo &createInfo,uint32_t subPassId,uint32_t sourceSubPassId,uint32_t destinationSubPassId,uint32_t sourceStageMask,uint32_t destinationStageMask,uint32_t sourceAccessMask,uint32_t destinationAccessMask) {
		if(subPassId >= createInfo.subPasses.size())
			return;
		createInfo.subPasses.at(subPassId).dependencies.push_back({
			sourceSubPassId,destinationSubPassId,
			static_cast<prosper::PipelineStageFlags>(sourceStageMask),static_cast<prosper::PipelineStageFlags>(destinationStageMask),
			static_cast<prosper::AccessFlags>(sourceAccessMask),static_cast<prosper::AccessFlags>(destinationAccessMask)
		});
	}));
	vulkanMod[defRenderPassCreateInfo];

	auto defImageSubresourceLayers = luabind::class_<prosper::util::ImageSubresourceLayers>("ImageSubresourceLayers");
	defImageSubresourceLayers.def(luabind::constructor<>());
	defImageSubresourceLayers.def_readwrite("aspectMask",reinterpret_cast<std::underlying_type_t<decltype(prosper::util::ImageSubresourceLayers::aspectMask)> prosper::util::ImageSubresourceLayers::*>(&prosper::util::ImageSubresourceLayers::aspectMask));
    defImageSubresourceLayers.def_readwrite("mipLevel",&prosper::util::ImageSubresourceLayers::mipLevel);
	defImageSubresourceLayers.def_readwrite("baseArrayLayer",&prosper::util::ImageSubresourceLayers::baseArrayLayer);
	defImageSubresourceLayers.def_readwrite("layerCount",&prosper::util::ImageSubresourceLayers::layerCount);
	vulkanMod[defImageSubresourceLayers];

	auto defBlitInfo = luabind::class_<prosper::util::BlitInfo>("BlitInfo");
	defBlitInfo.def(luabind::constructor<>());
	defBlitInfo.def_readwrite("srcSubresourceLayer",&prosper::util::BlitInfo::srcSubresourceLayer);
	defBlitInfo.def_readwrite("dstSubresourceLayer",&prosper::util::BlitInfo::dstSubresourceLayer);
	vulkanMod[defBlitInfo];

	static_assert(sizeof(prosper::Offset3D) == sizeof(Vector3i));
	auto defCopyInfo = luabind::class_<prosper::util::CopyInfo>("ImageCopyInfo");
	defCopyInfo.def(luabind::constructor<>());
	defCopyInfo.def_readwrite("width",&prosper::util::CopyInfo::width);
	defCopyInfo.def_readwrite("height",&prosper::util::CopyInfo::height);
	defCopyInfo.def_readwrite("srcSubresource",&prosper::util::CopyInfo::srcSubresource);
	defCopyInfo.def_readwrite("dstSubresource",&prosper::util::CopyInfo::dstSubresource);
	defCopyInfo.def_readwrite("srcOffset",reinterpret_cast<Vector3i prosper::util::CopyInfo::*>(&prosper::util::CopyInfo::srcOffset));
	defCopyInfo.def_readwrite("dstOffset",reinterpret_cast<Vector3i prosper::util::CopyInfo::*>(&prosper::util::CopyInfo::dstOffset));
	defCopyInfo.def_readwrite("srcImageLayout",reinterpret_cast<uint32_t prosper::util::CopyInfo::*>(&prosper::util::CopyInfo::srcImageLayout));
	defCopyInfo.def_readwrite("dstImageLayout",reinterpret_cast<uint32_t prosper::util::CopyInfo::*>(&prosper::util::CopyInfo::dstImageLayout));
	vulkanMod[defCopyInfo];

	auto defBufferCopyInfo = luabind::class_<prosper::util::BufferCopy>("BufferCopyInfo");
	defBufferCopyInfo.def(luabind::constructor<>());
	defBufferCopyInfo.def_readwrite("srcOffset",&prosper::util::BufferCopy::srcOffset);
	defBufferCopyInfo.def_readwrite("dstOffset",&prosper::util::BufferCopy::dstOffset);
	defBufferCopyInfo.def_readwrite("size",&prosper::util::BufferCopy::size);
	vulkanMod[defBufferCopyInfo];

	auto defBufferImageCopyInfo = luabind::class_<prosper::util::BufferImageCopyInfo>("BufferImageCopyInfo");
	defBufferImageCopyInfo.def(luabind::constructor<>());
	defBufferImageCopyInfo.def("SetWidth",static_cast<void(*)(lua_State*,prosper::util::BufferImageCopyInfo&,uint32_t)>([](lua_State *l,prosper::util::BufferImageCopyInfo &copyInfo,uint32_t width) {
		copyInfo.width = width;
	}));
	defBufferImageCopyInfo.def("SetHeight",static_cast<void(*)(lua_State*,prosper::util::BufferImageCopyInfo&,uint32_t)>([](lua_State *l,prosper::util::BufferImageCopyInfo &copyInfo,uint32_t height) {
		copyInfo.height = height;
	}));
	defBufferImageCopyInfo.def("GetWidth",static_cast<void(*)(lua_State*,prosper::util::BufferImageCopyInfo&)>([](lua_State *l,prosper::util::BufferImageCopyInfo &copyInfo) {
		if(copyInfo.width.has_value() == false)
			return;
		Lua::PushInt(l,*copyInfo.width);
	}));
	defBufferImageCopyInfo.def("GetHeight",static_cast<void(*)(lua_State*,prosper::util::BufferImageCopyInfo&)>([](lua_State *l,prosper::util::BufferImageCopyInfo &copyInfo) {
		if(copyInfo.height.has_value() == false)
			return;
		Lua::PushInt(l,*copyInfo.height);
	}));
	defBufferImageCopyInfo.def_readwrite("bufferOffset",&prosper::util::BufferImageCopyInfo::bufferOffset);
	defBufferImageCopyInfo.def_readwrite("width",&prosper::util::BufferImageCopyInfo::width);
	defBufferImageCopyInfo.def_readwrite("height",&prosper::util::BufferImageCopyInfo::height);
	defBufferImageCopyInfo.def_readwrite("mipLevel",&prosper::util::BufferImageCopyInfo::mipLevel);
	defBufferImageCopyInfo.def_readwrite("baseArrayLayer",&prosper::util::BufferImageCopyInfo::baseArrayLayer);
	defBufferImageCopyInfo.def_readwrite("layerCount",&prosper::util::BufferImageCopyInfo::layerCount);
	defBufferImageCopyInfo.def_readwrite("aspectMask",reinterpret_cast<uint32_t prosper::util::BufferImageCopyInfo::*>(&prosper::util::BufferImageCopyInfo::aspectMask));
	defBufferImageCopyInfo.def_readwrite("dstImageLayout",reinterpret_cast<uint32_t prosper::util::BufferImageCopyInfo::*>(&prosper::util::BufferImageCopyInfo::dstImageLayout));
	vulkanMod[defBufferImageCopyInfo];

	auto defPipelineBarrierInfo = luabind::class_<prosper::util::PipelineBarrierInfo>("PipelineBarrierInfo");
	defPipelineBarrierInfo.def(luabind::constructor<>());
	defPipelineBarrierInfo.def_readwrite("srcStageMask",reinterpret_cast<uint32_t prosper::util::PipelineBarrierInfo::*>(&prosper::util::PipelineBarrierInfo::srcStageMask));
	defPipelineBarrierInfo.def_readwrite("dstStageMask",reinterpret_cast<uint32_t prosper::util::PipelineBarrierInfo::*>(&prosper::util::PipelineBarrierInfo::dstStageMask));
	//defPipelineBarrierInfo.def_readwrite("bufferBarriers",&prosper::util::PipelineBarrierInfo::bufferBarriers); // prosper TODO
	//defPipelineBarrierInfo.def_readwrite("imageBarriers",&prosper::util::PipelineBarrierInfo::imageBarriers); // prosper TODO
	vulkanMod[defPipelineBarrierInfo];

	auto defImageSubresourceRange = luabind::class_<prosper::util::ImageSubresourceRange>("ImageSubresourceRange");
	defImageSubresourceRange.def(luabind::constructor<>());
	defImageSubresourceRange.def(luabind::constructor<uint32_t>());
	defImageSubresourceRange.def(luabind::constructor<uint32_t,uint32_t>());
	defImageSubresourceRange.def(luabind::constructor<uint32_t,uint32_t,uint32_t>());
	defImageSubresourceRange.def(luabind::constructor<uint32_t,uint32_t,uint32_t,uint32_t>());
	defImageSubresourceRange.def_readwrite("baseMipLevel",&prosper::util::ImageSubresourceRange::baseMipLevel);
	defImageSubresourceRange.def_readwrite("levelCount",&prosper::util::ImageSubresourceRange::levelCount);
	defImageSubresourceRange.def_readwrite("baseArrayLayer",&prosper::util::ImageSubresourceRange::baseArrayLayer);
	defImageSubresourceRange.def_readwrite("layerCount",&prosper::util::ImageSubresourceRange::layerCount);
	vulkanMod[defImageSubresourceRange];

	auto defClearImageInfo = luabind::class_<prosper::util::ClearImageInfo>("ClearImageInfo");
	defClearImageInfo.def(luabind::constructor<>());
	defClearImageInfo.def_readwrite("subresourceRange",&prosper::util::ClearImageInfo::subresourceRange);
	vulkanMod[defClearImageInfo];

	auto defVkTexture = luabind::class_<Lua::Vulkan::Texture>("Texture");
	defVkTexture.def(luabind::tostring(luabind::self));
	defVkTexture.def(luabind::const_self ==luabind::const_self);
	defVkTexture.def("GetImage",&Lua::Vulkan::VKTexture::GetImage);
	defVkTexture.def("GetImageView",static_cast<void(*)(lua_State*,Lua::Vulkan::Texture&,uint32_t)>(&Lua::Vulkan::VKTexture::GetImageView));
	defVkTexture.def("GetImageView",static_cast<void(*)(lua_State*,Lua::Vulkan::Texture&)>(&Lua::Vulkan::VKTexture::GetImageView));
	defVkTexture.def("GetSampler",&Lua::Vulkan::VKTexture::GetSampler);
	defVkTexture.def("GetWidth",&Lua::Vulkan::VKTexture::GetWidth);
	defVkTexture.def("GetHeight",&Lua::Vulkan::VKTexture::GetHeight);
	defVkTexture.def("GetFormat",&Lua::Vulkan::VKTexture::GetFormat);
	defVkTexture.def("IsValid",&Lua::Vulkan::VKTexture::IsValid);
	defVkTexture.def("SetDebugName",static_cast<void(*)(lua_State*,Lua::Vulkan::Texture&,const std::string&)>([](lua_State *l,Lua::Vulkan::Texture &tex,const std::string &name) {
		Lua::Vulkan::VKContextObject::SetDebugName<Lua::Vulkan::Texture>(l,tex,name,&Lua::Check<Lua::Vulkan::Texture>);
	}));
	defVkTexture.def("GetDebugName",static_cast<void(*)(lua_State*,Lua::Vulkan::Texture&)>([](lua_State *l,Lua::Vulkan::Texture &tex) {
		Lua::Vulkan::VKContextObject::GetDebugName<Lua::Vulkan::Texture>(l,tex,&Lua::Check<Lua::Vulkan::Texture>);
	}));
	defVkTexture.def("SetImageView",static_cast<void(*)(lua_State*,Lua::Vulkan::Texture&,Lua::Vulkan::ImageView&)>([](lua_State *l,Lua::Vulkan::Texture &tex,Lua::Vulkan::ImageView &imgView) {
		tex.SetImageView(imgView);
	}));
	//
	vulkanMod[defVkTexture];

	auto defVkImage = luabind::class_<Lua::Vulkan::Image>("Image");
	defVkImage.def(luabind::tostring(luabind::self));
	defVkImage.def(luabind::const_self ==luabind::const_self);
	defVkImage.def("IsValid",&Lua::Vulkan::VKImage::IsValid);
	defVkImage.def("GetAspectSubresourceLayout",&Lua::Vulkan::VKImage::GetAspectSubresourceLayout);
	defVkImage.def("GetAspectSubresourceLayout",static_cast<void(*)(lua_State*,Lua::Vulkan::Image&,uint32_t)>([](lua_State *l,Lua::Vulkan::Image &img,uint32_t layer) {
		Lua::Vulkan::VKImage::GetAspectSubresourceLayout(l,img,layer);
	}));
	defVkImage.def("GetAspectSubresourceLayout",static_cast<void(*)(lua_State*,Lua::Vulkan::Image&)>([](lua_State *l,Lua::Vulkan::Image &img) {
		Lua::Vulkan::VKImage::GetAspectSubresourceLayout(l,img);
	}));
	defVkImage.def("GetAlignment",&Lua::Vulkan::VKImage::GetAlignment);
	defVkImage.def("GetExtent2D",&Lua::Vulkan::VKImage::GetExtent2D);
	defVkImage.def("GetFormat",&Lua::Vulkan::VKImage::GetFormat);
#if 0
	defVkImage.def("GetExtent3D",&Lua::Vulkan::VKImage::GetExtent3D);
	defVkImage.def("GetMemoryTypes",&Lua::Vulkan::VKImage::GetMemoryTypes);
	defVkImage.def("GetStorageSize",&Lua::Vulkan::VKImage::GetStorageSize);
	//defVkImage.def("GetParentSwapchain",&Lua::Vulkan::VKImage::GetParentSwapchain);
	defVkImage.def("GetMemoryBlock",&Lua::Vulkan::VKImage::GetMemoryBlock);
	defVkImage.def("GetSubresourceRange",&Lua::Vulkan::VKImage::GetSubresourceRange);
#endif
	defVkImage.def("GetMipmapSize",&Lua::Vulkan::VKImage::GetMipmapSize);
	defVkImage.def("GetLayerCount",&Lua::Vulkan::VKImage::GetLayerCount);
	defVkImage.def("GetMipmapCount",&Lua::Vulkan::VKImage::GetMipmapCount);
	defVkImage.def("GetSampleCount",&Lua::Vulkan::VKImage::GetSampleCount);
	defVkImage.def("GetSharingMode",&Lua::Vulkan::VKImage::GetSharingMode);
	defVkImage.def("GetTiling",&Lua::Vulkan::VKImage::GetTiling);
	defVkImage.def("GetType",&Lua::Vulkan::VKImage::GetType);
	defVkImage.def("GetUsage",&Lua::Vulkan::VKImage::GetUsage);
	defVkImage.def("GetWidth",&Lua::Vulkan::VKImage::GetWidth);
	defVkImage.def("GetHeight",&Lua::Vulkan::VKImage::GetHeight);
	defVkImage.def("SetDebugName",static_cast<void(*)(lua_State*,Lua::Vulkan::Image&,const std::string&)>([](lua_State *l,Lua::Vulkan::Image &img,const std::string &name) {
		Lua::Vulkan::VKContextObject::SetDebugName<Lua::Vulkan::Image>(l,img,name,&Lua::Check<Lua::Vulkan::Image>);
	}));
	defVkImage.def("GetDebugName",static_cast<void(*)(lua_State*,Lua::Vulkan::Image&)>([](lua_State *l,Lua::Vulkan::Image &img) {
		Lua::Vulkan::VKContextObject::GetDebugName<Lua::Vulkan::Image>(l,img,&Lua::Check<Lua::Vulkan::Image>);
	}));
	defVkImage.def("Copy",static_cast<void(*)(lua_State*,Lua::Vulkan::Image&,Lua::Vulkan::CommandBuffer&,prosper::util::ImageCreateInfo&)>([](lua_State *l,Lua::Vulkan::Image &img,Lua::Vulkan::CommandBuffer &cmd,prosper::util::ImageCreateInfo &imgCreateInfo) {
		auto cpy = img.Copy(cmd,imgCreateInfo);
		if(cpy == nullptr)
			return;
		Lua::Push(l,cpy);
	}));
	defVkImage.def("Copy",static_cast<void(*)(lua_State*,Lua::Vulkan::Image&,Lua::Vulkan::CommandBuffer&)>([](lua_State *l,Lua::Vulkan::Image &img,Lua::Vulkan::CommandBuffer &cmd) {
		auto cpy = img.Copy(cmd,img.GetCreateInfo());
		if(cpy == nullptr)
			return;
		Lua::Push(l,cpy);
	}));
	defVkImage.def("GetCreateInfo",static_cast<void(*)(lua_State*,Lua::Vulkan::Image&)>([](lua_State *l,Lua::Vulkan::Image &img) {
		Lua::Push(l,img.GetCreateInfo());
	}));
	defVkImage.def("ToImageBuffer",static_cast<void(*)(lua_State*,Lua::Vulkan::Image&,uint32_t,uint32_t,uint32_t)>([](lua_State *l,Lua::Vulkan::Image &img,uint32_t includeLayers,uint32_t includeMipmaps,uint32_t targetFormat) {
		std::vector<std::vector<std::shared_ptr<uimg::ImageBuffer>>> imgBuffers;
		auto result = util::to_image_buffer(img,static_cast<uimg::ImageBuffer::Format>(targetFormat),imgBuffers,includeLayers,includeMipmaps);
		if(result == false || imgBuffers.empty())
			return;
		push_image_buffers(l,includeLayers,includeMipmaps,imgBuffers);
	}));
	defVkImage.def("ToImageBuffer",static_cast<void(*)(lua_State*,Lua::Vulkan::Image&,uint32_t,uint32_t)>([](lua_State *l,Lua::Vulkan::Image &img,uint32_t includeLayers,uint32_t includeMipmaps) {
		std::vector<std::vector<std::shared_ptr<uimg::ImageBuffer>>> imgBuffers;
		auto result = util::to_image_buffer(img,imgBuffers,includeLayers,includeMipmaps);
		if(result == false || imgBuffers.empty())
			return;
		push_image_buffers(l,includeLayers,includeMipmaps,imgBuffers);
	}));
	defVkImage.def("GetMemoryBuffer",static_cast<void(*)(lua_State*,Lua::Vulkan::Image&)>([](lua_State *l,Lua::Vulkan::Image &img) {
		auto *buf = img.GetMemoryBuffer();
		if(buf == nullptr)
			return;
		// The image should have ownership over the memory, so we'll just return a pointer. It's up to the user to make sure not to use the memory after the image has been destroyed!
		Lua::Push(l,buf);
		//Lua::Push(l,buf->shared_from_this());
	}));
	vulkanMod[defVkImage];

	auto debSubresourceLayout = luabind::class_<prosper::util::SubresourceLayout>("SubresourceLayout");
	debSubresourceLayout.def(luabind::constructor<>());
	debSubresourceLayout.def_readwrite("offset",&prosper::util::SubresourceLayout::offset);
	debSubresourceLayout.def_readwrite("size",&prosper::util::SubresourceLayout::size);
	debSubresourceLayout.def_readwrite("rowPitch",&prosper::util::SubresourceLayout::row_pitch);
	debSubresourceLayout.def_readwrite("arrayPitch",&prosper::util::SubresourceLayout::array_pitch);
	debSubresourceLayout.def_readwrite("depthPitch",&prosper::util::SubresourceLayout::depth_pitch);
	vulkanMod[debSubresourceLayout];

	auto defVkImageView = luabind::class_<Lua::Vulkan::ImageView>("ImageView");
	defVkImageView.def(luabind::tostring(luabind::self));
	defVkImageView.def(luabind::const_self ==luabind::const_self);
	defVkImageView.def("IsValid",&Lua::Vulkan::VKImageView::IsValid);
	defVkImageView.def("GetAspectMask",&Lua::Vulkan::VKImageView::GetAspectMask);
	defVkImageView.def("GetBaseLayer",&Lua::Vulkan::VKImageView::GetBaseLayer);
	defVkImageView.def("GetBaseMipmapLevel",&Lua::Vulkan::VKImageView::GetBaseMipmapLevel);
#if 0
	defVkImageView.def("GetBaseMipmapSize",&Lua::Vulkan::VKImageView::GetBaseMipmapSize);
	//defVkImageView.def("GetParentImage",&Lua::Vulkan::VKImageView::GetParentImage);
	defVkImageView.def("GetSubresourceRange",&Lua::Vulkan::VKImageView::GetSubresourceRange);
#endif
	defVkImageView.def("GetImageFormat",&Lua::Vulkan::VKImageView::GetImageFormat);
	defVkImageView.def("GetLayerCount",&Lua::Vulkan::VKImageView::GetLayerCount);
	defVkImageView.def("GetMipmapCount",&Lua::Vulkan::VKImageView::GetMipmapCount);
	defVkImageView.def("GetSwizzleArray",&Lua::Vulkan::VKImageView::GetSwizzleArray);
	defVkImageView.def("GetType",&Lua::Vulkan::VKImageView::GetType);
	defVkImageView.def("SetDebugName",static_cast<void(*)(lua_State*,Lua::Vulkan::ImageView&,const std::string&)>([](lua_State *l,Lua::Vulkan::ImageView &imgView,const std::string &name) {
		Lua::Vulkan::VKContextObject::SetDebugName<Lua::Vulkan::ImageView>(l,imgView,name,&Lua::Check<Lua::Vulkan::ImageView>);
	}));
	defVkImageView.def("GetDebugName",static_cast<void(*)(lua_State*,Lua::Vulkan::ImageView&)>([](lua_State *l,Lua::Vulkan::ImageView &imgView) {
		Lua::Vulkan::VKContextObject::GetDebugName<Lua::Vulkan::ImageView>(l,imgView,&Lua::Check<Lua::Vulkan::ImageView>);
	}));
	vulkanMod[defVkImageView];

	auto defVkSampler = luabind::class_<Lua::Vulkan::Sampler>("Sampler");
	defVkSampler.def(luabind::tostring(luabind::self));
	defVkSampler.def(luabind::const_self ==luabind::const_self);
	defVkSampler.def("IsValid",&Lua::Vulkan::VKSampler::IsValid);
	defVkSampler.def("Update",&Lua::Vulkan::VKSampler::Update);
	defVkSampler.def("GetMagFilter",&Lua::Vulkan::VKSampler::GetMagFilter);
	defVkSampler.def("GetMinFilter",&Lua::Vulkan::VKSampler::GetMinFilter);
	defVkSampler.def("GetMipmapMode",&Lua::Vulkan::VKSampler::GetMipmapMode);
	defVkSampler.def("GetAddressModeU",&Lua::Vulkan::VKSampler::GetAddressModeU);
	defVkSampler.def("GetAddressModeV",&Lua::Vulkan::VKSampler::GetAddressModeV);
	defVkSampler.def("GetAddressModeW",&Lua::Vulkan::VKSampler::GetAddressModeW);
	defVkSampler.def("GetMipLodBias",&Lua::Vulkan::VKSampler::GetMipLodBias);
	defVkSampler.def("GetAnisotropyEnabled",&Lua::Vulkan::VKSampler::GetAnisotropyEnabled);
	defVkSampler.def("GetMaxAnisotropy",&Lua::Vulkan::VKSampler::GetMaxAnisotropy);
	defVkSampler.def("GetCompareEnabled",&Lua::Vulkan::VKSampler::GetCompareEnabled);
	defVkSampler.def("GetCompareOperation",&Lua::Vulkan::VKSampler::GetCompareOperation);
	defVkSampler.def("GetMinLod",&Lua::Vulkan::VKSampler::GetMinLod);
	defVkSampler.def("GetMaxLod",&Lua::Vulkan::VKSampler::GetMaxLod);
	defVkSampler.def("GetBorderColor",&Lua::Vulkan::VKSampler::GetBorderColor);
	// defVkSampler.def("GetUnnormalizedCoordinates",&Lua::Vulkan::VKSampler::GetUseUnnormalizedCoordinates);
	defVkSampler.def("SetMagFilter",&Lua::Vulkan::VKSampler::SetMagFilter);
	defVkSampler.def("SetMinFilter",&Lua::Vulkan::VKSampler::SetMinFilter);
	defVkSampler.def("SetMipmapMode",&Lua::Vulkan::VKSampler::SetMipmapMode);
	defVkSampler.def("SetAddressModeU",&Lua::Vulkan::VKSampler::SetAddressModeU);
	defVkSampler.def("SetAddressModeV",&Lua::Vulkan::VKSampler::SetAddressModeV);
	defVkSampler.def("SetAddressModeW",&Lua::Vulkan::VKSampler::SetAddressModeW);
	//defVkSampler.def("SetMipLodBias",&Lua::Vulkan::VKSampler::SetMipLodBias);
	//defVkSampler.def("SetAnisotropyEnabled",&Lua::Vulkan::VKSampler::SetAnisotropyEnabled);
	defVkSampler.def("SetMaxAnisotropy",&Lua::Vulkan::VKSampler::SetMaxAnisotropy);
	defVkSampler.def("SetCompareEnabled",&Lua::Vulkan::VKSampler::SetCompareEnabled);
	defVkSampler.def("SetCompareOperation",&Lua::Vulkan::VKSampler::SetCompareOperation);
	defVkSampler.def("SetMinLod",&Lua::Vulkan::VKSampler::SetMinLod);
	defVkSampler.def("SetMaxLod",&Lua::Vulkan::VKSampler::SetMaxLod);
	defVkSampler.def("SetBorderColor",&Lua::Vulkan::VKSampler::SetBorderColor);
	// defVkSampler.def("SetUnnormalizedCoordinates",&Lua::Vulkan::VKSampler::SetUseUnnormalizedCoordinates);
	defVkSampler.def("SetDebugName",static_cast<void(*)(lua_State*,Lua::Vulkan::Sampler&,const std::string&)>([](lua_State *l,Lua::Vulkan::Sampler &smp,const std::string &name) {
		Lua::Vulkan::VKContextObject::SetDebugName<Lua::Vulkan::Sampler>(l,smp,name,&Lua::Check<Lua::Vulkan::Sampler>);
	}));
	defVkSampler.def("GetDebugName",static_cast<void(*)(lua_State*,Lua::Vulkan::Sampler&)>([](lua_State *l,Lua::Vulkan::Sampler &smp) {
		Lua::Vulkan::VKContextObject::GetDebugName<Lua::Vulkan::Sampler>(l,smp,&Lua::Check<Lua::Vulkan::Sampler>);
	}));
	vulkanMod[defVkSampler];

	auto defVkFramebuffer = luabind::class_<Lua::Vulkan::Framebuffer>("Framebuffer");
	defVkFramebuffer.def(luabind::tostring(luabind::self));
	defVkFramebuffer.def(luabind::const_self ==luabind::const_self);
	defVkFramebuffer.def("IsValid",&Lua::Vulkan::VKFramebuffer::IsValid);
	vulkanMod[defVkFramebuffer];
	
	auto defVkRenderPass = luabind::class_<Lua::Vulkan::RenderPass>("RenderPass");
	defVkRenderPass.def(luabind::tostring(luabind::self));
	defVkRenderPass.def(luabind::const_self ==luabind::const_self);
	defVkRenderPass.def("IsValid",&Lua::Vulkan::VKRenderPass::IsValid);
	vulkanMod[defVkRenderPass];
	
	auto defVkEvent = luabind::class_<Lua::Vulkan::Event>("Event");
	defVkEvent.def(luabind::tostring(luabind::self));
	defVkEvent.def(luabind::const_self ==luabind::const_self);
	defVkEvent.def("IsValid",&Lua::Vulkan::VKEvent::IsValid);
	vulkanMod[defVkEvent];
	
	auto defVkFence = luabind::class_<Lua::Vulkan::Fence>("Fence");
	defVkFence.def(luabind::tostring(luabind::self));
	defVkFence.def(luabind::const_self ==luabind::const_self);
	defVkFence.def("IsValid",&Lua::Vulkan::VKFence::IsValid);
	vulkanMod[defVkFence];
	
#if 0
	auto defVkSemaphore = luabind::class_<Lua::Vulkan::Semaphore>("Semaphore");
	defVkSemaphore.def(luabind::tostring(luabind::self));
	defVkSemaphore.def("IsValid",&Lua::Vulkan::VKSemaphore::IsValid);
	vulkanMod[defVkSemaphore];
	
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
	vulkanMod[defVkMemory];
#endif
	auto defVkCommandBuffer = luabind::class_<Lua::Vulkan::CommandBuffer>("CommandBuffer");
	defVkCommandBuffer.def(luabind::tostring(luabind::self));
	defVkCommandBuffer.def(luabind::const_self ==luabind::const_self);
	defVkCommandBuffer.def("RecordClearImage",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,Lua::Vulkan::Image&,const Color&,const prosper::util::ClearImageInfo&)>(&Lua::Vulkan::VKCommandBuffer::RecordClearImage));
	defVkCommandBuffer.def("RecordClearImage",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,Lua::Vulkan::Image&,const Color&)>([](lua_State *l,Lua::Vulkan::CommandBuffer &cmdBuffer,Lua::Vulkan::Image &img,const Color &col) {
		Lua::Vulkan::VKCommandBuffer::RecordClearImage(l,cmdBuffer,img,col);
	}));
	defVkCommandBuffer.def("RecordClearImage",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,Lua::Vulkan::Image&,float,const prosper::util::ClearImageInfo&)>(&Lua::Vulkan::VKCommandBuffer::RecordClearImage));
	defVkCommandBuffer.def("RecordClearImage",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,Lua::Vulkan::Image&,float)>([](lua_State *l,Lua::Vulkan::CommandBuffer &cmdBuffer,Lua::Vulkan::Image &img,float depth) {
		Lua::Vulkan::VKCommandBuffer::RecordClearImage(l,cmdBuffer,img,depth);
	}));
	defVkCommandBuffer.def("RecordClearAttachment",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,Lua::Vulkan::Image&,const Color&,uint32_t)>(&Lua::Vulkan::VKCommandBuffer::RecordClearAttachment));
	defVkCommandBuffer.def("RecordClearAttachment",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,Lua::Vulkan::Image&,float)>(&Lua::Vulkan::VKCommandBuffer::RecordClearAttachment));
	defVkCommandBuffer.def("RecordCopyImage",&Lua::Vulkan::VKCommandBuffer::RecordCopyImage);
	defVkCommandBuffer.def("RecordCopyBufferToImage",&Lua::Vulkan::VKCommandBuffer::RecordCopyBufferToImage);
	defVkCommandBuffer.def("RecordCopyBufferToImage",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,Lua::Vulkan::Buffer&,Lua::Vulkan::Image&)>([](lua_State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer,Lua::Vulkan::Buffer &bufSrc,Lua::Vulkan::Image &imgDst) {
		Lua::Vulkan::VKCommandBuffer::RecordCopyBufferToImage(l,hCommandBuffer,bufSrc,imgDst,{});
	}));
	defVkCommandBuffer.def("RecordCopyBuffer",&Lua::Vulkan::VKCommandBuffer::RecordCopyBuffer);
	defVkCommandBuffer.def("RecordUpdateBuffer",&Lua::Vulkan::VKCommandBuffer::RecordUpdateBuffer);
	defVkCommandBuffer.def("RecordBlitImage",&Lua::Vulkan::VKCommandBuffer::RecordBlitImage);
	defVkCommandBuffer.def("RecordResolveImage",&Lua::Vulkan::VKCommandBuffer::RecordResolveImage);
	defVkCommandBuffer.def("RecordBlitTexture",&Lua::Vulkan::VKCommandBuffer::RecordBlitTexture);
	defVkCommandBuffer.def("RecordGenerateMipmaps",&Lua::Vulkan::VKCommandBuffer::RecordGenerateMipmaps);
	defVkCommandBuffer.def("RecordPipelineBarrier",&Lua::Vulkan::VKCommandBuffer::RecordPipelineBarrier);
	defVkCommandBuffer.def("RecordImageBarrier",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,Lua::Vulkan::Image&,uint32_t,uint32_t,const prosper::util::ImageSubresourceRange&)>(&Lua::Vulkan::VKCommandBuffer::RecordImageBarrier));
	defVkCommandBuffer.def("RecordImageBarrier",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,Lua::Vulkan::Image&,uint32_t,uint32_t)>([](lua_State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer,Lua::Vulkan::Image &img,uint32_t oldLayout,uint32_t newLayout) {
		Lua::Vulkan::VKCommandBuffer::RecordImageBarrier(l,hCommandBuffer,img,oldLayout,newLayout,prosper::util::ImageSubresourceRange{});
	}));
	defVkCommandBuffer.def("RecordImageBarrier",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,Lua::Vulkan::Image&,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t)>(&Lua::Vulkan::VKCommandBuffer::RecordImageBarrier));
	defVkCommandBuffer.def("RecordImageBarrier",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,Lua::Vulkan::Image&,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t)>([](lua_State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer,Lua::Vulkan::Image &img,uint32_t srcStageMask,uint32_t dstStageMask,uint32_t oldLayout,uint32_t newLayout,uint32_t srcAccessMask,uint32_t dstAccessMask) {
		Lua::Vulkan::VKCommandBuffer::RecordImageBarrier(l,hCommandBuffer,img,srcStageMask,dstStageMask,oldLayout,newLayout,srcAccessMask,dstAccessMask,0u);
	}));
	defVkCommandBuffer.def("RecordBufferBarrier",&Lua::Vulkan::VKCommandBuffer::RecordBufferBarrier);
	defVkCommandBuffer.def("RecordBufferBarrier",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,Lua::Vulkan::Buffer&,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t)>([](lua_State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer,Lua::Vulkan::Buffer &buf,uint32_t srcStageMask,uint32_t dstStageMask,uint32_t srcAccessMask,uint32_t dstAccessMask,uint32_t offset) {
		Lua::Vulkan::VKCommandBuffer::RecordBufferBarrier(l,hCommandBuffer,buf,srcStageMask,dstStageMask,srcAccessMask,dstAccessMask,offset,std::numeric_limits<uint32_t>::max());
	}));
	defVkCommandBuffer.def("RecordBufferBarrier",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,Lua::Vulkan::Buffer&,uint32_t,uint32_t,uint32_t,uint32_t)>([](lua_State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer,Lua::Vulkan::Buffer &buf,uint32_t srcStageMask,uint32_t dstStageMask,uint32_t srcAccessMask,uint32_t dstAccessMask) {
		Lua::Vulkan::VKCommandBuffer::RecordBufferBarrier(l,hCommandBuffer,buf,srcStageMask,dstStageMask,srcAccessMask,dstAccessMask,0u,std::numeric_limits<uint32_t>::max());
	}));
	defVkCommandBuffer.def("RecordSetViewport",&Lua::Vulkan::VKCommandBuffer::RecordSetViewport);
	defVkCommandBuffer.def("RecordSetScissor",&Lua::Vulkan::VKCommandBuffer::RecordSetScissor);
	defVkCommandBuffer.def("RecordBeginRenderPass",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,Lua::Vulkan::RenderPassInfo&)>([](lua_State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer,Lua::Vulkan::RenderPassInfo &rpInfo) {
		Lua::Vulkan::VKCommandBuffer::RecordBeginRenderPass(l,hCommandBuffer,rpInfo);
	}));
	defVkCommandBuffer.def("RecordEndRenderPass",&Lua::Vulkan::VKCommandBuffer::RecordEndRenderPass);
	defVkCommandBuffer.def("RecordBindIndexBuffer",&Lua::Vulkan::VKCommandBuffer::RecordBindIndexBuffer);
	defVkCommandBuffer.def("RecordBindIndexBuffer",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,Lua::Vulkan::Buffer&,uint32_t)>([](lua_State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer,Lua::Vulkan::Buffer &indexBuffer,uint32_t indexType) {
		Lua::Vulkan::VKCommandBuffer::RecordBindIndexBuffer(l,hCommandBuffer,indexBuffer,indexType,0u);
	}));
	defVkCommandBuffer.def("RecordBindVertexBuffer",&Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffer);
	defVkCommandBuffer.def("RecordBindVertexBuffer",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,prosper::ShaderGraphics&,Lua::Vulkan::Buffer&,uint32_t)>([](lua_State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer,prosper::ShaderGraphics &graphics,Lua::Vulkan::Buffer &vertexBuffer,uint32_t startBinding) {
		Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffer(l,hCommandBuffer,graphics,vertexBuffer,startBinding,0u);
	}));
	defVkCommandBuffer.def("RecordBindVertexBuffer",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,prosper::ShaderGraphics&,Lua::Vulkan::Buffer&)>([](lua_State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer,prosper::ShaderGraphics &graphics,Lua::Vulkan::Buffer &vertexBuffer) {
		Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffer(l,hCommandBuffer,graphics,vertexBuffer,0u,0u);
	}));
	defVkCommandBuffer.def("RecordBindVertexBuffers",&Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffers);
	defVkCommandBuffer.def("RecordBindVertexBuffers",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,prosper::ShaderGraphics&,luabind::object,uint32_t)>([](lua_State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer,prosper::ShaderGraphics &graphics,luabind::object vertexBuffers,uint32_t startBinding) {
		Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffers(l,hCommandBuffer,graphics,vertexBuffers,startBinding,{});
	}));
	defVkCommandBuffer.def("RecordBindVertexBuffers",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,prosper::ShaderGraphics&,luabind::object)>([](lua_State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer,prosper::ShaderGraphics &graphics,luabind::object vertexBuffers) {
		Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffers(l,hCommandBuffer,graphics,vertexBuffers,0u,{});
	}));
	defVkCommandBuffer.def("RecordCopyImageToBuffer",&Lua::Vulkan::VKCommandBuffer::RecordCopyImageToBuffer);
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
	defVkCommandBuffer.def("RecordDraw",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,uint32_t,uint32_t,uint32_t,uint32_t)>([](lua_State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer,uint32_t vertexCount,uint32_t instanceCount,uint32_t firstVertex,uint32_t firstInstance) {
		Lua::Vulkan::VKCommandBuffer::RecordDraw(l,hCommandBuffer,vertexCount,instanceCount,firstVertex,firstInstance);
	}));
	defVkCommandBuffer.def("RecordDraw",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,uint32_t,uint32_t,uint32_t)>([](lua_State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer,uint32_t vertexCount,uint32_t instanceCount,uint32_t firstVertex) {
		Lua::Vulkan::VKCommandBuffer::RecordDraw(l,hCommandBuffer,vertexCount,instanceCount,firstVertex,0u);
	}));
	defVkCommandBuffer.def("RecordDraw",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,uint32_t,uint32_t)>([](lua_State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer,uint32_t vertexCount,uint32_t instanceCount) {
		Lua::Vulkan::VKCommandBuffer::RecordDraw(l,hCommandBuffer,vertexCount,instanceCount,0u,0u);
	}));
	defVkCommandBuffer.def("RecordDraw",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,uint32_t)>([](lua_State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer,uint32_t vertexCount) {
		Lua::Vulkan::VKCommandBuffer::RecordDraw(l,hCommandBuffer,vertexCount,1u,0u,0u);
	}));
	defVkCommandBuffer.def("RecordDrawIndexed",&Lua::Vulkan::VKCommandBuffer::RecordDrawIndexed);
	defVkCommandBuffer.def("RecordDrawIndexed",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,uint32_t,uint32_t,uint32_t)>([](lua_State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer,uint32_t indexCount,uint32_t instanceCount,uint32_t firstIndex) {
		Lua::Vulkan::VKCommandBuffer::RecordDrawIndexed(l,hCommandBuffer,indexCount,instanceCount,firstIndex,0u);
	}));
	defVkCommandBuffer.def("RecordDrawIndexed",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,uint32_t,uint32_t)>([](lua_State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer,uint32_t indexCount,uint32_t instanceCount) {
		Lua::Vulkan::VKCommandBuffer::RecordDrawIndexed(l,hCommandBuffer,indexCount,instanceCount,0u,0u);
	}));
	defVkCommandBuffer.def("RecordDrawIndexed",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,uint32_t)>([](lua_State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer,uint32_t indexCount) {
		Lua::Vulkan::VKCommandBuffer::RecordDrawIndexed(l,hCommandBuffer,indexCount,1u,0u,0u);
	}));
	defVkCommandBuffer.def("RecordDrawIndexedIndirect",&Lua::Vulkan::VKCommandBuffer::RecordDrawIndexedIndirect);
	defVkCommandBuffer.def("RecordDrawIndirect",&Lua::Vulkan::VKCommandBuffer::RecordDrawIndirect);
	defVkCommandBuffer.def("RecordFillBuffer",&Lua::Vulkan::VKCommandBuffer::RecordFillBuffer);
	//defVkCommandBuffer.def("RecordResetEvent",&Lua::Vulkan::VKCommandBuffer::RecordResetEvent);
	defVkCommandBuffer.def("RecordSetBlendConstants",&Lua::Vulkan::VKCommandBuffer::RecordSetBlendConstants);
	defVkCommandBuffer.def("RecordSetDepthBias",&Lua::Vulkan::VKCommandBuffer::RecordSetDepthBias);
	defVkCommandBuffer.def("RecordSetDepthBounds",&Lua::Vulkan::VKCommandBuffer::RecordSetDepthBounds);
	//defVkCommandBuffer.def("RecordSetEvent",&Lua::Vulkan::VKCommandBuffer::RecordSetEvent);
	defVkCommandBuffer.def("RecordSetLineWidth",&Lua::Vulkan::VKCommandBuffer::RecordSetLineWidth);
	defVkCommandBuffer.def("RecordSetStencilCompareMask",&Lua::Vulkan::VKCommandBuffer::RecordSetStencilCompareMask);
	defVkCommandBuffer.def("RecordSetStencilReference",&Lua::Vulkan::VKCommandBuffer::RecordSetStencilReference);
	defVkCommandBuffer.def("RecordSetStencilWriteMask",&Lua::Vulkan::VKCommandBuffer::RecordSetStencilWriteMask);
	//defVkCommandBuffer.def("RecordWaitEvents",&Lua::Vulkan::VKCommandBuffer::RecordWaitEvents); // TODO
	//defVkCommandBuffer.def("RecordWriteTimestamp",&Lua::Vulkan::VKCommandBuffer::RecordWriteTimestamp); // TODO
	//defVkCommandBuffer.def("RecordBeginQuery",&Lua::Vulkan::VKCommandBuffer::RecordBeginQuery); // TODO
	//defVkCommandBuffer.def("RecordEndQuery",&Lua::Vulkan::VKCommandBuffer::RecordEndQuery); // TODO
	defVkCommandBuffer.def("RecordDrawGradient",&Lua::Vulkan::VKCommandBuffer::RecordDrawGradient);
	defVkCommandBuffer.def("StopRecording",&Lua::Vulkan::VKCommandBuffer::StopRecording);
	defVkCommandBuffer.def("StartRecording",&Lua::Vulkan::VKCommandBuffer::StartRecording);
	defVkCommandBuffer.def("SetDebugName",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,const std::string&)>([](lua_State *l,Lua::Vulkan::CommandBuffer &cb,const std::string &name) {
		Lua::Vulkan::VKContextObject::SetDebugName<Lua::Vulkan::CommandBuffer>(l,cb,name,&Lua::Check<Lua::Vulkan::CommandBuffer>);
	}));
	defVkCommandBuffer.def("GetDebugName",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&)>([](lua_State *l,Lua::Vulkan::CommandBuffer &cb) {
		Lua::Vulkan::VKContextObject::GetDebugName<Lua::Vulkan::CommandBuffer>(l,cb,&Lua::Check<Lua::Vulkan::CommandBuffer>);
	}));
	vulkanMod[defVkCommandBuffer];

	auto devVkBuffer = luabind::class_<Lua::Vulkan::Buffer>("Buffer");
	devVkBuffer.def(luabind::tostring(luabind::self));
	devVkBuffer.def(luabind::const_self ==luabind::const_self);
	devVkBuffer.def("IsValid",&Lua::Vulkan::VKBuffer::IsValid);
	devVkBuffer.def("GetStartOffset",&Lua::Vulkan::VKBuffer::GetStartOffset);
	devVkBuffer.def("GetBaseIndex",&Lua::Vulkan::VKBuffer::GetBaseIndex);
	devVkBuffer.def("GetSize",&Lua::Vulkan::VKBuffer::GetSize);
	devVkBuffer.def("GetUsageFlags",&Lua::Vulkan::VKBuffer::GetUsageFlags);
	devVkBuffer.def("SetDebugName",static_cast<void(*)(lua_State*,Lua::Vulkan::Buffer&,const std::string&)>([](lua_State *l,Lua::Vulkan::Buffer &buf,const std::string &name) {
		Lua::Vulkan::VKContextObject::SetDebugName<Lua::Vulkan::Buffer>(l,buf,name,&Lua::Check<Lua::Vulkan::Buffer>);
	}));
	devVkBuffer.def("GetDebugName",static_cast<void(*)(lua_State*,Lua::Vulkan::Buffer&)>([](lua_State *l,Lua::Vulkan::Buffer &buf) {
		Lua::Vulkan::VKContextObject::GetDebugName<Lua::Vulkan::Buffer>(l,buf,&Lua::Check<Lua::Vulkan::Buffer>);
	}));
	devVkBuffer.def("GetCreateInfo",static_cast<void(*)(lua_State*,Lua::Vulkan::Buffer&)>([](lua_State *l,Lua::Vulkan::Buffer &buf) {
		Lua::Push(l,buf.GetCreateInfo());
	}));
	devVkBuffer.def("SetPermanentlyMapped",&Lua::Vulkan::VKBuffer::SetPermanentlyMapped);
	devVkBuffer.def("GetParent",&Lua::Vulkan::VKBuffer::GetParent);
	devVkBuffer.def("WriteMemory",&Lua::Vulkan::VKBuffer::Write);
	devVkBuffer.def("WriteMemory",static_cast<void(*)(lua_State*,Lua::Vulkan::Buffer&,uint32_t,::DataStream&)>([](lua_State *l,Lua::Vulkan::Buffer &hBuffer,uint32_t offset,::DataStream &ds) {
		Lua::Vulkan::VKBuffer::Write(l,hBuffer,offset,ds,0u,ds->GetSize());
	}));
	devVkBuffer.def("ReadMemory",static_cast<void(*)(lua_State*,Lua::Vulkan::Buffer&,uint32_t,uint32_t,::DataStream&,uint32_t)>(&Lua::Vulkan::VKBuffer::Read));
	devVkBuffer.def("ReadMemory",static_cast<void(*)(lua_State*,Lua::Vulkan::Buffer&,uint32_t,uint32_t,::DataStream&)>([](lua_State *l,Lua::Vulkan::Buffer &hBuffer,uint32_t offset,uint32_t size,::DataStream &ds) {
		Lua::Vulkan::VKBuffer::Read(l,hBuffer,offset,size,ds,0u);
	}));
	devVkBuffer.def("ReadMemory",static_cast<void(*)(lua_State*,Lua::Vulkan::Buffer&,uint32_t,uint32_t)>(&Lua::Vulkan::VKBuffer::Read));
	devVkBuffer.def("ReadMemory",static_cast<void(*)(lua_State*,Lua::Vulkan::Buffer&)>([](lua_State *l,Lua::Vulkan::Buffer &hBuffer) {
		Lua::Vulkan::VKBuffer::Read(l,hBuffer,0u,hBuffer.GetSize());
	}));
	devVkBuffer.def("MapMemory",&Lua::Vulkan::VKBuffer::Map);
	devVkBuffer.def("MapMemory",static_cast<void(*)(lua_State*,Lua::Vulkan::Buffer&)>([](lua_State *l,Lua::Vulkan::Buffer &hBuffer) {
		Lua::Vulkan::VKBuffer::Map(l,hBuffer,0u,hBuffer.GetSize());
	}));
	devVkBuffer.def("UnmapMemory",&Lua::Vulkan::VKBuffer::Unmap);
	vulkanMod[devVkBuffer];

	auto defVkDescriptorSet = luabind::class_<Lua::Vulkan::DescriptorSet>("DescriptorSet");
	defVkDescriptorSet.def(luabind::tostring(luabind::self));
	defVkDescriptorSet.def(luabind::const_self ==luabind::const_self);
#if 0
	defVkDescriptorSet.def("GetBindingInfo",static_cast<void(*)(lua_State*,Lua::Vulkan::DescriptorSet&,uint32_t)>(&Lua::Vulkan::VKDescriptorSet::GetBindingInfo));
	defVkDescriptorSet.def("GetBindingInfo",static_cast<void(*)(lua_State*,Lua::Vulkan::DescriptorSet&)>(&Lua::Vulkan::VKDescriptorSet::GetBindingInfo));
#endif
	defVkDescriptorSet.def("GetBindingCount",&Lua::Vulkan::VKDescriptorSet::GetBindingCount);
	defVkDescriptorSet.def("SetBindingTexture",static_cast<void(*)(lua_State*,Lua::Vulkan::DescriptorSet&,uint32_t,Lua::Vulkan::Texture&,uint32_t)>([](lua_State *l,Lua::Vulkan::DescriptorSet &hDescSet,uint32_t bindingIdx,Lua::Vulkan::Texture &texture,uint32_t layerId) {
		Lua::Vulkan::VKDescriptorSet::SetBindingTexture(l,hDescSet,bindingIdx,texture,layerId);
	}));
	defVkDescriptorSet.def("SetBindingTexture",static_cast<void(*)(lua_State*,Lua::Vulkan::DescriptorSet&,uint32_t,Lua::Vulkan::Texture&)>([](lua_State *l,Lua::Vulkan::DescriptorSet &hDescSet,uint32_t bindingIdx,Lua::Vulkan::Texture &texture) {
		Lua::Vulkan::VKDescriptorSet::SetBindingTexture(l,hDescSet,bindingIdx,texture);
	}));
	defVkDescriptorSet.def("SetBindingArrayTexture",static_cast<void(*)(lua_State*,Lua::Vulkan::DescriptorSet&,uint32_t,Lua::Vulkan::Texture&,uint32_t,uint32_t)>(&Lua::Vulkan::VKDescriptorSet::SetBindingArrayTexture));
	defVkDescriptorSet.def("SetBindingArrayTexture",static_cast<void(*)(lua_State*,Lua::Vulkan::DescriptorSet&,uint32_t,Lua::Vulkan::Texture&,uint32_t)>([](lua_State *l,Lua::Vulkan::DescriptorSet &hDescSet,uint32_t bindingIdx,Lua::Vulkan::Texture &texture,uint32_t arrayIdx) {
		Lua::Vulkan::VKDescriptorSet::SetBindingArrayTexture(l,hDescSet,bindingIdx,texture,arrayIdx);
	}));
	defVkDescriptorSet.def("SetBindingUniformBuffer",&Lua::Vulkan::VKDescriptorSet::SetBindingUniformBuffer);
	defVkDescriptorSet.def("SetBindingUniformBuffer",static_cast<void(*)(lua_State*,Lua::Vulkan::DescriptorSet&,uint32_t,Lua::Vulkan::Buffer&,uint32_t)>([](lua_State *l,Lua::Vulkan::DescriptorSet &hDescSet,uint32_t bindingIdx,Lua::Vulkan::Buffer &buffer,uint32_t startOffset) {
		Lua::Vulkan::VKDescriptorSet::SetBindingUniformBuffer(l,hDescSet,bindingIdx,buffer,startOffset,std::numeric_limits<uint32_t>::max());
	}));
	defVkDescriptorSet.def("SetBindingUniformBuffer",static_cast<void(*)(lua_State*,Lua::Vulkan::DescriptorSet&,uint32_t,Lua::Vulkan::Buffer&)>([](lua_State *l,Lua::Vulkan::DescriptorSet &hDescSet,uint32_t bindingIdx,Lua::Vulkan::Buffer &buffer) {
		Lua::Vulkan::VKDescriptorSet::SetBindingUniformBuffer(l,hDescSet,bindingIdx,buffer,0u,std::numeric_limits<uint32_t>::max());
	}));
	defVkDescriptorSet.def("SetBindingUniformBufferDynamic",&Lua::Vulkan::VKDescriptorSet::SetBindingUniformBufferDynamic);
	defVkDescriptorSet.def("SetBindingUniformBufferDynamic",static_cast<void(*)(lua_State*,Lua::Vulkan::DescriptorSet&,uint32_t,Lua::Vulkan::Buffer&,uint32_t)>([](lua_State *l,Lua::Vulkan::DescriptorSet &hDescSet,uint32_t bindingIdx,Lua::Vulkan::Buffer &buffer,uint32_t startOffset) {
		Lua::Vulkan::VKDescriptorSet::SetBindingUniformBufferDynamic(l,hDescSet,bindingIdx,buffer,startOffset,std::numeric_limits<uint32_t>::max());
	}));
	defVkDescriptorSet.def("SetBindingUniformBufferDynamic",static_cast<void(*)(lua_State*,Lua::Vulkan::DescriptorSet&,uint32_t,Lua::Vulkan::Buffer&)>([](lua_State *l,Lua::Vulkan::DescriptorSet &hDescSet,uint32_t bindingIdx,Lua::Vulkan::Buffer &buffer) {
		Lua::Vulkan::VKDescriptorSet::SetBindingUniformBufferDynamic(l,hDescSet,bindingIdx,buffer,0u,std::numeric_limits<uint32_t>::max());
	}));
	defVkDescriptorSet.def("SetBindingStorageBuffer",&Lua::Vulkan::VKDescriptorSet::SetBindingStorageBuffer);
	defVkDescriptorSet.def("SetBindingStorageBuffer",static_cast<void(*)(lua_State*,Lua::Vulkan::DescriptorSet&,uint32_t,Lua::Vulkan::Buffer&,uint32_t)>([](lua_State *l,Lua::Vulkan::DescriptorSet &hDescSet,uint32_t bindingIdx,Lua::Vulkan::Buffer &buffer,uint32_t startOffset) {
		Lua::Vulkan::VKDescriptorSet::SetBindingStorageBuffer(l,hDescSet,bindingIdx,buffer,startOffset,std::numeric_limits<uint32_t>::max());
	}));
	defVkDescriptorSet.def("SetBindingStorageBuffer",static_cast<void(*)(lua_State*,Lua::Vulkan::DescriptorSet&,uint32_t,Lua::Vulkan::Buffer&)>([](lua_State *l,Lua::Vulkan::DescriptorSet &hDescSet,uint32_t bindingIdx,Lua::Vulkan::Buffer &buffer) {
		Lua::Vulkan::VKDescriptorSet::SetBindingStorageBuffer(l,hDescSet,bindingIdx,buffer,0u,std::numeric_limits<uint32_t>::max());
	}));
	defVkDescriptorSet.def("SetDebugName",static_cast<void(*)(lua_State*,Lua::Vulkan::DescriptorSet&,const std::string&)>([](lua_State *l,Lua::Vulkan::DescriptorSet &ds,const std::string &name) {
		Lua::Vulkan::VKContextObject::SetDebugName<Lua::Vulkan::DescriptorSet>(l,ds,name,&Lua::Check<Lua::Vulkan::DescriptorSet>);
	}));
	defVkDescriptorSet.def("GetDebugName",static_cast<void(*)(lua_State*,Lua::Vulkan::DescriptorSet&)>([](lua_State *l,Lua::Vulkan::DescriptorSet &ds) {
		Lua::Vulkan::VKContextObject::GetDebugName<Lua::Vulkan::DescriptorSet>(l,ds,&Lua::Check<Lua::Vulkan::DescriptorSet>);
	}));
	vulkanMod[defVkDescriptorSet];
	
	auto defVkMesh = luabind::class_<pragma::VkMesh>("Mesh");
	defVkMesh.def(luabind::tostring(luabind::self));
	defVkMesh.def(luabind::const_self ==luabind::const_self);
	defVkMesh.def("GetVertexBuffer",&Lua::Vulkan::VKMesh::GetVertexBuffer);
	defVkMesh.def("GetVertexWeightBuffer",&Lua::Vulkan::VKMesh::GetVertexWeightBuffer);
	defVkMesh.def("GetAlphaBuffer",&Lua::Vulkan::VKMesh::GetAlphaBuffer);
	defVkMesh.def("GetIndexBuffer",&Lua::Vulkan::VKMesh::GetIndexBuffer);
	defVkMesh.def("SetVertexBuffer",&Lua::Vulkan::VKMesh::SetVertexBuffer);
	defVkMesh.def("SetVertexWeightBuffer",&Lua::Vulkan::VKMesh::SetVertexWeightBuffer);
	defVkMesh.def("SetAlphaBuffer",&Lua::Vulkan::VKMesh::SetAlphaBuffer);
	defVkMesh.def("SetIndexBuffer",&Lua::Vulkan::VKMesh::SetIndexBuffer);
	vulkanMod[defVkMesh];
	
	auto defVkRenderTarget = luabind::class_<Lua::Vulkan::RenderTarget>("RenderTarget");
	defVkRenderTarget.def(luabind::tostring(luabind::self));
	defVkRenderTarget.def(luabind::const_self ==luabind::const_self);
	defVkRenderTarget.def("GetTexture",&Lua::Vulkan::VKRenderTarget::GetTexture);
	defVkRenderTarget.def("GetTexture",static_cast<void(*)(lua_State*,Lua::Vulkan::RenderTarget&)>([](lua_State *l,Lua::Vulkan::RenderTarget &rt) {
		Lua::Vulkan::VKRenderTarget::GetTexture(l,rt,0u);
	}));
	defVkRenderTarget.def("GetRenderPass",&Lua::Vulkan::VKRenderTarget::GetRenderPass);
	defVkRenderTarget.def("GetFramebuffer",&Lua::Vulkan::VKRenderTarget::GetFramebuffer);
	defVkRenderTarget.def("GetWidth",&Lua::Vulkan::VKRenderTarget::GetWidth);
	defVkRenderTarget.def("GetHeight",&Lua::Vulkan::VKRenderTarget::GetHeight);
	defVkRenderTarget.def("GetFormat",&Lua::Vulkan::VKRenderTarget::GetFormat);
	defVkRenderTarget.def("SetDebugName",static_cast<void(*)(lua_State*,Lua::Vulkan::RenderTarget&,const std::string&)>([](lua_State *l,Lua::Vulkan::RenderTarget &rt,const std::string &name) {
		Lua::Vulkan::VKContextObject::SetDebugName<Lua::Vulkan::RenderTarget>(l,rt,name,&Lua::Check<Lua::Vulkan::RenderTarget>);
	}));
	defVkRenderTarget.def("GetDebugName",static_cast<void(*)(lua_State*,Lua::Vulkan::RenderTarget&)>([](lua_State *l,Lua::Vulkan::RenderTarget &rt) {
		Lua::Vulkan::VKContextObject::GetDebugName<Lua::Vulkan::RenderTarget>(l,rt,&Lua::Check<Lua::Vulkan::RenderTarget>);
	}));
	vulkanMod[defVkRenderTarget];
	
	auto defVkTimestampQuery = luabind::class_<Lua::Vulkan::TimestampQuery>("TimestampQuery");
	defVkTimestampQuery.def(luabind::tostring(luabind::self));
	defVkTimestampQuery.def(luabind::const_self ==luabind::const_self);
	defVkTimestampQuery.def("IsValid",&Lua::Vulkan::VKTimestampQuery::IsValid);
	vulkanMod[defVkTimestampQuery];
	
	auto defVkTimerQuery = luabind::class_<Lua::Vulkan::TimerQuery>("TimerQuery");
	defVkTimerQuery.def(luabind::tostring(luabind::self));
	defVkTimerQuery.def(luabind::const_self ==luabind::const_self);
	defVkTimerQuery.def("IsValid",&Lua::Vulkan::VKTimerQuery::IsValid);
	vulkanMod[defVkTimerQuery];

	auto defClearValue = luabind::class_<Lua::Vulkan::ClearValue>("ClearValue");
	defClearValue.def(luabind::constructor<>());
	defClearValue.def(luabind::constructor<const Color&>());
	defClearValue.def(luabind::constructor<float,uint32_t>());
	defClearValue.def(luabind::constructor<float>());
	defClearValue.def("SetColor",static_cast<void(*)(lua_State*,Lua::Vulkan::ClearValue&,const Color&)>([](lua_State *l,Lua::Vulkan::ClearValue &clearValue,const Color &clearColor) {
		clearValue.clearValue.setColor(prosper::ClearColorValue{std::array<float,4>{clearColor.r /255.f,clearColor.g /255.f,clearColor.b /255.f,clearColor.a /255.f}});
	}));
	defClearValue.def("SetDepthStencil",static_cast<void(*)(lua_State*,Lua::Vulkan::ClearValue&,float)>([](lua_State *l,Lua::Vulkan::ClearValue &clearValue,float depth) {
		clearValue.clearValue.setDepthStencil(prosper::ClearDepthStencilValue{depth});
	}));
	defClearValue.def("SetDepthStencil",static_cast<void(*)(lua_State*,Lua::Vulkan::ClearValue&,float,uint32_t)>([](lua_State *l,Lua::Vulkan::ClearValue &clearValue,float depth,uint32_t stencil) {
		clearValue.clearValue.setDepthStencil(prosper::ClearDepthStencilValue{depth,stencil});
	}));
	vulkanMod[defClearValue];

	auto defRenderPassInfo = luabind::class_<Lua::Vulkan::RenderPassInfo>("RenderPassInfo");
	defRenderPassInfo.def(luabind::constructor<const std::shared_ptr<Lua::Vulkan::RenderTarget>&>());
	defRenderPassInfo.def("SetLayer",static_cast<void(*)(lua_State*,Lua::Vulkan::RenderPassInfo&)>([](lua_State *l,Lua::Vulkan::RenderPassInfo &rpInfo) {
		rpInfo.layerId = {};
	}));
	defRenderPassInfo.def("SetLayer",static_cast<void(*)(lua_State*,Lua::Vulkan::RenderPassInfo&,uint32_t)>([](lua_State *l,Lua::Vulkan::RenderPassInfo &rpInfo,uint32_t layerId) {
		rpInfo.layerId = layerId;
	}));
	defRenderPassInfo.def("SetClearValues",static_cast<void(*)(lua_State*,Lua::Vulkan::RenderPassInfo&,luabind::object)>([](lua_State *l,Lua::Vulkan::RenderPassInfo &rpInfo,luabind::object tClearValues) {
		auto t = 2;
		Lua::CheckInt(l,t);
		auto numClearValues = Lua::GetObjectLength(l,t);
		rpInfo.clearValues.clear();
		rpInfo.clearValues.reserve(numClearValues);
		for(auto i=decltype(numClearValues){0u};i<numClearValues;++i)
		{
			Lua::PushInt(l,i +1u);
			Lua::GetTableValue(l,t);
			auto &clearVal = Lua::Check<Lua::Vulkan::ClearValue>(l,-1);
			rpInfo.clearValues.push_back(clearVal);
			Lua::Pop(l,1);
		}
	}));
	defRenderPassInfo.def("AddClearValue",static_cast<void(*)(lua_State*,Lua::Vulkan::RenderPassInfo&,Lua::Vulkan::ClearValue&)>([](lua_State *l,Lua::Vulkan::RenderPassInfo &rpInfo,Lua::Vulkan::ClearValue &clearValue) {
		rpInfo.clearValues.push_back(clearValue);
	}));
	defRenderPassInfo.def("SetRenderPass",static_cast<void(*)(lua_State*,Lua::Vulkan::RenderPassInfo&)>([](lua_State *l,Lua::Vulkan::RenderPassInfo &rpInfo) {
		rpInfo.renderPass = nullptr;
	}));
	defRenderPassInfo.def("SetRenderPass",static_cast<void(*)(lua_State*,Lua::Vulkan::RenderPassInfo&,Lua::Vulkan::RenderPass&)>([](lua_State *l,Lua::Vulkan::RenderPassInfo &rpInfo,Lua::Vulkan::RenderPass &rp) {
		rpInfo.renderPass = rp.shared_from_this();
	}));
	vulkanMod[defRenderPassInfo];
}

/////////////////////////////////

void Lua::Vulkan::VKTexture::IsValid(lua_State *l,Texture &hTex)
{
	Lua::PushBool(l,true);
}
void Lua::Vulkan::VKTexture::GetImage(lua_State *l,Texture &hTex)
{
	auto &img = hTex.GetImage();
	Lua::Push<std::shared_ptr<Image>>(l,img.shared_from_this());
}
void Lua::Vulkan::VKTexture::GetImageView(lua_State *l,Texture &hTex)
{
	auto *imgView = hTex.GetImageView();
	if(imgView != nullptr)
		Lua::Push<std::shared_ptr<ImageView>>(l,imgView->shared_from_this());
}
void Lua::Vulkan::VKTexture::GetImageView(lua_State *l,Texture &hTex,uint32_t layerId)
{
	auto *imgView = hTex.GetImageView(layerId);
	if(imgView != nullptr)
		Lua::Push<std::shared_ptr<ImageView>>(l,imgView->shared_from_this());
}
void Lua::Vulkan::VKTexture::GetSampler(lua_State *l,Texture &hTex)
{
	auto *sampler = hTex.GetSampler();
	if(sampler != nullptr)
		Lua::Push<std::shared_ptr<Sampler>>(l,sampler->shared_from_this());
}
void Lua::Vulkan::VKTexture::GetWidth(lua_State *l,Texture &hTex)
{
	auto &img = hTex.GetImage();
	auto extents = img.GetExtents();
	Lua::PushInt(l,extents.width);
}
void Lua::Vulkan::VKTexture::GetHeight(lua_State *l,Texture &hTex)
{
	auto &img = hTex.GetImage();
	auto extents = img.GetExtents();
	Lua::PushInt(l,extents.height);
}
void Lua::Vulkan::VKTexture::GetFormat(lua_State *l,Texture &hTex)
{
	auto &img = hTex.GetImage();
	Lua::PushInt(l,umath::to_integral(img.GetFormat()));
}
void Lua::Vulkan::VKTexture::IsMSAATexture(lua_State *l,Texture &hTex)
{
	Lua::PushBool(l,hTex.IsMSAATexture());
}

/////////////////////////////////

void Lua::Vulkan::VKImage::IsValid(lua_State *l,Image &hImg)
{
	Lua::PushBool(l,true);
}
void Lua::Vulkan::VKImage::GetAspectSubresourceLayout(lua_State *l,Image &hImg,uint32_t layer,uint32_t mipmap)
{
	auto layout = hImg.GetSubresourceLayout(layer,mipmap);
	if(layout.has_value() == false)
		return;
	Lua::Push(l,*layout);
}
void Lua::Vulkan::VKImage::GetAlignment(lua_State *l,Image &hImg)
{
	Lua::PushInt(l,hImg.GetAlignment()); // TODO: Plane parameter
}
void Lua::Vulkan::VKImage::GetExtent2D(lua_State *l,Image &hImg,uint32_t mipmap)
{
	auto extents = hImg.GetExtents(mipmap);
	Lua::PushInt(l,extents.width);
	Lua::PushInt(l,extents.height);
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
void Lua::Vulkan::VKImage::GetFormat(lua_State *l,Image &hImg)
{
	Lua::PushInt(l,static_cast<int32_t>(hImg.GetFormat()));
}
void Lua::Vulkan::VKImage::GetMipmapSize(lua_State *l,Image &hImg,uint32_t mipmap)
{
	uint32_t width;
	uint32_t height;
	auto extents = hImg.GetExtents(mipmap);
	Lua::PushInt(l,extents.width);
	Lua::PushInt(l,extents.height);
}
void Lua::Vulkan::VKImage::GetLayerCount(lua_State *l,Image &hImg)
{
	Lua::PushInt(l,hImg.GetLayerCount());
}
void Lua::Vulkan::VKImage::GetMipmapCount(lua_State *l,Image &hImg)
{
	Lua::PushInt(l,hImg.GetMipmapCount());
}
void Lua::Vulkan::VKImage::GetSampleCount(lua_State *l,Image &hImg)
{
	Lua::PushInt(l,umath::to_integral(hImg.GetSampleCount()));
}
void Lua::Vulkan::VKImage::GetSharingMode(lua_State *l,Image &hImg)
{
	Lua::PushInt(l,umath::to_integral(hImg.GetSharingMode()));
}
void Lua::Vulkan::VKImage::GetTiling(lua_State *l,Image &hImg)
{
	Lua::PushInt(l,static_cast<uint32_t>(hImg.GetTiling()));
}
void Lua::Vulkan::VKImage::GetType(lua_State *l,Image &hImg)
{
	Lua::PushInt(l,static_cast<uint32_t>(hImg.GetType()));
}
void Lua::Vulkan::VKImage::GetUsage(lua_State *l,Image &hImg)
{
	Lua::PushInt(l,umath::to_integral(hImg.GetCreateInfo().usage));
}
void Lua::Vulkan::VKImage::GetWidth(lua_State *l,Image &hImg)
{
	auto extents = hImg.GetExtents();
	Lua::PushInt(l,extents.width);
}
void Lua::Vulkan::VKImage::GetHeight(lua_State *l,Image &hImg)
{
	auto extents = hImg.GetExtents();
	Lua::PushInt(l,extents.height);
}

/////////////////////////////////

void Lua::Vulkan::VKImageView::IsValid(lua_State *l,ImageView &hImgView)
{
	Lua::PushBool(l,true);
}
void Lua::Vulkan::VKImageView::GetAspectMask(lua_State *l,ImageView &hImgView)
{
	Lua::PushInt(l,umath::to_integral(hImgView.GetAspectMask()));
}
void Lua::Vulkan::VKImageView::GetBaseLayer(lua_State *l,ImageView &hImgView)
{
	Lua::PushInt(l,hImgView.GetBaseLayer());
}
void Lua::Vulkan::VKImageView::GetBaseMipmapLevel(lua_State *l,ImageView &hImgView)
{
	Lua::PushInt(l,hImgView.GetBaseMipmapLevel());
}
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
void Lua::Vulkan::VKImageView::GetImageFormat(lua_State *l,ImageView &hImgView)
{
	Lua::PushInt(l,hImgView.GetFormat());
}
void Lua::Vulkan::VKImageView::GetLayerCount(lua_State *l,ImageView &hImgView)
{
	Lua::PushInt(l,hImgView.GetLayerCount());
}
void Lua::Vulkan::VKImageView::GetMipmapCount(lua_State *l,ImageView &hImgView)
{
	Lua::PushInt(l,hImgView.GetMipmapCount());
}
void Lua::Vulkan::VKImageView::GetSwizzleArray(lua_State *l,ImageView &hImgView)
{
	auto swizzle = hImgView.GetSwizzleArray();
	auto t = Lua::CreateTable(l);
	for(auto i=decltype(swizzle.size()){0};i<swizzle.size();++i)
	{
		Lua::PushInt(l,i +1);
		Lua::PushInt(l,static_cast<uint32_t>(swizzle.at(i)));
		Lua::SetTableValue(l,t);
	}
}
void Lua::Vulkan::VKImageView::GetType(lua_State *l,ImageView &hImgView)
{
	Lua::PushInt(l,static_cast<uint32_t>(hImgView.GetType()));
}

/////////////////////////////////

void Lua::Vulkan::VKSampler::IsValid(lua_State *l,Sampler &hSampler)
{
	Lua::PushBool(l,true);
}
void Lua::Vulkan::VKSampler::Update(lua_State *l,Sampler &hSampler)
{
	hSampler.Update();
}
void Lua::Vulkan::VKSampler::GetMagFilter(lua_State *l,Sampler &hSampler)
{
	Lua::PushInt(l,static_cast<uint32_t>(hSampler.GetMagFilter()));
}
void Lua::Vulkan::VKSampler::GetMinFilter(lua_State *l,Sampler &hSampler)
{
	Lua::PushInt(l,static_cast<uint32_t>(hSampler.GetMinFilter()));
}
void Lua::Vulkan::VKSampler::GetMipmapMode(lua_State *l,Sampler &hSampler)
{
	Lua::PushInt(l,static_cast<uint32_t>(hSampler.GetMipmapMode()));
}
void Lua::Vulkan::VKSampler::GetAddressModeU(lua_State *l,Sampler &hSampler)
{
	Lua::PushInt(l,static_cast<uint32_t>(hSampler.GetAddressModeU()));
}
void Lua::Vulkan::VKSampler::GetAddressModeV(lua_State *l,Sampler &hSampler)
{
	Lua::PushInt(l,static_cast<uint32_t>(hSampler.GetAddressModeV()));
}
void Lua::Vulkan::VKSampler::GetAddressModeW(lua_State *l,Sampler &hSampler)
{
	Lua::PushInt(l,static_cast<uint32_t>(hSampler.GetAddressModeW()));
}
void Lua::Vulkan::VKSampler::GetMipLodBias(lua_State *l,Sampler &hSampler)
{
	Lua::PushNumber(l,hSampler.GetLodBias());
}
void Lua::Vulkan::VKSampler::GetAnisotropyEnabled(lua_State *l,Sampler &hSampler)
{
	Lua::PushBool(l,hSampler.GetMaxAnisotropy() != 0.f);
}
void Lua::Vulkan::VKSampler::GetMaxAnisotropy(lua_State *l,Sampler &hSampler)
{
	Lua::PushNumber(l,hSampler.GetMaxAnisotropy());
}
void Lua::Vulkan::VKSampler::GetCompareEnabled(lua_State *l,Sampler &hSampler)
{
	Lua::PushBool(l,hSampler.GetCompareEnabled());
}
void Lua::Vulkan::VKSampler::GetCompareOperation(lua_State *l,Sampler &hSampler)
{
	Lua::PushInt(l,static_cast<uint32_t>(hSampler.GetCompareOp()));
}
void Lua::Vulkan::VKSampler::GetMinLod(lua_State *l,Sampler &hSampler)
{
	Lua::PushNumber(l,hSampler.GetMinLod());
}
void Lua::Vulkan::VKSampler::GetMaxLod(lua_State *l,Sampler &hSampler)
{
	Lua::PushNumber(l,hSampler.GetMaxLod());
}
void Lua::Vulkan::VKSampler::GetBorderColor(lua_State *l,Sampler &hSampler)
{
	Lua::PushInt(l,static_cast<uint32_t>(hSampler.GetBorderColor()));
}
#if 0
void Lua::Vulkan::VKSampler::GetUseUnnormalizedCoordinates(lua_State *l,Sampler &hSampler)
{
	Lua::PushBool(l,hSampler.GetUseUnnormalizedCoordinates());
}
#endif
void Lua::Vulkan::VKSampler::SetMagFilter(lua_State *l,Sampler &hSampler,int32_t magFilter)
{
	hSampler.SetMagFilter(static_cast<prosper::Filter>(magFilter));
}
void Lua::Vulkan::VKSampler::SetMinFilter(lua_State *l,Sampler &hSampler,int32_t minFilter)
{
	hSampler.SetMinFilter(static_cast<prosper::Filter>(minFilter));
}
void Lua::Vulkan::VKSampler::SetMipmapMode(lua_State *l,Sampler &hSampler,int32_t mipmapMode)
{
	hSampler.SetMipmapMode(static_cast<prosper::SamplerMipmapMode>(mipmapMode));
}
void Lua::Vulkan::VKSampler::SetAddressModeU(lua_State *l,Sampler &hSampler,int32_t addressModeU)
{
	hSampler.SetAddressModeU(static_cast<prosper::SamplerAddressMode>(addressModeU));
}
void Lua::Vulkan::VKSampler::SetAddressModeV(lua_State *l,Sampler &hSampler,int32_t addressModeV)
{
	hSampler.SetAddressModeV(static_cast<prosper::SamplerAddressMode>(addressModeV));
}
void Lua::Vulkan::VKSampler::SetAddressModeW(lua_State *l,Sampler &hSampler,int32_t addressModeW)
{
	hSampler.SetAddressModeW(static_cast<prosper::SamplerAddressMode>(addressModeW));
}
void Lua::Vulkan::VKSampler::SetMipLodBias(lua_State *l,Sampler &hSampler,float mipLodBias)
{
	//hSampler.SetMipLodBias(mipLodBias); // TODO
}
void Lua::Vulkan::VKSampler::SetAnisotropyEnabled(lua_State *l,Sampler &hSampler,bool anisotropyEnabled)
{
	//hSampler.SetAnisotropyEnabled(anisotropyEnabled); // TODO
}
void Lua::Vulkan::VKSampler::SetMaxAnisotropy(lua_State *l,Sampler &hSampler,float maxAnisotropy)
{
	hSampler.SetMaxAnisotropy(maxAnisotropy);
}
void Lua::Vulkan::VKSampler::SetCompareEnabled(lua_State *l,Sampler &hSampler,bool compareEnabled)
{
	hSampler.SetCompareEnable(compareEnabled);
}
void Lua::Vulkan::VKSampler::SetCompareOperation(lua_State *l,Sampler &hSampler,int32_t compareOp)
{
	hSampler.SetCompareOp(static_cast<prosper::CompareOp>(compareOp));
}
void Lua::Vulkan::VKSampler::SetMinLod(lua_State *l,Sampler &hSampler,float minLod)
{
	hSampler.SetMinLod(minLod);
}
void Lua::Vulkan::VKSampler::SetMaxLod(lua_State *l,Sampler &hSampler,float maxLod)
{
	hSampler.SetMaxLod(maxLod);
}
void Lua::Vulkan::VKSampler::SetBorderColor(lua_State *l,Sampler &hSampler,int32_t borderColor)
{
	hSampler.SetBorderColor(static_cast<prosper::BorderColor>(borderColor));
}
#if 0
void Lua::Vulkan::VKSampler::SetUseUnnormalizedCoordinates(lua_State *l,Sampler &hSampler,bool bUnnormalizedCoordinates)
{
	hSampler.SetUseUnnormalizedCoordinates(bUnnormalizedCoordinates);
}
#endif

/////////////////////////////////

void Lua::Vulkan::VKFramebuffer::IsValid(lua_State *l,Framebuffer &hFramebuffer)
{
	Lua::PushBool(l,true);
}
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
void Lua::Vulkan::VKFramebuffer::GetSize(lua_State *l,Framebuffer &hFramebuffer)
{
	Lua::PushInt(l,hFramebuffer.GetAttachmentCount());
	uint32_t width;
	uint32_t height;
	uint32_t depth;
	hFramebuffer.GetSize(width,height,depth);
	Lua::PushInt(l,width);
	Lua::PushInt(l,height);
	Lua::PushInt(l,depth);
}

/////////////////////////////////

void Lua::Vulkan::VKRenderPass::IsValid(lua_State *l,RenderPass &hRenderPass)
{
	Lua::PushBool(l,true);
}
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

void Lua::Vulkan::VKEvent::IsValid(lua_State *l,Event &hEvent)
{
	Lua::PushBool(l,true);
}
void Lua::Vulkan::VKEvent::GetStatus(lua_State *l,Event &hEvent)
{
	auto b = hEvent.IsSet();
	auto r = b ? prosper::Result::EventSet : prosper::Result::EventReset;
	Lua::PushInt(l,umath::to_integral(r));
}
void Lua::Vulkan::VKEvent::IsSet(lua_State *l,Event &hEvent)
{
	Lua::PushBool(l,hEvent.IsSet());
}

/////////////////////////////////

void Lua::Vulkan::VKFence::IsValid(lua_State *l,Fence &hFence)
{
	Lua::PushBool(l,true);
}

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

void Lua::Vulkan::VKCommandBuffer::IsValid(lua_State *l,CommandBuffer &hCommandBuffer)
{
	Lua::PushBool(l,true);
}
void Lua::Vulkan::VKCommandBuffer::RecordClearImage(lua_State *l,CommandBuffer &hCommandBuffer,Image &img,const Color &col,const prosper::util::ClearImageInfo &clearImageInfo)
{
	auto vcol = col.ToVector4();
	Lua::PushBool(l,hCommandBuffer.RecordClearImage(img,prosper::ImageLayout::TransferDstOptimal,{vcol.r,vcol.g,vcol.b,vcol.a},clearImageInfo));
}
void Lua::Vulkan::VKCommandBuffer::RecordClearImage(lua_State *l,CommandBuffer &hCommandBuffer,Image &img,float clearDepth,const prosper::util::ClearImageInfo &clearImageInfo)
{
	Lua::PushBool(l,hCommandBuffer.RecordClearImage(img,prosper::ImageLayout::TransferDstOptimal,clearDepth,clearImageInfo));
}
void Lua::Vulkan::VKCommandBuffer::RecordClearAttachment(lua_State *l,CommandBuffer &hCommandBuffer,Image &img,const Color &col,uint32_t attId)
{
	auto vcol = col.ToVector4();
	Lua::PushBool(l,hCommandBuffer.RecordClearAttachment(img,{vcol.r,vcol.g,vcol.b,vcol.a},attId));
}
void Lua::Vulkan::VKCommandBuffer::RecordClearAttachment(lua_State *l,CommandBuffer &hCommandBuffer,Image &img,float clearDepth)
{
	Lua::PushBool(l,hCommandBuffer.RecordClearAttachment(img,clearDepth));
}
void Lua::Vulkan::VKCommandBuffer::RecordCopyImage(lua_State *l,CommandBuffer &hCommandBuffer,Image &imgSrc,Image &imgDst,const prosper::util::CopyInfo &copyInfo)
{
	Lua::PushBool(l,hCommandBuffer.RecordCopyImage(copyInfo,imgSrc,imgDst));
}
void Lua::Vulkan::VKCommandBuffer::RecordCopyBufferToImage(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &bufSrc,Image &imgDst,const prosper::util::BufferImageCopyInfo &copyInfo)
{
	Lua::PushBool(l,hCommandBuffer.RecordCopyBufferToImage(copyInfo,bufSrc,imgDst));
}
void Lua::Vulkan::VKCommandBuffer::RecordCopyBuffer(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &bufSrc,Buffer &bufDst,const prosper::util::BufferCopy &copyInfo)
{
	Lua::PushBool(l,hCommandBuffer.RecordCopyBuffer(copyInfo,bufSrc,bufDst));
}
void Lua::Vulkan::VKCommandBuffer::RecordUpdateBuffer(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &buf,uint32_t offset,::DataStream &ds)
{
	Lua::PushBool(l,hCommandBuffer.RecordUpdateBuffer(buf,offset,ds->GetSize(),ds->GetData()));
}
void Lua::Vulkan::VKCommandBuffer::RecordBlitImage(lua_State *l,CommandBuffer &hCommandBuffer,Image &imgSrc,Image &imgDst,const prosper::util::BlitInfo &blitInfo)
{
	Lua::PushBool(l,hCommandBuffer.RecordBlitImage(blitInfo,imgSrc,imgDst));
}
void Lua::Vulkan::VKCommandBuffer::RecordResolveImage(lua_State *l,CommandBuffer &hCommandBuffer,Image &imgSrc,Image &imgDst)
{
	Lua::PushBool(l,hCommandBuffer.RecordResolveImage(imgSrc,imgDst));
}
void Lua::Vulkan::VKCommandBuffer::RecordBlitTexture(lua_State *l,CommandBuffer &hCommandBuffer,Texture &texSrc,Image &imgDst)
{
	Lua::PushBool(l,hCommandBuffer.RecordBlitTexture(texSrc,imgDst));
}
void Lua::Vulkan::VKCommandBuffer::RecordGenerateMipmaps(lua_State *l,CommandBuffer &hCommandBuffer,Image &img,uint32_t currentLayout,uint32_t srcAccessMask,uint32_t srcStage)
{
	Lua::PushBool(l,hCommandBuffer.RecordGenerateMipmaps(img,static_cast<prosper::ImageLayout>(currentLayout),static_cast<prosper::AccessFlags>(srcAccessMask),static_cast<prosper::PipelineStageFlags>(srcStage)));
}
void Lua::Vulkan::VKCommandBuffer::RecordPipelineBarrier(lua_State *l,CommandBuffer &hCommandBuffer,const prosper::util::PipelineBarrierInfo &barrierInfo)
{
	Lua::PushBool(l,hCommandBuffer.RecordPipelineBarrier(barrierInfo));
}
void Lua::Vulkan::VKCommandBuffer::RecordImageBarrier(
	lua_State *l,CommandBuffer &hCommandBuffer,Image &img,
	uint32_t oldLayout,uint32_t newLayout,const prosper::util::ImageSubresourceRange &subresourceRange
)
{
	Lua::PushBool(l,hCommandBuffer.RecordImageBarrier(
		img,
		static_cast<prosper::ImageLayout>(oldLayout),static_cast<prosper::ImageLayout>(newLayout),
		subresourceRange
	));
}
void Lua::Vulkan::VKCommandBuffer::RecordImageBarrier(
	lua_State *l,CommandBuffer &hCommandBuffer,Image &img,
	uint32_t srcStageMask,uint32_t dstStageMask,
	uint32_t oldLayout,uint32_t newLayout,
	uint32_t srcAccessMask,uint32_t dstAccessMask,
	uint32_t baseLayer
)
{
	Lua::PushBool(l,hCommandBuffer.RecordImageBarrier(
		img,
		static_cast<prosper::PipelineStageFlags>(srcStageMask),static_cast<prosper::PipelineStageFlags>(dstStageMask),
		static_cast<prosper::ImageLayout>(oldLayout),static_cast<prosper::ImageLayout>(newLayout),
		static_cast<prosper::AccessFlags>(srcAccessMask),static_cast<prosper::AccessFlags>(dstAccessMask),
		baseLayer
	));
}
void Lua::Vulkan::VKCommandBuffer::RecordBufferBarrier(
	lua_State *l,CommandBuffer &hCommandBuffer,Buffer &buf,
	uint32_t srcStageMask,uint32_t dstStageMask,
	uint32_t srcAccessMask,uint32_t dstAccessMask,
	uint32_t offset,uint32_t size
)
{
	auto lsize = (size != std::numeric_limits<uint32_t>::max()) ? static_cast<uint64_t>(size) : std::numeric_limits<uint64_t>::max();
	Lua::PushBool(l,hCommandBuffer.RecordBufferBarrier(
		buf,
		static_cast<prosper::PipelineStageFlags>(srcStageMask),static_cast<prosper::PipelineStageFlags>(dstStageMask),
		static_cast<prosper::AccessFlags>(srcAccessMask),static_cast<prosper::AccessFlags>(dstAccessMask),
		offset,lsize
	));
}
void Lua::Vulkan::VKCommandBuffer::RecordSetViewport(lua_State *l,CommandBuffer &hCommandBuffer,uint32_t width,uint32_t height,uint32_t x,uint32_t y)
{
	Lua::PushBool(l,hCommandBuffer.RecordSetViewport(width,height,x,y));
}
void Lua::Vulkan::VKCommandBuffer::RecordSetScissor(lua_State *l,CommandBuffer &hCommandBuffer,uint32_t width,uint32_t height,uint32_t x,uint32_t y)
{
	Lua::PushBool(l,hCommandBuffer.RecordSetScissor(width,height,x,y));
}
void Lua::Vulkan::VKCommandBuffer::RecordBeginRenderPass(lua_State *l,CommandBuffer &hCommandBuffer,Lua::Vulkan::RenderPassInfo &rpInfo)
{
	if(hCommandBuffer.IsPrimary() == false)
	{
		Lua::PushBool(l,false);
		return;
	}
	static_assert(sizeof(Lua::Vulkan::ClearValue) == sizeof(prosper::ClearValue));
	auto &primaryCmdBuffer = dynamic_cast<prosper::IPrimaryCommandBuffer&>(hCommandBuffer);
	if(rpInfo.layerId.has_value())
	{
		auto r = primaryCmdBuffer.RecordBeginRenderPass(
			*rpInfo.renderTarget,
			*rpInfo.layerId,
			reinterpret_cast<std::vector<prosper::ClearValue>&>(rpInfo.clearValues),
			rpInfo.renderPass.get()
		);
		Lua::PushBool(l,r);
		return;
	}
	auto r = primaryCmdBuffer.RecordBeginRenderPass(
		*rpInfo.renderTarget,
		reinterpret_cast<std::vector<prosper::ClearValue>&>(rpInfo.clearValues),
		rpInfo.renderPass.get()
	);
	Lua::PushBool(l,r);
}
void Lua::Vulkan::VKCommandBuffer::RecordEndRenderPass(lua_State *l,CommandBuffer &hCommandBuffer)
{
	if(hCommandBuffer.IsPrimary() == false)
	{
		Lua::PushBool(l,false);
		return;
	}
	auto &primaryCmdBuffer = dynamic_cast<prosper::IPrimaryCommandBuffer&>(hCommandBuffer);;
	Lua::PushBool(l,primaryCmdBuffer.RecordEndRenderPass());
}
void Lua::Vulkan::VKCommandBuffer::RecordBindIndexBuffer(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &indexBuffer,uint32_t indexType,uint32_t offset)
{
	Lua::PushBool(l,hCommandBuffer.RecordBindIndexBuffer(indexBuffer,static_cast<prosper::IndexType>(indexType),offset));
}
void Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffer(lua_State *l,CommandBuffer &hCommandBuffer,prosper::ShaderGraphics &graphics,Buffer &vertexBuffer,uint32_t startBinding,uint32_t offset)
{
	Lua::PushBool(l,hCommandBuffer.RecordBindVertexBuffers(graphics,{&vertexBuffer},startBinding,{static_cast<uint64_t>(offset)}));
}
void Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffers(lua_State *l,CommandBuffer &hCommandBuffer,prosper::ShaderGraphics &graphics,luabind::object vertexBuffers,uint32_t startBinding,luabind::object offsets)
{
	auto buffers = Lua::get_table_values<prosper::IBuffer*>(l,2,[](lua_State *l,int32_t idx) {
		return &Lua::Check<Buffer>(l,idx);
	});
	static std::vector<uint64_t> voffsets;
	voffsets.clear();
	if(Lua::IsSet(l,4))
	{
		voffsets = Lua::get_table_values<uint64_t>(l,4,[](lua_State *l,int32_t idx) {
			return static_cast<uint64_t>(Lua::CheckInt(l,idx));
		});
	}
	else
		voffsets.resize(buffers.size(),0ull);
	Lua::PushBool(l,hCommandBuffer.RecordBindVertexBuffers(graphics,buffers,startBinding,voffsets));
}
void Lua::Vulkan::VKCommandBuffer::RecordCopyImageToBuffer(lua_State *l,CommandBuffer &hCommandBuffer,Image &imgSrc,uint32_t srcImageLayout,Buffer &bufDst,const prosper::util::BufferImageCopyInfo &copyInfo)
{
	Lua::PushBool(l,hCommandBuffer.RecordCopyImageToBuffer(copyInfo,imgSrc,static_cast<prosper::ImageLayout>(srcImageLayout),bufDst));
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
void Lua::Vulkan::VKCommandBuffer::RecordDraw(lua_State *l,CommandBuffer &hCommandBuffer,uint32_t vertexCount,uint32_t instanceCount,uint32_t firstVertex,uint32_t firstInstance)
{
	Lua::PushBool(l,hCommandBuffer.RecordDraw(vertexCount,instanceCount,firstVertex,firstInstance));
}
void Lua::Vulkan::VKCommandBuffer::RecordDrawIndexed(lua_State *l,CommandBuffer &hCommandBuffer,uint32_t indexCount,uint32_t instanceCount,uint32_t firstIndex,uint32_t firstInstance)
{
	Lua::PushBool(l,hCommandBuffer.RecordDrawIndexed(indexCount,instanceCount,firstIndex,firstInstance));
}
void Lua::Vulkan::VKCommandBuffer::RecordDrawIndexedIndirect(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &buffer,uint32_t offset,uint32_t drawCount,uint32_t stride)
{
	Lua::PushBool(l,hCommandBuffer.RecordDrawIndexedIndirect(buffer,offset,drawCount,stride));
}
void Lua::Vulkan::VKCommandBuffer::RecordDrawIndirect(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &buffer,uint32_t offset,uint32_t drawCount,uint32_t stride)
{
	Lua::PushBool(l,hCommandBuffer.RecordDrawIndirect(buffer,offset,drawCount,stride));
}
void Lua::Vulkan::VKCommandBuffer::RecordFillBuffer(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &buffer,uint32_t offset,uint32_t size,uint32_t data)
{
	Lua::PushBool(l,hCommandBuffer.RecordFillBuffer(buffer,offset,size,data));
}
void Lua::Vulkan::VKCommandBuffer::RecordSetBlendConstants(lua_State *l,CommandBuffer &hCommandBuffer,const Vector4 &blendConstants)
{
	Lua::PushBool(l,hCommandBuffer.RecordSetBlendConstants(std::array<float,4>{blendConstants[0],blendConstants[1],blendConstants[2],blendConstants[3]}));
}
void Lua::Vulkan::VKCommandBuffer::RecordSetDepthBias(lua_State *l,CommandBuffer &hCommandBuffer,float depthBiasConstantFactor,float depthBiasClamp,float slopeScaledDepthBias)
{
	Lua::PushBool(l,hCommandBuffer.RecordSetDepthBias(depthBiasConstantFactor,depthBiasClamp,slopeScaledDepthBias));
}
void Lua::Vulkan::VKCommandBuffer::RecordSetDepthBounds(lua_State *l,CommandBuffer &hCommandBuffer,float minDepthBounds,float maxDepthBounds)
{
	Lua::PushBool(l,hCommandBuffer.RecordSetDepthBounds(minDepthBounds,maxDepthBounds));
}
void Lua::Vulkan::VKCommandBuffer::RecordSetLineWidth(lua_State *l,CommandBuffer &hCommandBuffer,float lineWidth)
{
	Lua::PushBool(l,hCommandBuffer.RecordSetLineWidth(lineWidth));
}
void Lua::Vulkan::VKCommandBuffer::RecordSetStencilCompareMask(lua_State *l,CommandBuffer &hCommandBuffer,uint32_t faceMask,uint32_t stencilCompareMask)
{
	Lua::PushBool(l,hCommandBuffer.RecordSetStencilCompareMask(static_cast<prosper::StencilFaceFlags>(faceMask),stencilCompareMask));
}
void Lua::Vulkan::VKCommandBuffer::RecordSetStencilReference(lua_State *l,CommandBuffer &hCommandBuffer,uint32_t faceMask,uint32_t stencilReference)
{
	Lua::PushBool(l,hCommandBuffer.RecordSetStencilReference(static_cast<prosper::StencilFaceFlags>(faceMask),stencilReference));
}
void Lua::Vulkan::VKCommandBuffer::RecordSetStencilWriteMask(lua_State *l,CommandBuffer &hCommandBuffer,uint32_t faceMask,uint32_t stencilWriteMask)
{
	Lua::PushBool(l,hCommandBuffer.RecordSetStencilWriteMask(static_cast<prosper::StencilFaceFlags>(faceMask),stencilWriteMask));
}
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
void Lua::Vulkan::VKCommandBuffer::RecordDrawGradient(lua_State *l,CommandBuffer &hCommandBuffer,RenderTarget &rt,const Vector2 &dir,luabind::object lnodes)
{
	if(hCommandBuffer.IsPrimary() == false)
	{
		Lua::PushBool(l,false);
		return;
	}
	auto nodes = get_gradient_nodes(l,4);
	auto primCmd = std::dynamic_pointer_cast<prosper::IPrimaryCommandBuffer>(hCommandBuffer.shared_from_this());
	Lua::PushBool(l,pragma::util::record_draw_gradient(c_engine->GetRenderContext(),primCmd,rt,dir,nodes));
}
void Lua::Vulkan::VKCommandBuffer::StopRecording(lua_State *l,CommandBuffer &hCommandBuffer)
{
	Lua::PushBool(l,hCommandBuffer.StopRecording());
}
void Lua::Vulkan::VKCommandBuffer::StartRecording(lua_State *l,CommandBuffer &hCommandBuffer,bool oneTimeSubmit,bool simultaneousUseAllowed)
{
	if(hCommandBuffer.IsPrimary() == false)
	{
		Lua::PushBool(l,false);
		return;
	}
	Lua::PushBool(l,dynamic_cast<prosper::IPrimaryCommandBuffer&>(hCommandBuffer).StartRecording(oneTimeSubmit,simultaneousUseAllowed));
}

/////////////////////////////////

void Lua::Vulkan::VKBuffer::IsValid(lua_State *l,Buffer &hBuffer)
{
	Lua::PushBool(l,true);
}
void Lua::Vulkan::VKBuffer::GetStartOffset(lua_State *l,Buffer &hBuffer)
{
	Lua::PushInt(l,hBuffer.GetStartOffset());
}
void Lua::Vulkan::VKBuffer::SetPermanentlyMapped(lua_State *l,Buffer &hBuffer,bool b)
{
	hBuffer.SetPermanentlyMapped(b);
}
void Lua::Vulkan::VKBuffer::GetBaseIndex(lua_State *l,Buffer &hBuffer)
{
	Lua::PushInt(l,hBuffer.GetBaseIndex());
}
void Lua::Vulkan::VKBuffer::GetSize(lua_State *l,Buffer &hBuffer)
{
	Lua::PushInt(l,hBuffer.GetSize());
}
void Lua::Vulkan::VKBuffer::GetUsageFlags(lua_State *l,Buffer &hBuffer)
{
	Lua::PushInt(l,umath::to_integral(hBuffer.GetUsageFlags()));
}
void Lua::Vulkan::VKBuffer::GetParent(lua_State *l,Buffer &hBuffer)
{
	auto parent = hBuffer.GetParent();
	if(parent == nullptr)
		return;
	Lua::Push(l,parent);
}
void Lua::Vulkan::VKBuffer::Write(lua_State *l,Buffer &hBuffer,uint32_t offset,::DataStream &ds,uint32_t dsOffset,uint32_t dsSize)
{
	Lua::PushBool(l,hBuffer.Write(offset,dsSize,ds->GetData() +dsOffset));
}
void Lua::Vulkan::VKBuffer::Read(lua_State *l,Buffer &hBuffer,uint32_t offset,uint32_t size)
{
	auto ds = ::DataStream(size);
	auto r = hBuffer.Read(offset,size,ds->GetData());
	if(r == false)
		return;
	Lua::Push(l,ds);
}
void Lua::Vulkan::VKBuffer::Read(lua_State *l,Buffer &hBuffer,uint32_t offset,uint32_t size,::DataStream &ds,uint32_t dsOffset)
{
	auto reqSize = size +dsOffset;
	if(ds->GetSize() < reqSize)
		ds->Resize(reqSize);
	Lua::PushBool(l,hBuffer.Read(offset,size,ds->GetData() +dsOffset));
}
void Lua::Vulkan::VKBuffer::Map(lua_State *l,Buffer &hBuffer,uint32_t offset,uint32_t size)
{
	Lua::PushBool(l,hBuffer.Map(offset,size));
}
void Lua::Vulkan::VKBuffer::Unmap(lua_State *l,Buffer &hBuffer)
{
	Lua::PushBool(l,hBuffer.Unmap());
}

/////////////////////////////////

void Lua::Vulkan::VKDescriptorSet::IsValid(lua_State *l,DescriptorSet &hDescSet)
{
	Lua::PushBool(l,true);
}
void Lua::Vulkan::VKDescriptorSet::GetBindingCount(lua_State *l,DescriptorSet &hDescSet)
{
	Lua::PushInt(l,hDescSet.GetBindingCount());
}
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
void Lua::Vulkan::VKDescriptorSet::SetBindingTexture(lua_State *l,Lua::Vulkan::DescriptorSet &hDescSet,uint32_t bindingIdx,Lua::Vulkan::Texture &texture)
{
	Lua::PushBool(l,hDescSet.GetDescriptorSet()->SetBindingTexture(texture,bindingIdx));
}
void Lua::Vulkan::VKDescriptorSet::SetBindingTexture(lua_State *l,Lua::Vulkan::DescriptorSet &hDescSet,uint32_t bindingIdx,Lua::Vulkan::Texture &texture,uint32_t layerId)
{
	Lua::PushBool(l,hDescSet.GetDescriptorSet()->SetBindingTexture(texture,bindingIdx,layerId));
}
void Lua::Vulkan::VKDescriptorSet::SetBindingArrayTexture(lua_State *l,DescriptorSet &hDescSet,uint32_t bindingIdx,Lua::Vulkan::Texture &texture,uint32_t arrayIdx,uint32_t layerId)
{
	Lua::PushBool(l,hDescSet.GetDescriptorSet()->SetBindingArrayTexture(texture,bindingIdx,arrayIdx,layerId));
}
void Lua::Vulkan::VKDescriptorSet::SetBindingArrayTexture(lua_State *l,DescriptorSet &hDescSet,uint32_t bindingIdx,Lua::Vulkan::Texture &texture,uint32_t arrayIdx)
{
	Lua::PushBool(l,hDescSet.GetDescriptorSet()->SetBindingArrayTexture(texture,bindingIdx,arrayIdx));
}
void Lua::Vulkan::VKDescriptorSet::SetBindingStorageBuffer(lua_State *l,DescriptorSet &hDescSet,uint32_t bindingIdx,Lua::Vulkan::Buffer &buffer,uint32_t startOffset,uint32_t size)
{
	Lua::PushBool(l,hDescSet.GetDescriptorSet()->SetBindingStorageBuffer(buffer,bindingIdx,startOffset,(size != std::numeric_limits<uint32_t>::max()) ? static_cast<uint64_t>(size) : std::numeric_limits<uint64_t>::max()));
}
void Lua::Vulkan::VKDescriptorSet::SetBindingUniformBuffer(lua_State *l,DescriptorSet &hDescSet,uint32_t bindingIdx,Lua::Vulkan::Buffer &buffer,uint32_t startOffset,uint32_t size)
{
	Lua::PushBool(l,hDescSet.GetDescriptorSet()->SetBindingUniformBuffer(buffer,bindingIdx,startOffset,(size != std::numeric_limits<uint32_t>::max()) ? static_cast<uint64_t>(size) : std::numeric_limits<uint64_t>::max()));
}
void Lua::Vulkan::VKDescriptorSet::SetBindingUniformBufferDynamic(lua_State *l,DescriptorSet &hDescSet,uint32_t bindingIdx,Lua::Vulkan::Buffer &buffer,uint32_t startOffset,uint32_t size)
{
	Lua::PushBool(l,hDescSet.GetDescriptorSet()->SetBindingDynamicUniformBuffer(buffer,bindingIdx,startOffset,(size != std::numeric_limits<uint32_t>::max()) ? static_cast<uint64_t>(size) : std::numeric_limits<uint64_t>::max()));
}

/////////////////////////////////

void Lua::Vulkan::VKMesh::GetVertexBuffer(lua_State *l,pragma::VkMesh &mesh)
{
	auto &vertexBuffer = mesh.GetVertexBuffer();
	if(vertexBuffer != nullptr)
		Lua::Push<std::shared_ptr<Buffer>>(l,vertexBuffer);
}
void Lua::Vulkan::VKMesh::GetVertexWeightBuffer(lua_State *l,pragma::VkMesh &mesh)
{
	auto &vertexWeightBuffer = mesh.GetVertexBuffer();
	if(vertexWeightBuffer != nullptr)
		Lua::Push<std::shared_ptr<Buffer>>(l,vertexWeightBuffer);
}
void Lua::Vulkan::VKMesh::GetAlphaBuffer(lua_State *l,pragma::VkMesh &mesh)
{
	auto &alphaBuffer = mesh.GetAlphaBuffer();
	if(alphaBuffer != nullptr)
		Lua::Push<std::shared_ptr<Buffer>>(l,alphaBuffer);
}
void Lua::Vulkan::VKMesh::GetIndexBuffer(lua_State *l,pragma::VkMesh &mesh)
{
	auto &indexBuffer = mesh.GetIndexBuffer();
	if(indexBuffer != nullptr)
		Lua::Push<std::shared_ptr<Buffer>>(l,indexBuffer);
}
void Lua::Vulkan::VKMesh::SetVertexBuffer(lua_State*,pragma::VkMesh &mesh,Buffer &hBuffer)
{
	mesh.SetVertexBuffer(hBuffer.shared_from_this());
}
void Lua::Vulkan::VKMesh::SetVertexWeightBuffer(lua_State*,pragma::VkMesh &mesh,Buffer &hBuffer)
{
	mesh.SetVertexWeightBuffer(hBuffer.shared_from_this());
}
void Lua::Vulkan::VKMesh::SetAlphaBuffer(lua_State*,pragma::VkMesh &mesh,Buffer &hBuffer)
{
	mesh.SetAlphaBuffer(hBuffer.shared_from_this());
}
void Lua::Vulkan::VKMesh::SetIndexBuffer(lua_State*,pragma::VkMesh &mesh,Buffer &hBuffer)
{
	mesh.SetIndexBuffer(hBuffer.shared_from_this());
}

/////////////////////////////////

void Lua::Vulkan::VKRenderTarget::IsValid(lua_State *l,RenderTarget &hRt)
{
	Lua::PushBool(l,true);
}
void Lua::Vulkan::VKRenderTarget::GetWidth(lua_State *l,RenderTarget &hRt)
{
	auto extents = hRt.GetTexture().GetImage().GetExtents();
	Lua::PushInt(l,extents.width);
}
void Lua::Vulkan::VKRenderTarget::GetHeight(lua_State *l,RenderTarget &hRt)
{
	auto extents = hRt.GetTexture().GetImage().GetExtents();
	Lua::PushInt(l,extents.height);
}
void Lua::Vulkan::VKRenderTarget::GetFormat(lua_State *l,RenderTarget &hRt)
{
	Lua::PushInt(l,umath::to_integral(hRt.GetTexture().GetImage().GetFormat()));
}
void Lua::Vulkan::VKRenderTarget::GetTexture(lua_State *l,RenderTarget &hRt,uint32_t idx)
{
	auto *tex = hRt.GetTexture(idx);
	if(tex == nullptr)
		return;
	Lua::Push<std::shared_ptr<Texture>>(l,tex->shared_from_this());
}
void Lua::Vulkan::VKRenderTarget::GetRenderPass(lua_State *l,RenderTarget &hRt)
{
	Lua::Push<std::shared_ptr<RenderPass>>(l,hRt.GetRenderPass().shared_from_this());
}
void Lua::Vulkan::VKRenderTarget::GetFramebuffer(lua_State *l,RenderTarget &hRt)
{
	Lua::Push<std::shared_ptr<Framebuffer>>(l,hRt.GetFramebuffer().shared_from_this());
}

/////////////////////////////////

void Lua::Vulkan::VKTimestampQuery::IsValid(lua_State *l,TimestampQuery &hTimestampQuery)
{
	Lua::PushBool(l,true);
}
void Lua::Vulkan::VKTimestampQuery::IsResultAvailable(lua_State *l,TimestampQuery &hTimestampQuery)
{
	Lua::PushBool(l,hTimestampQuery.IsResultAvailable());
}
void Lua::Vulkan::VKTimestampQuery::GetPipelineStage(lua_State *l,TimestampQuery &hTimestampQuery)
{
	Lua::PushInt(l,umath::to_integral(hTimestampQuery.GetPipelineStage()));
}
void Lua::Vulkan::VKTimestampQuery::Write(lua_State *l,TimestampQuery &hTimestampQuery,CommandBuffer &cmdBuffer)
{
	hTimestampQuery.Write(cmdBuffer);
}
void Lua::Vulkan::VKTimestampQuery::QueryResult(lua_State *l,TimestampQuery &hTimestampQuery)
{
	std::chrono::nanoseconds r;
	if(hTimestampQuery.QueryResult(r) == false)
		Lua::PushBool(l,false);
	else
		Lua::PushInt(l,r.count());
}

/////////////////////////////////

void Lua::Vulkan::VKTimerQuery::IsValid(lua_State *l,TimerQuery &hTimerQuery)
{
	Lua::PushBool(l,true);
}
void Lua::Vulkan::VKTimerQuery::Begin(lua_State*,TimerQuery &hTimerQuery,CommandBuffer &cmdBuffer)
{
	hTimerQuery.Begin(cmdBuffer);
}
void Lua::Vulkan::VKTimerQuery::End(lua_State*,TimerQuery &hTimerQuery,CommandBuffer &cmdBuffer)
{
	hTimerQuery.End(cmdBuffer);
}
void Lua::Vulkan::VKTimerQuery::IsResultAvailable(lua_State *l,TimerQuery &hTimerQuery)
{
	Lua::PushBool(l,hTimerQuery.IsResultAvailable());
}
void Lua::Vulkan::VKTimerQuery::QueryResult(lua_State *l,TimerQuery &hTimerQuery)
{
	std::chrono::nanoseconds r;
	if(hTimerQuery.QueryResult(r) == false)
		Lua::PushBool(l,false);
	else
		Lua::PushInt(l,r.count());
}
#pragma optimize("",on)
