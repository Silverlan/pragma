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
	auto &shaderDepthStage = GetPrepass().GetShader();
	if(shaderDepthStage.BeginDraw(drawCmd,pipelineType) == true)
	{
		shaderDepthStage.BindClipPlane(c_game->GetRenderClipPlane());
		shaderDepthStage.BindSceneCamera(const_cast<pragma::CSceneComponent&>(scene),*this,false);
		if((drawSceneInfo.renderFlags &FRender::Skybox) != FRender::None)
		{
			c_game->StartProfilingStage(CGame::GPUProfilingPhase::PrepassSkybox);
			if(hCam.valid())
			{
				RenderSystem::RenderPrepass(drawSceneInfo,RenderMode::Skybox);

				// 3D Skybox
				if(m_3dSkyCameras.empty() == false)
				{
					shaderDepthStage.Set3DSky(true);
					for(auto &hSkyCam : m_3dSkyCameras)
					{
						auto filteredMeshes = hSkyCam.valid() ? hSkyCam->GetRenderMeshCollectionHandler().GetRenderMeshData(RenderMode::World) : nullptr;
						if(filteredMeshes == nullptr)
							continue;
						auto &ent = hSkyCam->GetEntity();
						auto &pos = ent.GetPosition();
						Vector4 drawOrigin {pos.x,pos.y,pos.z,hSkyCam->GetSkyboxScale()};
						shaderDepthStage.BindDrawOrigin(drawOrigin);
						RenderSystem::RenderPrepass(drawSceneInfo,*filteredMeshes);
					}
					shaderDepthStage.Set3DSky(false);
				}
			}
			c_game->StopProfilingStage(CGame::GPUProfilingPhase::PrepassSkybox);
		}
		shaderDepthStage.BindDrawOrigin(Vector4{0.f,0.f,0.f,1.f});
		if((drawSceneInfo.renderFlags &FRender::World) != FRender::None)
		{
			c_game->StartProfilingStage(CGame::GPUProfilingPhase::PrepassWorld);
			if(hCam.valid())
				RenderSystem::RenderPrepass(drawSceneInfo,RenderMode::World);
			c_game->StopProfilingStage(CGame::GPUProfilingPhase::PrepassWorld);
		}
		c_game->CallCallbacks<void>("RenderPrepass");
		c_game->CallLuaCallbacks<void,std::reference_wrapper<const util::DrawSceneInfo>>("RenderPrepass",drawSceneInfo);

		shaderDepthStage.BindSceneCamera(const_cast<pragma::CSceneComponent&>(scene),*this,true);
		auto *pl = c_game->GetLocalPlayer();
		if((drawSceneInfo.renderFlags &FRender::View) != FRender::None && pl != nullptr && pl->IsInFirstPersonMode() == true && cvDrawView->GetBool() == true)
		{
			c_game->StartProfilingStage(CGame::GPUProfilingPhase::PrepassView);
			if(hCam.valid())
				RenderSystem::RenderPrepass(drawSceneInfo,RenderMode::View);
			c_game->StopProfilingStage(CGame::GPUProfilingPhase::PrepassView);
		}
		shaderDepthStage.EndDraw();
	}
}
