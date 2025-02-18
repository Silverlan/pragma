--[[
    Copyright (C) 2021 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

local LOGGER = log.register_logger("image_processor")

util.register_class("util.ImageProcessor")

util.register_class("util.ImageProcessor.Stage")
function util.ImageProcessor.Stage:__init(f)
	self:SetEnabled(true)
	self.m_executor = f
end
function util.ImageProcessor.Stage:Apply(drawCmd, dsTex, rtDst)
	LOGGER:Info("Applying stage...")
	self.m_executor(drawCmd, dsTex, rtDst)
end
function util.ImageProcessor.Stage:SetEnabled(enabled)
	self.m_enabled = enabled
end
function util.ImageProcessor.Stage:IsEnabled()
	return self.m_enabled
end

----------------

function util.ImageProcessor:__init(w, h)
	self.m_stages = {}
	self.m_nameToStage = {}
	self.m_textures = {}
	self.m_width = w
	self.m_height = h
	self:Reset()
end

function util.ImageProcessor:CreateTextureDescriptorSet(tex)
	local dsInfo = shader.DescriptorSetInfo("TEXTURE", {
		shader.DescriptorSetBinding(
			"TEXTURE",
			prosper.DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			prosper.SHADER_STAGE_FRAGMENT_BIT
		),
	})
	local ds = prosper.create_descriptor_set(dsInfo)
	ds:SetBindingTexture(0, tex)
	return ds
end

function util.ImageProcessor:SetInputTexture(tex)
	local ds = self:CreateTextureDescriptorSet(tex)
	self.m_inputTexture = {
		texture = tex,
		descriptorSet = ds,
	}
end

function util.ImageProcessor:GetInputTexture()
	return (self.m_inputTexture ~= nil) and self.m_inputTexture.texture or nil
end
function util.ImageProcessor:GetTexture(i)
	return (self.m_textures[i] ~= nil) and self.m_textures[i].texture or nil
end

function util.ImageProcessor:GetWidth()
	return self.m_width
end
function util.ImageProcessor:GetHeight()
	return self.m_height
end

function util.ImageProcessor:AddStage(name, stage)
	if type(stage) == "function" then
		stage = util.ImageProcessor.Stage(stage)
	end
	table.insert(self.m_stages, stage)
	self.m_nameToStage[name] = stage
end

function util.ImageProcessor:SetStageEnabled(name, enabled)
	if self.m_nameToStage[name] == nil then
		return
	end
	self.m_nameToStage[name]:SetEnabled(enabled)
end

function util.ImageProcessor:AcquireTexture(drawCmd)
	LOGGER:Info("Acquiring texture...")
	local curTex = self.m_curTexture
	local prevTex = self.m_prevTexture
	if prevTex ~= nil and #self.m_textures == 1 then
		LOGGER:Warn("Only one texture available, skipping...")
		return
	end
	local tex = self.m_textures[curTex]
	if tex == nil then
		LOGGER:Warn("No texture available, skipping...")
		return
	end
	self.m_prevTexture = curTex
	self.m_curTexture = self.m_curTexture + 1
	if self.m_curTexture == #self.m_textures + 1 then
		self.m_curTexture = 1
	end

	local srcData = (prevTex ~= nil) and self.m_textures[prevTex] or nil
	local texSrc = (srcData ~= nil) and srcData.texture or nil
	local texDst = tex.texture
	if texSrc ~= nil then
		--[[LOGGER:Debug(
			"Transitioning source texture '"
				.. texSrc:GetImage():GetDebugName()
				.. "' from COLOR_ATTACHMENT to SHADER_READ_ONLY"
		)
		drawCmd:RecordImageBarrier(
			texSrc:GetImage(),
			prosper.IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			prosper.IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		)]]
	end

	LOGGER:Debug(
		"Transitioning destination texture '"
			.. texDst:GetImage():GetDebugName()
			.. "' from SHADER_READ_ONLY to COLOR_ATTACHMENT"
	)
	drawCmd:RecordImageBarrier(
		texDst:GetImage(),
		prosper.IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		prosper.IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	)

	local dsSrc = (srcData ~= nil) and srcData.descriptorSet or nil
	if dsSrc == nil then
		dsSrc = self.m_inputTexture.descriptorSet
	end
	return tex.renderTarget, dsSrc, texSrc
end

function util.ImageProcessor:Reset()
	self.m_prevTexture = nil
	self.m_curTexture = 1
end

function util.ImageProcessor:AddStagingTexture(createInfo)
	createInfo.usageFlags =
		bit.bor(createInfo.usageFlags, prosper.IMAGE_USAGE_COLOR_ATTACHMENT_BIT, prosper.IMAGE_USAGE_SAMPLED_BIT)
	createInfo.tiling = prosper.IMAGE_TILING_OPTIMAL
	createInfo.memoryFeatures = prosper.MEMORY_FEATURE_GPU_BULK_BIT
	createInfo.postCreateLayout = prosper.IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	local img = prosper.create_image(createInfo)
	img:SetDebugName("image_processor_staging_tex")
	local samplerCreateInfo = prosper.SamplerCreateInfo()
	samplerCreateInfo.addressModeU = prosper.SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE -- TODO: This should be the default for the SamplerCreateInfo struct; TODO: Add additional constructors
	samplerCreateInfo.addressModeV = prosper.SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
	samplerCreateInfo.addressModeW = prosper.SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
	local tex =
		prosper.create_texture(img, prosper.TextureCreateInfo(), prosper.ImageViewCreateInfo(), samplerCreateInfo)
	tex:SetDebugName("image_processor_staging_tex")
	LOGGER:Debug(
		"Created staging texture '"
			.. tex:GetImage():GetDebugName()
			.. "' with initial layout '"
			.. prosper.image_layout_to_string(createInfo.postCreateLayout)
			.. "'"
	)

	local rpCreateInfo = prosper.RenderPassCreateInfo()
	rpCreateInfo:AddAttachment(
		createInfo.format,
		prosper.IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		prosper.IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		prosper.ATTACHMENT_LOAD_OP_LOAD,
		prosper.ATTACHMENT_STORE_OP_STORE
	)
	local rp = prosper.create_render_pass(rpCreateInfo)
	local rt = prosper.create_render_target(prosper.RenderTargetCreateInfo(), { tex }, rp)
	rt:SetDebugName("image_processor_staging_rt")

	local dsInfo = shader.DescriptorSetInfo("TEXTURE", {
		shader.DescriptorSetBinding(
			"TEXTURE",
			prosper.DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			prosper.SHADER_STAGE_FRAGMENT_BIT
		),
	})
	local ds = prosper.create_descriptor_set(dsInfo)
	ds:SetBindingTexture(0, tex)
	table.insert(self.m_textures, {
		renderTarget = rt,
		texture = tex,
		descriptorSet = ds,
	})
end

function util.ImageProcessor:Apply(drawCmd, inputTex)
	self:Reset()
	LOGGER:Info("Applying image processor...")
	local tex
	for _, stage in ipairs(self.m_stages) do
		if stage:IsEnabled() then
			local rtDst, dsSrc, texSrc = self:AcquireTexture(drawCmd)
			if rtDst == nil then
				break
			end
			LOGGER:Info("Beginning render pass with render target '" .. rtDst:GetDebugName() .. "'")
			if drawCmd:RecordBeginRenderPass(prosper.RenderPassInfo(rtDst)) then
				stage:Apply(drawCmd, dsSrc, rtDst)

				LOGGER:Info("Ending render pass")
				drawCmd:RecordEndRenderPass()
			end
			tex = rtDst:GetTexture()
			LOGGER:Debug(
				"Image '"
					.. tex:GetImage():GetDebugName()
					.. "' is now in layout '"
					.. prosper.image_layout_to_string(rtDst:GetRenderPass():GetFinalLayout(0))
					.. "'"
			)
		end
	end
	if tex == nil then
		LOGGER:Warn("No texture available, skipping...")
		return
	end
	LOGGER:Info("Image processor applied successfully!")
	return tex
end

function util.ImageProcessor:Finalize(drawCmd)
	LOGGER:Info("Finalizing image processor...")
	local texInput = self:GetInputTexture()
	local lastTexInfo = self.m_textures[self.m_prevTexture]
	if texInput == nil or lastTexInfo == nil then
		LOGGER:Warn("No input texture or last texture available, skipping...")
		return false
	end
	local lastTex = lastTexInfo.texture
	LOGGER:Debug("Transitioning input texture '" .. texInput:GetImage():GetDebugName() .. "' to TRANSFER_DST")
	drawCmd:RecordImageBarrier(
		texInput:GetImage(),
		prosper.IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		prosper.IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	)
	LOGGER:Debug("Transitioning last texture '" .. lastTex:GetImage():GetDebugName() .. "' to TRANSFER_SRC")
	drawCmd:RecordImageBarrier(
		lastTex:GetImage(),
		prosper.IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		prosper.IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
	)

	LOGGER:Info("Blitting last texture to input texture...")
	drawCmd:RecordBlitImage(lastTex:GetImage(), texInput:GetImage(), prosper.BlitInfo())

	LOGGER:Debug("Transitioning input texture '" .. texInput:GetImage():GetDebugName() .. "' to SHADER_READ_ONLY")
	drawCmd:RecordImageBarrier(
		texInput:GetImage(),
		prosper.IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		prosper.IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	)
	LOGGER:Debug("Transitioning last texture '" .. lastTex:GetImage():GetDebugName() .. "' to SHADER_READ_ONLY")
	drawCmd:RecordImageBarrier(
		lastTex:GetImage(),
		prosper.IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		prosper.IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	)
	return true
end
