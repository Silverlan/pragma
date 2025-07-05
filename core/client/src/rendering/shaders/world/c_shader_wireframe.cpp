// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_wireframe.hpp"
#include <shader/prosper_pipeline_create_info.hpp>

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
