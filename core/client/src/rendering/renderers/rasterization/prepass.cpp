/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/renderers/rasterization/culled_mesh_data.hpp"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree_impl.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_transform_component.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/console/c_cvar.h"
#include <cmaterialmanager.h>
#include <pragma/console/convars.h>
#include <prosper_util.hpp>
#include <prosper_command_buffer.hpp>
#include <buffers/prosper_uniform_resizable_buffer.hpp>
#include <image/prosper_msaa_texture.hpp>
#include <sharedutils/util_shaderinfo.hpp>

using namespace pragma::rendering;

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;
#pragma optimize("",off)
static auto cvDrawView = GetClientConVar("render_draw_view");
void RasterizationRenderer::RenderPrepass(const util::DrawSceneInfo &drawSceneInfo)
{
	auto &scene = *drawSceneInfo.scene;
	auto &drawCmd = drawSceneInfo.commandBuffer;
	auto &hCam = scene.GetActiveCamera();
	auto bReflection = ((drawSceneInfo.renderFlags &FRender::Reflection) != FRender::None) ? true : false;
	auto pipelineType = (bReflection == true) ? pragma::ShaderPrepassBase::Pipeline::Reflection :
		(GetSampleCount() == prosper::SampleCountFlags::e1Bit) ? pragma::ShaderPrepassBase::Pipeline::Regular :
		pragma::ShaderPrepassBase::Pipeline::MultiSample;
	//RenderSystem::RenderPrepass(drawSceneInfo,RenderMode::Skybox);
	RenderSystem::RenderPrepass(drawSceneInfo,RenderMode::World);
}
#pragma optimize("",on)
