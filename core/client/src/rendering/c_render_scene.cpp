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
#include <pragma/util/profiling_stages.h>
#include <pragma/rendering/c_sci_gpu_timer_manager.hpp>
#include <sharedutils/scope_guard.h>

extern DLLCENGINE CEngine *c_engine;

void CGame::RenderScenePresent(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,prosper::Texture &texPostHdr,prosper::Image &outImg)
{
	prosper::util::record_image_barrier(*(*drawCmd),*outImg,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL);
	prosper::util::record_blit_texture(*(*drawCmd),texPostHdr,*outImg);
	prosper::util::record_image_barrier(*(*drawCmd),*(*texPostHdr.GetImage()),Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
	prosper::util::record_image_barrier(*(*drawCmd),*outImg,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
}

std::shared_ptr<prosper::PrimaryCommandBuffer> CGame::GetCurrentDrawCommandBuffer() const {return m_currentDrawCmd.lock();}

//Shader::Base *CGame::GetShaderOverride() {return m_shaderOverride;} // prosper TODO
//void CGame::SetShaderOverride(Shader::Base *shader) {m_shaderOverride = shader;} // prosper TODO

void CGame::RenderScene(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,std::shared_ptr<prosper::RenderTarget> &rt,FRender renderFlags)
{
	m_currentDrawCmd = drawCmd;
	ScopeGuard sgCurrentDrawCmd {[this]() {
		m_currentDrawCmd = {};
	}};

	auto &scene = GetRenderScene();
	auto *renderer = scene->GetRenderer();
	if(renderer)
	{
		renderer->RenderScene(drawCmd,rt,renderFlags);
		StartProfilingStage(CGame::GPUProfilingPhase::Present);
		StartProfilingStage(CGame::CPUProfilingPhase::Present);
		RenderScenePresent(drawCmd,*renderer->GetPresentationTexture(),*rt->GetTexture()->GetImage());
		StopProfilingStage(CGame::CPUProfilingPhase::Present);
		StopProfilingStage(CGame::GPUProfilingPhase::Present);
	}
}
