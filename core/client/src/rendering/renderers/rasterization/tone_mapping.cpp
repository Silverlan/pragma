/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_hdr.hpp"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree_impl.hpp"
#include "pragma/game/c_game.h"
#include <prosper_descriptor_set_group.hpp>
#include <prosper_util.hpp>
#include <image/prosper_render_target.hpp>

using namespace pragma::rendering;

extern DLLCLIENT CGame *c_game;

void RasterizationRenderer::RenderToneMapping(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,prosper::IDescriptorSet &descSetHdrResolve)
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
		drawCmd->RecordImageBarrier(*srcImg,prosper::ImageLayout::ColorAttachmentOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal);
		//.RecordImageBarrier(*(*drawCmd),**srcImgBloom,prosper::ImageLayout::ColorAttachmentOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal);
		drawCmd->RecordImageBarrier(*srcImgGlow,prosper::ImageLayout::ColorAttachmentOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal);
	}
	auto &dstTexPostHdr = hdrInfo.toneMappedRenderTarget->GetTexture();
	auto &dstImgPostHdr = dstTexPostHdr.GetImage();
	if(drawCmd->RecordBeginRenderPass(*hdrInfo.toneMappedRenderTarget) == true)
	{
		auto &shaderPPHdr = static_cast<pragma::ShaderPPHDR&>(*hShaderTonemapping);

		if(shaderPPHdr.BeginDraw(drawCmd) == true)
		{
			const float bloomAdditiveScale = 0.5f;
			auto glowScale = (GetGlowInfo().bGlowScheduled == true) ? 1.f : 0.f;
			shaderPPHdr.Draw(descSetHdrResolve,GetHDRExposure(),bloomAdditiveScale,glowScale);
			shaderPPHdr.EndDraw();
		}
		drawCmd->RecordEndRenderPass();

		drawCmd->RecordPostRenderPassImageBarrier(
			dstImgPostHdr,
			prosper::ImageLayout::ColorAttachmentOptimal,prosper::ImageLayout::TransferSrcOptimal
		);
	}
	if(IsMultiSampled() == false)
	{
		drawCmd->RecordImageBarrier(*srcImg,prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::ColorAttachmentOptimal);
		//.RecordImageBarrier(*(*drawCmd),srcImgBloom,prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::ColorAttachmentOptimal);
		drawCmd->RecordImageBarrier(*srcImgGlow,prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::ColorAttachmentOptimal);
	}
}
