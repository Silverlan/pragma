#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/c_engine.h"
#include "pragma/model/c_side.h"
#include <mathutil/uquat.h>
#include "pragma/debug/c_debugoverlay.h"
#include "pragma/rendering/shaders/c_shader.h"
#include "pragma/rendering/c_rendermode.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/brush/c_brushmesh.h"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/rendering/lighting/c_light.h"
#include <wgui/wgui.h>
#include "pragma/rendering/scene/camera.h"
#include "pragma/rendering/uniformbinding.h"
#include "pragma/console/c_cvar.h"
#include "pragma/entities/components/c_vehicle_component.hpp"
//#include "shader_gaussianblur.h" // prosper TODO
#include "pragma/rendering/world_environment.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_glow.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_hdr.hpp"
#include "pragma/rendering/shaders/image/c_shader_additive.h"
#include "pragma/rendering/shaders/debug/c_shader_debug.h"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fog.hpp"
#include "pragma/rendering/shaders/world/c_shader_flat.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_hdr.hpp"
#ifdef PHYS_ENGINE_BULLET
#include "pragma/physics/c_physdebug.h"
#elif PHYS_ENGINE_PHYSX
#include "pragma/physics/pxvisualizer.h"
#endif
#include "pragma/rendering/shaders/particles/c_shader_particle.hpp"
#include "pragma/rendering/rendersystem.h"
#include <pragma/lua/luacallback.h>
#include "pragma/rendering/scene/scene.h"
#include "pragma/opengl/renderhierarchy.h"
#include <pragma/performancetimer.h>
#include "pragma/rendering/lighting/c_light_ranged.h"
#include "pragma/rendering/lighting/c_light_spot.h"
#include "pragma/rendering/lighting/c_light_point.h"
#include "luasystem.h"
#include "pragma/rendering/shaders/post_processing/c_shader_postprocessing.h"
#include "pragma/gui/widebugdepthtexture.h"
#include "pragma/debug/c_debug_game_gui.h"
#include <pragma/lua/luafunction_call.h>
#include "pragma/entities/environment/lights/c_env_light_spot.h"
#include "pragma/rendering/lighting/shadows/c_shadowmap.h"
#include "pragma/rendering/lighting/shadows/c_shadowmapcasc.h"
#include "pragma/lua/libraries/c_lua_vulkan.h"
#include "pragma/rendering/shaders/particles/c_shader_particle_polyboard.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_ssao.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_ssao_blur.hpp"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/rendering/shaders/c_shader_forwardp_light_culling.hpp"
#include <image/prosper_msaa_texture.hpp>
#include <image/prosper_render_target.hpp>
#include <prosper_util.hpp>
#include <prosper_command_buffer.hpp>
#include <textureinfo.h>
#include <pragma/util/profiling_stages.h>
#include <sharedutils/scope_guard.h>
#include <pragma/rendering/c_sci_gpu_timer_manager.hpp>
#include <sharedutils/scope_guard.h>
#include <pragma/entities/entity_iterator.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;
#pragma optimize("",off)
static void CVAR_CALLBACK_render_vsync_enabled(NetworkState*,ConVar*,int,int val)
{
	glfwSwapInterval((val == 0) ? 0 : 1);
}
REGISTER_CONVAR_CALLBACK_CL(render_vsync_enabled,CVAR_CALLBACK_render_vsync_enabled);

#include <pragma/physics/physenvironment.h>
static CallbackHandle cbDrawPhysics;
static CallbackHandle cbDrawPhysicsEnd;
static void CVAR_CALLBACK_debug_physics_draw(NetworkState*,ConVar*,int,int val)
{
	if(cbDrawPhysics.IsValid())
		cbDrawPhysics.Remove();
	if(cbDrawPhysicsEnd.IsValid())
		cbDrawPhysicsEnd.Remove();
	if(c_game == NULL)
		return;
	PhysEnv *physEnv = c_game->GetPhysicsEnvironment();
	if(physEnv == NULL)
		return;
	WVBtIDebugDraw *debugDraw = c_game->GetPhysicsDebugInterface();
	if(debugDraw == NULL)
		return;
	if(val == 0)
	{
		debugDraw->setDebugMode(btIDebugDraw::DBG_NoDebug);
		return;
	}
	cbDrawPhysics = c_game->AddCallback("Think",FunctionCallback<>::Create([]() {
		auto *debugDraw = c_game->GetPhysicsDebugInterface();
		auto &vehicles = pragma::CVehicleComponent::GetAll();
		for(auto it=vehicles.begin();it!=vehicles.end();++it)
		{
			auto *vhc = *it;
			auto *btVhc = vhc->GetBtVehicle();
			if(btVhc != nullptr)
			{
				for(UChar i=0;i<vhc->GetWheelCount();++i)
				{
					btVhc->updateWheelTransform(i,true);
					auto *info = vhc->GetWheelInfo(i);
					if(info != nullptr)
					{
						auto &t = info->m_worldTransform;
						//debugDraw->drawTransform(t,info->m_wheelsRadius);
						//debugDraw->drawCylinder(info->m_wheelsRadius,info->m_wheelsRadius *0.5f,0,t,btVector3(1.f,0.f,0.f));
					}
				}
			}
		}
	}));
	cbDrawPhysicsEnd = c_game->AddCallback("OnGameEnd",FunctionCallback<>::Create([]() {
		cbDrawPhysics.Remove();
		cbDrawPhysicsEnd.Remove();
	}));
	auto mode = btIDebugDraw::DBG_DrawAabb |
		btIDebugDraw::DBG_DrawConstraintLimits |
		btIDebugDraw::DBG_DrawConstraints |
		btIDebugDraw::DBG_DrawContactPoints |
		btIDebugDraw::DBG_DrawFeaturesText |
		btIDebugDraw::DBG_DrawFrames |
		btIDebugDraw::DBG_DrawNormals |
		btIDebugDraw::DBG_DrawText |
		btIDebugDraw::DBG_DrawWireframe |
		btIDebugDraw::DBG_EnableCCD;
	if(val == 2)
		mode = btIDebugDraw::DBG_DrawWireframe;
	else if(val == 3)
		mode = btIDebugDraw::DBG_DrawConstraints;
	else if(val == 4)
		mode = btIDebugDraw::DBG_DrawNormals;
	debugDraw->setDebugMode(mode);
}
REGISTER_CONVAR_CALLBACK_CL(debug_physics_draw,CVAR_CALLBACK_debug_physics_draw);

#ifdef PHYS_ENGINE_PHYSX
static void GetPhysXDebugColor(physx::PxU32 eCol,float *col)
{
	switch(eCol)
	{
	case physx::PxDebugColor::Enum::eARGB_BLACK:
		{
			col[0] = 0.f;
			col[1] = 0.f;
			col[2] = 0.f;
			break;
		}
	case physx::PxDebugColor::Enum::eARGB_RED:
		{
			col[0] = 1.f;
			col[1] = 0.f;
			col[2] = 0.f;
			break;
		}
	case physx::PxDebugColor::Enum::eARGB_GREEN:
		{
			col[0] = 0.f;
			col[1] = 1.f;
			col[2] = 0.f;
			break;
		}
	case physx::PxDebugColor::Enum::eARGB_BLUE:
		{
			col[0] = 0.f;
			col[1] = 0.f;
			col[2] = 1.f;
			break;
		}
	case physx::PxDebugColor::Enum::eARGB_YELLOW:
		{
			col[0] = 1.f;
			col[1] = 1.f;
			col[2] = 0.f;
			break;
		}
	case physx::PxDebugColor::Enum::eARGB_MAGENTA:
		{
			col[0] = 1.f;
			col[1] = 0.f;
			col[2] = 1.f;
			break;
		}
	case physx::PxDebugColor::Enum::eARGB_CYAN:
		{
			col[0] = 0.f;
			col[1] = 1.f;
			col[2] = 1.f;
			break;
		}
	case physx::PxDebugColor::Enum::eARGB_GREY:
		{
			col[0] = 0.5f;
			col[1] = 0.5f;
			col[2] = 0.5f;
			break;
		}
	case physx::PxDebugColor::Enum::eARGB_DARKRED:
		{
			col[0] = 0.345f;
			col[1] = 0.f;
			col[2] = 0.f;
			break;
		}
	case physx::PxDebugColor::Enum::eARGB_DARKGREEN:
		{
			col[0] = 0.f;
			col[1] = 0.345f;
			col[2] = 0.f;
			break;
		}
	case physx::PxDebugColor::Enum::eARGB_DARKBLUE:
		{
			col[0] = 0.f;
			col[1] = 0.f;
			col[2] = 0.345f;
			break;
		}
	case physx::PxDebugColor::Enum::eARGB_WHITE:
	default:
		{
			col[0] = 1.f;
			col[1] = 1.f;
			col[2] = 1.f;
			break;
		}
	};
}
#endif

void CGame::RenderGlowMeshes(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,const Scene &scene,RenderMode renderMode)
{
	auto &glowInfo = const_cast<Scene&>(scene).GetGlowInfo();
	auto *shader = static_cast<pragma::ShaderGlow*>(glowInfo.shader.get());
	if(shader == nullptr)
		return;
	auto *renderInfo = scene.GetRenderInfo(renderMode);
	if(renderInfo == nullptr)
		return;
	if(shader->BeginDraw(drawCmd) == true)
	{
		shader->BindSceneCamera(scene,(renderMode == RenderMode::View) ? true : false);
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

void CGame::SetFrameDepthBufferSamplingRequired() {m_bFrameDepthBufferSamplingRequired = true;}

static CVar cvRenderPhysics = GetClientConVar("debug_physics_draw");
static CVar cvDrawParticles = GetClientConVar("render_draw_particles");
static CVar cvDrawGlow = GetClientConVar("render_draw_glow");
static CVar cvDrawTranslucent = GetClientConVar("render_draw_translucent");
void CGame::Render(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,float interpolation,FRender renderFlags)
{
#ifdef ENABLE_PERFORMANCE_TIMER
	static unsigned int renderTimer = PerformanceTimer::InitializeTimer("CGame::Render");
	PerformanceTimer::StartMeasurement(renderTimer);
#endif
	if(m_renderScene == nullptr)
		return;
	
	// TODO: Transform positions:
	// pos += velocity *interpolation;
	// Re-Implement interpolation
	auto &cam = *m_renderScene->camera.get();
	auto &scene = m_renderScene;
	//bool bWorld = (renderFlags &FRENDER_WORLD) == FRENDER_WORLD;
	bool bShadows = (renderFlags &FRender::Shadows) == FRender::Shadows;
	//Scene *scene = GetRenderScene();
	//std::vector<CHCMeshInfo> &renderMeshes = scene->chc->PerformCulling();
	auto &renderMeshes = scene->GetCulledMeshes();
/*	if(bShadows == true)
	{
		auto &lights = OcclusionCulling::PerformCulling(GetLights());
		RenderSystem::RenderShadows(lights,cam,renderMeshes);
		OpenGL::BindVertexArray(0);
	}*/

	//ScopeGuard sgDepthImg {};
	auto &culledParticles = scene->GetCulledParticles();
	auto bShouldDrawParticles = (renderFlags &FRender::Particles) == FRender::Particles && cvDrawParticles->GetBool() == true && culledParticles.empty() == false;
	if(bShouldDrawParticles == true)
		SetFrameDepthBufferSamplingRequired();
	if(m_bFrameDepthBufferSamplingRequired == true)
	{
		auto &prepass = scene->GetPrepass();
		auto &dstDepthTex = *prepass.textureDepthSampled;
		auto &ptrDstDepthImg = dstDepthTex.GetImage();
		auto &dstDepthImg = *ptrDstDepthImg;

		auto &hdrInfo = scene->GetHDRInfo();
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
		**drawCmd,*scene->GetForwardPlusInstance().GetTileVisLightIndexBuffer(),
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
		**drawCmd,*scene->GetCameraBuffer(),
		Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT | Anvil::PipelineStageFlagBits::VERTEX_SHADER_BIT | Anvil::PipelineStageFlagBits::GEOMETRY_SHADER_BIT,
		Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
	);

	// Render settings buffer barrier
	prosper::util::record_buffer_barrier(
		**drawCmd,*scene->GetRenderSettingsBuffer(),
		Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT | Anvil::PipelineStageFlagBits::VERTEX_SHADER_BIT | Anvil::PipelineStageFlagBits::GEOMETRY_SHADER_BIT,
		Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
	);

	if(scene->BeginRenderPass(drawCmd) == false)
		return;

	auto bGlow = cvDrawGlow->GetBool();
	auto bTranslucent = cvDrawTranslucent->GetBool();
	auto bReflection = (renderFlags &FRender::Reflection) != FRender::None;
	//c_engine->StartGPUTimer(GPUTimerEvent::Skybox); // prosper TODO
	if((renderFlags &FRender::Skybox) != FRender::None)
	{
		CallCallbacks("PreRenderSkybox");
		RenderSystem::Render(drawCmd,RenderMode::Skybox,bReflection);
		CallCallbacks("PostRenderSkybox");
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
		CallCallbacks("PreRenderWorld");

		//c_engine->StartGPUTimer(GPUTimerEvent::World); // prosper TODO
			RenderSystem::Render(drawCmd,RenderMode::World,bReflection);
		//c_engine->StopGPUTimer(GPUTimerEvent::World); // prosper TODO

		//c_engine->StartGPUTimer(GPUTimerEvent::WorldTranslucent); // prosper TODO
		auto *renderInfo = scene->GetRenderInfo(RenderMode::World);
		if(renderInfo != nullptr)
			RenderSystem::Render(drawCmd,cam,RenderMode::World,bReflection,renderInfo->translucentMeshes);
		//c_engine->StopGPUTimer(GPUTimerEvent::WorldTranslucent); // prosper TODO

		CallCallbacks("PostRenderWorld");
	}

	// Start particle sub-pass
	//prosper::util::record_next_sub_pass(**drawCmd);

	//if((renderFlags &FRENDER_WORLD) == FRENDER_WORLD)
	//	RenderWorldEntities(interpolation,entsRender);
	//c_engine->StartGPUTimer(GPUTimerEvent::Particles); // prosper TODO
	if(bShouldDrawParticles)
	{
		CallCallbacks("PreRenderParticles");

		auto &glowInfo = m_scene->GetGlowInfo();

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

		CallCallbacks("PostRenderParticles");
	}
	//c_engine->StopGPUTimer(GPUTimerEvent::Particles); // prosper TODO
	
	CallCallbacks<void>("Render");
	CallLuaCallbacks("Render");

	//c_engine->StartGPUTimer(GPUTimerEvent::Debug); // prosper TODO
	if((renderFlags &FRender::Debug) == FRender::Debug)
	{
		CallCallbacks("PreRenderDebug");

		DebugRenderer::Render(drawCmd,cam);
		if(cvRenderPhysics->GetBool())
		{
#ifdef PHYS_ENGINE_BULLET
			m_btDebugDraw->Render(drawCmd,cam);
#elif PHYS_ENGINE_PHYSX
			const physx::PxRenderBuffer &pxRenderBuffer = GetPhysXScene()->getRenderBuffer();
			PxVisualizer::RenderScene(pxRenderBuffer);
#endif
		}
		CallCallbacks("PostRenderDebug");
	}
	//c_engine->StopGPUTimer(GPUTimerEvent::Debug); // prosper TODO

	if((renderFlags &FRender::Water) != FRender::None)
	{
		CallCallbacks("PreRenderWater");

		//c_engine->StartGPUTimer(GPUTimerEvent::Water); // prosper TODO
			auto numShaderInvocations = RenderSystem::Render(drawCmd,RenderMode::Water,bReflection);
		//c_engine->StopGPUTimer(GPUTimerEvent::Water); // prosper TODO

		CallCallbacks("PostRenderWater");

		if(numShaderInvocations > 0)
		{
			//auto &prepass = scene->GetPrepass(); // prosper TODO
			// Note: The texture buffer was probably already resolved at this point (e.g. by particle render pass), however
			// we will need the water depth values for the upcoming render passes as well, so we'll have to resolve again.
			// To do that, we'll reset the MSAA texture, so at the next resolve-call it will update accordingly.
			//prepass.textureDepth->Reset(); // prosper TODO
			//prepass.textureDepth->Resolve(); // prosper TODO
		}
	}

	if((renderFlags &FRender::View) != FRender::None)
	{
		auto *pl = GetLocalPlayer();
		if(pl != nullptr && pl->IsInFirstPersonMode())
		{
			CallCallbacks("PreRenderView");

			//c_engine->StartGPUTimer(GPUTimerEvent::View); // prosper TODO
				RenderSystem::Render(drawCmd,RenderMode::View,bReflection);
			//c_engine->StopGPUTimer(GPUTimerEvent::View); // prosper TODO

			//c_engine->StartGPUTimer(GPUTimerEvent::ViewParticles); // prosper TODO
			if((renderFlags &FRender::Particles) == FRender::Particles)
			{
				auto &culledParticles = scene->GetCulledParticles();
				auto &glowInfo = m_scene->GetGlowInfo();
				RenderParticleSystems(drawCmd,culledParticles,interpolation,RenderMode::View,false,&glowInfo.tmpBloomParticles);
				if(bGlow == false)
					glowInfo.tmpBloomParticles.clear();
				if(!glowInfo.tmpBloomParticles.empty())
					glowInfo.bGlowScheduled = true;
			}
			//c_engine->StartGPUTimer(GPUTimerEvent::ViewParticles); // prosper TODO
			//RenderGlowMeshes(cam,true);

			CallCallbacks("PostRenderView");
		}
	}
	scene->EndRenderPass(drawCmd);
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
#ifdef ENABLE_PERFORMANCE_TIMER
	PerformanceTimer::EndMeasurement(renderTimer);
#endif
}
WVBtIDebugDraw *CGame::GetPhysicsDebugInterface() {return m_btDebugDraw;}

static CVar cvCulling = GetClientConVar("cl_render_occlusion_culling");
void CGame::GetOccludedEntities(std::vector<CBaseEntity*> &entsOccluded)
{
	 // prosper TODO
#if 0
	auto &scene = GetRenderScene();
	auto &cam = scene->camera;
	auto &posCam = cam->GetPos();
	bool bCull = (cvCulling->GetInt() != 0) ? true : false;
	//bool bFogEnabled = FogController::IsFogEnabled();
	float fogDist = 0.f;
	//if(bFogEnabled == true)
	//	fogDist = FogController::GetFarDistance();

	//std::vector<Plane> &planes = cam->GetFrustumPlanes();
	for(int i=0;i<m_ents.size();i++)
	{
		CBaseEntity *ent = m_ents[i];
		if(ent != NULL && ent->IsSpawned() && ent->GetRenderMode() == RenderMode::World && ent->ShouldDraw(posCam))//(ent->ShouldDraw() || ent->ShouldDrawShadow()))
		{
			Sphere &sp = ent->GetRenderSphereBounds();
			if(sp.radius > 0.f)
			{
				auto &pos = ent->GetPosition();
				auto posSphere = pos +sp.pos;
				if(bCull == false || /*bFogEnabled == false ||*/ glm::distance(posCam,posSphere) < (fogDist +sp.radius))
					entsOccluded.push_back(ent);
			}
		}
	}
#endif
}

void CGame::GetRenderEntities(std::vector<CBaseEntity*> &entsRender)
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

void CGame::RenderParticleSystems(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,std::vector<pragma::CParticleSystemComponent*> &particles,float interpolation,RenderMode renderMode,Bool bloom,std::vector<pragma::CParticleSystemComponent*> *bloomParticles)
{
	auto &scene = GetRenderScene();
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
				scene->EndRenderPass(drawCmd);

				auto &hdrInfo = scene->GetHDRInfo();
				auto &prepass = scene->GetPrepass();
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
				scene->BeginRenderPass(drawCmd,hdrInfo.rpIntermediate.get());
			}
			//scene->ResolveDepthTexture(drawCmd); // Particles aren't multisampled, but requires scene depth buffer
			particle->Render(drawCmd,*scene,bloom);
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

static void CVAR_CALLBACK_debug_render_depth_buffer(NetworkState*,ConVar*,bool,bool val)
{
	static std::unique_ptr<DebugGameGUI> dbg = nullptr;
	if(dbg == nullptr)
	{
		if(val == false)
			return;
		dbg = std::make_unique<DebugGameGUI>([]() {
			auto &scene = c_game->GetScene();
			auto &wgui = WGUI::GetInstance();

			auto r = wgui.Create<WIDebugDepthTexture>();
			r->SetTexture(*scene->GetPrepass().textureDepth,{
				Anvil::PipelineStageFlagBits::LATE_FRAGMENT_TESTS_BIT,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,Anvil::AccessFlagBits::DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
			},{
				Anvil::PipelineStageFlagBits::EARLY_FRAGMENT_TESTS_BIT,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,Anvil::AccessFlagBits::DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
			});
			r->SetShouldResolveImage(true);
			r->SetSize(1024,1024);
			r->Update();
			return r->GetHandle();
		});
		auto *d = dbg.get();
		dbg->AddCallback("PostRenderScene",FunctionCallback<>::Create([d]() {
			auto *el = d->GetGUIElement();
			if(el == nullptr)
				return;
			static_cast<WIDebugDepthTexture*>(el)->Update();
		}));
		return;
	}
	else if(val == true)
		return;
	dbg = nullptr;
}
REGISTER_CONVAR_CALLBACK_CL(debug_render_depth_buffer,CVAR_CALLBACK_debug_render_depth_buffer);

static CVar cvDrawScene = GetClientConVar("render_draw_scene");
static CVar cvDrawWorld = GetClientConVar("render_draw_world");
static CVar cvClearScene = GetClientConVar("render_clear_scene");
static CVar cvClearSceneColor = GetClientConVar("render_clear_scene_color");
static CVar cvParticleQuality = GetClientConVar("cl_render_particle_quality");
void CGame::RenderScenes(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,std::shared_ptr<prosper::RenderTarget> &rt,FRender renderFlags,const Color *clearColor)//const Vulkan::RenderPass &renderPass,const Vulkan::Framebuffer &framebuffer,const Vulkan::CommandBuffer &drawCmd,FRender renderFlags,const Color *clearColor)
{
	if(cvDrawScene->GetBool() == false)
		return;
	//auto &context = const_cast<Vulkan::Context&>(c_engine->GetRenderContext()); // prosper TODO
	//context.SwapFrameIndex(); // prosper TODO

	// We have to free all shadow map depth buffers because
	// they might have to be re-assigned for this frame.
	// This only has to be done once per frame, not per scene!
	auto &shadowMaps = ShadowMap::GetAll();
	auto numShadowMaps = shadowMaps.Size();
	for(auto i=decltype(numShadowMaps){0};i<numShadowMaps;++i)
	{
		auto *shadowMap = shadowMaps[i];
		if(shadowMap == nullptr)
			continue;
		shadowMap->FreeRenderTarget();
	}

	auto drawWorld = cvDrawWorld->GetInt();
	if(drawWorld == 2)
		renderFlags &= ~(FRender::Shadows | FRender::Glow);
	else if(drawWorld == 0)
		renderFlags &= ~(FRender::Shadows | FRender::Glow | FRender::View | FRender::World | FRender::Skybox);

	if(cvParticleQuality->GetInt() <= 0)
		renderFlags &= ~FRender::Particles;

	// Update particle systems
	EntityIterator itParticles {*this};
	itParticles.AttachFilter<TEntityIteratorFilterComponent<pragma::CParticleSystemComponent>>();
	for(auto *ent : itParticles)
	{
		auto &tDelta = DeltaTime();
		auto pt = ent->GetComponent<pragma::CParticleSystemComponent>();
		if(pt.valid())
			pt->Simulate(tDelta);
	}

	auto &scene = GetRenderScene();
	if(scene == nullptr)
	{
		Con::cwar<<"WARNING: No active render scene!"<<Con::endl;
		return;
	}
	if(scene->IsValid() == false)
	{
		Con::cwar<<"WARNING: Attempted to render invalid scene!"<<Con::endl;
		return;
	}
	auto &hdrInfo = scene->GetHDRInfo();
	/*auto &rt = scene->GetRenderTarget();
	auto &img = hdrInfo.texture->GetImage();
	auto w = img->GetWidth();
	auto h = img->GetHeight();
	drawCmd->SetViewport(w,h); // TODO: Why do we need to reset these?
	drawCmd->SetScissor(w,h);*/ // prosper TODO

	if(cvClearScene->GetBool() == true || drawWorld == 2 || clearColor != nullptr)
	{
		auto clearCol = (clearColor != nullptr) ? clearColor->ToVector4() : Color(cvClearSceneColor->GetString()).ToVector4();
		auto &hdrImg = hdrInfo.hdrRenderTarget->GetTexture()->GetImage();
		prosper::util::record_image_barrier(*(*drawCmd),*(*hdrImg),Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL);
		prosper::util::record_clear_image(*(*drawCmd),*(*hdrImg),Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,{{clearCol.r,clearCol.g,clearCol.b,clearCol.a}});
		prosper::util::record_image_barrier(*(*drawCmd),*(*hdrImg),Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
	}

	// Update Exposure
	//c_engine->StartGPUTimer(GPUTimerEvent::UpdateExposure); // prosper TODO
	auto frame = c_engine->GetLastFrameId();
	if(frame > 0)
		hdrInfo.UpdateExposure(*rt->GetTexture());
	//c_engine->StopGPUTimer(GPUTimerEvent::UpdateExposure); // prosper TODO

	// Update time
	UpdateShaderTimeData();

	CallCallbacks("PreRenderScenes");

	static auto bSkipCallbacks = false;
	if(bSkipCallbacks == false)
	{
		bSkipCallbacks = true;
		ScopeGuard guard([]() {bSkipCallbacks = false;});
		auto ret = false;
		m_bMainRenderPass = false;


		auto bSkipScene = CallCallbacksWithOptionalReturn<
			bool,std::reference_wrapper<std::shared_ptr<prosper::PrimaryCommandBuffer>>,std::reference_wrapper<std::shared_ptr<prosper::RenderTarget>>
		>("DrawScene",ret,std::ref(drawCmd),std::ref(rt)) == CallbackReturnType::HasReturnValue;
		m_bMainRenderPass = true;
		if(bSkipScene == true && ret == true)
			return;
		m_bMainRenderPass = false;
		if(CallLuaCallbacks<
			bool,std::reference_wrapper<std::shared_ptr<prosper::PrimaryCommandBuffer>>,std::reference_wrapper<std::shared_ptr<prosper::RenderTarget>>
		>("DrawScene",&bSkipScene,std::ref(drawCmd),std::ref(rt)) == CallbackReturnType::HasReturnValue && bSkipScene == true)
		{
			CallCallbacks("PostRenderScenes");
			m_bMainRenderPass = true;
			return;
		}
		else
			m_bMainRenderPass = true;
	}
	RenderScene(drawCmd,rt,renderFlags);
	CallCallbacks("PostRenderScenes");
	CallLuaCallbacks("PostRenderScenes");
}

bool CGame::IsInMainRenderPass() const {return m_bMainRenderPass;}
#pragma optimize("",on)
