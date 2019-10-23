#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_hdr.hpp"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree_impl.hpp"
#include "pragma/game/c_game.h"
#include <prosper_descriptor_set_group.hpp>
#include <prosper_util.hpp>
#include <image/prosper_render_target.hpp>

using namespace pragma::rendering;

extern DLLCLIENT CGame *c_game;

void RasterizationRenderer::RenderToneMapping(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,prosper::DescriptorSet &descSetHdrResolve)
{
	auto hShaderTonemapping = c_game->GetGameShader(CGame::GameShader::PPTonemapping);
	if(hShaderTonemapping.expired())
		return;
	auto &hdrInfo = GetHDRInfo();
	auto *srcImg = descSetHdrResolve.GetBoundImage(umath::to_integral(pragma::ShaderPPHDR::TextureBinding::Texture));
	auto *srcImgBloom = descSetHdrResolve.GetBoundImage(umath::to_integral(pragma::ShaderPPHDR::TextureBinding::Bloom));
	auto *srcImgGlow = descSetHdrResolve.GetBoundImage(umath::to_integral(pragma::ShaderPPHDR::TextureBinding::Glow));
	if(IsMultiSampled() == false) // The resolved images already have the correct layout
	{
		prosper::util::record_image_barrier(*(*drawCmd),**srcImg,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
		//prosper::util::record_image_barrier(*(*drawCmd),**srcImgBloom,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
		prosper::util::record_image_barrier(*(*drawCmd),**srcImgGlow,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
	}
	auto &dstTexPostHdr = *hdrInfo.toneMappedRenderTarget->GetTexture();
	auto &dstImgPostHdr = *dstTexPostHdr.GetImage();
	if(prosper::util::record_begin_render_pass(*(*drawCmd),*hdrInfo.toneMappedRenderTarget) == true)
	{
		auto &shaderPPHdr = static_cast<pragma::ShaderPPHDR&>(*hShaderTonemapping);

		if(shaderPPHdr.BeginDraw(drawCmd) == true)
		{
			const float bloomAdditiveScale = 0.5f;
			auto glowScale = (GetGlowInfo().bGlowScheduled == true) ? 1.f : 0.f;
			shaderPPHdr.Draw(*descSetHdrResolve,GetHDRExposure(),bloomAdditiveScale,glowScale);
			shaderPPHdr.EndDraw();
		}
		prosper::util::record_end_render_pass(*(*drawCmd));

		prosper::util::record_post_render_pass_image_barrier(
			**drawCmd,*dstImgPostHdr,
			Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL
		);
	}
	if(IsMultiSampled() == false)
	{
		prosper::util::record_image_barrier(*(*drawCmd),**srcImg,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
		//prosper::util::record_image_barrier(*(*drawCmd),srcImgBloom,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
		prosper::util::record_image_barrier(*(*drawCmd),**srcImgGlow,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
	}
}
