/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/rendersystem.h"
#include "pragma/rendering/render_queue.hpp"
#include "cmaterialmanager.h"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/entity_instance_index_buffer.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/rendering/shaders/world/c_shader_wireframe.hpp"
#include "pragma/rendering/sortedrendermeshcontainer.h"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/entities/components/renderers/rasterization/culled_mesh_data.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/rendering/render_stats.hpp"
#include "pragma/debug/renderdebuginfo.hpp"
#include "textureinfo.h"
#include "pragma/console/c_cvar.h"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree_impl.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_vertex_animated_component.hpp"
#include <cmaterial.h>
#include <pragma/rendering/c_sci_gpu_timer_manager.hpp>
#include <pragma/model/animation/vertex_animation.hpp>
#include <prosper_util.hpp>
#include <image/prosper_render_target.hpp>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

// Disables rendering of meshes and shadows; For debug purposes only!
#define DEBUG_RENDER_DISABLED 0

#include "pragma/rendering/render_processor.hpp"

uint32_t pragma::rendering::LightingStageRenderProcessor::Render(const pragma::rendering::RenderQueue &renderQueue, RenderPassStats *optStats, std::optional<uint32_t> worldRenderQueueIndex) { return BaseRenderProcessor::Render(renderQueue, RenderPass::Lighting, optStats, worldRenderQueueIndex); }
