#include "stdafx_client.h"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/renderers/rasterization/culled_mesh_data.hpp"
#include "pragma/rendering/renderers/rasterization/hdr_data.hpp"
#include "pragma/rendering/renderers/rasterization/glow_data.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_glow.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fog.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fxaa.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_hdr.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_ssao.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_ssao_blur.hpp"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/rendering/c_settings.hpp"
#include "pragma/console/c_cvar.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <shader/prosper_shader_blur.hpp>
#include <image/prosper_render_target.hpp>
#include <image/prosper_msaa_texture.hpp>
#include <sharedutils/util_shaderinfo.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

using namespace pragma::rendering;

static auto cvDrawParticles = GetClientConVar("render_draw_particles");
static auto cvDrawGlow = GetClientConVar("render_draw_glow");
static auto cvDrawTranslucent = GetClientConVar("render_draw_translucent");
void RasterizationRenderer::Render(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,float interpolation,FRender renderFlags)
{
	// TODO: Transform positions:
	// pos += velocity *interpolation;
	// Re-Implement interpolation
	auto &scene = GetScene();
	auto &cam = scene.GetActiveCamera();
	//bool bWorld = (renderFlags &FRENDER_WORLD) == FRENDER_WORLD;
	bool bShadows = (renderFlags &FRender::Shadows) == FRender::Shadows;
	//Scene *scene = GetRenderScene();
	//std::vector<CHCMeshInfo> &renderMeshes = scene->chc->PerformCulling();
	auto &renderMeshes = GetCulledMeshes();
	/*	if(bShadows == true)
	{
	auto &lights = OcclusionCulling::PerformCulling(GetLights());
	RenderSystem::RenderShadows(lights,cam,renderMeshes);
	OpenGL::BindVertexArray(0);
	}*/

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

		RenderParticleSystems(drawCmd,culledParticles,interpolation,RenderMode::World,false,&glowInfo.tmpBloomParticles);
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
				RenderParticleSystems(drawCmd,culledParticles,interpolation,RenderMode::View,false,&glowInfo.tmpBloomParticles);
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

void RasterizationRenderer::RenderGlowMeshes(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,RenderMode renderMode)
{
	auto &glowInfo = GetGlowInfo();
	auto *shader = static_cast<pragma::ShaderGlow*>(glowInfo.shader.get());
	if(shader == nullptr)
		return;
	auto *renderInfo = GetRenderInfo(renderMode);
	if(renderInfo == nullptr)
		return;
	if(shader->BeginDraw(drawCmd) == true)
	{
		shader->BindSceneCamera(*this,(renderMode == RenderMode::View) ? true : false);
		for(auto &matContainer : renderInfo->glowMeshes)
		{
			auto *mat = matContainer->material;
			if(shader->BindGlowMaterial(static_cast<CMaterial&>(*mat)) == true)
			{
				for(auto &pair : matContainer->containers)
				{
					shader->BindEntity(*pair.first);
					for(auto *mesh : pair.second.meshes)
						shader->Draw(*mesh);
				}
			}
		}
		shader->EndDraw();
	}
}

void RasterizationRenderer::GetRenderEntities(std::vector<CBaseEntity*> &entsRender)
{
	// prosper TODO
#if 0
	auto &scene = GetRenderScene();
	auto &cam = scene->camera;
	//Vector3 &posCam = cam->GetPos();
	bool bCull = cvCulling->GetBool();
	//bool bFogEnabled = FogController::IsFogEnabled();
	float fogDist;
	//if(bFogEnabled == true)
	//	fogDist = FogController::GetFarDistance();

	auto &planes = cam->GetFrustumPlanes();
	for(int i=0;i<m_entsOccluded.size();i++)
	{
		CBaseEntity *ent = m_entsOccluded[i];
		auto &pos = ent->GetPosition();
		Sphere &sp = ent->GetRenderSphereBounds();
		Vector3 posSphere = pos +sp.pos;
		if(bCull == false || Intersection::SphereInPlaneMesh(posSphere,sp.radius,planes) != INTERSECT_OUTSIDE)
		{
			Vector3 min,max;
			ent->GetRenderBounds(&min,&max);
			if(bCull == false || Intersection::AABBInPlaneMesh(pos +min,pos +max,planes) != INTERSECT_OUTSIDE)
				entsRender.push_back(ent);
		}
	}
#endif
}

void RasterizationRenderer::RenderParticleSystems(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,std::vector<pragma::CParticleSystemComponent*> &particles,float interpolation,RenderMode renderMode,Bool bloom,std::vector<pragma::CParticleSystemComponent*> *bloomParticles)
{
	auto bFirst = true;
	for(auto *particle : particles)
	{
		if(particle != nullptr && particle->IsActive() == true && particle->GetRenderMode() == renderMode)
		{
			if(bFirst == true)
			{
				bFirst = false;

				// We need to end the current render pass, because we need the depth buffer with everything
				// that has been rendered thus far.
				EndRenderPass(drawCmd);

				auto &hdrInfo = GetHDRInfo();
				auto &prepass = GetPrepass();
				if(prepass.textureDepth->IsMSAATexture())
				{
					auto &msaaTex = static_cast<prosper::MSAATexture&>(*prepass.textureDepth);
					msaaTex.Resolve(
						**drawCmd,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
						Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL
					); // Particles aren't multisampled, but requires scene depth buffer
					msaaTex.Reset(); // Depth buffer isn't complete yet; We need to reset, otherwise the next resolve will not update it properly
				}
				//else
				//	prosper::util::record_image_barrier(**drawCmd,**prepass.textureDepth->GetImage(),Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);

				// Restart render pass
				BeginRenderPass(drawCmd,hdrInfo.rpIntermediate.get());
			}
			//scene->ResolveDepthTexture(drawCmd); // Particles aren't multisampled, but requires scene depth buffer
			particle->Render(drawCmd,*this,bloom);
			if(bloomParticles != nullptr)
			{
				if(particle->GetBloomScale() != 0.f)
					bloomParticles->push_back(particle);
				auto &children = particle->GetChildren();
				bloomParticles->reserve(bloomParticles->size() +children.size());
				for(auto &hChild : children)
				{
					if(hChild.expired())
						continue;
					bloomParticles->push_back(hChild.get());
				}
			}
		}
	}
}

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

		auto &hdrTex = hdrInfo.hdrRenderTarget->GetTexture();
		prosper::util::record_image_barrier(*(*drawCmd),*(*hdrTex->GetImage()),Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
		if(prosper::util::record_begin_render_pass(*(*drawCmd),*hdrInfo.hdrStagingRenderTarget) == true)
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
					*(*hdrInfo.descSetGroupHdr)->get_descriptor_set(0u),
					*(*hdrInfo.descSetGroupDepthPostProcessing)->get_descriptor_set(0u),
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

static auto cvAntiAliasing = GetClientConVar("cl_render_anti_aliasing");
static auto cvFxaaSubPixelAliasingRemoval = GetClientConVar("cl_render_fxaa_sub_pixel_aliasing_removal_amount");
static auto cvFxaaEdgeThreshold = GetClientConVar("cl_render_fxaa_edge_threshold");
static auto cvFxaaMinEdgeThreshold = GetClientConVar("cl_render_fxaa_min_edge_threshold");
void RasterizationRenderer::RenderScenePostProcessing(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,FRender renderFlags)
{
	c_game->StartProfilingStage(CGame::GPUProfilingPhase::PostProcessingFog);
	RenderSceneFog(drawCmd);
	c_game->StopProfilingStage(CGame::GPUProfilingPhase::PostProcessingFog);

	auto &hdrInfo = GetHDRInfo();
	if(static_cast<AntiAliasing>(cvAntiAliasing->GetInt()) == AntiAliasing::FXAA)
	{
		c_game->StartProfilingStage(CGame::GPUProfilingPhase::PostProcessingFXAA);
		// HDR needs to be resolved before FXAA is applied
		// Note: This will be undone by FXAA shader! (because HDR is required for post-processing)
		auto &descSetGroupHdrResolve = hdrInfo.descSetGroupHdrResolve;
		RenderSceneResolveHDR(drawCmd,*(*descSetGroupHdrResolve)->get_descriptor_set(0u),true);

		auto &hdrInfo = GetHDRInfo();
		auto whShaderPPFXAA = c_game->GetGameShader(CGame::GameShader::PPFXAA);
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

				prosper::util::record_post_render_pass_image_barrier(
					**drawCmd,**hdrInfo.hdrStagingRenderTarget->GetTexture()->GetImage(),
					Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL
				);
			}
			prosper::util::record_image_barrier(*(*drawCmd),*(*hdrInfo.hdrRenderTarget->GetTexture()->GetImage()),Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
			prosper::util::record_image_barrier(*(*drawCmd),*(*hdrTex->GetImage()),Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
			hdrInfo.BlitStagingRenderTargetToMainRenderTarget(*drawCmd);
		}
		c_game->StopProfilingStage(CGame::GPUProfilingPhase::PostProcessingFXAA);
	}

	// Glow Effects
	//c_engine->StartGPUTimer(GPUTimerEvent::Glow); // prosper TODO
	auto &glowInfo = GetGlowInfo();
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
		c_game->StartProfilingStage(CGame::GPUProfilingPhase::PostProcessingGlow);
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
			auto *renderInfo = GetRenderInfo(static_cast<RenderMode>(i));
			if(renderInfo == nullptr || renderInfo->glowMeshes.empty() == true)
				continue;
			RenderGlowMeshes(drawCmd,static_cast<RenderMode>(i));
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
		c_game->StopProfilingStage(CGame::GPUProfilingPhase::PostProcessingGlow);

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

	c_game->CallCallbacks<void,FRender>("RenderPostProcessing",renderFlags);
	c_game->CallLuaCallbacks("RenderPostProcessing");

	// Bloom
	c_game->StartProfilingStage(CGame::GPUProfilingPhase::PostProcessingBloom);
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
	c_game->StopProfilingStage(CGame::GPUProfilingPhase::PostProcessingBloom);
	//
	
	if(umath::is_flag_set(renderFlags,FRender::HDR))
	{
		// Don't bother resolving HDR; Just apply the barrier
		prosper::util::record_image_barrier(
			*(*drawCmd),**hdrInfo.hdrRenderTarget->GetTexture()->GetImage(),
			Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL
		);
		return;
	}
	c_game->StartProfilingStage(CGame::GPUProfilingPhase::PostProcessingHDR);
	auto &descSetGroupHdrResolve = hdrInfo.descSetGroupHdrResolve;
	RenderSceneResolveHDR(drawCmd,*(*descSetGroupHdrResolve)->get_descriptor_set(0u));
	c_game->StopProfilingStage(CGame::GPUProfilingPhase::PostProcessingHDR);
}

void RasterizationRenderer::RenderSceneResolveHDR(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,Anvil::DescriptorSet &descSetHdrResolve,bool toneMappingOnly)
{
	auto &hdrInfo = GetHDRInfo();
	if(hdrInfo.shaderPPHdr.expired())
		return;
	auto &srcImg = *prosper::util::get_descriptor_set_image(descSetHdrResolve,umath::to_integral(pragma::ShaderPPHDR::TextureBinding::Texture));
	auto &srcImgBloom = *prosper::util::get_descriptor_set_image(descSetHdrResolve,umath::to_integral(pragma::ShaderPPHDR::TextureBinding::Bloom));
	auto &srcImgGlow = *prosper::util::get_descriptor_set_image(descSetHdrResolve,umath::to_integral(pragma::ShaderPPHDR::TextureBinding::Glow));
	if(IsMultiSampled() == false) // The resolved images already have the correct layout
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
			auto glowScale = (GetGlowInfo().bGlowScheduled == true) ? 1.f : 0.f;
			shaderPPHdr.Draw(descSetHdrResolve,GetHDRExposure(),bloomAdditiveScale,glowScale,toneMappingOnly);
			shaderPPHdr.EndDraw();
		}
		prosper::util::record_end_render_pass(*(*drawCmd));

		prosper::util::record_post_render_pass_image_barrier(
			**drawCmd,**hdrInfo.postHdrRenderTarget->GetTexture()->GetImage(),
			Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL
		);
	}
	if(IsMultiSampled() == false)
	{
		prosper::util::record_image_barrier(*(*drawCmd),srcImg,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
		//prosper::util::record_image_barrier(*(*drawCmd),srcImgBloom,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
		prosper::util::record_image_barrier(*(*drawCmd),srcImgGlow,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
	}
}

void RasterizationRenderer::RenderScenePrepass(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd)
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
			fp.Compute(*drawCmd,*(*depthTex->GetImage()),*scene.GetCameraDescriptorSetCompute());
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
					if(l == nullptr || scene.HasLightSource(*l) == false)
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

		UpdateLightDescriptorSets(culledLightSources);
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
			RenderSystem::RenderShadows(drawCmd,culledLightSources);
		}
		//c_engine->StopGPUTimer(GPUTimerEvent::Shadow); // prosper TODO
		//drawCmd->SetViewport(w,h); // Reset the viewport

		//auto &imgDepth = textureDepth->GetImage(); // prosper TODO
		//imgDepth->SetDrawLayout(Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL); // prosper TODO
		c_game->StopProfilingStage(CGame::GPUProfilingPhase::Shadows);
		c_game->StopProfilingStage(CGame::CPUProfilingPhase::Shadows);
	}
}

static auto cvDrawSky = GetClientConVar("render_draw_sky");
static auto cvDrawWater = GetClientConVar("render_draw_water");
static auto cvDrawView = GetClientConVar("render_draw_view");
bool RasterizationRenderer::RenderScene(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,FRender renderFlags)
{
	if(BaseRenderer::RenderScene(drawCmd,renderFlags) == false)
		return false;
	auto &hdrInfo = GetHDRInfo();
	//auto &rt = scene->GetRenderTarget();
	// Prepare multi-buffers
	//auto &context = const_cast<Vulkan::Context&>(c_engine->GetRenderContext());

	// Update lights
	//scene->CullLightSources();
	//auto &culledLights = scene->GetCulledLights();

	// Update particles
	c_game->StartProfilingStage(CGame::CPUProfilingPhase::OcclusionCulling);
	GetOcclusionCullingHandler().PerformCulling(*this,GetCulledParticles());

	//auto &img = hdrInfo.texture->GetImage();
	//auto w = img->GetWidth();
	//auto h = img->GetHeight();

	/*if(bProfiling == true)
	StartStageProfiling(umath::to_integral(ProfilingStage::ClientGameRenderShadows)); // TODO: Only for main scene
	c_engine->StartGPUTimer(GPUTimerEvent::Shadow); // TODO: Only for main scene
	RenderSystem::RenderShadows(culledLights); // TODO: Only update if not yet updated in this frame
	c_engine->StopGPUTimer(GPUTimerEvent::Shadow);
	drawCmd->SetViewport(w,h); // Reset the viewport
	if(bProfiling == true)
	EndStageProfiling(umath::to_integral(ProfilingStage::ClientGameRenderShadows));*/

	c_game->CallCallbacks<void>("OnPreRender");

	// Render Shadows
	auto &scene = GetScene();
	auto &cam = scene.GetActiveCamera();
	auto &renderMeshes = GetCulledMeshes();
	GetOcclusionCullingHandler().PerformCulling(*this,renderMeshes);
	c_game->StopProfilingStage(CGame::CPUProfilingPhase::OcclusionCulling);

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

	// Prepare rendering
	c_game->StartProfilingStage(CGame::CPUProfilingPhase::PrepareRendering);
	auto bGlow = cvDrawGlow->GetBool();
	auto bTranslucent = cvDrawTranslucent->GetBool();
	if((renderFlags &FRender::Skybox) == FRender::Skybox && c_game->IsRenderModeEnabled(RenderMode::Skybox) && cvDrawSky->GetBool() == true)
		PrepareRendering(RenderMode::Skybox,bTranslucent,bGlow);
	else
		renderFlags &= ~FRender::Skybox;

	if((renderFlags &FRender::World) == FRender::World && c_game->IsRenderModeEnabled(RenderMode::World))
		PrepareRendering(RenderMode::World,bTranslucent,bGlow);
	else
		renderFlags &= ~FRender::World;

	if((renderFlags &FRender::Water) == FRender::Water && c_game->IsRenderModeEnabled(RenderMode::Water) && cvDrawWater->GetBool() == true)
		PrepareRendering(RenderMode::Water,bTranslucent,bGlow);
	else
		renderFlags &= ~FRender::Water;

	auto *pl = c_game->GetLocalPlayer();
	if((renderFlags &FRender::View) == FRender::View && c_game->IsRenderModeEnabled(RenderMode::View) && pl != nullptr && pl->IsInFirstPersonMode() == true && cvDrawView->GetBool() == true)
		PrepareRendering(RenderMode::View,bTranslucent,bGlow);
	else
		renderFlags &= ~FRender::View;
	c_game->StopProfilingStage(CGame::CPUProfilingPhase::PrepareRendering);
	//

	c_game->StartProfilingStage(CGame::GPUProfilingPhase::Scene);
	// Experimental (Deferred shading)
	{
		// Pre-render depths and normals (if SSAO is enabled)
		auto prepassMode = GetPrepassMode();
		if(prepassMode != PrepassMode::NoPrepass)
		{
			c_game->StartProfilingStage(CGame::CPUProfilingPhase::Prepass);
			c_game->StartProfilingStage(CGame::GPUProfilingPhase::Prepass);
			auto &prepass = GetPrepass();
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
				**drawCmd,*scene.GetCameraBuffer(),
				Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT | Anvil::PipelineStageFlagBits::VERTEX_SHADER_BIT | Anvil::PipelineStageFlagBits::GEOMETRY_SHADER_BIT,
				Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
			);

			// View camera buffer barrier
			prosper::util::record_buffer_barrier(
				**drawCmd,*scene.GetViewCameraBuffer(),
				Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT,
				Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
			);

			prepass.BeginRenderPass(*drawCmd);
			auto bReflection = ((renderFlags &FRender::Reflection) != FRender::None) ? true : false;
			auto pipelineType = (bReflection == true) ? pragma::ShaderPrepassBase::Pipeline::Reflection :
				(GetSampleCount() == Anvil::SampleCountFlagBits::_1_BIT) ? pragma::ShaderPrepassBase::Pipeline::Regular :
				pragma::ShaderPrepassBase::Pipeline::MultiSample;
			auto &shaderDepthStage = GetPrepass().GetShader();
			if(shaderDepthStage.BeginDraw(drawCmd,pipelineType) == true)
			{
				shaderDepthStage.BindClipPlane(c_game->GetRenderClipPlane());
				shaderDepthStage.BindSceneCamera(*this,false);
				if((renderFlags &FRender::Skybox) != FRender::None)
				{
					c_game->StartProfilingStage(CGame::GPUProfilingPhase::PrepassSkybox);
					if(cam.valid())
						RenderSystem::RenderPrepass(drawCmd,*cam,GetCulledMeshes(),RenderMode::Skybox,bReflection);
					c_game->StopProfilingStage(CGame::GPUProfilingPhase::PrepassSkybox);
				}
				if((renderFlags &FRender::World) != FRender::None)
				{
					c_game->StartProfilingStage(CGame::GPUProfilingPhase::PrepassWorld);
					if(cam.valid())
						RenderSystem::RenderPrepass(drawCmd,*cam,GetCulledMeshes(),RenderMode::World,bReflection);
					c_game->StopProfilingStage(CGame::GPUProfilingPhase::PrepassWorld);
				}
				c_game->CallCallbacks<void>("RenderPrepass");
				c_game->CallLuaCallbacks("RenderPrepass");

				shaderDepthStage.BindSceneCamera(*this,true);
				if((renderFlags &FRender::View) != FRender::None)
				{
					c_game->StartProfilingStage(CGame::GPUProfilingPhase::PrepassView);
					if(cam.valid())
						RenderSystem::RenderPrepass(drawCmd,*cam,GetCulledMeshes(),RenderMode::View,bReflection);
					c_game->StopProfilingStage(CGame::GPUProfilingPhase::PrepassView);
				}
				shaderDepthStage.EndDraw();
			}
			prepass.EndRenderPass(*drawCmd);
			c_game->StopProfilingStage(CGame::GPUProfilingPhase::Prepass);
			c_game->StopProfilingStage(CGame::CPUProfilingPhase::Prepass);
		}


		auto &ssaoInfo = GetSSAOInfo();
		auto *shaderSSAO = static_cast<pragma::ShaderSSAO*>(ssaoInfo.GetSSAOShader());
		auto *shaderSSAOBlur = static_cast<pragma::ShaderSSAOBlur*>(ssaoInfo.GetSSAOBlurShader());
		if(IsSSAOEnabled() == true && shaderSSAO != nullptr && shaderSSAOBlur != nullptr)
		{
			c_game->StartProfilingStage(CGame::CPUProfilingPhase::SSAO);
			c_game->StartProfilingStage(CGame::GPUProfilingPhase::SSAO);
			// Pre-render depths, positions and normals (Required for SSAO)
			auto *renderInfo  = GetRenderInfo(RenderMode::World);
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
	c_game->StopProfilingStage(CGame::GPUProfilingPhase::Scene);

	c_game->StartProfilingStage(CGame::CPUProfilingPhase::PostProcessing);
	c_game->StartProfilingStage(CGame::GPUProfilingPhase::PostProcessing);
	RenderScenePostProcessing(drawCmd,renderFlags);
	c_game->StopProfilingStage(CGame::GPUProfilingPhase::PostProcessing);
	c_game->StopProfilingStage(CGame::CPUProfilingPhase::PostProcessing);

	auto bloomTexMsaa = hdrInfo.hdrRenderTarget->GetTexture(1u);
	if(bloomTexMsaa->IsMSAATexture())
		static_cast<prosper::MSAATexture&>(*bloomTexMsaa).Reset();

	/*c_engine->StopGPUTimer(GPUTimerEvent::GameRender); // prosper TODO

	scene->EndRenderPass(drawCmd);
	if(bProfiling == true)
	EndStageProfiling(umath::to_integral(ProfilingStage::ClientGameRenderScene));*/
	return true;
}

static auto cvDrawWorld = GetClientConVar("render_draw_world");
void RasterizationRenderer::PrepareRendering(RenderMode renderMode,bool bUpdateTranslucentMeshes,bool bUpdateGlowMeshes)
{
	auto it = m_renderInfo.find(renderMode);
	if(it == m_renderInfo.end())
		it = m_renderInfo.insert(decltype(m_renderInfo)::value_type(renderMode,std::make_shared<CulledMeshData>())).first;

	auto &renderInfo = it->second;
	auto &cam = GetScene().GetActiveCamera();
	auto &posCam = cam.valid() ? cam->GetEntity().GetPosition() : uvec::ORIGIN;

	auto drawWorld = cvDrawWorld->GetInt();
	auto *matLoad = c_game->GetLoadMaterial();
	auto &renderMeshes = GetCulledMeshes();
	auto &glowMeshes = renderInfo->glowMeshes;
	auto &translucentMeshes = renderInfo->translucentMeshes;
	auto &processed = renderInfo->processed;
	auto &containers = renderInfo->containers;
	glowMeshes.clear();
	translucentMeshes.clear();
	processed.clear();
	containers.clear();

	for(auto it=renderMeshes.begin();it!=renderMeshes.end();++it)
	{
		auto &info = *it;
		auto *ent = static_cast<CBaseEntity*>(info.hEntity.get());
		auto pRenderComponent = ent->GetRenderComponent();
		if(pRenderComponent.expired())
			continue;
		auto rm = pRenderComponent->GetRenderMode();
		if(renderMode == rm)
		{
			auto itProcessed = renderInfo->processed.find(ent);
			if(itProcessed == renderInfo->processed.end())
			{
				auto drawCmd = c_game->GetCurrentDrawCommandBuffer();
				pRenderComponent->UpdateRenderData(drawCmd);//,true);
				pRenderComponent->Render(renderMode);

				auto wpRenderBuffer = pRenderComponent->GetRenderBuffer();
				if(wpRenderBuffer.expired() == false)
				{
					prosper::util::record_buffer_barrier(
						**drawCmd,*wpRenderBuffer.lock(),
						Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::VERTEX_SHADER_BIT | Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT,
						Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
					);
					auto pAnimComponent = ent->GetAnimatedComponent();
					if(pAnimComponent.valid())
					{
						auto wpBoneBuffer = static_cast<pragma::CAnimatedComponent*>(pAnimComponent.get())->GetBoneBuffer();
						if(wpBoneBuffer.expired() == false)
						{
							prosper::util::record_buffer_barrier(
								**drawCmd,*wpBoneBuffer.lock(),
								Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::VERTEX_SHADER_BIT | Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT,
								Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
							);
						}
					}
				}
				processed.insert(std::remove_reference_t<decltype(processed)>::value_type(ent,true));
			}

			auto &mdlComponent = pRenderComponent->GetModelComponent();
			auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
			assert(mdl != nullptr);
			auto &materials = mdl->GetMaterials();
			if(!materials.empty())
			{
				auto *mesh = static_cast<CModelMesh*>(info.mesh);
				auto &meshes = mesh->GetSubMeshes();
				for(auto it=meshes.begin();it!=meshes.end();++it)
				{
					auto *subMesh = static_cast<CModelSubMesh*>(it->get());
					auto idxTexture = subMesh->GetTexture();
					auto *mat = mdl->GetMaterial(mdlComponent->GetSkin(),idxTexture);
					if(mat == nullptr)
						mat = client->GetMaterialManager().GetErrorMaterial();
					/*else
					{
						auto *diffuse = mat->GetDiffuseMap();
						if(diffuse == nullptr || diffuse->texture == nullptr)
							mat = client->GetMaterialManager().GetErrorMaterial();
					}*/
					if(mat != nullptr)
					{
						if(!mat->IsLoaded())
							mat = matLoad;
						//auto &hMat = materials[idxTexture];
						//if(hMat.IsValid())
						//{
						if(mat != nullptr)// && mat->GetDiffuseMap() != nullptr && static_cast<Texture*>(mat->GetDiffuseMap()->texture) != nullptr &&static_cast<Texture*>(mat->GetDiffuseMap()->texture)->error == false && static_cast<Texture*>(mat->GetDiffuseMap()->texture)->GetTextureID() != 0)
						{
							// Fill glow map
							auto *glowMap = mat->GetGlowMap();
							if(bUpdateGlowMeshes == true)
							{
								if(glowMap != nullptr)
								{
									auto itMat = std::find_if(glowMeshes.begin(),glowMeshes.end(),[&mat](const std::unique_ptr<RenderSystem::MaterialMeshContainer> &m) {
										return (m->material == mat) ? true : false;
									});
									if(itMat == glowMeshes.end())
									{
										glowMeshes.push_back(std::make_unique<RenderSystem::MaterialMeshContainer>(mat));
										itMat = glowMeshes.end() -1;
									}
									auto itEnt = (*itMat)->containers.find(ent);
									if(itEnt == (*itMat)->containers.end())
										itEnt = (*itMat)->containers.emplace(ent,EntityMeshInfo{ent}).first;
									itEnt->second.meshes.push_back(subMesh);
								}
							}
							//
							auto *info = mat->GetShaderInfo();
							if(info != nullptr)
							{
								auto *base = static_cast<::util::WeakHandle<prosper::Shader>*>(const_cast<util::ShaderInfo*>(info)->GetShader().get())->get();
								prosper::Shader *shader = nullptr;
								if(drawWorld == 2)
									shader = m_whShaderWireframe.get();
								else if(base != nullptr && base->GetBaseTypeHashCode() == pragma::ShaderTextured3DBase::HASH_TYPE)
									shader = GetShaderOverride(static_cast<pragma::ShaderTextured3D*>(base));
								if(shader != nullptr && shader->GetBaseTypeHashCode() == pragma::ShaderTextured3DBase::HASH_TYPE)
								{
									// Translucent?
									if(mat->IsTranslucent() == true)
									{
										if(bUpdateTranslucentMeshes == true)
										{
											auto pTrComponent = ent->GetTransformComponent();
											auto pos = subMesh->GetCenter();
											if(pTrComponent.valid())
											{
												uvec::rotate(&pos,pTrComponent->GetOrientation());
												pos += pTrComponent->GetPosition();
											}
											auto distance = uvec::length_sqr(pos -posCam);
											translucentMeshes.push_back(std::make_unique<RenderSystem::TranslucentMesh>(ent,subMesh,mat,shader->GetHandle(),distance));
										}
										continue; // Skip translucent meshes
									}
									//
									ShaderMeshContainer *shaderContainer = nullptr;
									auto itShader = std::find_if(containers.begin(),containers.end(),[shader](const std::unique_ptr<ShaderMeshContainer> &c) {
										return (c->shader.get() == shader) ? true : false;
									});
									if(itShader != containers.end())
										shaderContainer = itShader->get();
									if(shaderContainer == nullptr)
									{
										if(containers.size() == containers.capacity())
											containers.reserve(containers.capacity() +10);
										containers.push_back(std::make_unique<ShaderMeshContainer>(static_cast<pragma::ShaderTextured3D*>(shader)));
										shaderContainer = containers.back().get();
									}
									RenderSystem::MaterialMeshContainer *matContainer = nullptr;
									auto itMat = std::find_if(shaderContainer->containers.begin(),shaderContainer->containers.end(),[mat](const std::unique_ptr<RenderSystem::MaterialMeshContainer> &m) {
										return (m->material == mat) ? true : false;
									});
									if(itMat != shaderContainer->containers.end())
										matContainer = itMat->get();
									if(matContainer == nullptr)
									{
										if(shaderContainer->containers.size() == shaderContainer->containers.capacity())
											shaderContainer->containers.reserve(shaderContainer->containers.capacity() +10);
										shaderContainer->containers.push_back(std::make_unique<RenderSystem::MaterialMeshContainer>(mat));
										matContainer = shaderContainer->containers.back().get();
									}
									EntityMeshInfo *entContainer = nullptr;
									auto itEnt = matContainer->containers.find(ent);
									if(itEnt != matContainer->containers.end())
										entContainer = &itEnt->second;
									if(entContainer == nullptr)
										entContainer = &matContainer->containers.emplace(ent,EntityMeshInfo{ent}).first->second;
									entContainer->meshes.push_back(subMesh);
								}
							}
						}
					}
				}
			}
		}
	}
	if(glowMeshes.empty() == false)
		m_glowInfo.bGlowScheduled = true;
	if(bUpdateTranslucentMeshes == true)
	{
		// Sort translucent meshes by distance
		std::sort(translucentMeshes.begin(),translucentMeshes.end(),[](const std::unique_ptr<RenderSystem::TranslucentMesh> &a,const std::unique_ptr<RenderSystem::TranslucentMesh> &b) {
			return a->distance < b->distance;
		});
	}
}
