/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/entities/components/renderers/rasterization/culled_mesh_data.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree_impl.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/console/c_cvar.h"
#include "pragma/game/c_game.h"
#include <pragma/console/convars.h>
#include <pragma/c_engine.h>
#include <prosper_util.hpp>
#include <prosper_command_buffer.hpp>
#include <shader/prosper_shader_blur.hpp>
#include <image/prosper_render_target.hpp>
#include <cmaterial.h>

using namespace pragma::rendering;

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;


void pragma::CRasterizationRendererComponent::RenderGlowObjects(const util::DrawSceneInfo &drawSceneInfo)
{
#if 0
	auto &glowInfo = GetGlowInfo();
	if(glowInfo.bGlowScheduled == false || drawSceneInfo.scene.expired())
		return;
	auto &scene = *drawSceneInfo.scene;
	c_game->StartProfilingStage(CGame::GPUProfilingPhase::PostProcessingGlow);
	auto &drawCmd = drawSceneInfo.commandBuffer;
	drawCmd->RecordBeginRenderPass(*glowInfo.renderTarget,{
		prosper::ClearValue{prosper::ClearColorValue{std::array<float,4>{0.f,0.f,0.f,1.f}}},
		prosper::ClearValue{prosper::ClearDepthStencilValue{}}
	});
	if(!glowInfo.tmpBloomParticles.empty())
	{
		RenderParticleSystems(drawSceneInfo,glowInfo.tmpBloomParticles,RenderMode::World,true);
		RenderParticleSystems(drawSceneInfo,glowInfo.tmpBloomParticles,RenderMode::View,true);
		glowInfo.tmpBloomParticles.clear();
	}
	for(auto i=std::underlying_type_t<RenderMode>{0};i<umath::to_integral(RenderMode::Count);++i)
	{
		auto *renderInfo = scene.GetSceneRenderDesc().GetRenderInfo(static_cast<RenderMode>(i));
		if(renderInfo == nullptr || renderInfo->glowMeshes.empty() == true)
			continue;
		RenderGlowMeshes(drawCmd,scene,static_cast<RenderMode>(i));
	}

	drawCmd->RecordEndRenderPass();

	// TODO: Implement according to http://www.iryoku.com/next-generation-post-processing-in-call-of-duty-advanced-warfare
	/*.RecordGenerateMipmaps(
		**drawCmd,**img,prosper::ImageLayout::ShaderReadOnlyOptimal,
		prosper::AccessFlags::ShaderReadBit,prosper::PipelineStageFlags::FragmentShaderBit
	);*/

	const auto blurAmount = 5;
	auto &blurImg = glowInfo.blurSet->GetFinalRenderTarget()->GetTexture().GetImage();
	for(auto i=decltype(blurAmount){0u};i<blurAmount;++i)
	{
		prosper::util::record_blur_image(c_engine->GetRenderContext(),drawCmd,*glowInfo.blurSet,{
			Vector4(1.f,1.f,1.f,1.f), /* color scale */
			1.75f, /* blur size */
			3 /* kernel size */
		});
	}

	drawCmd->RecordImageBarrier(glowInfo.renderTarget->GetTexture().GetImage(),prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::ColorAttachmentOptimal);
	c_game->StopProfilingStage(CGame::GPUProfilingPhase::PostProcessingGlow);
#endif
}

void pragma::CRasterizationRendererComponent::RenderGlowMeshes(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, const pragma::CSceneComponent &scene, pragma::rendering::SceneRenderPass renderMode)
{
	// TODO
#if 0
	auto &glowInfo = GetGlowInfo();
	auto *shader = static_cast<pragma::ShaderGlow*>(glowInfo.shader.get());
	if(shader == nullptr)
		return;
	auto *renderInfo = scene.GetSceneRenderDesc().GetRenderInfo(renderMode);
	if(renderInfo == nullptr)
		return;
	if(shader->BeginDraw(drawCmd) == true)
	{
		shader->BindSceneCamera(const_cast<pragma::CSceneComponent&>(scene),*this,(renderMode == RenderMode::View) ? true : false);
		for(auto &matContainer : renderInfo->glowMeshes)
		{
			auto *mat = matContainer->material;
			if(shader->BindGlowMaterial(static_cast<CMaterial&>(*mat)) == true)
			{
				for(auto &pair : matContainer->containers)
				{
					shader->BindEntity(*pair.first);
					for(auto *mesh : pair.second.meshes)
						shader->Draw(*mesh);
				}
			}
		}
		shader->EndDraw();
	}
#endif
}
//
//
static void cmd_render_bloom_enabled(NetworkState *, const ConVar &, bool, bool enabled)
{
	if(client == nullptr)
		return;
	client->UpdateGameWorldShaderSettings();
}

REGISTER_CONVAR_CALLBACK_CL(render_bloom_enabled, cmd_render_bloom_enabled);
