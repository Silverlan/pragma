/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/renderers/rasterization/culled_mesh_data.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_glow.hpp"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree_impl.hpp"
#include "pragma/game/c_game.h"
#include <pragma/c_engine.h>
#include <prosper_util.hpp>
#include <prosper_command_buffer.hpp>
#include <shader/prosper_shader_blur.hpp>
#include <image/prosper_render_target.hpp>
#include <cmaterial.h>

using namespace pragma::rendering;

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;


void RasterizationRenderer::RenderBloom(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd)
{
	c_game->StartProfilingStage(CGame::GPUProfilingPhase::PostProcessingBloom);
	auto &hdrInfo = GetHDRInfo();
	auto bloomTexMsaa = hdrInfo.sceneRenderTarget->GetTexture(1u);
	// Blit high-res bloom image into low-res image, which is cheaper to blur
	drawCmd->RecordImageBarrier(hdrInfo.bloomTexture->GetImage(),prosper::ImageLayout::ColorAttachmentOptimal,prosper::ImageLayout::TransferSrcOptimal);
	drawCmd->RecordImageBarrier(hdrInfo.bloomBlurRenderTarget->GetTexture().GetImage(),prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::TransferDstOptimal);
	drawCmd->RecordBlitTexture(*hdrInfo.bloomTexture,hdrInfo.bloomBlurRenderTarget->GetTexture().GetImage());

	static auto blurSize = 5.f;
	static int32_t kernelSize = 9u;
	static uint32_t blurAmount = 5u;

	drawCmd->RecordImageBarrier(hdrInfo.bloomBlurRenderTarget->GetTexture().GetImage(),prosper::ImageLayout::TransferDstOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal);
	for(auto i=decltype(blurAmount){0};i<blurAmount;++i)
	{
		prosper::util::record_blur_image(*c_engine,drawCmd,*hdrInfo.bloomBlurSet,{
			Vector4(1.f,1.f,1.f,1.f),
			blurSize,
			kernelSize
		});
	}
	drawCmd->RecordImageBarrier(hdrInfo.bloomTexture->GetImage(),prosper::ImageLayout::TransferSrcOptimal,prosper::ImageLayout::ColorAttachmentOptimal);
	c_game->StopProfilingStage(CGame::GPUProfilingPhase::PostProcessingBloom);
}

void RasterizationRenderer::RenderGlowObjects(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd)
{
	auto &glowInfo = GetGlowInfo();
	if(glowInfo.bGlowScheduled == false)
		return;
	c_game->StartProfilingStage(CGame::GPUProfilingPhase::PostProcessingGlow);
	drawCmd->RecordBeginRenderPass(*glowInfo.renderTarget,{
		vk::ClearValue{vk::ClearColorValue{std::array<float,4>{0.f,0.f,0.f,1.f}}},
		vk::ClearValue{vk::ClearDepthStencilValue{}}
	});
	if(!glowInfo.tmpBloomParticles.empty())
	{
		RenderParticleSystems(drawCmd,glowInfo.tmpBloomParticles,RenderMode::World,true);
		RenderParticleSystems(drawCmd,glowInfo.tmpBloomParticles,RenderMode::View,true);
		glowInfo.tmpBloomParticles.clear();
	}
	for(auto i=std::underlying_type_t<RenderMode>{0};i<umath::to_integral(RenderMode::Count);++i)
	{
		auto *renderInfo = GetRenderInfo(static_cast<RenderMode>(i));
		if(renderInfo == nullptr || renderInfo->glowMeshes.empty() == true)
			continue;
		RenderGlowMeshes(drawCmd,static_cast<RenderMode>(i));
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
		prosper::util::record_blur_image(*c_engine,drawCmd,*glowInfo.blurSet,{
			Vector4(1.f,1.f,1.f,1.f), /* color scale */
			1.75f, /* blur size */
			3 /* kernel size */
		});
	}

	drawCmd->RecordImageBarrier(glowInfo.renderTarget->GetTexture().GetImage(),prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::ColorAttachmentOptimal);
	c_game->StopProfilingStage(CGame::GPUProfilingPhase::PostProcessingGlow);
}

void RasterizationRenderer::RenderGlowMeshes(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,RenderMode renderMode)
{
	auto &glowInfo = GetGlowInfo();
	auto *shader = static_cast<pragma::ShaderGlow*>(glowInfo.shader.get());
	if(shader == nullptr)
		return;
	auto *renderInfo = GetRenderInfo(renderMode);
	if(renderInfo == nullptr)
		return;
	if(shader->BeginDraw(drawCmd) == true)
	{
		shader->BindSceneCamera(*this,(renderMode == RenderMode::View) ? true : false);
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
}

