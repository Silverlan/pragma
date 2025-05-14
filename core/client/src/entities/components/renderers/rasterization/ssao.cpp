/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "pragma/rendering/shaders/post_processing/c_shader_ssao.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_ssao_blur.hpp"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree_impl.hpp"
#include "pragma/rendering/c_ssao.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/game/c_game.h"
#include <prosper_command_buffer.hpp>
#include <prosper_util.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <image/prosper_render_target.hpp>
#include <image/prosper_msaa_texture.hpp>

using namespace pragma::rendering;

extern DLLCLIENT CGame *c_game;

void pragma::CRasterizationRendererComponent::RenderSSAO(const util::DrawSceneInfo &drawSceneInfo)
{
	auto &ssaoInfo = GetSSAOInfo();
	auto *shaderSSAO = static_cast<pragma::ShaderSSAO *>(ssaoInfo.GetSSAOShader());
	auto *shaderSSAOBlur = static_cast<pragma::ShaderSSAOBlur *>(ssaoInfo.GetSSAOBlurShader());
	if(IsSSAOEnabled() == false || shaderSSAO == nullptr || shaderSSAOBlur == nullptr || drawSceneInfo.scene.expired())
		return;
	auto &scene = *drawSceneInfo.scene;
	c_game->StartProfilingStage("SSAO");
	c_game->StartGPUProfilingStage("SSAO");
	// Pre-render depths, positions and normals (Required for SSAO)
	//auto *renderInfo  = scene.GetSceneRenderDesc().GetRenderInfo(RenderMode::World);
	auto &drawCmd = drawSceneInfo.commandBuffer;
	//if(renderInfo != nullptr)
	{
		// SSAO
		auto &prepass = GetPrepass();
		auto &ssaoImg = ssaoInfo.renderTarget->GetTexture().GetImage();

		auto texNormals = prepass.textureNormals;
		auto bNormalsMultiSampled = texNormals->IsMSAATexture();
		if(bNormalsMultiSampled) {
			texNormals = static_cast<prosper::MSAATexture &>(*texNormals).Resolve(*drawCmd, prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
		}
		else
			drawCmd->RecordImageBarrier(texNormals->GetImage(), prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
		auto texDepth = prepass.textureDepth;
		if(texDepth->IsMSAATexture()) {
			texDepth = static_cast<prosper::MSAATexture &>(*texDepth).Resolve(*drawCmd, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
		}
		else
			drawCmd->RecordImageBarrier(texDepth->GetImage(), prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);

		drawCmd->RecordBeginRenderPass(*ssaoInfo.renderTarget);
		auto &renderImage = ssaoInfo.renderTarget->GetTexture().GetImage();
		auto extents = renderImage.GetExtents();

		prosper::ShaderBindState bindState {*drawCmd};
		if(shaderSSAO->RecordBeginDraw(bindState) == true) {
			shaderSSAO->RecordDraw(bindState, scene, *ssaoInfo.descSetGroupPrepass->GetDescriptorSet(), {extents.width, extents.height});
			shaderSSAO->RecordEndDraw(bindState);
		}

		drawCmd->RecordEndRenderPass();

		drawCmd->RecordImageBarrier(texNormals->GetImage(), prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::ColorAttachmentOptimal);
		drawCmd->RecordImageBarrier(texDepth->GetImage(), prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::DepthStencilAttachmentOptimal);

		// Blur SSAO
		drawCmd->RecordImageBarrier(ssaoInfo.renderTargetBlur->GetTexture().GetImage(), prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::ColorAttachmentOptimal);
		drawCmd->RecordBeginRenderPass(*ssaoInfo.renderTargetBlur);

		if(shaderSSAOBlur->RecordBeginDraw(bindState) == true) {
			shaderSSAOBlur->RecordDraw(bindState, *ssaoInfo.descSetGroupOcclusion->GetDescriptorSet());
			shaderSSAOBlur->RecordEndDraw(bindState);
		}

		drawCmd->RecordEndRenderPass();
		//

		if(bNormalsMultiSampled) {
			drawCmd->RecordImageBarrier(texNormals->GetImage(), prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
			drawCmd->RecordImageBarrier(texDepth->GetImage(), prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
		}

		drawCmd->RecordImageBarrier(ssaoInfo.renderTarget->GetTexture().GetImage(), prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::ColorAttachmentOptimal);
	}
	c_game->StopGPUProfilingStage(); // SSAO
	c_game->StopProfilingStage(); // SSAO
}
