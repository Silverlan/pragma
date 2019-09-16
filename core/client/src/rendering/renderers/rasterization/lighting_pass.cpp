#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/renderers/rasterization/culled_mesh_data.hpp"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree_impl.hpp"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/entities/environment/lights/c_env_light.h"
#include "pragma/debug/c_debugoverlay.h"
#include "pragma/game/c_game.h"
#include "pragma/console/c_cvar.h"
#include <pragma/console/convars.h>
#include <prosper_util.hpp>

using namespace pragma::rendering;

extern DLLCLIENT CGame *c_game;

static auto cvDrawParticles = GetClientConVar("render_draw_particles");
static auto cvDrawGlow = GetClientConVar("render_draw_glow");
static auto cvDrawTranslucent = GetClientConVar("render_draw_translucent");
void RasterizationRenderer::RenderLightingPass(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,FRender renderFlags)
{
	auto &scene = GetScene();
	auto &cam = scene.GetActiveCamera();
	bool bShadows = (renderFlags &FRender::Shadows) == FRender::Shadows;
	auto &renderMeshes = GetCulledMeshes();

	//ScopeGuard sgDepthImg {};
	auto &culledParticles = GetCulledParticles();
	auto bShouldDrawParticles = (renderFlags &FRender::Particles) == FRender::Particles && cvDrawParticles->GetBool() == true && culledParticles.empty() == false;
	if(bShouldDrawParticles == true)
		SetFrameDepthBufferSamplingRequired();
	if(m_bFrameDepthBufferSamplingRequired == true)
	{
		auto &prepass = GetPrepass();
		auto &dstDepthTex = *prepass.textureDepthSampled;
		auto &ptrDstDepthImg = dstDepthTex.GetImage();
		auto &dstDepthImg = *ptrDstDepthImg;

		auto &hdrInfo = GetHDRInfo();
		std::function<void(prosper::CommandBuffer&)> fTransitionSampleImgToTransferDst = nullptr;
		hdrInfo.BlitMainDepthBufferToSamplableDepthBuffer(*drawCmd,fTransitionSampleImgToTransferDst);

		// If this being commented causes issues, check map test_particles for comparison
		//sgDepthImg = [fTransitionSampleImgToTransferDst,drawCmd,ptrDstDepthImg]() { // Transfer destination image back to TransferDstOptimal layout after render pass has ended
		//prosper::util::record_image_barrier(**drawCmd,**ptrDstDepthImg,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL);
		//fTransitionSampleImgToTransferDst(*drawCmd);
		//};
	}
	m_bFrameDepthBufferSamplingRequired = false;

	// Visible light tile index buffer
	prosper::util::record_buffer_barrier(
		**drawCmd,*GetForwardPlusInstance().GetTileVisLightIndexBuffer(),
		Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT,
		Anvil::AccessFlagBits::SHADER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
	);

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

	auto &bufLightSources = pragma::CLightComponent::GetGlobalRenderBuffer();
	auto &bufShadowData = pragma::CLightComponent::GetGlobalShadowBuffer();
	// Light source data barrier
	prosper::util::record_buffer_barrier(
		**drawCmd,bufLightSources,
		Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT | Anvil::PipelineStageFlagBits::VERTEX_SHADER_BIT | Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT,
		Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
	);

	// Shadow data barrier
	prosper::util::record_buffer_barrier(
		**drawCmd,bufShadowData,
		Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT | Anvil::PipelineStageFlagBits::VERTEX_SHADER_BIT | Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT,
		Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
	);

	auto &renderSettingsBufferData = c_game->GetGlobalRenderSettingsBufferData();
	// Debug buffer barrier
	prosper::util::record_buffer_barrier(
		**drawCmd,*renderSettingsBufferData.debugBuffer,
		Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT,
		Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
	);

	// Time buffer barrier
	prosper::util::record_buffer_barrier(
		**drawCmd,*renderSettingsBufferData.timeBuffer,
		Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT | Anvil::PipelineStageFlagBits::VERTEX_SHADER_BIT,
		Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
	);

	// CSM buffer barrier
	prosper::util::record_buffer_barrier(
		**drawCmd,*renderSettingsBufferData.csmBuffer,
		Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT,
		Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
	);

	// Camera buffer barrier
	prosper::util::record_buffer_barrier(
		**drawCmd,*scene.GetCameraBuffer(),
		Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT | Anvil::PipelineStageFlagBits::VERTEX_SHADER_BIT | Anvil::PipelineStageFlagBits::GEOMETRY_SHADER_BIT,
		Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
	);

	// Render settings buffer barrier
	prosper::util::record_buffer_barrier(
		**drawCmd,*scene.GetRenderSettingsBuffer(),
		Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT | Anvil::PipelineStageFlagBits::VERTEX_SHADER_BIT | Anvil::PipelineStageFlagBits::GEOMETRY_SHADER_BIT,
		Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
	);

	if(BeginRenderPass(drawCmd) == false)
		return;

	c_game->StartProfilingStage(CGame::CPUProfilingPhase::RenderWorld);
	auto bGlow = cvDrawGlow->GetBool();
	auto bTranslucent = cvDrawTranslucent->GetBool();
	auto bReflection = (renderFlags &FRender::Reflection) != FRender::None;
	//c_engine->StartGPUTimer(GPUTimerEvent::Skybox); // prosper TODO
	if((renderFlags &FRender::Skybox) != FRender::None)
	{
		c_game->StartProfilingStage(CGame::GPUProfilingPhase::Skybox);
		c_game->CallCallbacks("PreRenderSkybox");
		RenderSystem::Render(drawCmd,RenderMode::Skybox,bReflection);
		c_game->CallCallbacks("PostRenderSkybox");
		c_game->StopProfilingStage(CGame::GPUProfilingPhase::Skybox);
	}
	//c_engine->StopGPUTimer(GPUTimerEvent::Skybox); // prosper TODO

	// Simple rendering test using a flat (unlit) shader
	/*{
	static auto wpShader = c_engine->GetShaderManager().GetShader("flat");
	if(wpShader.expired() == false)
	{
	auto *shader = static_cast<pragma::ShaderFlat*>(wpShader.get());
	if(shader->BeginDraw(c_engine->GetDrawCommandBuffer(),c_engine->GetWindowWidth(),c_engine->GetWindowHeight()) == true)
	{
	std::vector<BasePlayer*> players;
	GetPlayers(&players);
	auto *pl = dplayers.front();
	shader->BindEntity(*pl);
	shader->BindScene(*scene,false);

	auto &mdl = pl->GetModel();
	auto &mats = mdl->GetMaterials();
	for(auto &meshGroup : mdl->GetMeshGroups())
	{
	for(auto &mesh : meshGroup->GetMeshes())
	{
	for(auto &subMesh : mesh->GetSubMeshes())
	{
	auto *mat = static_cast<CMaterial*>(mdl->GetMaterial(subMesh->GetTexture()));
	if(mat == nullptr)
	continue;
	shader->BindMaterial(*mat);
	shader->Draw(static_cast<CModelSubMesh&>(*subMesh));
	}
	}
	}
	shader->EndDraw();
	}
	}
	}*/

	if((renderFlags &FRender::World) != FRender::None)
	{
		//#ifdef _DEBUG
		//#error "Handle this via RenderMode, not renderflags!"
		//#endif
		c_game->StartProfilingStage(CGame::GPUProfilingPhase::World);
		c_game->CallCallbacks("PreRenderWorld");

		//c_engine->StartGPUTimer(GPUTimerEvent::World); // prosper TODO
		RenderSystem::Render(drawCmd,RenderMode::World,bReflection);
		//c_engine->StopGPUTimer(GPUTimerEvent::World); // prosper TODO

		//c_engine->StartGPUTimer(GPUTimerEvent::WorldTranslucent); // prosper TODO
		auto *renderInfo = GetRenderInfo(RenderMode::World);
		if(renderInfo != nullptr && cam.valid())
			RenderSystem::Render(drawCmd,*cam,RenderMode::World,bReflection,renderInfo->translucentMeshes);
		//c_engine->StopGPUTimer(GPUTimerEvent::WorldTranslucent); // prosper TODO

		c_game->CallCallbacks("PostRenderWorld");
		c_game->StopProfilingStage(CGame::GPUProfilingPhase::World);
	}

	// Start particle sub-pass
	//prosper::util::record_next_sub_pass(**drawCmd);

	//if((renderFlags &FRENDER_WORLD) == FRENDER_WORLD)
	//	RenderWorldEntities(interpolation,entsRender);
	//c_engine->StartGPUTimer(GPUTimerEvent::Particles); // prosper TODO
	if(bShouldDrawParticles)
	{
		c_game->StartProfilingStage(CGame::GPUProfilingPhase::Particles);
		c_game->CallCallbacks("PreRenderParticles");

		auto &glowInfo = GetGlowInfo();

		// Vertex buffer barrier
		prosper::util::record_buffer_barrier(
			**drawCmd,*pragma::CParticleSystemComponent::GetGlobalVertexBuffer(),
			Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::VERTEX_INPUT_BIT,
			Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::VERTEX_ATTRIBUTE_READ_BIT
		);

		for(auto *particle : culledParticles)
		{
			auto &ptBuffer = particle->GetParticleBuffer();
			if (ptBuffer != nullptr)
			{
				// Particle buffer barrier
				prosper::util::record_buffer_barrier(
					**drawCmd, *ptBuffer,
					Anvil::PipelineStageFlagBits::TRANSFER_BIT, Anvil::PipelineStageFlagBits::VERTEX_INPUT_BIT,
					Anvil::AccessFlagBits::TRANSFER_WRITE_BIT, Anvil::AccessFlagBits::VERTEX_ATTRIBUTE_READ_BIT
				);
			}

			auto &animStartBuffer = particle->GetAnimationStartBuffer();
			if (animStartBuffer != nullptr)
			{
				// Animation start buffer barrier
				prosper::util::record_buffer_barrier(
					**drawCmd, *animStartBuffer,
					Anvil::PipelineStageFlagBits::TRANSFER_BIT, Anvil::PipelineStageFlagBits::VERTEX_INPUT_BIT,
					Anvil::AccessFlagBits::TRANSFER_WRITE_BIT, Anvil::AccessFlagBits::VERTEX_ATTRIBUTE_READ_BIT
				);
			}

			auto &animBuffer = particle->GetAnimationBuffer();
			if (animBuffer != nullptr)
			{
				// Animation buffer barrier
				prosper::util::record_buffer_barrier(
					**drawCmd, *animBuffer,
					Anvil::PipelineStageFlagBits::TRANSFER_BIT, Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT,
					Anvil::AccessFlagBits::TRANSFER_WRITE_BIT, Anvil::AccessFlagBits::SHADER_READ_BIT
				);
			}
		}

		RenderParticleSystems(drawCmd,culledParticles,RenderMode::World,false,&glowInfo.tmpBloomParticles);
		if(bGlow == false)
			glowInfo.tmpBloomParticles.clear();
		if(!glowInfo.tmpBloomParticles.empty())
			glowInfo.bGlowScheduled = true;

		c_game->CallCallbacks("PostRenderParticles");
		c_game->StopProfilingStage(CGame::GPUProfilingPhase::Particles);
	}
	//c_engine->StopGPUTimer(GPUTimerEvent::Particles); // prosper TODO

	c_game->CallCallbacks<void>("Render");
	c_game->CallLuaCallbacks("Render");

	//c_engine->StartGPUTimer(GPUTimerEvent::Debug); // prosper TODO
	if((renderFlags &FRender::Debug) == FRender::Debug)
	{
		c_game->StartProfilingStage(CGame::GPUProfilingPhase::Debug);
		c_game->CallCallbacks("PreRenderDebug");

		if(cam.valid())
			DebugRenderer::Render(drawCmd,*cam);
		c_game->RenderDebugPhysics(drawCmd,*cam);
		c_game->CallCallbacks("PostRenderDebug");
		c_game->StopProfilingStage(CGame::GPUProfilingPhase::Debug);
	}
	//c_engine->StopGPUTimer(GPUTimerEvent::Debug); // prosper TODO

	if((renderFlags &FRender::Water) != FRender::None)
	{
		c_game->StartProfilingStage(CGame::GPUProfilingPhase::Water);
		c_game->CallCallbacks("PreRenderWater");

		//c_engine->StartGPUTimer(GPUTimerEvent::Water); // prosper TODO
		auto numShaderInvocations = RenderSystem::Render(drawCmd,RenderMode::Water,bReflection);
		//c_engine->StopGPUTimer(GPUTimerEvent::Water); // prosper TODO

		c_game->CallCallbacks("PostRenderWater");

		if(numShaderInvocations > 0)
		{
			//auto &prepass = scene->GetPrepass(); // prosper TODO
			// Note: The texture buffer was probably already resolved at this point (e.g. by particle render pass), however
			// we will need the water depth values for the upcoming render passes as well, so we'll have to resolve again.
			// To do that, we'll reset the MSAA texture, so at the next resolve-call it will update accordingly.
			//prepass.textureDepth->Reset(); // prosper TODO
			//prepass.textureDepth->Resolve(); // prosper TODO
		}
		c_game->StopProfilingStage(CGame::GPUProfilingPhase::Water);
	}

	if((renderFlags &FRender::View) != FRender::None)
	{
		auto *pl = c_game->GetLocalPlayer();
		if(pl != nullptr && pl->IsInFirstPersonMode())
		{
			c_game->StartProfilingStage(CGame::GPUProfilingPhase::View);
			c_game->CallCallbacks("PreRenderView");

			//c_engine->StartGPUTimer(GPUTimerEvent::View); // prosper TODO
			RenderSystem::Render(drawCmd,RenderMode::View,bReflection);
			//c_engine->StopGPUTimer(GPUTimerEvent::View); // prosper TODO

			//c_engine->StartGPUTimer(GPUTimerEvent::ViewParticles); // prosper TODO
			if((renderFlags &FRender::Particles) == FRender::Particles)
			{
				auto &culledParticles = GetCulledParticles();
				auto &glowInfo = GetGlowInfo();
				RenderParticleSystems(drawCmd,culledParticles,RenderMode::View,false,&glowInfo.tmpBloomParticles);
				if(bGlow == false)
					glowInfo.tmpBloomParticles.clear();
				if(!glowInfo.tmpBloomParticles.empty())
					glowInfo.bGlowScheduled = true;
			}
			//c_engine->StartGPUTimer(GPUTimerEvent::ViewParticles); // prosper TODO
			//RenderGlowMeshes(cam,true);

			c_game->CallCallbacks("PostRenderView");
			c_game->StopProfilingStage(CGame::GPUProfilingPhase::View);
		}
	}
	EndRenderPass(drawCmd);
	c_game->StopProfilingStage(CGame::CPUProfilingPhase::RenderWorld);
	/*
	At this point:
	* m_renderTextures[0] contains the actual scene rendering
	* m_renderTextures[1] contains bright color components of actual scene (Used for bloom)
	* m_glowFBO contains all glowing objects
	*/

	// Blur our glow / bloom effects
	/*static auto *pp = static_cast<ShaderGaussianBlur*>(GetShader("pp_gaussianblur"));

	// Bloom
	int w,h;
	OpenGL::GetViewportSize(&w,&h);
	auto tmpBuffer = m_glowBlurFBO; // Using m_glowBlurFBO as a temporary buffer, since it's not in use at this point
	auto tmpTexture = m_glowBlurTexture;
	OpenGL::BindFrameBuffer(m_renderFBO,GL_READ_FRAMEBUFFER);
	OpenGL::BindFrameBuffer(tmpBuffer,GL_DRAW_FRAMEBUFFER);
	glReadBuffer(GL_COLOR_ATTACHMENT1); // We only need the bloom components
	OpenGL::BlitFrameBuffer(w,h); // Multi-Sample texture to regular texture
	glReadBuffer(GL_COLOR_ATTACHMENT0);

	glDepthMask(false);

	static float blurSize = 5.f;
	static int kernelSize = 9;
	static int blurAmount = 5; // Needs to be uneven

	for(decltype(blurAmount) i=0;i<blurAmount;i++)
	pp->Render(((i %2) == 0) ? tmpTexture : m_bloomTexture,((i %2) == 0) ? m_bloomFBO : tmpBuffer,blurSize,kernelSize); // Blur our bloom texture

	// Bloom texture is added to screen framebuffer in c_game.cpp:Think
	//

	// Glow
	if(pp != nullptr && m_bFirstGlow == false) // Only do this if we actually have any glowing objects on screen
	{
	const float blurSize = 1.75f;
	const int kernelSize = 3;
	const int blurAmount = 5; // Needs to be uneven

	for(auto i=0;i<blurAmount;i++)
	pp->Render(((i %2) == 0) ? m_glowTexture : m_glowBlurTexture,((i %2) == 0) ? m_glowBlurFBO : m_glowFBO,blurSize,kernelSize); // Blur our bloom texture
	*/ // Vulkan TODO
	/*#ifndef _DEBUG
	#error Disable me
	static unsigned int nextRender = 0;
	nextRender++;
	if(nextRender >= 100)
	{
	nextRender = 0;
	int w,h;
	OpenGL::GetViewportSize(&w,&h);
	//client->SaveTextureAsTGA("test_glow.tga",GL_TEXTURE_2D);
	client->SaveFrameBufferAsTGA("test_glow.tga",0,0,w,h,GL_RGB);
	}
	#endif*/ // Debug code; Saves the glow framebuffer as image every 100 frames
	/*	OpenGL::UseProgram(0);
	}
	//
	glDepthMask(true);
	if(drawWorld == 2)
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

	//m_shaderScreen->Render(m_renderTexture,m_renderScreenVertexBuffer);
	*/ // Vulkan TODO
}
