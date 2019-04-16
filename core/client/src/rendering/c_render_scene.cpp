#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fog.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_hdr.hpp"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_ssao.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_ssao_blur.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fxaa.hpp"
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
#pragma optimize("",off)
void CGame::RenderSceneFog(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd)
{
	auto &scene = GetRenderScene();
	auto &hdrInfo = scene->GetHDRInfo();
	auto descSetGroupFog = m_descSetGroupFogOverride;
	if(descSetGroupFog == nullptr)
	{
		auto *worldEnv = scene->GetWorldEnvironment();
		if(worldEnv != nullptr)
		{
			auto &fog = worldEnv->GetFogSettings();
			if(fog.IsEnabled() == true)
				descSetGroupFog = scene->GetFogDescriptorSetGroup();
		}
	}
	auto hShaderFog = GetGameShader(GameShader::PPFog);
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

		auto &hdrTex = hdrInfo.hdrRenderTarget->GetTexture();
		prosper::util::record_image_barrier(*(*drawCmd),*(*hdrTex->GetImage()),Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
		if(prosper::util::record_begin_render_pass(*(*drawCmd),*hdrInfo.hdrStagingRenderTarget) == true)
		{
			if(shaderFog.BeginDraw(drawCmd) == true)
			{
				prosper::util::record_buffer_barrier(
					**drawCmd,*scene->GetCameraBuffer(),
					Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT,
					Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
				);
				prosper::util::record_buffer_barrier(
					**drawCmd,*scene->GetRenderSettingsBuffer(),
					Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT,
					Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
				);
				prosper::util::record_buffer_barrier(
					**drawCmd,*scene->GetFogBuffer(),
					Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT,
					Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
				);
				shaderFog.Draw(
					*(*hdrInfo.descSetGroupHdr)->get_descriptor_set(0u),
					*(*hdrInfo.descSetGroupDepthPostProcessing)->get_descriptor_set(0u),
					*scene->GetCameraDescriptorSetGraphics(),
					*(*scene->GetFogDescriptorSetGroup())->get_descriptor_set(0u)
				);
				shaderFog.EndDraw();
			}
			prosper::util::record_end_render_pass(*(*drawCmd));
		}
		prosper::util::record_image_barrier(*(*drawCmd),*(*texDepth->GetImage()),Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		hdrInfo.BlitStagingRenderTargetToMainRenderTarget(*drawCmd);
	}
}

static auto cvAntiAliasing = GetClientConVar("cl_render_anti_aliasing");
static auto cvFxaaSubPixelAliasingRemoval = GetClientConVar("cl_render_fxaa_sub_pixel_aliasing_removal_amount");
static auto cvFxaaEdgeThreshold = GetClientConVar("cl_render_fxaa_edge_threshold");
static auto cvFxaaMinEdgeThreshold = GetClientConVar("cl_render_fxaa_min_edge_threshold");
void CGame::RenderScenePostProcessing(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,FRender renderFlags)
{
	c_engine->StartGPUTimer(GPUTimerEvent::PostProcessingFog);
	RenderSceneFog(drawCmd);
	c_engine->StopGPUTimer(GPUTimerEvent::PostProcessingFog);

	auto &scene = GetRenderScene();
	auto &hdrInfo = scene->GetHDRInfo();
	if(static_cast<AntiAliasing>(cvAntiAliasing->GetInt()) == AntiAliasing::FXAA)
	{
		c_engine->StartGPUTimer(GPUTimerEvent::PostProcessingFXAA);
		// HDR needs to be resolved before FXAA is applied
		// Note: This will be undone by FXAA shader! (because HDR is required for post-processing)
		auto &descSetGroupHdrResolve = hdrInfo.descSetGroupHdrResolve;
		RenderSceneResolveHDR(drawCmd,*(*descSetGroupHdrResolve)->get_descriptor_set(0u),true);

		auto &scene = GetRenderScene();
		auto &hdrInfo = scene->GetHDRInfo();
		auto whShaderPPFXAA = GetGameShader(GameShader::PPFXAA);
		if(whShaderPPFXAA.valid() == true)
		{
			auto &shaderFXAA = static_cast<pragma::ShaderPPFXAA&>(*whShaderPPFXAA.get());
			auto &prepass = hdrInfo.prepass;

			auto &hdrTex = hdrInfo.postHdrRenderTarget->GetTexture();
			prosper::util::record_image_barrier(*(*drawCmd),*(*hdrTex->GetImage()),Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
			if(prosper::util::record_begin_render_pass(*(*drawCmd),*hdrInfo.hdrStagingRenderTarget) == true)
			{
				if(shaderFXAA.BeginDraw(drawCmd) == true)
				{
					pragma::ShaderPPFXAA::PushConstants pushConstants {};
					pushConstants.subPixelAliasingRemoval = cvFxaaSubPixelAliasingRemoval->GetFloat();
					pushConstants.edgeThreshold = cvFxaaEdgeThreshold->GetFloat();
					pushConstants.minEdgeThreshold = cvFxaaMinEdgeThreshold->GetFloat();

					shaderFXAA.Draw(
						*(*hdrInfo.descSetGroupPostHdr)->get_descriptor_set(0u),
						pushConstants
					);
					shaderFXAA.EndDraw();
				}
				prosper::util::record_end_render_pass(*(*drawCmd));
			}
			prosper::util::record_image_barrier(*(*drawCmd),*(*hdrInfo.hdrRenderTarget->GetTexture()->GetImage()),Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
			prosper::util::record_image_barrier(*(*drawCmd),*(*hdrTex->GetImage()),Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
			hdrInfo.BlitStagingRenderTargetToMainRenderTarget(*drawCmd);
		}
		c_engine->StopGPUTimer(GPUTimerEvent::PostProcessingFXAA);
	}

	// Glow Effects
	//c_engine->StartGPUTimer(GPUTimerEvent::Glow); // prosper TODO
	auto &glowInfo = scene->GetGlowInfo();
	if(glowInfo.bGlowScheduled == true)
	{
		/*auto depthTex = scene->GetPrepass().textureDepth;
		if(depthTex->IsMSAATexture())
			depthTex = static_cast<prosper::MSAATexture&>(*depthTex).Resolve(*drawCmd,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL);
		prosper::util::record_image_barrier(
			*drawCmd,*depthTex->GetImage(),
			Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL
		);
		auto &depthTex = scene->ResolveDepthTexture(drawCmd); // Glow render pass isn't multisampled, but requires scene depth buffer
		depthTex->GetImage()->SetDrawLayout(Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		auto &tex = glowInfo.renderTarget.GetTexture();
		tex->GetImage()->SetDrawLayout(Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);*/ // prosper TODO
		c_engine->StartGPUTimer(GPUTimerEvent::PostProcessingGlow);
		prosper::util::record_begin_render_pass(*(*drawCmd),*glowInfo.renderTarget,{
			vk::ClearValue{vk::ClearColorValue{std::array<float,4>{0.f,0.f,0.f,1.f}}},
			vk::ClearValue{vk::ClearDepthStencilValue{}}
		});
			if(!glowInfo.tmpBloomParticles.empty())
			{
				const auto interpolation = 0.f; // TODO
				RenderParticleSystems(drawCmd,glowInfo.tmpBloomParticles,interpolation,RenderMode::World,true);
				RenderParticleSystems(drawCmd,glowInfo.tmpBloomParticles,interpolation,RenderMode::View,true);
				glowInfo.tmpBloomParticles.clear();
			}
			for(auto i=std::underlying_type_t<RenderMode>{0};i<umath::to_integral(RenderMode::Count);++i)
			{
				auto *renderInfo = scene->GetRenderInfo(static_cast<RenderMode>(i));
				if(renderInfo == nullptr || renderInfo->glowMeshes.empty() == true)
					continue;
				RenderGlowMeshes(drawCmd,*scene,static_cast<RenderMode>(i));
			}

		prosper::util::record_end_render_pass(*(*drawCmd));

		const auto blurAmount = 5;
		auto &blurImg = *glowInfo.blurSet->GetFinalRenderTarget()->GetTexture()->GetImage();
		for(auto i=decltype(blurAmount){0u};i<blurAmount;++i)
		{
			prosper::util::record_blur_image(c_engine->GetDevice(),drawCmd,*glowInfo.blurSet,{
				Vector4(1.f,1.f,1.f,1.f), /* color scale */
				1.75f, /* blur size */
				3 /* kernel size */
			});
		}

		prosper::util::record_image_barrier(*(*drawCmd),*(*glowInfo.renderTarget->GetTexture()->GetImage()),Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
		c_engine->StopGPUTimer(GPUTimerEvent::PostProcessingGlow);

#if 0
		//drawCmd->ExecuteCommand(m_glowInfo.cmdBufferBlur);
		static auto hShader = ShaderSystem::get_shader("pp_gaussianblur");
		auto &shaderBlur = static_cast<Shader::GaussianBlur&>(*hShader.get());

		//drawCmd->InsertBarrier(m_glowInfo.renderTarget.GetTexture()->GetImage(),Vulkan::get_image_read_access_mask(),Vulkan::get_image_write_access_mask(),Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
		//drawCmd->InsertBarrier(m_glowInfo.renderTarget.GetTexture()->GetImage(),Vulkan::get_image_write_access_mask(),Vulkan::get_image_read_access_mask(),Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
		//drawCmd->SetImageLayout(m_glowInfo.renderTarget.GetTexture()->GetImage(),Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
		//drawCmd->SetImageLayout(m_glowInfo.renderTarget.GetTexture()->GetImage(),Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
		//drawCmd->InsertBarrier(m_glowInfo.renderTarget.GetTexture()->GetImage());
		//m_glowInfo.cmdBufferBlur->InsertBarrier(m_glowInfo.renderTarget.GetTexture()->GetImage());
		//for(auto i=decltype(blurAmount){0};i<blurAmount;++i)
		//	shaderBlur.Render(drawCmd,m_glowInfo.renderTarget.GetRenderPass(),Vector2i(width,height),m_glowInfo.renderTarget.GetTexture(),m_glowInfo.renderTarget.blurBuffer,m_glowInfo.renderTarget.GetFramebuffer(),blurSize,kernelSize);
		//drawCmd->ExecuteCommand(glowInfo.cmdBufferBlur);
		glowInfo.cmdBufferBlur(drawCmd); // prosper TODO
		glowInfo.renderTarget.blurBuffer.tmpBlurTexture->GetImage()->SetInternalLayout(Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
		glowInfo.renderTarget.GetImage()->SetInternalLayout(Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
#endif // prosper TODO
	}
	//c_engine->StopGPUTimer(GPUTimerEvent::Glow); // prosper TODO
	//

	CallCallbacks<void,FRender>("RenderPostProcessing",renderFlags);
	CallLuaCallbacks("RenderPostProcessing");

	// Bloom
	c_engine->StartGPUTimer(GPUTimerEvent::PostProcessingBloom);
	auto bloomTexMsaa = hdrInfo.hdrRenderTarget->GetTexture(1u);
	// Blit high-res bloom image into low-res image, which is cheaper to blur
	prosper::util::record_image_barrier(*(*drawCmd),**hdrInfo.bloomTexture->GetImage(),Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL);
	prosper::util::record_image_barrier(*(*drawCmd),**hdrInfo.bloomBlurTexture->GetImage(),Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL);
	prosper::util::record_blit_texture(**drawCmd,*hdrInfo.bloomTexture,**hdrInfo.bloomBlurTexture->GetImage());

	const auto blurSize = 5.f;
	const auto kernelSize = 9u;
	const auto blurAmount = 5u;
	prosper::util::record_image_barrier(*(*drawCmd),**hdrInfo.bloomBlurTexture->GetImage(),Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
	for(auto i=decltype(blurAmount){0};i<blurAmount;++i)
	{
		prosper::util::record_blur_image(c_engine->GetDevice(),drawCmd,*hdrInfo.bloomBlurSet,{
			Vector4(1.f,1.f,1.f,1.f),
			blurSize,
			kernelSize
		});
	}
	prosper::util::record_image_barrier(*(*drawCmd),**hdrInfo.bloomTexture->GetImage(),Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
	c_engine->StopGPUTimer(GPUTimerEvent::PostProcessingBloom);
	//
	
	c_engine->StartGPUTimer(GPUTimerEvent::PostProcessingHDR);
	auto &descSetGroupHdrResolve = hdrInfo.descSetGroupHdrResolve;
	RenderSceneResolveHDR(drawCmd,*(*descSetGroupHdrResolve)->get_descriptor_set(0u));
	c_engine->StopGPUTimer(GPUTimerEvent::PostProcessingHDR);
}

void CGame::RenderSceneResolveHDR(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,Anvil::DescriptorSet &descSetHdrResolve,bool toneMappingOnly)
{
	auto &scene = GetRenderScene();
	auto &hdrInfo = scene->GetHDRInfo();
	if(hdrInfo.shaderPPHdr.expired())
		return;
	auto &srcImg = *prosper::util::get_descriptor_set_image(descSetHdrResolve,umath::to_integral(pragma::ShaderPPHDR::TextureBinding::Texture));
	auto &srcImgBloom = *prosper::util::get_descriptor_set_image(descSetHdrResolve,umath::to_integral(pragma::ShaderPPHDR::TextureBinding::Bloom));
	auto &srcImgGlow = *prosper::util::get_descriptor_set_image(descSetHdrResolve,umath::to_integral(pragma::ShaderPPHDR::TextureBinding::Glow));
	if(scene->IsMultiSampled() == false) // The resolved images already have the correct layout
	{
		prosper::util::record_image_barrier(*(*drawCmd),srcImg,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
		//prosper::util::record_image_barrier(*(*drawCmd),srcImgBloom,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
		prosper::util::record_image_barrier(*(*drawCmd),srcImgGlow,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
	}
	auto &dstTexPostHdr = *hdrInfo.postHdrRenderTarget->GetTexture();
	auto &dstImgPostHdr = *dstTexPostHdr.GetImage();
	if(prosper::util::record_begin_render_pass(*(*drawCmd),*hdrInfo.postHdrRenderTarget) == true)
	{
		auto &shaderPPHdr = static_cast<pragma::ShaderPPHDR&>(*hdrInfo.shaderPPHdr.get());
		
		if(shaderPPHdr.BeginDraw(drawCmd) == true)
		{
			const float bloomAdditiveScale = 0.5f;
			auto glowScale = (scene->GetGlowInfo().bGlowScheduled == true) ? 1.f : 0.f;
			shaderPPHdr.Draw(descSetHdrResolve,GetHDRExposure(),bloomAdditiveScale,glowScale,toneMappingOnly);
			shaderPPHdr.EndDraw();
		}
		prosper::util::record_end_render_pass(*(*drawCmd));
	}
	if(scene->IsMultiSampled() == false)
	{
		prosper::util::record_image_barrier(*(*drawCmd),srcImg,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
		//prosper::util::record_image_barrier(*(*drawCmd),srcImgBloom,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
		prosper::util::record_image_barrier(*(*drawCmd),srcImgGlow,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
	}
}

void CGame::RenderScenePresent(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,prosper::Texture &texPostHdr,prosper::Image &outImg)
{
	prosper::util::record_image_barrier(*(*drawCmd),*outImg,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL);
	prosper::util::record_blit_texture(*(*drawCmd),texPostHdr,*outImg);
	prosper::util::record_image_barrier(*(*drawCmd),*(*texPostHdr.GetImage()),Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
	prosper::util::record_image_barrier(*(*drawCmd),*outImg,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
}

void CGame::RenderScenePrepass(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd)
{
	auto &scene = GetRenderScene();
	auto &prepass = scene->GetPrepass();
	auto bProfiling = c_engine->IsProfilingEnabled();
	{
		c_engine->StartGPUTimer(GPUTimerEvent::CullLightSources);
		auto depthTex = prepass.textureDepth;
		auto bMultisampled = depthTex->IsMSAATexture();
		if(depthTex->IsMSAATexture())
		{
			depthTex = static_cast<prosper::MSAATexture&>(*depthTex).Resolve(
				*(*drawCmd),Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL
			);
		}
		else
			prosper::util::record_image_barrier(*(*drawCmd),*(*depthTex->GetImage()),Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);

		static std::vector<pragma::CLightComponent*> culledLightSources;
		culledLightSources.clear();
		auto &fp = scene->GetForwardPlusInstance();

		// Camera buffer
		prosper::util::record_buffer_barrier(
			**drawCmd,*scene->GetCameraBuffer(),
			Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT,
			Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
		);

		// Render settings buffer
		prosper::util::record_buffer_barrier(
			**drawCmd,*scene->GetRenderSettingsBuffer(),
			Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT,
			Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
		);
		fp.Compute(*drawCmd,*(*depthTex->GetImage()),*scene->GetCameraDescriptorSetCompute());
		auto &lightBits = fp.GetShadowLightBits();
		for(auto i=decltype(lightBits.size()){0};i<lightBits.size();++i)
		{
			auto &intVal = lightBits.at(i);
			const auto numBits = 32u;
			for(auto j=0u;j<numBits;++j)
			{
				if(!(intVal &(1<<j))) // If bit is set, this light is visible on screen
					continue;
				auto shadowIdx = i *numBits +j;
				auto *l = pragma::CLightComponent::GetLightByShadowBufferIndex(shadowIdx);
				if(l == nullptr || scene->HasLightSource(*l) == false)
					continue;
				culledLightSources.push_back(l);
			}
		}

		// Don't write to depth image until compute shader has completed reading from it
		if(!bMultisampled)
			prosper::util::record_image_barrier(*(*drawCmd),*(*depthTex->GetImage()),Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		
		scene->UpdateLightDescriptorSets(culledLightSources);
		c_engine->StopGPUTimer(GPUTimerEvent::CullLightSources);

		c_engine->StartGPUTimer(GPUTimerEvent::Shadows);
		// Update shadows
		if(bProfiling == true)
			StartStageProfiling(umath::to_integral(ProfilingStage::ClientGameRenderShadows)); // TODO: Only for main scene
		//c_engine->StartGPUTimer(GPUTimerEvent::Shadow); // TODO: Only for main scene // prosper TODO

		// Entity instance buffer barrier
		prosper::util::record_buffer_barrier(
			**drawCmd,*pragma::CRenderComponent::GetInstanceBuffer(),
			Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT | Anvil::PipelineStageFlagBits::VERTEX_SHADER_BIT | Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT,
			Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
		);

		// Entity bone buffer barrier
		prosper::util::record_buffer_barrier(
			**drawCmd,*pragma::get_instance_bone_buffer(),
			Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT | Anvil::PipelineStageFlagBits::VERTEX_SHADER_BIT | Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT,
			Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
		);
		
			RenderSystem::RenderShadows(drawCmd,culledLightSources);
		//c_engine->StopGPUTimer(GPUTimerEvent::Shadow); // prosper TODO
			//drawCmd->SetViewport(w,h); // Reset the viewport
		if(bProfiling == true)
			EndStageProfiling(umath::to_integral(ProfilingStage::ClientGameRenderShadows));

		for(auto *l : culledLightSources)
		{
			if(l == nullptr)
				continue;
			auto *shadowMap = l->GetShadowMap();
			if(shadowMap == nullptr)
				continue;
			/*auto *depthTex = shadowMap->GetDepthTexture();
			if(depthTex == nullptr || (*depthTex == nullptr))
				continue;
			(*depthTex)->GetImage()->SetDrawLayout(Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);*/ // prosper TODO
		}

		//auto &imgDepth = textureDepth->GetImage(); // prosper TODO
		//imgDepth->SetDrawLayout(Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL); // prosper TODO
		c_engine->StopGPUTimer(GPUTimerEvent::Shadows);
	}
}

std::shared_ptr<prosper::PrimaryCommandBuffer> CGame::GetCurrentDrawCommandBuffer() const {return m_currentDrawCmd.lock();}

static auto cvDrawSky = GetClientConVar("render_draw_sky");
static auto cvDrawWater = GetClientConVar("render_draw_water");
static auto cvDrawGlow = GetClientConVar("render_draw_glow");
static auto cvDrawView = GetClientConVar("render_draw_view");
static auto cvDrawTranslucent = GetClientConVar("render_draw_translucent");
void CGame::RenderScene(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,std::shared_ptr<prosper::RenderTarget> &rt,FRender renderFlags)//const Vulkan::RenderPass &renderPass,const Vulkan::Framebuffer &framebuffer,const Vulkan::CommandBuffer &drawCmd,FRender renderFlags,const Color *clearColor)
{
	m_currentDrawCmd = drawCmd;
	ScopeGuard sgCurrentDrawCmd {[this]() {
		m_currentDrawCmd = {};
	}};

	auto &scene = GetRenderScene();
	scene->BeginDraw();
	auto &hdrInfo = scene->GetHDRInfo();
	//auto &rt = scene->GetRenderTarget();
	// Prepare multi-buffers
	//auto &context = const_cast<Vulkan::Context&>(c_engine->GetRenderContext());

	// Update lights
	//scene->CullLightSources();
	//auto &culledLights = scene->GetCulledLights();

	// Update particles
	scene->GetOcclusionCullingHandler().PerformCulling(*scene,scene->GetCulledParticles());

	//auto &img = hdrInfo.texture->GetImage();
	//auto w = img->GetWidth();
	//auto h = img->GetHeight();
	auto bProfiling = c_engine->IsProfilingEnabled();

	/*if(bProfiling == true)
		StartStageProfiling(umath::to_integral(ProfilingStage::ClientGameRenderShadows)); // TODO: Only for main scene
	c_engine->StartGPUTimer(GPUTimerEvent::Shadow); // TODO: Only for main scene
		RenderSystem::RenderShadows(culledLights); // TODO: Only update if not yet updated in this frame
	c_engine->StopGPUTimer(GPUTimerEvent::Shadow);
		drawCmd->SetViewport(w,h); // Reset the viewport
	if(bProfiling == true)
		EndStageProfiling(umath::to_integral(ProfilingStage::ClientGameRenderShadows));*/
	scene->UpdateBuffers(drawCmd);
	
	CallCallbacks<void>("OnPreRender");

	// Render Shadows
	auto &cam = scene->camera;
	if(bProfiling == true)
		StartStageProfiling(umath::to_integral(ProfilingStage::ClientGameOcclusionCulling));
	auto &renderMeshes = scene->GetCulledMeshes();
	scene->GetOcclusionCullingHandler().PerformCulling(*scene,renderMeshes);
	if(bProfiling == true)
		EndStageProfiling(umath::to_integral(ProfilingStage::ClientGameOcclusionCulling));
	
	/*ScopeGuard resetViewport([&drawCmd]() {
		auto &context = c_engine->GetRenderContext();
		auto w = context.GetWidth();
		auto h = context.GetHeight();
		drawCmd->SetViewport(w,h);
		drawCmd->SetScissor(w,h);
	});*/ // prosper TODO

	// Shadow Test

	//
	auto interpolation = 0.f;
	if(bProfiling == true)
		StartStageProfiling(umath::to_integral(ProfilingStage::ClientGameRenderScene));

	// Prepare rendering
	auto bGlow = cvDrawGlow->GetBool();
	auto bTranslucent = cvDrawTranslucent->GetBool();
	if((renderFlags &FRender::Skybox) == FRender::Skybox && IsRenderModeEnabled(RenderMode::Skybox) && cvDrawSky->GetBool() == true)
		scene->PrepareRendering(RenderMode::Skybox,bTranslucent,bGlow);
	else
		renderFlags &= ~FRender::Skybox;

	if((renderFlags &FRender::World) == FRender::World && IsRenderModeEnabled(RenderMode::World))
		scene->PrepareRendering(RenderMode::World,bTranslucent,bGlow);
	else
		renderFlags &= ~FRender::World;

	if((renderFlags &FRender::Water) == FRender::Water && IsRenderModeEnabled(RenderMode::Water) && cvDrawWater->GetBool() == true)
		scene->PrepareRendering(RenderMode::Water,bTranslucent,bGlow);
	else
		renderFlags &= ~FRender::Water;

	auto *pl = GetLocalPlayer();
	if((renderFlags &FRender::View) == FRender::View && IsRenderModeEnabled(RenderMode::View) && pl != nullptr && pl->IsInFirstPersonMode() == true && cvDrawView->GetBool() == true)
		scene->PrepareRendering(RenderMode::View,bTranslucent,bGlow);
	else
		renderFlags &= ~FRender::View;
	//
	
	auto &gpuTimerManager = c_engine->GetGPUTimerManager();
	gpuTimerManager.StartTimer(GPUTimerEvent::Scene);
	// Experimental (Deferred shading)
	{
		// Pre-render depths and normals (if SSAO is enabled)
		auto prepassMode = scene->GetPrepassMode();
		if(prepassMode != Scene::PrepassMode::NoPrepass)
		{
			gpuTimerManager.StartTimer(GPUTimerEvent::Prepass);
			auto &prepass = scene->GetPrepass();
			if(prepass.textureDepth->IsMSAATexture())
				static_cast<prosper::MSAATexture&>(*prepass.textureDepth).Reset();
			if(prepass.textureNormals != nullptr && prepass.textureNormals->IsMSAATexture())
				static_cast<prosper::MSAATexture&>(*prepass.textureNormals).Reset();

			// Entity instance buffer barrier
			prosper::util::record_buffer_barrier(
				**drawCmd,*pragma::CRenderComponent::GetInstanceBuffer(),
				Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT | Anvil::PipelineStageFlagBits::VERTEX_SHADER_BIT | Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT,
				Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
			);

			// Entity bone buffer barrier
			prosper::util::record_buffer_barrier(
				**drawCmd,*pragma::get_instance_bone_buffer(),
				Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT | Anvil::PipelineStageFlagBits::VERTEX_SHADER_BIT | Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT,
				Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
			);

			// Camera buffer barrier
			prosper::util::record_buffer_barrier(
				**drawCmd,*scene->GetCameraBuffer(),
				Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT | Anvil::PipelineStageFlagBits::VERTEX_SHADER_BIT | Anvil::PipelineStageFlagBits::GEOMETRY_SHADER_BIT,
				Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
			);

			// View camera buffer barrier
			prosper::util::record_buffer_barrier(
				**drawCmd,*scene->GetViewCameraBuffer(),
				Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT,
				Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
			);

			prepass.BeginRenderPass(*drawCmd);
				auto bReflection = ((renderFlags &FRender::Reflection) != FRender::None) ? true : false;
				auto pipelineType = (bReflection == true) ? pragma::ShaderPrepassBase::Pipeline::Reflection :
					(scene->GetSampleCount() == Anvil::SampleCountFlagBits::_1_BIT) ? pragma::ShaderPrepassBase::Pipeline::Regular :
					pragma::ShaderPrepassBase::Pipeline::MultiSample;
				auto &shaderDepthStage = scene->GetPrepass().GetShader();
				if(shaderDepthStage.BeginDraw(drawCmd,pipelineType) == true)
				{
					shaderDepthStage.BindClipPlane(GetRenderClipPlane());
					shaderDepthStage.BindSceneCamera(*scene,false);
					if((renderFlags &FRender::Skybox) != FRender::None)
					{
						gpuTimerManager.StartTimer(GPUTimerEvent::PrepassSkybox);
						RenderSystem::RenderPrepass(drawCmd,*scene->GetCamera(),scene->GetCulledMeshes(),RenderMode::Skybox,bReflection);
						gpuTimerManager.StopTimer(GPUTimerEvent::PrepassSkybox);
					}
					if((renderFlags &FRender::World) != FRender::None)
					{
						gpuTimerManager.StartTimer(GPUTimerEvent::PrepassWorld);
						RenderSystem::RenderPrepass(drawCmd,*scene->GetCamera(),scene->GetCulledMeshes(),RenderMode::World,bReflection);
						gpuTimerManager.StopTimer(GPUTimerEvent::PrepassWorld);
					}
					CallCallbacks<void>("RenderPrepass");
					CallLuaCallbacks("RenderPrepass");

					shaderDepthStage.BindSceneCamera(*scene,true);
					if((renderFlags &FRender::View) != FRender::None)
					{
						gpuTimerManager.StartTimer(GPUTimerEvent::PrepassView);
						RenderSystem::RenderPrepass(drawCmd,*scene->GetCamera(),scene->GetCulledMeshes(),RenderMode::View,bReflection);
						gpuTimerManager.StopTimer(GPUTimerEvent::PrepassView);
					}
					shaderDepthStage.EndDraw();
				}
			prepass.EndRenderPass(*drawCmd);
			gpuTimerManager.StopTimer(GPUTimerEvent::Prepass);
		}
		

		auto &ssaoInfo = scene->GetSSAOInfo();
		auto *shaderSSAO = static_cast<pragma::ShaderSSAO*>(ssaoInfo.GetSSAOShader());
		auto *shaderSSAOBlur = static_cast<pragma::ShaderSSAOBlur*>(ssaoInfo.GetSSAOBlurShader());
		if(scene->IsSSAOEnabled() == true && shaderSSAO != nullptr && shaderSSAOBlur != nullptr)
		{
			gpuTimerManager.StartTimer(GPUTimerEvent::SSAO);
			// Pre-render depths, positions and normals (Required for SSAO)
			auto *renderInfo  = scene->GetRenderInfo(RenderMode::World);
			if(renderInfo != nullptr)
			{
				/*auto &imgPos = ssaoInfo.texturePositions->GetImage();
				auto &imgNormals = ssaoInfo.textureNormals->GetImage();
				auto &imgDepth = ssaoInfo.textureDepth->GetImage();
				imgPos->SetDrawLayout(Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
				imgNormals->SetDrawLayout(Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
				imgDepth->SetDrawLayout(Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
				drawCmd->BeginRenderPass(ssaoInfo.renderPass,ssaoInfo.framebuffer,w,h,{
					vk::ClearValue{vk::ClearColorValue{std::array<float,4>{0.f,0.f,0.f,1.f}}}, // Position buffer
					vk::ClearValue{vk::ClearColorValue{std::array<float,4>{0.f,0.f,0.f,1.f}}}, // Normal buffer
					vk::ClearValue{vk::ClearDepthStencilValue{1.f,0}} // Clear depth
				});
					RenderSystem::RenderDepths(*scene->GetCamera(),scene->GetCulledMeshes());
				drawCmd->EndRenderPass();*/

				// SSAO
				auto &prepass = scene->GetPrepass();
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
						shaderSSAO->Draw(*scene,*(*ssaoInfo.descSetGroupPrepass)->get_descriptor_set(0u),{extents.width,extents.height});
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
			gpuTimerManager.StopTimer(GPUTimerEvent::SSAO);
		}
	}
	//

	// Experimental (Forward+)
	RenderScenePrepass(drawCmd);

		/*scene->BeginRenderPass(drawCmd,clearColor); // prosper TODO
			//if(drawWorld == 2)
			//	drawCmd->ClearAttachment(0,hdrInfo.texture->GetImage()); // Vulkan TODO: Only if wireframe!!
		auto &hdrInfo = scene->GetHDRInfo();
		hdrInfo.texture->Reset();
		hdrInfo.textureBloom->Reset();
		c_engine->StartGPUTimer(GPUTimerEvent::GameRender);*/

	Render(drawCmd,interpolation,renderFlags); // TODO: Interpolation
	
	auto &texHdr = hdrInfo.hdrRenderTarget->GetTexture();
	if(texHdr->IsMSAATexture())
	{
		static_cast<prosper::MSAATexture&>(*texHdr).Resolve(
			**drawCmd,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,
			Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL
		);
	}
	gpuTimerManager.StopTimer(GPUTimerEvent::Scene);

	gpuTimerManager.StartTimer(GPUTimerEvent::PostProcessing);
	RenderScenePostProcessing(drawCmd,renderFlags);
	gpuTimerManager.StopTimer(GPUTimerEvent::PostProcessing);

	gpuTimerManager.StartTimer(GPUTimerEvent::Present);
	RenderScenePresent(drawCmd,*hdrInfo.postHdrRenderTarget->GetTexture(),*rt->GetTexture()->GetImage());
	gpuTimerManager.StopTimer(GPUTimerEvent::Present);
	
	auto bloomTexMsaa = hdrInfo.hdrRenderTarget->GetTexture(1u);
	if(bloomTexMsaa->IsMSAATexture())
		static_cast<prosper::MSAATexture&>(*bloomTexMsaa).Reset();

	/*c_engine->StopGPUTimer(GPUTimerEvent::GameRender); // prosper TODO
	
	scene->EndRenderPass(drawCmd);
	if(bProfiling == true)
		EndStageProfiling(umath::to_integral(ProfilingStage::ClientGameRenderScene));*/

	static uint32_t skipRenderPass = 0
		//| 1 // Glow Effects
		//| 2 // Bloom
		//| 4 // Additive
		//| 8 // Additive 2
		//| 16 // HDR
		//| 32 // Post Render
	;

	if(bProfiling == true)
		StartStageProfiling(umath::to_integral(ProfilingStage::ClientGameRenderPostProcessing));

	if(bProfiling == true)
		EndStageProfiling(umath::to_integral(ProfilingStage::ClientGameRenderPostProcessing));
}

//Shader::Base *CGame::GetShaderOverride() {return m_shaderOverride;} // prosper TODO
//void CGame::SetShaderOverride(Shader::Base *shader) {m_shaderOverride = shader;} // prosper TODO
#pragma optimize("",on)
