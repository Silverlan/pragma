/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/c_engine.h"
#include "pragma/model/c_side.h"
#include <mathutil/uquat.h>
#include "pragma/debug/c_debugoverlay.h"
#include "pragma/rendering/c_rendermode.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/brush/c_brushmesh.h"
#include "pragma/entities/components/c_player_component.hpp"
#include <wgui/wgui.h>
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/console/c_cvar.h"
#include "pragma/console/c_cvar_global_functions.h"
#include "pragma/entities/components/c_vehicle_component.hpp"
//#include "shader_gaussianblur.h" // prosper TODO
#include "pragma/rendering/world_environment.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_hdr.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fog.hpp"
#include "pragma/rendering/shaders/world/c_shader_flat.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_hdr.hpp"
#include "pragma/rendering/shaders/particles/c_shader_particle.hpp"
#include "pragma/rendering/rendersystem.h"
#include "pragma/rendering/render_queue.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include <pragma/lua/luacallback.h>
#include "pragma/entities/components/c_scene_component.hpp"
#include "luasystem.h"
#include "pragma/gui/widebugdepthtexture.h"
#include "pragma/debug/c_debug_game_gui.h"
#include <pragma/lua/luafunction_call.h>
#include "pragma/entities/environment/lights/c_env_light_spot.h"
#include "pragma/lua/libraries/c_lua_vulkan.h"
#include "pragma/rendering/shaders/particles/c_shader_particle_polyboard.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_ssao.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_ssao_blur.hpp"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/rendering/shaders/c_shader_forwardp_light_culling.hpp"
#include "pragma/physics/c_phys_visual_debugger.hpp"
#include <pragma/console/sh_cmd.h>
#include <pragma/entities/entity_component_system_t.hpp>
#include <image/prosper_msaa_texture.hpp>
#include <image/prosper_render_target.hpp>
#include <prosper_util.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_window.hpp>
#include <textureinfo.h>
#include <sharedutils/scope_guard.h>
#include <pragma/rendering/c_sci_gpu_timer_manager.hpp>
#include <sharedutils/scope_guard.h>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/physics/visual_debugger.hpp>
#include <pragma/physics/environment.hpp>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

static void CVAR_CALLBACK_render_vsync_enabled(NetworkState *, ConVar *, int, int val) { glfwSwapInterval((val == 0) ? 0 : 1); }
REGISTER_CONVAR_CALLBACK_CL(render_vsync_enabled, CVAR_CALLBACK_render_vsync_enabled);

static CallbackHandle cbDrawPhysics;
static CallbackHandle cbDrawPhysicsEnd;
static void CVAR_CALLBACK_debug_physics_draw(NetworkState *, ConVar *, int, int val, bool serverside)
{
	if(cbDrawPhysics.IsValid())
		cbDrawPhysics.Remove();
	if(cbDrawPhysicsEnd.IsValid())
		cbDrawPhysicsEnd.Remove();
	//auto *physEnv = c_game->GetPhysicsEnvironment();
	Game *game;
	if(serverside) {
		auto *nw = c_engine->GetServerNetworkState();
		game = nw ? nw->GetGameState() : nullptr;
	}
	else
		game = c_game;
	if(game == nullptr)
		return;
	auto *physEnv = game->GetPhysicsEnvironment();
	if(physEnv == nullptr)
		return;
	if(val == 0) {
		physEnv->SetVisualDebugger(nullptr);
		return;
	}
	auto visDebugger = std::make_unique<CPhysVisualDebugger>();
	physEnv->SetVisualDebugger(std::move(visDebugger));
	/*if(val == 0)
	{
		visDebugger->SetDebugMode(pragma::physics::IVisualDebugger::DebugMode::None);
		return;
	}*/
	cbDrawPhysics = c_game->AddCallback("Think", FunctionCallback<>::Create([serverside]() {
		Game *game;
		if(serverside) {
			auto *nw = c_engine->GetServerNetworkState();
			game = nw ? nw->GetGameState() : nullptr;
		}
		else
			game = c_game;
		if(game == nullptr)
			return;
		auto *physEnv = game->GetPhysicsEnvironment();
		auto *debugDraw = physEnv ? physEnv->GetVisualDebugger() : nullptr;
		if(debugDraw == nullptr)
			return;
		auto &vehicles = pragma::CVehicleComponent::GetAll();
		for(auto it = vehicles.begin(); it != vehicles.end(); ++it) {
			auto *vhc = *it;
#ifdef ENABLE_DEPRECATED_PHYSICS
			auto *btVhc = vhc->GetBtVehicle();
			if(btVhc != nullptr) {
				for(UChar i = 0; i < vhc->GetWheelCount(); ++i) {
					btVhc->updateWheelTransform(i, true);
					auto *info = vhc->GetWheelInfo(i);
					if(info != nullptr) {
						auto &t = info->m_worldTransform;
						//debugDraw->drawTransform(t,info->m_wheelsRadius);
						//debugDraw->drawCylinder(info->m_wheelsRadius,info->m_wheelsRadius *0.5f,0,t,btVector3(1.f,0.f,0.f));
					}
				}
			}
#endif
		}
	}));
	cbDrawPhysicsEnd = c_game->AddCallback("OnGameEnd", FunctionCallback<>::Create([]() {
		cbDrawPhysics.Remove();
		cbDrawPhysicsEnd.Remove();
	}));
	auto mode = pragma::physics::IVisualDebugger::DebugMode::All;
	if(val == 2)
		mode = pragma::physics::IVisualDebugger::DebugMode::Wireframe;
	else if(val == 3)
		mode = pragma::physics::IVisualDebugger::DebugMode::Constraints;
	else if(val == 4)
		mode = pragma::physics::IVisualDebugger::DebugMode::Normals;
	visDebugger->SetDebugMode(mode);
}
REGISTER_CONVAR_CALLBACK_CL(debug_physics_draw, [](NetworkState *nw, ConVar *cv, int oldVal, int val) { CVAR_CALLBACK_debug_physics_draw(nw, cv, oldVal, val, false); });
REGISTER_CONVAR_CALLBACK_CL(sv_debug_physics_draw, [](NetworkState *nw, ConVar *cv, int oldVal, int val) { CVAR_CALLBACK_debug_physics_draw(nw, cv, oldVal, val, true); });

void Console::commands::debug_render_validation_error_enabled(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(argv.empty()) {
		Con::cwar << "No validation error id specified!" << Con::endl;
		return;
	}
	auto &id = argv.front();
	auto enabled = true;
	if(argv.size() > 1)
		enabled = util::to_boolean(argv[1]);
	c_engine->SetValidationErrorDisabled(id, !enabled);
}

void Console::commands::debug_render_depth_buffer(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	static std::unique_ptr<DebugGameGUI> dbg = nullptr;
	if(dbg) {
		dbg = nullptr;
		return;
	}
	auto &ent = pl->GetEntity();
	if(ent.IsCharacter() == false)
		return;
	auto charComponent = ent.GetCharacterComponent();
	auto ents = command::find_target_entity(state, *charComponent, argv);
	EntityHandle hEnt {};
	if(ents.empty() == false)
		hEnt = ents.front()->GetHandle();
	dbg = std::make_unique<DebugGameGUI>([hEnt]() {
		pragma::CSceneComponent *scene = nullptr;
		if(hEnt.valid()) {
			auto sceneC = hEnt.get()->GetComponent<pragma::CSceneComponent>();
			if(sceneC.expired()) {
				Con::cwar << "Scene not found!" << Con::endl;
				return WIHandle {};
			}
			scene = sceneC.get();
		}
		else
			scene = c_game->GetScene();
		auto *renderer = scene ? scene->GetRenderer() : nullptr;
		auto raster = renderer ? renderer->GetEntity().GetComponent<pragma::CRasterizationRendererComponent>() : pragma::ComponentHandle<pragma::CRasterizationRendererComponent> {};
		if(raster.expired())
			return WIHandle {};
		auto &wgui = WGUI::GetInstance();

		auto r = wgui.Create<WIDebugDepthTexture>();
		r->SetTexture(*raster->GetPrepass().textureDepth, {prosper::PipelineStageFlags::LateFragmentTestsBit, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::AccessFlags::DepthStencilAttachmentWriteBit},
		  {prosper::PipelineStageFlags::EarlyFragmentTestsBit, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::AccessFlags::DepthStencilAttachmentWriteBit});
		r->SetShouldResolveImage(true);
		r->SetSize(1024, 1024);
		r->Update();
		return r->GetHandle();
	});
	auto *d = dbg.get();
	dbg->AddCallback("PostRenderScene", FunctionCallback<void, std::reference_wrapper<const util::DrawSceneInfo>>::Create([d](std::reference_wrapper<const util::DrawSceneInfo> drawSceneInfo) {
		auto *el = d->GetGUIElement();
		if(el == nullptr)
			return;
		static_cast<WIDebugDepthTexture *>(el)->Update();
	}));
}

static CVar cvDrawScene = GetClientConVar("render_draw_scene");
static CVar cvDrawWorld = GetClientConVar("render_draw_world");
static CVar cvDrawStatic = GetClientConVar("render_draw_static");
static CVar cvDrawDynamic = GetClientConVar("render_draw_dynamic");
static CVar cvDrawTranslucent = GetClientConVar("render_draw_translucent");
static CVar cvClearScene = GetClientConVar("render_clear_scene");
static CVar cvClearSceneColor = GetClientConVar("render_clear_scene_color");
static CVar cvParticleQuality = GetClientConVar("cl_render_particle_quality");
void CGame::RenderScenes(util::DrawSceneInfo &drawSceneInfo)
{
	// Update particle systems
	// TODO: This isn't a good place for this and particle systems should
	// only be updated if visible (?)
	auto &cmd = *drawSceneInfo.commandBuffer;
	EntityIterator itParticles {*this};
	itParticles.AttachFilter<TEntityIteratorFilterComponent<pragma::CParticleSystemComponent>>();
	for(auto *ent : itParticles) {
		auto &tDelta = DeltaTime();
		auto pt = ent->GetComponent<pragma::CParticleSystemComponent>();
		if(pt.valid() && pt->GetParent() == nullptr && pt->ShouldAutoSimulate()) {
			pt->Simulate(tDelta);

			auto &renderers = pt->GetRenderers();
			if(!renderers.empty()) {
				auto &renderer = renderers.front();
				renderer->PreRender(cmd);
			}

			// Vertex buffer barrier
			auto &ptBuffer = pt->GetParticleBuffer();
			if(ptBuffer != nullptr) {
				// Particle buffer barrier
				cmd.RecordBufferBarrier(*ptBuffer, prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::VertexInputBit, prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::VertexAttributeReadBit);
			}

			auto &animBuffer = pt->GetParticleAnimationBuffer();
			if(animBuffer != nullptr) {
				// Animation start buffer barrier
				cmd.RecordBufferBarrier(*animBuffer, prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::VertexInputBit, prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::VertexAttributeReadBit);
			}

			auto &spriteSheetBuffer = pt->GetSpriteSheetBuffer();
			if(spriteSheetBuffer != nullptr) {
				// Animation buffer barrier
				cmd.RecordBufferBarrier(*spriteSheetBuffer, prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit, prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::ShaderReadBit);
			}
		}
	}

	if(drawSceneInfo.scene.expired()) {
		auto *sceneC = GetRenderScene();
		drawSceneInfo.scene = sceneC ? sceneC->GetHandle<pragma::CSceneComponent>() : pragma::ComponentHandle<pragma::CSceneComponent> {};
	}
	auto &scene = drawSceneInfo.scene;
	if(scene.expired()) {
		Con::cwar << "No active render scene!" << Con::endl;
		return;
	}
	if(scene->IsValid() == false) {
		Con::cwar << "Attempted to render invalid scene!" << Con::endl;
		return;
	}
	CallCallbacks<void, std::reference_wrapper<const util::DrawSceneInfo>>("PreRenderScenes", std::ref(drawSceneInfo));
	CallLuaCallbacks<void, util::DrawSceneInfo *>("PreRenderScenes", &drawSceneInfo);
	CallLuaCallbacks<void, util::DrawSceneInfo *>("RenderScenes", &drawSceneInfo);

	if(IsDefaultGameRenderEnabled()) {
		GetPrimaryCameraRenderMask(drawSceneInfo.inclusionMask, drawSceneInfo.exclusionMask);
		QueueForRendering(drawSceneInfo);
	}

	// This is the only callback that allows adding sub-passes
	CallCallbacks("OnRenderScenes");
	CallLuaCallbacks<void>("OnRenderScenes");

	// Note: At this point no changes must be done to the scene whatsoever!
	// Any change in the scene will result in undefined behavior until this function
	// has completed execution!

	// We'll queue up building the render queues before we start rendering, so
	// most of it can be done in the background
	RenderScenes(m_sceneRenderQueue);

	CallCallbacks("PostRenderScenes");
	CallLuaCallbacks("PostRenderScenes");

	m_sceneRenderQueue.clear();
}

void CGame::GetPrimaryCameraRenderMask(::pragma::rendering::RenderMask &inclusionMask, ::pragma::rendering::RenderMask &exclusionMask) const
{
	auto *lp = m_plLocal.get();
	if(lp && lp->IsInFirstPersonMode()) {
		exclusionMask |= m_thirdPersonRenderMask;
		inclusionMask |= m_firstPersonRenderMask;

		exclusionMask &= ~m_firstPersonRenderMask;
		inclusionMask &= ~m_thirdPersonRenderMask;
	}
	else {
		exclusionMask |= m_firstPersonRenderMask;
		inclusionMask |= m_thirdPersonRenderMask;

		exclusionMask &= ~m_thirdPersonRenderMask;
		inclusionMask &= ~m_firstPersonRenderMask;
	}
}

void CGame::RenderScenes(const std::vector<util::DrawSceneInfo> &drawSceneInfos)
{
	if(cvDrawScene->GetBool() == false)
		return;
	auto drawWorld = cvDrawWorld->GetInt();

	std::function<void(const std::vector<util::DrawSceneInfo> &)> buildRenderQueues = nullptr;
	buildRenderQueues = [&buildRenderQueues, drawWorld](const std::vector<util::DrawSceneInfo> &drawSceneInfos) {
		for(auto &cdrawSceneInfo : drawSceneInfos) {
			auto &drawSceneInfo = const_cast<util::DrawSceneInfo &>(cdrawSceneInfo);
			if(drawSceneInfo.scene.expired())
				continue;

			if(drawSceneInfo.subPasses) {
				// This scene has sub-scenes we have to consider first!
				buildRenderQueues(*drawSceneInfo.subPasses);
			}

			if(drawWorld == 2)
				drawSceneInfo.renderFlags &= ~(RenderFlags::Shadows | RenderFlags::Glow);
			else if(drawWorld == 0)
				drawSceneInfo.renderFlags &= ~(RenderFlags::Shadows | RenderFlags::Glow | RenderFlags::View | RenderFlags::World | RenderFlags::Skybox);

			if(cvDrawStatic->GetBool() == false)
				drawSceneInfo.renderFlags &= ~RenderFlags::Static;
			if(cvDrawDynamic->GetBool() == false)
				drawSceneInfo.renderFlags &= ~RenderFlags::Dynamic;
			if(cvDrawTranslucent->GetBool() == false)
				drawSceneInfo.renderFlags &= ~RenderFlags::Translucent;

			if(cvParticleQuality->GetInt() <= 0)
				drawSceneInfo.renderFlags &= ~RenderFlags::Particles;

			if(drawSceneInfo.commandBuffer == nullptr)
				drawSceneInfo.commandBuffer = c_engine->GetRenderContext().GetWindow().GetDrawCommandBuffer();
			// Modify render flags depending on console variables
			auto &renderFlags = drawSceneInfo.renderFlags;
			auto drawWorld = cvDrawWorld->GetBool();
			if(drawWorld == false)
				umath::set_flag(renderFlags, RenderFlags::World, false);

			auto *pl = c_game->GetLocalPlayer();
			if(pl == nullptr || pl->IsInFirstPersonMode() == false)
				umath::set_flag(renderFlags, RenderFlags::View, false);

			drawSceneInfo.scene->BuildRenderQueues(drawSceneInfo);
		}
	};
	buildRenderQueues(drawSceneInfos);

	// Inform the render queue builder that no further render queues will be queued for building.
	// It will call the render queue completion functions automatically once all render queues have
	// finished building. No completion functions can be executed before this function is called.
	GetRenderQueueBuilder().SetReadyForCompletion();

	// Update time
	UpdateShaderTimeData();

	// Initiate the command buffer build threads for all queued scenes.
	// If we have multiple scenes to render (e.g. left eye and right eye for VR),
	// the command buffers can all be built in parallel.
	std::function<void(const std::vector<util::DrawSceneInfo> &)> buildCommandBuffers = nullptr;
	buildCommandBuffers = [&buildCommandBuffers, drawWorld](const std::vector<util::DrawSceneInfo> &drawSceneInfos) {
		for(auto &cdrawSceneInfo : drawSceneInfos) {
			auto &drawSceneInfo = const_cast<util::DrawSceneInfo &>(cdrawSceneInfo);
			if(drawSceneInfo.scene.expired() || umath::is_flag_set(drawSceneInfo.flags, util::DrawSceneInfo::Flags::DisableRender))
				continue;
			if(drawSceneInfo.subPasses) {
				// This scene has sub-scenes we have to consider first!
				buildCommandBuffers(*drawSceneInfo.subPasses);
			}
			drawSceneInfo.scene->RecordRenderCommandBuffers(drawSceneInfo);
		}
	};
	buildCommandBuffers(drawSceneInfos);

	// Render the scenes
	std::function<void(const std::vector<util::DrawSceneInfo> &)> renderScenes = nullptr;
	renderScenes = [this, &renderScenes, drawWorld](const std::vector<util::DrawSceneInfo> &drawSceneInfos) {
		for(auto &cdrawSceneInfo : drawSceneInfos) {
			auto &drawSceneInfo = const_cast<util::DrawSceneInfo &>(cdrawSceneInfo);
			if(drawSceneInfo.scene.expired() || umath::is_flag_set(drawSceneInfo.flags, util::DrawSceneInfo::Flags::DisableRender))
				continue;

			if(drawSceneInfo.subPasses) {
				// This scene has sub-scenes we have to consider first!
				renderScenes(*drawSceneInfo.subPasses);
			}

			auto &scene = drawSceneInfo.scene;
			auto &drawCmd = drawSceneInfo.commandBuffer;
			if(drawCmd == nullptr || drawCmd->IsPrimary() == false)
				continue;
			auto *primCmd = static_cast<prosper::IPrimaryCommandBuffer *>(drawCmd.get());
			auto newRecording = !primCmd->IsRecording();
			if(newRecording)
				static_cast<prosper::IPrimaryCommandBuffer *>(drawCmd.get())->StartRecording();
			if(cvClearScene->GetBool() == true || drawWorld == 2 || drawSceneInfo.clearColor.has_value()) {
				auto clearCol = drawSceneInfo.clearColor.has_value() ? drawSceneInfo.clearColor->ToVector4() : Color(cvClearSceneColor->GetString()).ToVector4();
				auto &hdrImg = scene->GetRenderer()->GetSceneTexture()->GetImage();
				drawCmd->RecordImageBarrier(hdrImg, prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::TransferDstOptimal);
				drawCmd->RecordClearImage(hdrImg, prosper::ImageLayout::TransferDstOptimal, {{clearCol.r, clearCol.g, clearCol.b, clearCol.a}});
				drawCmd->RecordImageBarrier(hdrImg, prosper::ImageLayout::TransferDstOptimal, prosper::ImageLayout::ColorAttachmentOptimal);
			}

			// Update Exposure
			auto *renderer = scene->GetRenderer();
			auto raster = renderer ? renderer->GetEntity().GetComponent<pragma::CRasterizationRendererComponent>() : pragma::ComponentHandle<pragma::CRasterizationRendererComponent> {};
			if(raster.valid()) {
				//c_engine->StartGPUTimer(GPUTimerEvent::UpdateExposure); // prosper TODO
				auto frame = c_engine->GetRenderContext().GetLastFrameId();
				if(frame > 0)
					raster->GetHDRInfo().UpdateExposure();
				//c_engine->StopGPUTimer(GPUTimerEvent::UpdateExposure); // prosper TODO
			}
			// TODO
#if 0
			auto ret = false;
			m_bMainRenderPass = false;

			auto bSkipScene = CallCallbacksWithOptionalReturn<
				bool,std::reference_wrapper<const util::DrawSceneInfo>
			>("DrawScene",ret,std::ref(drawSceneInfo)) == CallbackReturnType::HasReturnValue;
			m_bMainRenderPass = true;
			if(bSkipScene == true && ret == true)
				return;
			m_bMainRenderPass = false;
			if(CallLuaCallbacks<
				bool,std::reference_wrapper<const util::DrawSceneInfo>
			>("DrawScene",&bSkipScene,std::ref(drawSceneInfo)) == CallbackReturnType::HasReturnValue && bSkipScene == true)
			{
				CallCallbacks("PostRenderScenes");
				m_bMainRenderPass = true;
				return;
			}
			else
				m_bMainRenderPass = true;
#endif
			RenderScene(drawSceneInfo);
			if(newRecording)
				static_cast<prosper::IPrimaryCommandBuffer *>(drawCmd.get())->StopRecording();
		}
	};
	renderScenes(drawSceneInfos);
	m_renderQueueBuilder->Flush();

	// At this point all render threads (render queue and command buffer builders) are guaranteed
	// to have completed their work for this frame. The scene is now safe for writing again.
}

bool CGame::IsInMainRenderPass() const { return m_bMainRenderPass; }
