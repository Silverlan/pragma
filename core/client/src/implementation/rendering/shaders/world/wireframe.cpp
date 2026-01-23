// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.world_wireframe;

using namespace pragma;

ShaderWireframe::ShaderWireframe(prosper::IPrContext &context, const std::string &identifier) : ShaderPBR(context, identifier, "programs/scene/textured", "programs/scene/wireframe")
{
	// SetBaseShader<ShaderTextured3DBase>();
}

void ShaderWireframe::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	ShaderPBR::InitializeGfxPipeline(pipelineInfo, pipelineIdx);
	prosper::util::set_graphics_pipeline_polygon_mode(pipelineInfo, prosper::PolygonMode::Line);
	prosper::util::set_graphics_pipeline_line_width(pipelineInfo, 2.f);
	pipelineInfo.ToggleDepthWrites(true);
}
