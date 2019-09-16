#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/renderers/rasterization/hdr_data.hpp"
#include "pragma/rendering/world_environment.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fog.hpp"
#include "pragma/game/c_game.h"
#include <prosper_util.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <image/prosper_render_target.hpp>
#include <image/prosper_msaa_texture.hpp>

using namespace pragma::rendering;

extern DLLCLIENT CGame *c_game;

void RasterizationRenderer::RenderSceneFog(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd)
{
	auto &scene = GetScene();
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
	auto hShaderFog = c_game->GetGameShader(CGame::GameShader::PPFog);
	if(descSetGroupFog != nullptr && hShaderFog.expired() == false)
	{
		auto &shaderFog = static_cast<pragma::ShaderPPFog&>(*hShaderFog.get());
		auto &prepass = hdrInfo.prepass;
		auto texDepth = prepass.textureDepth;
		if(texDepth->IsMSAATexture())
		{
			texDepth = static_cast<prosper::MSAATexture&>(*texDepth).Resolve(
				*(*drawCmd),Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL
			);
		}
		else
			prosper::util::record_image_barrier(*(*drawCmd),*(*texDepth->GetImage()),Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
		//texDepth->GetImage()->SetDrawLayout(Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);

		auto &hdrTex = hdrInfo.sceneRenderTarget->GetTexture();
		prosper::util::record_image_barrier(*(*drawCmd),*(*hdrTex->GetImage()),Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
		if(prosper::util::record_begin_render_pass(*(*drawCmd),*hdrInfo.hdrPostProcessingRenderTarget) == true)
		{
			if(shaderFog.BeginDraw(drawCmd) == true)
			{
				prosper::util::record_buffer_barrier(
					**drawCmd,*scene.GetCameraBuffer(),
					Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT,
					Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
				);
				prosper::util::record_buffer_barrier(
					**drawCmd,*scene.GetRenderSettingsBuffer(),
					Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT,
					Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
				);
				prosper::util::record_buffer_barrier(
					**drawCmd,*scene.GetFogBuffer(),
					Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT,
					Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
				);
				shaderFog.Draw(
					*(*hdrInfo.dsgHDRPostProcessing)->get_descriptor_set(0u),
					*(*hdrInfo.dsgDepthPostProcessing)->get_descriptor_set(0u),
					*scene.GetCameraDescriptorSetGraphics(),
					*(*scene.GetFogDescriptorSetGroup())->get_descriptor_set(0u)
				);
				shaderFog.EndDraw();
			}
			prosper::util::record_end_render_pass(*(*drawCmd));
		}
		prosper::util::record_image_barrier(*(*drawCmd),*(*texDepth->GetImage()),Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		hdrInfo.BlitStagingRenderTargetToMainRenderTarget(*drawCmd);
	}
}
