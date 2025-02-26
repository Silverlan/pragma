/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/components/renderers/c_renderer_pp_dof_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_dof.hpp"
#include "pragma/rendering/world_environment.hpp"
#include "pragma/entities/entity_component_system_t.hpp"
#include "pragma/console/c_cvar.h"
#include <pragma/entities/entity_component_manager_t.hpp>
#include <prosper_command_buffer.hpp>
#include <shader/prosper_shader_blur.hpp>
#include <image/prosper_msaa_texture.hpp>
#include <image/prosper_render_target.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

CRendererPpDoFComponent::CRendererPpDoFComponent(BaseEntity &ent) : CRendererPpBaseComponent(ent) {}
void CRendererPpDoFComponent::DoRenderEffect(const util::DrawSceneInfo &drawSceneInfo)
{
	if(drawSceneInfo.renderStats)
		(*drawSceneInfo.renderStats)->BeginGpuTimer(RenderStats::RenderStage::PostProcessingGpuDoF, *drawSceneInfo.commandBuffer);
	c_game->StartGPUProfilingStage("PostProcessingDoF");

	util::ScopeGuard scopeGuard {[&drawSceneInfo]() {
		c_game->StopGPUProfilingStage(); // PostProcessingDoF
		if(drawSceneInfo.renderStats)
			(*drawSceneInfo.renderStats)->EndGpuTimer(RenderStats::RenderStage::PostProcessingGpuDoF, *drawSceneInfo.commandBuffer);
	}};

	if(drawSceneInfo.scene.expired() || m_renderer.expired())
		return;
	auto &scene = *drawSceneInfo.scene;
	auto &cam = scene.GetActiveCamera();
	auto &hdrInfo = m_renderer->GetHDRInfo();
	auto &drawCmd = drawSceneInfo.commandBuffer;
	auto &hShaderDof = c_game->GetGameShader(CGame::GameShader::PPDoF);
	if(hShaderDof.expired() || cam.expired())
		return;
	auto opticalC = cam->GetEntity().GetComponent<pragma::COpticalCameraComponent>();
	if(opticalC.expired())
		return;
	auto &shaderDoF = static_cast<pragma::ShaderPPDoF &>(*hShaderDof.get());
	auto &prepass = hdrInfo.prepass;
	auto texDepth = prepass.textureDepth;
	if(texDepth->IsMSAATexture()) {
		texDepth = static_cast<prosper::MSAATexture &>(*texDepth).Resolve(*drawCmd, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
	}
	else
		drawCmd->RecordImageBarrier(texDepth->GetImage(), prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
	//texDepth->GetImage()->SetDrawLayout(prosper::ImageLayout::ShaderReadOnlyOptimal);

	auto &hdrTex = hdrInfo.sceneRenderTarget->GetTexture();
	drawCmd->RecordImageBarrier(hdrTex.GetImage(), prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
	drawCmd->RecordBufferBarrier(*scene.GetCameraBuffer(), prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit, prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::ShaderReadBit);
	drawCmd->RecordBufferBarrier(*scene.GetRenderSettingsBuffer(), prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit, prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::ShaderReadBit);
	drawCmd->RecordBufferBarrier(*scene.GetFogBuffer(), prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit, prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::ShaderReadBit);
	if(drawCmd->RecordBeginRenderPass(*hdrInfo.hdrPostProcessingRenderTarget) == true) {
		prosper::ShaderBindState bindState {*drawCmd};
		if(shaderDoF.RecordBeginDraw(bindState) == true) {
			pragma::ShaderPPDoF::PushConstants pushConstants {};
			pushConstants.mvp = cam->GetViewMatrix() * cam->GetProjectionMatrix();
			pushConstants.width = scene.GetWidth();
			pushConstants.height = scene.GetHeight();

			pushConstants.focalDepth = opticalC->GetFocalDistance();
			pushConstants.focalLength = opticalC->GetFocalLength();
			pushConstants.fstop = opticalC->GetFStop();

			pushConstants.zNear = cam->GetNearZ();
			pushConstants.zFar = cam->GetFarZ();

			pushConstants.flags = pragma::COpticalCameraComponent::Flags::None;
			umath::set_flag(pushConstants.flags, pragma::COpticalCameraComponent::Flags::EnableVignette, opticalC->IsVignetteEnabled());
			umath::set_flag(pushConstants.flags, pragma::COpticalCameraComponent::Flags::PentagonBokehShape, opticalC->GetPentagonBokehShape());
			umath::set_flag(pushConstants.flags, pragma::COpticalCameraComponent::Flags::DebugShowDepth, opticalC->GetDebugShowDepth());
			umath::set_flag(pushConstants.flags, pragma::COpticalCameraComponent::Flags::DebugShowFocus, opticalC->GetDebugShowFocus());
			pushConstants.rings = opticalC->GetRingCount();
			pushConstants.ringSamples = opticalC->GetRingSamples();
			pushConstants.CoC = opticalC->GetCircleOfConfusionSize();
			pushConstants.maxBlur = opticalC->GetMaxBlur();
			pushConstants.dither = opticalC->GetDitherAmount();
			pushConstants.vignIn = opticalC->GetVignettingInnerBorder();
			pushConstants.vignOut = opticalC->GetVignettingOuterBorder();
			pushConstants.pentagonShapeFeather = opticalC->GetPentagonShapeFeather();

			shaderDoF.RecordDraw(bindState, *hdrInfo.dsgHDRPostProcessing->GetDescriptorSet(), *hdrInfo.dsgDepthPostProcessing->GetDescriptorSet(), pushConstants);
			shaderDoF.RecordEndDraw(bindState);
		}
		drawCmd->RecordEndRenderPass();
	}
	drawCmd->RecordImageBarrier(texDepth->GetImage(), prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::DepthStencilAttachmentOptimal);

	hdrInfo.BlitStagingRenderTargetToMainRenderTarget(drawSceneInfo);
}
void CRendererPpDoFComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
