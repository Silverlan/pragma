-- SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

include("/shaders/example_2d/shader.lua")

local shader2d = shader.get("example_2d")
if shader2d == nil then
	return
end

if example_shader ~= nil then
	-- Cleanup, if this script was called previously
	util.remove(example_shader.guiElement)
end
example_shader = {}

-- Load image
local imgBuf = util.load_image("materials/third_party/lua_logo.png", false)
if imgBuf == nil then
	return
end

-- Create texture
local imgCreateInfo = prosper.create_image_create_info(imgBuf)
local imgIn = prosper.create_image(imgBuf, imgCreateInfo)
local texIn = prosper.create_texture(
	imgIn,
	prosper.TextureCreateInfo(),
	prosper.ImageViewCreateInfo(),
	prosper.SamplerCreateInfo()
)

-- Create render target
imgCreateInfo.usageFlags = bit.bor(imgCreateInfo.usageFlags, prosper.IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
local imgOut = prosper.create_image(imgCreateInfo)
local texOut = prosper.create_texture(
	imgOut,
	prosper.TextureCreateInfo(),
	prosper.ImageViewCreateInfo(),
	prosper.SamplerCreateInfo()
)
local rt = prosper.create_render_target(prosper.RenderTargetCreateInfo(), texOut, shader2d:GetRenderPass())

-- Create descriptor set for input texture
local dsTex = shader2d:CreateDescriptorSet(shader.Example2d.DESCRIPTOR_SET_TEXTURE)
dsTex:SetBindingTexture(shader.Example2d.TEXTURE_BINDING_TEXTURE, texIn)

-- Apply shader
local drawCmd = game.get_setup_command_buffer()
if drawCmd:RecordBeginRenderPass(prosper.RenderPassInfo(rt)) then
	shader2d:Draw(drawCmd, dsTex)
	drawCmd:RecordEndRenderPass()
end
game.flush_setup_command_buffer()

-- Display via GUI element
local el = gui.create("WITexturedRect")
el:SetSize(512, 512)
el:SetTexture(texOut)

example_shader.guiElement = el
