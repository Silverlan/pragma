/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/entities/components/renderers/rasterization/hdr_data.hpp"
#include "pragma/rendering/world_environment.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fog.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/game/c_game.h"
#include <prosper_util.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <prosper_command_buffer.hpp>
#include <image/prosper_render_target.hpp>
#include <image/prosper_msaa_texture.hpp>

using namespace pragma::rendering;

extern DLLCLIENT CGame *c_game;

void pragma::CRasterizationRendererComponent::RenderSceneFog(const util::DrawSceneInfo &drawSceneInfo)
{
	if(drawSceneInfo.scene.expired())
		return;
	auto &scene = *drawSceneInfo.scene;
	auto &hdrInfo = GetHDRInfo();
	auto descSetGroupFog = m_descSetGroupFogOverride;
	if(descSetGroupFog == nullptr)
	{
		auto *worldEnv = scene.GetWorldEnvironment();
		if(worldEnv != nullptr)
		{
			auto &fog = worldEnv->GetFogSettings();
			if(fog.IsEnabled() == true)
				descSetGroupFog = scene.GetFogDescriptorSetGroup();
		}
	}
	auto &drawCmd = drawSceneInfo.commandBuffer;
	auto hShaderFog = c_game->GetGameShader(CGame::GameShader::PPFog);
	if(descSetGroupFog == nullptr || hShaderFog.expired())
		return;
	auto &shaderFog = static_cast<pragma::ShaderPPFog&>(*hShaderFog.get());
	auto &prepass = hdrInfo.prepass;
	auto texDepth = prepass.textureDepth;
	if(texDepth->IsMSAATexture())
	{
		texDepth = static_cast<prosper::MSAATexture&>(*texDepth).Resolve(
			*drawCmd,prosper::ImageLayout::DepthStencilAttachmentOptimal,prosper::ImageLayout::DepthStencilAttachmentOptimal,
			prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal
		);
	}
	else
		drawCmd->RecordImageBarrier(texDepth->GetImage(),prosper::ImageLayout::DepthStencilAttachmentOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal);
	//texDepth->GetImage()->SetDrawLayout(prosper::ImageLayout::ShaderReadOnlyOptimal);

	auto &hdrTex = hdrInfo.sceneRenderTarget->GetTexture();
	drawCmd->RecordImageBarrier(hdrTex.GetImage(),prosper::ImageLayout::ColorAttachmentOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal);
	drawCmd->RecordBufferBarrier(
		*scene.GetCameraBuffer(),
		prosper::PipelineStageFlags::TransferBit,prosper::PipelineStageFlags::FragmentShaderBit,
		prosper::AccessFlags::TransferWriteBit,prosper::AccessFlags::ShaderReadBit
	);
	drawCmd->RecordBufferBarrier(
		*scene.GetRenderSettingsBuffer(),
		prosper::PipelineStageFlags::TransferBit,prosper::PipelineStageFlags::FragmentShaderBit,
		prosper::AccessFlags::TransferWriteBit,prosper::AccessFlags::ShaderReadBit
	);
	drawCmd->RecordBufferBarrier(
		*scene.GetFogBuffer(),
		prosper::PipelineStageFlags::TransferBit,prosper::PipelineStageFlags::FragmentShaderBit,
		prosper::AccessFlags::TransferWriteBit,prosper::AccessFlags::ShaderReadBit
	);
	if(drawCmd->RecordBeginRenderPass(*hdrInfo.hdrPostProcessingRenderTarget) == true)
	{
		if(shaderFog.BeginDraw(drawCmd) == true)
		{
			shaderFog.Draw(
				*hdrInfo.dsgHDRPostProcessing->GetDescriptorSet(),
				*hdrInfo.dsgDepthPostProcessing->GetDescriptorSet(),
				*scene.GetCameraDescriptorSetGraphics(),
				*scene.GetFogDescriptorSetGroup()->GetDescriptorSet()
			);
			shaderFog.EndDraw();
		}
		drawCmd->RecordEndRenderPass();
	}
	drawCmd->RecordImageBarrier(texDepth->GetImage(),prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::DepthStencilAttachmentOptimal);

	hdrInfo.BlitStagingRenderTargetToMainRenderTarget(drawSceneInfo);
}
