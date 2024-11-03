--[[
    Copyright (C) 2024 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

local Shader = util.register_class("shader.BwShader", shader.BasePbr)

Shader.FragmentShader = "programs/scene/bw"
Shader.VertexShader = "programs/scene/textured"
function Shader:InitializePipeline(pipelineInfo, pipelineIdx)
	shader.BasePbr.InitializePipeline(self, pipelineInfo, pipelineIdx)
end
shader.register("bw", Shader)
