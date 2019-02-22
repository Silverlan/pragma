#include "stdafx_client.h"
#include "pragma/lua/libraries/c_lua_vulkan.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/shaders/image/c_shader_gradient.hpp"
#include "pragma/rendering/shaders/c_shader_lua.hpp"
#include <pragma/lua/classes/ldef_color.h>
#include <pragma/lua/classes/ldef_vector.h>
#include <pragma/lua/classes/lerrorcode.h>
#include <pragma/lua/classes/ldatastream.h>
#include <luasystem.h>
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
#include <prosper_descriptor_set_group.hpp>
#include <prosper_util_square_shape.hpp>
#include <wrappers/event.h>
#include <wrappers/memory_block.h>
#include <misc/fence_create_info.h>
#include <misc/event_create_info.h>
#include <misc/framebuffer_create_info.h>
#include <misc/image_create_info.h>
#include <misc/image_view_create_info.h>
#include "pragma/model/vk_mesh.h"

extern DLLCENGINE CEngine *c_engine;
namespace Lua
{
	namespace Vulkan
	{
		DLLCLIENT int create_buffer(lua_State *l);
		DLLCLIENT int create_texture(lua_State *l);
		DLLCLIENT int create_descriptor_set(lua_State *l);
		DLLCLIENT int create_image(lua_State *l);
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
		namespace VKContextObject
		{
			template<class T>
				void SetDebugName(lua_State *l,T &o,const std::string &name,T*(*fCheck)(lua_State*,int))
			{
				fCheck(l,1);
				o->SetDebugName(name);
			}
			template<class T>
				void GetDebugName(lua_State *l,T &o,T*(*fCheck)(lua_State*,int))
			{
				fCheck(l,1);
				Lua::PushString(l,o->GetDebugName());
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
			DLLCLIENT void GetExtent3D(lua_State *l,Image &hImg,uint32_t mipmap=0u);
			DLLCLIENT void GetFormat(lua_State *l,Image &hImg);
			DLLCLIENT void GetMemoryTypes(lua_State *l,Image &hImg);
			DLLCLIENT void GetMipmapSize(lua_State *l,Image &hImg,uint32_t mipmap=0u);
			DLLCLIENT void GetLayerCount(lua_State *l,Image &hImg);
			DLLCLIENT void GetMipmapCount(lua_State *l,Image &hImg);
			DLLCLIENT void GetSampleCount(lua_State *l,Image &hImg);
			DLLCLIENT void GetSharingMode(lua_State *l,Image &hImg);
			DLLCLIENT void GetStorageSize(lua_State *l,Image &hImg);
			DLLCLIENT void GetTiling(lua_State *l,Image &hImg);
			DLLCLIENT void GetType(lua_State *l,Image &hImg);
			DLLCLIENT void GetUsage(lua_State *l,Image &hImg);
			DLLCLIENT void GetMemoryBlock(lua_State *l,Image &hImg);
			DLLCLIENT void GetMemoryRequirements(lua_State *l,Image &hImg);
			DLLCLIENT void GetParentSwapchain(lua_State *l,Image &hImg);
			DLLCLIENT void GetSubresourceRange(lua_State *l,Image &hImg);
			DLLCLIENT void GetWidth(lua_State *l,Image &hImg);
			DLLCLIENT void GetHeight(lua_State *l,Image &hImg);
		};
		namespace VKImageView
		{
			DLLCLIENT void IsValid(lua_State *l,ImageView &hImgView);
			DLLCLIENT void GetAspectMask(lua_State *l,ImageView &hImgView);
			DLLCLIENT void GetBaseLayer(lua_State *l,ImageView &hImgView);
			DLLCLIENT void GetBaseMipmapLevel(lua_State *l,ImageView &hImgView);
			DLLCLIENT void GetBaseMipmapSize(lua_State *l,ImageView &hImgView);
			DLLCLIENT void GetImageFormat(lua_State *l,ImageView &hImgView);
			DLLCLIENT void GetLayerCount(lua_State *l,ImageView &hImgView);
			DLLCLIENT void GetMipmapCount(lua_State *l,ImageView &hImgView);
			DLLCLIENT void GetParentImage(lua_State *l,ImageView &hImgView);
			DLLCLIENT void GetSubresourceRange(lua_State *l,ImageView &hImgView);
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
			DLLCLIENT void GetUseUnnormalizedCoordinates(lua_State *l,Sampler &hSampler);
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
			DLLCLIENT void SetUseUnnormalizedCoordinates(lua_State *l,Sampler &hSampler,bool bUnnormalizedCoordinates);
		};
		namespace VKFramebuffer
		{
			DLLCLIENT void IsValid(lua_State *l,Framebuffer &hFramebuffer);
			DLLCLIENT void GetAttachment(lua_State *l,Framebuffer &hFramebuffer,uint32_t attId);
			DLLCLIENT void GetAttachmentCount(lua_State *l,Framebuffer &hFramebuffer);
			DLLCLIENT void GetSize(lua_State *l,Framebuffer &hFramebuffer);
		};
		namespace VKRenderPass
		{
			DLLCLIENT void IsValid(lua_State *l,RenderPass &hRenderPass);
			DLLCLIENT void GetInfo(lua_State *l,RenderPass &hRenderPass);
			DLLCLIENT void GetSwapchain(lua_State *l,RenderPass &hRenderPass);
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
		namespace VKCommandBuffer
		{
			DLLCLIENT void IsValid(lua_State *l,CommandBuffer &hCommandBuffer);
			DLLCLIENT void RecordClearImage(lua_State *l,CommandBuffer &hCommandBuffer,Image &img,const Color &col,const prosper::util::ClearImageInfo &clearImageInfo={});
			DLLCLIENT void RecordClearImage(lua_State *l,CommandBuffer &hCommandBuffer,Image &img,float clearDepth,const prosper::util::ClearImageInfo &clearImageInfo={});
			DLLCLIENT void RecordClearAttachment(lua_State *l,CommandBuffer &hCommandBuffer,Image &img,const Color &col,uint32_t attId=0u);
			DLLCLIENT void RecordClearAttachment(lua_State *l,CommandBuffer &hCommandBuffer,Image &img,float clearDepth);
			DLLCLIENT void RecordCopyImage(lua_State *l,CommandBuffer &hCommandBuffer,Image &imgSrc,Image &imgDst,const prosper::util::CopyInfo &copyInfo);
			DLLCLIENT void RecordCopyBufferToImage(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &bufSrc,Image &imgDst,const prosper::util::BufferImageCopyInfo &copyInfo);
			DLLCLIENT void RecordCopyBuffer(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &bufSrc,Buffer &bufDst,const Anvil::BufferCopy &copyInfo);
			DLLCLIENT void RecordUpdateBuffer(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &buf,uint32_t offset,::DataStream &ds,uint32_t dstStageMask,uint32_t dstAccessMask);
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
			DLLCLIENT void RecordBindVertexBuffer(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &vertexBuffer,uint32_t startBinding,uint32_t offset);
			DLLCLIENT void RecordBindVertexBuffers(lua_State *l,CommandBuffer &hCommandBuffer,luabind::object vertexBuffers,uint32_t startBinding,luabind::object offsets);
			DLLCLIENT void RecordCopyImageToBuffer(lua_State *l,CommandBuffer &hCommandBuffer,Image &imgSrc,uint32_t srcImageLayout,Buffer &bufDst,const prosper::util::BufferImageCopyInfo &copyInfo);
			DLLCLIENT void RecordDispatch(lua_State *l,CommandBuffer &hCommandBuffer,uint32_t x,uint32_t y,uint32_t z);
			DLLCLIENT void RecordDispatchIndirect(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &buffer,uint32_t offset);
			DLLCLIENT void RecordDraw(lua_State *l,CommandBuffer &hCommandBuffer,uint32_t vertexCount,uint32_t instanceCount,uint32_t firstVertex,uint32_t firstInstance);
			DLLCLIENT void RecordDrawIndexed(lua_State *l,CommandBuffer &hCommandBuffer,uint32_t indexCount,uint32_t instanceCount,uint32_t firstIndex,uint32_t vertexOffset,uint32_t firstInstance);
			DLLCLIENT void RecordDrawIndexedIndirect(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &buffer,uint32_t offset,uint32_t drawCount,uint32_t stride);
			DLLCLIENT void RecordDrawIndirect(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &buffer,uint32_t offset,uint32_t drawCount,uint32_t stride);
			DLLCLIENT void RecordFillBuffer(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &buffer,uint32_t offset,uint32_t size,uint32_t data);
			DLLCLIENT void RecordResetEvent(lua_State *l,CommandBuffer &hCommandBuffer,Event &ev,uint32_t stageMask);
			DLLCLIENT void RecordSetBlendConstants(lua_State *l,CommandBuffer &hCommandBuffer,const Vector4 &blendConstants);
			DLLCLIENT void RecordSetDepthBias(lua_State *l,CommandBuffer &hCommandBuffer,float depthBiasConstantFactor,float depthBiasClamp,float slopeScaledDepthBias);
			DLLCLIENT void RecordSetDepthBounds(lua_State *l,CommandBuffer &hCommandBuffer,float minDepthBounds,float maxDepthBounds);
			DLLCLIENT void RecordSetEvent(lua_State *l,CommandBuffer &hCommandBuffer,Event &ev,uint32_t stageMask);
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
			DLLCLIENT void GetBindingInfo(lua_State *l,DescriptorSet &hDescSet);
			DLLCLIENT void GetBindingInfo(lua_State *l,DescriptorSet &hDescSet,uint32_t bindingIdx);
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
			DLLCLIENT void GetVertexBuffer(lua_State *l,std::shared_ptr<pragma::VkMesh> &mesh);
			DLLCLIENT void GetVertexWeightBuffer(lua_State *l,std::shared_ptr<pragma::VkMesh> &mesh);
			DLLCLIENT void GetAlphaBuffer(lua_State *l,std::shared_ptr<pragma::VkMesh> &mesh);
			DLLCLIENT void GetIndexBuffer(lua_State *l,std::shared_ptr<pragma::VkMesh> &mesh);
			DLLCLIENT void SetVertexBuffer(lua_State *l,std::shared_ptr<pragma::VkMesh> &mesh,Buffer &hImg);
			DLLCLIENT void SetVertexWeightBuffer(lua_State *l,std::shared_ptr<pragma::VkMesh> &mesh,Buffer &hImg);
			DLLCLIENT void SetAlphaBuffer(lua_State *l,std::shared_ptr<pragma::VkMesh> &mesh,Buffer &hImg);
			DLLCLIENT void SetIndexBuffer(lua_State *l,std::shared_ptr<pragma::VkMesh> &mesh,Buffer &hImg);
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
			DLLCLIENT void Begin(lua_State *l,TimerQuery &hTimerQuery);
			DLLCLIENT void End(lua_State *l,TimerQuery &hTimerQuery);
			DLLCLIENT void IsResultAvailable(lua_State *l,TimerQuery &hTimerQuery);
			DLLCLIENT void QueryResult(lua_State *l,TimerQuery &hTimerQuery);
		};
	};
};

DLLCLIENT std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::Image &hImg)
{
	out<<"VKImage[";
	if(hImg == nullptr)
		out<<"NULL";
	else
	{
		auto *img = hImg.get();
		auto extents = img->GetExtents();
		out<<extents.width<<"x"<<extents.height;
		out<<"][";
		auto format = img->GetFormat();
		out<<prosper::util::to_string(format);
	}
	out<<"]";
	return out;
}

DLLCLIENT std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::ImageView &hImgView)
{
	out<<"VKImageView[";
	if(hImgView == nullptr)
		out<<"NULL";
	else
	{
		//auto *imgView = hImgView.get();

	}
	out<<"]";
	return out;
}

DLLCLIENT std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::Sampler &hSampler)
{
	out<<"VKSampler[";
	if(hSampler == nullptr)
		out<<"NULL";
	else
	{
		//auto *sampler = hSampler.get();

	}
	out<<"]";
	return out;
}

DLLCLIENT std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::Texture &hTex)
{
	out<<"VKTexture[";
	if(hTex == nullptr)
		out<<"NULL";
	else
	{
		auto *tex = hTex.get();
		auto &img = tex->GetImage();
		auto &imgView = tex->GetImageView();
		auto &sampler = tex->GetSampler();
		if(img == nullptr)
			out<<"NULL][";
		else
			out<<img<<"][";
		if(imgView == nullptr)
			out<<"NULL][";
		else
			out<<imgView<<"][";
		if(sampler == nullptr)
			out<<"NULL";
		else
			out<<sampler;
	}
	out<<"]";
	return out;
}

DLLCLIENT std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::Framebuffer &hFramebuffer)
{
	out<<"VKFramebuffer[";
	if(hFramebuffer == nullptr)
		out<<"NULL";
	else
	{
		//auto *framebuffer = hFramebuffer.get();

	}
	out<<"]";
	return out;
}

DLLCLIENT std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::RenderPass &hRenderPass)
{
	out<<"VKRenderPass[";
	if(hRenderPass == nullptr)
		out<<"NULL";
	else
	{
		//auto *renderPass = hRenderPass.get();
		
	}
	out<<"]";
	return out;
}

DLLCLIENT std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::Event &hEvent)
{
	out<<"VKEvent[";
	if(hEvent == nullptr)
		out<<"NULL";
	else
	{
		auto *ev = hEvent.get();
		auto r = vk::Result::eEventReset;
		if(ev->is_set())
			r = vk::Result::eEventSet;
		out<<prosper::util::to_string(r);
	}
	out<<"]";
	return out;
}

DLLCLIENT std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::Fence &hFence)
{
	out<<"VKFence[";
	if(hFence == nullptr)
		out<<"NULL";
	else
	{
		//auto *fence = hFence.get();

	}
	out<<"]";
	return out;
}

DLLCLIENT std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::Semaphore &hSemaphore)
{
	out<<"VKSemaphore[";
	if(hSemaphore == nullptr)
		out<<"NULL";
	else
	{
		//auto *semaphore = hSemaphore.get();

	}
	out<<"]";
	return out;
}

DLLCLIENT std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::Memory &hMemory)
{
	out<<"VKMemory[";
	if(&hMemory == nullptr)
		out<<"NULL";
	else
	{
		//auto *memory = hMemory.get();

	}
	out<<"]";
	return out;
}

DLLCLIENT std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::CommandBuffer &hCommandBuffer)
{
	out<<"VKCommandBuffer[";
	if(hCommandBuffer == nullptr)
		out<<"NULL";
	else
	{
		//auto *commandBuffer = hCommandBuffer.get();

	}
	out<<"]";
	return out;
}

DLLCLIENT std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::Buffer &hBuffer)
{
	out<<"VKBuffer[";
	if(hBuffer == nullptr)
		out<<"NULL";
	else
	{
		auto *buffer = hBuffer.get();
		out<<buffer->GetSize();
	}
	out<<"]";
	return out;
}

DLLCLIENT std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::DescriptorSet &hDescSet)
{
	out<<"VKDescriptorSet[";
	if(hDescSet == nullptr)
		out<<"NULL";
	else
	{
		auto *descSet = hDescSet.get();
		//out<<descSet->GetLayoutCount(); // prosper TODO
	}
	out<<"]";
	return out;
}

DLLCLIENT std::ostream &operator<<(std::ostream &out,const std::shared_ptr<pragma::VkMesh>&)
{
	out<<"VKMesh";
	return out;
}

DLLCLIENT std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::RenderTarget &hRt)
{
	out<<"VKRenderTarget[";
	if(hRt == nullptr)
		out<<"NULL";
	else
	{
		auto *rt = hRt.get();
		auto &tex = rt->GetTexture();
		auto &framebuffer = rt->GetFramebuffer();
		auto &renderPass = rt->GetRenderPass();

		if(tex == nullptr)
			out<<"NULL][";
		else
			out<<tex<<"][";
		if(framebuffer == nullptr)
			out<<"NULL][";
		else
			out<<framebuffer<<"][";
		if(renderPass == nullptr)
			out<<"NULL][";
		else
			out<<renderPass<<"][";
	}
	out<<"]";
	return out;
}

DLLCLIENT std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::TimestampQuery &hTimestampQuery)
{
	out<<"VKTimestampQuery[";
	if(hTimestampQuery == nullptr)
		out<<"NULL";
	else
	{
		auto *query = hTimestampQuery.get();
		out<<query->IsResultAvailable();
	}
	out<<"]";
	return out;
}

DLLCLIENT std::ostream &operator<<(std::ostream &out,const Lua::Vulkan::TimerQuery &hTimerQuery)
{
	out<<"VKTimerQuery[";
	if(hTimerQuery == nullptr)
		out<<"NULL";
	else
	{
		auto *query = hTimerQuery.get();
		out<<query->IsResultAvailable();
	}
	out<<"]";
	return out;
}

int Lua::Vulkan::create_buffer(lua_State *l)
{
	auto arg = 1;
	auto &bufCreateInfo = *Lua::CheckVKBufferCreateInfo(l,arg++);
	Buffer buf = nullptr;
	if(Lua::IsSet(l,arg) == false)
		buf = prosper::util::create_buffer(c_engine->GetDevice(),bufCreateInfo);
	else
	{
		auto &ds = *Lua::CheckDataStream(l,arg++);
		buf = prosper::util::create_buffer(c_engine->GetDevice(),bufCreateInfo,ds->GetData());
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

		auto type = Anvil::DescriptorType::UNIFORM_BUFFER;
		get_table_value<ptrdiff_t,decltype(type)>(l,"type",tBinding,type,Lua::CheckInt);

		auto shaderStages = vk::ShaderStageFlagBits::eAllGraphics;
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
	auto dsg = prosper::util::create_descriptor_set_group(c_engine->GetDevice(),shaderDescSetInfo);
	if(dsg == nullptr)
		return 0;
	dsg->SetDebugName("lua_dsg");
	Lua::Push(l,dsg);
	return 1;
}

int Lua::Vulkan::create_image(lua_State *l)
{
	auto arg = 1;
	auto &imgCreateInfo = *Lua::CheckVKImageCreateInfo(l,arg++);
	Image img = nullptr;
	if(Lua::IsSet(l,arg) == false)
		img = prosper::util::create_image(c_engine->GetDevice(),imgCreateInfo);
	else
	{
		auto &ds = *Lua::CheckDataStream(l,arg++);
		img = prosper::util::create_image(c_engine->GetDevice(),imgCreateInfo,ds->GetData());
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
	auto &img = *Lua::CheckVKImage(l,arg++);
	auto &texCreateInfo = *Lua::CheckVKTextureCreateInfo(l,arg++);
	prosper::util::ImageViewCreateInfo *imgViewCreateInfo = nullptr;
	prosper::util::SamplerCreateInfo *samplerCreateInfo = nullptr;
	if(Lua::IsSet(l,arg))
	{
		imgViewCreateInfo = Lua::CheckVKImageViewCreateInfo(l,arg++);
		if(Lua::IsSet(l,arg))
			samplerCreateInfo = Lua::CheckVKSamplerCreateInfo(l,arg++);
	}
	auto tex = prosper::util::create_texture(c_engine->GetDevice(),texCreateInfo,img,imgViewCreateInfo,samplerCreateInfo);
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
	std::vector<Anvil::ImageView*> attachments;
	auto tAttachments = arg++;
	Lua::CheckTable(l,tAttachments);
	auto numAttachments = Lua::GetObjectLength(l,tAttachments);
	attachments.reserve(numAttachments);
	for(auto i=decltype(numAttachments){0};i<numAttachments;++i)
	{
		Lua::PushInt(l,i +1); /* 1 */
		Lua::GetTableValue(l,tAttachments);

		attachments.push_back(&(*Lua::CheckVKImageView(l,-1))->GetAnvilImageView());

		Lua::Pop(l,1); /* 0 */
	}
	auto layers = 1u;
	if(Lua::IsSet(l,arg))
		layers = Lua::CheckInt(l,arg++);
	auto fb = prosper::util::create_framebuffer(c_engine->GetDevice(),width,height,layers,attachments);
	if(fb == nullptr)
		return 0;
	fb->SetDebugName("lua_fb");
	Lua::Push(l,fb);
	return 1;
}
int Lua::Vulkan::create_render_pass(lua_State *l)
{
	auto &rpCreateInfo = *Lua::CheckVKRenderPassCreateInfo(l,1);
	auto rp = prosper::util::create_render_pass(c_engine->GetDevice(),rpCreateInfo);
	if(rp == nullptr)
		return 0;
	rp->SetDebugName("lua_rp");
	Lua::Push(l,rp);
	return 1;
}

int Lua::Vulkan::create_render_target(lua_State *l)
{
	auto arg = 1;
	auto &rtCreateInfo = *Lua::CheckVKRenderTargetCreateInfo(l,arg++);
	std::vector<Texture> textures;
	if(Lua::IsVKTexture(l,arg))
		textures.push_back(*Lua::CheckVKTexture(l,arg++));
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

			textures.push_back(*Lua::CheckVKTexture(l,-1));

			Lua::Pop(l,1); /* 0 */
		}
	}
	Lua::Vulkan::RenderPass rp = nullptr;
	if(Lua::IsSet(l,arg))
		rp = *Lua::CheckVKRenderPass(l,arg++);
	auto rt = prosper::util::create_render_target(c_engine->GetDevice(),textures,rp,rtCreateInfo);
	if(rt == nullptr)
		return 0;
	rt->SetDebugName("lua_rt");
	Lua::Push(l,rt);
	return 1;
}

int Lua::Vulkan::create_event(lua_State *l)
{
	auto ev = prosper::util::unique_ptr_to_shared_ptr(Anvil::Event::create(Anvil::EventCreateInfo::create(&c_engine->GetDevice())));
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
	auto fence = prosper::util::unique_ptr_to_shared_ptr(Anvil::Fence::create(Anvil::FenceCreateInfo::create(&c_engine->GetDevice(),bCreateSignalled)));
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
	Lua::PushString(l,::prosper::util::to_string(static_cast<vk::Result>(result)));
	return 1;
}
int Lua::Vulkan::format_to_string(lua_State *l)
{
	auto format = Lua::CheckInt(l,1);
	Lua::PushString(l,::prosper::util::to_string(static_cast<Anvil::Format>(format)));
	return 1;
}
int Lua::Vulkan::shader_stage_to_string(lua_State *l)
{
	auto shaderStage = Lua::CheckInt(l,1);
	Lua::PushString(l,::prosper::util::to_string(static_cast<Anvil::ShaderStageFlagBits>(shaderStage)));
	return 1;
}
int Lua::Vulkan::is_depth_format(lua_State *l)
{
	auto format = Lua::CheckInt(l,1);
	Lua::PushBool(l,::prosper::util::is_depth_format(static_cast<Anvil::Format>(format)));
	return 1;
}
int Lua::Vulkan::is_compressed_format(lua_State *l)
{
	auto format = Lua::CheckInt(l,1);
	Lua::PushBool(l,::prosper::util::is_compressed_format(static_cast<Anvil::Format>(format)));
	return 1;
}
int Lua::Vulkan::is_uncompressed_format(lua_State *l)
{
	auto format = Lua::CheckInt(l,1);
	Lua::PushBool(l,::prosper::util::is_uncompressed_format(static_cast<Anvil::Format>(format)));
	return 1;
}
int Lua::Vulkan::get_bit_size(lua_State *l)
{
	auto format = Lua::CheckInt(l,1);
	Lua::PushInt(l,::prosper::util::get_bit_size(static_cast<Anvil::Format>(format)));
	return 1;
}
int Lua::Vulkan::get_byte_size(lua_State *l)
{
	auto format = Lua::CheckInt(l,1);
	Lua::PushInt(l,::prosper::util::get_byte_size(static_cast<Anvil::Format>(format)));
	return 1;
}
int Lua::Vulkan::get_swapchain_image_count(lua_State *l)
{
	Lua::PushInt(l,c_engine->GetSwapchainImageCount());
	return 1;
}
int Lua::Vulkan::wait_idle(lua_State *l)
{
	c_engine->WaitIdle();
	return 0;
}
int Lua::Vulkan::get_square_vertex_uv_buffer(lua_State *l)
{
	auto uvBuffer = prosper::util::get_square_vertex_uv_buffer(c_engine->GetDevice());
	Lua::Push(l,uvBuffer);
	return 1;
}
int Lua::Vulkan::get_square_vertex_buffer(lua_State *l)
{
	auto vertexBuffer = prosper::util::get_square_vertex_buffer(c_engine->GetDevice());
	Lua::Push(l,vertexBuffer);
	return 1;
}
int Lua::Vulkan::get_square_uv_buffer(lua_State *l)
{
	auto uvBuffer = prosper::util::get_square_uv_buffer(c_engine->GetDevice());
	Lua::Push(l,uvBuffer);
	return 1;
}
int Lua::Vulkan::get_square_vertices(lua_State *l)
{
	auto &squareVerts = prosper::util::get_square_vertices();
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
	auto &squareUvs = prosper::util::get_square_uv_coordinates();
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
	auto numVerts = prosper::util::get_square_vertex_count();
	Lua::PushInt(l,numVerts);
	return 1;
}
int Lua::Vulkan::get_square_vertex_format(lua_State *l)
{
	auto format = prosper::util::get_square_vertex_format();
	Lua::PushInt(l,umath::to_integral(format));
	return 1;
}
int Lua::Vulkan::get_square_uv_format(lua_State *l)
{
	auto uvFormat = prosper::util::get_square_uv_format();
	Lua::PushInt(l,umath::to_integral(uvFormat));
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
	createInfo.format = static_cast<Anvil::Format>(format);
	createInfo.usage = Anvil::ImageUsageFlagBits::SAMPLED_BIT | Anvil::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT;
	createInfo.postCreateLayout = Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL;
	createInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
	auto &dev = c_engine->GetDevice();
	auto img = prosper::util::create_image(dev,createInfo);
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	auto texture = prosper::util::create_texture(dev,{},img,&imgViewCreateInfo,&samplerCreateInfo);
	auto rt = prosper::util::create_render_target(dev,{texture},static_cast<prosper::ShaderGraphics&>(*whShader.get()).GetRenderPass());
	rt->SetDebugName("lua_gradient");
	auto cb = FunctionCallback<void,std::reference_wrapper<std::shared_ptr<prosper::PrimaryCommandBuffer>>>::Create(nullptr);
	static_cast<Callback<void,std::reference_wrapper<std::shared_ptr<prosper::PrimaryCommandBuffer>>>*>(cb.get())->SetFunction([cb,rt,dir,nodes](std::reference_wrapper<std::shared_ptr<prosper::PrimaryCommandBuffer>> drawCmd) mutable {
		c_engine->KeepResourceAliveUntilPresentationComplete(rt);
		pragma::util::record_draw_gradient(
			*c_engine,drawCmd.get(),*rt,dir,nodes
		);
		if(cb.IsValid())
			cb.Remove();
	});
	c_engine->AddCallback("DrawFrame",cb);
	Lua::Push(l,texture);
	return 1;
}

void ClientState::RegisterVulkanLuaInterface(Lua::Interface &lua)
{
	auto &vulkanMod = lua.RegisterLibrary("vulkan",{
		{"create_descriptor_set",Lua::Vulkan::create_descriptor_set},
		{"create_buffer",Lua::Vulkan::create_buffer},
		{"create_image",Lua::Vulkan::create_image},
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
			luabind::def("get_square_uv_format",&Lua::Vulkan::get_square_uv_format)
		]
	];

	Lua::RegisterLibraryEnums(lua.GetState(),"vulkan",{
		{"FORMAT_UNDEFINED",umath::to_integral(vk::Format::eUndefined)},
		{"FORMAT_R4G4_UNORM_PACK8",umath::to_integral(vk::Format::eR4G4UnormPack8)},
		{"FORMAT_R4G4B4A4_UNORM_PACK16",umath::to_integral(vk::Format::eR4G4B4A4UnormPack16)},
		{"FORMAT_B4G4R4A4_UNORM_PACK16",umath::to_integral(vk::Format::eB4G4R4A4UnormPack16)},
		{"FORMAT_R5G6B5_UNORM_PACK16",umath::to_integral(vk::Format::eR5G6B5UnormPack16)},
		{"FORMAT_B5G6R5_UNORM_PACK16",umath::to_integral(vk::Format::eB5G6R5UnormPack16)},
		{"FORMAT_R5G5B5A1_UNORM_PACK16",umath::to_integral(vk::Format::eR5G5B5A1UnormPack16)},
		{"FORMAT_B5G5R5A1_UNORM_PACK16",umath::to_integral(vk::Format::eB5G5R5A1UnormPack16)},
		{"FORMAT_A1R5G5B5_UNORM_PACK16",umath::to_integral(vk::Format::eA1R5G5B5UnormPack16)},
		{"FORMAT_R8_UNORM",umath::to_integral(vk::Format::eR8Unorm)},
		{"FORMAT_R8_SNORM",umath::to_integral(vk::Format::eR8Snorm)},
		{"FORMAT_R8_USCALED",umath::to_integral(vk::Format::eR8Uscaled)},
		{"FORMAT_R8_SSCALED",umath::to_integral(vk::Format::eR8Sscaled)},
		{"FORMAT_R8_UINT",umath::to_integral(vk::Format::eR8Uint)},
		{"FORMAT_R8_SINT",umath::to_integral(vk::Format::eR8Sint)},
		{"FORMAT_R8_SRGB",umath::to_integral(vk::Format::eR8Srgb)},
		{"FORMAT_R8G8_UNORM",umath::to_integral(vk::Format::eR8G8Unorm)},
		{"FORMAT_R8G8_SNORM",umath::to_integral(vk::Format::eR8G8Snorm)},
		{"FORMAT_R8G8_USCALED",umath::to_integral(vk::Format::eR8G8Uscaled)},
		{"FORMAT_R8G8_SSCALED",umath::to_integral(vk::Format::eR8G8Sscaled)},
		{"FORMAT_R8G8_UINT",umath::to_integral(vk::Format::eR8G8Uint)},
		{"FORMAT_R8G8_SINT",umath::to_integral(vk::Format::eR8G8Sint)},
		{"FORMAT_R8G8_SRGB",umath::to_integral(vk::Format::eR8G8Srgb)},
		{"FORMAT_R8G8B8_UNORM",umath::to_integral(vk::Format::eR8G8B8Unorm)},
		{"FORMAT_R8G8B8_SNORM",umath::to_integral(vk::Format::eR8G8B8Snorm)},
		{"FORMAT_R8G8B8_USCALED",umath::to_integral(vk::Format::eR8G8B8Uscaled)},
		{"FORMAT_R8G8B8_SSCALED",umath::to_integral(vk::Format::eR8G8B8Sscaled)},
		{"FORMAT_R8G8B8_UINT",umath::to_integral(vk::Format::eR8G8B8Uint)},
		{"FORMAT_R8G8B8_SINT",umath::to_integral(vk::Format::eR8G8B8Sint)},
		{"FORMAT_R8G8B8_SRGB",umath::to_integral(vk::Format::eR8G8B8Srgb)},
		{"FORMAT_B8G8R8_UNORM",umath::to_integral(vk::Format::eB8G8R8Unorm)},
		{"FORMAT_B8G8R8_SNORM",umath::to_integral(vk::Format::eB8G8R8Snorm)},
		{"FORMAT_B8G8R8_USCALED",umath::to_integral(vk::Format::eB8G8R8Uscaled)},
		{"FORMAT_B8G8R8_SSCALED",umath::to_integral(vk::Format::eB8G8R8Sscaled)},
		{"FORMAT_B8G8R8_UINT",umath::to_integral(vk::Format::eB8G8R8Uint)},
		{"FORMAT_B8G8R8_SINT",umath::to_integral(vk::Format::eB8G8R8Sint)},
		{"FORMAT_B8G8R8_SRGB",umath::to_integral(vk::Format::eB8G8R8Srgb)},
		{"FORMAT_R8G8B8A8_UNORM",umath::to_integral(vk::Format::eR8G8B8A8Unorm)},
		{"FORMAT_R8G8B8A8_SNORM",umath::to_integral(vk::Format::eR8G8B8A8Snorm)},
		{"FORMAT_R8G8B8A8_USCALED",umath::to_integral(vk::Format::eR8G8B8A8Uscaled)},
		{"FORMAT_R8G8B8A8_SSCALED",umath::to_integral(vk::Format::eR8G8B8A8Sscaled)},
		{"FORMAT_R8G8B8A8_UINT",umath::to_integral(vk::Format::eR8G8B8A8Uint)},
		{"FORMAT_R8G8B8A8_SINT",umath::to_integral(vk::Format::eR8G8B8A8Sint)},
		{"FORMAT_R8G8B8A8_SRGB",umath::to_integral(vk::Format::eR8G8B8A8Srgb)},
		{"FORMAT_B8G8R8A8_UNORM",umath::to_integral(vk::Format::eB8G8R8A8Unorm)},
		{"FORMAT_B8G8R8A8_SNORM",umath::to_integral(vk::Format::eB8G8R8A8Snorm)},
		{"FORMAT_B8G8R8A8_USCALED",umath::to_integral(vk::Format::eB8G8R8A8Uscaled)},
		{"FORMAT_B8G8R8A8_SSCALED",umath::to_integral(vk::Format::eB8G8R8A8Sscaled)},
		{"FORMAT_B8G8R8A8_UINT",umath::to_integral(vk::Format::eB8G8R8A8Uint)},
		{"FORMAT_B8G8R8A8_SINT",umath::to_integral(vk::Format::eB8G8R8A8Sint)},
		{"FORMAT_B8G8R8A8_SRGB",umath::to_integral(vk::Format::eB8G8R8A8Srgb)},
		{"FORMAT_A8B8G8R8_UNORM_PACK32",umath::to_integral(vk::Format::eA8B8G8R8UnormPack32)},
		{"FORMAT_A8B8G8R8_SNORM_PACK32",umath::to_integral(vk::Format::eA8B8G8R8SnormPack32)},
		{"FORMAT_A8B8G8R8_USCALED_PACK32",umath::to_integral(vk::Format::eA8B8G8R8UscaledPack32)},
		{"FORMAT_A8B8G8R8_SSCALED_PACK32",umath::to_integral(vk::Format::eA8B8G8R8SscaledPack32)},
		{"FORMAT_A8B8G8R8_UINT_PACK32",umath::to_integral(vk::Format::eA8B8G8R8UintPack32)},
		{"FORMAT_A8B8G8R8_SINT_PACK32",umath::to_integral(vk::Format::eA8B8G8R8SintPack32)},
		{"FORMAT_A8B8G8R8_SRGB_PACK32",umath::to_integral(vk::Format::eA8B8G8R8SrgbPack32)},
		{"FORMAT_A2R10G10B10_UNORM_PACK32",umath::to_integral(vk::Format::eA2R10G10B10UnormPack32)},
		{"FORMAT_A2R10G10B10_SNORM_PACK32",umath::to_integral(vk::Format::eA2R10G10B10SnormPack32)},
		{"FORMAT_A2R10G10B10_USCALED_PACK32",umath::to_integral(vk::Format::eA2R10G10B10UscaledPack32)},
		{"FORMAT_A2R10G10B10_SSCALED_PACK32",umath::to_integral(vk::Format::eA2R10G10B10SscaledPack32)},
		{"FORMAT_A2R10G10B10_UINT_PACK32",umath::to_integral(vk::Format::eA2R10G10B10UintPack32)},
		{"FORMAT_A2R10G10B10_SINT_PACK32",umath::to_integral(vk::Format::eA2R10G10B10SintPack32)},
		{"FORMAT_A2B10G10R10_UNORM_PACK32",umath::to_integral(vk::Format::eA2B10G10R10UnormPack32)},
		{"FORMAT_A2B10G10R10_SNORM_PACK32",umath::to_integral(vk::Format::eA2B10G10R10SnormPack32)},
		{"FORMAT_A2B10G10R10_USCALED_PACK32",umath::to_integral(vk::Format::eA2B10G10R10UscaledPack32)},
		{"FORMAT_A2B10G10R10_SSCALED_PACK32",umath::to_integral(vk::Format::eA2B10G10R10SscaledPack32)},
		{"FORMAT_A2B10G10R10_UINT_PACK32",umath::to_integral(vk::Format::eA2B10G10R10UintPack32)},
		{"FORMAT_A2B10G10R10_SINT_PACK32",umath::to_integral(vk::Format::eA2B10G10R10SintPack32)},
		{"FORMAT_R16_UNORM",umath::to_integral(vk::Format::eR16Unorm)},
		{"FORMAT_R16_SNORM",umath::to_integral(vk::Format::eR16Snorm)},
		{"FORMAT_R16_USCALED",umath::to_integral(vk::Format::eR16Uscaled)},
		{"FORMAT_R16_SSCALED",umath::to_integral(vk::Format::eR16Sscaled)},
		{"FORMAT_R16_UINT",umath::to_integral(vk::Format::eR16Uint)},
		{"FORMAT_R16_SINT",umath::to_integral(vk::Format::eR16Sint)},
		{"FORMAT_R16_SFLOAT",umath::to_integral(vk::Format::eR16Sfloat)},
		{"FORMAT_R16G16_UNORM",umath::to_integral(vk::Format::eR16G16Unorm)},
		{"FORMAT_R16G16_SNORM",umath::to_integral(vk::Format::eR16G16Snorm)},
		{"FORMAT_R16G16_USCALED",umath::to_integral(vk::Format::eR16G16Uscaled)},
		{"FORMAT_R16G16_SSCALED",umath::to_integral(vk::Format::eR16G16Sscaled)},
		{"FORMAT_R16G16_UINT",umath::to_integral(vk::Format::eR16G16Uint)},
		{"FORMAT_R16G16_SINT",umath::to_integral(vk::Format::eR16G16Sint)},
		{"FORMAT_R16G16_SFLOAT",umath::to_integral(vk::Format::eR16G16Sfloat)},
		{"FORMAT_R16G16B16_UNORM",umath::to_integral(vk::Format::eR16G16B16Unorm)},
		{"FORMAT_R16G16B16_SNORM",umath::to_integral(vk::Format::eR16G16B16Snorm)},
		{"FORMAT_R16G16B16_USCALED",umath::to_integral(vk::Format::eR16G16B16Uscaled)},
		{"FORMAT_R16G16B16_SSCALED",umath::to_integral(vk::Format::eR16G16B16Sscaled)},
		{"FORMAT_R16G16B16_UINT",umath::to_integral(vk::Format::eR16G16B16Uint)},
		{"FORMAT_R16G16B16_SINT",umath::to_integral(vk::Format::eR16G16B16Sint)},
		{"FORMAT_R16G16B16_SFLOAT",umath::to_integral(vk::Format::eR16G16B16Sfloat)},
		{"FORMAT_R16G16B16A16_UNORM",umath::to_integral(vk::Format::eR16G16B16A16Unorm)},
		{"FORMAT_R16G16B16A16_SNORM",umath::to_integral(vk::Format::eR16G16B16A16Snorm)},
		{"FORMAT_R16G16B16A16_USCALED",umath::to_integral(vk::Format::eR16G16B16A16Uscaled)},
		{"FORMAT_R16G16B16A16_SSCALED",umath::to_integral(vk::Format::eR16G16B16A16Sscaled)},
		{"FORMAT_R16G16B16A16_UINT",umath::to_integral(vk::Format::eR16G16B16A16Uint)},
		{"FORMAT_R16G16B16A16_SINT",umath::to_integral(vk::Format::eR16G16B16A16Sint)},
		{"FORMAT_R16G16B16A16_SFLOAT",umath::to_integral(vk::Format::eR16G16B16A16Sfloat)},
		{"FORMAT_R32_UINT",umath::to_integral(vk::Format::eR32Uint)},
		{"FORMAT_R32_SINT",umath::to_integral(vk::Format::eR32Sint)},
		{"FORMAT_R32_SFLOAT",umath::to_integral(vk::Format::eR32Sfloat)},
		{"FORMAT_R32G32_UINT",umath::to_integral(vk::Format::eR32G32Uint)},
		{"FORMAT_R32G32_SINT",umath::to_integral(vk::Format::eR32G32Sint)},
		{"FORMAT_R32G32_SFLOAT",umath::to_integral(vk::Format::eR32G32Sfloat)},
		{"FORMAT_R32G32B32_UINT",umath::to_integral(vk::Format::eR32G32B32Uint)},
		{"FORMAT_R32G32B32_SINT",umath::to_integral(vk::Format::eR32G32B32Sint)},
		{"FORMAT_R32G32B32_SFLOAT",umath::to_integral(vk::Format::eR32G32B32Sfloat)},
		{"FORMAT_R32G32B32A32_UINT",umath::to_integral(vk::Format::eR32G32B32A32Uint)},
		{"FORMAT_R32G32B32A32_SINT",umath::to_integral(vk::Format::eR32G32B32A32Sint)},
		{"FORMAT_R32G32B32A32_SFLOAT",umath::to_integral(vk::Format::eR32G32B32A32Sfloat)},
		{"FORMAT_R64_UINT",umath::to_integral(vk::Format::eR64Uint)},
		{"FORMAT_R64_SINT",umath::to_integral(vk::Format::eR64Sint)},
		{"FORMAT_R64_SFLOAT",umath::to_integral(vk::Format::eR64Sfloat)},
		{"FORMAT_R64G64_UINT",umath::to_integral(vk::Format::eR64G64Uint)},
		{"FORMAT_R64G64_SINT",umath::to_integral(vk::Format::eR64G64Sint)},
		{"FORMAT_R64G64_SFLOAT",umath::to_integral(vk::Format::eR64G64Sfloat)},
		{"FORMAT_R64G64B64_UINT",umath::to_integral(vk::Format::eR64G64B64Uint)},
		{"FORMAT_R64G64B64_SINT",umath::to_integral(vk::Format::eR64G64B64Sint)},
		{"FORMAT_R64G64B64_SFLOAT",umath::to_integral(vk::Format::eR64G64B64Sfloat)},
		{"FORMAT_R64G64B64A64_UINT",umath::to_integral(vk::Format::eR64G64B64A64Uint)},
		{"FORMAT_R64G64B64A64_SINT",umath::to_integral(vk::Format::eR64G64B64A64Sint)},
		{"FORMAT_R64G64B64A64_SFLOAT",umath::to_integral(vk::Format::eR64G64B64A64Sfloat)},
		{"FORMAT_B10G11R11_UFLOAT_PACK32",umath::to_integral(vk::Format::eB10G11R11UfloatPack32)},
		{"FORMAT_E5B9G9R9_UFLOAT_PACK32",umath::to_integral(vk::Format::eE5B9G9R9UfloatPack32)},
		{"FORMAT_D16_UNORM",umath::to_integral(vk::Format::eD16Unorm)},
		{"FORMAT_X8_D24_UNORM_PACK32",umath::to_integral(vk::Format::eX8D24UnormPack32)},
		{"FORMAT_D32_SFLOAT",umath::to_integral(vk::Format::eD32Sfloat)},
		{"FORMAT_S8_UINT",umath::to_integral(vk::Format::eS8Uint)},
		{"FORMAT_D16_UNORM_S8_UINT",umath::to_integral(vk::Format::eD16UnormS8Uint)},
		{"FORMAT_D24_UNORM_S8_UINT",umath::to_integral(vk::Format::eD24UnormS8Uint)},
		{"FORMAT_D32_SFLOAT_S8_UINT",umath::to_integral(vk::Format::eD32SfloatS8Uint)},
		{"FORMAT_BC1_RGB_UNORM_BLOCK",umath::to_integral(vk::Format::eBc1RgbUnormBlock)},
		{"FORMAT_BC1_RGB_SRGB_BLOCK",umath::to_integral(vk::Format::eBc1RgbSrgbBlock)},
		{"FORMAT_BC1_RGBA_UNORM_BLOCK",umath::to_integral(vk::Format::eBc1RgbaUnormBlock)},
		{"FORMAT_BC1_RGBA_SRGB_BLOCK",umath::to_integral(vk::Format::eBc1RgbaSrgbBlock)},
		{"FORMAT_BC2_UNORM_BLOCK",umath::to_integral(vk::Format::eBc2UnormBlock)},
		{"FORMAT_BC2_SRGB_BLOCK",umath::to_integral(vk::Format::eBc2SrgbBlock)},
		{"FORMAT_BC3_UNORM_BLOCK",umath::to_integral(vk::Format::eBc3UnormBlock)},
		{"FORMAT_BC3_SRGB_BLOCK",umath::to_integral(vk::Format::eBc3SrgbBlock)},
		{"FORMAT_BC4_UNORM_BLOCK",umath::to_integral(vk::Format::eBc4UnormBlock)},
		{"FORMAT_BC4_SNORM_BLOCK",umath::to_integral(vk::Format::eBc4SnormBlock)},
		{"FORMAT_BC5_UNORM_BLOCK",umath::to_integral(vk::Format::eBc5UnormBlock)},
		{"FORMAT_BC5_SNORM_BLOCK",umath::to_integral(vk::Format::eBc5SnormBlock)},
		{"FORMAT_BC6H_UFLOAT_BLOCK",umath::to_integral(vk::Format::eBc6HUfloatBlock)},
		{"FORMAT_BC6H_SFLOAT_BLOCK",umath::to_integral(vk::Format::eBc6HSfloatBlock)},
		{"FORMAT_BC7_UNORM_BLOCK",umath::to_integral(vk::Format::eBc7UnormBlock)},
		{"FORMAT_BC7_SRGB_BLOCK",umath::to_integral(vk::Format::eBc7SrgbBlock)},
		{"FORMAT_ETC2_R8G8B8_UNORM_BLOCK",umath::to_integral(vk::Format::eEtc2R8G8B8UnormBlock)},
		{"FORMAT_ETC2_R8G8B8_SRGB_BLOCK",umath::to_integral(vk::Format::eEtc2R8G8B8SrgbBlock)},
		{"FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK",umath::to_integral(vk::Format::eEtc2R8G8B8A1UnormBlock)},
		{"FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK",umath::to_integral(vk::Format::eEtc2R8G8B8A1SrgbBlock)},
		{"FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK",umath::to_integral(vk::Format::eEtc2R8G8B8A8UnormBlock)},
		{"FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK",umath::to_integral(vk::Format::eEtc2R8G8B8A8SrgbBlock)},
		{"FORMAT_EAC_R11_UNORM_BLOCK",umath::to_integral(vk::Format::eEacR11UnormBlock)},
		{"FORMAT_EAC_R11_SNORM_BLOCK",umath::to_integral(vk::Format::eEacR11SnormBlock)},
		{"FORMAT_EAC_R11G11_UNORM_BLOCK",umath::to_integral(vk::Format::eEacR11G11UnormBlock)},
		{"FORMAT_EAC_R11G11_SNORM_BLOCK",umath::to_integral(vk::Format::eEacR11G11SnormBlock)},
		{"FORMAT_ASTC_4x4_UNORM_BLOCK",umath::to_integral(vk::Format::eAstc4x4UnormBlock)},
		{"FORMAT_ASTC_4x4_SRGB_BLOCK",umath::to_integral(vk::Format::eAstc4x4SrgbBlock)},
		{"FORMAT_ASTC_5x4_UNORM_BLOCK",umath::to_integral(vk::Format::eAstc5x4UnormBlock)},
		{"FORMAT_ASTC_5x4_SRGB_BLOCK",umath::to_integral(vk::Format::eAstc5x4SrgbBlock)},
		{"FORMAT_ASTC_5x5_UNORM_BLOCK",umath::to_integral(vk::Format::eAstc5x5UnormBlock)},
		{"FORMAT_ASTC_5x5_SRGB_BLOCK",umath::to_integral(vk::Format::eAstc5x5SrgbBlock)},
		{"FORMAT_ASTC_6x5_UNORM_BLOCK",umath::to_integral(vk::Format::eAstc6x5UnormBlock)},
		{"FORMAT_ASTC_6x5_SRGB_BLOCK",umath::to_integral(vk::Format::eAstc6x5SrgbBlock)},
		{"FORMAT_ASTC_6x6_UNORM_BLOCK",umath::to_integral(vk::Format::eAstc6x6UnormBlock)},
		{"FORMAT_ASTC_6x6_SRGB_BLOCK",umath::to_integral(vk::Format::eAstc6x6SrgbBlock)},
		{"FORMAT_ASTC_8x5_UNORM_BLOCK",umath::to_integral(vk::Format::eAstc8x5UnormBlock)},
		{"FORMAT_ASTC_8x5_SRGB_BLOCK",umath::to_integral(vk::Format::eAstc8x5SrgbBlock)},
		{"FORMAT_ASTC_8x6_UNORM_BLOCK",umath::to_integral(vk::Format::eAstc8x6UnormBlock)},
		{"FORMAT_ASTC_8x6_SRGB_BLOCK",umath::to_integral(vk::Format::eAstc8x6SrgbBlock)},
		{"FORMAT_ASTC_8x8_UNORM_BLOCK",umath::to_integral(vk::Format::eAstc8x8UnormBlock)},
		{"FORMAT_ASTC_8x8_SRGB_BLOCK",umath::to_integral(vk::Format::eAstc8x8SrgbBlock)},
		{"FORMAT_ASTC_10x5_UNORM_BLOCK",umath::to_integral(vk::Format::eAstc10x5UnormBlock)},
		{"FORMAT_ASTC_10x5_SRGB_BLOCK",umath::to_integral(vk::Format::eAstc10x5SrgbBlock)},
		{"FORMAT_ASTC_10x6_UNORM_BLOCK",umath::to_integral(vk::Format::eAstc10x6UnormBlock)},
		{"FORMAT_ASTC_10x6_SRGB_BLOCK",umath::to_integral(vk::Format::eAstc10x6SrgbBlock)},
		{"FORMAT_ASTC_10x8_UNORM_BLOCK",umath::to_integral(vk::Format::eAstc10x8UnormBlock)},
		{"FORMAT_ASTC_10x8_SRGB_BLOCK",umath::to_integral(vk::Format::eAstc10x8SrgbBlock)},
		{"FORMAT_ASTC_10x10_UNORM_BLOCK",umath::to_integral(vk::Format::eAstc10x10UnormBlock)},
		{"FORMAT_ASTC_10x10_SRGB_BLOCK",umath::to_integral(vk::Format::eAstc10x10SrgbBlock)},
		{"FORMAT_ASTC_12x10_UNORM_BLOCK",umath::to_integral(vk::Format::eAstc12x10UnormBlock)},
		{"FORMAT_ASTC_12x10_SRGB_BLOCK",umath::to_integral(vk::Format::eAstc12x10SrgbBlock)},
		{"FORMAT_ASTC_12x12_UNORM_BLOCK",umath::to_integral(vk::Format::eAstc12x12UnormBlock)},
		{"FORMAT_ASTC_12x12_SRGB_BLOCK",umath::to_integral(vk::Format::eAstc12x12SrgbBlock)},

		{"RESULT_SUCCESS",umath::to_integral(vk::Result::eSuccess)},
		{"RESULT_NOT_READY",umath::to_integral(vk::Result::eNotReady)},
		{"RESULT_TIMEOUT",umath::to_integral(vk::Result::eTimeout)},
		{"RESULT_EVENT_SET",umath::to_integral(vk::Result::eEventSet)},
		{"RESULT_EVENT_RESET",umath::to_integral(vk::Result::eEventReset)},
		{"RESULT_INCOMPLETE",umath::to_integral(vk::Result::eIncomplete)},
		{"RESULT_ERROR_OUT_OF_HOST_MEMORY",umath::to_integral(vk::Result::eErrorOutOfHostMemory)},
		{"RESULT_ERROR_OUT_OF_DEVICE_MEMORY",umath::to_integral(vk::Result::eErrorOutOfDeviceMemory)},
		{"RESULT_ERROR_INITIALIZATION_FAILED",umath::to_integral(vk::Result::eErrorInitializationFailed)},
		{"RESULT_ERROR_DEVICE_LOST",umath::to_integral(vk::Result::eErrorDeviceLost)},
		{"RESULT_ERROR_MEMORY_MAP_FAILED",umath::to_integral(vk::Result::eErrorMemoryMapFailed)},
		{"RESULT_ERROR_LAYER_NOT_PRESENT",umath::to_integral(vk::Result::eErrorLayerNotPresent)},
		{"RESULT_ERROR_EXTENSION_NOT_PRESENT",umath::to_integral(vk::Result::eErrorExtensionNotPresent)},
		{"RESULT_ERROR_FEATURE_NOT_PRESENT",umath::to_integral(vk::Result::eErrorFeatureNotPresent)},
		{"RESULT_ERROR_INCOMPATIBLE_DRIVER",umath::to_integral(vk::Result::eErrorIncompatibleDriver)},
		{"RESULT_ERROR_TOO_MANY_OBJECTS",umath::to_integral(vk::Result::eErrorTooManyObjects)},
		{"RESULT_ERROR_FORMAT_NOT_SUPPORTED",umath::to_integral(vk::Result::eErrorFormatNotSupported)},
		{"RESULT_ERROR_SURFACE_LOST_KHR",umath::to_integral(vk::Result::eErrorSurfaceLostKHR)},
		{"RESULT_ERROR_NATIVE_WINDOW_IN_USE_KHR",umath::to_integral(vk::Result::eErrorNativeWindowInUseKHR)},
		{"RESULT_SUBOPTIMAL_KHR",umath::to_integral(vk::Result::eSuboptimalKHR)},
		{"RESULT_ERROR_OUT_OF_DATE_KHR",umath::to_integral(vk::Result::eErrorOutOfDateKHR)},
		{"RESULT_ERROR_INCOMPATIBLE_DISPLAY_KHR",umath::to_integral(vk::Result::eErrorIncompatibleDisplayKHR)},
		{"RESULT_ERROR_VALIDATION_FAILED_EXT",umath::to_integral(vk::Result::eErrorValidationFailedEXT)},

		{"SAMPLER_MIPMAP_MODE_LINEAR",umath::to_integral(vk::SamplerMipmapMode::eLinear)},
		{"SAMPLER_MIPMAP_MODE_NEAREST",umath::to_integral(vk::SamplerMipmapMode::eNearest)},

		{"SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER",umath::to_integral(vk::SamplerAddressMode::eClampToBorder)},
		{"SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE",umath::to_integral(vk::SamplerAddressMode::eClampToEdge)},
		{"SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE",umath::to_integral(vk::SamplerAddressMode::eMirrorClampToEdge)},
		{"SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT",umath::to_integral(vk::SamplerAddressMode::eMirroredRepeat)},
		{"SAMPLER_ADDRESS_MODE_REPEAT",umath::to_integral(vk::SamplerAddressMode::eRepeat)},

		{"COMPARE_OP_ALWAYS",umath::to_integral(vk::CompareOp::eAlways)},
		{"COMPARE_OP_EQUAL",umath::to_integral(vk::CompareOp::eEqual)},
		{"COMPARE_OP_GREATER",umath::to_integral(vk::CompareOp::eGreater)},
		{"COMPARE_OP_GREATER_OR_EQUAL",umath::to_integral(vk::CompareOp::eGreaterOrEqual)},
		{"COMPARE_OP_LESS",umath::to_integral(vk::CompareOp::eLess)},
		{"COMPARE_OP_LESS_OR_EQUAL",umath::to_integral(vk::CompareOp::eLessOrEqual)},
		{"COMPARE_OP_NEVER",umath::to_integral(vk::CompareOp::eNever)},
		{"COMPARE_OP_NOT_EQUAL",umath::to_integral(vk::CompareOp::eNotEqual)},

		{"BORDER_COLOR_FLOAT_OPAQUE_BLACK",umath::to_integral(vk::BorderColor::eFloatOpaqueBlack)},
		{"BORDER_COLOR_FLOAT_OPAQUE_WHITE",umath::to_integral(vk::BorderColor::eFloatOpaqueWhite)},
		{"BORDER_COLOR_floatRANSPARENT_BLACK",umath::to_integral(vk::BorderColor::eFloatTransparentBlack)},
		{"BORDER_COLOR_INT_OPAQUE_BLACK",umath::to_integral(vk::BorderColor::eIntOpaqueBlack)},
		{"BORDER_COLOR_INT_OPAQUE_WHITE",umath::to_integral(vk::BorderColor::eIntOpaqueWhite)},
		{"BORDER_COLOR_INT_TRANSPARENT_BLACK",umath::to_integral(vk::BorderColor::eIntTransparentBlack)},

		{"IMAGE_LAYOUT_UNDEFINED",umath::to_integral(vk::ImageLayout::eUndefined)},
		{"IMAGE_LAYOUT_GENERAL",umath::to_integral(vk::ImageLayout::eGeneral)},
		{"IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL",umath::to_integral(vk::ImageLayout::eColorAttachmentOptimal)},
		{"IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL",umath::to_integral(vk::ImageLayout::eDepthStencilAttachmentOptimal)},
		{"IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL",umath::to_integral(vk::ImageLayout::eDepthStencilReadOnlyOptimal)},
		{"IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL",umath::to_integral(vk::ImageLayout::eShaderReadOnlyOptimal)},
		{"IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL",umath::to_integral(vk::ImageLayout::eTransferSrcOptimal)},
		{"IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL",umath::to_integral(vk::ImageLayout::eTransferDstOptimal)},
		{"IMAGE_LAYOUT_PREINITIALIZED",umath::to_integral(vk::ImageLayout::ePreinitialized)},
		{"IMAGE_LAYOUT_PRESENT_SRC_KHR",umath::to_integral(vk::ImageLayout::ePresentSrcKHR)},

		{"ATTACHMENT_LOAD_OP_LOAD",umath::to_integral(vk::AttachmentLoadOp::eLoad)},
		{"ATTACHMENT_LOAD_OP_CLEAR",umath::to_integral(vk::AttachmentLoadOp::eClear)},
		{"ATTACHMENT_LOAD_OP_DONT_CARE",umath::to_integral(vk::AttachmentLoadOp::eDontCare)},

		{"ATTACHMENT_STORE_OP_STORE",umath::to_integral(vk::AttachmentStoreOp::eStore)},
		{"ATTACHMENT_STORE_OP_DONT_CARE",umath::to_integral(vk::AttachmentStoreOp::eDontCare)},

		{"IMAGE_TYPE_1D",umath::to_integral(vk::ImageType::e1D)},
		{"IMAGE_TYPE_2D",umath::to_integral(vk::ImageType::e2D)},
		{"IMAGE_TYPE_3D",umath::to_integral(vk::ImageType::e3D)},

		{"IMAGE_TILING_OPTIMAL",umath::to_integral(vk::ImageTiling::eOptimal)},
		{"IMAGE_TILING_LINEAR",umath::to_integral(vk::ImageTiling::eLinear)},

		{"IMAGE_VIEW_TYPE_1D",umath::to_integral(vk::ImageViewType::e1D)},
		{"IMAGE_VIEW_TYPE_2D",umath::to_integral(vk::ImageViewType::e2D)},
		{"IMAGE_VIEW_TYPE_3D",umath::to_integral(vk::ImageViewType::e3D)},
		{"IMAGE_VIEW_TYPE_CUBE",umath::to_integral(vk::ImageViewType::eCube)},
		{"IMAGE_VIEW_TYPE_1D_ARRAY",umath::to_integral(vk::ImageViewType::e1DArray)},
		{"IMAGE_VIEW_TYPE_2D_ARRAY",umath::to_integral(vk::ImageViewType::e2DArray)},
		{"IMAGE_VIEW_TYPE_CUBE_ARRAY",umath::to_integral(vk::ImageViewType::eCubeArray)},

		{"COMMAND_BUFFER_LEVEL_PRIMARY",umath::to_integral(vk::CommandBufferLevel::ePrimary)},
		{"COMMAND_BUFFER_LEVEL_SECONDARY",umath::to_integral(vk::CommandBufferLevel::eSecondary)},

		{"COMPONENT_SWIZZLE_IDENTITY",umath::to_integral(vk::ComponentSwizzle::eIdentity)},
		{"COMPONENT_SWIZZLE_ZERO",umath::to_integral(vk::ComponentSwizzle::eZero)},
		{"COMPONENT_SWIZZLE_ONE",umath::to_integral(vk::ComponentSwizzle::eOne)},
		{"COMPONENT_SWIZZLE_R",umath::to_integral(vk::ComponentSwizzle::eR)},
		{"COMPONENT_SWIZZLE_G",umath::to_integral(vk::ComponentSwizzle::eG)},
		{"COMPONENT_SWIZZLE_B",umath::to_integral(vk::ComponentSwizzle::eB)},
		{"COMPONENT_SWIZZLE_A",umath::to_integral(vk::ComponentSwizzle::eA)},

		{"DESCRIPTOR_TYPE_SAMPLER",umath::to_integral(vk::DescriptorType::eSampler)},
		{"DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER",umath::to_integral(vk::DescriptorType::eCombinedImageSampler)},
		{"DESCRIPTOR_TYPE_SAMPLED_IMAGE",umath::to_integral(vk::DescriptorType::eSampledImage)},
		{"DESCRIPTOR_TYPE_STORAGE_IMAGE",umath::to_integral(vk::DescriptorType::eStorageImage)},
		{"DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER",umath::to_integral(vk::DescriptorType::eUniformTexelBuffer)},
		{"DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER",umath::to_integral(vk::DescriptorType::eStorageTexelBuffer)},
		{"DESCRIPTOR_TYPE_UNIFORM_BUFFER",umath::to_integral(vk::DescriptorType::eUniformBuffer)},
		{"DESCRIPTOR_TYPE_STORAGE_BUFFER",umath::to_integral(vk::DescriptorType::eStorageBuffer)},
		{"DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC",umath::to_integral(vk::DescriptorType::eUniformBufferDynamic)},
		{"DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC",umath::to_integral(vk::DescriptorType::eStorageBufferDynamic)},
		{"DESCRIPTOR_TYPE_INPUT_ATTACHMENT",umath::to_integral(vk::DescriptorType::eInputAttachment)},

		{"QUERY_TYPE_OCCLUSION",umath::to_integral(vk::QueryType::eOcclusion)},
		{"QUERY_TYPE_PIPELINE_STATISTICS",umath::to_integral(vk::QueryType::ePipelineStatistics)},
		{"QUERY_TYPE_TIMESTAMP",umath::to_integral(vk::QueryType::eTimestamp)},

		{"PIPELINE_BIND_POINT_GRAPHICS",umath::to_integral(vk::PipelineBindPoint::eGraphics)},
		{"PIPELINE_BIND_POINT_COMPUTE",umath::to_integral(vk::PipelineBindPoint::eCompute)},

		{"PIPELINE_CACHE_HEADER_VERSION_ONE",umath::to_integral(vk::PipelineCacheHeaderVersion::eOne)},

		{"PRIMITIVE_TOPOLOGY_POINT_LIST",umath::to_integral(vk::PrimitiveTopology::ePointList)},
		{"PRIMITIVE_TOPOLOGY_LINE_LIST",umath::to_integral(vk::PrimitiveTopology::eLineList)},
		{"PRIMITIVE_TOPOLOGY_LINE_STRIP",umath::to_integral(vk::PrimitiveTopology::eLineStrip)},
		{"PRIMITIVE_TOPOLOGY_TRIANGLE_LIST",umath::to_integral(vk::PrimitiveTopology::eTriangleList)},
		{"PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP",umath::to_integral(vk::PrimitiveTopology::eTriangleStrip)},
		{"PRIMITIVE_TOPOLOGY_TRIANGLE_FAN",umath::to_integral(vk::PrimitiveTopology::eTriangleFan)},
		{"PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY",umath::to_integral(vk::PrimitiveTopology::eLineListWithAdjacency)},
		{"PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY",umath::to_integral(vk::PrimitiveTopology::eLineStripWithAdjacency)},
		{"PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY",umath::to_integral(vk::PrimitiveTopology::eTriangleListWithAdjacency)},
		{"PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY",umath::to_integral(vk::PrimitiveTopology::eTriangleStripWithAdjacency)},
		{"PRIMITIVE_TOPOLOGY_PATCH_LIST",umath::to_integral(vk::PrimitiveTopology::ePatchList)},

		{"SHARING_MODE_EXCLUSIVE",umath::to_integral(vk::SharingMode::eExclusive)},
		{"SHARING_MODE_CONCURRENT",umath::to_integral(vk::SharingMode::eConcurrent)},

		{"INDEX_TYPE_UINT16",umath::to_integral(vk::IndexType::eUint16)},
		{"INDEX_TYPE_UINT32",umath::to_integral(vk::IndexType::eUint32)},

		{"FILTER_NEAREST",umath::to_integral(vk::Filter::eNearest)},
		{"FILTER_LINEAR",umath::to_integral(vk::Filter::eLinear)},

		{"POLYGON_MODE_FILL",umath::to_integral(vk::PolygonMode::eFill)},
		{"POLYGON_MODE_LINE",umath::to_integral(vk::PolygonMode::eLine)},
		{"POLYGON_MODE_POINT",umath::to_integral(vk::PolygonMode::ePoint)},

		{"CULL_MODE_NONE",umath::to_integral(vk::CullModeFlagBits::eNone)},
		{"CULL_MODE_FRONT_BIT",umath::to_integral(vk::CullModeFlagBits::eFront)},
		{"CULL_MODE_BACK_BIT",umath::to_integral(vk::CullModeFlagBits::eBack)},
		{"CULL_MODE_FRONT_AND_BACK",umath::to_integral(vk::CullModeFlagBits::eFrontAndBack)},

		{"FRONT_FACE_COUNTER_CLOCKWISE",umath::to_integral(vk::FrontFace::eCounterClockwise)},
		{"FRONT_FACE_CLOCKWISE",umath::to_integral(vk::FrontFace::eClockwise)},

		{"BLEND_FACTOR_ZERO",umath::to_integral(vk::BlendFactor::eZero)},
		{"BLEND_FACTOR_ONE",umath::to_integral(vk::BlendFactor::eOne)},
		{"BLEND_FACTOR_SRC_COLOR",umath::to_integral(vk::BlendFactor::eSrcColor)},
		{"BLEND_FACTOR_ONE_MINUS_SRC_COLOR",umath::to_integral(vk::BlendFactor::eOneMinusSrcColor)},
		{"BLEND_FACTOR_DST_COLOR",umath::to_integral(vk::BlendFactor::eDstColor)},
		{"BLEND_FACTOR_ONE_MINUS_DST_COLOR",umath::to_integral(vk::BlendFactor::eOneMinusDstColor)},
		{"BLEND_FACTOR_SRC_ALPHA",umath::to_integral(vk::BlendFactor::eSrcAlpha)},
		{"BLEND_FACTOR_ONE_MINUS_SRC_ALPHA",umath::to_integral(vk::BlendFactor::eOneMinusSrcAlpha)},
		{"BLEND_FACTOR_DST_ALPHA",umath::to_integral(vk::BlendFactor::eDstAlpha)},
		{"BLEND_FACTOR_ONE_MINUS_DST_ALPHA",umath::to_integral(vk::BlendFactor::eOneMinusDstAlpha)},
		{"BLEND_FACTOR_CONSTANT_COLOR",umath::to_integral(vk::BlendFactor::eConstantColor)},
		{"BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR",umath::to_integral(vk::BlendFactor::eOneMinusConstantColor)},
		{"BLEND_FACTOR_CONSTANT_ALPHA",umath::to_integral(vk::BlendFactor::eConstantAlpha)},
		{"BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA",umath::to_integral(vk::BlendFactor::eOneMinusConstantAlpha)},
		{"BLEND_FACTOR_SRC_ALPHA_SATURATE",umath::to_integral(vk::BlendFactor::eSrcAlphaSaturate)},
		{"BLEND_FACTOR_SRC1_COLOR",umath::to_integral(vk::BlendFactor::eSrc1Color)},
		{"BLEND_FACTOR_ONE_MINUS_SRC1_COLOR",umath::to_integral(vk::BlendFactor::eOneMinusSrc1Color)},
		{"BLEND_FACTOR_SRC1_ALPHA",umath::to_integral(vk::BlendFactor::eSrc1Alpha)},
		{"BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA",umath::to_integral(vk::BlendFactor::eOneMinusSrc1Alpha)},

		{"BLEND_OP_ADD",umath::to_integral(vk::BlendOp::eAdd)},
		{"BLEND_OP_SUBTRACT",umath::to_integral(vk::BlendOp::eSubtract)},
		{"BLEND_OP_REVERSE_SUBTRACT",umath::to_integral(vk::BlendOp::eReverseSubtract)},
		{"BLEND_OP_MIN",umath::to_integral(vk::BlendOp::eMin)},
		{"BLEND_OP_MAX",umath::to_integral(vk::BlendOp::eMax)},

		{"STENCIL_OP_KEEP",umath::to_integral(vk::StencilOp::eKeep)},
		{"STENCIL_OP_ZERO",umath::to_integral(vk::StencilOp::eZero)},
		{"STENCIL_OP_REPLACE",umath::to_integral(vk::StencilOp::eReplace)},
		{"STENCIL_OP_INCREMENT_AND_CLAMP",umath::to_integral(vk::StencilOp::eIncrementAndClamp)},
		{"STENCIL_OP_DECREMENT_AND_CLAMP",umath::to_integral(vk::StencilOp::eDecrementAndClamp)},
		{"STENCIL_OP_INVERT",umath::to_integral(vk::StencilOp::eInvert)},
		{"STENCIL_OP_INCREMENT_AND_WRAP",umath::to_integral(vk::StencilOp::eIncrementAndWrap)},
		{"STENCIL_OP_DECREMENT_AND_WRAP",umath::to_integral(vk::StencilOp::eDecrementAndWrap)},

		{"LOGIC_OP_CLEAR",umath::to_integral(vk::LogicOp::eClear)},
		{"LOGIC_OP_AND",umath::to_integral(vk::LogicOp::eAnd)},
		{"LOGIC_OP_AND_REVERSE",umath::to_integral(vk::LogicOp::eAndReverse)},
		{"LOGIC_OP_COPY",umath::to_integral(vk::LogicOp::eCopy)},
		{"LOGIC_OP_AND_INVERTED",umath::to_integral(vk::LogicOp::eAndInverted)},
		{"LOGIC_OP_NO_OP",umath::to_integral(vk::LogicOp::eNoOp)},
		{"LOGIC_OP_XOR",umath::to_integral(vk::LogicOp::eXor)},
		{"LOGIC_OP_OR",umath::to_integral(vk::LogicOp::eOr)},
		{"LOGIC_OP_NOR",umath::to_integral(vk::LogicOp::eNor)},
		{"LOGIC_OP_EQUIVALENT",umath::to_integral(vk::LogicOp::eEquivalent)},
		{"LOGIC_OP_INVERT",umath::to_integral(vk::LogicOp::eInvert)},
		{"LOGIC_OP_OR_REVERSE",umath::to_integral(vk::LogicOp::eOrReverse)},
		{"LOGIC_OP_COPY_INVERTED",umath::to_integral(vk::LogicOp::eCopyInverted)},
		{"LOGIC_OP_OR_INVERTED",umath::to_integral(vk::LogicOp::eOrInverted)},
		{"LOGIC_OP_NAND",umath::to_integral(vk::LogicOp::eNand)},
		{"LOGIC_OP_SET",umath::to_integral(vk::LogicOp::eSet)},

		{"INTERNAL_ALLOCATION_TYPE_EXECUTABLE",umath::to_integral(vk::InternalAllocationType::eExecutable)},

		{"SYSTEM_ALLOCATION_SCOPE_COMMAND",umath::to_integral(vk::SystemAllocationScope::eCommand)},
		{"SYSTEM_ALLOCATION_SCOPE_OBJECT",umath::to_integral(vk::SystemAllocationScope::eObject)},
		{"SYSTEM_ALLOCATION_SCOPE_CACHE",umath::to_integral(vk::SystemAllocationScope::eCache)},
		{"SYSTEM_ALLOCATION_SCOPE_DEVICE",umath::to_integral(vk::SystemAllocationScope::eDevice)},
		{"SYSTEM_ALLOCATION_SCOPE_INSTANCE",umath::to_integral(vk::SystemAllocationScope::eInstance)},

		{"PHYSICAL_DEVICE_TYPE_OTHER",umath::to_integral(vk::PhysicalDeviceType::eOther)},
		{"PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU",umath::to_integral(vk::PhysicalDeviceType::eIntegratedGpu)},
		{"PHYSICAL_DEVICE_TYPE_DISCRETE_GPU",umath::to_integral(vk::PhysicalDeviceType::eDiscreteGpu)},
		{"PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU",umath::to_integral(vk::PhysicalDeviceType::eVirtualGpu)},
		{"PHYSICAL_DEVICE_TYPE_CPU",umath::to_integral(vk::PhysicalDeviceType::eCpu)},

		{"VERTEX_INPUT_RATE_VERTEX",umath::to_integral(vk::VertexInputRate::eVertex)},
		{"VERTEX_INPUT_RATE_INSTANCE",umath::to_integral(vk::VertexInputRate::eInstance)},

		{"SUBPASS_CONTENTS_INLINE",umath::to_integral(vk::SubpassContents::eInline)},
		{"SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS",umath::to_integral(vk::SubpassContents::eSecondaryCommandBuffers)},

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
		{"DYNAMIC_STATE_VIEWPORT_W_SCALING_NV_BIT",umath::to_integral(prosper::util::DynamicStateFlags::ViewportWScalingNV)},
		{"DYNAMIC_STATE_DISCARD_RECTANGLE_EXT_BIT",umath::to_integral(prosper::util::DynamicStateFlags::DiscardRectangleEXT)},
		{"DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT_BIT",umath::to_integral(prosper::util::DynamicStateFlags::SampleLocationsEXT)},
		{"DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV_BIT",umath::to_integral(prosper::util::DynamicStateFlags::ViewportShadingRatePaletteNV)},
		{"DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV_BIT",umath::to_integral(prosper::util::DynamicStateFlags::ViewportCoarseSampleOrderNV)},
		{"DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV_BIT",umath::to_integral(prosper::util::DynamicStateFlags::ExclusiveScissorNV)},

		{"QUEUE_GRAPHICS_BIT",umath::to_integral(vk::QueueFlagBits::eGraphics)},
		{"QUEUE_COMPUTE_BIT",umath::to_integral(vk::QueueFlagBits::eCompute)},
		{"QUEUE_TRANSFER_BIT",umath::to_integral(vk::QueueFlagBits::eTransfer)},
		{"QUEUE_SPARSE_BINDING_BIT",umath::to_integral(vk::QueueFlagBits::eSparseBinding)},

		{"MEMORY_PROPERTY_DEVICE_LOCAL_BIT",umath::to_integral(vk::MemoryPropertyFlagBits::eDeviceLocal)},
		{"MEMORY_PROPERTY_HOST_VISIBLE_BIT",umath::to_integral(vk::MemoryPropertyFlagBits::eHostVisible)},
		{"MEMORY_PROPERTY_HOST_COHERENT_BIT",umath::to_integral(vk::MemoryPropertyFlagBits::eHostCoherent)},
		{"MEMORY_PROPERTY_HOST_CACHED_BIT",umath::to_integral(vk::MemoryPropertyFlagBits::eHostCached)},
		{"MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT",umath::to_integral(vk::MemoryPropertyFlagBits::eLazilyAllocated)},

		{"MEMORY_HEAP_DEVICE_LOCAL_BIT",umath::to_integral(vk::MemoryHeapFlagBits::eDeviceLocal)},
	
		{"ACCESS_INDIRECT_COMMAND_READ_BIT",umath::to_integral(vk::AccessFlagBits::eIndirectCommandRead)},
		{"ACCESS_INDEX_READ_BIT",umath::to_integral(vk::AccessFlagBits::eIndexRead)},
		{"ACCESS_VERTEX_ATTRIBUTE_READ_BIT",umath::to_integral(vk::AccessFlagBits::eVertexAttributeRead)},
		{"ACCESS_UNIFORM_READ_BIT",umath::to_integral(vk::AccessFlagBits::eUniformRead)},
		{"ACCESS_INPUT_ATTACHMENT_READ_BIT",umath::to_integral(vk::AccessFlagBits::eInputAttachmentRead)},
		{"ACCESS_SHADER_READ_BIT",umath::to_integral(vk::AccessFlagBits::eShaderRead)},
		{"ACCESS_SHADER_WRITE_BIT",umath::to_integral(vk::AccessFlagBits::eShaderWrite)},
		{"ACCESS_COLOR_ATTACHMENT_READ_BIT",umath::to_integral(vk::AccessFlagBits::eColorAttachmentRead)},
		{"ACCESS_COLOR_ATTACHMENT_WRITE_BIT",umath::to_integral(vk::AccessFlagBits::eColorAttachmentWrite)},
		{"ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT",umath::to_integral(vk::AccessFlagBits::eDepthStencilAttachmentRead)},
		{"ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT",umath::to_integral(vk::AccessFlagBits::eDepthStencilAttachmentWrite)},
		{"ACCESS_TRANSFER_READ_BIT",umath::to_integral(vk::AccessFlagBits::eTransferRead)},
		{"ACCESS_TRANSFER_WRITE_BIT",umath::to_integral(vk::AccessFlagBits::eTransferWrite)},
		{"ACCESS_HOST_READ_BIT",umath::to_integral(vk::AccessFlagBits::eHostRead)},
		{"ACCESS_HOST_WRITE_BIT",umath::to_integral(vk::AccessFlagBits::eHostWrite)},
		{"ACCESS_MEMORY_READ_BIT",umath::to_integral(vk::AccessFlagBits::eMemoryRead)},
		{"ACCESS_MEMORY_WRITE_BIT",umath::to_integral(vk::AccessFlagBits::eMemoryWrite)},

		{"BUFFER_USAGE_TRANSFER_SRC_BIT",umath::to_integral(vk::BufferUsageFlagBits::eTransferSrc)},
		{"BUFFER_USAGE_TRANSFER_DST_BIT",umath::to_integral(vk::BufferUsageFlagBits::eTransferDst)},
		{"BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT",umath::to_integral(vk::BufferUsageFlagBits::eUniformTexelBuffer)},
		{"BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT",umath::to_integral(vk::BufferUsageFlagBits::eStorageTexelBuffer)},
		{"BUFFER_USAGE_UNIFORM_BUFFER_BIT",umath::to_integral(vk::BufferUsageFlagBits::eUniformBuffer)},
		{"BUFFER_USAGE_STORAGE_BUFFER_BIT",umath::to_integral(vk::BufferUsageFlagBits::eStorageBuffer)},
		{"BUFFER_USAGE_INDEX_BUFFER_BIT",umath::to_integral(vk::BufferUsageFlagBits::eIndexBuffer)},
		{"BUFFER_USAGE_VERTEX_BUFFER_BIT",umath::to_integral(vk::BufferUsageFlagBits::eVertexBuffer)},
		{"BUFFER_USAGE_INDIRECT_BUFFER_BIT",umath::to_integral(vk::BufferUsageFlagBits::eIndirectBuffer)},

		{"BUFFER_CREATE_SPARSE_BINDING_BIT",umath::to_integral(vk::BufferCreateFlagBits::eSparseBinding)},
		{"BUFFER_CREATE_SPARSE_RESIDENCY_BIT",umath::to_integral(vk::BufferCreateFlagBits::eSparseResidency)},
		{"BUFFER_CREATE_SPARSE_ALIASED_BIT",umath::to_integral(vk::BufferCreateFlagBits::eSparseAliased)},

		{"SHADER_STAGE_VERTEX_BIT",umath::to_integral(vk::ShaderStageFlagBits::eVertex)},
		{"SHADER_STAGE_TESSELLATION_CONTROL_BIT",umath::to_integral(vk::ShaderStageFlagBits::eTessellationControl)},
		{"SHADER_STAGE_TESSELLATION_EVALUATION_BIT",umath::to_integral(vk::ShaderStageFlagBits::eTessellationEvaluation)},
		{"SHADER_STAGE_GEOMETRY_BIT",umath::to_integral(vk::ShaderStageFlagBits::eGeometry)},
		{"SHADER_STAGE_FRAGMENT_BIT",umath::to_integral(vk::ShaderStageFlagBits::eFragment)},
		{"SHADER_STAGE_COMPUTE_BIT",umath::to_integral(vk::ShaderStageFlagBits::eCompute)},
		{"SHADER_STAGE_ALL_GRAPHICS",umath::to_integral(vk::ShaderStageFlagBits::eAllGraphics)},
		{"SHADER_STAGE_ALL",umath::to_integral(vk::ShaderStageFlagBits::eAll)},

		{"IMAGE_USAGE_TRANSFER_SRC_BIT",umath::to_integral(vk::ImageUsageFlagBits::eTransferSrc)},
		{"IMAGE_USAGE_TRANSFER_DST_BIT",umath::to_integral(vk::ImageUsageFlagBits::eTransferDst)},
		{"IMAGE_USAGE_SAMPLED_BIT",umath::to_integral(vk::ImageUsageFlagBits::eSampled)},
		{"IMAGE_USAGE_STORAGE_BIT",umath::to_integral(vk::ImageUsageFlagBits::eStorage)},
		{"IMAGE_USAGE_COLOR_ATTACHMENT_BIT",umath::to_integral(vk::ImageUsageFlagBits::eColorAttachment)},
		{"IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT",umath::to_integral(vk::ImageUsageFlagBits::eDepthStencilAttachment)},
		{"IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT",umath::to_integral(vk::ImageUsageFlagBits::eTransientAttachment)},
		{"IMAGE_USAGE_INPUT_ATTACHMENT_BIT",umath::to_integral(vk::ImageUsageFlagBits::eInputAttachment)},

		{"IMAGE_CREATE_SPARSE_BINDING_BIT",umath::to_integral(vk::ImageCreateFlagBits::eSparseBinding)},
		{"IMAGE_CREATE_SPARSE_RESIDENCY_BIT",umath::to_integral(vk::ImageCreateFlagBits::eSparseResidency)},
		{"IMAGE_CREATE_SPARSE_ALIASED_BIT",umath::to_integral(vk::ImageCreateFlagBits::eSparseAliased)},
		{"IMAGE_CREATE_MUTABLE_FORMAT_BIT",umath::to_integral(vk::ImageCreateFlagBits::eMutableFormat)},
		{"IMAGE_CREATE_CUBE_COMPATIBLE_BIT",umath::to_integral(vk::ImageCreateFlagBits::eCubeCompatible)},

		{"PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT",umath::to_integral(vk::PipelineCreateFlagBits::eDisableOptimization)},
		{"PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT",umath::to_integral(vk::PipelineCreateFlagBits::eAllowDerivatives)},
		{"PIPELINE_CREATE_DERIVATIVE_BIT",umath::to_integral(vk::PipelineCreateFlagBits::eDerivative)},

		{"COLOR_COMPONENT_R_BIT",umath::to_integral(vk::ColorComponentFlagBits::eR)},
		{"COLOR_COMPONENT_G_BIT",umath::to_integral(vk::ColorComponentFlagBits::eG)},
		{"COLOR_COMPONENT_B_BIT",umath::to_integral(vk::ColorComponentFlagBits::eB)},
		{"COLOR_COMPONENT_A_BIT",umath::to_integral(vk::ColorComponentFlagBits::eA)},

		{"FENCE_CREATE_SIGNALED_BIT",umath::to_integral(vk::FenceCreateFlagBits::eSignaled)},

		{"FORMAT_FEATURE_SAMPLED_IMAGE_BIT",umath::to_integral(vk::FormatFeatureFlagBits::eSampledImage)},
		{"FORMAT_FEATURE_STORAGE_IMAGE_BIT",umath::to_integral(vk::FormatFeatureFlagBits::eStorageImage)},
		{"FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT",umath::to_integral(vk::FormatFeatureFlagBits::eStorageImageAtomic)},
		{"FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT",umath::to_integral(vk::FormatFeatureFlagBits::eUniformTexelBuffer)},
		{"FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT",umath::to_integral(vk::FormatFeatureFlagBits::eStorageTexelBuffer)},
		{"FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT",umath::to_integral(vk::FormatFeatureFlagBits::eStorageTexelBufferAtomic)},
		{"FORMAT_FEATURE_VERTEX_BUFFER_BIT",umath::to_integral(vk::FormatFeatureFlagBits::eVertexBuffer)},
		{"FORMAT_FEATURE_COLOR_ATTACHMENT_BIT",umath::to_integral(vk::FormatFeatureFlagBits::eColorAttachment)},
		{"FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT",umath::to_integral(vk::FormatFeatureFlagBits::eColorAttachmentBlend)},
		{"FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT",umath::to_integral(vk::FormatFeatureFlagBits::eDepthStencilAttachment)},
		{"FORMAT_FEATURE_BLIT_SRC_BIT",umath::to_integral(vk::FormatFeatureFlagBits::eBlitSrc)},
		{"FORMAT_FEATURE_BLIT_DST_BIT",umath::to_integral(vk::FormatFeatureFlagBits::eBlitDst)},
		{"FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT",umath::to_integral(vk::FormatFeatureFlagBits::eSampledImageFilterLinear)},

		{"QUERY_CONTROL_PRECISE_BIT",umath::to_integral(vk::QueryControlFlagBits::ePrecise)},

		{"QUERY_RESULT_64_BIT",umath::to_integral(vk::QueryResultFlagBits::e64)},
		{"QUERY_RESULT_WAIT_BIT",umath::to_integral(vk::QueryResultFlagBits::eWait)},
		{"QUERY_RESULT_WITH_AVAILABILITY_BIT",umath::to_integral(vk::QueryResultFlagBits::eWithAvailability)},
		{"QUERY_RESULT_PARTIAL_BIT",umath::to_integral(vk::QueryResultFlagBits::ePartial)},

		{"COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT",umath::to_integral(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)},
		{"COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT",umath::to_integral(vk::CommandBufferUsageFlagBits::eRenderPassContinue)},
		{"COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT",umath::to_integral(vk::CommandBufferUsageFlagBits::eSimultaneousUse)},

		{"QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT",umath::to_integral(vk::QueryPipelineStatisticFlagBits::eInputAssemblyVertices)},
		{"QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT",umath::to_integral(vk::QueryPipelineStatisticFlagBits::eInputAssemblyPrimitives)},
		{"QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT",umath::to_integral(vk::QueryPipelineStatisticFlagBits::eVertexShaderInvocations)},
		{"QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT",umath::to_integral(vk::QueryPipelineStatisticFlagBits::eGeometryShaderInvocations)},
		{"QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT",umath::to_integral(vk::QueryPipelineStatisticFlagBits::eGeometryShaderPrimitives)},
		{"QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT",umath::to_integral(vk::QueryPipelineStatisticFlagBits::eClippingInvocations)},
		{"QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT",umath::to_integral(vk::QueryPipelineStatisticFlagBits::eClippingPrimitives)},
		{"QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT",umath::to_integral(vk::QueryPipelineStatisticFlagBits::eFragmentShaderInvocations)},
		{"QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT",umath::to_integral(vk::QueryPipelineStatisticFlagBits::eTessellationControlShaderPatches)},
		{"QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT",umath::to_integral(vk::QueryPipelineStatisticFlagBits::eTessellationEvaluationShaderInvocations)},
		{"QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT",umath::to_integral(vk::QueryPipelineStatisticFlagBits::eComputeShaderInvocations)},

		{"IMAGE_ASPECT_COLOR_BIT",umath::to_integral(vk::ImageAspectFlagBits::eColor)},
		{"IMAGE_ASPECT_DEPTH_BIT",umath::to_integral(vk::ImageAspectFlagBits::eDepth)},
		{"IMAGE_ASPECT_STENCIL_BIT",umath::to_integral(vk::ImageAspectFlagBits::eStencil)},
		{"IMAGE_ASPECT_METADATA_BIT",umath::to_integral(vk::ImageAspectFlagBits::eMetadata)},

		{"SPARSE_IMAGE_FORMAT_SINGLE_MIPTAIL_BIT",umath::to_integral(vk::SparseImageFormatFlagBits::eSingleMiptail)},
		{"SPARSE_IMAGE_FORMAT_ALIGNED_MIP_SIZE_BIT",umath::to_integral(vk::SparseImageFormatFlagBits::eAlignedMipSize)},
		{"SPARSE_IMAGE_FORMAT_NONSTANDARD_BLOCK_SIZE_BIT",umath::to_integral(vk::SparseImageFormatFlagBits::eNonstandardBlockSize)},

		{"SPARSE_MEMORY_BIND_METADATA_BIT",umath::to_integral(vk::SparseMemoryBindFlagBits::eMetadata)},

		{"PIPELINE_STAGE_TOP_OF_PIPE_BIT",umath::to_integral(vk::PipelineStageFlagBits::eTopOfPipe)},
		{"PIPELINE_STAGE_DRAW_INDIRECT_BIT",umath::to_integral(vk::PipelineStageFlagBits::eDrawIndirect)},
		{"PIPELINE_STAGE_VERTEX_INPUT_BIT",umath::to_integral(vk::PipelineStageFlagBits::eVertexInput)},
		{"PIPELINE_STAGE_VERTEX_SHADER_BIT",umath::to_integral(vk::PipelineStageFlagBits::eVertexShader)},
		{"PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT",umath::to_integral(vk::PipelineStageFlagBits::eTessellationControlShader)},
		{"PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT",umath::to_integral(vk::PipelineStageFlagBits::eTessellationEvaluationShader)},
		{"PIPELINE_STAGE_GEOMETRY_SHADER_BIT",umath::to_integral(vk::PipelineStageFlagBits::eGeometryShader)},
		{"PIPELINE_STAGE_FRAGMENT_SHADER_BIT",umath::to_integral(vk::PipelineStageFlagBits::eFragmentShader)},
		{"PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT",umath::to_integral(vk::PipelineStageFlagBits::eEarlyFragmentTests)},
		{"PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT",umath::to_integral(vk::PipelineStageFlagBits::eLateFragmentTests)},
		{"PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT",umath::to_integral(vk::PipelineStageFlagBits::eColorAttachmentOutput)},
		{"PIPELINE_STAGE_COMPUTE_SHADER_BIT",umath::to_integral(vk::PipelineStageFlagBits::eComputeShader)},
		{"PIPELINE_STAGE_TRANSFER_BIT",umath::to_integral(vk::PipelineStageFlagBits::eTransfer)},
		{"PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT",umath::to_integral(vk::PipelineStageFlagBits::eBottomOfPipe)},
		{"PIPELINE_STAGE_HOST_BIT",umath::to_integral(vk::PipelineStageFlagBits::eHost)},
		{"PIPELINE_STAGE_ALL_GRAPHICS_BIT",umath::to_integral(vk::PipelineStageFlagBits::eAllGraphics)},
		{"PIPELINE_STAGE_ALL_COMMANDS_BIT",umath::to_integral(vk::PipelineStageFlagBits::eAllCommands)},

		{"COMMAND_POOL_CREATE_TRANSIENT_BIT",umath::to_integral(vk::CommandPoolCreateFlagBits::eTransient)},
		{"COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT",umath::to_integral(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)},

		{"COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT",umath::to_integral(vk::CommandPoolResetFlagBits::eReleaseResources)},

		{"COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT",umath::to_integral(vk::CommandBufferResetFlagBits::eReleaseResources)},

		{"SAMPLE_COUNT_1_BIT",umath::to_integral(vk::SampleCountFlagBits::e1)},
		{"SAMPLE_COUNT_2_BIT",umath::to_integral(vk::SampleCountFlagBits::e2)},
		{"SAMPLE_COUNT_4_BIT",umath::to_integral(vk::SampleCountFlagBits::e4)},
		{"SAMPLE_COUNT_8_BIT",umath::to_integral(vk::SampleCountFlagBits::e8)},
		{"SAMPLE_COUNT_16_BIT",umath::to_integral(vk::SampleCountFlagBits::e16)},
		{"SAMPLE_COUNT_32_BIT",umath::to_integral(vk::SampleCountFlagBits::e32)},
		{"SAMPLE_COUNT_64_BIT",umath::to_integral(vk::SampleCountFlagBits::e64)},

		{"ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT",umath::to_integral(vk::AttachmentDescriptionFlagBits::eMayAlias)},

		{"STENCIL_FACE_FRONT_BIT",umath::to_integral(vk::StencilFaceFlagBits::eFront)},
		{"STENCIL_FACE_BACK_BIT",umath::to_integral(vk::StencilFaceFlagBits::eBack)},
		{"STENCIL_FRONT_AND_BACK",umath::to_integral(vk::StencilFaceFlagBits::eVkStencilFrontAndBack)},

		{"DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT",umath::to_integral(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)},

		{"DEPENDENCY_BY_REGION_BIT",umath::to_integral(vk::DependencyFlagBits::eByRegion)},

		{"PRESENT_MODE_IMMEDIATE_KHR",umath::to_integral(vk::PresentModeKHR::eImmediate)},
		{"PRESENT_MODE_MAILBOX_KHR",umath::to_integral(vk::PresentModeKHR::eMailbox)},
		{"PRESENT_MODE_FIFO_KHR",umath::to_integral(vk::PresentModeKHR::eFifo)},
		{"PRESENT_MODE_FIFO_RELAXED_KHR",umath::to_integral(vk::PresentModeKHR::eFifoRelaxed)},

		{"COLORSPACE_SRGB_NONLINEAR_KHR",umath::to_integral(vk::ColorSpaceKHR::eSrgbNonlinear)},

		{"DISPLAY_PLANE_ALPHA_OPAQUE_BIT_KHR",umath::to_integral(vk::DisplayPlaneAlphaFlagBitsKHR::eOpaque)},
		{"DISPLAY_PLANE_ALPHA_GLOBAL_BIT_KHR",umath::to_integral(vk::DisplayPlaneAlphaFlagBitsKHR::eGlobal)},
		{"DISPLAY_PLANE_ALPHA_PER_PIXEL_BIT_KHR",umath::to_integral(vk::DisplayPlaneAlphaFlagBitsKHR::ePerPixel)},
		{"DISPLAY_PLANE_ALPHA_PER_PIXEL_PREMULTIPLIED_BIT_KHR",umath::to_integral(vk::DisplayPlaneAlphaFlagBitsKHR::ePerPixelPremultiplied)},

		{"COMPOSITE_ALPHA_OPAQUE_BIT_KHR",umath::to_integral(vk::CompositeAlphaFlagBitsKHR::eOpaque)},
		{"COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR",umath::to_integral(vk::CompositeAlphaFlagBitsKHR::ePreMultiplied)},
		{"COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR",umath::to_integral(vk::CompositeAlphaFlagBitsKHR::ePostMultiplied)},
		{"COMPOSITE_ALPHA_INHERIT_BIT_KHR",umath::to_integral(vk::CompositeAlphaFlagBitsKHR::eInherit)},

		{"SURFACE_TRANSFORM_IDENTITY_BIT_KHR",umath::to_integral(vk::SurfaceTransformFlagBitsKHR::eIdentity)},
		{"SURFACE_TRANSFORM_ROTATE_90_BIT_KHR",umath::to_integral(vk::SurfaceTransformFlagBitsKHR::eRotate90)},
		{"SURFACE_TRANSFORM_ROTATE_180_BIT_KHR",umath::to_integral(vk::SurfaceTransformFlagBitsKHR::eRotate180)},
		{"SURFACE_TRANSFORM_ROTATE_270_BIT_KHR",umath::to_integral(vk::SurfaceTransformFlagBitsKHR::eRotate270)},
		{"SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR",umath::to_integral(vk::SurfaceTransformFlagBitsKHR::eHorizontalMirror)},
		{"SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR",umath::to_integral(vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate90)},
		{"SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR",umath::to_integral(vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate180)},
		{"SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR",umath::to_integral(vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate270)},
		{"SURFACE_TRANSFORM_INHERIT_BIT_KHR",umath::to_integral(vk::SurfaceTransformFlagBitsKHR::eInherit)},

		{"DEBUG_REPORT_INFORMATION_BIT_EXT",umath::to_integral(vk::DebugReportFlagBitsEXT::eInformation)},
		{"DEBUG_REPORT_WARNING_BIT_EXT",umath::to_integral(vk::DebugReportFlagBitsEXT::eWarning)},
		{"DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT",umath::to_integral(vk::DebugReportFlagBitsEXT::ePerformanceWarning)},
		{"DEBUG_REPORT_ERROR_BIT_EXT",umath::to_integral(vk::DebugReportFlagBitsEXT::eError)},
		{"DEBUG_REPORT_DEBUG_BIT_EXT",umath::to_integral(vk::DebugReportFlagBitsEXT::eDebug)},

		{"DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT",umath::to_integral(vk::DebugReportObjectTypeEXT::eUnknown)},
		{"DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT",umath::to_integral(vk::DebugReportObjectTypeEXT::eInstance)},
		{"DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT",umath::to_integral(vk::DebugReportObjectTypeEXT::ePhysicalDevice)},
		{"DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT",umath::to_integral(vk::DebugReportObjectTypeEXT::eDevice)},
		{"DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT",umath::to_integral(vk::DebugReportObjectTypeEXT::eQueue)},
		{"DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT",umath::to_integral(vk::DebugReportObjectTypeEXT::eSemaphore)},
		{"DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT",umath::to_integral(vk::DebugReportObjectTypeEXT::eCommandBuffer)},
		{"DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT",umath::to_integral(vk::DebugReportObjectTypeEXT::eFence)},
		{"DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT",umath::to_integral(vk::DebugReportObjectTypeEXT::eDeviceMemory)},
		{"DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT",umath::to_integral(vk::DebugReportObjectTypeEXT::eBuffer)},
		{"DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT",umath::to_integral(vk::DebugReportObjectTypeEXT::eImage)},
		{"DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT",umath::to_integral(vk::DebugReportObjectTypeEXT::eEvent)},
		{"DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT",umath::to_integral(vk::DebugReportObjectTypeEXT::eQueryPool)},
		{"DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT",umath::to_integral(vk::DebugReportObjectTypeEXT::eBufferView)},
		{"DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT",umath::to_integral(vk::DebugReportObjectTypeEXT::eImageView)},
		{"DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT",umath::to_integral(vk::DebugReportObjectTypeEXT::eShaderModule)},
		{"DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT",umath::to_integral(vk::DebugReportObjectTypeEXT::ePipelineCache)},
		{"DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT",umath::to_integral(vk::DebugReportObjectTypeEXT::ePipelineLayout)},
		{"DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT",umath::to_integral(vk::DebugReportObjectTypeEXT::eRenderPass)},
		{"DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT",umath::to_integral(vk::DebugReportObjectTypeEXT::ePipeline)},
		{"DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT",umath::to_integral(vk::DebugReportObjectTypeEXT::eDescriptorSetLayout)},
		{"DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT",umath::to_integral(vk::DebugReportObjectTypeEXT::eSampler)},
		{"DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT",umath::to_integral(vk::DebugReportObjectTypeEXT::eDescriptorPool)},
		{"DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT",umath::to_integral(vk::DebugReportObjectTypeEXT::eDescriptorSet)},
		{"DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT",umath::to_integral(vk::DebugReportObjectTypeEXT::eFramebuffer)},
		{"DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT",umath::to_integral(vk::DebugReportObjectTypeEXT::eCommandPool)},
		{"DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT",umath::to_integral(vk::DebugReportObjectTypeEXT::eSurfaceKhr)},
		{"DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT",umath::to_integral(vk::DebugReportObjectTypeEXT::eSwapchainKhr)},

		{"MEMORY_FEATURE_DEVICE_LOCAL_BIT",umath::to_integral(prosper::util::MemoryFeatureFlags::DeviceLocal)},
		{"MEMORY_FEATURE_HOST_CACHED_BIT",umath::to_integral(prosper::util::MemoryFeatureFlags::HostCached)},
		{"MEMORY_FEATURE_HOST_COHERENT_BIT",umath::to_integral(prosper::util::MemoryFeatureFlags::HostCoherent)},
		{"MEMORY_FEATURE_LAZILY_ALLOCATED_BIT",umath::to_integral(prosper::util::MemoryFeatureFlags::LazilyAllocated)},
		{"MEMORY_FEATURE_HOST_ACCESSABLE_BIT",umath::to_integral(prosper::util::MemoryFeatureFlags::HostAccessable)},
		{"MEMORY_FEATURE_GPU_BULK",umath::to_integral(prosper::util::MemoryFeatureFlags::GPUBulk)},
		{"MEMORY_FEATURE_CPU_TO_GPU",umath::to_integral(prosper::util::MemoryFeatureFlags::CPUToGPU)},
		{"MEMORY_FEATURE_GPU_TO_CPU",umath::to_integral(prosper::util::MemoryFeatureFlags::GPUToCPU)},

		{"QUEUE_FAMILY_GRAPHICS_BIT",umath::to_integral(prosper::util::QueueFamilyFlags::Graphics)},
		{"QUEUE_FAMILY_COMPUTE_BIT",umath::to_integral(prosper::util::QueueFamilyFlags::Compute)},
		{"QUEUE_FAMILY_DMA_BIT",umath::to_integral(prosper::util::QueueFamilyFlags::DMA)}
	});

	auto defShaderStatisticsInfoAMD = luabind::class_<vk::ShaderStatisticsInfoAMD>("ShaderStatisticsInfoAMD");
	defShaderStatisticsInfoAMD.def_readwrite("shaderStageMask",reinterpret_cast<uint32_t vk::ShaderStatisticsInfoAMD::*>(&vk::ShaderStatisticsInfoAMD::shaderStageMask));
	defShaderStatisticsInfoAMD.def_readwrite("resourceUsage",reinterpret_cast<uint32_t vk::ShaderStatisticsInfoAMD::*>(&vk::ShaderStatisticsInfoAMD::resourceUsage));
	defShaderStatisticsInfoAMD.def_readwrite("numPhysicalVgprs",&vk::ShaderStatisticsInfoAMD::numPhysicalVgprs);
	defShaderStatisticsInfoAMD.def_readwrite("numPhysicalSgprs",&vk::ShaderStatisticsInfoAMD::numPhysicalSgprs);
	defShaderStatisticsInfoAMD.def_readwrite("numAvailableVgprs",&vk::ShaderStatisticsInfoAMD::numAvailableVgprs);
	defShaderStatisticsInfoAMD.def_readwrite("numAvailableSgprs",&vk::ShaderStatisticsInfoAMD::numAvailableSgprs);
	defShaderStatisticsInfoAMD.property("computeWorkGroupSize",static_cast<Vector3i(*)(vk::ShaderStatisticsInfoAMD&)>([](vk::ShaderStatisticsInfoAMD &stats) -> Vector3i {
		return {stats.computeWorkGroupSize[0],stats.computeWorkGroupSize[1],stats.computeWorkGroupSize[2]};
	}),static_cast<void(*)(vk::ShaderStatisticsInfoAMD&,const Vector3i&)>([](vk::ShaderStatisticsInfoAMD &stats,const Vector3i &v) {
		stats.computeWorkGroupSize[0] = v.x;
		stats.computeWorkGroupSize[1] = v.y;
		stats.computeWorkGroupSize[2] = v.z;
	}));
	vulkanMod[defShaderStatisticsInfoAMD];

	auto defShaderResourceUsageAMD = luabind::class_<vk::ShaderResourceUsageAMD>("ShaderResourceUsageAMD");
	defShaderResourceUsageAMD.def_readwrite("numUsedVgprs",&vk::ShaderResourceUsageAMD::numUsedVgprs);
	defShaderResourceUsageAMD.def_readwrite("numUsedSgprs",&vk::ShaderResourceUsageAMD::numUsedSgprs);
	defShaderResourceUsageAMD.def_readwrite("ldsSizePerLocalWorkGroup",&vk::ShaderResourceUsageAMD::ldsSizePerLocalWorkGroup);
	defShaderResourceUsageAMD.def_readwrite("ldsUsageSizeInBytes",&vk::ShaderResourceUsageAMD::ldsUsageSizeInBytes);
	defShaderResourceUsageAMD.def_readwrite("scratchMemUsageInBytes",&vk::ShaderResourceUsageAMD::scratchMemUsageInBytes);
	vulkanMod[defShaderResourceUsageAMD];

	auto defPipelineColorBlendAttachmentState = luabind::class_<vk::PipelineColorBlendAttachmentState>("PipelineColorBlendAttachmentState");
	defPipelineColorBlendAttachmentState.def(luabind::constructor<>());
	defPipelineColorBlendAttachmentState.def_readwrite("blendEnable",reinterpret_cast<bool vk::PipelineColorBlendAttachmentState::*>(&vk::PipelineColorBlendAttachmentState::blendEnable));
	defPipelineColorBlendAttachmentState.def_readwrite("srcColorBlendFactor",reinterpret_cast<uint32_t vk::PipelineColorBlendAttachmentState::*>(&vk::PipelineColorBlendAttachmentState::srcColorBlendFactor));
	defPipelineColorBlendAttachmentState.def_readwrite("dstColorBlendFactor",reinterpret_cast<uint32_t vk::PipelineColorBlendAttachmentState::*>(&vk::PipelineColorBlendAttachmentState::dstColorBlendFactor));
	defPipelineColorBlendAttachmentState.def_readwrite("colorBlendOp",reinterpret_cast<uint32_t vk::PipelineColorBlendAttachmentState::*>(&vk::PipelineColorBlendAttachmentState::colorBlendOp));
	defPipelineColorBlendAttachmentState.def_readwrite("srcAlphaBlendFactor",reinterpret_cast<uint32_t vk::PipelineColorBlendAttachmentState::*>(&vk::PipelineColorBlendAttachmentState::srcAlphaBlendFactor));
	defPipelineColorBlendAttachmentState.def_readwrite("dstAlphaBlendFactor",reinterpret_cast<uint32_t vk::PipelineColorBlendAttachmentState::*>(&vk::PipelineColorBlendAttachmentState::dstAlphaBlendFactor));
	defPipelineColorBlendAttachmentState.def_readwrite("alphaBlendOp",reinterpret_cast<uint32_t vk::PipelineColorBlendAttachmentState::*>(&vk::PipelineColorBlendAttachmentState::alphaBlendOp));
	defPipelineColorBlendAttachmentState.def_readwrite("colorWriteMask",reinterpret_cast<uint32_t vk::PipelineColorBlendAttachmentState::*>(&vk::PipelineColorBlendAttachmentState::colorWriteMask));
	vulkanMod[defPipelineColorBlendAttachmentState];

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
	defSamplerCreateInfo.def_readwrite("useUnnormalizedCoordinates",&prosper::util::SamplerCreateInfo::useUnnormalizedCoordinates);
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
		createInfo.attachments.push_back({static_cast<Anvil::Format>(format),static_cast<Anvil::ImageLayout>(initialLayout),static_cast<Anvil::AttachmentLoadOp>(loadOp),static_cast<Anvil::AttachmentStoreOp>(storeOp),static_cast<Anvil::SampleCountFlagBits>(sampleCount),static_cast<Anvil::ImageLayout>(finalLayout)});
		Lua::PushInt(l,createInfo.attachments.size() -1ull);
	}));
	defRenderPassCreateInfo.def("AddAttachment",static_cast<void(*)(lua_State*,prosper::util::RenderPassCreateInfo&,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t)>([](lua_State *l,prosper::util::RenderPassCreateInfo &createInfo,uint32_t format,uint32_t initialLayout,uint32_t finalLayout,uint32_t loadOp,uint32_t storeOp) {
		createInfo.attachments.push_back({static_cast<Anvil::Format>(format),static_cast<Anvil::ImageLayout>(initialLayout),static_cast<Anvil::AttachmentLoadOp>(loadOp),static_cast<Anvil::AttachmentStoreOp>(storeOp),Anvil::SampleCountFlagBits::_1_BIT,static_cast<Anvil::ImageLayout>(finalLayout)});
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
			static_cast<Anvil::PipelineStageFlagBits>(sourceStageMask),static_cast<Anvil::PipelineStageFlagBits>(destinationStageMask),
			static_cast<Anvil::AccessFlagBits>(sourceAccessMask),static_cast<Anvil::AccessFlagBits>(destinationAccessMask)
		});
	}));
	vulkanMod[defRenderPassCreateInfo];

	auto defImageSubresourceLayers = luabind::class_<vk::ImageSubresourceLayers>("ImageSubresourceLayers");
	defImageSubresourceLayers.def(luabind::constructor<>());
	defImageSubresourceLayers.def_readwrite("aspectMask",&vk::ImageSubresourceLayers::aspectMask);
    defImageSubresourceLayers.def_readwrite("mipLevel",&vk::ImageSubresourceLayers::mipLevel);
	defImageSubresourceLayers.def_readwrite("baseArrayLayer",&vk::ImageSubresourceLayers::baseArrayLayer);
	defImageSubresourceLayers.def_readwrite("layerCount",&vk::ImageSubresourceLayers::layerCount);
	vulkanMod[defImageSubresourceLayers];

	auto defBlitInfo = luabind::class_<prosper::util::BlitInfo>("BlitInfo");
	defBlitInfo.def(luabind::constructor<>());
	defBlitInfo.def_readwrite("srcSubresourceLayer",&prosper::util::BlitInfo::srcSubresourceLayer);
	defBlitInfo.def_readwrite("dstSubresourceLayer",&prosper::util::BlitInfo::dstSubresourceLayer);
	vulkanMod[defBlitInfo];

	static_assert(sizeof(vk::Offset3D) == sizeof(Vector3i));
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

	auto defBufferCopyInfo = luabind::class_<Anvil::BufferCopy>("BufferCopyInfo");
	defBufferCopyInfo.def(luabind::constructor<>());
	defBufferCopyInfo.def_readwrite("srcOffset",&Anvil::BufferCopy::src_offset);
	defBufferCopyInfo.def_readwrite("dstOffset",&Anvil::BufferCopy::dst_offset);
	defBufferCopyInfo.def_readwrite("size",&Anvil::BufferCopy::size);
	vulkanMod[defBufferCopyInfo];

	auto defBufferImageCopyInfo = luabind::class_<prosper::util::BufferImageCopyInfo>("BufferImageCopyInfo");
	defBufferImageCopyInfo.def(luabind::constructor<>());
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
	defVkTexture.def("GetImage",&Lua::Vulkan::VKTexture::GetImage);
	defVkTexture.def("GetImageView",static_cast<void(*)(lua_State*,Lua::Vulkan::Texture&,uint32_t)>(&Lua::Vulkan::VKTexture::GetImageView));
	defVkTexture.def("GetImageView",static_cast<void(*)(lua_State*,Lua::Vulkan::Texture&)>(&Lua::Vulkan::VKTexture::GetImageView));
	defVkTexture.def("GetSampler",&Lua::Vulkan::VKTexture::GetSampler);
	defVkTexture.def("GetWidth",&Lua::Vulkan::VKTexture::GetWidth);
	defVkTexture.def("GetHeight",&Lua::Vulkan::VKTexture::GetHeight);
	defVkTexture.def("GetFormat",&Lua::Vulkan::VKTexture::GetFormat);
	defVkTexture.def("IsValid",&Lua::Vulkan::VKTexture::IsValid);
	defVkTexture.def("SetDebugName",static_cast<void(*)(lua_State*,Lua::Vulkan::Texture&,const std::string&)>([](lua_State *l,Lua::Vulkan::Texture &tex,const std::string &name) {
		Lua::Vulkan::VKContextObject::SetDebugName<Lua::Vulkan::Texture>(l,tex,name,&Lua::CheckVKTexture);
	}));
	defVkTexture.def("GetDebugName",static_cast<void(*)(lua_State*,Lua::Vulkan::Texture&)>([](lua_State *l,Lua::Vulkan::Texture &tex) {
		Lua::Vulkan::VKContextObject::GetDebugName<Lua::Vulkan::Texture>(l,tex,&Lua::CheckVKTexture);
	}));
	//
	vulkanMod[defVkTexture];

	auto defVkImage = luabind::class_<Lua::Vulkan::Image>("Image");
	defVkImage.def(luabind::tostring(luabind::self));
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
	defVkImage.def("GetExtent3D",&Lua::Vulkan::VKImage::GetExtent3D);
	defVkImage.def("GetFormat",&Lua::Vulkan::VKImage::GetFormat);
	defVkImage.def("GetMemoryTypes",&Lua::Vulkan::VKImage::GetMemoryTypes);
	defVkImage.def("GetMipmapSize",&Lua::Vulkan::VKImage::GetMipmapSize);
	defVkImage.def("GetLayerCount",&Lua::Vulkan::VKImage::GetLayerCount);
	defVkImage.def("GetMipmapCount",&Lua::Vulkan::VKImage::GetMipmapCount);
	defVkImage.def("GetSampleCount",&Lua::Vulkan::VKImage::GetSampleCount);
	defVkImage.def("GetSharingMode",&Lua::Vulkan::VKImage::GetSharingMode);
	defVkImage.def("GetStorageSize",&Lua::Vulkan::VKImage::GetStorageSize);
	defVkImage.def("GetTiling",&Lua::Vulkan::VKImage::GetTiling);
	defVkImage.def("GetType",&Lua::Vulkan::VKImage::GetType);
	defVkImage.def("GetUsage",&Lua::Vulkan::VKImage::GetUsage);
	defVkImage.def("GetMemoryBlock",&Lua::Vulkan::VKImage::GetMemoryBlock);
	//defVkImage.def("GetMemoryRequirements",&Lua::Vulkan::VKImage::GetMemoryRequirements);
	defVkImage.def("GetSubresourceRange",&Lua::Vulkan::VKImage::GetSubresourceRange);
	//defVkImage.def("GetParentSwapchain",&Lua::Vulkan::VKImage::GetParentSwapchain);
	defVkImage.def("GetWidth",&Lua::Vulkan::VKImage::GetWidth);
	defVkImage.def("GetHeight",&Lua::Vulkan::VKImage::GetHeight);
	defVkImage.def("SetDebugName",static_cast<void(*)(lua_State*,Lua::Vulkan::Image&,const std::string&)>([](lua_State *l,Lua::Vulkan::Image &img,const std::string &name) {
		Lua::Vulkan::VKContextObject::SetDebugName<Lua::Vulkan::Image>(l,img,name,&Lua::CheckVKImage);
	}));
	defVkImage.def("GetDebugName",static_cast<void(*)(lua_State*,Lua::Vulkan::Image&)>([](lua_State *l,Lua::Vulkan::Image &img) {
		Lua::Vulkan::VKContextObject::GetDebugName<Lua::Vulkan::Image>(l,img,&Lua::CheckVKImage);
	}));
	vulkanMod[defVkImage];

	auto debSubresourceLayout = luabind::class_<vk::SubresourceLayout>("SubresourceLayout");
	debSubresourceLayout.def(luabind::constructor<>());
	debSubresourceLayout.def_readwrite("offset",&vk::SubresourceLayout::offset);
	debSubresourceLayout.def_readwrite("size",&vk::SubresourceLayout::size);
	debSubresourceLayout.def_readwrite("rowPitch",&vk::SubresourceLayout::rowPitch);
	debSubresourceLayout.def_readwrite("arrayPitch",&vk::SubresourceLayout::arrayPitch);
	debSubresourceLayout.def_readwrite("depthPitch",&vk::SubresourceLayout::depthPitch);
	vulkanMod[debSubresourceLayout];

	auto defVkImageView = luabind::class_<Lua::Vulkan::ImageView>("ImageView");
	defVkImageView.def(luabind::tostring(luabind::self));
	defVkImageView.def("IsValid",&Lua::Vulkan::VKImageView::IsValid);
	defVkImageView.def("GetAspectMask",&Lua::Vulkan::VKImageView::GetAspectMask);
	defVkImageView.def("GetBaseLayer",&Lua::Vulkan::VKImageView::GetBaseLayer);
	defVkImageView.def("GetBaseMipmapLevel",&Lua::Vulkan::VKImageView::GetBaseMipmapLevel);
	defVkImageView.def("GetBaseMipmapSize",&Lua::Vulkan::VKImageView::GetBaseMipmapSize);
	defVkImageView.def("GetImageFormat",&Lua::Vulkan::VKImageView::GetImageFormat);
	defVkImageView.def("GetLayerCount",&Lua::Vulkan::VKImageView::GetLayerCount);
	defVkImageView.def("GetMipmapCount",&Lua::Vulkan::VKImageView::GetMipmapCount);
	//defVkImageView.def("GetParentImage",&Lua::Vulkan::VKImageView::GetParentImage);
	defVkImageView.def("GetSubresourceRange",&Lua::Vulkan::VKImageView::GetSubresourceRange);
	defVkImageView.def("GetSwizzleArray",&Lua::Vulkan::VKImageView::GetSwizzleArray);
	defVkImageView.def("GetType",&Lua::Vulkan::VKImageView::GetType);
	defVkImageView.def("SetDebugName",static_cast<void(*)(lua_State*,Lua::Vulkan::ImageView&,const std::string&)>([](lua_State *l,Lua::Vulkan::ImageView &imgView,const std::string &name) {
		Lua::Vulkan::VKContextObject::SetDebugName<Lua::Vulkan::ImageView>(l,imgView,name,&Lua::CheckVKImageView);
	}));
	defVkImageView.def("GetDebugName",static_cast<void(*)(lua_State*,Lua::Vulkan::ImageView&)>([](lua_State *l,Lua::Vulkan::ImageView &imgView) {
		Lua::Vulkan::VKContextObject::GetDebugName<Lua::Vulkan::ImageView>(l,imgView,&Lua::CheckVKImageView);
	}));
	vulkanMod[defVkImageView];

	auto defVkSampler = luabind::class_<Lua::Vulkan::Sampler>("Sampler");
	defVkSampler.def(luabind::tostring(luabind::self));
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
	defVkSampler.def("GetUnnormalizedCoordinates",&Lua::Vulkan::VKSampler::GetUseUnnormalizedCoordinates);
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
	defVkSampler.def("SetUnnormalizedCoordinates",&Lua::Vulkan::VKSampler::SetUseUnnormalizedCoordinates);
	defVkSampler.def("SetDebugName",static_cast<void(*)(lua_State*,Lua::Vulkan::Sampler&,const std::string&)>([](lua_State *l,Lua::Vulkan::Sampler &smp,const std::string &name) {
		Lua::Vulkan::VKContextObject::SetDebugName<Lua::Vulkan::Sampler>(l,smp,name,&Lua::CheckVKSampler);
	}));
	defVkSampler.def("GetDebugName",static_cast<void(*)(lua_State*,Lua::Vulkan::Sampler&)>([](lua_State *l,Lua::Vulkan::Sampler &smp) {
		Lua::Vulkan::VKContextObject::GetDebugName<Lua::Vulkan::Sampler>(l,smp,&Lua::CheckVKSampler);
	}));
	vulkanMod[defVkSampler];

	auto defVkFramebuffer = luabind::class_<Lua::Vulkan::Framebuffer>("Framebuffer");
	defVkFramebuffer.def(luabind::tostring(luabind::self));
	defVkFramebuffer.def("IsValid",&Lua::Vulkan::VKFramebuffer::IsValid);
	vulkanMod[defVkFramebuffer];
	
	auto defVkRenderPass = luabind::class_<Lua::Vulkan::RenderPass>("RenderPass");
	defVkRenderPass.def(luabind::tostring(luabind::self));
	defVkRenderPass.def("IsValid",&Lua::Vulkan::VKRenderPass::IsValid);
	vulkanMod[defVkRenderPass];
	
	auto defVkEvent = luabind::class_<Lua::Vulkan::Event>("Event");
	defVkEvent.def(luabind::tostring(luabind::self));
	defVkEvent.def("IsValid",&Lua::Vulkan::VKEvent::IsValid);
	vulkanMod[defVkEvent];
	
	auto defVkFence = luabind::class_<Lua::Vulkan::Fence>("Fence");
	defVkFence.def(luabind::tostring(luabind::self));
	defVkFence.def("IsValid",&Lua::Vulkan::VKFence::IsValid);
	vulkanMod[defVkFence];
	
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
	
	auto defVkCommandBuffer = luabind::class_<Lua::Vulkan::CommandBuffer>("CommandBuffer");
	defVkCommandBuffer.def(luabind::tostring(luabind::self));
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
	defVkCommandBuffer.def("RecordBindVertexBuffer",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,Lua::Vulkan::Buffer&,uint32_t)>([](lua_State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer,Lua::Vulkan::Buffer &vertexBuffer,uint32_t startBinding) {
		Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffer(l,hCommandBuffer,vertexBuffer,startBinding,0u);
	}));
	defVkCommandBuffer.def("RecordBindVertexBuffer",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,Lua::Vulkan::Buffer&)>([](lua_State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer,Lua::Vulkan::Buffer &vertexBuffer) {
		Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffer(l,hCommandBuffer,vertexBuffer,0u,0u);
	}));
	defVkCommandBuffer.def("RecordBindVertexBuffers",&Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffers);
	defVkCommandBuffer.def("RecordBindVertexBuffers",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,luabind::object,uint32_t)>([](lua_State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer,luabind::object vertexBuffers,uint32_t startBinding) {
		Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffers(l,hCommandBuffer,vertexBuffers,startBinding,{});
	}));
	defVkCommandBuffer.def("RecordBindVertexBuffers",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,luabind::object)>([](lua_State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer,luabind::object vertexBuffers) {
		Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffers(l,hCommandBuffer,vertexBuffers,0u,{});
	}));
	defVkCommandBuffer.def("RecordCopyImageToBuffer",&Lua::Vulkan::VKCommandBuffer::RecordCopyImageToBuffer);
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
	defVkCommandBuffer.def("RecordDrawIndexed",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,uint32_t,uint32_t,uint32_t,uint32_t)>([](lua_State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer,uint32_t indexCount,uint32_t instanceCount,uint32_t firstIndex,uint32_t vertexOffset) {
		Lua::Vulkan::VKCommandBuffer::RecordDrawIndexed(l,hCommandBuffer,indexCount,instanceCount,firstIndex,vertexOffset,0u);
	}));
	defVkCommandBuffer.def("RecordDrawIndexed",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,uint32_t,uint32_t,uint32_t)>([](lua_State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer,uint32_t indexCount,uint32_t instanceCount,uint32_t firstIndex) {
		Lua::Vulkan::VKCommandBuffer::RecordDrawIndexed(l,hCommandBuffer,indexCount,instanceCount,firstIndex,0u,0u);
	}));
	defVkCommandBuffer.def("RecordDrawIndexed",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,uint32_t,uint32_t)>([](lua_State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer,uint32_t indexCount,uint32_t instanceCount) {
		Lua::Vulkan::VKCommandBuffer::RecordDrawIndexed(l,hCommandBuffer,indexCount,instanceCount,0u,0u,0u);
	}));
	defVkCommandBuffer.def("RecordDrawIndexed",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&,uint32_t)>([](lua_State *l,Lua::Vulkan::CommandBuffer &hCommandBuffer,uint32_t indexCount) {
		Lua::Vulkan::VKCommandBuffer::RecordDrawIndexed(l,hCommandBuffer,indexCount,1u,0u,0u,0u);
	}));
	defVkCommandBuffer.def("RecordDrawIndexedIndirect",&Lua::Vulkan::VKCommandBuffer::RecordDrawIndexedIndirect);
	defVkCommandBuffer.def("RecordDrawIndirect",&Lua::Vulkan::VKCommandBuffer::RecordDrawIndirect);
	defVkCommandBuffer.def("RecordFillBuffer",&Lua::Vulkan::VKCommandBuffer::RecordFillBuffer);
	defVkCommandBuffer.def("RecordResetEvent",&Lua::Vulkan::VKCommandBuffer::RecordResetEvent);
	defVkCommandBuffer.def("RecordSetBlendConstants",&Lua::Vulkan::VKCommandBuffer::RecordSetBlendConstants);
	defVkCommandBuffer.def("RecordSetDepthBias",&Lua::Vulkan::VKCommandBuffer::RecordSetDepthBias);
	defVkCommandBuffer.def("RecordSetDepthBounds",&Lua::Vulkan::VKCommandBuffer::RecordSetDepthBounds);
	defVkCommandBuffer.def("RecordSetEvent",&Lua::Vulkan::VKCommandBuffer::RecordSetEvent);
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
		Lua::Vulkan::VKContextObject::SetDebugName<Lua::Vulkan::CommandBuffer>(l,cb,name,&Lua::CheckVKCommandBuffer);
	}));
	defVkCommandBuffer.def("GetDebugName",static_cast<void(*)(lua_State*,Lua::Vulkan::CommandBuffer&)>([](lua_State *l,Lua::Vulkan::CommandBuffer &cb) {
		Lua::Vulkan::VKContextObject::GetDebugName<Lua::Vulkan::CommandBuffer>(l,cb,&Lua::CheckVKCommandBuffer);
	}));
	vulkanMod[defVkCommandBuffer];

	auto devVkBuffer = luabind::class_<Lua::Vulkan::Buffer>("Buffer");
	devVkBuffer.def(luabind::tostring(luabind::self));
	devVkBuffer.def("IsValid",&Lua::Vulkan::VKBuffer::IsValid);
	devVkBuffer.def("GetStartOffset",&Lua::Vulkan::VKBuffer::GetStartOffset);
	devVkBuffer.def("GetBaseIndex",&Lua::Vulkan::VKBuffer::GetBaseIndex);
	devVkBuffer.def("GetSize",&Lua::Vulkan::VKBuffer::GetSize);
	devVkBuffer.def("GetUsageFlags",&Lua::Vulkan::VKBuffer::GetUsageFlags);
	devVkBuffer.def("SetDebugName",static_cast<void(*)(lua_State*,Lua::Vulkan::Buffer&,const std::string&)>([](lua_State *l,Lua::Vulkan::Buffer &buf,const std::string &name) {
		Lua::Vulkan::VKContextObject::SetDebugName<Lua::Vulkan::Buffer>(l,buf,name,&Lua::CheckVKBuffer);
	}));
	devVkBuffer.def("GetDebugName",static_cast<void(*)(lua_State*,Lua::Vulkan::Buffer&)>([](lua_State *l,Lua::Vulkan::Buffer &buf) {
		Lua::Vulkan::VKContextObject::GetDebugName<Lua::Vulkan::Buffer>(l,buf,&Lua::CheckVKBuffer);
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
		Lua::Vulkan::VKBuffer::Read(l,hBuffer,0u,hBuffer->GetSize());
	}));
	devVkBuffer.def("MapMemory",&Lua::Vulkan::VKBuffer::Map);
	devVkBuffer.def("MapMemory",static_cast<void(*)(lua_State*,Lua::Vulkan::Buffer&)>([](lua_State *l,Lua::Vulkan::Buffer &hBuffer) {
		Lua::Vulkan::VKBuffer::Map(l,hBuffer,0u,hBuffer->GetSize());
	}));
	devVkBuffer.def("UnmapMemory",&Lua::Vulkan::VKBuffer::Unmap);
	vulkanMod[devVkBuffer];

	auto defVkDescriptorSet = luabind::class_<Lua::Vulkan::DescriptorSet>("DescriptorSet");
	defVkDescriptorSet.def(luabind::tostring(luabind::self));
	defVkDescriptorSet.def("GetBindingInfo",static_cast<void(*)(lua_State*,Lua::Vulkan::DescriptorSet&,uint32_t)>(&Lua::Vulkan::VKDescriptorSet::GetBindingInfo));
	defVkDescriptorSet.def("GetBindingInfo",static_cast<void(*)(lua_State*,Lua::Vulkan::DescriptorSet&)>(&Lua::Vulkan::VKDescriptorSet::GetBindingInfo));
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
		Lua::Vulkan::VKContextObject::SetDebugName<Lua::Vulkan::DescriptorSet>(l,ds,name,&Lua::CheckVKDescriptorSet);
	}));
	defVkDescriptorSet.def("GetDebugName",static_cast<void(*)(lua_State*,Lua::Vulkan::DescriptorSet&)>([](lua_State *l,Lua::Vulkan::DescriptorSet &ds) {
		Lua::Vulkan::VKContextObject::GetDebugName<Lua::Vulkan::DescriptorSet>(l,ds,&Lua::CheckVKDescriptorSet);
	}));
	vulkanMod[defVkDescriptorSet];
	
	auto defVkMesh = luabind::class_<std::shared_ptr<pragma::VkMesh>>("Mesh");
	defVkMesh.def(luabind::tostring(luabind::self));
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
		Lua::Vulkan::VKContextObject::SetDebugName<Lua::Vulkan::RenderTarget>(l,rt,name,&Lua::CheckVKRenderTarget);
	}));
	defVkRenderTarget.def("GetDebugName",static_cast<void(*)(lua_State*,Lua::Vulkan::RenderTarget&)>([](lua_State *l,Lua::Vulkan::RenderTarget &rt) {
		Lua::Vulkan::VKContextObject::GetDebugName<Lua::Vulkan::RenderTarget>(l,rt,&Lua::CheckVKRenderTarget);
	}));
	vulkanMod[defVkRenderTarget];
	
	auto defVkTimestampQuery = luabind::class_<Lua::Vulkan::TimestampQuery>("TimestampQuery");
	defVkTimestampQuery.def(luabind::tostring(luabind::self));
	defVkTimestampQuery.def("IsValid",&Lua::Vulkan::VKTimestampQuery::IsValid);
	vulkanMod[defVkTimestampQuery];
	
	auto defVkTimerQuery = luabind::class_<Lua::Vulkan::TimerQuery>("TimerQuery");
	defVkTimerQuery.def(luabind::tostring(luabind::self));
	defVkTimerQuery.def("IsValid",&Lua::Vulkan::VKTimerQuery::IsValid);
	vulkanMod[defVkTimerQuery];

	auto defClearValue = luabind::class_<Lua::Vulkan::ClearValue>("ClearValue");
	defClearValue.def(luabind::constructor<>());
	defClearValue.def(luabind::constructor<const Color&>());
	defClearValue.def(luabind::constructor<float,uint32_t>());
	defClearValue.def(luabind::constructor<float>());
	defClearValue.def("SetColor",static_cast<void(*)(lua_State*,Lua::Vulkan::ClearValue&,const Color&)>([](lua_State *l,Lua::Vulkan::ClearValue &clearValue,const Color &clearColor) {
		clearValue.clearValue.setColor(vk::ClearColorValue{std::array<float,4>{clearColor.r /255.f,clearColor.g /255.f,clearColor.b /255.f,clearColor.a /255.f}});
	}));
	defClearValue.def("SetDepthStencil",static_cast<void(*)(lua_State*,Lua::Vulkan::ClearValue&,float)>([](lua_State *l,Lua::Vulkan::ClearValue &clearValue,float depth) {
		clearValue.clearValue.setDepthStencil(vk::ClearDepthStencilValue{depth});
	}));
	defClearValue.def("SetDepthStencil",static_cast<void(*)(lua_State*,Lua::Vulkan::ClearValue&,float,uint32_t)>([](lua_State *l,Lua::Vulkan::ClearValue &clearValue,float depth,uint32_t stencil) {
		clearValue.clearValue.setDepthStencil(vk::ClearDepthStencilValue{depth,stencil});
	}));
	vulkanMod[defClearValue];

	auto defRenderPassInfo = luabind::class_<Lua::Vulkan::RenderPassInfo>("RenderPassInfo");
	defRenderPassInfo.def(luabind::constructor<const Lua::Vulkan::RenderTarget&>());
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
		rpInfo.renderPass = rp;
	}));
	vulkanMod[defRenderPassInfo];
}

/////////////////////////////////

void Lua::Vulkan::VKTexture::IsValid(lua_State *l,Texture &hTex)
{
	Lua::PushBool(l,hTex != nullptr);
}
void Lua::Vulkan::VKTexture::GetImage(lua_State *l,Texture &hTex)
{
	Lua::CheckVKTexture(l,1);
	auto &img = hTex->GetImage();
	Lua::Push<Image>(l,img);
}
void Lua::Vulkan::VKTexture::GetImageView(lua_State *l,Texture &hTex)
{
	Lua::CheckVKTexture(l,1);
	auto &imgView = hTex->GetImageView();
	if(imgView != nullptr)
		Lua::Push<ImageView>(l,imgView);
}
void Lua::Vulkan::VKTexture::GetImageView(lua_State *l,Texture &hTex,uint32_t layerId)
{
	Lua::CheckVKTexture(l,1);
	auto &imgView = hTex->GetImageView(layerId);
	if(imgView != nullptr)
		Lua::Push<ImageView>(l,imgView);
}
void Lua::Vulkan::VKTexture::GetSampler(lua_State *l,Texture &hTex)
{
	Lua::CheckVKTexture(l,1);
	auto &sampler = hTex->GetSampler();
	if(sampler != nullptr)
		Lua::Push<Sampler>(l,sampler);
}
void Lua::Vulkan::VKTexture::GetWidth(lua_State *l,Texture &hTex)
{
	Lua::CheckVKTexture(l,1);
	auto &img = hTex->GetImage();
	auto extents = img->GetExtents();
	Lua::PushInt(l,extents.width);
}
void Lua::Vulkan::VKTexture::GetHeight(lua_State *l,Texture &hTex)
{
	Lua::CheckVKTexture(l,1);
	auto &img = hTex->GetImage();
	auto extents = img->GetExtents();
	Lua::PushInt(l,extents.height);
}
void Lua::Vulkan::VKTexture::GetFormat(lua_State *l,Texture &hTex)
{
	Lua::CheckVKTexture(l,1);
	auto &img = hTex->GetImage();
	Lua::PushInt(l,umath::to_integral(img->GetFormat()));
}
void Lua::Vulkan::VKTexture::IsMSAATexture(lua_State *l,Texture &hTex)
{
	Lua::CheckVKTexture(l,1);
	Lua::PushBool(l,hTex->IsMSAATexture());
}

/////////////////////////////////

void Lua::Vulkan::VKImage::IsValid(lua_State *l,Image &hImg)
{
	Lua::PushBool(l,hImg != nullptr);
}
void Lua::Vulkan::VKImage::GetAspectSubresourceLayout(lua_State *l,Image &hImg,uint32_t layer,uint32_t mipmap)
{
	Lua::CheckVKImage(l,1);
	Anvil::SubresourceLayout layout;
	auto r = (*hImg)->get_aspect_subresource_layout(
		prosper::util::get_aspect_mask(hImg->GetFormat()),
		layer,mipmap,&layout
	);
	Lua::PushBool(l,r);
	if(r == false)
		return;
	Lua::Push(l,layout);
}
void Lua::Vulkan::VKImage::GetAlignment(lua_State *l,Image &hImg)
{
	Lua::CheckVKImage(l,1);
	Lua::PushInt(l,(*hImg)->get_image_alignment());
}
void Lua::Vulkan::VKImage::GetExtent2D(lua_State *l,Image &hImg,uint32_t mipmap)
{
	Lua::CheckVKImage(l,1);
	auto extents = hImg->GetExtents(mipmap);
	Lua::PushInt(l,extents.width);
	Lua::PushInt(l,extents.height);
}
void Lua::Vulkan::VKImage::GetExtent3D(lua_State *l,Image &hImg,uint32_t mipmap)
{
	Lua::CheckVKImage(l,1);
	auto extents = (*hImg)->get_image_extent_3D(mipmap);
	Lua::PushInt(l,extents.width);
	Lua::PushInt(l,extents.height);
	Lua::PushInt(l,extents.depth);
}
void Lua::Vulkan::VKImage::GetFormat(lua_State *l,Image &hImg)
{
	Lua::CheckVKImage(l,1);
	Lua::PushInt(l,static_cast<int32_t>(hImg->GetFormat()));
}
void Lua::Vulkan::VKImage::GetMemoryTypes(lua_State *l,Image &hImg)
{
	Lua::CheckVKImage(l,1);
	Lua::PushInt(l,(*hImg)->get_image_memory_types());
}
void Lua::Vulkan::VKImage::GetMipmapSize(lua_State *l,Image &hImg,uint32_t mipmap)
{
	Lua::CheckVKImage(l,1);
	uint32_t width;
	uint32_t height;
	uint32_t depth;
	auto r = (*hImg)->get_image_mipmap_size(mipmap,&width,&height,&depth);
	if(r == false)
		return;
	Lua::PushInt(l,width);
	Lua::PushInt(l,height);
	Lua::PushInt(l,depth);
}
void Lua::Vulkan::VKImage::GetLayerCount(lua_State *l,Image &hImg)
{
	Lua::CheckVKImage(l,1);
	Lua::PushInt(l,hImg->GetLayerCount());
}
void Lua::Vulkan::VKImage::GetMipmapCount(lua_State *l,Image &hImg)
{
	Lua::CheckVKImage(l,1);
	Lua::PushInt(l,hImg->GetMipmapCount());
}
void Lua::Vulkan::VKImage::GetSampleCount(lua_State *l,Image &hImg)
{
	Lua::CheckVKImage(l,1);
	Lua::PushInt(l,umath::to_integral(hImg->GetSampleCount()));
}
void Lua::Vulkan::VKImage::GetSharingMode(lua_State *l,Image &hImg)
{
	Lua::CheckVKImage(l,1);
	Lua::PushInt(l,umath::to_integral(hImg->GetSharingMode()));
}
void Lua::Vulkan::VKImage::GetStorageSize(lua_State *l,Image &hImg)
{
	Lua::CheckVKImage(l,1);
	Lua::PushInt(l,(*hImg)->get_image_storage_size());
}
void Lua::Vulkan::VKImage::GetTiling(lua_State *l,Image &hImg)
{
	Lua::CheckVKImage(l,1);
	Lua::PushInt(l,static_cast<uint32_t>(hImg->GetTiling()));
}
void Lua::Vulkan::VKImage::GetType(lua_State *l,Image &hImg)
{
	Lua::CheckVKImage(l,1);
	Lua::PushInt(l,static_cast<uint32_t>(hImg->GetType()));
}
void Lua::Vulkan::VKImage::GetUsage(lua_State *l,Image &hImg)
{
	Lua::CheckVKImage(l,1);
	Lua::PushInt(l,static_cast<uint32_t>(hImg->GetUsageFlags().get_vk()));
}
void Lua::Vulkan::VKImage::GetMemoryBlock(lua_State *l,Image &hImg)
{
	Lua::CheckVKImage(l,1);
	Lua::Push<Memory*>(l,(*hImg)->get_memory_block());
}
void Lua::Vulkan::VKImage::GetMemoryRequirements(lua_State *l,Image &hImg)
{
	Lua::CheckVKImage(l,1);
	(*hImg)->get_memory_requirements(); // prosper TODO
}
void Lua::Vulkan::VKImage::GetParentSwapchain(lua_State *l,Image &hImg)
{
	Lua::CheckVKImage(l,1);
	(*hImg)->get_create_info_ptr()->get_swapchain(); // prosper TODO
}
void Lua::Vulkan::VKImage::GetSubresourceRange(lua_State *l,Image &hImg)
{
	Lua::CheckVKImage(l,1);
	auto subresourceRange = (*hImg)->get_subresource_range();
	Lua::Push(l,reinterpret_cast<vk::ImageSubresourceRange&>(subresourceRange));
}
void Lua::Vulkan::VKImage::GetWidth(lua_State *l,Image &hImg)
{
	Lua::CheckVKImage(l,1);
	auto extents = hImg->GetExtents();
	Lua::PushInt(l,extents.width);
}
void Lua::Vulkan::VKImage::GetHeight(lua_State *l,Image &hImg)
{
	Lua::CheckVKImage(l,1);
	auto extents = (*hImg)->get_image_extent_3D(0u);
	Lua::PushInt(l,extents.height);
}

/////////////////////////////////

void Lua::Vulkan::VKImageView::IsValid(lua_State *l,ImageView &hImgView)
{
	Lua::PushBool(l,hImgView != nullptr);
}
void Lua::Vulkan::VKImageView::GetAspectMask(lua_State *l,ImageView &hImgView)
{
	Lua::CheckVKImageView(l,1);
	Lua::PushInt(l,static_cast<uint32_t>(hImgView->GetAspectMask().get_vk()));
}
void Lua::Vulkan::VKImageView::GetBaseLayer(lua_State *l,ImageView &hImgView)
{
	Lua::CheckVKImageView(l,1);
	Lua::PushInt(l,hImgView->GetBaseLayer());
}
void Lua::Vulkan::VKImageView::GetBaseMipmapLevel(lua_State *l,ImageView &hImgView)
{
	Lua::CheckVKImageView(l,1);
	Lua::PushInt(l,hImgView->GetBaseMipmapLevel());
}
void Lua::Vulkan::VKImageView::GetBaseMipmapSize(lua_State *l,ImageView &hImgView)
{
	Lua::CheckVKImageView(l,1);
	auto width = 0u;
	auto height = 0u;
	auto depth = 0u;
	(*hImgView)->get_base_mipmap_size(&width,&height,&depth);
	Lua::PushInt(l,width);
	Lua::PushInt(l,height);
	Lua::PushInt(l,depth);
}
void Lua::Vulkan::VKImageView::GetImageFormat(lua_State *l,ImageView &hImgView)
{
	Lua::CheckVKImageView(l,1);
	Lua::PushInt(l,hImgView->GetFormat());
}
void Lua::Vulkan::VKImageView::GetLayerCount(lua_State *l,ImageView &hImgView)
{
	Lua::CheckVKImageView(l,1);
	Lua::PushInt(l,hImgView->GetLayerCount());
}
void Lua::Vulkan::VKImageView::GetMipmapCount(lua_State *l,ImageView &hImgView)
{
	Lua::CheckVKImageView(l,1);
	Lua::PushInt(l,hImgView->GetMipmapCount());
}
void Lua::Vulkan::VKImageView::GetParentImage(lua_State *l,ImageView &hImgView)
{
	Lua::CheckVKImageView(l,1);
	auto *img = (*hImgView)->get_create_info_ptr()->get_parent_image();
	if(img == nullptr)
		return;
	// prosper TODO: get_shared_from_this()?
}
void Lua::Vulkan::VKImageView::GetSubresourceRange(lua_State *l,ImageView &hImgView)
{
	Lua::CheckVKImageView(l,1);
	auto subresourceRange = (*hImgView)->get_subresource_range();
	Lua::Push(l,reinterpret_cast<vk::ImageSubresourceRange&>(subresourceRange));
}
void Lua::Vulkan::VKImageView::GetSwizzleArray(lua_State *l,ImageView &hImgView)
{
	Lua::CheckVKImageView(l,1);
	auto swizzle = hImgView->GetSwizzleArray();
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
	Lua::CheckVKImageView(l,1);
	Lua::PushInt(l,static_cast<uint32_t>(hImgView->GetType()));
}

/////////////////////////////////

void Lua::Vulkan::VKSampler::IsValid(lua_State *l,Sampler &hSampler)
{
	Lua::PushBool(l,hSampler != nullptr);
}
void Lua::Vulkan::VKSampler::Update(lua_State *l,Sampler &hSampler)
{
	Lua::CheckVKSampler(l,1);
	hSampler->Update();
}
void Lua::Vulkan::VKSampler::GetMagFilter(lua_State *l,Sampler &hSampler)
{
	Lua::CheckVKSampler(l,1);
	Lua::PushInt(l,static_cast<uint32_t>(hSampler->GetMagFilter()));
}
void Lua::Vulkan::VKSampler::GetMinFilter(lua_State *l,Sampler &hSampler)
{
	Lua::CheckVKSampler(l,1);
	Lua::PushInt(l,static_cast<uint32_t>(hSampler->GetMinFilter()));
}
void Lua::Vulkan::VKSampler::GetMipmapMode(lua_State *l,Sampler &hSampler)
{
	Lua::CheckVKSampler(l,1);
	Lua::PushInt(l,static_cast<uint32_t>(hSampler->GetMipmapMode()));
}
void Lua::Vulkan::VKSampler::GetAddressModeU(lua_State *l,Sampler &hSampler)
{
	Lua::CheckVKSampler(l,1);
	Lua::PushInt(l,static_cast<uint32_t>(hSampler->GetAddressModeU()));
}
void Lua::Vulkan::VKSampler::GetAddressModeV(lua_State *l,Sampler &hSampler)
{
	Lua::CheckVKSampler(l,1);
	Lua::PushInt(l,static_cast<uint32_t>(hSampler->GetAddressModeV()));
}
void Lua::Vulkan::VKSampler::GetAddressModeW(lua_State *l,Sampler &hSampler)
{
	Lua::CheckVKSampler(l,1);
	Lua::PushInt(l,static_cast<uint32_t>(hSampler->GetAddressModeW()));
}
void Lua::Vulkan::VKSampler::GetMipLodBias(lua_State *l,Sampler &hSampler)
{
	Lua::CheckVKSampler(l,1);
	Lua::PushNumber(l,hSampler->GetLodBias());
}
void Lua::Vulkan::VKSampler::GetAnisotropyEnabled(lua_State *l,Sampler &hSampler)
{
	Lua::CheckVKSampler(l,1);
	Lua::PushBool(l,hSampler->GetMaxAnisotropy() != 0.f);
}
void Lua::Vulkan::VKSampler::GetMaxAnisotropy(lua_State *l,Sampler &hSampler)
{
	Lua::CheckVKSampler(l,1);
	Lua::PushNumber(l,hSampler->GetMaxAnisotropy());
}
void Lua::Vulkan::VKSampler::GetCompareEnabled(lua_State *l,Sampler &hSampler)
{
	Lua::CheckVKSampler(l,1);
	Lua::PushBool(l,hSampler->GetCompareEnabled());
}
void Lua::Vulkan::VKSampler::GetCompareOperation(lua_State *l,Sampler &hSampler)
{
	Lua::CheckVKSampler(l,1);
	Lua::PushInt(l,static_cast<uint32_t>(hSampler->GetCompareOp()));
}
void Lua::Vulkan::VKSampler::GetMinLod(lua_State *l,Sampler &hSampler)
{
	Lua::CheckVKSampler(l,1);
	Lua::PushNumber(l,hSampler->GetMinLod());
}
void Lua::Vulkan::VKSampler::GetMaxLod(lua_State *l,Sampler &hSampler)
{
	Lua::CheckVKSampler(l,1);
	Lua::PushNumber(l,hSampler->GetMaxLod());
}
void Lua::Vulkan::VKSampler::GetBorderColor(lua_State *l,Sampler &hSampler)
{
	Lua::CheckVKSampler(l,1);
	Lua::PushInt(l,static_cast<uint32_t>(hSampler->GetBorderColor()));
}
void Lua::Vulkan::VKSampler::GetUseUnnormalizedCoordinates(lua_State *l,Sampler &hSampler)
{
	Lua::CheckVKSampler(l,1);
	Lua::PushBool(l,hSampler->GetUseUnnormalizedCoordinates());
}
void Lua::Vulkan::VKSampler::SetMagFilter(lua_State *l,Sampler &hSampler,int32_t magFilter)
{
	Lua::CheckVKSampler(l,1);
	hSampler->SetMagFilter(static_cast<Anvil::Filter>(magFilter));
}
void Lua::Vulkan::VKSampler::SetMinFilter(lua_State *l,Sampler &hSampler,int32_t minFilter)
{
	Lua::CheckVKSampler(l,1);
	hSampler->SetMinFilter(static_cast<Anvil::Filter>(minFilter));
}
void Lua::Vulkan::VKSampler::SetMipmapMode(lua_State *l,Sampler &hSampler,int32_t mipmapMode)
{
	Lua::CheckVKSampler(l,1);
	hSampler->SetMipmapMode(static_cast<Anvil::SamplerMipmapMode>(mipmapMode));
}
void Lua::Vulkan::VKSampler::SetAddressModeU(lua_State *l,Sampler &hSampler,int32_t addressModeU)
{
	Lua::CheckVKSampler(l,1);
	hSampler->SetAddressModeU(static_cast<Anvil::SamplerAddressMode>(addressModeU));
}
void Lua::Vulkan::VKSampler::SetAddressModeV(lua_State *l,Sampler &hSampler,int32_t addressModeV)
{
	Lua::CheckVKSampler(l,1);
	hSampler->SetAddressModeV(static_cast<Anvil::SamplerAddressMode>(addressModeV));
}
void Lua::Vulkan::VKSampler::SetAddressModeW(lua_State *l,Sampler &hSampler,int32_t addressModeW)
{
	Lua::CheckVKSampler(l,1);
	hSampler->SetAddressModeW(static_cast<Anvil::SamplerAddressMode>(addressModeW));
}
void Lua::Vulkan::VKSampler::SetMipLodBias(lua_State *l,Sampler &hSampler,float mipLodBias)
{
	Lua::CheckVKSampler(l,1);
	//hSampler->SetMipLodBias(mipLodBias); // TODO
}
void Lua::Vulkan::VKSampler::SetAnisotropyEnabled(lua_State *l,Sampler &hSampler,bool anisotropyEnabled)
{
	Lua::CheckVKSampler(l,1);
	//hSampler->SetAnisotropyEnabled(anisotropyEnabled); // TODO
}
void Lua::Vulkan::VKSampler::SetMaxAnisotropy(lua_State *l,Sampler &hSampler,float maxAnisotropy)
{
	Lua::CheckVKSampler(l,1);
	hSampler->SetMaxAnisotropy(maxAnisotropy);
}
void Lua::Vulkan::VKSampler::SetCompareEnabled(lua_State *l,Sampler &hSampler,bool compareEnabled)
{
	Lua::CheckVKSampler(l,1);
	hSampler->SetCompareEnable(compareEnabled);
}
void Lua::Vulkan::VKSampler::SetCompareOperation(lua_State *l,Sampler &hSampler,int32_t compareOp)
{
	Lua::CheckVKSampler(l,1);
	hSampler->SetCompareOp(static_cast<Anvil::CompareOp>(compareOp));
}
void Lua::Vulkan::VKSampler::SetMinLod(lua_State *l,Sampler &hSampler,float minLod)
{
	Lua::CheckVKSampler(l,1);
	hSampler->SetMinLod(minLod);
}
void Lua::Vulkan::VKSampler::SetMaxLod(lua_State *l,Sampler &hSampler,float maxLod)
{
	Lua::CheckVKSampler(l,1);
	hSampler->SetMaxLod(maxLod);
}
void Lua::Vulkan::VKSampler::SetBorderColor(lua_State *l,Sampler &hSampler,int32_t borderColor)
{
	Lua::CheckVKSampler(l,1);
	hSampler->SetBorderColor(static_cast<Anvil::BorderColor>(borderColor));
}
void Lua::Vulkan::VKSampler::SetUseUnnormalizedCoordinates(lua_State *l,Sampler &hSampler,bool bUnnormalizedCoordinates)
{
	Lua::CheckVKSampler(l,1);
	hSampler->SetUseUnnormalizedCoordinates(bUnnormalizedCoordinates);
}

/////////////////////////////////

void Lua::Vulkan::VKFramebuffer::IsValid(lua_State *l,Framebuffer &hFramebuffer)
{
	Lua::PushBool(l,hFramebuffer != nullptr);
}
void Lua::Vulkan::VKFramebuffer::GetAttachment(lua_State *l,Framebuffer &hFramebuffer,uint32_t attId)
{
	Lua::CheckVKFramebuffer(l,1);
	Anvil::ImageView *imgView;
	if((*hFramebuffer)->get_create_info_ptr()->get_attachment_at_index(attId,&imgView) == false || imgView == nullptr)
		return;
	// prosper TODO
}
void Lua::Vulkan::VKFramebuffer::GetAttachmentCount(lua_State *l,Framebuffer &hFramebuffer)
{
	Lua::CheckVKFramebuffer(l,1);
	Lua::PushInt(l,hFramebuffer->GetAttachmentCount());
}
void Lua::Vulkan::VKFramebuffer::GetSize(lua_State *l,Framebuffer &hFramebuffer)
{
	Lua::CheckVKFramebuffer(l,1);
	Lua::PushInt(l,hFramebuffer->GetAttachmentCount());
	uint32_t width;
	uint32_t height;
	uint32_t depth;
	hFramebuffer->GetSize(width,height,depth);
	Lua::PushInt(l,width);
	Lua::PushInt(l,height);
	Lua::PushInt(l,depth);
}

/////////////////////////////////

void Lua::Vulkan::VKRenderPass::IsValid(lua_State *l,RenderPass &hRenderPass)
{
	Lua::PushBool(l,hRenderPass != nullptr);
}
void Lua::Vulkan::VKRenderPass::GetInfo(lua_State *l,RenderPass &hRenderPass)
{
	Lua::CheckVKRenderPass(l,1);
	auto *info = (*hRenderPass)->get_render_pass_create_info(); // prosper TODO

}
void Lua::Vulkan::VKRenderPass::GetSwapchain(lua_State *l,RenderPass &hRenderPass)
{
	Lua::CheckVKRenderPass(l,1);
	auto *swapchain = (*hRenderPass)->get_swapchain(); // prosper TODO
}

/////////////////////////////////

void Lua::Vulkan::VKEvent::IsValid(lua_State *l,Event &hEvent)
{
	Lua::PushBool(l,hEvent != nullptr);
}
void Lua::Vulkan::VKEvent::GetStatus(lua_State *l,Event &hEvent)
{
	Lua::CheckVKEvent(l,1);
	auto b = hEvent->is_set();
	auto r = b ? vk::Result::eEventSet : vk::Result::eEventReset;
	Lua::PushInt(l,umath::to_integral(r));
}
void Lua::Vulkan::VKEvent::IsSet(lua_State *l,Event &hEvent)
{
	Lua::CheckVKEvent(l,1);
	Lua::PushBool(l,hEvent->is_set());
}

/////////////////////////////////

void Lua::Vulkan::VKFence::IsValid(lua_State *l,Fence &hFence)
{
	Lua::PushBool(l,hFence != nullptr);
}

/////////////////////////////////

void Lua::Vulkan::VKSemaphore::IsValid(lua_State *l,Semaphore &hSemaphore)
{
	Lua::PushBool(l,hSemaphore != nullptr);
}

/////////////////////////////////

void Lua::Vulkan::VKMemory::IsValid(lua_State *l,Memory &hMemory)
{
	Lua::PushBool(l,&hMemory != nullptr);
}
void Lua::Vulkan::VKMemory::GetParentMemory(lua_State *l,Memory &hMemory)
{
	Lua::CheckVKMemory(l,1);
	auto *pParentMemory = hMemory.get_create_info_ptr()->get_parent_memory_block();
	if(pParentMemory == nullptr)
		return;
	Lua::Push<Memory*>(l,pParentMemory);
}
void Lua::Vulkan::VKMemory::GetSize(lua_State *l,Memory &hMemory)
{
	Lua::CheckVKMemory(l,1);
	Lua::PushInt(l,hMemory.get_create_info_ptr()->get_size());
}
void Lua::Vulkan::VKMemory::GetStartOffset(lua_State *l,Memory &hMemory)
{
	Lua::CheckVKMemory(l,1);
	Lua::PushInt(l,hMemory.get_start_offset());
}
void Lua::Vulkan::VKMemory::Write(lua_State *l,Memory &hMemory,uint32_t offset,::DataStream &ds,uint32_t dsOffset,uint32_t dsSize)
{
	Lua::CheckVKMemory(l,1);
	Lua::PushBool(l,hMemory.write(offset,dsSize,ds->GetData() +dsOffset));
}
void Lua::Vulkan::VKMemory::Read(lua_State *l,Memory &hMemory,uint32_t offset,uint32_t size)
{
	Lua::CheckVKMemory(l,1);
	auto ds = ::DataStream(size);
	if(hMemory.read(offset,size,ds->GetData()) == false)
		return;
	Lua::Push(l,ds);
}
void Lua::Vulkan::VKMemory::Read(lua_State *l,Memory &hMemory,uint32_t offset,uint32_t size,::DataStream &ds,uint32_t dsOffset)
{
	Lua::CheckVKMemory(l,1);
	auto reqSize = dsOffset +size;
	if(ds->GetSize() < reqSize)
		ds->Resize(reqSize);
	Lua::PushBool(l,hMemory.read(offset,size,ds->GetData() +dsOffset));
}
void Lua::Vulkan::VKMemory::Map(lua_State *l,Memory &hMemory,uint32_t offset,uint32_t size)
{
	Lua::CheckVKMemory(l,1);
	Lua::PushBool(l,hMemory.map(offset,size));
}
void Lua::Vulkan::VKMemory::Unmap(lua_State *l,Memory &hMemory)
{
	Lua::CheckVKMemory(l,1);
	Lua::PushBool(l,hMemory.unmap());
}

/////////////////////////////////

void Lua::Vulkan::VKCommandBuffer::IsValid(lua_State *l,CommandBuffer &hCommandBuffer)
{
	Lua::PushBool(l,hCommandBuffer != nullptr);
}
void Lua::Vulkan::VKCommandBuffer::RecordClearImage(lua_State *l,CommandBuffer &hCommandBuffer,Image &img,const Color &col,const prosper::util::ClearImageInfo &clearImageInfo)
{
	Lua::CheckVKCommandBuffer(l,1);
	auto vcol = col.ToVector4();
	Lua::PushBool(l,prosper::util::record_clear_image(hCommandBuffer->GetAnvilCommandBuffer(),img->GetAnvilImage(),Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,{vcol.r,vcol.g,vcol.b,vcol.a},clearImageInfo));
}
void Lua::Vulkan::VKCommandBuffer::RecordClearImage(lua_State *l,CommandBuffer &hCommandBuffer,Image &img,float clearDepth,const prosper::util::ClearImageInfo &clearImageInfo)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,prosper::util::record_clear_image(hCommandBuffer->GetAnvilCommandBuffer(),img->GetAnvilImage(),Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,clearDepth,clearImageInfo));
}
void Lua::Vulkan::VKCommandBuffer::RecordClearAttachment(lua_State *l,CommandBuffer &hCommandBuffer,Image &img,const Color &col,uint32_t attId)
{
	Lua::CheckVKCommandBuffer(l,1);
	auto vcol = col.ToVector4();
	Lua::PushBool(l,prosper::util::record_clear_attachment(hCommandBuffer->GetAnvilCommandBuffer(),img->GetAnvilImage(),{vcol.r,vcol.g,vcol.b,vcol.a},attId));
}
void Lua::Vulkan::VKCommandBuffer::RecordClearAttachment(lua_State *l,CommandBuffer &hCommandBuffer,Image &img,float clearDepth)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,prosper::util::record_clear_attachment(hCommandBuffer->GetAnvilCommandBuffer(),img->GetAnvilImage(),clearDepth));
}
void Lua::Vulkan::VKCommandBuffer::RecordCopyImage(lua_State *l,CommandBuffer &hCommandBuffer,Image &imgSrc,Image &imgDst,const prosper::util::CopyInfo &copyInfo)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,prosper::util::record_copy_image(hCommandBuffer->GetAnvilCommandBuffer(),copyInfo,imgSrc->GetAnvilImage(),imgDst->GetAnvilImage()));
}
void Lua::Vulkan::VKCommandBuffer::RecordCopyBufferToImage(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &bufSrc,Image &imgDst,const prosper::util::BufferImageCopyInfo &copyInfo)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,prosper::util::record_copy_buffer_to_image(hCommandBuffer->GetAnvilCommandBuffer(),copyInfo,*bufSrc,imgDst->GetAnvilImage()));
}
void Lua::Vulkan::VKCommandBuffer::RecordCopyBuffer(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &bufSrc,Buffer &bufDst,const Anvil::BufferCopy &copyInfo)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,prosper::util::record_copy_buffer(hCommandBuffer->GetAnvilCommandBuffer(),copyInfo,*bufSrc,*bufDst));
}
void Lua::Vulkan::VKCommandBuffer::RecordUpdateBuffer(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &buf,uint32_t offset,::DataStream &ds,uint32_t dstStageMask,uint32_t dstAccessMask)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,prosper::util::record_update_buffer(hCommandBuffer->GetAnvilCommandBuffer(),*buf,offset,ds->GetSize(),ds->GetData(),static_cast<Anvil::PipelineStageFlagBits>(dstStageMask),static_cast<Anvil::AccessFlagBits>(dstAccessMask)));
}
void Lua::Vulkan::VKCommandBuffer::RecordBlitImage(lua_State *l,CommandBuffer &hCommandBuffer,Image &imgSrc,Image &imgDst,const prosper::util::BlitInfo &blitInfo)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,prosper::util::record_blit_image(hCommandBuffer->GetAnvilCommandBuffer(),blitInfo,imgSrc->GetAnvilImage(),imgDst->GetAnvilImage()));
}
void Lua::Vulkan::VKCommandBuffer::RecordResolveImage(lua_State *l,CommandBuffer &hCommandBuffer,Image &imgSrc,Image &imgDst)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,prosper::util::record_resolve_image(hCommandBuffer->GetAnvilCommandBuffer(),imgSrc->GetAnvilImage(),imgDst->GetAnvilImage()));
}
void Lua::Vulkan::VKCommandBuffer::RecordBlitTexture(lua_State *l,CommandBuffer &hCommandBuffer,Texture &texSrc,Image &imgDst)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,prosper::util::record_blit_texture(hCommandBuffer->GetAnvilCommandBuffer(),*texSrc,imgDst->GetAnvilImage()));
}
void Lua::Vulkan::VKCommandBuffer::RecordGenerateMipmaps(lua_State *l,CommandBuffer &hCommandBuffer,Image &img,uint32_t currentLayout,uint32_t srcAccessMask,uint32_t srcStage)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,prosper::util::record_generate_mipmaps(hCommandBuffer->GetAnvilCommandBuffer(),img->GetAnvilImage(),static_cast<Anvil::ImageLayout>(currentLayout),static_cast<Anvil::AccessFlagBits>(srcAccessMask),static_cast<Anvil::PipelineStageFlagBits>(srcStage)));
}
void Lua::Vulkan::VKCommandBuffer::RecordPipelineBarrier(lua_State *l,CommandBuffer &hCommandBuffer,const prosper::util::PipelineBarrierInfo &barrierInfo)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,prosper::util::record_pipeline_barrier(hCommandBuffer->GetAnvilCommandBuffer(),barrierInfo));
}
void Lua::Vulkan::VKCommandBuffer::RecordImageBarrier(
	lua_State *l,CommandBuffer &hCommandBuffer,Image &img,
	uint32_t oldLayout,uint32_t newLayout,const prosper::util::ImageSubresourceRange &subresourceRange
)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,prosper::util::record_image_barrier(
		hCommandBuffer->GetAnvilCommandBuffer(),img->GetAnvilImage(),
		static_cast<Anvil::ImageLayout>(oldLayout),static_cast<Anvil::ImageLayout>(newLayout),
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
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,prosper::util::record_image_barrier(
		hCommandBuffer->GetAnvilCommandBuffer(),img->GetAnvilImage(),
		static_cast<Anvil::PipelineStageFlagBits>(srcStageMask),static_cast<Anvil::PipelineStageFlagBits>(dstStageMask),
		static_cast<Anvil::ImageLayout>(oldLayout),static_cast<Anvil::ImageLayout>(newLayout),
		static_cast<Anvil::AccessFlagBits>(srcAccessMask),static_cast<Anvil::AccessFlagBits>(dstAccessMask),
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
	Lua::CheckVKCommandBuffer(l,1);
	auto lsize = (size != std::numeric_limits<uint32_t>::max()) ? static_cast<uint64_t>(size) : std::numeric_limits<uint64_t>::max();
	Lua::PushBool(l,prosper::util::record_buffer_barrier(
		hCommandBuffer->GetAnvilCommandBuffer(),*buf,
		static_cast<Anvil::PipelineStageFlagBits>(srcStageMask),static_cast<Anvil::PipelineStageFlagBits>(dstStageMask),
		static_cast<Anvil::AccessFlagBits>(srcAccessMask),static_cast<Anvil::AccessFlagBits>(dstAccessMask),
		offset,lsize
	));
}
void Lua::Vulkan::VKCommandBuffer::RecordSetViewport(lua_State *l,CommandBuffer &hCommandBuffer,uint32_t width,uint32_t height,uint32_t x,uint32_t y)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,prosper::util::record_set_viewport(hCommandBuffer->GetAnvilCommandBuffer(),width,height,x,y));
}
void Lua::Vulkan::VKCommandBuffer::RecordSetScissor(lua_State *l,CommandBuffer &hCommandBuffer,uint32_t width,uint32_t height,uint32_t x,uint32_t y)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,prosper::util::record_set_scissor(hCommandBuffer->GetAnvilCommandBuffer(),width,height,x,y));
}
void Lua::Vulkan::VKCommandBuffer::RecordBeginRenderPass(lua_State *l,CommandBuffer &hCommandBuffer,Lua::Vulkan::RenderPassInfo &rpInfo)
{
	Lua::CheckVKCommandBuffer(l,1);
	if((*hCommandBuffer)->get_command_buffer_type() != Anvil::CommandBufferType::COMMAND_BUFFER_TYPE_PRIMARY)
	{
		Lua::PushBool(l,false);
		return;
	}
	static_assert(sizeof(Lua::Vulkan::ClearValue) == sizeof(vk::ClearValue));
	auto &primaryCmdBuffer = static_cast<Anvil::PrimaryCommandBuffer&>(*(*hCommandBuffer));
	if(rpInfo.layerId.has_value())
	{
		auto r = prosper::util::record_begin_render_pass(
			primaryCmdBuffer,*rpInfo.renderTarget,
			*rpInfo.layerId,
			reinterpret_cast<std::vector<vk::ClearValue>&>(rpInfo.clearValues),
			rpInfo.renderPass.get()
		);
		Lua::PushBool(l,r);
		return;
	}
	auto r = prosper::util::record_begin_render_pass(
		primaryCmdBuffer,*rpInfo.renderTarget,
		reinterpret_cast<std::vector<vk::ClearValue>&>(rpInfo.clearValues),
		rpInfo.renderPass.get()
	);
	Lua::PushBool(l,r);
}
void Lua::Vulkan::VKCommandBuffer::RecordEndRenderPass(lua_State *l,CommandBuffer &hCommandBuffer)
{
	Lua::CheckVKCommandBuffer(l,1);
	if((*hCommandBuffer)->get_command_buffer_type() != Anvil::CommandBufferType::COMMAND_BUFFER_TYPE_PRIMARY)
	{
		Lua::PushBool(l,false);
		return;
	}
	auto &primaryCmdBuffer = static_cast<Anvil::PrimaryCommandBuffer&>(*(*hCommandBuffer));
	Lua::PushBool(l,prosper::util::record_end_render_pass(primaryCmdBuffer));
}
void Lua::Vulkan::VKCommandBuffer::RecordBindIndexBuffer(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &indexBuffer,uint32_t indexType,uint32_t offset)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,(*hCommandBuffer)->record_bind_index_buffer(&indexBuffer->GetAnvilBuffer(),offset,static_cast<Anvil::IndexType>(indexType)));
}
void Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffer(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &vertexBuffer,uint32_t startBinding,uint32_t offset)
{
	Lua::CheckVKCommandBuffer(l,1);
	auto *anvBuffer = &vertexBuffer->GetAnvilBuffer();
	auto vOffset = static_cast<uint64_t>(offset);
	Lua::PushBool(l,(*hCommandBuffer)->record_bind_vertex_buffers(startBinding,1u,&anvBuffer,&vOffset));
}
void Lua::Vulkan::VKCommandBuffer::RecordBindVertexBuffers(lua_State *l,CommandBuffer &hCommandBuffer,luabind::object vertexBuffers,uint32_t startBinding,luabind::object offsets)
{
	Lua::CheckVKCommandBuffer(l,1);
	auto buffers = Lua::get_table_values<Anvil::Buffer*>(l,2,[](lua_State *l,int32_t idx) {
		return &(*Lua::CheckVKBuffer(l,idx))->GetAnvilBuffer();
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
	Lua::PushBool(l,(*hCommandBuffer)->record_bind_vertex_buffers(startBinding,buffers.size(),buffers.data(),voffsets.data()));
}
void Lua::Vulkan::VKCommandBuffer::RecordCopyImageToBuffer(lua_State *l,CommandBuffer &hCommandBuffer,Image &imgSrc,uint32_t srcImageLayout,Buffer &bufDst,const prosper::util::BufferImageCopyInfo &copyInfo)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,prosper::util::record_copy_image_to_buffer(*(*hCommandBuffer),copyInfo,imgSrc->GetAnvilImage(),static_cast<Anvil::ImageLayout>(srcImageLayout),*bufDst));
}
void Lua::Vulkan::VKCommandBuffer::RecordDispatch(lua_State *l,CommandBuffer &hCommandBuffer,uint32_t x,uint32_t y,uint32_t z)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,(*hCommandBuffer)->record_dispatch(x,y,z));
}
void Lua::Vulkan::VKCommandBuffer::RecordDispatchIndirect(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &buffer,uint32_t offset)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,(*hCommandBuffer)->record_dispatch_indirect(&buffer->GetAnvilBuffer(),offset));
}
void Lua::Vulkan::VKCommandBuffer::RecordDraw(lua_State *l,CommandBuffer &hCommandBuffer,uint32_t vertexCount,uint32_t instanceCount,uint32_t firstVertex,uint32_t firstInstance)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,(*hCommandBuffer)->record_draw(vertexCount,instanceCount,firstVertex,firstInstance));
}
void Lua::Vulkan::VKCommandBuffer::RecordDrawIndexed(lua_State *l,CommandBuffer &hCommandBuffer,uint32_t indexCount,uint32_t instanceCount,uint32_t firstIndex,uint32_t vertexOffset,uint32_t firstInstance)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,(*hCommandBuffer)->record_draw_indexed(indexCount,instanceCount,firstIndex,vertexOffset,firstInstance));
}
void Lua::Vulkan::VKCommandBuffer::RecordDrawIndexedIndirect(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &buffer,uint32_t offset,uint32_t drawCount,uint32_t stride)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,(*hCommandBuffer)->record_draw_indexed_indirect(&buffer->GetAnvilBuffer(),offset,drawCount,stride));
}
void Lua::Vulkan::VKCommandBuffer::RecordDrawIndirect(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &buffer,uint32_t offset,uint32_t drawCount,uint32_t stride)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,(*hCommandBuffer)->record_draw_indirect(&buffer->GetAnvilBuffer(),offset,drawCount,stride));
}
void Lua::Vulkan::VKCommandBuffer::RecordFillBuffer(lua_State *l,CommandBuffer &hCommandBuffer,Buffer &buffer,uint32_t offset,uint32_t size,uint32_t data)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,(*hCommandBuffer)->record_fill_buffer(&buffer->GetAnvilBuffer(),offset,size,data));
}
void Lua::Vulkan::VKCommandBuffer::RecordResetEvent(lua_State *l,CommandBuffer &hCommandBuffer,Event &ev,uint32_t stageMask)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,(*hCommandBuffer)->record_reset_event(ev.get(),static_cast<Anvil::PipelineStageFlagBits>(stageMask)));
}
void Lua::Vulkan::VKCommandBuffer::RecordSetBlendConstants(lua_State *l,CommandBuffer &hCommandBuffer,const Vector4 &blendConstants)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,(*hCommandBuffer)->record_set_blend_constants(reinterpret_cast<const float*>(&blendConstants)));
}
void Lua::Vulkan::VKCommandBuffer::RecordSetDepthBias(lua_State *l,CommandBuffer &hCommandBuffer,float depthBiasConstantFactor,float depthBiasClamp,float slopeScaledDepthBias)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,(*hCommandBuffer)->record_set_depth_bias(depthBiasConstantFactor,depthBiasClamp,slopeScaledDepthBias));
}
void Lua::Vulkan::VKCommandBuffer::RecordSetDepthBounds(lua_State *l,CommandBuffer &hCommandBuffer,float minDepthBounds,float maxDepthBounds)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,(*hCommandBuffer)->record_set_depth_bounds(minDepthBounds,maxDepthBounds));
}
void Lua::Vulkan::VKCommandBuffer::RecordSetEvent(lua_State *l,CommandBuffer &hCommandBuffer,Event &ev,uint32_t stageMask)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,(*hCommandBuffer)->record_set_event(ev.get(),static_cast<Anvil::PipelineStageFlagBits>(stageMask)));
}
void Lua::Vulkan::VKCommandBuffer::RecordSetLineWidth(lua_State *l,CommandBuffer &hCommandBuffer,float lineWidth)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,(*hCommandBuffer)->record_set_line_width(lineWidth));
}
void Lua::Vulkan::VKCommandBuffer::RecordSetStencilCompareMask(lua_State *l,CommandBuffer &hCommandBuffer,uint32_t faceMask,uint32_t stencilCompareMask)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,(*hCommandBuffer)->record_set_stencil_compare_mask(static_cast<Anvil::StencilFaceFlagBits>(faceMask),stencilCompareMask));
}
void Lua::Vulkan::VKCommandBuffer::RecordSetStencilReference(lua_State *l,CommandBuffer &hCommandBuffer,uint32_t faceMask,uint32_t stencilReference)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,(*hCommandBuffer)->record_set_stencil_reference(static_cast<Anvil::StencilFaceFlagBits>(faceMask),stencilReference));
}
void Lua::Vulkan::VKCommandBuffer::RecordSetStencilWriteMask(lua_State *l,CommandBuffer &hCommandBuffer,uint32_t faceMask,uint32_t stencilWriteMask)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,(*hCommandBuffer)->record_set_stencil_write_mask(static_cast<Anvil::StencilFaceFlagBits>(faceMask),stencilWriteMask));
}
/*void Lua::Vulkan::VKCommandBuffer::RecordWaitEvents(lua_State *l,CommandBuffer &hCommandBuffer)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,hCommandBuffer->record_wait_events(event,));
}
void Lua::Vulkan::VKCommandBuffer::RecordWriteTimestamp(lua_State *l,CommandBuffer &hCommandBuffer)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,hCommandBuffer->record_write_timestamp(stage,queryPool,queryIndex));
}
void Lua::Vulkan::VKCommandBuffer::RecordBeginQuery(lua_State *l,CommandBuffer &hCommandBuffer)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,hCommandBuffer->record_begin_query(queryPool,entry,flags));
}
void Lua::Vulkan::VKCommandBuffer::RecordEndQuery(lua_State *l,CommandBuffer &hCommandBuffer)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,hCommandBuffer->record_end_query(queryPool,queryIndex));
}*/ // TODO
void Lua::Vulkan::VKCommandBuffer::RecordDrawGradient(lua_State *l,CommandBuffer &hCommandBuffer,RenderTarget &rt,const Vector2 &dir,luabind::object lnodes)
{
	Lua::CheckVKCommandBuffer(l,1);
	if((*hCommandBuffer)->get_command_buffer_type() != Anvil::CommandBufferType::COMMAND_BUFFER_TYPE_PRIMARY)
	{
		Lua::PushBool(l,false);
		return;
	}
	auto nodes = get_gradient_nodes(l,4);
	auto primCmd = std::static_pointer_cast<prosper::PrimaryCommandBuffer>(hCommandBuffer);
	Lua::PushBool(l,pragma::util::record_draw_gradient(*c_engine,primCmd,*rt,dir,nodes));
}
void Lua::Vulkan::VKCommandBuffer::StopRecording(lua_State *l,CommandBuffer &hCommandBuffer)
{
	Lua::CheckVKCommandBuffer(l,1);
	Lua::PushBool(l,(*hCommandBuffer)->stop_recording());
}
void Lua::Vulkan::VKCommandBuffer::StartRecording(lua_State *l,CommandBuffer &hCommandBuffer,bool oneTimeSubmit,bool simultaneousUseAllowed)
{
	Lua::CheckVKCommandBuffer(l,1);
	if((*hCommandBuffer)->get_command_buffer_type() != Anvil::CommandBufferType::COMMAND_BUFFER_TYPE_PRIMARY)
	{
		Lua::PushBool(l,false);
		return;
	}
	auto &primCmd = static_cast<Anvil::PrimaryCommandBuffer&>(*(*hCommandBuffer));
	Lua::PushBool(l,primCmd.start_recording(oneTimeSubmit,simultaneousUseAllowed));
}

/////////////////////////////////

void Lua::Vulkan::VKBuffer::IsValid(lua_State *l,Buffer &hBuffer)
{
	Lua::PushBool(l,hBuffer != nullptr);
}
void Lua::Vulkan::VKBuffer::GetStartOffset(lua_State *l,Buffer &hBuffer)
{
	Lua::CheckVKBuffer(l,1);
	Lua::PushInt(l,hBuffer->GetStartOffset());
}
void Lua::Vulkan::VKBuffer::SetPermanentlyMapped(lua_State *l,Buffer &hBuffer,bool b)
{
	Lua::CheckVKBuffer(l,1);
	hBuffer->SetPermanentlyMapped(b);
}
void Lua::Vulkan::VKBuffer::GetBaseIndex(lua_State *l,Buffer &hBuffer)
{
	Lua::CheckVKBuffer(l,1);
	Lua::PushInt(l,hBuffer->GetBaseIndex());
}
void Lua::Vulkan::VKBuffer::GetSize(lua_State *l,Buffer &hBuffer)
{
	Lua::CheckVKBuffer(l,1);
	Lua::PushInt(l,hBuffer->GetSize());
}
void Lua::Vulkan::VKBuffer::GetUsageFlags(lua_State *l,Buffer &hBuffer)
{
	Lua::CheckVKBuffer(l,1);
	Lua::PushInt(l,static_cast<uint32_t>(hBuffer->GetUsageFlags().get_vk()));
}
void Lua::Vulkan::VKBuffer::GetParent(lua_State *l,Buffer &hBuffer)
{
	Lua::CheckVKBuffer(l,1);
	auto parent = hBuffer->GetParent();
	if(parent == nullptr)
		return;
	Lua::Push(l,parent);
}
void Lua::Vulkan::VKBuffer::Write(lua_State *l,Buffer &hBuffer,uint32_t offset,::DataStream &ds,uint32_t dsOffset,uint32_t dsSize)
{
	Lua::CheckVKBuffer(l,1);
	Lua::PushBool(l,hBuffer->Write(offset,dsSize,ds->GetData() +dsOffset));
}
void Lua::Vulkan::VKBuffer::Read(lua_State *l,Buffer &hBuffer,uint32_t offset,uint32_t size)
{
	Lua::CheckVKBuffer(l,1);
	auto ds = ::DataStream(size);
	auto r = hBuffer->Read(offset,size,ds->GetData());
	if(r == false)
		return;
	Lua::Push(l,ds);
}
void Lua::Vulkan::VKBuffer::Read(lua_State *l,Buffer &hBuffer,uint32_t offset,uint32_t size,::DataStream &ds,uint32_t dsOffset)
{
	Lua::CheckVKBuffer(l,1);
	auto reqSize = size +dsOffset;
	if(ds->GetSize() < reqSize)
		ds->Resize(reqSize);
	Lua::PushBool(l,hBuffer->Read(offset,size,ds->GetData() +dsOffset));
}
void Lua::Vulkan::VKBuffer::Map(lua_State *l,Buffer &hBuffer,uint32_t offset,uint32_t size)
{
	Lua::CheckVKBuffer(l,1);
	Lua::PushBool(l,hBuffer->Map(offset,size));
}
void Lua::Vulkan::VKBuffer::Unmap(lua_State *l,Buffer &hBuffer)
{
	Lua::CheckVKBuffer(l,1);
	Lua::PushBool(l,hBuffer->Unmap());
}

/////////////////////////////////

void Lua::Vulkan::VKDescriptorSet::IsValid(lua_State *l,DescriptorSet &hDescSet)
{
	Lua::PushBool(l,hDescSet != nullptr);
}
void Lua::Vulkan::VKDescriptorSet::GetBindingCount(lua_State *l,DescriptorSet &hDescSet)
{
	Lua::CheckVKDescriptorSet(l,1);
	auto *infos = (*hDescSet)->get_descriptor_set_create_info();
	if(infos == nullptr || infos->empty())
		return;
	Lua::PushInt(l,infos->at(0)->get_n_bindings());
}
void Lua::Vulkan::VKDescriptorSet::GetBindingInfo(lua_State *l,DescriptorSet &hDescSet)
{
	Lua::CheckVKDescriptorSet(l,1);
	auto *infos = (*hDescSet)->get_descriptor_set_create_info();
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
	Lua::CheckVKDescriptorSet(l,1);
	auto *infos = (*hDescSet)->get_descriptor_set_create_info();
	if(infos == nullptr || infos->empty())
		return;
	pragma::LuaDescriptorSetBinding binding {};
	if(infos->at(0)->get_binding_properties_by_index_number(
			bindingIdx,&binding.bindingIndex,reinterpret_cast<Anvil::DescriptorType*>(&binding.type),&binding.descriptorArraySize,reinterpret_cast<Anvil::ShaderStageFlags*>(&binding.shaderStages)
		) == false)
		return;
	Lua::Push(l,binding);
}
void Lua::Vulkan::VKDescriptorSet::SetBindingTexture(lua_State *l,Lua::Vulkan::DescriptorSet &hDescSet,uint32_t bindingIdx,Lua::Vulkan::Texture &texture)
{
	Lua::CheckVKDescriptorSet(l,1);
	Lua::PushBool(l,prosper::util::set_descriptor_set_binding_texture(*(*hDescSet)->get_descriptor_set(0u),*texture,bindingIdx));
}
void Lua::Vulkan::VKDescriptorSet::SetBindingTexture(lua_State *l,Lua::Vulkan::DescriptorSet &hDescSet,uint32_t bindingIdx,Lua::Vulkan::Texture &texture,uint32_t layerId)
{
	Lua::CheckVKDescriptorSet(l,1);
	Lua::PushBool(l,prosper::util::set_descriptor_set_binding_texture(*(*hDescSet)->get_descriptor_set(0u),*texture,bindingIdx,layerId));
}
void Lua::Vulkan::VKDescriptorSet::SetBindingArrayTexture(lua_State *l,DescriptorSet &hDescSet,uint32_t bindingIdx,Lua::Vulkan::Texture &texture,uint32_t arrayIdx,uint32_t layerId)
{
	Lua::CheckVKDescriptorSet(l,1);
	Lua::PushBool(l,prosper::util::set_descriptor_set_binding_array_texture(*(*hDescSet)->get_descriptor_set(0u),*texture,bindingIdx,arrayIdx,layerId));
}
void Lua::Vulkan::VKDescriptorSet::SetBindingArrayTexture(lua_State *l,DescriptorSet &hDescSet,uint32_t bindingIdx,Lua::Vulkan::Texture &texture,uint32_t arrayIdx)
{
	Lua::CheckVKDescriptorSet(l,1);
	Lua::PushBool(l,prosper::util::set_descriptor_set_binding_array_texture(*(*hDescSet)->get_descriptor_set(0u),*texture,bindingIdx,arrayIdx));
}
void Lua::Vulkan::VKDescriptorSet::SetBindingStorageBuffer(lua_State *l,DescriptorSet &hDescSet,uint32_t bindingIdx,Lua::Vulkan::Buffer &buffer,uint32_t startOffset,uint32_t size)
{
	Lua::CheckVKDescriptorSet(l,1);
	Lua::PushBool(l,prosper::util::set_descriptor_set_binding_storage_buffer(*(*hDescSet)->get_descriptor_set(0u),*buffer,bindingIdx,startOffset,(size != std::numeric_limits<uint32_t>::max()) ? static_cast<uint64_t>(size) : std::numeric_limits<uint64_t>::max()));
}
void Lua::Vulkan::VKDescriptorSet::SetBindingUniformBuffer(lua_State *l,DescriptorSet &hDescSet,uint32_t bindingIdx,Lua::Vulkan::Buffer &buffer,uint32_t startOffset,uint32_t size)
{
	Lua::CheckVKDescriptorSet(l,1);
	Lua::PushBool(l,prosper::util::set_descriptor_set_binding_uniform_buffer(*(*hDescSet)->get_descriptor_set(0u),*buffer,bindingIdx,startOffset,(size != std::numeric_limits<uint32_t>::max()) ? static_cast<uint64_t>(size) : std::numeric_limits<uint64_t>::max()));
}
void Lua::Vulkan::VKDescriptorSet::SetBindingUniformBufferDynamic(lua_State *l,DescriptorSet &hDescSet,uint32_t bindingIdx,Lua::Vulkan::Buffer &buffer,uint32_t startOffset,uint32_t size)
{
	Lua::CheckVKDescriptorSet(l,1);
	Lua::PushBool(l,prosper::util::set_descriptor_set_binding_dynamic_uniform_buffer(*(*hDescSet)->get_descriptor_set(0u),*buffer,bindingIdx,startOffset,(size != std::numeric_limits<uint32_t>::max()) ? static_cast<uint64_t>(size) : std::numeric_limits<uint64_t>::max()));
}

/////////////////////////////////

void Lua::Vulkan::VKMesh::GetVertexBuffer(lua_State *l,std::shared_ptr<pragma::VkMesh> &mesh)
{
	auto &vertexBuffer = mesh->GetVertexBuffer();
	if(vertexBuffer != nullptr)
		Lua::Push<Buffer>(l,vertexBuffer);
}
void Lua::Vulkan::VKMesh::GetVertexWeightBuffer(lua_State *l,std::shared_ptr<pragma::VkMesh> &mesh)
{
	auto &vertexWeightBuffer = mesh->GetVertexBuffer();
	if(vertexWeightBuffer != nullptr)
		Lua::Push<Buffer>(l,vertexWeightBuffer);
}
void Lua::Vulkan::VKMesh::GetAlphaBuffer(lua_State *l,std::shared_ptr<pragma::VkMesh> &mesh)
{
	auto &alphaBuffer = mesh->GetAlphaBuffer();
	if(alphaBuffer != nullptr)
		Lua::Push<Buffer>(l,alphaBuffer);
}
void Lua::Vulkan::VKMesh::GetIndexBuffer(lua_State *l,std::shared_ptr<pragma::VkMesh> &mesh)
{
	auto &indexBuffer = mesh->GetIndexBuffer();
	if(indexBuffer != nullptr)
		Lua::Push<Buffer>(l,indexBuffer);
}
void Lua::Vulkan::VKMesh::SetVertexBuffer(lua_State*,std::shared_ptr<pragma::VkMesh> &mesh,Buffer &hBuffer)
{
	mesh->SetVertexBuffer(hBuffer);
}
void Lua::Vulkan::VKMesh::SetVertexWeightBuffer(lua_State*,std::shared_ptr<pragma::VkMesh> &mesh,Buffer &hBuffer)
{
	mesh->SetVertexWeightBuffer(hBuffer);
}
void Lua::Vulkan::VKMesh::SetAlphaBuffer(lua_State*,std::shared_ptr<pragma::VkMesh> &mesh,Buffer &hBuffer)
{
	mesh->SetAlphaBuffer(hBuffer);
}
void Lua::Vulkan::VKMesh::SetIndexBuffer(lua_State*,std::shared_ptr<pragma::VkMesh> &mesh,Buffer &hBuffer)
{
	mesh->SetIndexBuffer(hBuffer);
}

/////////////////////////////////

void Lua::Vulkan::VKRenderTarget::IsValid(lua_State *l,RenderTarget &hRt)
{
	Lua::PushBool(l,hRt != nullptr);
}
void Lua::Vulkan::VKRenderTarget::GetWidth(lua_State *l,RenderTarget &hRt)
{
	Lua::CheckVKRenderTarget(l,1);
	auto extents = hRt->GetTexture()->GetImage()->GetExtents();
	Lua::PushInt(l,extents.width);
}
void Lua::Vulkan::VKRenderTarget::GetHeight(lua_State *l,RenderTarget &hRt)
{
	Lua::CheckVKRenderTarget(l,1);
	auto extents = hRt->GetTexture()->GetImage()->GetExtents();
	Lua::PushInt(l,extents.height);
}
void Lua::Vulkan::VKRenderTarget::GetFormat(lua_State *l,RenderTarget &hRt)
{
	Lua::CheckVKRenderTarget(l,1);
	Lua::PushInt(l,umath::to_integral(hRt->GetTexture()->GetImage()->GetFormat()));
}
void Lua::Vulkan::VKRenderTarget::GetTexture(lua_State *l,RenderTarget &hRt,uint32_t idx)
{
	Lua::CheckVKRenderTarget(l,1);
	auto &tex = hRt->GetTexture(idx);
	if(tex == nullptr)
		return;
	Lua::Push<Texture>(l,tex);
}
void Lua::Vulkan::VKRenderTarget::GetRenderPass(lua_State *l,RenderTarget &hRt)
{
	Lua::CheckVKRenderTarget(l,1);
	Lua::Push<RenderPass>(l,hRt->GetRenderPass());
}
void Lua::Vulkan::VKRenderTarget::GetFramebuffer(lua_State *l,RenderTarget &hRt)
{
	Lua::CheckVKRenderTarget(l,1);
	Lua::Push<Framebuffer>(l,hRt->GetFramebuffer());
}

/////////////////////////////////

void Lua::Vulkan::VKTimestampQuery::IsValid(lua_State *l,TimestampQuery &hTimestampQuery)
{
	Lua::PushBool(l,hTimestampQuery != nullptr);
}
void Lua::Vulkan::VKTimestampQuery::IsResultAvailable(lua_State *l,TimestampQuery &hTimestampQuery)
{
	Lua::CheckVKTimestampQuery(l,1);
	Lua::PushBool(l,hTimestampQuery->IsResultAvailable());
}
void Lua::Vulkan::VKTimestampQuery::GetPipelineStage(lua_State *l,TimestampQuery &hTimestampQuery)
{
	Lua::CheckVKTimestampQuery(l,1);
	Lua::PushInt(l,umath::to_integral(hTimestampQuery->GetPipelineStage()));
}
void Lua::Vulkan::VKTimestampQuery::Write(lua_State *l,TimestampQuery &hTimestampQuery,CommandBuffer &cmdBuffer)
{
	Lua::CheckVKTimestampQuery(l,1);
	hTimestampQuery->Write(*(*cmdBuffer));
}
void Lua::Vulkan::VKTimestampQuery::QueryResult(lua_State *l,TimestampQuery &hTimestampQuery)
{
	Lua::CheckVKTimestampQuery(l,1);
	auto r = 0.L;
	if(hTimestampQuery->QueryResult(r) == false)
		Lua::PushBool(l,false);
	else
		Lua::PushNumber(l,r);
}

/////////////////////////////////

void Lua::Vulkan::VKTimerQuery::IsValid(lua_State *l,TimerQuery &hTimerQuery)
{
	Lua::PushBool(l,hTimerQuery != nullptr);
}
void Lua::Vulkan::VKTimerQuery::Begin(lua_State*,TimerQuery &hTimerQuery)
{
	hTimerQuery->Begin();
}
void Lua::Vulkan::VKTimerQuery::End(lua_State*,TimerQuery &hTimerQuery)
{
	hTimerQuery->End();
}
void Lua::Vulkan::VKTimerQuery::IsResultAvailable(lua_State *l,TimerQuery &hTimerQuery)
{
	Lua::PushBool(l,hTimerQuery->IsResultAvailable());
}
void Lua::Vulkan::VKTimerQuery::QueryResult(lua_State *l,TimerQuery &hTimerQuery)
{
	auto r = 0.L;
	if(hTimerQuery->QueryResult(r) == false)
		Lua::PushBool(l,false);
	else
		Lua::PushNumber(l,r);
}
