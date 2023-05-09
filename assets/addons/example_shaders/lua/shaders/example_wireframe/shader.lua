--[[
    Copyright (C) 2021 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

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
