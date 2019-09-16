#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_ssao.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_ssao_blur.hpp"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree_impl.hpp"
#include "pragma/rendering/c_ssao.hpp"
#include "pragma/game/c_game.h"
#include <prosper_command_buffer.hpp>
#include <prosper_util.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <image/prosper_render_target.hpp>
#include <image/prosper_msaa_texture.hpp>

using namespace pragma::rendering;

extern DLLCLIENT CGame *c_game;

void RasterizationRenderer::RenderSSAO(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd)
{
	auto &ssaoInfo = GetSSAOInfo();
	auto *shaderSSAO = static_cast<pragma::ShaderSSAO*>(ssaoInfo.GetSSAOShader());
	auto *shaderSSAOBlur = static_cast<pragma::ShaderSSAOBlur*>(ssaoInfo.GetSSAOBlurShader());
	if(IsSSAOEnabled() == false || shaderSSAO == nullptr || shaderSSAOBlur == nullptr)
		return;
	auto &scene = GetScene();
	c_game->StartProfilingStage(CGame::CPUProfilingPhase::SSAO);
	c_game->StartProfilingStage(CGame::GPUProfilingPhase::SSAO);
	// Pre-render depths, positions and normals (Required for SSAO)
	auto *renderInfo  = GetRenderInfo(RenderMode::World);
	if(renderInfo != nullptr)
	{
		// SSAO
		auto &prepass = GetPrepass();
		auto &ssaoImg = ssaoInfo.renderTarget->GetTexture()->GetImage();

		auto texNormals = prepass.textureNormals;
		auto bNormalsMultiSampled = texNormals->IsMSAATexture();
		if(bNormalsMultiSampled)
		{
			texNormals = static_cast<prosper::MSAATexture&>(*texNormals).Resolve(
				*(*drawCmd),Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,
				Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL
			);
		}
		else
			prosper::util::record_image_barrier(*(*drawCmd),*(*texNormals->GetImage()),Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
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

		prosper::util::record_begin_render_pass(*(*drawCmd),*ssaoInfo.renderTarget);
		auto &renderImage = ssaoInfo.renderTarget->GetTexture()->GetImage();
		auto extents = renderImage.get()->GetExtents();

		if(shaderSSAO->BeginDraw(drawCmd) == true)
		{
			shaderSSAO->Draw(scene,*(*ssaoInfo.descSetGroupPrepass)->get_descriptor_set(0u),{extents.width,extents.height});
			shaderSSAO->EndDraw();
		}

		prosper::util::record_end_render_pass(*(*drawCmd));

		prosper::util::record_image_barrier(*(*drawCmd),*(*texNormals->GetImage()),Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
		prosper::util::record_image_barrier(*(*drawCmd),*(*texDepth->GetImage()),Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		// Blur SSAO
		prosper::util::record_image_barrier(*(*drawCmd),*(*ssaoInfo.renderTargetBlur->GetTexture()->GetImage()),Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
		prosper::util::record_begin_render_pass(*(*drawCmd),*ssaoInfo.renderTargetBlur);

		if(shaderSSAOBlur->BeginDraw(drawCmd) == true)
		{
			shaderSSAOBlur->Draw(*(*ssaoInfo.descSetGroupOcclusion)->get_descriptor_set(0u));
			shaderSSAOBlur->EndDraw();
		}

		prosper::util::record_end_render_pass(*(*drawCmd));
		//

		if(bNormalsMultiSampled)
		{
			prosper::util::record_image_barrier(*(*drawCmd),*(*texNormals->GetImage()),Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
			prosper::util::record_image_barrier(*(*drawCmd),*(*texDepth->GetImage()),Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
		}

		prosper::util::record_image_barrier(*(*drawCmd),*(*ssaoInfo.renderTarget->GetTexture()->GetImage()),Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
	}
	c_game->StopProfilingStage(CGame::GPUProfilingPhase::SSAO);
	c_game->StopProfilingStage(CGame::CPUProfilingPhase::SSAO);
}
