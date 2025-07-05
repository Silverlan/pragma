-- SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

local Shader = util.register_class("shader.ExampleWireframe", shader.BaseTexturedLit3D)

Shader.FragmentShader = "examples/fs_example_simple_game_shader"
Shader.VertexShader = "world/vs_textured"
function Shader:Initialize()
	self:SetDepthPrepassEnabled(false)
end
function Shader:InitializePipeline(pipelineInfo, pipelineIdx)
	shader.BaseTexturedLit3D.InitializePipeline(self, pipelineInfo, pipelineIdx)

	pipelineInfo:SetPolygonMode(prosper.POLYGON_MODE_LINE)
	pipelineInfo:SetDepthWritesEnabled(true)
end
shader.register("example_wireframe", Shader)
