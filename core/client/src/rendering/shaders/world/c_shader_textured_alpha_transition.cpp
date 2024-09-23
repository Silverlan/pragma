/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_textured_alpha_transition.hpp"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/vk_mesh.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include <buffers/prosper_buffer.hpp>
#include <prosper_util.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <cmaterial.h>

using namespace pragma;

extern DLLCLIENT CEngine *c_engine;

decltype(ShaderTexturedAlphaTransition::VERTEX_BINDING_ALPHA) ShaderTexturedAlphaTransition::VERTEX_BINDING_ALPHA = {prosper::VertexInputRate::Vertex};
decltype(ShaderTexturedAlphaTransition::VERTEX_ATTRIBUTE_ALPHA) ShaderTexturedAlphaTransition::VERTEX_ATTRIBUTE_ALPHA = {VERTEX_BINDING_ALPHA, prosper::Format::R32G32_SFloat};
ShaderTexturedAlphaTransition::ShaderTexturedAlphaTransition(prosper::IPrContext &context, const std::string &identifier) : ShaderGameWorldLightingPass(context, identifier, "world/vs_textured_alpha_transition", "world/fs_textured_alpha_transition")
{
	// SetBaseShader<ShaderTextured3DBase>();
}

void ShaderTexturedAlphaTransition::InitializeGfxPipelinePushConstantRanges() { AttachPushConstantRange(0u, sizeof(ShaderGameWorldLightingPass::PushConstants) + sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit); }
void ShaderTexturedAlphaTransition::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { ShaderGameWorldLightingPass::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }
void ShaderTexturedAlphaTransition::InitializeShaderResources()
{
	ShaderGameWorldLightingPass::InitializeShaderResources();
	AddVertexAttribute(VERTEX_ATTRIBUTE_ALPHA);
}
