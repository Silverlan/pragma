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
#include "pragma/entities/baseentity.h"
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
#include <pragma/lua/classes/entity_components.hpp>
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
#include <pragma/entities/attribute_specialization_type.hpp>
#include <pragma/entities/components/component_member_flags.hpp>
#include <pragma/entities/entity_component_manager_t.hpp>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

static void CVAR_CALLBACK_render_vsync_enabled(NetworkState *, const ConVar &, int, int val) { glfwSwapInterval((val == 0) ? 0 : 1); }
REGISTER_CONVAR_CALLBACK_CL(render_vsync_enabled, CVAR_CALLBACK_render_vsync_enabled);

static CallbackHandle cbDrawPhysics;
static CallbackHandle cbDrawPhysicsEnd;
static void CVAR_CALLBACK_debug_physics_draw(NetworkState *, const ConVar &, int, int val, bool serverside)
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
REGISTER_CONVAR_CALLBACK_CL(debug_physics_draw, [](NetworkState *nw, const ConVar &cv, int oldVal, int val) { CVAR_CALLBACK_debug_physics_draw(nw, cv, oldVal, val, false); });
REGISTER_CONVAR_CALLBACK_CL(sv_debug_physics_draw, [](NetworkState *nw, const ConVar &cv, int oldVal, int val) { CVAR_CALLBACK_debug_physics_draw(nw, cv, oldVal, val, true); });

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

static void print_component_properties(const pragma::ComponentMemberInfo &memberInfo, pragma::BaseEntityComponent &component)
{
	auto name = static_cast<std::string>(memberInfo.GetName().str);
	Con::cout << "Name: " << name << Con::endl;
	Con::cout << "Specialization Type: " << magic_enum::enum_name(memberInfo.GetSpecializationType()) << Con::endl;
	auto *customSpecType = memberInfo.GetCustomSpecializationType();
	if(customSpecType)
		Con::cout << "Custom Specialization Type: " << *customSpecType << Con::endl;

	auto min = memberInfo.GetMin();
	if(min)
		Con::cout << "Min: " << *min << Con::endl;

	auto max = memberInfo.GetMin();
	if(max)
		Con::cout << "Max: " << *max << Con::endl;

	auto stepSize = memberInfo.GetStepSize();
	if(stepSize)
		Con::cout << "Step Size: " << *stepSize << Con::endl;

	auto &metaData = memberInfo.GetMetaData();
	if(metaData) {
		std::stringstream ss;
		metaData->ToAscii(udm::AsciiSaveFlags::Default | udm::AsciiSaveFlags::DontCompressLz4Arrays, ss, name);
		Con::cout << "Meta Data:" << Con::endl;
		Con::cout << ss.str() << Con::endl;
		Con::cout << Con::endl;
	}

	Con::cout << "Is Enum: " << (memberInfo.IsEnum() ? "Yes" : "No") << Con::endl;

	if(memberInfo.IsEnum()) {
		std::vector<int64_t> values;
		if(memberInfo.GetEnumValues(values)) {
			Con::cout << "Enum Values:" << Con::endl;
			for(auto v : values)
				Con::cout << v << "," << Con::endl;
		}
	}

	Con::cout << "Flags: " << magic_enum::flags::enum_name(memberInfo.GetFlags()) << Con::endl;
	Con::cout << "Type: " << magic_enum::enum_name(memberInfo.type) << Con::endl;
	Con::cout << "User Index: " << memberInfo.userIndex << Con::endl;

	Con::cout << "Default Value: ";
	pragma::ents::visit_member(memberInfo.type, [&memberInfo](auto tag) {
		using T = typename decltype(tag)::type;
		if(pragma::ents::is_udm_member_type(memberInfo.type)) {
			if constexpr(udm::is_convertible<T, udm::String>()) {
				T defaultVal;
				memberInfo.GetDefault<T>(defaultVal);
				auto strDefaultVal = udm::convert<T, udm::String>(defaultVal);
				Con::cout << strDefaultVal;
				return;
			}
		}
		Con::cout << "<Not convertible to string>";
	});
	Con::cout << Con::endl;

	Con::cout << "Value: ";
	pragma::ents::visit_member(memberInfo.type, [&memberInfo, &component](auto tag) {
		using T = typename decltype(tag)::type;
		if(pragma::ents::is_udm_member_type(memberInfo.type)) {
			if constexpr(udm::is_convertible<T, udm::String>()) {
				T value;
				memberInfo.getterFunction(memberInfo, component, &value);
				auto strVal = udm::convert<T, udm::String>(value);
				Con::cout << strVal;
				return;
			}
		}
		Con::cout << "<Not convertible to string>";
	});
	Con::cout << Con::endl << Con::endl;
}
void Console::commands::debug_dump_component_properties(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	auto &ent = pl->GetEntity();
	if(ent.IsCharacter() == false)
		return;
	auto charComponent = ent.GetCharacterComponent();
	if(charComponent.expired())
		return;
	auto ents = command::find_target_entity(state, *charComponent, argv);
	if(ents.empty())
		return;
	auto *entTgt = ents.front();
	Con::cout << "Properties of target entity '";
	entTgt->print(Con::cout);
	Con::cout << "':" << Con::endl;
	for(auto &c : entTgt->GetComponents()) {
		std::vector<const pragma::ComponentMemberInfo *> memberInfos;
		uint32_t i = 0;
		auto *info = c->GetMemberInfo(i++);
		while(info != nullptr) {
			memberInfos.push_back(info);
			info = c->GetMemberInfo(i++);
		}
		if(memberInfos.empty())
			continue;
		auto *cInfo = c->GetComponentInfo();
		std::string cName = "Unknown";
		if(cInfo)
			cName = cInfo->name.str;
		Con::cout << "Component '" << cName << "':" << Con::endl;
		uint32_t idx = 0;
		for(auto &memberInfo : memberInfos) {
			Con::cout << "Index " << (idx++) << ":" << Con::endl;
			print_component_properties(*memberInfo, *c);
		}
		Con::cout << Con::endl << Con::endl << Con::endl;
	};
}

void Console::commands::debug_render_depth_buffer(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	c_engine->GetRenderContext().WaitIdle();
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

static void debug_dump_render_queues(const util::DrawSceneInfo &drawSceneInfo)
{
	std::stringstream ss;
	ss << "Scene Info:\n";
	ss << "Render Flags: " << magic_enum::flags::enum_name(drawSceneInfo.renderFlags) << "\n";
	if(drawSceneInfo.clearColor)
		ss << "Clear Color: " << *drawSceneInfo.clearColor << "\n";
	if(drawSceneInfo.toneMapping)
		ss << "Tone Mapping: " << magic_enum::enum_name(*drawSceneInfo.toneMapping) << "\n";
	if(drawSceneInfo.clipPlane)
		ss << "Clip Plane: " << *drawSceneInfo.clipPlane << "\n";
	if(drawSceneInfo.pvsOrigin)
		ss << "PVS Origin: " << *drawSceneInfo.pvsOrigin << "\n";
	ss << "Exclusion Mask: " << umath::to_integral(drawSceneInfo.exclusionMask) << "\n";
	ss << "Inclusion Mask: " << umath::to_integral(drawSceneInfo.inclusionMask) << "\n";
	if(drawSceneInfo.outputImage) {
		auto &img = *drawSceneInfo.outputImage;
		ss << "Output Image: " << umath::to_integral(img.GetFormat()) << ", " << img.GetWidth() << "x" << img.GetHeight() << "\n";
	}
	ss << "Output Layer Id: " << drawSceneInfo.outputLayerId << "\n";
	ss << "Flags: " << magic_enum::flags::enum_name(drawSceneInfo.flags) << "\n";

	ss << "Render Queues:\n";
	auto fPrintQueue = [&ss](const pragma::rendering::RenderQueue &queue) {
		auto curMaterial = std::numeric_limits<MaterialIndex>::max();
		auto curPipeline = std::numeric_limits<prosper::PipelineID>::max();
		auto curEntity = std::numeric_limits<EntityIndex>::max();

		queue.WaitForCompletion();
		for(auto &item : queue.queue) {
			if(item.material != curMaterial) {
				curMaterial = item.material;
				auto *mat = item.GetMaterial();
				ss << util::get_true_color_code(Color::Lime) << "Material" << util::get_reset_color_code() << ": " << (mat ? mat->GetName() : "NULL") << "\n";
			}
			if(item.pipelineId != curPipeline) {
				curPipeline = item.pipelineId;
				uint32_t pipelineIdx;
				auto *shader = item.GetShader(pipelineIdx);
				ss << util::get_true_color_code(Color::Aqua) << "Shader" << util::get_reset_color_code() << ": ";
				if(shader)
					ss << shader->GetIdentifier() << " (" << pipelineIdx << ")\n";
				else
					ss << "NULL\n";
			}
			if(item.entity != curEntity) {
				curEntity = item.entity;
				auto *ent = item.GetEntity();
				ss << util::get_true_color_code(Color::Orange) << "Entity" << util::get_reset_color_code() << ": " << (ent ? ent->ToString() : "NULL") << "\n";
			}
			auto *mesh = item.GetMesh();
			ss << util::get_true_color_code(Color::White) << "Mesh" << util::get_reset_color_code() << ": ";
			if(!mesh)
				ss << "NULL\n";
			else
				ss << *mesh << "\n";
		}
	};

	auto &renderDesc = drawSceneInfo.scene->GetSceneRenderDesc();
	auto &worldRenderQueues = renderDesc.GetWorldRenderQueues();
	for(auto &queue : worldRenderQueues) {
		ss << "\nWorld Render Queue...\n";
		fPrintQueue(*queue);
	}

	auto n = umath::to_integral(pragma::rendering::SceneRenderPass::Count);
	for(auto i = decltype(n) {0u}; i < n; ++i) {
		for(auto translucent : {false, true}) {
			auto *queue = renderDesc.GetRenderQueue(static_cast<pragma::rendering::SceneRenderPass>(i), translucent);
			if(!queue)
				continue;
			queue->WaitForCompletion();
			if(queue->queue.empty())
				continue;
			ss << "\n" << util::get_true_color_code(Color::Magenta) << "Scene pass" << util::get_reset_color_code() << ": " << magic_enum::enum_name(static_cast<pragma::rendering::SceneRenderPass>(i));
			if(translucent)
				ss << " (translucent)\n";
			else
				ss << " (opaque)\n";
			fPrintQueue(*queue);
		}
	}

	Con::cout << ss.str() << Con::endl;
}
static void debug_dump_render_queues(const std::vector<util::DrawSceneInfo> &drawSceneInfos)
{
	Con::cout << "Dumping render queues..." << Con::endl;
	uint32_t i = 0;
	for(auto &drawSceneInfo : drawSceneInfos) {
		Con::cout << util::get_true_color_code(Color::Red) << "Scene #" << (i++) << util::get_reset_color_code() << Con::endl;
		debug_dump_render_queues(drawSceneInfo);
	}
}

bool g_dumpRenderQueues = false;
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

	if(g_dumpRenderQueues && !drawSceneInfos.empty()) {
		g_dumpRenderQueues = false;
		debug_dump_render_queues(drawSceneInfos);
	}

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
