// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/console/helper.hpp"
//#include "shader_gaussianblur.h" // prosper TODO

module pragma.client;

import :game;
import :client_state;
import :debug;
import :engine;
import :entities.components;
import :gui;
import :model;
import :physics;
import :rendering.shaders;
import :scripting.lua;

static void CVAR_CALLBACK_render_vsync_enabled(pragma::NetworkState *, const pragma::console::ConVar &, int, int val) { pragma::platform::set_swap_interval((val == 0) ? 0 : 1); }
namespace {
	auto UVN = pragma::console::client::register_variable_listener<int>("render_vsync_enabled", &CVAR_CALLBACK_render_vsync_enabled);
}

static CallbackHandle cbDrawPhysics;
static CallbackHandle cbDrawPhysicsEnd;
static void CVAR_CALLBACK_debug_physics_draw(pragma::NetworkState *, const pragma::console::ConVar &, int, int val, bool serverside)
{
	if(cbDrawPhysics.IsValid())
		cbDrawPhysics.Remove();
	if(cbDrawPhysicsEnd.IsValid())
		cbDrawPhysicsEnd.Remove();
	//auto *physEnv = pragma::get_cgame()->GetPhysicsEnvironment();
	pragma::Game *game;
	if(serverside) {
		auto *nw = pragma::get_cengine()->GetServerNetworkState();
		game = nw ? nw->GetGameState() : nullptr;
	}
	else
		game = pragma::get_cgame();
	if(game == nullptr)
		return;
	auto *physEnv = game->GetPhysicsEnvironment();
	if(physEnv == nullptr)
		return;
	if(val == 0) {
		physEnv->SetVisualDebugger(nullptr);
		return;
	}
	auto visDebugger = std::make_unique<pragma::physics::CPhysVisualDebugger>();
	physEnv->SetVisualDebugger(std::move(visDebugger));
	/*if(val == 0)
	{
		visDebugger->SetDebugMode(pragma::physics::IVisualDebugger::DebugMode::None);
		return;
	}*/
	cbDrawPhysics = pragma::get_cgame()->AddCallback("Think", FunctionCallback<>::Create([serverside]() {
		pragma::Game *game;
		if(serverside) {
			auto *nw = pragma::get_cengine()->GetServerNetworkState();
			game = nw ? nw->GetGameState() : nullptr;
		}
		else
			game = pragma::get_cgame();
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
	cbDrawPhysicsEnd = pragma::get_cgame()->AddCallback("OnGameEnd", FunctionCallback<>::Create([]() {
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
namespace {
	auto UVN = pragma::console::client::register_variable_listener<int>("debug_physics_draw", +[](pragma::NetworkState *nw, const pragma::console::ConVar &cv, int oldVal, int val) { CVAR_CALLBACK_debug_physics_draw(nw, cv, oldVal, val, false); });
}
namespace {
	auto UVN = pragma::console::client::register_variable_listener<int>("sv_debug_physics_draw", +[](pragma::NetworkState *nw, const pragma::console::ConVar &cv, int oldVal, int val) { CVAR_CALLBACK_debug_physics_draw(nw, cv, oldVal, val, true); });
}

static void debug_render_validation_error_enabled(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(argv.empty()) {
		Con::CWAR << "No validation error id specified!" << Con::endl;
		return;
	}
	auto &id = argv.front();
	auto enabled = true;
	if(argv.size() > 1)
		enabled = pragma::util::to_boolean(argv[1]);
	pragma::get_cengine()->SetValidationErrorDisabled(id, !enabled);
}
namespace {
	auto UVN = pragma::console::client::register_command("debug_render_validation_error_enabled", &debug_render_validation_error_enabled, pragma::console::ConVarFlags::None, "Enables or disables the specified validation error.");
}

static void print_component_properties(const pragma::ComponentMemberInfo &memberInfo, pragma::BaseEntityComponent &component)
{
	auto name = static_cast<std::string>(memberInfo.GetName().str);
	Con::COUT << "Name: " << name << Con::endl;
	Con::COUT << "Specialization Type: " << magic_enum::enum_name(memberInfo.GetSpecializationType()) << Con::endl;
	auto *customSpecType = memberInfo.GetCustomSpecializationType();
	if(customSpecType)
		Con::COUT << "Custom Specialization Type: " << *customSpecType << Con::endl;

	auto min = memberInfo.GetMin();
	if(min)
		Con::COUT << "Min: " << *min << Con::endl;

	auto max = memberInfo.GetMin();
	if(max)
		Con::COUT << "Max: " << *max << Con::endl;

	auto stepSize = memberInfo.GetStepSize();
	if(stepSize)
		Con::COUT << "Step Size: " << *stepSize << Con::endl;

	auto &metaData = memberInfo.GetMetaData();
	if(metaData) {
		std::stringstream ss;
		metaData->ToAscii(udm::AsciiSaveFlags::Default | udm::AsciiSaveFlags::DontCompressLz4Arrays, ss, name);
		Con::COUT << "Meta Data:" << Con::endl;
		Con::COUT << ss.str() << Con::endl;
		Con::COUT << Con::endl;
	}

	Con::COUT << "Is Enum: " << (memberInfo.IsEnum() ? "Yes" : "No") << Con::endl;

	if(memberInfo.IsEnum()) {
		std::vector<int64_t> values;
		if(memberInfo.GetEnumValues(values)) {
			Con::COUT << "Enum Values:" << Con::endl;
			for(auto v : values)
				Con::COUT << v << "," << Con::endl;
		}
	}

	Con::COUT << "Flags: " << magic_enum::enum_flags_name(memberInfo.GetFlags()) << Con::endl;
	Con::COUT << "Type: " << magic_enum::enum_name(memberInfo.type) << Con::endl;
	Con::COUT << "User Index: " << memberInfo.userIndex << Con::endl;

	Con::COUT << "Default Value: ";
	pragma::ents::visit_member(memberInfo.type, [&memberInfo](auto tag) {
		using T = typename decltype(tag)::type;
		if(pragma::ents::is_udm_member_type(memberInfo.type)) {
			if constexpr(udm::is_convertible<T, udm::String>()) {
				T defaultVal;
				memberInfo.GetDefault<T>(defaultVal);
				auto strDefaultVal = udm::convert<T, udm::String>(defaultVal);
				Con::COUT << strDefaultVal;
				return;
			}
		}
		Con::COUT << "<Not convertible to string>";
	});
	Con::COUT << Con::endl;

	Con::COUT << "Value: ";
	pragma::ents::visit_member(memberInfo.type, [&memberInfo, &component](auto tag) {
		using T = typename decltype(tag)::type;
		if(pragma::ents::is_udm_member_type(memberInfo.type)) {
			if constexpr(udm::is_convertible<T, udm::String>()) {
				T value;
				memberInfo.getterFunction(memberInfo, component, &value);
				auto strVal = udm::convert<T, udm::String>(value);
				Con::COUT << strVal;
				return;
			}
		}
		Con::COUT << "<Not convertible to string>";
	});
	Con::COUT << Con::endl << Con::endl;
}
static void debug_dump_component_properties(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	auto &ent = pl->GetEntity();
	if(ent.IsCharacter() == false)
		return;
	auto charComponent = ent.GetCharacterComponent();
	if(charComponent.expired())
		return;
	auto ents = pragma::console::find_target_entity(state, *charComponent, argv);
	if(ents.empty())
		return;
	auto *entTgt = ents.front();
	Con::COUT << "Properties of target entity '" << *entTgt << "':" << Con::endl;
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
		Con::COUT << "Component '" << cName << "':" << Con::endl;
		uint32_t idx = 0;
		for(auto &memberInfo : memberInfos) {
			Con::COUT << "Index " << (idx++) << ":" << Con::endl;
			print_component_properties(*memberInfo, *c);
		}
		Con::COUT << Con::endl << Con::endl << Con::endl;
	};
}
namespace {
	auto UVN = pragma::console::client::register_command("debug_dump_component_properties", &debug_dump_component_properties, pragma::console::ConVarFlags::None, "Dumps entity component property values to the console.");
}

static void debug_render_depth_buffer(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	pragma::get_cengine()->GetRenderContext().WaitIdle();
	static std::unique_ptr<DebugGameGUI> dbg = nullptr;
	if(dbg) {
		dbg = nullptr;
		return;
	}
	auto &ent = pl->GetEntity();
	if(ent.IsCharacter() == false)
		return;
	auto charComponent = ent.GetCharacterComponent();
	auto ents = pragma::console::find_target_entity(state, *charComponent, argv);
	EntityHandle hEnt {};
	if(ents.empty() == false)
		hEnt = ents.front()->GetHandle();
	dbg = std::make_unique<DebugGameGUI>([hEnt]() {
		pragma::CSceneComponent *scene = nullptr;
		if(hEnt.valid()) {
			auto sceneC = hEnt.get()->GetComponent<pragma::CSceneComponent>();
			if(sceneC.expired()) {
				Con::CWAR << "Scene not found!" << Con::endl;
				return pragma::gui::WIHandle {};
			}
			scene = sceneC.get();
		}
		else
			scene = pragma::get_cgame()->GetScene<pragma::CSceneComponent>();
		auto *renderer = scene ? scene->GetRenderer<pragma::CRendererComponent>() : nullptr;
		auto raster = renderer ? renderer->GetEntity().GetComponent<pragma::CRasterizationRendererComponent>() : pragma::ComponentHandle<pragma::CRasterizationRendererComponent> {};
		if(raster.expired())
			return pragma::gui::WIHandle {};
		auto &wgui = pragma::gui::WGUI::GetInstance();

		auto r = wgui.Create<pragma::gui::types::WIDebugDepthTexture>();
		r->SetTexture(*raster->GetPrepass().textureDepth, {prosper::PipelineStageFlags::LateFragmentTestsBit, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::AccessFlags::DepthStencilAttachmentWriteBit},
		  {prosper::PipelineStageFlags::EarlyFragmentTestsBit, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::AccessFlags::DepthStencilAttachmentWriteBit});
		r->SetShouldResolveImage(true);
		r->SetSize(1024, 1024);
		r->Update();
		return r->GetHandle();
	});
	auto *d = dbg.get();
	dbg->AddCallback("PostRenderScene", FunctionCallback<void, std::reference_wrapper<const pragma::rendering::DrawSceneInfo>>::Create([d](std::reference_wrapper<const pragma::rendering::DrawSceneInfo> drawSceneInfo) {
		auto *el = d->GetGUIElement();
		if(el == nullptr)
			return;
		static_cast<pragma::gui::types::WIDebugDepthTexture *>(el)->Update();
	}));
}
namespace {
	auto UVN = pragma::console::client::register_command("debug_render_depth_buffer", &debug_render_depth_buffer, pragma::console::ConVarFlags::None, "Draws the scene depth buffer to screen.");
}

static auto cvDrawScene = pragma::console::get_client_con_var("render_draw_scene");
static auto cvDrawWorld = pragma::console::get_client_con_var("render_draw_world");
static auto cvDrawStatic = pragma::console::get_client_con_var("render_draw_static");
static auto cvDrawDynamic = pragma::console::get_client_con_var("render_draw_dynamic");
static auto cvDrawTranslucent = pragma::console::get_client_con_var("render_draw_translucent");
static auto cvClearScene = pragma::console::get_client_con_var("render_clear_scene");
static auto cvClearSceneColor = pragma::console::get_client_con_var("render_clear_scene_color");
static auto cvParticleQuality = pragma::console::get_client_con_var("cl_render_particle_quality");
void pragma::CGame::RenderScenes(rendering::DrawSceneInfo &drawSceneInfo)
{
#ifdef PRAGMA_ENABLE_SHADER_DEBUG_PRINT
	GetGlobalRenderSettingsBufferData().EvaluateDebugPrint();
#endif

	auto &inputDataManager = GetGlobalShaderInputDataManager();
	inputDataManager.UpdateBufferData(*drawSceneInfo.commandBuffer);

	CallCallbacks<void, std::reference_wrapper<const rendering::DrawSceneInfo>>("UpdateRenderBuffers", std::ref(drawSceneInfo));

	StartProfilingStage("RenderScenes");
	util::ScopeGuard sg {[this]() {
		StopProfilingStage(); // RenderScenes
	}};

	// Update particle systems
	// TODO: This isn't a good place for this and particle systems should
	// only be updated if visible (?)
	auto &cmd = *drawSceneInfo.commandBuffer;
	ecs::EntityIterator itParticles {*this};
	itParticles.AttachFilter<TEntityIteratorFilterComponent<ecs::CParticleSystemComponent>>();
	for(auto *ent : itParticles) {
		auto &tDelta = DeltaTime();
		auto pt = ent->GetComponent<ecs::CParticleSystemComponent>();
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
		auto *sceneC = GetRenderScene<CSceneComponent>();
		drawSceneInfo.scene = sceneC ? sceneC->GetHandle<CSceneComponent>() : pragma::ComponentHandle<CSceneComponent> {};
	}
	auto &scene = drawSceneInfo.scene;
	if(scene.expired()) {
		Con::CWAR << "No active render scene!" << Con::endl;
		return;
	}
	if(scene->IsValid() == false) {
		Con::CWAR << "Attempted to render invalid scene!" << Con::endl;
		return;
	}
	StartProfilingStage("PreRenderScenesCallbacks");
	CallCallbacks<void, std::reference_wrapper<const rendering::DrawSceneInfo>>("PreRenderScenes", std::ref(drawSceneInfo));
	CallLuaCallbacks<void, rendering::DrawSceneInfo *>("PreRenderScenes", &drawSceneInfo);
	StopProfilingStage(); // PreRenderScenesCallbacks

	StartProfilingStage("RenderScenesCallbacks");
	CallLuaCallbacks<void, rendering::DrawSceneInfo *>("RenderScenes", &drawSceneInfo);
	StopProfilingStage(); // RenderScenesCallbacks

	if(IsDefaultGameRenderEnabled()) {
		GetPrimaryCameraRenderMask(drawSceneInfo.inclusionMask, drawSceneInfo.exclusionMask);
		QueueForRendering(drawSceneInfo);
	}

	// This is the only callback that allows adding sub-passes
	StartProfilingStage("OnRenderScenesCallbacks");
	CallCallbacks("OnRenderScenes");
	CallLuaCallbacks<void>("OnRenderScenes");
	StopProfilingStage(); // OnRenderScenesCallbacks

	// Note: At this point no changes must be done to the scene whatsoever!
	// Any change in the scene will result in undefined behavior until this function
	// has completed execution!

	// We'll queue up building the render queues before we start rendering, so
	// most of it can be done in the background
	StartProfilingStage("RenderScenes");
	RenderScenes(m_sceneRenderQueue);
	StopProfilingStage(); // RenderScenes

	CallCallbacks("PostRenderScenes");
	CallLuaCallbacks("PostRenderScenes");

	m_sceneRenderQueue.clear();
}

void pragma::CGame::GetPrimaryCameraRenderMask(rendering::RenderMask &inclusionMask, rendering::RenderMask &exclusionMask) const
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

static void debug_dump_render_queues(const pragma::rendering::DrawSceneInfo &drawSceneInfo)
{
	std::stringstream ss;
	ss << "Scene Info:\n";
	ss << "Render Flags: " << magic_enum::enum_flags_name(drawSceneInfo.renderFlags) << "\n";
	if(drawSceneInfo.clearColor)
		ss << "Clear Color: " << *drawSceneInfo.clearColor << "\n";
	if(drawSceneInfo.toneMapping)
		ss << "Tone Mapping: " << magic_enum::enum_name(*drawSceneInfo.toneMapping) << "\n";
	if(drawSceneInfo.clipPlane)
		ss << "Clip Plane: " << *drawSceneInfo.clipPlane << "\n";
	if(drawSceneInfo.pvsOrigin)
		ss << "PVS Origin: " << *drawSceneInfo.pvsOrigin << "\n";
	ss << "Exclusion Mask: " << pragma::math::to_integral(drawSceneInfo.exclusionMask) << "\n";
	ss << "Inclusion Mask: " << pragma::math::to_integral(drawSceneInfo.inclusionMask) << "\n";
	if(drawSceneInfo.outputImage) {
		auto &img = *drawSceneInfo.outputImage;
		ss << "Output Image: " << pragma::math::to_integral(img.GetFormat()) << ", " << img.GetWidth() << "x" << img.GetHeight() << "\n";
	}
	ss << "Output Layer Id: " << drawSceneInfo.outputLayerId << "\n";
	ss << "Flags: " << magic_enum::enum_flags_name(drawSceneInfo.flags) << "\n";

	ss << "Render Queues:\n";
	auto fPrintQueue = [&ss](const pragma::rendering::RenderQueue &queue) {
		auto curMaterial = std::numeric_limits<pragma::material::MaterialIndex>::max();
		auto curPipeline = std::numeric_limits<prosper::PipelineID>::max();
		auto curEntity = std::numeric_limits<EntityIndex>::max();

		queue.WaitForCompletion();
		for(auto &item : queue.queue) {
			if(item.material != curMaterial) {
				curMaterial = item.material;
				auto *mat = item.GetMaterial();
				ss << pragma::console::get_true_color_code(colors::Lime) << "Material" << pragma::console::get_reset_color_code() << ": " << (mat ? mat->GetName() : "NULL") << "\n";
			}
			if(item.pipelineId != curPipeline) {
				curPipeline = item.pipelineId;
				uint32_t pipelineIdx;
				auto *shader = item.GetShader(pipelineIdx);
				ss << pragma::console::get_true_color_code(colors::Aqua) << "Shader" << pragma::console::get_reset_color_code() << ": ";
				if(shader)
					ss << shader->GetIdentifier() << " (" << pipelineIdx << ")\n";
				else
					ss << "NULL\n";
			}
			if(item.entity != curEntity) {
				curEntity = item.entity;
				auto *ent = item.GetEntity();
				ss << pragma::console::get_true_color_code(colors::Orange) << "Entity" << pragma::console::get_reset_color_code() << ": " << (ent ? ent->ToString() : "NULL") << "\n";
			}
			auto *mesh = item.GetMesh();
			ss << pragma::console::get_true_color_code(colors::White) << "Mesh" << pragma::console::get_reset_color_code() << ": ";
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

	auto n = pragma::math::to_integral(pragma::rendering::SceneRenderPass::Count);
	for(auto i = decltype(n) {0u}; i < n; ++i) {
		for(auto translucent : {false, true}) {
			auto *queue = renderDesc.GetRenderQueue(static_cast<pragma::rendering::SceneRenderPass>(i), translucent);
			if(!queue)
				continue;
			queue->WaitForCompletion();
			if(queue->queue.empty())
				continue;
			ss << "\n" << pragma::console::get_true_color_code(colors::Magenta) << "Scene pass" << pragma::console::get_reset_color_code() << ": " << magic_enum::enum_name(static_cast<pragma::rendering::SceneRenderPass>(i));
			if(translucent)
				ss << " (translucent)\n";
			else
				ss << " (opaque)\n";
			fPrintQueue(*queue);
		}
	}

	Con::COUT << ss.str() << Con::endl;
}
static void debug_dump_render_queues(const std::vector<pragma::rendering::DrawSceneInfo> &drawSceneInfos)
{
	Con::COUT << "Dumping render queues..." << Con::endl;
	uint32_t i = 0;
	for(auto &drawSceneInfo : drawSceneInfos) {
		Con::COUT << pragma::console::get_true_color_code(colors::Red) << "Scene #" << (i++) << pragma::console::get_reset_color_code() << Con::endl;
		debug_dump_render_queues(drawSceneInfo);
	}
}

bool g_dumpRenderQueues = false;
void pragma::CGame::RenderScenes(const std::vector<rendering::DrawSceneInfo> &drawSceneInfos)
{
	if(cvDrawScene->GetBool() == false)
		return;
	auto drawWorld = cvDrawWorld->GetInt();

	std::function<void(const std::vector<rendering::DrawSceneInfo> &)> buildRenderQueues = nullptr;
	buildRenderQueues = [&buildRenderQueues, drawWorld](const std::vector<rendering::DrawSceneInfo> &drawSceneInfos) {
		for(auto &cdrawSceneInfo : drawSceneInfos) {
			auto &drawSceneInfo = const_cast<rendering::DrawSceneInfo &>(cdrawSceneInfo);
			if(drawSceneInfo.scene.expired())
				continue;

			if(drawSceneInfo.subPasses) {
				// This scene has sub-scenes we have to consider first!
				buildRenderQueues(*drawSceneInfo.subPasses);
			}

			if(drawWorld == 2)
				drawSceneInfo.renderFlags &= ~(rendering::RenderFlags::Shadows);
			else if(drawWorld == 0)
				drawSceneInfo.renderFlags &= ~(rendering::RenderFlags::Shadows | rendering::RenderFlags::View | rendering::RenderFlags::World | rendering::RenderFlags::Skybox);

			if(cvDrawStatic->GetBool() == false)
				drawSceneInfo.renderFlags &= ~rendering::RenderFlags::Static;
			if(cvDrawDynamic->GetBool() == false)
				drawSceneInfo.renderFlags &= ~rendering::RenderFlags::Dynamic;
			if(cvDrawTranslucent->GetBool() == false)
				drawSceneInfo.renderFlags &= ~rendering::RenderFlags::Translucent;

			if(cvParticleQuality->GetInt() <= 0)
				drawSceneInfo.renderFlags &= ~rendering::RenderFlags::Particles;

			if(drawSceneInfo.commandBuffer == nullptr)
				drawSceneInfo.commandBuffer = get_cengine()->GetRenderContext().GetWindow().GetDrawCommandBuffer();
			// Modify render flags depending on console variables
			auto &renderFlags = drawSceneInfo.renderFlags;
			auto drawWorld = cvDrawWorld->GetBool();
			if(drawWorld == false)
				math::set_flag(renderFlags, rendering::RenderFlags::World, false);

			auto *pl = get_cgame()->GetLocalPlayer();
			if(pl == nullptr || pl->IsInFirstPersonMode() == false)
				math::set_flag(renderFlags, rendering::RenderFlags::View, false);

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
	std::function<void(const std::vector<rendering::DrawSceneInfo> &)> buildCommandBuffers = nullptr;
	buildCommandBuffers = [&buildCommandBuffers, drawWorld](const std::vector<rendering::DrawSceneInfo> &drawSceneInfos) {
		for(auto &cdrawSceneInfo : drawSceneInfos) {
			auto &drawSceneInfo = const_cast<rendering::DrawSceneInfo &>(cdrawSceneInfo);
			if(drawSceneInfo.scene.expired() || math::is_flag_set(drawSceneInfo.flags, rendering::DrawSceneInfo::Flags::DisableRender))
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
	std::function<void(const std::vector<rendering::DrawSceneInfo> &)> renderScenes = nullptr;
	renderScenes = [this, &renderScenes, drawWorld](const std::vector<rendering::DrawSceneInfo> &drawSceneInfos) {
		for(auto &cdrawSceneInfo : drawSceneInfos) {
			auto &drawSceneInfo = const_cast<rendering::DrawSceneInfo &>(cdrawSceneInfo);
			if(drawSceneInfo.scene.expired() || math::is_flag_set(drawSceneInfo.flags, rendering::DrawSceneInfo::Flags::DisableRender))
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
				auto &hdrImg = scene->GetRenderer<CRendererComponent>()->GetSceneTexture()->GetImage();
				drawCmd->RecordImageBarrier(hdrImg, prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::TransferDstOptimal);
				drawCmd->RecordClearImage(hdrImg, prosper::ImageLayout::TransferDstOptimal, {{clearCol.r, clearCol.g, clearCol.b, clearCol.a}});
				drawCmd->RecordImageBarrier(hdrImg, prosper::ImageLayout::TransferDstOptimal, prosper::ImageLayout::ColorAttachmentOptimal);
			}

			// Update Exposure
			auto *renderer = scene->GetRenderer<CRendererComponent>();
			auto raster = renderer ? renderer->GetEntity().GetComponent<CRasterizationRendererComponent>() : pragma::ComponentHandle<CRasterizationRendererComponent> {};
			if(raster.valid()) {
				//pragma::get_cengine()->StartGPUTimer(GPUTimerEvent::UpdateExposure); // prosper TODO
				auto frame = get_cengine()->GetRenderContext().GetLastFrameId();
				if(frame > 0)
					raster->GetHDRInfo().UpdateExposure();
				//pragma::get_cengine()->StopGPUTimer(GPUTimerEvent::UpdateExposure); // prosper TODO
			}
			// TODO
#if 0
			auto ret = false;
			m_bMainRenderPass = false;

			auto bSkipScene = CallCallbacksWithOptionalReturn<
				bool,std::reference_wrapper<const pragma::rendering::DrawSceneInfo>
			>("DrawScene",ret,std::ref(drawSceneInfo)) == CallbackReturnType::HasReturnValue;
			m_bMainRenderPass = true;
			if(bSkipScene == true && ret == true)
				return;
			m_bMainRenderPass = false;
			if(CallLuaCallbacks<
				bool,std::reference_wrapper<const pragma::rendering::DrawSceneInfo>
			>("DrawScene",&bSkipScene,std::ref(drawSceneInfo)) == CallbackReturnType::HasReturnValue && bSkipScene == true)
			{
				CallCallbacks("PostRenderScenes");
				m_bMainRenderPass = true;
				return;
			}
			else
				m_bMainRenderPass = true;
#endif
			StartProfilingStage("RenderScene");
			RenderScene(drawSceneInfo);
			StopProfilingStage(); // RenderScene
			if(newRecording)
				static_cast<prosper::IPrimaryCommandBuffer *>(drawCmd.get())->StopRecording();
		}
	};
	renderScenes(drawSceneInfos);

	StartProfilingStage("FlushRenderQueueBuilder");
	m_renderQueueBuilder->Flush();
	StopProfilingStage(); // FlushRenderQueueBuilder

	// At this point all render threads (render queue and command buffer builders) are guaranteed
	// to have completed their work for this frame. The scene is now safe for writing again.
}

bool pragma::CGame::IsInMainRenderPass() const { return m_bMainRenderPass; }
