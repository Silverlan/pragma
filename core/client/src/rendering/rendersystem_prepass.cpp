// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/rendering/render_queue.hpp"
#include "pragma/rendering/render_stats.hpp"
#include "pragma/game/c_game.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/rendering/render_processor.hpp"
#include <sharedutils/alpha_mode.hpp>
#include <pragma/model/animation/vertex_animation.hpp>
#include <prosper_command_buffer.hpp>
#include <image/prosper_render_target.hpp>
#include <pragma/model/c_vertex_buffer_data.hpp>

import pragma.client.entities.components;
import pragma.client.model;

extern CEngine *c_engine;
extern ClientState *client;
extern CGame *c_game;

pragma::rendering::DepthStageRenderProcessor::DepthStageRenderProcessor(const util::RenderPassDrawInfo &drawSceneInfo, const Vector4 &drawOrigin) : BaseRenderProcessor {drawSceneInfo, drawOrigin} { SetCountNonOpaqueMaterialsOnly(true); }
uint32_t pragma::rendering::DepthStageRenderProcessor::Render(const pragma::rendering::RenderQueue &renderQueue, RenderPass renderPass, RenderPassStats *optStats, std::optional<uint32_t> worldRenderQueueIndex)
{
	return BaseRenderProcessor::Render(renderQueue, renderPass, optStats, worldRenderQueueIndex);
}

void pragma::rendering::DepthStageRenderProcessor::BindLight(CLightComponent &light, uint32_t layerId) { m_shaderProcessor.RecordBindLight(light, layerId); }
