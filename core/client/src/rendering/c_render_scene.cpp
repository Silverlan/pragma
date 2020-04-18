#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fog.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_hdr.hpp"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_ssao.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_ssao_blur.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fxaa.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/world_environment.hpp"
#include "pragma/rendering/scene/scene.h"
#include "pragma/console/c_cvar.h"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/rendering/c_settings.hpp"
#include <shader/prosper_shader_blur.hpp>
#include <prosper_util.hpp>
#include <image/prosper_render_target.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <image/prosper_msaa_texture.hpp>
#include <pragma/rendering/c_sci_gpu_timer_manager.hpp>
#include <sharedutils/scope_guard.h>

extern DLLCENGINE CEngine *c_engine;


void CGame::RenderScenePresent(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,prosper::Texture &texPostHdr,prosper::Image &outImage,uint32_t layerId)
{
	prosper::util::record_image_barrier(*(*drawCmd),*outImage,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL);
	prosper::util::BlitInfo blitInfo {};
	blitInfo.dstSubresourceLayer.base_array_layer = layerId;
	prosper::util::record_blit_image(**drawCmd,blitInfo,**texPostHdr.GetImage(),*outImage);
	prosper::util::record_image_barrier(*(*drawCmd),*(*texPostHdr.GetImage()),Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
	prosper::util::record_image_barrier(*(*drawCmd),*outImage,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
}

std::shared_ptr<prosper::PrimaryCommandBuffer> CGame::GetCurrentDrawCommandBuffer() const {return m_currentDrawCmd.lock();}

//Shader::Base *CGame::GetShaderOverride() {return m_shaderOverride;} // prosper TODO
//void CGame::SetShaderOverride(Shader::Base *shader) {m_shaderOverride = shader;} // prosper TODO

void CGame::RenderScene(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,prosper::Image &outImage,FRender renderFlags,uint32_t outLayerId)
{
	m_currentDrawCmd = drawCmd;
	ScopeGuard sgCurrentDrawCmd {[this]() {
		m_currentDrawCmd = {};
	}};

	auto &scene = GetRenderScene();
	auto *renderer = scene->GetRenderer();
	if(renderer)
	{
		renderer->RenderScene(drawCmd,renderFlags);
		StartProfilingStage(CGame::GPUProfilingPhase::Present);
		StartProfilingStage(CGame::CPUProfilingPhase::Present);

		prosper::Texture *presentationTexture = nullptr;
		if(umath::is_flag_set(renderFlags,FRender::HDR))
			presentationTexture = renderer->GetHDRPresentationTexture().get();
		else
			presentationTexture = renderer->GetPresentationTexture().get();

		RenderScenePresent(drawCmd,*presentationTexture,outImage,outLayerId);
		StopProfilingStage(CGame::CPUProfilingPhase::Present);
		StopProfilingStage(CGame::GPUProfilingPhase::Present);
	}
}

