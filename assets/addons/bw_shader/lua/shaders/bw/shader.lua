-- SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

local Shader = util.register_class("shader.BwShader", shader.BasePbr)

Shader.FragmentShader = "programs/scene/bw"
Shader.VertexShader = "programs/scene/textured"
function Shader:InitializePipeline(pipelineInfo, pipelineIdx)
	shader.BasePbr.InitializePipeline(self, pipelineInfo, pipelineIdx)
end
shader.register("bw", Shader)
