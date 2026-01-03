// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.rasterization_renderer;
import :client_state;
import :engine;
import :game;

using namespace pragma::rendering;

void pragma::CRasterizationRendererComponent::RenderGlowObjects(const DrawSceneInfo &drawSceneInfo)
{
#if 0
	auto &glowInfo = GetGlowInfo();
	if(glowInfo.bGlowScheduled == false || drawSceneInfo.scene.expired())
		return;
	auto &scene = *drawSceneInfo.scene;
	pragma::get_cgame()->StartProfilingStage(pragma::CGame::GPUProfilingPhase::PostProcessingGlow);
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
	for(auto i=std::underlying_type_t<RenderMode>{0};i<pragma::math::to_integral(RenderMode::Count);++i)
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
		prosper::util::record_blur_image(pragma::get_cengine()->GetRenderContext(),drawCmd,*glowInfo.blurSet,{
			Vector4(1.f,1.f,1.f,1.f), /* color scale */
			1.75f, /* blur size */
			3 /* kernel size */
		});
	}

	drawCmd->RecordImageBarrier(glowInfo.renderTarget->GetTexture().GetImage(),prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::ColorAttachmentOptimal);
	pragma::get_cgame()->StopProfilingStage(pragma::CGame::GPUProfilingPhase::PostProcessingGlow);
#endif
}

void pragma::CRasterizationRendererComponent::RenderGlowMeshes(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, const CSceneComponent &scene, SceneRenderPass renderMode)
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
			if(shader->BindGlowMaterial(static_cast<material::CMaterial&>(*mat)) == true)
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
static void cmd_render_bloom_enabled(pragma::NetworkState *, const pragma::console::ConVar &, bool, bool enabled)
{
	auto *client = pragma::get_client_state();
	if(client == nullptr)
		return;
	client->UpdateGameWorldShaderSettings();
}
namespace {
	auto _ = pragma::console::client::register_variable_listener<bool>("render_bloom_enabled", &cmd_render_bloom_enabled);
}
