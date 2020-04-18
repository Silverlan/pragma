#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/world_environment.hpp"
#include "pragma/entities/environment/lights/c_env_light.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/entities/environment/lights/c_env_shadow.hpp"
#include "pragma/game/c_game.h"
#include <image/prosper_msaa_texture.hpp>
#include <prosper_util.hpp>
#include <prosper_descriptor_set_group.hpp>

using namespace pragma::rendering;

extern DLLCLIENT CGame *c_game;

void RasterizationRenderer::CullLightSources(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd)
{
	auto &scene = GetScene();
	auto &prepass = GetPrepass();
	{
		c_game->StartProfilingStage(CGame::CPUProfilingPhase::CullLightSources);
		c_game->StartProfilingStage(CGame::GPUProfilingPhase::CullLightSources);
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
		auto &fp = GetForwardPlusInstance();

		// Camera buffer
		prosper::util::record_buffer_barrier(
			**drawCmd,*scene.GetCameraBuffer(),
			Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT,
			Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
		);

		// Render settings buffer
		prosper::util::record_buffer_barrier(
			**drawCmd,*scene.GetRenderSettingsBuffer(),
			Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT,
			Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
		);

		auto *worldEnv = scene.GetWorldEnvironment();
		if(worldEnv->IsUnlit() == false)
		{
			fp.Compute(*drawCmd,*(*depthTex->GetImage()),**scene.GetCameraDescriptorSetCompute());
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
					if(l == nullptr || static_cast<CBaseEntity&>(l->GetEntity()).IsInScene(scene) == false)
						continue;
					culledLightSources.push_back(l);

					auto &renderBuffer = l->GetRenderBuffer();
					if(renderBuffer)
					{
						prosper::util::record_buffer_barrier(
							**drawCmd,*renderBuffer,
							Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT,
							Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
						);
					}
					auto &shadowBuffer = l->GetShadowBuffer();
					if(shadowBuffer)
					{
						prosper::util::record_buffer_barrier(
							**drawCmd,*shadowBuffer,
							Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT,
							Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
						);
					}
				}
			}
		}

		// Don't write to depth image until compute shader has completed reading from it
		if(!bMultisampled)
			prosper::util::record_image_barrier(*(*drawCmd),*(*depthTex->GetImage()),Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		c_game->StopProfilingStage(CGame::GPUProfilingPhase::CullLightSources);
		c_game->StopProfilingStage(CGame::CPUProfilingPhase::CullLightSources);

		c_game->StartProfilingStage(CGame::CPUProfilingPhase::Shadows);
		c_game->StartProfilingStage(CGame::GPUProfilingPhase::Shadows);
		// Update shadows
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

		if(worldEnv->IsUnlit() == false)
		{
			// Note: We want light sources to re-use their previous render target texture if they have one.
			// To achieve this, we simply update their render target by calling 'RequestRenderTarget' again.
			// Shadowmaps that don't have a render target assigned, will get one in 'RenderSystem::RenderShadows'.
			for(auto *l : culledLightSources)
			{
				auto hSm = l->GetShadowMap(pragma::CLightComponent::ShadowMapType::Static);
				if(hSm.valid() && hSm->HasRenderTarget())
					hSm->RequestRenderTarget();

				hSm = l->GetShadowMap(pragma::CLightComponent::ShadowMapType::Dynamic);
				if(hSm.valid() && hSm->HasRenderTarget())
					hSm->RequestRenderTarget();
			}
			RenderSystem::RenderShadows(drawCmd,*this,culledLightSources);
		}
		//c_engine->StopGPUTimer(GPUTimerEvent::Shadow); // prosper TODO
		//drawCmd->SetViewport(w,h); // Reset the viewport

		//auto &imgDepth = textureDepth->GetImage(); // prosper TODO
		//imgDepth->SetDrawLayout(Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL); // prosper TODO
		c_game->StopProfilingStage(CGame::GPUProfilingPhase::Shadows);
		c_game->StopProfilingStage(CGame::CPUProfilingPhase::Shadows);
	}
}
