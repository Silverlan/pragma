--- @meta
--- 
--- @class prosper
prosper = {}

--- 
--- @param format enum prosper::Format
--- @return string ret0
function prosper.format_to_string(format) end

--- 
--- @param arg0 class prosper::IPrContext
--- @return prosper.Event ret0
function prosper.create_event(arg0) end

--- 
--- @param rtCreateInfo prosper.RenderTargetCreateInfo
--- @param textures table
--- @return prosper.RenderTarget ret0
--- @overload fun(rtCreateInfo: prosper.RenderTargetCreateInfo, textures: table, renderPass: prosper.RenderPass): prosper.RenderTarget
--- @overload fun(rtCreateInfo: prosper.RenderTargetCreateInfo, texture: prosper.Texture, renderPass: prosper.RenderPass): prosper.RenderTarget
--- @overload fun(rtCreateInfo: prosper.RenderTargetCreateInfo, texture: prosper.Texture): prosper.RenderTarget
function prosper.create_render_target(rtCreateInfo, textures) end

--- 
--- @param context class prosper::IPrContext
--- @param bufCreateInfo prosper.BufferCreateInfo
--- @return prosper.Buffer ret0
--- @overload fun(context: class prosper::IPrContext, bufCreateInfo: prosper.BufferCreateInfo, ds: util.DataStream): prosper.Buffer
function prosper.create_buffer(context, bufCreateInfo) end

--- 
--- @param ldescSetInfo shader.DescriptorSetInfo
--- @return prosper.DescriptorSet ret0
function prosper.create_descriptor_set(ldescSetInfo) end

--- 
--- @param imgBuffer util.ImageBuffer
--- @param imgCreateInfo prosper.ImageCreateInfo
--- @return prosper.Image ret0
--- @overload fun(imgBuffer: util.ImageBuffer): prosper.Image
--- @overload fun(arg0: class std::array<class std::shared_ptr<class uimg::ImageBuffer>,6>, arg1: prosper.ImageCreateInfo): prosper.Image
--- @overload fun(arg0: class std::array<class std::shared_ptr<class uimg::ImageBuffer>,6>): prosper.Image
--- @overload fun(imgCreateInfo: prosper.ImageCreateInfo): prosper.Image
--- @overload fun(imgCreateInfo: prosper.ImageCreateInfo, ds: util.DataStream): prosper.Image
function prosper.create_image(imgBuffer, imgCreateInfo) end

--- 
--- @param stage enum prosper::ShaderStage
--- @return string ret0
function prosper.shader_stage_to_string(stage) end

--- 
--- @param width int
--- @param height int
--- @param attachments table
--- @return prosper.Framebuffer ret0
--- @overload fun(width: int, height: int, attachments: table, layers: int): prosper.Framebuffer
function prosper.create_framebuffer(width, height, attachments) end

--- 
--- @param imgBuf util.ImageBuffer
--- @return prosper.ImageCreateInfo ret0
--- @overload fun(imgBuf: util.ImageBuffer, cubemap: bool): prosper.ImageCreateInfo
function prosper.create_image_create_info(imgBuf) end

--- 
--- @return variant ret0
function prosper.wait_for_current_swapchain_command_buffer_completion() end

--- 
--- @param context class prosper::IPrContext
--- @param imgViewCreateInfo prosper.ImageViewCreateInfo
--- @param img prosper.Image
--- @return prosper.ImageView ret0
function prosper.create_image_view(context, imgViewCreateInfo, img) end

--- 
--- @param img prosper.Image
--- @param texCreateInfo prosper.TextureCreateInfo
--- @return prosper.Texture ret0
--- @overload fun(img: prosper.Image, texCreateInfo: prosper.TextureCreateInfo, imgViewCreateInfo: prosper.ImageViewCreateInfo): prosper.Texture
--- @overload fun(img: prosper.Image, texCreateInfo: prosper.TextureCreateInfo, imgViewCreateInfo: prosper.ImageViewCreateInfo, samplerCreateInfo: prosper.SamplerCreateInfo): prosper.Texture
function prosper.create_texture(img, texCreateInfo) end

--- 
--- @param rpCreateInfo prosper.RenderPassCreateInfo
--- @return prosper.RenderPass ret0
function prosper.create_render_pass(rpCreateInfo) end

--- 
--- @param width int
--- @param height int
--- @param format enum prosper::Format
--- @param dir math.Vector2
--- @param tNodes table
--- @return prosper.Texture ret0
function prosper.create_gradient_texture(width, height, format, dir, tNodes) end

--- 
--- @param srcTex prosper.Texture
--- @param blurStrength int
--- @return variant ret0
--- @overload fun(srcTex: prosper.Texture, blurStrength: int): variant
function prosper.blur_texture(srcTex, blurStrength) end

--- 
--- @return prosper.Fence ret0
--- @overload fun(createSignalled: bool): prosper.Fence
function prosper.create_fence() end

--- 
--- @param w int
--- @param h int
--- @return int ret0
function prosper.calculate_mipmap_count(w, h) end

--- 
--- @param v int
--- @param level int
--- @return int ret0
--- @overload fun(w: int, h: int, level: int): int
function prosper.calculate_mipmap_size(v, level) end

--- 
--- @param r enum prosper::Result
--- @return string ret0
function prosper.result_to_string(r) end

--- 
--- @param format enum prosper::Format
--- @return bool ret0
function prosper.is_depth_format(format) end

--- 
--- @param format enum prosper::Format
--- @return bool ret0
function prosper.is_compressed_format(format) end

--- 
--- @param format enum prosper::Format
--- @return bool ret0
function prosper.is_uncompressed_format(format) end

--- 
--- @param format enum prosper::Format
--- @return int ret0
function prosper.get_bit_size(format) end

--- 
--- @return prosper.CommandBuffer ret0
function prosper.create_primary_command_buffer() end

--- 
--- @param format enum prosper::Format
--- @return int ret0
function prosper.get_byte_size(format) end

--- 
--- @param arg0 class prosper::IPrContext
--- @param arg1 bool
function prosper.wait_idle(arg0, arg1) end

--- 
--- @param arg0 class prosper::IPrContext
--- @return string ret0
function prosper.get_api_identifier(arg0) end

--- 
--- @param arg0 class prosper::IPrContext
--- @return string ret0
function prosper.get_api_abbreviation(arg0) end

--- 
--- @return prosper.CommandBuffer ret0
function prosper.create_secondary_command_buffer() end

--- 
--- @param arg0 class prosper::IPrContext
function prosper.flush(arg0) end

--- 
--- @param arg0 class CEngine
--- @param arg1 prosper.WindowCreateInfo
--- @return prosper.Window ret0
function prosper.create_window(arg0, arg1) end


--- 
--- @class prosper.PreparedCommandBuffer
--- @field enableDrawArgs bool 
--- @overload fun():prosper.PreparedCommandBuffer
prosper.PreparedCommandBuffer = {}

--- 
--- @param cmd prosper.CommandBuffer
--- @return bool ret0
function prosper.PreparedCommandBuffer:RecordCommands(cmd) end

--- 
--- @param lineWidth struct Lua::Vulkan::PreparedCommandLuaArg
function prosper.PreparedCommandBuffer:RecordSetLineWidth(lineWidth) end

--- 
--- @param faceMask struct Lua::Vulkan::PreparedCommandLuaArg
--- @param stencilReference struct Lua::Vulkan::PreparedCommandLuaArg
function prosper.PreparedCommandBuffer:RecordSetStencilReference(faceMask, stencilReference) end

--- 
--- @param buf prosper.Buffer
--- @param srcStageMask struct Lua::Vulkan::PreparedCommandLuaArg
--- @param dstStageMask struct Lua::Vulkan::PreparedCommandLuaArg
--- @param srcAccessMask struct Lua::Vulkan::PreparedCommandLuaArg
--- @param dstAccessMask struct Lua::Vulkan::PreparedCommandLuaArg
--- @overload fun(buf: prosper.Buffer, srcStageMask: struct Lua::Vulkan::PreparedCommandLuaArg, dstStageMask: struct Lua::Vulkan::PreparedCommandLuaArg, srcAccessMask: struct Lua::Vulkan::PreparedCommandLuaArg, dstAccessMask: struct Lua::Vulkan::PreparedCommandLuaArg, offset: struct Lua::Vulkan::PreparedCommandLuaArg, size: struct Lua::Vulkan::PreparedCommandLuaArg): 
function prosper.PreparedCommandBuffer:RecordBufferBarrier(buf, srcStageMask, dstStageMask, srcAccessMask, dstAccessMask) end

--- 
--- @param img prosper.Image
--- @param color util.Color
--- @return bool ret0
function prosper.PreparedCommandBuffer:RecordClearImage(img, color) end

--- 
--- @param width struct Lua::Vulkan::PreparedCommandLuaArg
--- @param height struct Lua::Vulkan::PreparedCommandLuaArg
--- @param x struct Lua::Vulkan::PreparedCommandLuaArg
--- @param y struct Lua::Vulkan::PreparedCommandLuaArg
function prosper.PreparedCommandBuffer:RecordSetScissor(width, height, x, y) end


--- 
--- @class prosper.PreparedCommandBuffer.DynArg
--- @overload fun(arg1: string):prosper.PreparedCommandBuffer.DynArg
prosper.PreparedCommandBuffer.DynArg = {}


--- 
--- @class prosper.PipelineBarrierInfo
--- @field srcStageMask int 
--- @field dstStageMask int 
--- @overload fun():prosper.PipelineBarrierInfo
prosper.PipelineBarrierInfo = {}


--- 
--- @class prosper.BufferCopyInfo
--- @field size int 
--- @field srcOffset int 
--- @field dstOffset int 
--- @overload fun():prosper.BufferCopyInfo
prosper.BufferCopyInfo = {}


--- 
--- @class prosper.Image
prosper.Image = {}

--- 
--- @param cmd prosper.CommandBuffer
--- @return prosper.Image ret0
--- @overload fun(cmd: prosper.CommandBuffer, imgCreateInfo: prosper.ImageCreateInfo): prosper.Image
function prosper.Image:Copy(cmd) end

--- 
--- @return bool ret0
function prosper.Image:IsNormalMap() end

--- 
--- @return enum prosper::SampleCountFlags ret0
function prosper.Image:GetSampleCount() end

--- 
--- @param 1 prosper.Image
function prosper.Image:__eq(arg1) end

--- 
--- @return prosper.SubresourceLayout ret0
--- @overload fun(layer: int): prosper.SubresourceLayout
--- @overload fun(arg1: int, arg2: int): prosper.SubresourceLayout
function prosper.Image:GetAspectSubresourceLayout() end

--- 
--- @return bool ret0
function prosper.Image:IsSrgb() end

--- 
--- @param arg1 bool
function prosper.Image:SetSrgb(arg1) end

--- 
function prosper.Image:__tostring() end

--- 
--- @param x int
--- @param y int
--- @param imgBuf util.ImageBuffer
--- @overload fun(x: int, y: int, imgBuf: util.ImageBuffer, layerIndex: int): 
--- @overload fun(x: int, y: int, imgBuf: util.ImageBuffer, layerIndex: int, mipLevel: int): 
function prosper.Image:WriteMemory(x, y, imgBuf) end

--- 
--- @param mipmap int
--- @return int ret0_1
--- @return int ret0_2
function prosper.Image:GetMipmapSize(mipmap) end

--- 
--- @return int ret0
function prosper.Image:GetLayerCount() end

--- 
--- @return int ret0
function prosper.Image:GetMipmapCount() end

--- 
--- @return enum prosper::ImageUsageFlags ret0
function prosper.Image:GetUsage() end

--- 
--- @return enum prosper::SharingMode ret0
function prosper.Image:GetSharingMode() end

--- 
--- @return enum prosper::ImageTiling ret0
function prosper.Image:GetTiling() end

--- 
--- @return prosper.ImageCreateInfo ret0
function prosper.Image:GetCreateInfo() end

--- 
--- @param info prosper.Image.ToImageBufferInfo
--- @overload fun(includeLayers: bool, includeMipmaps: bool): 
--- @overload fun(includeLayers: bool, includeMipmaps: bool, targetFormat: int): 
--- @overload fun(includeLayers: bool, includeMipmaps: bool, targetFormat: int, inputImageLayout: enum prosper::ImageLayout): 
function prosper.Image:ToImageBuffer(info) end

--- 
--- @return prosper.Buffer ret0
function prosper.Image:GetMemoryBuffer() end

--- 
--- @param cmd prosper.CommandBuffer
--- @param format enum prosper::Format
--- @return prosper.Image ret0
function prosper.Image:Convert(cmd, format) end

--- 
--- @param arg1 int
--- @return int ret0
--- @overload fun(arg1: int): int
function prosper.Image:GetWidth(arg1) end

--- 
--- @return enum prosper::ImageType ret0
function prosper.Image:GetType() end

--- 
--- @param name string
function prosper.Image:SetDebugName(name) end

--- 
--- @param arg1 int
--- @return int ret0
--- @overload fun(arg1: int): int
function prosper.Image:GetHeight(arg1) end

--- 
--- @return string ret0
function prosper.Image:GetDebugName() end

--- 
--- @return bool ret0
function prosper.Image:IsValid() end

--- 
--- @return int ret0
function prosper.Image:GetAlignment() end

--- 
--- @return enum prosper::Format ret0
function prosper.Image:GetFormat() end

--- 
--- @param arg1 bool
function prosper.Image:SetNormalMap(arg1) end

--- 
--- @param mipmap int
--- @return int ret0_1
--- @return int ret0_2
function prosper.Image:GetExtent2D(mipmap) end


--- 
--- @class prosper.Image.ToImageBufferInfo
--- @field inputImageLayout prosper::ImageLayout 
--- @field stagingImage prosper.Image 
--- @field targetFormat uimg::Format 
--- @field includeLayers bool 
--- @field includeMipmaps bool 
--- @overload fun():prosper.Image.ToImageBufferInfo
prosper.Image.ToImageBufferInfo = {}


--- 
--- @class prosper.ImageCopyInfo
--- @field srcSubresource  
--- @field height int 
--- @field srcOffset math.Vectori 
--- @field dstSubresource  
--- @field dstOffset math.Vectori 
--- @field width int 
--- @field srcImageLayout int 
--- @field dstImageLayout int 
--- @overload fun():prosper.ImageCopyInfo
prosper.ImageCopyInfo = {}


--- 
--- @class prosper.RenderBuffer
prosper.RenderBuffer = {}


--- 
--- @class prosper.ClearImageInfo
--- @field subresourceRange prosper.ImageSubresourceRange 
--- @overload fun():prosper.ClearImageInfo
prosper.ClearImageInfo = {}


--- 
--- @class prosper.RenderPassCreateInfo
--- @overload fun():prosper.RenderPassCreateInfo
prosper.RenderPassCreateInfo = {}

--- 
--- @return int ret0
function prosper.RenderPassCreateInfo:AddSubPass() end

--- 
--- @param subPassId int
--- @param bEnabled bool
function prosper.RenderPassCreateInfo:SetSubPassDepthStencilAttachmentEnabled(subPassId, bEnabled) end

--- 
--- @param format int
--- @param initialLayout int
--- @param finalLayout int
--- @param loadOp int
--- @param storeOp int
--- @return int ret0
--- @overload fun(format: int, initialLayout: int, finalLayout: int, loadOp: int, storeOp: int, sampleCount: int): int
function prosper.RenderPassCreateInfo:AddAttachment(format, initialLayout, finalLayout, loadOp, storeOp) end

--- 
--- @param subPassId int
--- @param colorAttId int
function prosper.RenderPassCreateInfo:AddSubPassColorAttachment(subPassId, colorAttId) end

--- 
--- @param subPassId int
--- @param sourceSubPassId int
--- @param destinationSubPassId int
--- @param sourceStageMask int
--- @param destinationStageMask int
--- @param sourceAccessMask int
--- @param destinationAccessMask int
function prosper.RenderPassCreateInfo:AddSubPassDependency(subPassId, sourceSubPassId, destinationSubPassId, sourceStageMask, destinationStageMask, sourceAccessMask, destinationAccessMask) end


--- 
--- @class prosper.RenderPass
prosper.RenderPass = {}

--- 
--- @param 1 prosper.RenderPass
function prosper.RenderPass:__eq(arg1) end

--- 
--- @return bool ret0
function prosper.RenderPass:IsValid() end

--- 
function prosper.RenderPass:__tostring() end


--- 
--- @class prosper.Texture
prosper.Texture = {}

--- 
--- @return enum prosper::Format ret0
function prosper.Texture:GetFormat() end

--- 
--- @return int ret0
function prosper.Texture:GetHeight() end

--- 
--- @param arg1 prosper.ImageView
function prosper.Texture:SetImageView(arg1) end

--- 
--- @return int ret0
function prosper.Texture:GetWidth() end

--- 
--- @return bool ret0
function prosper.Texture:IsValid() end

--- 
--- @param 1 prosper.Texture
function prosper.Texture:__eq(arg1) end

--- 
function prosper.Texture:__tostring() end

--- 
--- @return string ret0
function prosper.Texture:GetDebugName() end

--- 
--- @return prosper.Sampler ret0
function prosper.Texture:GetSampler() end

--- 
--- @param arg1 int
--- @return prosper.ImageView ret0
--- @overload fun(): prosper.ImageView
function prosper.Texture:GetImageView(arg1) end

--- 
--- @param name string
function prosper.Texture:SetDebugName(name) end

--- 
--- @return prosper.Image ret0
function prosper.Texture:GetImage() end


--- 
--- @class prosper.ImageViewCreateInfo
--- @field swizzleRed int 
--- @field swizzleGreen int 
--- @field baseMipmap int 
--- @field swizzleBlue int 
--- @field swizzleAlpha int 
--- @field baseLayer int 
--- @field format int 
--- @field mipmapLevels int 
--- @field levelCount int 
--- @overload fun():prosper.ImageViewCreateInfo
prosper.ImageViewCreateInfo = {}


--- 
--- @class prosper.TimestampQuery
prosper.TimestampQuery = {}

--- 
--- @param 1 prosper.TimestampQuery
function prosper.TimestampQuery:__eq(arg1) end

--- 
--- @return bool ret0
function prosper.TimestampQuery:IsValid() end

--- 
function prosper.TimestampQuery:__tostring() end


--- 
--- @class prosper.Window
prosper.Window = {}

--- 
--- @return bool ret0
function prosper.Window:GetStickyMouseButtonsEnabled() end

--- 
--- @return math.Vector2 ret0
function prosper.Window:GetCursorPos() end

--- 
function prosper.Window:SwapBuffers() end

--- 
--- @return bool ret0
function prosper.Window:IsVisible() end

--- 
--- @param size math.Vector2i
function prosper.Window:SetSize(size) end

--- 
--- @return bool ret0
function prosper.Window:IsFocused() end

--- 
--- @return bool ret0
function prosper.Window:IsResizable() end

--- 
--- @return math.Vector2 ret0_1
--- @return math.Vector2 ret0_2
--- @return math.Vector2 ret0_3
--- @return math.Vector2 ret0_4
function prosper.Window:GetMonitorBounds() end

--- 
--- @return bool ret0
function prosper.Window:ShouldClose() end

--- 
--- @return bool ret0
function prosper.Window:IsDecorated() end

--- 
--- @return math.Vector2i ret0
function prosper.Window:GetSize() end

--- 
--- @param title string
function prosper.Window:SetWindowTitle(title) end

--- 
--- @param shouldClose bool
function prosper.Window:SetShouldClose(shouldClose) end

--- 
--- @return string ret0
function prosper.Window:GetClipboardString() end

--- 
--- @param str string
function prosper.Window:SetClipboardString(str) end

--- 
--- @param pos math.Vector2
function prosper.Window:SetCursorPos(pos) end

--- 
--- @param enabled bool
function prosper.Window:SetStickyKeysEnabled(enabled) end

--- 
--- @param b prosper.Window
--- @return bool ret0
function prosper.Window:__eq(b) end

--- 
--- @param function unknown
function prosper.Window:AddCloseListener(function_) end

--- 
--- @return bool ret0
function prosper.Window:GetStickyKeysEnabled() end

--- 
--- @return int ret0
function prosper.Window:GetSwapchainImageCount() end

--- 
--- @param enabled bool
function prosper.Window:SetStickyMouseButtonsEnabled(enabled) end

--- 
--- @return math.Vector2i ret0
function prosper.Window:GetFramebufferSize() end

--- 
--- @return math.Vector2i ret0
function prosper.Window:GetPos() end

--- 
--- @return math.Vector4i ret0
function prosper.Window:GetFrameSize() end

--- 
function prosper.Window:Iconify() end

--- 
function prosper.Window:Restore() end

--- 
function prosper.Window:Maximize() end

--- 
function prosper.Window:MakeContextCurrent() end

--- 
function prosper.Window:Show() end

--- 
function prosper.Window:Hide() end

--- 
function prosper.Window:ClearCursorPosOverride() end

--- 
--- @param pos math.Vector2
function prosper.Window:SetCursorPosOverride(pos) end

--- 
--- @return bool ret0
function prosper.Window:IsMaximized() end

--- 
--- @param cursor class GLFW::Cursor
function prosper.Window:SetCursor(cursor) end

--- 
--- @return bool ret0
function prosper.Window:IsIconified() end

--- 
--- @return bool ret0
function prosper.Window:IsFloating() end

--- 
function prosper.Window:ClearCursor() end

--- 
--- @param key enum GLFW::Key
--- @return enum GLFW::KeyState ret0
function prosper.Window:GetKeyState(key) end

--- 
--- @param pos math.Vector2i
function prosper.Window:SetPos(pos) end

--- 
--- @param mouseButton enum GLFW::MouseButton
--- @return enum GLFW::KeyState ret0
function prosper.Window:GetMouseButtonState(mouseButton) end

--- 
--- @param cursorMode enum GLFW::CursorMode
function prosper.Window:SetCursorInputMode(cursorMode) end

--- 
--- @return enum GLFW::CursorMode ret0
function prosper.Window:GetCursorInputMode() end

--- 
--- @return bool ret0
function prosper.Window:IsValid() end

--- 
function prosper.Window:Close() end


--- 
--- @class prosper.BlitInfo
--- @field offsetDst math.Vector2i 
--- @field extentsSrc math.Vector2i 
--- @field extentsDst math.Vector2i 
--- @field srcSubresourceLayer  
--- @field dstSubresourceLayer  
--- @field offsetSrc math.Vector2i 
--- @overload fun():prosper.BlitInfo
prosper.BlitInfo = {}


--- 
--- @class prosper.ClearValue
--- @overload fun(arg1: number):prosper.ClearValue
--- @overload fun(arg1: number, arg2: int):prosper.ClearValue
--- @overload fun(arg1: util.Color):prosper.ClearValue
--- @overload fun():prosper.ClearValue
prosper.ClearValue = {}

--- 
--- @param clearColor util.Color
function prosper.ClearValue:SetColor(clearColor) end

--- 
--- @param depth number
--- @param stencil int
--- @overload fun(depth: number): 
function prosper.ClearValue:SetDepthStencil(depth, stencil) end


--- 
--- @class prosper.SamplerCreateInfo
--- @field maxLod number 
--- @field borderColor int 
--- @field magFilter int 
--- @field addressModeV int 
--- @field maxAnisotropy number 
--- @field mipLodBias number 
--- @field addressModeW int 
--- @field minFilter int 
--- @field mipmapMode int 
--- @field addressModeU int 
--- @field compareEnable bool 
--- @field compareOp int 
--- @field minLod number 
--- @overload fun():prosper.SamplerCreateInfo
prosper.SamplerCreateInfo = {}


--- 
--- @class prosper.ImageView
prosper.ImageView = {}

--- 
--- @return string ret0
function prosper.ImageView:GetDebugName() end

--- 
--- @param name string
function prosper.ImageView:SetDebugName(name) end

--- 
--- @return bool ret0
function prosper.ImageView:IsValid() end

--- 
--- @param 1 prosper.ImageView
function prosper.ImageView:__eq(arg1) end

--- 
--- @return enum prosper::Format ret0
function prosper.ImageView:GetImageFormat() end

--- 
function prosper.ImageView:__tostring() end

--- 
--- @return int ret0
function prosper.ImageView:GetLayerCount() end

--- 
--- @return enum prosper::ImageAspectFlags ret0
function prosper.ImageView:GetAspectMask() end

--- 
--- @return enum prosper::ImageViewType ret0
function prosper.ImageView:GetType() end

--- 
--- @return int ret0
function prosper.ImageView:GetBaseMipmapLevel() end

--- 
--- @return class std::array<enum prosper::ComponentSwizzle,4> ret0
function prosper.ImageView:GetSwizzleArray() end

--- 
--- @return int ret0
function prosper.ImageView:GetMipmapCount() end

--- 
--- @return int ret0
function prosper.ImageView:GetBaseLayer() end


--- 
--- @class prosper.BufferCreateInfo
--- @field usageFlags int 
--- @field memoryFeatures int 
--- @field size int 
--- @field flags int 
--- @field queueFamilyMask int 
--- @overload fun():prosper.BufferCreateInfo
prosper.BufferCreateInfo = {}


--- @enum FlagSparse
prosper.BufferCreateInfo = {
	FLAG_SPARSE_ALIASED_RESIDENCY_BIT = 8,
	FLAG_SPARSE_BIT = 4,
}

--- @enum Flag
prosper.BufferCreateInfo = {
	FLAG_NONE = 0,
	FLAG_CONCURRENT_SHARING_BIT = 1,
	FLAG_DONT_ALLOCATE_MEMORY_BIT = 2,
}

--- 
--- @class prosper.ImageSubresourceLayers
--- @field aspectMask int 
--- @field layerCount int 
--- @field mipLevel int 
--- @field baseArrayLayer int 
--- @overload fun():prosper.ImageSubresourceLayers
prosper.ImageSubresourceLayers = {}


--- 
--- @class prosper.TimerQuery
prosper.TimerQuery = {}

--- 
--- @param 1 prosper.TimerQuery
function prosper.TimerQuery:__eq(arg1) end

--- 
--- @return bool ret0
function prosper.TimerQuery:IsValid() end

--- 
function prosper.TimerQuery:__tostring() end


--- 
--- @class prosper.ImageCreateInfo
--- @field usageFlags int 
--- @field memoryFeatures int 
--- @field height int 
--- @field tiling int 
--- @field format int 
--- @field layers int 
--- @field samples int 
--- @field postCreateLayout int 
--- @field type int 
--- @field width int 
--- @field flags int 
--- @field queueFamilyMask int 
--- @overload fun():prosper.ImageCreateInfo
prosper.ImageCreateInfo = {}


--- @enum FlagSparse
prosper.ImageCreateInfo = {
	FLAG_SPARSE_ALIASED_RESIDENCY_BIT = 16,
	FLAG_SPARSE_BIT = 8,
}

--- @enum Flag
prosper.ImageCreateInfo = {
	FLAG_DONT_ALLOCATE_MEMORY_BIT = 64,
	FLAG_ALLOCATE_DISCRETE_MEMORY_BIT = 32,
	FLAG_NORMAL_MAP_BIT = 256,
	FLAG_NONE = 0,
	FLAG_CONCURRENT_SHARING_BIT = 2,
	FLAG_CUBEMAP_BIT = 1,
	FLAG_SRGB_BIT = 128,
	FLAG_FULL_MIPMAP_CHAIN_BIT = 4,
}

--- 
--- @class prosper.CommandBuffer
prosper.CommandBuffer = {}

--- 
--- @param minDepthBounds number
--- @param maxDepthBounds number
--- @return bool ret0
function prosper.CommandBuffer:RecordSetDepthBounds(minDepthBounds, maxDepthBounds) end

--- 
--- @param rpInfo prosper.RenderPassInfo
--- @return bool ret0
function prosper.CommandBuffer:RecordBeginRenderPass(rpInfo) end

--- 
--- @return bool ret0
function prosper.CommandBuffer:IsRecording() end

--- 
--- @param imgSrc prosper.Image
--- @param imgDst prosper.Image
--- @param blitInfo prosper.BlitInfo
--- @return bool ret0
function prosper.CommandBuffer:RecordBlitImage(imgSrc, imgDst, blitInfo) end

--- 
--- @param img prosper.Image
--- @param clearDepth number
--- @return bool ret0
--- @overload fun(img: prosper.Image, clearDepth: number, clearStencil: int): bool
--- @overload fun(img: prosper.Image, col: util.Color, attId: int): bool
function prosper.CommandBuffer:RecordClearAttachment(img, clearDepth) end

--- 
--- @param img prosper.Image
--- @param clearDepth number
--- @param clearStencil int
--- @return bool ret0
--- @overload fun(img: prosper.Image, clearDepth: number, clearStencil: int, clearImageInfo: prosper.ClearImageInfo): bool
--- @overload fun(img: prosper.Image, col: util.Color): bool
--- @overload fun(img: prosper.Image, col: util.Color, clearImageInfo: prosper.ClearImageInfo): bool
function prosper.CommandBuffer:RecordClearImage(img, clearDepth, clearStencil) end

--- 
--- @param imgSrc prosper.Image
--- @param imgDst prosper.Image
--- @param copyInfo prosper.ImageCopyInfo
--- @return bool ret0
function prosper.CommandBuffer:RecordCopyImage(imgSrc, imgDst, copyInfo) end

--- 
--- @param bufSrc prosper.Buffer
--- @param bufDst prosper.Buffer
--- @param copyInfo prosper.BufferCopyInfo
--- @return bool ret0
function prosper.CommandBuffer:RecordCopyBuffer(bufSrc, bufDst, copyInfo) end

--- 
--- @param bufSrc prosper.Buffer
--- @param imgDst prosper.Image
--- @return bool ret0
--- @overload fun(bufSrc: prosper.Buffer, imgDst: prosper.Image, copyInfo: prosper.BufferImageCopyInfo): bool
function prosper.CommandBuffer:RecordCopyBufferToImage(bufSrc, imgDst) end

--- 
--- @param buf prosper.Buffer
--- @param offset int
--- @param type enum udm::Type
--- @param value struct luabind::adl::udm_type<class luabind::adl::object,1,1,0>
--- @return bool ret0
--- @overload fun(buf: prosper.Buffer, offset: int, ds: util.DataStream): bool
function prosper.CommandBuffer:RecordUpdateBuffer(buf, offset, type, value) end

--- 
--- @param faceMask int
--- @param stencilWriteMask int
--- @return bool ret0
function prosper.CommandBuffer:RecordSetStencilWriteMask(faceMask, stencilWriteMask) end

--- 
--- @param depthBiasConstantFactor number
--- @param depthBiasClamp number
--- @param slopeScaledDepthBias number
--- @return bool ret0
function prosper.CommandBuffer:RecordSetDepthBias(depthBiasConstantFactor, depthBiasClamp, slopeScaledDepthBias) end

--- 
--- @param imgSrc prosper.Image
--- @param imgDst prosper.Image
--- @return bool ret0
function prosper.CommandBuffer:RecordResolveImage(imgSrc, imgDst) end

--- 
--- @param texSrc prosper.Texture
--- @param imgDst prosper.Image
--- @return bool ret0
function prosper.CommandBuffer:RecordBlitTexture(texSrc, imgDst) end

--- 
--- @param vertexCount int
--- @return bool ret0
--- @overload fun(vertexCount: int, instanceCount: int): bool
--- @overload fun(vertexCount: int, instanceCount: int, firstVertex: int): bool
--- @overload fun(vertexCount: int, instanceCount: int, firstVertex: int, firstInstance: int): bool
function prosper.CommandBuffer:RecordDraw(vertexCount) end

--- 
--- @param img prosper.Image
--- @param currentLayout int
--- @param srcAccessMask int
--- @param srcStage int
--- @return bool ret0
function prosper.CommandBuffer:RecordGenerateMipmaps(img, currentLayout, srcAccessMask, srcStage) end

--- 
--- @param barrierInfo prosper.PipelineBarrierInfo
--- @return bool ret0
function prosper.CommandBuffer:RecordPipelineBarrier(barrierInfo) end

--- 
--- @param img prosper.Image
--- @param srcStageMask int
--- @param dstStageMask int
--- @param oldLayout int
--- @param newLayout int
--- @param srcAccessMask int
--- @param dstAccessMask int
--- @return bool ret0
--- @overload fun(img: prosper.Image, srcStageMask: int, dstStageMask: int, oldLayout: int, newLayout: int, srcAccessMask: int, dstAccessMask: int, baseLayer: int): bool
--- @overload fun(img: prosper.Image, oldLayout: int, newLayout: int): bool
--- @overload fun(img: prosper.Image, oldLayout: int, newLayout: int, subresourceRange: prosper.ImageSubresourceRange): bool
function prosper.CommandBuffer:RecordImageBarrier(img, srcStageMask, dstStageMask, oldLayout, newLayout, srcAccessMask, dstAccessMask) end

--- 
--- @param buf prosper.Buffer
--- @param srcStageMask int
--- @param dstStageMask int
--- @param srcAccessMask int
--- @param dstAccessMask int
--- @return bool ret0
--- @overload fun(buf: prosper.Buffer, srcStageMask: int, dstStageMask: int, srcAccessMask: int, dstAccessMask: int, offset: int): bool
--- @overload fun(buf: prosper.Buffer, srcStageMask: int, dstStageMask: int, srcAccessMask: int, dstAccessMask: int, offset: int, size: int): bool
function prosper.CommandBuffer:RecordBufferBarrier(buf, srcStageMask, dstStageMask, srcAccessMask, dstAccessMask) end

--- 
--- @param width int
--- @param height int
--- @param x int
--- @param y int
--- @return bool ret0
function prosper.CommandBuffer:RecordSetViewport(width, height, x, y) end

--- 
--- @param width int
--- @param height int
--- @param x int
--- @param y int
--- @return bool ret0
function prosper.CommandBuffer:RecordSetScissor(width, height, x, y) end

--- 
--- @return bool ret0
function prosper.CommandBuffer:RecordEndRenderPass() end

--- 
--- @return bool ret0
function prosper.CommandBuffer:StopRecording() end

--- 
--- @param imgSrc prosper.Image
--- @param srcImageLayout int
--- @param bufDst prosper.Buffer
--- @param copyInfo prosper.BufferImageCopyInfo
--- @return bool ret0
function prosper.CommandBuffer:RecordCopyImageToBuffer(imgSrc, srcImageLayout, bufDst, copyInfo) end

--- 
--- @param buffer prosper.Buffer
--- @param offset int
--- @param drawCount int
--- @param stride int
--- @return bool ret0
function prosper.CommandBuffer:RecordDrawIndexedIndirect(buffer, offset, drawCount, stride) end

--- 
--- @param buffer prosper.Buffer
--- @param offset int
--- @param drawCount int
--- @param stride int
--- @return bool ret0
function prosper.CommandBuffer:RecordDrawIndirect(buffer, offset, drawCount, stride) end

--- 
--- @param buffer prosper.Buffer
--- @param offset int
--- @param size int
--- @param data int
--- @return bool ret0
function prosper.CommandBuffer:RecordFillBuffer(buffer, offset, size, data) end

--- 
--- @param blendConstants math.Vector4
--- @return bool ret0
function prosper.CommandBuffer:RecordSetBlendConstants(blendConstants) end

--- 
--- @param 1 prosper.CommandBuffer
function prosper.CommandBuffer:__eq(arg1) end

--- 
function prosper.CommandBuffer:__tostring() end

--- 
function prosper.CommandBuffer:Flush() end

--- 
--- @param indexBuffer prosper.Buffer
--- @param indexType int
--- @return bool ret0
--- @overload fun(indexBuffer: prosper.Buffer, indexType: int, offset: int): bool
function prosper.CommandBuffer:RecordBindIndexBuffer(indexBuffer, indexType) end

--- 
--- @param oneTimeSubmit bool
--- @param simultaneousUseAllowed bool
--- @return bool ret0
function prosper.CommandBuffer:StartRecording(oneTimeSubmit, simultaneousUseAllowed) end

--- 
--- @param rt prosper.RenderTarget
--- @param dir math.Vector2
--- @param lnodes any
--- @return bool ret0
function prosper.CommandBuffer:RecordDrawGradient(rt, dir, lnodes) end

--- 
--- @param faceMask int
--- @param stencilReference int
--- @return bool ret0
function prosper.CommandBuffer:RecordSetStencilReference(faceMask, stencilReference) end

--- 
--- @param faceMask int
--- @param stencilCompareMask int
--- @return bool ret0
function prosper.CommandBuffer:RecordSetStencilCompareMask(faceMask, stencilCompareMask) end

--- 
--- @param lineWidth number
--- @return bool ret0
function prosper.CommandBuffer:RecordSetLineWidth(lineWidth) end

--- 
--- @param name string
function prosper.CommandBuffer:SetDebugName(name) end

--- 
--- @return string ret0
function prosper.CommandBuffer:GetDebugName() end

--- 
--- @param graphics shader.Graphics
--- @param vertexBuffer prosper.Buffer
--- @return bool ret0
--- @overload fun(graphics: shader.Graphics, vertexBuffer: prosper.Buffer, startBinding: int): bool
--- @overload fun(graphics: shader.Graphics, vertexBuffer: prosper.Buffer, startBinding: int, offset: int): bool
function prosper.CommandBuffer:RecordBindVertexBuffer(graphics, vertexBuffer) end

--- 
--- @param graphics shader.Graphics
--- @param buffers table
--- @return bool ret0
--- @overload fun(graphics: shader.Graphics, buffers: table, startBinding: int): bool
--- @overload fun(graphics: shader.Graphics, buffers: table, startBinding: int, voffsets: table): bool
function prosper.CommandBuffer:RecordBindVertexBuffers(graphics, buffers) end

--- 
--- @param indexCount int
--- @return bool ret0
--- @overload fun(indexCount: int, instanceCount: int): bool
--- @overload fun(indexCount: int, instanceCount: int, firstIndex: int): bool
--- @overload fun(indexCount: int, instanceCount: int, firstIndex: int, firstInstance: int): bool
function prosper.CommandBuffer:RecordDrawIndexed(indexCount) end


--- 
--- @class prosper.SwapBuffer
prosper.SwapBuffer = {}


--- 
--- @class prosper.Mesh
prosper.Mesh = {}

--- 
--- @param arg1 prosper.Buffer
function prosper.Mesh:SetVertexBuffer(arg1) end

--- 
--- @param arg1 prosper.Buffer
function prosper.Mesh:SetAlphaBuffer(arg1) end

--- 
--- @param arg1 prosper.Buffer
--- @param arg2 enum pragma::model::IndexType
function prosper.Mesh:SetIndexBuffer(arg1, arg2) end

--- 
--- @return prosper.Buffer ret0
function prosper.Mesh:GetAlphaBuffer() end

--- 
--- @param arg1 prosper.Buffer
function prosper.Mesh:SetVertexWeightBuffer(arg1) end

--- 
function prosper.Mesh:__tostring() end

--- 
--- @return prosper.Buffer ret0
function prosper.Mesh:GetVertexBuffer() end

--- 
function prosper.Mesh:ClearBuffers() end

--- 
--- @param 1 prosper.Mesh
function prosper.Mesh:__eq(arg1) end

--- 
--- @return prosper.Buffer ret0
function prosper.Mesh:GetIndexBuffer() end

--- 
--- @return prosper.Buffer ret0
function prosper.Mesh:GetVertexWeightBuffer() end


--- 
--- @class prosper.RenderPassInfo
--- @overload fun(arg1: prosper.RenderTarget):prosper.RenderPassInfo
prosper.RenderPassInfo = {}

--- 
--- @param layerId int
--- @overload fun(): 
function prosper.RenderPassInfo:SetLayer(layerId) end

--- 
--- @param clearValues table
function prosper.RenderPassInfo:SetClearValues(clearValues) end

--- 
--- @param rp prosper.RenderPass
--- @overload fun(): 
function prosper.RenderPassInfo:SetRenderPass(rp) end

--- 
--- @param clearValue prosper.ClearValue
function prosper.RenderPassInfo:AddClearValue(clearValue) end


--- 
--- @class prosper.TextureCreateInfo
--- @field imageView prosper.ImageView 
--- @field sampler prosper.Sampler 
--- @field flags int 
--- @overload fun():prosper.TextureCreateInfo
prosper.TextureCreateInfo = {}


--- @enum Flag
prosper.TextureCreateInfo = {
	FLAG_CREATE_IMAGE_VIEW_FOR_EACH_LAYER = 2,
	FLAG_NONE = 0,
	FLAG_RESOLVABLE = 1,
}

--- 
--- @class prosper.Buffer
prosper.Buffer = {}

--- 
--- @return prosper.BufferCreateInfo ret0
function prosper.Buffer:GetCreateInfo() end

--- 
--- @param mapFlags enum prosper::IBuffer::MapFlags
--- @return bool ret0
--- @overload fun(offset: int, size: int, mapFlags: enum prosper::IBuffer::MapFlags): bool
function prosper.Buffer:MapMemory(mapFlags) end

--- 
--- @return int ret0
function prosper.Buffer:GetStartOffset() end

--- 
--- @return int ret0
function prosper.Buffer:GetSize() end

--- 
--- @return util.DataStream ret0
--- @overload fun(offset: int, size: int): util.DataStream
--- @overload fun(offset: int, type: enum udm::Type, value: struct luabind::adl::udm_type<class luabind::adl::object,1,1,0>): util.DataStream
--- @overload fun(offset: int, size: int, ds: util.DataStream): util.DataStream
--- @overload fun(offset: int, size: int, ds: util.DataStream, dsOffset: int): util.DataStream
function prosper.Buffer:ReadMemory() end

--- 
--- @return int ret0
function prosper.Buffer:GetBaseIndex() end

--- 
--- @return enum prosper::BufferUsageFlags ret0
function prosper.Buffer:GetUsageFlags() end

--- 
--- @param name string
function prosper.Buffer:SetDebugName(name) end

--- 
--- @return bool ret0
function prosper.Buffer:IsValid() end

--- 
function prosper.Buffer:__tostring() end

--- 
--- @param offset int
--- @param ds util.DataStream
--- @return bool ret0
--- @overload fun(offset: int, type: enum udm::Type, value: struct luabind::adl::udm_type<class luabind::adl::object,1,1,0>): bool
--- @overload fun(offset: int, ds: util.DataStream, dsOffset: int, dsSize: int): bool
function prosper.Buffer:WriteMemory(offset, ds) end

--- 
--- @param arg1 bool
--- @param arg2 enum prosper::IBuffer::MapFlags
function prosper.Buffer:SetPermanentlyMapped(arg1, arg2) end

--- 
--- @return bool ret0
function prosper.Buffer:UnmapMemory() end

--- 
--- @return string ret0
function prosper.Buffer:GetDebugName() end

--- 
--- @param 1 prosper.Buffer
function prosper.Buffer:__eq(arg1) end

--- 
--- @return prosper.Buffer ret0
function prosper.Buffer:GetParent() end


--- 
--- @class prosper.DescriptorSet
prosper.DescriptorSet = {}

--- 
--- @param bindingIdx int
--- @param buffer prosper.Buffer
--- @return bool ret0
--- @overload fun(bindingIdx: int, buffer: prosper.Buffer, startOffset: int): bool
--- @overload fun(bindingIdx: int, buffer: prosper.Buffer, startOffset: int, size: int): bool
function prosper.DescriptorSet:SetBindingUniformBufferDynamic(bindingIdx, buffer) end

--- 
--- @param bindingIdx int
--- @param buffer prosper.Buffer
--- @return bool ret0
--- @overload fun(bindingIdx: int, buffer: prosper.Buffer, startOffset: int): bool
--- @overload fun(bindingIdx: int, buffer: prosper.Buffer, startOffset: int, size: int): bool
function prosper.DescriptorSet:SetBindingStorageBuffer(bindingIdx, buffer) end

--- 
--- @return bool ret0
function prosper.DescriptorSet:Update() end

--- 
function prosper.DescriptorSet:__tostring() end

--- 
--- @return int ret0
function prosper.DescriptorSet:GetBindingCount() end

--- 
--- @param bindingIdx int
--- @param texture prosper.Texture
--- @return bool ret0
--- @overload fun(bindingIdx: int, texture: prosper.Texture, layerId: int): bool
function prosper.DescriptorSet:SetBindingTexture(bindingIdx, texture) end

--- 
--- @param name string
function prosper.DescriptorSet:SetDebugName(name) end

--- 
--- @param 1 prosper.DescriptorSet
function prosper.DescriptorSet:__eq(arg1) end

--- 
--- @return string ret0
function prosper.DescriptorSet:GetDebugName() end

--- 
--- @param bindingIdx int
--- @param texture prosper.Texture
--- @param arrayIdx int
--- @return bool ret0
--- @overload fun(bindingIdx: int, texture: prosper.Texture, arrayIdx: int, layerId: int): bool
function prosper.DescriptorSet:SetBindingArrayTexture(bindingIdx, texture, arrayIdx) end

--- 
--- @param bindingIdx int
--- @param buffer prosper.Buffer
--- @return bool ret0
--- @overload fun(bindingIdx: int, buffer: prosper.Buffer, startOffset: int): bool
--- @overload fun(bindingIdx: int, buffer: prosper.Buffer, startOffset: int, size: int): bool
function prosper.DescriptorSet:SetBindingUniformBuffer(bindingIdx, buffer) end


--- 
--- @class prosper.Fence
prosper.Fence = {}

--- 
--- @param 1 prosper.Fence
function prosper.Fence:__eq(arg1) end

--- 
--- @return bool ret0
function prosper.Fence:IsValid() end

--- 
function prosper.Fence:__tostring() end


--- 
--- @class prosper.Event
prosper.Event = {}

--- 
--- @param 1 prosper.Event
function prosper.Event:__eq(arg1) end

--- 
--- @return bool ret0
function prosper.Event:IsValid() end

--- 
function prosper.Event:__tostring() end


--- 
--- @class prosper.SubresourceLayout
--- @field offset int 
--- @field size int 
--- @field arrayPitch int 
--- @field rowPitch int 
--- @field depthPitch int 
--- @overload fun():prosper.SubresourceLayout
prosper.SubresourceLayout = {}


--- 
--- @class prosper.Framebuffer
prosper.Framebuffer = {}

--- 
--- @param 1 prosper.Framebuffer
function prosper.Framebuffer:__eq(arg1) end

--- 
--- @return int ret0
function prosper.Framebuffer:GetWidth() end

--- 
--- @return bool ret0
function prosper.Framebuffer:IsValid() end

--- 
--- @return int ret0
function prosper.Framebuffer:GetHeight() end

--- 
function prosper.Framebuffer:__tostring() end


--- 
--- @class prosper.RenderTarget
prosper.RenderTarget = {}

--- 
--- @return enum prosper::Format ret0
function prosper.RenderTarget:GetFormat() end

--- 
--- @return int ret0
function prosper.RenderTarget:GetHeight() end

--- 
--- @return prosper.Texture ret0
function prosper.RenderTarget:GetDepthStencilAttachmentTexture() end

--- 
--- @return int ret0
function prosper.RenderTarget:GetWidth() end

--- 
--- @param 1 prosper.RenderTarget
function prosper.RenderTarget:__eq(arg1) end

--- 
function prosper.RenderTarget:__tostring() end

--- 
--- @return string ret0
function prosper.RenderTarget:GetDebugName() end

--- 
--- @param arg1 int
--- @return prosper.Texture ret0
--- @overload fun(): prosper.Texture
function prosper.RenderTarget:GetTexture(arg1) end

--- 
--- @return prosper.Framebuffer ret0
function prosper.RenderTarget:GetFramebuffer() end

--- 
--- @param name string
function prosper.RenderTarget:SetDebugName(name) end

--- 
--- @param idx int
--- @return prosper.Texture ret0
--- @overload fun(idx: int): prosper.Texture
function prosper.RenderTarget:GetColorAttachmentTexture(idx) end

--- 
--- @return prosper.RenderPass ret0
function prosper.RenderTarget:GetRenderPass() end


--- 
--- @class prosper.WindowCreateInfo
--- @field greenBits int 
--- @field samples int 
--- @field resizable bool 
--- @field depthBits int 
--- @field focused bool 
--- @field visible bool 
--- @field decorated bool 
--- @field floating bool 
--- @field autoIconify bool 
--- @field stereo bool 
--- @field srgbCapable bool 
--- @field doublebuffer bool 
--- @field height int 
--- @field alphaBits int 
--- @field stencilBits int 
--- @field title string 
--- @field width int 
--- @field redBits int 
--- @field blueBits int 
--- @field refreshRate int 
--- @overload fun():prosper.WindowCreateInfo
prosper.WindowCreateInfo = {}


--- 
--- @class prosper.CommandBufferRecorder
prosper.CommandBufferRecorder = {}

--- 
--- @param 1 prosper.CommandBufferRecorder
function prosper.CommandBufferRecorder:__eq(arg1) end

--- 
--- @return bool ret0
function prosper.CommandBufferRecorder:IsValid() end

--- 
--- @param drawCmd prosper.CommandBuffer
--- @return bool ret0
function prosper.CommandBufferRecorder:ExecuteCommands(drawCmd) end

--- 
function prosper.CommandBufferRecorder:__tostring() end

--- 
--- @return bool ret0
function prosper.CommandBufferRecorder:IsPending() end


--- 
--- @class prosper.Sampler
prosper.Sampler = {}

--- 
--- @return enum prosper::Filter ret0
function prosper.Sampler:GetMagFilter() end

--- 
--- @param arg1 number
function prosper.Sampler:SetMinLod(arg1) end

--- 
--- @param arg1 enum prosper::SamplerAddressMode
function prosper.Sampler:SetAddressModeV(arg1) end

--- 
--- @return enum prosper::SamplerMipmapMode ret0
function prosper.Sampler:GetMipmapMode() end

--- 
--- @return bool ret0
function prosper.Sampler:GetAnisotropyEnabled() end

--- 
--- @return string ret0
function prosper.Sampler:GetDebugName() end

--- 
--- @return enum prosper::SamplerAddressMode ret0
function prosper.Sampler:GetAddressModeU() end

--- 
--- @return enum prosper::SamplerAddressMode ret0
function prosper.Sampler:GetAddressModeV() end

--- 
function prosper.Sampler:__tostring() end

--- 
--- @return number ret0
function prosper.Sampler:GetMipLodBias() end

--- 
--- @param arg1 enum prosper::CompareOp
function prosper.Sampler:SetCompareOperation(arg1) end

--- 
--- @return enum prosper::SamplerAddressMode ret0
function prosper.Sampler:GetAddressModeW() end

--- 
--- @return bool ret0
function prosper.Sampler:IsValid() end

--- 
--- @param arg1 enum prosper::SamplerAddressMode
function prosper.Sampler:SetAddressModeU(arg1) end

--- 
--- @return enum prosper::CompareOp ret0
function prosper.Sampler:GetCompareOperation() end

--- 
--- @return number ret0
function prosper.Sampler:GetMinLod() end

--- 
--- @return bool ret0
function prosper.Sampler:Update() end

--- 
--- @return enum prosper::BorderColor ret0
function prosper.Sampler:GetBorderColor() end

--- 
--- @param arg1 enum prosper::Filter
function prosper.Sampler:SetMagFilter(arg1) end

--- 
--- @param arg1 enum prosper::Filter
function prosper.Sampler:SetMinFilter(arg1) end

--- 
--- @param arg1 enum prosper::SamplerMipmapMode
function prosper.Sampler:SetMipmapMode(arg1) end

--- 
--- @param arg1 enum prosper::SamplerAddressMode
function prosper.Sampler:SetAddressModeW(arg1) end

--- 
--- @param arg1 number
function prosper.Sampler:SetMaxAnisotropy(arg1) end

--- 
--- @param arg1 bool
function prosper.Sampler:SetCompareEnabled(arg1) end

--- 
--- @param arg1 number
function prosper.Sampler:SetMaxLod(arg1) end

--- 
--- @param arg1 enum prosper::BorderColor
function prosper.Sampler:SetBorderColor(arg1) end

--- 
--- @param name string
function prosper.Sampler:SetDebugName(name) end

--- 
--- @param 1 prosper.Sampler
function prosper.Sampler:__eq(arg1) end

--- 
--- @return number ret0
function prosper.Sampler:GetMaxLod() end

--- 
--- @return bool ret0
function prosper.Sampler:GetCompareEnabled() end

--- 
--- @return number ret0
function prosper.Sampler:GetMaxAnisotropy() end

--- 
--- @return enum prosper::Filter ret0
function prosper.Sampler:GetMinFilter() end


--- 
--- @class prosper.ImageSubresourceRange
--- @field baseArrayLayer int 
--- @field layerCount int 
--- @field baseMipLevel int 
--- @field levelCount int 
--- @overload fun(arg1: int, arg2: int, arg3: int, arg4: int):prosper.ImageSubresourceRange
--- @overload fun(arg1: int, arg2: int, arg3: int):prosper.ImageSubresourceRange
--- @overload fun(arg1: int, arg2: int):prosper.ImageSubresourceRange
--- @overload fun(arg1: int):prosper.ImageSubresourceRange
--- @overload fun():prosper.ImageSubresourceRange
prosper.ImageSubresourceRange = {}


--- 
--- @class prosper.BufferImageCopyInfo
--- @field dstImageLayout int 
--- @field imageSize math.Vector2i 
--- @field aspectMask int 
--- @field layerCount int 
--- @field mipLevel int 
--- @field baseArrayLayer int 
--- @field bufferOffset int 
--- @overload fun():prosper.BufferImageCopyInfo
prosper.BufferImageCopyInfo = {}

--- 
--- @return math.Vector2i ret0
function prosper.BufferImageCopyInfo:GetImageSize() end

--- 
--- @param width int
--- @param height int
function prosper.BufferImageCopyInfo:SetImageSize(width, height) end


--- 
--- @class prosper.RenderTargetCreateInfo
--- @field useLayerFramebuffers bool 
--- @overload fun():prosper.RenderTargetCreateInfo
prosper.RenderTargetCreateInfo = {}


