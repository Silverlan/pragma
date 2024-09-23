--[[
    Copyright (C) 2024 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

local Shader = util.register_class("shader.ToonShader", shader.BaseTexturedLit3D)

Shader.FragmentShader = "programs/scene/toon"
Shader.VertexShader = "programs/scene/textured"
Shader.ShaderMaterial = "toon"
function Shader:InitializePipeline(pipelineInfo, pipelineIdx)
	shader.BaseTexturedLit3D.InitializePipeline(self, pipelineInfo, pipelineIdx)
end
shader.register("toon", Shader)
