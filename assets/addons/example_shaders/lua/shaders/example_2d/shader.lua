--[[
    Copyright (C) 2021 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

local Shader = util.register_class("shader.Example2d", shader.BaseGraphics)

Shader.DESCRIPTOR_SET_TEXTURE = 0
Shader.TEXTURE_BINDING_TEXTURE = 0

Shader.FragmentShader = "examples/fs_example_2d"
Shader.VertexShader = "screen/vs_screen_uv"

function Shader:InitializePipeline(pipelineInfo, pipelineIdx)
	shader.BaseGraphics.InitializePipeline(self, pipelineInfo, pipelineIdx)
	pipelineInfo:AttachVertexAttribute(shader.VertexBinding(prosper.VERTEX_INPUT_RATE_VERTEX), {
		shader.VertexAttribute(prosper.FORMAT_R32G32_SFLOAT), -- Position
		shader.VertexAttribute(prosper.FORMAT_R32G32_SFLOAT), -- UV
	})
	pipelineInfo:AttachDescriptorSetInfo(shader.DescriptorSetInfo("TEXTURE", {
		shader.DescriptorSetBinding(
			"TEXTURE",
			prosper.DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			prosper.SHADER_STAGE_FRAGMENT_BIT
		),
	}))

	pipelineInfo:SetPolygonMode(prosper.POLYGON_MODE_FILL)
	pipelineInfo:SetPrimitiveTopology(prosper.PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
end
function Shader:InitializeRenderPass(pipelineIdx)
	local rpCreateInfo = prosper.RenderPassCreateInfo()
	rpCreateInfo:AddAttachment(
		prosper.FORMAT_R8G8B8A8_UNORM,
		prosper.IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		prosper.IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		prosper.ATTACHMENT_LOAD_OP_LOAD,
		prosper.ATTACHMENT_STORE_OP_STORE
	)
	return { prosper.create_render_pass(rpCreateInfo) }
end
function Shader:Draw(drawCmd, dsTex)
	if self:IsValid() == false then
		return
	end
	local bindState = shader.BindState(drawCmd)
	if self:RecordBeginDraw(bindState) == false then
		return
	end
	local buf, numVerts = prosper.util.get_square_vertex_uv_buffer()
	self:RecordBindVertexBuffers(bindState, { buf })
	self:RecordBindDescriptorSet(bindState, dsTex)

	self:RecordDraw(bindState, prosper.util.get_square_vertex_count())
	self:RecordEndDraw(bindState)
end
shader.register("example_2d", Shader)
