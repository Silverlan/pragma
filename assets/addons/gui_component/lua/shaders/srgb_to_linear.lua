-- SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

util.register_class("shader.SRGBToLinear", shader.BaseImageProcessing)

shader.SRGBToLinear.FragmentShader = "screen/fs_srgb_to_linear"
shader.SRGBToLinear.VertexShader = "screen/vs_screen_uv"

------------------

function shader.SRGBToLinear:__init()
	shader.BaseImageProcessing.__init(self)
end
function shader.SRGBToLinear:InitializePipeline(pipelineInfo, pipelineIdx)
	shader.BaseImageProcessing.InitializePipeline(self, pipelineInfo, pipelineIdx)
end
function shader.SRGBToLinear:Draw(drawCmd, ds)
	local baseShader = self:GetShader()
	if baseShader:IsValid() == false then
		return
	end
	local bindState = shader.BindState(drawCmd)
	if baseShader:RecordBeginDraw(bindState) == false then
		return
	end
	self:RecordDraw(bindState, ds)
	baseShader:RecordEndDraw(bindState)
end
shader.register("srgb_to_linear", shader.SRGBToLinear)
