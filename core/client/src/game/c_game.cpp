/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/lua/converters/gui_element_converter_t.hpp"
#include "pragma/input/input_binding_layer.hpp"
#include "pragma/model/c_polymesh.h"
#include "pragma/model/brush/c_brushmesh.h"
#include "pragma/entities/c_world.h"
#include "pragma/c_engine.h"
#include "pragma/debug/c_debugoverlay.h"
#include <wgui/wgui.h>
#include <wgui/wibase.h>
#include "pragma/gui/wiluabase.h"
#include "luasystem.h"
#include <wgui/wihandle.h>
#include "pragma/lua/classes/c_lwibase.h"
#include "cmaterialmanager.h"
#include <cmaterial_manager2.hpp>
//#include "shader_screen.h" // prosper TODO
#include "pragma/lua/classes/c_ldef_wgui.h"
#include "pragma/entities/c_listener.h"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_bsp_leaf_component.hpp"
#include "pragma/entities/components/c_toggle_component.hpp"
#include "pragma/entities/components/c_light_map_receiver_component.hpp"
#include "pragma/entities/components/c_light_map_component.hpp"
#include "pragma/entities/components/c_scene_component.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/entities/components/c_gamemode_component.hpp"
#include "pragma/entities/components/c_game_component.hpp"
#include "pragma/entities/components/c_observer_component.hpp"
#include "pragma/entities/components/c_observable_component.hpp"
#include "pragma/entities/game/c_game_occlusion_culler.hpp"
#include "pragma/entities/util/c_util_pbr_converter.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/level/mapgeometry.h"
#include "pragma/model/c_modelmanager.h"
#include "pragma/gui/wiluahandlewrapper.h"
#include "pragma/entities/c_viewmodel.h"
#include "pragma/entities/c_viewbody.h"
#include "pragma/entities/c_player.hpp"
#include <pragma/physics/physobj.h>
#include <pragma/util/util_game.hpp>
#include "pragma/console/c_cvar.h"
#include "pragma/rendering/c_rendermode.h"
#include "pragma/rendering/shaders/post_processing/c_shader_hdr.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/renderers/raytracing_renderer.hpp"
#include "pragma/rendering/render_queue_worker.hpp"
#include "pragma/ai/c_navsystem.h"
#include <texturemanager/texturemanager.h>
#include <pragma/physics/environment.hpp>
#include "pragma/rendering/rendersystem.h"
#include "pragma/rendering/render_queue.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include <pragma/lua/luacallback.h>
#include "pragma/rendering/occlusion_culling/chc.hpp"
#include "pragma/rendering/c_msaa.h"
#include "pragma/gui/wgui_luainterface.h"
#include "textureinfo.h"
#include "pragma/networking/iclient.hpp"
#include <pragma/networking/nwm_util.h>
#include "pragma/debug/renderdebuginfo.hpp"
#include "pragma/game/c_game_callback.h"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree_impl.hpp"
#include "pragma/rendering/world_environment.hpp"
#include "pragma/audio/c_sound_efx.hpp"
#include <pragma/lua/luafunction_call.h>
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/shaders/c_shader_lua.hpp"
#include "pragma/lua/classes/c_lparticle_modifiers.hpp"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/entities/components/c_weapon_component.hpp"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/entities/components/c_character_component.hpp"
#include "pragma/entities/components/c_vehicle_component.hpp"
#include "pragma/entities/components/c_ai_component.hpp"
#include "pragma/entities/components/c_physics_component.hpp"
#include "pragma/entities/environment/c_env_reflection_probe.hpp"
#include "pragma/entities/game/c_game_shadow_manager.hpp"
#include "pragma/rendering/c_settings.hpp"
#include "pragma/physics/c_phys_visual_debugger.hpp"
#include <pragma/entities/baseplayer.hpp>
#include <pragma/util/giblet_create_info.hpp>
#include <alsound_effect.hpp>
#include <alsoundsystem.hpp>
#include <prosper_command_buffer.hpp>
#include <pragma/networking/enums.hpp>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/components/base_physics_component.hpp>
#include <pragma/entities/components/velocity_component.hpp>
#include <prosper_util.hpp>
#include <image/prosper_sampler.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/components/map_component.hpp>
#include <pragma/networking/snapshot_flags.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/entities/components/action_input_controller_component.hpp>
#include <pragma/entities/components/action_input_controller_component.hpp>
#include <pragma/rendering/c_sci_gpu_timer_manager.hpp>
#include <pragma/level/level_info.hpp>
#include <pragma/lua/util.hpp>
#include <pragma/asset_types/world.hpp>
#include <sharedutils/util_library.hpp>
#include <shader/prosper_pipeline_loader.hpp>
#include <util_image.hpp>
#include <util_image_buffer.hpp>
#include <udm.hpp>
#include <prosper_window.hpp>

extern EntityClassMap<CBaseEntity> *g_ClientEntityFactories;
extern ClientEntityNetworkMap *g_ClEntityNetworkMap;
extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;
DLLCLIENT CGame *c_game = NULL;
DLLCLIENT pragma::physics::IEnvironment *c_physEnv = NULL;

CGame::MessagePacketTracker::MessagePacketTracker() : lastInMessageId(0), outMessageId(0) { std::fill(messageTimestamps.begin(), messageTimestamps.end(), 0); }

bool CGame::MessagePacketTracker::IsMessageInOrder(uint8_t messageId) const { return (messageTimestamps[messageId] > messageTimestamps[lastInMessageId]) ? true : false; }

void CGame::MessagePacketTracker::CheckMessages(uint8_t messageId, std::vector<double> &lostPacketTimestamps, const double &tCur)
{
	if(messageId != static_cast<decltype(messageId)>(lastInMessageId + 1)) // We've lost at least 1 packet
	{
		for(auto id = static_cast<decltype(messageId)>(lastInMessageId + 1); id != messageId; ++id) // Assume all messages inbetween are lost packets
			lostPacketTimestamps.push_back(tCur);
	}
	lastInMessageId = messageId;
}

void CGame::MessagePacketTracker::CheckMessages(uint8_t messageId, std::vector<double> &lostPacketTimestamps)
{
	if(messageId != static_cast<decltype(messageId)>(lastInMessageId + 1)) // We've lost at least 1 packet
	{
		for(auto id = static_cast<decltype(messageId)>(lastInMessageId + 1); id != messageId; ++id) // Check which packets we've lost
		{
			auto &ts = messageTimestamps[id];
			if(ts < messageTimestamps[messageId] && ts > messageTimestamps[lastInMessageId]) // We've lost this packet
				lostPacketTimestamps.push_back(ts);
		}
	}
	lastInMessageId = messageId;
}

//////////////////////////

static std::vector<std::string> g_requiredGameTextures = {"white", "black", "transparent", "white_unlit", "error"};
std::vector<std::string> &get_required_game_textures() { return g_requiredGameTextures; }

namespace pragma::rendering {
	class LightingStageRenderProcessor;
	class DepthStageRenderProcessor;
};
static auto cvWorkerThreadCount = GetClientConVar("render_queue_worker_thread_count");
CGame::CGame(NetworkState *state)
    : Game(state), m_tServer(0), m_renderScene(util::TWeakSharedHandle<pragma::CSceneComponent> {}), m_matOverride(NULL), m_colScale(1, 1, 1, 1),
      //m_shaderOverride(NULL), // prosper TODO
      m_matLoad(), m_scene(nullptr),
      /*m_dummyVertexBuffer(nullptr),*/ m_tLastClientUpdate(0.0), // prosper TODO
      m_snapshotTracker {}, m_userInputTracker {}, m_viewFov {util::FloatProperty::Create(pragma::BaseEnvCameraComponent::DEFAULT_VIEWMODEL_FOV)}, m_luaInputBindingLayerRegister {std::make_unique<pragma::LuaInputBindingLayerRegister>()}
{
	std::fill(m_renderModesEnabled.begin(), m_renderModesEnabled.end(), true);
	c_game = this;

	m_luaShaderManager = std::make_shared<pragma::LuaShaderManager>();
	m_luaParticleModifierManager = std::make_shared<pragma::LuaParticleModifierManager>();

	umath::set_flag(m_stateFlags, StateFlags::PrepassShaderPipelineReloadRequired, false);
	umath::set_flag(m_stateFlags, StateFlags::GameWorldShaderPipelineReloadRequired, false);

	RegisterRenderMask("water", true);
	m_thirdPersonRenderMask = RegisterRenderMask("thirdperson", true);
	m_firstPersonRenderMask = RegisterRenderMask("firstperson", false);

	RegisterCallback<void, CGame *>("OnGameEnd");
	RegisterCallback<void, pragma::CLightDirectionalComponent *, pragma::CLightDirectionalComponent *>("OnEnvironmentLightSourceChanged");
	RegisterCallback<void, std::reference_wrapper<const util::DrawSceneInfo>>("Render");
	RegisterCallback<void, std::reference_wrapper<const util::DrawSceneInfo>>("PreRenderScenes");
	RegisterCallback<void>("OnRenderScenes");
	RegisterCallbackWithOptionalReturn<bool, std::reference_wrapper<const util::DrawSceneInfo>>("DrawScene");
	RegisterCallback<void>("PostRenderScenes");
	RegisterCallback<void, std::reference_wrapper<const util::DrawSceneInfo>>("RenderPostProcessing");
	RegisterCallback<void, std::reference_wrapper<const util::DrawSceneInfo>>("OnPreRender");
	RegisterCallback<void, std::reference_wrapper<const util::DrawSceneInfo>, std::reference_wrapper<pragma::rendering::DepthStageRenderProcessor>>("RenderPrepass");
	RegisterCallback<void, std::reference_wrapper<const util::DrawSceneInfo>>("PreRenderScene");
	RegisterCallback<void, std::reference_wrapper<const util::DrawSceneInfo>>("PostRenderScene");
	RegisterCallback<void, pragma::CPlayerComponent *>("OnLocalPlayerSpawned");
	RegisterCallback<void, std::reference_wrapper<Vector3>, std::reference_wrapper<Quat>, std::reference_wrapper<Quat>>("CalcView");
	RegisterCallback<void, std::reference_wrapper<Vector3>, std::reference_wrapper<Quat>>("CalcViewOffset");
	RegisterCallback<void, std::reference_wrapper<const util::DrawSceneInfo>, std::reference_wrapper<std::shared_ptr<prosper::RenderTarget>>>("PreRender");
	RegisterCallback<void, std::reference_wrapper<const util::DrawSceneInfo>, std::reference_wrapper<std::shared_ptr<prosper::RenderTarget>>>("PostRender");
	RegisterCallback<void, CBaseEntity *>("UpdateEntityModel");
	RegisterCallback<void, WIBase *, WIBase *>("OnGUIFocusChanged");

	auto &staticCallbacks = get_static_client_callbacks();
	for(auto it = staticCallbacks.begin(); it != staticCallbacks.end(); ++it) {
		auto &name = it->first;
		auto &hCallback = it->second;
		AddCallback(name, hCallback);
	}

	LoadAuxEffects("fx_generic.udm");
	for(auto &rsnd : client->GetSounds()) {
		auto &snd = static_cast<CALSound &>(rsnd.get());
		snd.SetPitchModifier(GetTimeScale()); // TODO Implement SetPitchModifier for SoundScripts
	}
	AddCallback("OnSoundCreated", FunctionCallback<void, ALSound *>::Create([](ALSound *snd) {
		auto *csnd = dynamic_cast<CALSound *>(snd);
		if(csnd == nullptr)
			return;
		csnd->SetPitchModifier(c_game->GetTimeScale());
	}));

	WGUI::GetInstance().SetFocusCallback([this](WIBase *oldFocus, WIBase *newFocus) {
		CallCallbacks<void, WIBase *, WIBase *>("OnGUIFocusChanged", oldFocus, newFocus);

		auto *l = GetLuaState();
		if(l == nullptr)
			return;
		auto oOldFocus = oldFocus ? WGUILuaInterface::GetLuaObject(l, *oldFocus) : luabind::object {};
		auto oNewFocus = newFocus ? WGUILuaInterface::GetLuaObject(l, *newFocus) : luabind::object {};
		CallLuaCallbacks<void, luabind::object, luabind::object>("OnGUIFocusChanged", oOldFocus, oNewFocus);
	});

	m_cbGPUProfilingHandle = c_engine->AddGPUProfilingHandler([this](bool profilingEnabled) {
		if(profilingEnabled == false) {
			m_gpuProfilingStageManager = nullptr;
			return;
		}
		m_gpuProfilingStageManager = std::make_unique<pragma::debug::ProfilingStageManager<pragma::debug::GPUProfilingStage>>();
		auto &gpuProfiler = c_engine->GetGPUProfiler();
		m_gpuProfilingStageManager->InitializeProfilingStageManager(gpuProfiler);
	});
	m_cbProfilingHandle = c_engine->AddProfilingHandler([this](bool profilingEnabled) {
		if(profilingEnabled == false) {
			m_profilingStageManager = nullptr;
			return;
		}
		auto &cpuProfiler = c_engine->GetProfiler();
		m_profilingStageManager = std::make_unique<pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage>>();
		m_profilingStageManager->InitializeProfilingStageManager(cpuProfiler);
	});

	m_renderQueueBuilder = std::make_unique<pragma::rendering::RenderQueueBuilder>();
	m_renderQueueWorkerManager = std::make_unique<pragma::rendering::RenderQueueWorkerManager>(umath::clamp(cvWorkerThreadCount->GetInt(), 1, 20));

	auto &texManager = static_cast<msys::CMaterialManager &>(static_cast<ClientState *>(GetNetworkState())->GetMaterialManager()).GetTextureManager();
	for(auto &tex : g_requiredGameTextures) {
		texManager.LoadAsset(tex); // Pre-loaded in ClientState constructor
		texManager.FlagAssetAsAlwaysInUse(tex, true);
	}
}

CGame::~CGame() {}

void CGame::OnRemove()
{
	m_flags |= GameFlags::ClosingGame;
	m_renderQueueWorkerManager = nullptr;
	m_renderQueueBuilder = nullptr;
	c_engine->GetRenderContext().WaitIdle();
	WGUI::GetInstance().SetFocusCallback(nullptr);
	if(m_hCbDrawFrame.IsValid())
		m_hCbDrawFrame.Remove();
	if(m_cbGPUProfilingHandle.IsValid())
		m_cbGPUProfilingHandle.Remove();
	if(m_cbProfilingHandle.IsValid())
		m_cbProfilingHandle.Remove();
	CallCallbacks<void, CGame *>("OnGameEnd", this);
	while(!m_luaGUIObjects.empty()) {
		auto &hElement = m_luaGUIObjects.front();
		if(hElement.IsValid())
			hElement->Remove();
		m_luaGUIObjects.pop();
	}
	Lua::gui::clear_lua_callbacks(GetLuaState());

	// This will make sure all lua-created input binding layers have been destroyed
	m_luaInputBindingLayerRegister = nullptr;

	DebugRenderer::ClearObjects();

	SetTimeScale(1.f);

	for(auto *ent : m_ents) {
		if(!ent)
			continue;
		ent->Remove();
	}
	m_listener = decltype(m_listener) {};
	m_worldComponents.clear();
	m_plLocal = decltype(m_plLocal) {};
	m_viewModel = decltype(m_viewModel) {};
	m_viewBody = decltype(m_viewBody) {};

	for(auto &layer : c_engine->GetInputBindingLayers())
		layer->ClearLuaKeyMappings();
	const_cast<InputBindingLayer &>(c_engine->GetEffectiveInputBindingLayer()).ClearLuaKeyMappings();

	c_physEnv = nullptr;
	if(m_renderScene.valid())
		m_renderScene->GetEntity().Remove();
	m_worldEnvironment = nullptr;
	m_globalRenderSettingsBufferData = nullptr;
	m_luaGUIElements = {};
	m_luaShaderManager = nullptr;
	m_luaParticleModifierManager = nullptr;
	m_gpuProfilingStageManager = nullptr;
	m_profilingStageManager = nullptr;
	pragma::reset_lua_shaders();

	ClearSoundCache();

	Game::OnRemove();
}

void CGame::GetRegisteredEntities(std::vector<std::string> &classes, std::vector<std::string> &luaClasses) const
{
	std::unordered_map<std::string, CBaseEntity *(*)(void)> *factories = nullptr;
	g_ClientEntityFactories->GetFactories(&factories);
	classes.reserve(classes.size() + factories->size());
	for(auto &pair : *factories)
		classes.push_back(pair.first);
	GetLuaRegisteredEntities(luaClasses);
}

void CGame::OnGameWorldShaderSettingsChanged(const pragma::rendering::GameWorldShaderSettings &newSettings, const pragma::rendering::GameWorldShaderSettings &oldSettings)
{
	if(newSettings.fxaaEnabled != oldSettings.fxaaEnabled || newSettings.bloomEnabled != oldSettings.bloomEnabled) {
		auto shader = c_engine->GetShaderManager().GetShader("pp_hdr");
		if(shader.valid())
			shader->ReloadPipelines();
	}
	if(newSettings.ssaoEnabled != oldSettings.ssaoEnabled)
		ReloadPrepassShaderPipelines();
	if(newSettings.shadowQuality != oldSettings.shadowQuality || newSettings.ssaoEnabled != oldSettings.ssaoEnabled || newSettings.bloomEnabled != oldSettings.bloomEnabled || newSettings.debugModeEnabled != oldSettings.debugModeEnabled || newSettings.iblEnabled != oldSettings.iblEnabled
	  || newSettings.dynamicLightingEnabled != oldSettings.dynamicLightingEnabled || newSettings.dynamicShadowsEnabled != oldSettings.dynamicShadowsEnabled)
		ReloadGameWorldShaderPipelines();
	if(newSettings.dynamicLightingEnabled != oldSettings.dynamicLightingEnabled) {
		auto shader = c_engine->GetShaderManager().GetShader("forwardp_light_culling");
		if(shader.valid())
			shader->ReloadPipelines();
	}
}

static void cmd_render_ibl_enabled(NetworkState *, const ConVar &, bool, bool enabled)
{
	if(client == nullptr)
		return;
	client->UpdateGameWorldShaderSettings();
}
REGISTER_CONVAR_CALLBACK_CL(render_ibl_enabled, cmd_render_ibl_enabled);
REGISTER_CONVAR_CALLBACK_CL(render_dynamic_lighting_enabled, cmd_render_ibl_enabled);
REGISTER_CONVAR_CALLBACK_CL(render_dynamic_shadows_enabled, cmd_render_ibl_enabled);

static void cmd_render_queue_worker_thread_count(NetworkState *, const ConVar &, int, int val)
{
	if(c_game == nullptr)
		return;
	val = umath::clamp(val, 1, 20);
	c_game->GetRenderQueueWorkerManager().SetWorkerCount(val);
}
REGISTER_CONVAR_CALLBACK_CL(render_queue_worker_thread_count, cmd_render_queue_worker_thread_count);

static void cmd_render_queue_worker_jobs_per_batch(NetworkState *, const ConVar &, int, int val)
{
	if(c_game == nullptr)
		return;
	val = umath::max(val, 1);
	c_game->GetRenderQueueWorkerManager().SetJobsPerBatchCount(val);
}
REGISTER_CONVAR_CALLBACK_CL(render_queue_worker_jobs_per_batch, cmd_render_queue_worker_jobs_per_batch);

pragma::rendering::RenderQueueBuilder &CGame::GetRenderQueueBuilder() { return *m_renderQueueBuilder; }
pragma::rendering::RenderQueueWorkerManager &CGame::GetRenderQueueWorkerManager() { return *m_renderQueueWorkerManager; }

void CGame::UpdateTime()
{
	// TODO: This also has to be applied serverside?
	auto dt = c_engine->GetDeltaFrameTime();
	float timeScale = GetTimeScale();
	m_ctCur.UpdateByDelta(dt * timeScale);
	m_ctReal.UpdateByDelta(dt);
	m_tCur = CDouble(m_ctCur());
	m_tReal = CDouble(m_ctReal());
	m_tDelta = CDouble(m_tCur - m_tLast);
	m_tDeltaReal = CDouble(m_tReal - m_tLastReal);
}

bool CGame::StartGPUProfilingStage(const char *stage) { return m_gpuProfilingStageManager && m_gpuProfilingStageManager->StartProfilerStage(stage); }
bool CGame::StopGPUProfilingStage() { return m_gpuProfilingStageManager && m_gpuProfilingStageManager->StopProfilerStage(); }
pragma::debug::ProfilingStageManager<pragma::debug::GPUProfilingStage> *CGame::GetGPUProfilingStageManager() { return m_gpuProfilingStageManager.get(); }

pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage> *CGame::GetProfilingStageManager() { return m_profilingStageManager.get(); }
bool CGame::StartProfilingStage(const char *stage) { return m_profilingStageManager && m_profilingStageManager->StartProfilerStage(stage); }
bool CGame::StopProfilingStage() { return m_profilingStageManager && m_profilingStageManager->StopProfilerStage(); }

std::shared_ptr<pragma::EntityComponentManager> CGame::InitializeEntityComponentManager() { return std::make_shared<pragma::CEntityComponentManager>(); }

void CGame::OnReceivedRegisterNetEvent(NetPacket &packet)
{
	auto name = packet->ReadString();
	auto localId = SetupNetEvent(name);
	auto sharedId = packet->Read<pragma::NetEventId>();
	if(sharedId >= m_clientNetEventData.sharedNetEventIdToLocalId.size()) {
		if(m_clientNetEventData.sharedNetEventIdToLocalId.size() == m_clientNetEventData.sharedNetEventIdToLocalId.capacity())
			m_clientNetEventData.sharedNetEventIdToLocalId.reserve(m_clientNetEventData.sharedNetEventIdToLocalId.size() * 1.1f + 100);
		m_clientNetEventData.sharedNetEventIdToLocalId.resize(sharedId + 1, std::numeric_limits<pragma::NetEventId>::max());
	}
	m_clientNetEventData.sharedNetEventIdToLocalId[sharedId] = localId;
}

pragma::NetEventId CGame::SharedNetEventIdToLocal(pragma::NetEventId evId) const { return (evId < m_clientNetEventData.sharedNetEventIdToLocalId.size()) ? m_clientNetEventData.sharedNetEventIdToLocalId[evId] : std::numeric_limits<pragma::NetEventId>::max(); }

pragma::NetEventId CGame::LocalNetEventIdToShared(pragma::NetEventId evId) const { return (evId < m_clientNetEventData.localNetEventIdToSharedId.size()) ? m_clientNetEventData.localNetEventIdToSharedId[evId] : std::numeric_limits<pragma::NetEventId>::max(); }

pragma::NetEventId CGame::FindNetEvent(const std::string &name) const
{
	auto it = m_clientNetEventData.localNetEventIds.find(name);
	if(it == m_clientNetEventData.localNetEventIds.end())
		return std::numeric_limits<pragma::NetEventId>::max();
	return it->second;
}

pragma::NetEventId CGame::SetupNetEvent(const std::string &name)
{
	auto it = m_clientNetEventData.localNetEventIds.find(name);
	if(it != m_clientNetEventData.localNetEventIds.end())
		return it->second;
	m_clientNetEventData.localNetEventIds.insert(std::make_pair(name, m_clientNetEventData.nextLocalNetEventId++));
	return m_clientNetEventData.nextLocalNetEventId - 1;
}

std::shared_ptr<pragma::nav::Mesh> CGame::LoadNavMesh(const std::string &fname) { return pragma::nav::CMesh::Load(*this, fname); }

WorldEnvironment &CGame::GetWorldEnvironment() { return *m_worldEnvironment; }
const WorldEnvironment &CGame::GetWorldEnvironment() const { return const_cast<CGame *>(this)->GetWorldEnvironment(); }

void CGame::InitializeWorldEnvironment() { m_worldEnvironment = WorldEnvironment::Create(); }

void CGame::SetRenderClipPlane(const Vector4 &clipPlane) { m_clipPlane = clipPlane; }
const Vector4 &CGame::GetRenderClipPlane() const { return m_clipPlane; }

pragma::CParticleSystemComponent *CGame::CreateParticleTracer(const Vector3 &start, const Vector3 &end, float radius, const Color &col, float length, float speed, const std::string &material, float bloomScale)
{
	std::stringstream ssColor;
	ssColor << col.r << " " << col.g << " " << col.b << " " << col.a;
	std::unordered_map<std::string, std::string> values {{"maxparticles", "1"}, {"max_node_count", "2"}, {"emission_rate", "10000"}, {"material", material}, {"radius", std::to_string(radius)}, {"color", ssColor.str()}, {"sort_particles", "0"}, {"bloom_scale", std::to_string(bloomScale)}};
	auto *particle = pragma::CParticleSystemComponent::Create(values, nullptr);
	if(particle == nullptr)
		return nullptr;
	std::unordered_map<std::string, std::string> beamValues {{"node_start", "1"}, {"node_end", "2"}, {"curvature", "0.0"}};
	particle->AddRenderer("beam", beamValues);
	auto hParticle = particle->GetHandle<pragma::CParticleSystemComponent>();

	auto dir = end - start;
	auto prevDist = uvec::length(dir);
	if(prevDist > 0.f)
		dir /= prevDist;
	particle->SetNodeTarget(1, start);
	particle->SetNodeTarget(2, start + dir * static_cast<float>(length));
	particle->Start();
	auto cb = FunctionCallback<void>::Create(nullptr);
	auto cStart = start;
	auto bSkip = true;
	static_cast<Callback<void> *>(cb.get())->SetFunction([cb, hParticle, cStart, dir, end, speed, length, prevDist, bSkip]() mutable {
		if(hParticle.expired()) {
			cb.Remove();
			return;
		}
		if(bSkip == true) // Skip the first tick (Wait until particle has actually spawned!)
		{
			bSkip = false;
			return;
		}
		auto &tDelta = c_game->DeltaTime();
		cStart += dir * static_cast<float>(speed * tDelta);
		auto dist = uvec::distance(cStart, end);
		length = umath::min(static_cast<float>(length), dist);
		if(length <= 0.f || dist > prevDist) {
			hParticle->GetEntity().Remove();
			cb.Remove();
			return;
		}
		auto nEnd = cStart + dir * static_cast<float>(length);
		auto *pt = hParticle.get();
		pt->SetNodeTarget(1, cStart);
		pt->SetNodeTarget(2, nEnd);
		prevDist = dist;
	});
	c_game->AddCallback("Think", cb);
	return particle;
}

void CGame::SetRenderModeEnabled(pragma::rendering::SceneRenderPass renderMode, bool bEnabled) { m_renderModesEnabled[umath::to_integral(renderMode)] = bEnabled; }
void CGame::EnableRenderMode(pragma::rendering::SceneRenderPass renderMode) { SetRenderModeEnabled(renderMode, true); }
void CGame::DisableRenderMode(pragma::rendering::SceneRenderPass renderMode) { SetRenderModeEnabled(renderMode, false); }
bool CGame::IsRenderModeEnabled(pragma::rendering::SceneRenderPass renderMode) const { return m_renderModesEnabled[umath::to_integral(renderMode)]; }

Material *CGame::GetLoadMaterial() { return m_matLoad.get(); }
void CGame::OnEntityCreated(BaseEntity *ent)
{
	Game::OnEntityCreated(ent);
	if(typeid(*ent) == typeid(CGameEntity)) {
		m_entGame = ent->GetHandle();
		m_gameComponent = ent->GetComponent<pragma::CGameComponent>();
	}
}

pragma::CGameComponent *CGame::GetGameComponent() { return m_gameComponent.get(); }

pragma::CViewModelComponent *CGame::GetViewModel()
{
	if(m_viewModel.expired())
		return NULL;
	return m_viewModel.get();
}

pragma::CViewBodyComponent *CGame::GetViewBody()
{
	if(m_viewBody.expired())
		return NULL;
	return m_viewBody.get();
}

static void shader_handler(Material *mat)
{
	if(mat->IsLoaded() == false)
		return;
	auto &shaderManager = c_engine->GetShaderManager();
	auto whShader = shaderManager.GetShader(mat->GetShaderIdentifier());
	if(whShader.expired())
		return;
	auto &cmat = *static_cast<CMaterial *>(mat);
	// Clear descriptor set in case the shader has changed; The new shader will re-create it appropriately
	cmat.SetDescriptorSetGroup(*whShader.get(), nullptr);

	auto *shader = dynamic_cast<pragma::ShaderTexturedBase *>(whShader.get());
	auto *pShader = dynamic_cast<prosper::Shader *>(shader);
	if(pShader == nullptr || pShader->IsValid() == false)
		return;
	mat->SetUserData(shader);
	shader->InitializeMaterialDescriptorSet(cmat, false);

	// TODO: Cache this
	auto *prepass = dynamic_cast<pragma::ShaderTexturedBase *>(c_engine->GetShader("prepass").get());
	if(prepass)
		prepass->InitializeMaterialDescriptorSet(cmat, false); // TODO: Only if this is a material with masked transparency?
}

void CGame::ReloadMaterialShader(CMaterial *mat)
{
	auto *shader = static_cast<pragma::ShaderTexturedBase *>(mat->GetUserData());
	if(shader == nullptr)
		return;
	shader->InitializeMaterialDescriptorSet(*mat, true);
}

void CGame::Initialize()
{
	Game::Initialize();
	auto &materialManager = static_cast<msys::CMaterialManager &>(client->GetMaterialManager());
	materialManager.SetShaderHandler(&shader_handler);
	pragma::CRenderComponent::InitializeBuffers();
	pragma::CLightComponent::InitializeBuffers();
	CModelSubMesh::InitializeBuffers();
	pragma::CParticleSystemComponent::InitializeBuffers();

	InitShaders();

	pragma::CParticleSystemComponent::Precache("impact");
	pragma::CParticleSystemComponent::Precache("muzzleflash");
	pragma::CParticleSystemComponent::Precache("explosion");

	// Initialize Scene (Has to be initialized AFTER shaders!)

	InitializeWorldEnvironment();
	auto *mat = client->LoadMaterial("loading", CallbackHandle {}, false, true);
	m_matLoad = mat ? mat->GetHandle() : nullptr;
}

static void render_debug_mode(NetworkState *, const ConVar &, int32_t, int32_t debugMode)
{
	if(client == nullptr)
		return;
	client->UpdateGameWorldShaderSettings();
	EntityIterator entIt {*c_game, EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CSceneComponent>>();
	for(auto *ent : entIt) {
		auto sceneC = ent->GetComponent<pragma::CSceneComponent>();
		sceneC->SetDebugMode(static_cast<pragma::SceneDebugMode>(debugMode));
	}
}
REGISTER_CONVAR_CALLBACK_CL(render_debug_mode, render_debug_mode);

static void CVAR_CALLBACK_render_unlit(NetworkState *nw, const ConVar &cv, bool prev, bool val) { render_debug_mode(nw, cv, prev, umath::to_integral(pragma::SceneDebugMode::Unlit)); }
REGISTER_CONVAR_CALLBACK_CL(render_unlit, CVAR_CALLBACK_render_unlit);

void CGame::SetViewModelFOV(float fov) { *m_viewFov = fov; }
const util::PFloatProperty &CGame::GetViewModelFOVProperty() const { return m_viewFov; }
float CGame::GetViewModelFOV() const { return *m_viewFov; }
float CGame::GetViewModelFOVRad() const { return umath::deg_to_rad(*m_viewFov); }
Mat4 CGame::GetViewModelProjectionMatrix() const
{
	auto *cam = GetPrimaryCamera();
	auto aspectRatio = cam ? cam->GetAspectRatio() : 1.f;
	auto nearZ = cam ? cam->GetNearZ() : pragma::BaseEnvCameraComponent::DEFAULT_NEAR_Z;
	auto farZ = cam ? cam->GetFarZ() : pragma::BaseEnvCameraComponent::DEFAULT_FAR_Z;
	return pragma::BaseEnvCameraComponent::CalcProjectionMatrix(*m_viewFov, aspectRatio, nearZ, farZ);
}

pragma::CCameraComponent *CGame::CreateCamera(float aspectRatio, float fov, float nearZ, float farZ)
{
	auto *cam = CreateEntity<CEnvCamera>();
	auto whCamComponent = cam ? cam->GetComponent<pragma::CCameraComponent>() : pragma::ComponentHandle<pragma::CCameraComponent> {};
	if(whCamComponent.expired()) {
		if(cam)
			cam->RemoveSafely();
		return nullptr;
	}
	auto *pCameraComponent = whCamComponent.get();
	pCameraComponent->SetAspectRatio(aspectRatio);
	pCameraComponent->SetFOV(fov);
	pCameraComponent->SetNearZ(nearZ);
	pCameraComponent->SetFarZ(farZ);
	pCameraComponent->UpdateMatrices();
	cam->Spawn();
	return pCameraComponent;
}

pragma::CCameraComponent *CGame::CreateCamera(uint32_t width, uint32_t height, float fov, float nearZ, float farZ) { return CreateCamera(width / static_cast<float>(height), fov, nearZ, farZ); }

void CGame::InitializeGame() // Called by NET_cl_resourcecomplete
{
	Game::InitializeGame();
	SetupLua();

	m_hCbDrawFrame = c_engine->AddCallback("DrawFrame", FunctionCallback<void, std::reference_wrapper<std::shared_ptr<prosper::IPrimaryCommandBuffer>>>::Create([this](std::reference_wrapper<std::shared_ptr<prosper::IPrimaryCommandBuffer>> drawCmd) {
		auto baseDrawCmd = std::static_pointer_cast<prosper::ICommandBuffer>(drawCmd.get());
		CallLuaCallbacks<void, std::shared_ptr<prosper::ICommandBuffer>>("DrawFrame", baseDrawCmd);
	}));

	auto &materialManager = static_cast<msys::CMaterialManager &>(client->GetMaterialManager());
	if(m_surfaceMaterialManager)
		m_surfaceMaterialManager->Load("scripts/physics/materials.udm");

	auto resolution = c_engine->GetRenderResolution();
	c_engine->GetRenderContext().GetPipelineLoader().Flush();
	c_engine->GetRenderContext().SavePipelineCache();

	m_globalRenderSettingsBufferData = std::make_unique<GlobalRenderSettingsBufferData>();
	auto *scene = pragma::CSceneComponent::Create(pragma::CSceneComponent::CreateInfo {});
	if(scene) {
		scene->GetEntity().SetName("scene_game");
		m_scene = scene->GetHandle<pragma::CSceneComponent>();
		m_scene->SetDebugMode(static_cast<pragma::SceneDebugMode>(GetConVarInt("render_debug_mode")));
		SetViewModelFOV(GetConVarFloat("cl_fov_viewmodel"));

		auto *entRenderer = CreateEntity<CRasterizationRenderer>();
		if(entRenderer) {
			auto rasterization = entRenderer->GetComponent<pragma::CRasterizationRendererComponent>();
			if(rasterization.valid()) {
				auto *renderer = rasterization->GetRendererComponent();
				if(renderer) {
					m_scene->SetRenderer(renderer);
					rasterization->SetSSAOEnabled(GetConVarBool("cl_render_ssao"));
					m_scene->ReloadRenderTarget(static_cast<uint32_t>(resolution.x), static_cast<uint32_t>(resolution.y));
					m_scene->SetWorldEnvironment(GetWorldEnvironment());
				}
			}
			entRenderer->Spawn();
		}

		SetRenderScene(*scene);
	}

	Resize(false);

	auto *cam = CreateCamera(m_scene->GetWidth(), m_scene->GetHeight(), GetConVarFloat("cl_render_fov"), c_engine->GetNearZ(), c_engine->GetFarZ());
	if(cam) {
		auto toggleC = cam->GetEntity().GetComponent<pragma::CToggleComponent>();
		if(toggleC.valid())
			toggleC->TurnOn();
		m_scene->SetActiveCamera(*cam);
		m_primaryCamera = cam->GetHandle<pragma::CCameraComponent>();

		cam->GetEntity().AddComponent<pragma::CObserverComponent>();
	}

	m_flags |= GameFlags::GameInitialized;
	CallCallbacks<void, Game *>("OnGameInitialized", this);
	for(auto *gmC : GetGamemodeComponents())
		gmC->OnGameInitialized();
}

void CGame::RequestResource(const std::string &fileName)
{
	Con::ccl << "[CGame] RequestResource '" << fileName << "'" << Con::endl;
	auto fName = FileManager::GetCanonicalizedPath(fileName);
	auto it = std::find(m_requestedResources.begin(), m_requestedResources.end(), fName);
	if(it != m_requestedResources.end())
		return;
	m_requestedResources.push_back(fName);
	NetPacket p;
	p->WriteString(fName);
	client->SendPacket("query_resource", p, pragma::networking::Protocol::SlowReliable);
	Con::ccl << "[CGame] Request sent!" << Con::endl;
}

void CGame::Resize(bool reloadRenderTarget)
{
	if(reloadRenderTarget)
		ReloadRenderFrameBuffer();
	auto *cam = GetRenderCamera();
	if(cam != nullptr) {
		cam->SetAspectRatio(c_engine->GetWindow().GetAspectRatio());
		cam->UpdateMatrices();
	}

	CallLuaCallbacks("OnRenderTargetResized");
}

void CGame::PreGUIDraw() { CallLuaCallbacks<void>("PreGUIDraw"); }
void CGame::PostGUIDraw() { CallLuaCallbacks<void>("PostGUIDraw"); }
void CGame::PreGUIRecord() { CallLuaCallbacks<void>("PreGUIRecord"); }
void CGame::PostGUIRecord() { CallLuaCallbacks<void>("PostGUIRecord"); }
void CGame::SetDefaultGameRenderEnabled(bool enabled) { m_defaultGameRenderEnabled = enabled; }
bool CGame::IsDefaultGameRenderEnabled() const { return m_defaultGameRenderEnabled; }
uint32_t CGame::GetNumberOfScenesQueuedForRendering() const { return m_sceneRenderQueue.size(); }
util::DrawSceneInfo *CGame::GetQueuedSceneRenderInfo(uint32_t i) { return (i < m_sceneRenderQueue.size()) ? &m_sceneRenderQueue[i] : nullptr; }
void CGame::QueueForRendering(const util::DrawSceneInfo &drawSceneInfo) { m_sceneRenderQueue.push_back(drawSceneInfo); }
const std::vector<util::DrawSceneInfo> &CGame::GetQueuedRenderScenes() const { return m_sceneRenderQueue; }
void CGame::SetRenderScene(pragma::CSceneComponent &scene) { m_renderScene = scene.GetHandle<pragma::CSceneComponent>(); }
void CGame::ResetRenderScene() { m_renderScene = m_scene; }
pragma::CSceneComponent *CGame::GetRenderScene() { return m_renderScene.get(); }
const pragma::CSceneComponent *CGame::GetRenderScene() const { return const_cast<CGame *>(this)->GetRenderScene(); }
pragma::CCameraComponent *CGame::GetRenderCamera() const
{
	if(m_renderScene.expired())
		return nullptr;
	return const_cast<pragma::CCameraComponent *>(m_renderScene->GetActiveCamera().get());
}
void CGame::SetGameplayControlCamera(pragma::CCameraComponent &cam)
{
	m_controlCamera = cam.GetHandle<pragma::CCameraComponent>();
	m_stateFlags &= ~StateFlags::DisableGamplayControlCamera;
}
void CGame::ResetGameplayControlCamera()
{
	m_controlCamera = pragma::ComponentHandle<pragma::CCameraComponent> {};
	m_stateFlags &= ~StateFlags::DisableGamplayControlCamera;
}
void CGame::ClearGameplayControlCamera()
{
	m_controlCamera = pragma::ComponentHandle<pragma::CCameraComponent> {};
	m_stateFlags |= StateFlags::DisableGamplayControlCamera;
}
pragma::CCameraComponent *CGame::GetGameplayControlCamera()
{
	if(m_controlCamera.valid())
		return m_controlCamera.get();
	if(umath::is_flag_set(m_stateFlags, StateFlags::DisableGamplayControlCamera))
		return nullptr;
	return GetRenderCamera();
}
pragma::CCameraComponent *CGame::GetPrimaryCamera() const { return const_cast<pragma::CCameraComponent *>(m_primaryCamera.get()); }

void CGame::SetMaterialOverride(Material *mat) { m_matOverride = mat; }
Material *CGame::GetMaterialOverride() { return m_matOverride; }

void CGame::SetColorScale(const Vector4 &col) { m_colScale = col; }
Vector4 &CGame::GetColorScale() { return m_colScale; }
void CGame::SetAlphaScale(float a) { m_colScale.a = a; }
float CGame::GetAlphaScale() { return m_colScale.a; }

WIBase *CGame::CreateGUIElement(std::string className, WIBase *parent)
{
	auto *o = m_luaGUIElements.GetClassObject(className);
	auto &gui = WGUI::GetInstance();
	WIBase *el = nullptr;
	if(o != nullptr) {
		luabind::object r;
#ifndef LUABIND_NO_EXCEPTIONS
		try {
#endif
			// Object is created through Luabind using 'new'
			// but we steal it with our WILuaBaseHolder holder so it can get destroyed through
			// the WGUI library instead.
			r = (*o)();

			auto *elLua = luabind::object_cast<WILuaBase *>(r);
			auto *holder = luabind::object_cast<pragma::lua::WILuaBaseHolder *>(r);
			if(elLua && holder) {
				ustring::to_lower(className);
				elLua->SetupLua(r, className);
				WGUI::GetInstance().RegisterElement(*elLua, className, parent);
				holder->SetHandle(util::weak_shared_handle_cast<WIBase, WILuaBase>(elLua->GetHandle()));
				el = elLua;
			}
			else {
				Con::cwar << Con::PREFIX_CLIENT << "Unable to create lua GUI Element '" << className << "': Lua class is not derived from valid GUI base!" << Con::endl;
				return nullptr;
			}

#ifndef LUABIND_NO_EXCEPTIONS
		}
		catch(luabind::error &) {
			Lua::HandleLuaError(GetLuaState());
			return nullptr;
		}
#endif
		if(!r) {
			Con::cwar << Con::PREFIX_CLIENT << "Unable to create lua GUI Element '" << className << "'!" << Con::endl;
			return nullptr;
		}
	}
	else
		el = gui.Create(className, parent);
	if(el == nullptr) {
		static auto skipLuaFile = false;
		if(skipLuaFile == true)
			return nullptr;
		auto lclassName = className;
		ustring::to_lower(lclassName);
		auto basePath = "gui/" + lclassName;
		auto luaPath = Lua::find_script_file(basePath);
		if(luaPath && ExecuteLuaFile(*luaPath)) {
			o = m_luaGUIElements.GetClassObject(className);
			if(o != nullptr) {
				skipLuaFile = true;
				auto *r = CreateGUIElement(className, parent);
				skipLuaFile = false;
				return r;
			}
		}
		return nullptr;
	}
	m_luaGUIObjects.push(el->GetHandle());
	return el;
}

static CVar cvLODBias = GetClientConVar("cl_render_lod_bias");
void CGame::SetLODBias(int32_t bias) { client->SetConVar("cl_render_lod_bias", std::to_string(bias)); }
int32_t CGame::GetLODBias() const { return cvLODBias->GetInt(); }
uint32_t CGame::GetLOD(float dist, uint32_t maxLod) const
{
	auto lod = CUInt32(dist / LOD_SWAP_DISTANCE) + GetLODBias();
	if(lod < 0)
		lod = 0;
	if(lod > maxLod)
		lod = maxLod;
	return static_cast<uint32_t>(lod);
}

void CGame::CreateGiblet(const GibletCreateInfo &info, pragma::CParticleSystemComponent **particle)
{
	if(particle != nullptr)
		*particle = nullptr;
	if(info.lifetime <= 0.f)
		return;
	auto *pt = pragma::CParticleSystemComponent::Create({{"maxparticles", "1"}, {"emission_rate", "10000"}, {"cast_shadows", "1"}, {"radius", std::to_string(info.scale)},
	  {"world_rotation", std::to_string(info.rotation.w) + " " + std::to_string(info.rotation.x) + " " + std::to_string(info.rotation.y) + " " + std::to_string(info.rotation.z)}});
	if(pt == nullptr)
		return;
	pt->AddInitializer("lifetime_random", {{"lifetime_min", std::to_string(info.lifetime)}, {"lifetime_max", std::to_string(info.lifetime)}});
	pt->AddInitializer("initial_velocity", {{"velocity", std::to_string(info.velocity.x) + " " + std::to_string(info.velocity.y) + " " + std::to_string(info.velocity.z)}});
	pt->AddInitializer("initial_angular_velocity", {{"velocity", std::to_string(info.angularVelocity.x) + " " + std::to_string(info.angularVelocity.y) + " " + std::to_string(info.angularVelocity.z)}});
	pt->AddRenderer("model", {{"model", info.model}, {"skin", std::to_string(info.skin)}});

	if(info.physShape != GibletCreateInfo::PhysShape::None) {
		std::stringstream ssTranslationOffset;
		ssTranslationOffset << info.physTranslationOffset.x << " " << info.physTranslationOffset.y << " " << info.physTranslationOffset.z;
		std::stringstream ssRotationOffset;
		ssRotationOffset << info.physRotationOffset.p << " " << info.physRotationOffset.y << " " << info.physRotationOffset.r;
		switch(info.physShape) {
		case GibletCreateInfo::PhysShape::Model:
			pt->AddOperator("physics_model", {{"model", info.model}, {"mass", std::to_string(info.mass)}, {"translation_offset", ssTranslationOffset.str()}, {"rotation_offset", ssRotationOffset.str()}});
			break;
		case GibletCreateInfo::PhysShape::Sphere:
			pt->AddOperator("physics_sphere", {{"mass", std::to_string(info.mass)}, {"translation_offset", ssTranslationOffset.str()}, {"rotation_offset", ssRotationOffset.str()}, {"radius", std::to_string(info.physRadius)}});
			break;
		case GibletCreateInfo::PhysShape::Box:
			pt->AddOperator("physics_box", {{"mass", std::to_string(info.mass)}, {"translation_offset", ssTranslationOffset.str()}, {"rotation_offset", ssRotationOffset.str()}, {"extent", std::to_string(info.physRadius)}});
			break;
		case GibletCreateInfo::PhysShape::Cylinder:
			pt->AddOperator("physics_cylinder", {{"mass", std::to_string(info.mass)}, {"translation_offset", ssTranslationOffset.str()}, {"rotation_offset", ssRotationOffset.str()}, {"radius", std::to_string(info.physRadius)}, {"height", std::to_string(info.physHeight)}});
			break;
		}
	}
	pt->AddOperator("gravity", {{"effective_scale", "1.0"}});
	pt->AddOperator("color_fade", {{"alpha", "0"}, {"fade_start", std::to_string(info.lifetime - 0.2f)}, {"fade_end", std::to_string(info.lifetime)}});
	auto pTrComponent = pt->GetEntity().GetTransformComponent();
	if(pTrComponent != nullptr)
		pTrComponent->SetPosition(info.position);
	pt->SetRemoveOnComplete(true);
	pt->Start();
	if(particle != nullptr)
		*particle = pt;
}

void CGame::CreateGiblet(const GibletCreateInfo &info) { CreateGiblet(info, nullptr); }

WIBase *CGame::CreateGUIElement(std::string name, WIHandle *hParent)
{
	StringToLower(name);
	WIBase *pParent = NULL;
	if(hParent != NULL && hParent->IsValid())
		pParent = hParent->get();
	return CreateGUIElement(name, pParent);
}
LuaGUIManager &CGame::GetLuaGUIManager() { return m_luaGUIElements; }
pragma::LuaShaderManager &CGame::GetLuaShaderManager() { return *m_luaShaderManager; }
pragma::LuaParticleModifierManager &CGame::GetLuaParticleModifierManager() { return *m_luaParticleModifierManager; }
pragma::LuaInputBindingLayerRegister &CGame::GetLuaInputBindingLayerRegister() { return *m_luaInputBindingLayerRegister; }

void CGame::SetUp()
{
	Game::SetUp();
	CListener *listener = CreateEntity<CListener>();
	m_listener = listener->GetComponent<pragma::CListenerComponent>();

	CViewModel *vm = CreateEntity<CViewModel>();
	m_viewModel = vm->GetComponent<pragma::CViewModelComponent>();

	CViewBody *body = CreateEntity<CViewBody>();
	m_viewBody = body->GetComponent<pragma::CViewBodyComponent>();

	auto *entPbrConverter = CreateEntity<CUtilPBRConverter>();
	entPbrConverter->Spawn();

	auto *entShadowManager = CreateEntity<CShadowManager>();
	entShadowManager->Spawn();

	auto *entOcclusionCuller = CreateEntity<COcclusionCuller>();
	entOcclusionCuller->Spawn();
}

bool WriteTGA(const char *name, int w, int h, unsigned char *pixels, int size);

void WriteCubeMapSide(int w, int, int blockSize, int block, float *inPixels, unsigned char *outPixels)
{
	//Scene &scene = *c_engine->GetScene(0);
	//Camera &cam = scene.camera;
	float n = 1.f;    //cam.GetZNear();
	float f = 1000.f; //cam.GetZFar();
	int numBlocksW = w / blockSize;
	//int numBlocksH = h /blockSize;
	unsigned int offset = (block % 4) * blockSize * 3 + (block / 4) * (w * blockSize) * 3;
	int p = 0;
	for(int y = 0; y < blockSize * 3; y += 3) {
		for(int x = 0; x < blockSize * 3; x += 3) {
			float v = inPixels[p];
			v = (2.f * n) / (f + n - v * (f - n));
			v *= 255.f;

			outPixels[offset + x] = CUChar(v);
			outPixels[offset + x + 1] = CUChar(v);
			outPixels[offset + x + 2] = CUChar(v);
			p++;
		}
		offset += blockSize * numBlocksW * 3;
	}
}

static CVar cvAntiAliasing = GetClientConVar("cl_render_anti_aliasing");
static CVar cvMsaaSamples = GetClientConVar("cl_render_msaa_samples");
uint32_t CGame::GetMSAASampleCount()
{
	auto bMsaaEnabled = static_cast<pragma::rendering::AntiAliasing>(cvAntiAliasing->GetInt()) == pragma::rendering::AntiAliasing::MSAA;
	unsigned int numSamples = bMsaaEnabled ? umath::pow(2, cvMsaaSamples->GetInt()) : 0;
	ClampMSAASampleCount(&numSamples);
	return numSamples;
}
void CGame::ReloadRenderFrameBuffer()
{
	if(m_scene.valid())
		m_scene->ReloadRenderTarget(m_scene->GetWidth(), m_scene->GetHeight());
}

void CGame::Think()
{
	Game::Think();
	auto *scene = GetRenderScene();
	auto *cam = GetPrimaryCamera();

	double tDelta = m_stateNetwork->DeltaTime();
	m_tServer += DeltaTime();
	if(m_gameComponent.valid())
		m_gameComponent->UpdateFrame(cam);
	CallCallbacks<void>("Think");
	CallLuaCallbacks("Think");
	if(m_gameComponent.valid())
		m_gameComponent->UpdateCamera(cam);

	if(scene)
		SetRenderScene(*scene);

	auto &info = get_render_debug_info();
	info.Reset();

	PostThink();
}

static CVar cvUpdateRate = GetClientConVar("cl_updaterate");
void CGame::Tick()
{
	Game::Tick();
	//HandlePlayerMovement();
	auto &t = RealTime();
	auto updateRate = cvUpdateRate->GetFloat();
	if(updateRate > 0.f && t - m_tLastClientUpdate >= (1.0 / static_cast<double>(updateRate))) {
		m_tLastClientUpdate = t;
		SendUserInput();
	}
	CallCallbacks<void>("Tick");
	CallLuaCallbacks("Tick");
	PostTick();
}

void CGame::ReloadGameWorldShaderPipelines() const
{
	if(umath::is_flag_set(m_stateFlags, StateFlags::GameWorldShaderPipelineReloadRequired))
		return;
	umath::set_flag(const_cast<CGame *>(this)->m_stateFlags, StateFlags::GameWorldShaderPipelineReloadRequired);

	auto cb = FunctionCallback<void>::Create(nullptr);
	static_cast<Callback<void> *>(cb.get())->SetFunction([this, cb]() mutable {
		cb.Remove();

		if(!umath::is_flag_set(const_cast<CGame *>(this)->m_stateFlags, StateFlags::GameWorldShaderPipelineReloadRequired))
			return;
		umath::set_flag(const_cast<CGame *>(this)->m_stateFlags, StateFlags::GameWorldShaderPipelineReloadRequired, false);
		auto &shaderManager = c_engine->GetShaderManager();
		for(auto &shader : shaderManager.GetShaders()) {
			auto *gameWorldShader = dynamic_cast<pragma::ShaderGameWorldLightingPass *>(shader.get());
			if(gameWorldShader == nullptr)
				continue;
			gameWorldShader->ReloadPipelines();
		}
		c_engine->GetRenderContext().GetPipelineLoader().Flush();
	});
	const_cast<CGame *>(this)->AddCallback("PreRenderScenes", cb);
}
void CGame::ReloadPrepassShaderPipelines() const
{
	if(umath::is_flag_set(m_stateFlags, StateFlags::PrepassShaderPipelineReloadRequired))
		return;
	umath::set_flag(const_cast<CGame *>(this)->m_stateFlags, StateFlags::PrepassShaderPipelineReloadRequired);

	auto cb = FunctionCallback<void>::Create(nullptr);
	static_cast<Callback<void> *>(cb.get())->SetFunction([this, cb]() mutable {
		cb.Remove();

		umath::set_flag(const_cast<CGame *>(this)->m_stateFlags, StateFlags::PrepassShaderPipelineReloadRequired, false);
		auto &shader = GetGameShader(GameShader::Prepass);
		if(shader.valid())
			shader.get()->ReloadPipelines();
	});
	const_cast<CGame *>(this)->AddCallback("PreRenderScenes", cb);
}

static CVar cvSimEnabled = GetClientConVar("cl_physics_simulation_enabled");
bool CGame::IsPhysicsSimulationEnabled() const { return cvSimEnabled->GetBool(); }

const util::WeakHandle<prosper::Shader> &CGame::GetGameShader(GameShader shader) const { return m_gameShaders.at(umath::to_integral(shader)); }

LuaCallbackHandler &CGame::GetInputCallbackHandler() { return m_inputCallbackHandler; }

std::shared_ptr<ModelMesh> CGame::CreateModelMesh() const { return std::make_shared<CModelMesh>(); }
std::shared_ptr<ModelSubMesh> CGame::CreateModelSubMesh() const { return std::make_shared<CModelSubMesh>(); }

Float CGame::GetHDRExposure() const
{
	auto *renderer = m_scene->GetRenderer();
	auto raster = renderer ? renderer->GetEntity().GetComponent<pragma::CRasterizationRendererComponent>() : pragma::ComponentHandle<pragma::CRasterizationRendererComponent> {};
	return raster.valid() ? raster->GetHDRExposure() : 0.f;
}
Float CGame::GetMaxHDRExposure() const
{
	auto *renderer = m_scene->GetRenderer();
	auto raster = renderer ? renderer->GetEntity().GetComponent<pragma::CRasterizationRendererComponent>() : pragma::ComponentHandle<pragma::CRasterizationRendererComponent> {};
	return raster.valid() ? raster->GetMaxHDRExposure() : 0.f;
}
void CGame::SetMaxHDRExposure(Float exposure)
{
	auto *renderer = m_scene->GetRenderer();
	auto raster = renderer ? renderer->GetEntity().GetComponent<pragma::CRasterizationRendererComponent>() : pragma::ComponentHandle<pragma::CRasterizationRendererComponent> {};
	if(raster.expired())
		return;
	raster->SetMaxHDRExposure(exposure);
}

void CGame::OnMapLoaded()
{
	Game::OnMapLoaded();

	auto *scene = GetRenderScene();
	if(scene)
		scene->UpdateRenderData();

	// Update reflection probes
	// TODO: Make sure all map materials have been fully loaded before doing this!
	// pragma::CReflectionProbeComponent::BuildAllReflectionProbes(*this);
}

void CGame::InitializeMapEntities(pragma::asset::WorldData &worldData, std::vector<EntityHandle> &outEnts)
{
	auto &entityData = worldData.GetEntities();
	outEnts.reserve(entityData.size());

	std::unordered_map<uint32_t, EntityHandle> mapIndexToEntity;
	EntityIterator entIt {*this, EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::MapComponent>>();
	mapIndexToEntity.reserve(entIt.GetCount());
	for(auto *ent : entIt) {
		auto mapC = ent->GetComponent<pragma::MapComponent>();
		mapIndexToEntity[mapC->GetMapIndex()] = ent->GetHandle();
	}

	for(auto &entData : entityData) {
		if(entData->IsClientSideOnly()) {
			auto *ent = CreateMapEntity(*entData);
			if(ent)
				outEnts.push_back(ent->GetHandle());
			continue;
		}

		// Entity should already have been created by the server, look for it
		auto mapIdx = entData->GetMapIndex();
		auto it = mapIndexToEntity.find(mapIdx);
		if(it == mapIndexToEntity.end() || it->second.IsValid() == false)
			continue;
		auto &ent = *it->second.Get();
		outEnts.push_back(it->second);
		for(auto &[cType, cData] : entData->GetComponents()) {
			auto flags = cData->GetFlags();
			if(!umath::is_flag_set(flags, pragma::asset::ComponentData::Flags::ClientsideOnly))
				continue;
			CreateMapComponent(ent, cType, *cData);
		}
	}

	for(auto &hEnt : outEnts) {
		if(hEnt.valid() == false)
			continue;
		auto &ent = *hEnt.get();
		if(ent.IsWorld()) {
			auto pWorldComponent = ent.GetComponent<pragma::CWorldComponent>();
			if(pWorldComponent.valid()) {
				auto *bspTree = worldData.GetBSPTree();
				if(bspTree)
					pWorldComponent->SetBSPTree(bspTree->shared_from_this(), worldData.GetClusterMeshIndices());
			}
		}

		auto &mdl = ent.GetModel();
		if(mdl == nullptr) {
			auto pRenderComponent = static_cast<CBaseEntity &>(ent).GetRenderComponent();
			if(pRenderComponent) {
				Vector3 min {};
				Vector3 max {};
				auto pPhysComponent = ent.GetPhysicsComponent();
				if(pPhysComponent != nullptr)
					pPhysComponent->GetCollisionBounds(&min, &max);
				pRenderComponent->SetLocalRenderBounds(min, max);
			}
		}
	}
}

void CGame::InitializeWorldData(pragma::asset::WorldData &worldData)
{
	Game::InitializeWorldData(worldData);

	auto &texManager = static_cast<msys::CMaterialManager &>(static_cast<ClientState *>(GetNetworkState())->GetMaterialManager()).GetTextureManager();
	auto texture = texManager.LoadAsset(worldData.GetLightmapAtlasTexturePath(GetMapName()));
	if(texture) {
		prosper::util::SamplerCreateInfo samplerCreateInfo {};
		auto sampler = c_engine->GetRenderContext().CreateSampler(samplerCreateInfo);
		texture->GetVkTexture()->SetSampler(*sampler);

		auto &tex = *static_cast<Texture *>(texture.get());
		auto lightmapAtlas = tex.GetVkTexture();
		//auto lightmapAtlas = pragma::CLightMapComponent::CreateLightmapTexture(img->GetWidth(),img->GetHeight(),static_cast<uint16_t*>(img->GetData()));
		auto *scene = GetScene();
		auto *renderer = scene ? scene->GetRenderer() : nullptr;
		if(renderer != nullptr) {
			scene->GetSceneRenderDesc().ReloadOcclusionCullingHandler(); // Required if BSP occlusion culling is specified
			if(lightmapAtlas != nullptr) {
				auto *entWorld = c_game->GetWorld();
				auto lightMapC = entWorld ? entWorld->GetEntity().GetComponent<pragma::CLightMapComponent>() : pragma::ComponentHandle<pragma::CLightMapComponent> {};
				if(lightMapC.valid())
					pragma::CRasterizationRendererComponent::UpdateLightmap(*lightMapC);
			}
		}

		// Find map entities with lightmap uv sets
		EntityIterator entIt {*c_game, EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending};
		entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::MapComponent>>();
		for(auto *ent : entIt)
			pragma::CLightMapReceiverComponent::SetupLightMapUvData(static_cast<CBaseEntity &>(*ent));

		// Generate lightmap uv buffers for all entities
		if(worldData.IsLegacyLightMapEnabled()) {
			std::vector<std::shared_ptr<prosper::IBuffer>> buffers {};
			auto globalLightmapUvBuffer = pragma::CLightMapComponent::GenerateLightmapUVBuffers(buffers);

			auto *world = c_game->GetWorld();
			if(world && globalLightmapUvBuffer) {
				auto lightMapC = world->GetEntity().GetComponent<pragma::CLightMapComponent>();
				if(lightMapC.valid()) {
					// lightMapC->SetLightMapIntensity(worldData.GetLightMapIntensity());
					lightMapC->SetLightMapExposure(worldData.GetLightMapExposure());
					lightMapC->InitializeLightMapData(lightmapAtlas, globalLightmapUvBuffer, buffers);
					auto *scene = GetRenderScene();
					if(scene)
						scene->SetLightMap(*lightMapC);
				}
			}
		}
	}

	auto &materialManager = static_cast<msys::CMaterialManager &>(client->GetMaterialManager());
	materialManager.ReloadMaterialShaders();
}

bool CGame::LoadMap(const std::string &map, const Vector3 &origin, std::vector<EntityHandle> *entities)
{
	bool r = Game::LoadMap(map, origin, entities);
	m_flags |= GameFlags::MapLoaded;
	if(r == true) {
		CallCallbacks<void>("OnMapLoaded");
		CallLuaCallbacks<void>("OnMapLoaded");
		for(auto *gmC : GetGamemodeComponents())
			gmC->OnMapInitialized();
	}
	OnMapLoaded();

	std::string dsp = "fx_";
	dsp += map;
	dsp += ".udm";
	LoadAuxEffects(dsp.c_str());
	return r;
}

void CGame::BuildVMF(const char *)
{
	//Game::BuildVMF<CWorld,CPolyMesh,CPoly,CBrushMesh>(map);
}

static CVar cvTimescale = GetClientConVar("host_timescale");
float CGame::GetTimeScale() { return cvTimescale->GetFloat(); }

void CGame::SetTimeScale(float t)
{
	Game::SetTimeScale(t);
	for(auto &rsnd : client->GetSounds()) {
		auto &snd = static_cast<CALSound &>(rsnd.get());
		snd.SetPitchModifier(t); // TODO Implement SetPitchModifier for SoundScripts
	}
}

bool CGame::IsServer() { return false; }
bool CGame::IsClient() { return true; }

void CGame::SetLocalPlayer(pragma::CPlayerComponent *pl)
{
	m_plLocal = pl->GetHandle<pragma::CPlayerComponent>();
	pl->SetLocalPlayer(true);

	auto *cam = GetPrimaryCamera();
	if(cam) {
		auto observerC = cam->GetEntity().GetComponent<pragma::CObserverComponent>();
		auto observableC = pl->GetEntity().GetComponent<pragma::CObservableComponent>();
		if(observerC.valid() && observableC.valid())
			observerC->SetObserverTarget(observableC.get());
	}

	CallCallbacks<void, pragma::CPlayerComponent *>("OnLocalPlayerSpawned", pl);
	CallLuaCallbacks<void, luabind::object>("OnLocalPlayerSpawned", pl->GetLuaObject());
}

void CGame::OnReceivedPlayerInputResponse(uint8_t userInputId)
{
	if(m_userInputTracker.IsMessageInOrder(userInputId) == false)
		return;
	m_userInputTracker.CheckMessages(userInputId, m_lostPackets);
	//UpdateLatency(m_userInputTracker.messageTimestamps[userInputId]);
}

uint16_t CGame::GetLatency() const
{
	auto *cl = client->GetClient();
	if(cl == nullptr)
		return 0;
	return cl->GetLatency();
}

pragma::rendering::RenderMask CGame::GetInclusiveRenderMasks() const { return m_inclusiveRenderMasks; }
pragma::rendering::RenderMask CGame::GetExclusiveRenderMasks() const { return m_exclusiveRenderMasks; }
bool CGame::IsInclusiveRenderMask(pragma::rendering::RenderMask mask) const { return umath::is_flag_set(m_inclusiveRenderMasks, mask); }
bool CGame::IsExclusiveRenderMask(pragma::rendering::RenderMask mask) const { return umath::is_flag_set(m_exclusiveRenderMasks, mask); }
pragma::rendering::RenderMask CGame::RegisterRenderMask(const std::string &name, bool inclusiveByDefault)
{
	constexpr auto highestAllowedMask = (static_cast<uint64_t>(1) << (sizeof(uint64_t) * 8 - 1));
	if(umath::to_integral(m_nextCustomRenderMaskIndex) == highestAllowedMask)
		throw std::runtime_error {"Exceeded maximum allowed number of custom render masks!"};
	auto mask = GetRenderMask(name);
	if(mask.has_value())
		return *mask;
	auto id = m_nextCustomRenderMaskIndex;
	m_customRenderMasks.insert(std::make_pair(name, id));
	m_nextCustomRenderMaskIndex = static_cast<decltype(m_nextCustomRenderMaskIndex)>(umath::to_integral(m_nextCustomRenderMaskIndex) << 1);
	if(inclusiveByDefault)
		m_inclusiveRenderMasks |= id;
	else
		m_exclusiveRenderMasks |= id;
	return id;
}
std::optional<pragma::rendering::RenderMask> CGame::GetRenderMask(const std::string &name)
{
	auto it = m_customRenderMasks.find(name);
	return (it != m_customRenderMasks.end()) ? it->second : std::optional<pragma::rendering::RenderMask> {};
}
const std::string *CGame::FindRenderMaskName(pragma::rendering::RenderMask mask) const
{
	auto it = std::find_if(m_customRenderMasks.begin(), m_customRenderMasks.end(), [mask](const std::pair<std::string, pragma::rendering::RenderMask> &pair) { return pair.second == mask; });
	if(it == m_customRenderMasks.end())
		return nullptr;
	return &it->first;
}

void CGame::SendUserInput()
{
	auto *pl = GetLocalPlayer();
	if(pl == NULL)
		return;
	m_userInputTracker.messageTimestamps[m_userInputTracker.outMessageId] = RealTime();

	NetPacket p;
	p->Write<uint8_t>(m_userInputTracker.outMessageId++);
	//p->Write<uint8_t>(m_latency); // Deprecated: Let the networkmanager handle it
	auto &ent = pl->GetEntity();
	auto charComponent = ent.GetCharacterComponent();
	auto pTrComponent = ent.GetTransformComponent();
	auto orientation = charComponent.valid() ? charComponent->GetViewOrientation() : pTrComponent != nullptr ? pTrComponent->GetRotation() : uquat::identity();
	nwm::write_quat(p, orientation);
	p->Write<Vector3>(pl->GetViewPos());

	auto *actionInputC = pl->GetActionInputController();
	auto actions = actionInputC ? actionInputC->GetActionInputs() : Action::None;
	p->Write<Action>(actions);
	auto bControllers = c_engine->GetControllersEnabled();
	p->Write<bool>(bControllers);
	if(bControllers == true) {
		auto actionValues = umath::get_power_of_2_values(umath::to_integral(actions));
		for(auto v : actionValues) {
			auto magnitude = 0.f;
			if(actionInputC)
				actionInputC->GetActionInputAxisMagnitude(static_cast<Action>(v));
			p->Write<float>(magnitude);
		}
	}
	client->SendPacket("userinput", p, pragma::networking::Protocol::FastUnreliable);
}

double &CGame::ServerTime() { return m_tServer; }
void CGame::SetServerTime(double t) { m_tServer = t; }

bool CGame::RunLua(const std::string &lua) { return Game::RunLua(lua, "lua_run_cl"); }

void CGame::UpdateLostPackets()
{
	auto &t = RealTime();
	while(!m_lostPackets.empty() && t - m_lostPackets.front() > 1.0)
		m_lostPackets.erase(m_lostPackets.begin());
}

uint32_t CGame::GetLostPacketCount()
{
	UpdateLostPackets();
	return static_cast<uint32_t>(m_lostPackets.size());
}

#include <pragma/physics/controller.hpp>
void CGame::ReceiveSnapshot(NetPacket &packet)
{
	//Con::ccl<<"Received snapshot.."<<Con::endl;
	//auto tOld = m_tServer;
	auto latency = GetLatency() / 2.f; // Latency is entire roundtrip; We need the time for one way
	auto tActivated = (util::clock::to_int(util::clock::get_duration_since_start()) - packet.GetTimeActivated()) / 1'000'000.0;
	//Con::ccl<<"Snapshot delay: "<<+latency<<"+ "<<tActivated<<" = "<<(latency +tActivated)<<Con::endl;
	auto tDelta = static_cast<float>((latency + tActivated) / 1'000.0);

	auto snapshotId = packet->Read<uint8_t>();
	m_tServer = packet->Read<double>();

	auto &t = RealTime();
	m_snapshotTracker.messageTimestamps[snapshotId] = m_tServer;
	if(m_snapshotTracker.IsMessageInOrder(snapshotId) == false)
		return; // Old snapshot; Just skip it (We're already received a newer snapshot, this one's out of order)
	m_snapshotTracker.CheckMessages(snapshotId, m_lostPackets, t);

	//std::cout<<"Received snapshot with "<<(m_tServer -tOld)<<" time difference to last snapshot"<<std::endl;
	const auto maxCorrectionDistance = umath::pow2(10.f);
	unsigned int numEnts = packet->Read<unsigned int>();
	for(unsigned int i = 0; i < numEnts; i++) {
		CBaseEntity *ent = static_cast<CBaseEntity *>(nwm::read_entity(packet));
		Vector3 pos = nwm::read_vector(packet);
		Vector3 vel = nwm::read_vector(packet);
		Vector3 angVel = nwm::read_vector(packet);
		auto orientation = nwm::read_quat(packet);
		auto entDataSize = packet->Read<UInt8>();
		if(ent != NULL) {
			pos += vel * tDelta;
			if(uvec::length_sqr(angVel) > 0.0)
				orientation = uquat::create(EulerAngles(umath::rad_to_deg(angVel.x), umath::rad_to_deg(angVel.y), umath::rad_to_deg(angVel.z)) * tDelta) * orientation; // TODO: Check if this is correct

			// Move the entity to the correct position without teleporting it.
			// Teleporting can lead to odd physics glitches.
			auto pTrComponent = ent->GetTransformComponent();
			auto posEnt = pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3 {};
			auto correctionVel = pos - posEnt;
			auto l = uvec::length_sqr(correctionVel);
#ifdef ENABLE_DEPRECATED_PHYSICS
			// TODO: This should be enabled for bullet! -> Move into bullet module somehow?
			if(l > maxCorrectionDistance)
#endif
			{
				if(pTrComponent != nullptr)
					pTrComponent->SetPosition(pos); // Too far away, just snap into position
			}
#ifdef ENABLE_DEPRECATED_PHYSICS
			// TODO: This should be enabled for bullet! -> Move into bullet module somehow?
			else {
				auto *pPhysComponent = static_cast<pragma::CPhysicsComponent *>(ent->GetPhysicsComponent().get());
				if(pPhysComponent != nullptr)
					pPhysComponent->SetLinearCorrectionVelocity(pos - posEnt);
			}
#endif
			//
			auto pVelComponent = ent->GetComponent<pragma::VelocityComponent>();
			if(pVelComponent.valid()) {
				pVelComponent->SetVelocity(vel);
				pVelComponent->SetAngularVelocity(angVel);
			}
			if(pTrComponent != nullptr)
				pTrComponent->SetRotation(orientation);
			ent->ReceiveSnapshotData(packet);
		}
		else
			packet->SetOffset(packet->GetOffset() + entDataSize);

		auto flags = packet->Read<pragma::SnapshotFlags>();
		if((flags & pragma::SnapshotFlags::PhysicsData) != pragma::SnapshotFlags::None) {
			auto numObjs = packet->Read<uint8_t>();
			if(ent != NULL) {
				auto pPhysComponent = ent->GetPhysicsComponent();
				PhysObj *physObj = pPhysComponent != nullptr ? pPhysComponent->GetPhysicsObject() : nullptr;
				if(physObj != NULL && !physObj->IsStatic()) {
					auto colObjs = physObj->GetCollisionObjects();
					auto numActualObjs = colObjs.size();
					for(auto i = decltype(numObjs) {0}; i < numObjs; ++i) {
						auto pos = packet->Read<Vector3>();
						auto rot = packet->Read<Quat>();
						auto vel = packet->Read<Vector3>();
						auto angVel = packet->Read<Vector3>();
						if(physObj->IsController()) {
							auto *physController = static_cast<ControllerPhysObj *>(physObj);
							//physController->SetPosition(pos);
							physController->SetOrientation(rot);
							physController->SetLinearVelocity(vel);
							physController->SetAngularVelocity(angVel);
						}
						else if(i < numActualObjs) {
							auto &hObj = colObjs[i];
							if(hObj.IsValid()) {
								pos += vel * tDelta;
								auto l = uvec::length_sqr(angVel);
								if(l > 0.0)
									rot = uquat::create(EulerAngles(umath::rad_to_deg(angVel.x), umath::rad_to_deg(angVel.y), umath::rad_to_deg(angVel.z)) * tDelta) * rot; // TODO: Check if this is correct
								auto *o = hObj.Get();
								o->SetRotation(rot);
								if(o->IsRigid()) {
									auto *rigid = o->GetRigidBody();

									//auto correctionVel = pos -rigid->GetPos();
									//auto l = uvec::length_sqr(correctionVel);
									//if(l > maxCorrectionDistance)
									rigid->SetPos(pos); // Too far away, just snap into position
									//else
									//	rigid->SetLinearCorrectionVelocity(pos -ent->GetPosition());

									rigid->SetLinearVelocity(vel);
									rigid->SetAngularVelocity(angVel);
								}
								else
									o->SetPos(pos);
							}
						}
					}
				}
				else
					packet->SetOffset(packet->GetOffset() + numObjs * (sizeof(Vector3) * 3 + sizeof(Quat)));
			}
			else
				packet->SetOffset(packet->GetOffset() + numObjs * (sizeof(Vector3) * 3 + sizeof(Quat)));
		}

		if((flags & pragma::SnapshotFlags::ComponentData) != pragma::SnapshotFlags::None) {
			auto &componentManager = static_cast<pragma::CEntityComponentManager &>(GetEntityComponentManager());
			auto &svComponentToClComponentTable = componentManager.GetServerComponentIdToClientComponentIdTable();
			auto &componentTypes = componentManager.GetRegisteredComponentTypes();
			auto numComponents = packet->Read<uint8_t>();
			for(auto i = decltype(numComponents) {0}; i < numComponents; ++i) {
				auto svId = packet->Read<pragma::ComponentId>();
				auto componentSize = packet->Read<uint8_t>();
				auto componentEndOffset = packet->GetOffset() + componentSize;
				if(ent != nullptr && svId < svComponentToClComponentTable.size() && svComponentToClComponentTable.at(svId) != pragma::CEntityComponentManager::INVALID_COMPONENT) {
					auto clId = svComponentToClComponentTable.at(svId);
					if(clId >= componentTypes.size())
						throw std::runtime_error("Invalid client component type index " + std::to_string(clId) + "!");
					auto pComponent = ent->FindComponent(clId);
					if(pComponent.valid()) {
						auto *pSnapshotComponent = dynamic_cast<pragma::CBaseSnapshotComponent *>(pComponent.get());
						if(pSnapshotComponent != nullptr)
							pSnapshotComponent->ReceiveSnapshotData(packet);
					}
				}

				packet->SetOffset(componentEndOffset);
			}
		}
	}

	unsigned char numPlayers = packet->Read<unsigned char>();
	for(int i = 0; i < numPlayers; i++) {
		auto *plComponent = nwm::read_player(packet);
		auto *pl = (plComponent != nullptr) ? static_cast<CPlayer *>(plComponent->GetBasePlayer()) : nullptr;
		auto orientation = nwm::read_quat(packet);
		unsigned char numKeys = packet->Read<unsigned char>();
		for(int i = 0; i < numKeys; i++) {
			unsigned short key = packet->Read<unsigned short>();
			UNUSED(key);
			char pressed = packet->Read<char>();
			int action = (pressed == 1) ? GLFW_PRESS : GLFW_RELEASE;
			UNUSED(action);
		}
		/*if(ent != nullptr)
		{
			static Quat last(0.f,0.f,0.f,0.f);
			if(orientation.x != last.x || orientation.y != last.y || orientation.z != last.z || orientation.w != last.w)
			{
				last = orientation;
				std::cout<<"Orientation Update: ("<<orientation.w<<","<<orientation.x<<","<<orientation.y<<","<<orientation.z<<")"<<std::endl;
			}
		}*/
		if(plComponent != NULL && pl->IsCharacter()) {
			auto charComponent = pl->GetCharacterComponent();
			charComponent->SetViewOrientation(orientation);
		}
	}
}

static void set_action_input(Action action, bool b, bool bKeepMagnitude, const float *inMagnitude = nullptr)
{
	auto magnitude = 0.f;
	if(inMagnitude != nullptr)
		magnitude = *inMagnitude;
	else if(bKeepMagnitude == false)
		magnitude = (b == true) ? 1.f : 0.f;
	auto *pl = c_game->GetLocalPlayer();
	if(pl == nullptr)
		return;
	auto *actionInputC = pl->GetActionInputController();
	if(!actionInputC)
		return;
	if(bKeepMagnitude == false)
		actionInputC->SetActionInputAxisMagnitude(action, magnitude);
	if(b == false) {
		actionInputC->SetActionInput(action, b, true);
		return;
	}
	if(actionInputC->GetRawActionInput(action))
		return;
	actionInputC->SetActionInput(action, b, true);
}
void CGame::SetActionInput(Action action, bool b, bool bKeepMagnitude) { set_action_input(action, b, bKeepMagnitude); }
void CGame::SetActionInput(Action action, bool b, float magnitude) { set_action_input(action, b, false, &magnitude); }

bool CGame::GetActionInput(Action action)
{
	auto *pl = GetLocalPlayer();
	if(pl == NULL)
		return false;
	auto *actionInputC = pl->GetActionInputController();
	if(!actionInputC)
		return false;
	return actionInputC->GetActionInput(action);
}

void CGame::DrawLine(const Vector3 &start, const Vector3 &end, const Color &color, float duration) { DebugRenderer::DrawLine(start, end, {color, duration}); }
void CGame::DrawBox(const Vector3 &origin, const Vector3 &start, const Vector3 &end, const EulerAngles &ang, const Color &colorOutline, const std::optional<Color> &fillColor, float duration)
{
	DebugRenderInfo renderInfo {};
	renderInfo.SetOrigin(origin);
	renderInfo.SetRotation(uquat::create(ang));
	renderInfo.SetDuration(duration);
	if(fillColor) {
		renderInfo.SetColor(*fillColor);
		renderInfo.SetOutlineColor(colorOutline);
		DebugRenderer::DrawBox(start, end, renderInfo);
	}
	else {
		renderInfo.SetColor(colorOutline);
		DebugRenderer::DrawBox(start, end, renderInfo);
	}
}
void CGame::DrawPlane(const Vector3 &n, float dist, const Color &color, float duration) { DebugRenderer::DrawPlane(n, dist, {color, duration}); }
void CGame::DrawMesh(const std::vector<Vector3> &meshVerts, const Color &color, const Color &colorOutline, float duration) { DebugRenderer::DrawMesh(meshVerts, {color, colorOutline, duration}); }
static auto cvRenderPhysics = GetClientConVar("debug_physics_draw");
static auto cvSvRenderPhysics = GetClientConVar("sv_debug_physics_draw");
void CGame::RenderDebugPhysics(std::shared_ptr<prosper::ICommandBuffer> &drawCmd, pragma::CCameraComponent &cam)
{
	if(cvRenderPhysics->GetBool()) {
		auto *physEnv = GetPhysicsEnvironment();
		auto *pVisualDebugger = physEnv ? physEnv->GetVisualDebugger() : nullptr;
		if(pVisualDebugger)
			static_cast<CPhysVisualDebugger &>(*pVisualDebugger).Render(drawCmd, cam);
	}
	if(cvSvRenderPhysics->GetBool()) {
		// Serverside physics (singleplayer only)
		auto *svState = c_engine->GetServerNetworkState();
		auto *game = svState ? svState->GetGameState() : nullptr;
		auto *physEnv = game ? game->GetPhysicsEnvironment() : nullptr;
		auto *pVisualDebugger = physEnv ? physEnv->GetVisualDebugger() : nullptr;
		if(pVisualDebugger)
			static_cast<CPhysVisualDebugger &>(*pVisualDebugger).Render(drawCmd, cam);
	}
}

bool CGame::LoadAuxEffects(const std::string &fname)
{
	std::string path = "scripts/soundfx/";
	path += fname;

	std::string err;
	auto udmData = util::load_udm_asset(fname, &err);
	if(udmData == nullptr)
		return false;
	auto &data = *udmData;
	auto udm = data.GetAssetData().GetData();
	if(!udm)
		return false;
	for(auto pair : udm.ElIt()) {
		std::string name = std::string {pair.key};
		StringToLower(name);
		std::string type;
		pair.property["type"](type);
		al::create_aux_effect(name, type, pair.property);
	}
	return true;
}
std::shared_ptr<al::IEffect> CGame::GetAuxEffect(const std::string &name) { return c_engine->GetAuxEffect(name); }

bool CGame::SaveImage(prosper::IImage &image, const std::string &fileName, const uimg::TextureInfo &imageWriteInfo) const
{
	auto path = ufile::get_path_from_filename(fileName);
	FileManager::CreatePath(path.c_str());
	return prosper::util::save_texture(fileName, image, imageWriteInfo, [fileName](const std::string &err) { Con::cwar << "Unable to save image '" << fileName << "': " << err << Con::endl; });
}

bool CGame::SaveImage(const std::vector<std::vector<const void *>> &imgLayerMipmapData, uint32_t width, uint32_t height, uint32_t szPerPixel, const std::string &fileName, const uimg::TextureInfo &imageWriteInfo, bool cubemap) const
{
	auto path = ufile::get_path_from_filename(fileName);
	FileManager::CreatePath(path.c_str());
	uimg::TextureSaveInfo texSaveInfo;
	texSaveInfo.texInfo = imageWriteInfo;
	texSaveInfo.width = width;
	texSaveInfo.height = height;
	texSaveInfo.szPerPixel = szPerPixel;
	texSaveInfo.cubemap = cubemap;
	return uimg::save_texture(fileName, imgLayerMipmapData, texSaveInfo, [fileName](const std::string &err) { Con::cwar << "Unable to save image '" << fileName << "': " << err << Con::endl; });
}

bool CGame::SaveImage(uimg::ImageBuffer &imgBuffer, const std::string &fileName, const uimg::TextureInfo &imageWriteInfo, bool cubemap) const
{
	auto path = ufile::get_path_from_filename(fileName);
	filemanager::create_path(path);
	uimg::TextureSaveInfo texSaveInfo;
	texSaveInfo.texInfo = imageWriteInfo;
	texSaveInfo.cubemap = cubemap;
	return uimg::save_texture(fileName, imgBuffer, texSaveInfo, [fileName](const std::string &err) { Con::cwar << "Unable to save image '" << fileName << "': " << err << Con::endl; });
}

static CVar cvFriction = GetClientConVar("sv_friction");
Float CGame::GetFrictionScale() const { return cvFriction->GetFloat(); }
static CVar cvRestitution = GetClientConVar("sv_restitution");
Float CGame::GetRestitutionScale() const { return cvRestitution->GetFloat(); }
