--[[
    Copyright (C) 2024 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

local cvRampTexture = console.register_variable(
	"toon_shader_ramp_texture",
	udm.TYPE_STRING,
	"toon/ramp_2band",
	bit.bor(console.FLAG_BIT_ARCHIVE),
	"The ramp texture to use for the toon shader."
)

console.add_change_callback("toon_shader_ramp_texture", function(old, new)
	local sh = shader.get("toon")
	if util.is_valid(sh) == false then
		return
	end
	sh:GetWrapper():InitializeRampTexture()
end)

local Shader = util.register_class("shader.ToonShader", shader.BaseTexturedLit3D)

Shader.FragmentShader = "programs/scene/toon"
Shader.VertexShader = "programs/scene/textured"
Shader.ShaderMaterial = "toon"
function Shader:InitializePipeline(pipelineInfo, pipelineIdx)
	shader.BaseTexturedLit3D.InitializePipeline(self, pipelineInfo, pipelineIdx)
end
function Shader:InitializeRampTexture()
	if self.m_rampDescriptorSet == nil then
		return
	end
	prosper.wait_idle(true)

	local textures = {
		cvRampTexture:GetString(),
		"toon/ramp_texture",
		"white",
	}
	for i, texName in ipairs(textures) do
		local tex = asset.load(texName, asset.TYPE_TEXTURE)
		if tex ~= nil then
			self.m_rampTexture = tex
			self.m_rampDescriptorSet:SetBindingTexture(0, tex:GetVkTexture())
			break
		else
			if i == #textures then
				log.error("Failed to load ramp texture '" .. texName .. "' for toon shader!")
			else
				log.error(
					"Failed to load ramp texture '"
						.. texName
						.. "' for toon shader! Falling back to '"
						.. textures[i + 1]
						.. "'..."
				)
			end
		end
	end
end
function Shader:InitializeShaderResources()
	shader.BaseTexturedLit3D.InitializeShaderResources(self)
	local rampDsInfo = shader.DescriptorSetInfo("RAMP", {
		shader.DescriptorSetBinding(
			"TEXTURE",
			prosper.DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			prosper.SHADER_STAGE_FRAGMENT_BIT
		),
	})
	self:AttachDescriptorSetInfo(rampDsInfo)
end
function Shader:OnInitializationComplete()
	local setIdx = self:GetShader():FindDescriptorSetIndex("RAMP")
	if setIdx == nil then
		return
	end
	self.m_rampDescriptorSet = self:GetShader():CreateDescriptorSet(setIdx)
	self:InitializeRampTexture()

	local pcb = self:GetBindPcb()
	pcb:RecordBindDescriptorSet(self.m_rampDescriptorSet, setIdx)
end
shader.register("toon", Shader)
