/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/rendersystem.h"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/entities/components/renderers/rasterization/culled_mesh_data.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/rendering/render_queue.hpp"
#include "pragma/rendering/render_stats.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/entities/components/c_vertex_animated_component.hpp"
#include <sharedutils/alpha_mode.hpp>
#include <pragma/model/animation/vertex_animation.hpp>
#include <prosper_command_buffer.hpp>
#include <image/prosper_render_target.hpp>
#include <pragma/model/c_vertex_buffer_data.hpp>
#include <pragma/model/vk_mesh.h>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

#include "pragma/rendering/render_processor.hpp"

pragma::rendering::DepthStageRenderProcessor::DepthStageRenderProcessor(const util::RenderPassDrawInfo &drawSceneInfo,RenderFlags flags,const Vector4 &drawOrigin)
	: BaseRenderProcessor{drawSceneInfo,flags,drawOrigin}
{
	SetCountNonOpaqueMaterialsOnly(true);
}
uint32_t pragma::rendering::DepthStageRenderProcessor::Render(const pragma::rendering::RenderQueue &renderQueue,RenderPassStats *optStats,std::optional<uint32_t> worldRenderQueueIndex)
{
	return BaseRenderProcessor::Render(renderQueue,true,optStats,worldRenderQueueIndex);
}

void RenderSystem::RenderPrepass(const util::DrawSceneInfo &drawSceneInfo,RenderMode renderMode)
{
	// TODO: Remove me
}

void pragma::rendering::DepthStageRenderProcessor::BindLight(CLightComponent &light,uint32_t layerId)
{
	m_shaderProcessor.RecordBindLight(light,layerId);
}
