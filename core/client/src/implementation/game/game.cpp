// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/console/helper.hpp"

//#include "shader_screen.h" // prosper TODO

module pragma.client;

import :game;
import :ai;
import :audio;
import :client_state;
import :debug;
import :engine;
import :entities;
import :entities.components;
import :gui;
import :model;
import :particle_system;
import :physics;
import :scripting.lua;

DLLCLIENT pragma::physics::IEnvironment *c_physEnv = nullptr;

pragma::CGame::MessagePacketTracker::MessagePacketTracker() : lastInMessageId(0), outMessageId(0) { std::fill(messageTimestamps.begin(), messageTimestamps.end(), 0); }

bool pragma::CGame::MessagePacketTracker::IsMessageInOrder(uint8_t messageId) const { return (messageTimestamps[messageId] > messageTimestamps[lastInMessageId]) ? true : false; }

void pragma::CGame::MessagePacketTracker::CheckMessages(uint8_t messageId, std::vector<double> &lostPacketTimestamps, const double &tCur)
{
	if(messageId != static_cast<decltype(messageId)>(lastInMessageId + 1)) // We've lost at least 1 packet
	{
		for(auto id = static_cast<decltype(messageId)>(lastInMessageId + 1); id != messageId; ++id) // Assume all messages inbetween are lost packets
			lostPacketTimestamps.push_back(tCur);
	}
	lastInMessageId = messageId;
}

void pragma::CGame::MessagePacketTracker::CheckMessages(uint8_t messageId, std::vector<double> &lostPacketTimestamps)
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
static auto cvWorkerThreadCount = pragma::console::get_client_con_var("render_queue_worker_thread_count");

static pragma::CGame *g_game = nullptr;
pragma::CGame *pragma::get_client_game() { return g_game; }
pragma::CGame *pragma::get_cgame() { return g_game; }

pragma::CGame::CGame(NetworkState *state)
    : Game(state), m_tServer(0), m_renderScene(pragma::util::TWeakSharedHandle<BaseEntityComponent> {}), m_matOverride(nullptr), m_colScale(1, 1, 1, 1),
      //m_shaderOverride(nullptr), // prosper TODO
      m_matLoad(), m_scene(nullptr),
      /*m_dummyVertexBuffer(nullptr),*/ m_tLastClientUpdate(0.0), // prosper TODO
      m_snapshotTracker {}, m_userInputTracker {}, m_viewFov {util::FloatProperty::Create(baseEnvCameraComponent::DEFAULT_VIEWMODEL_FOV)}, m_luaInputBindingLayerRegister {std::make_unique<LuaInputBindingLayerRegister>()}
{
	std::fill(m_renderModesEnabled.begin(), m_renderModesEnabled.end(), true);
	g_game = this;

	m_luaShaderManager = pragma::util::make_shared<LuaShaderManager>();
	m_luaParticleModifierManager = pragma::util::make_shared<pts::LuaParticleModifierManager>();

	math::set_flag(m_stateFlags, StateFlags::PrepassShaderPipelineReloadRequired, false);
	math::set_flag(m_stateFlags, StateFlags::GameWorldShaderPipelineReloadRequired, false);

	RegisterRenderMask("water", true);
	m_thirdPersonRenderMask = RegisterRenderMask("thirdperson", true);
	m_firstPersonRenderMask = RegisterRenderMask("firstperson", false);

	RegisterCallback<void, CGame *>("OnGameEnd");
	RegisterCallback<void, CLightDirectionalComponent *, CLightDirectionalComponent *>("OnEnvironmentLightSourceChanged");
	RegisterCallback<void, std::reference_wrapper<const rendering::DrawSceneInfo>>("Render");
	RegisterCallback<void, std::reference_wrapper<const rendering::DrawSceneInfo>>("PreRenderScenes");
	RegisterCallback<void, std::reference_wrapper<const rendering::DrawSceneInfo>>("UpdateRenderBuffers");
	RegisterCallback<void>("OnRenderScenes");
	RegisterCallbackWithOptionalReturn<bool, std::reference_wrapper<const rendering::DrawSceneInfo>>("DrawScene");
	RegisterCallback<void>("PostRenderScenes");
	RegisterCallback<void, std::reference_wrapper<const rendering::DrawSceneInfo>>("RenderPostProcessing");
	RegisterCallback<void, std::reference_wrapper<const rendering::DrawSceneInfo>>("OnPreRender");
	RegisterCallback<void, std::reference_wrapper<const rendering::DrawSceneInfo>, std::reference_wrapper<rendering::DepthStageRenderProcessor>>("RenderPrepass");
	RegisterCallback<void, std::reference_wrapper<const rendering::DrawSceneInfo>>("PreRenderScene");
	RegisterCallback<void, std::reference_wrapper<const rendering::DrawSceneInfo>>("PostRenderScene");
	RegisterCallback<void, CPlayerComponent *>("OnLocalPlayerSpawned");
	RegisterCallback<void, std::reference_wrapper<Vector3>, std::reference_wrapper<Quat>, std::reference_wrapper<Quat>>("CalcView");
	RegisterCallback<void, std::reference_wrapper<Vector3>, std::reference_wrapper<Quat>>("CalcViewOffset");
	RegisterCallback<void, std::reference_wrapper<const rendering::DrawSceneInfo>, std::reference_wrapper<std::shared_ptr<prosper::RenderTarget>>>("PreRender");
	RegisterCallback<void, std::reference_wrapper<const rendering::DrawSceneInfo>, std::reference_wrapper<std::shared_ptr<prosper::RenderTarget>>>("PostRender");
	RegisterCallback<void, ecs::CBaseEntity *>("UpdateEntityModel");
	RegisterCallback<void, gui::types::WIBase *, gui::types::WIBase *>("OnGUIFocusChanged");

	LoadAuxEffects("fx_generic.udm");
	for(auto &rsnd : get_client_state()->GetSounds()) {
		auto &snd = static_cast<audio::CALSound &>(rsnd.get());
		snd.SetPitchModifier(GetTimeScale()); // TODO Implement SetPitchModifier for SoundScripts
	}
	AddCallback("OnSoundCreated", FunctionCallback<void, audio::ALSound *>::Create([](audio::ALSound *snd) {
		auto *csnd = dynamic_cast<audio::CALSound *>(snd);
		if(csnd == nullptr)
			return;
		csnd->SetPitchModifier(get_cgame()->GetTimeScale());
	}));

	gui::WGUI::GetInstance().SetFocusCallback([this](gui::types::WIBase *oldFocus, gui::types::WIBase *newFocus) {
		CallCallbacks<void, gui::types::WIBase *, gui::types::WIBase *>("OnGUIFocusChanged", oldFocus, newFocus);

		auto *l = GetLuaState();
		if(l == nullptr)
			return;
		auto oOldFocus = oldFocus ? gui::WGUILuaInterface::GetLuaObject(l, *oldFocus) : luabind::object {};
		auto oNewFocus = newFocus ? gui::WGUILuaInterface::GetLuaObject(l, *newFocus) : luabind::object {};
		CallLuaCallbacks<void, luabind::object, luabind::object>("OnGUIFocusChanged", oOldFocus, oNewFocus);
	});

	gui::WGUI::GetInstance().SetUiMouseButtonCallback([this](gui::types::WIBase &el, platform::MouseButton mouseButton, platform::KeyState state, platform::Modifier mods) {
		CallCallbacks<void, gui::types::WIBase *, platform::MouseButton, platform::KeyState, platform::Modifier>("OnGUIMouseButtonEvent", &el, mouseButton, state, mods);

		auto *l = GetLuaState();
		if(l == nullptr)
			return;
		auto oEl = gui::WGUILuaInterface::GetLuaObject(l, el);
		CallLuaCallbacks<void, luabind::object, platform::MouseButton, platform::KeyState, platform::Modifier>("OnGUIMouseButtonEvent", oEl, mouseButton, state, mods);
	});
	gui::WGUI::GetInstance().SetUiKeyboardCallback([this](gui::types::WIBase &el, platform::Key key, int scanCode, platform::KeyState state, platform::Modifier mods) {
		CallCallbacks<void, gui::types::WIBase *, platform::Key, int, platform::KeyState, platform::Modifier>("OnGUIKeyboardEvent", &el, key, scanCode, state, mods);

		auto *l = GetLuaState();
		if(l == nullptr)
			return;
		auto oEl = gui::WGUILuaInterface::GetLuaObject(l, el);
		CallLuaCallbacks<void, luabind::object, platform::Key, int, platform::KeyState, platform::Modifier>("OnGUIKeyboardEvent", oEl, key, scanCode, state, mods);
	});
	gui::WGUI::GetInstance().SetUiCharCallback([this](gui::types::WIBase &el, unsigned int c) {
		CallCallbacks<void, gui::types::WIBase *, unsigned int>("OnGUICharEvent", &el, c);

		auto *l = GetLuaState();
		if(l == nullptr)
			return;
		auto oEl = gui::WGUILuaInterface::GetLuaObject(l, el);
		CallLuaCallbacks<void, luabind::object, unsigned int>("OnGUICharEvent", oEl, c);
	});
	gui::WGUI::GetInstance().SetUiScrollCallback([this](gui::types::WIBase &el, Vector2 offset) {
		CallCallbacks<void, gui::types::WIBase *, Vector2>("OnGUIScrollEvent", &el, offset);

		auto *l = GetLuaState();
		if(l == nullptr)
			return;
		auto oEl = gui::WGUILuaInterface::GetLuaObject(l, el);
		CallLuaCallbacks<void, luabind::object, Vector2>("OnGUIScrollEvent", oEl, offset);
	});

	m_cbGPUProfilingHandle = get_cengine()->AddGPUProfilingHandler([this](bool profilingEnabled) {
		if(profilingEnabled == false) {
			m_gpuProfilingStageManager = nullptr;
			return;
		}
		m_gpuProfilingStageManager = std::make_unique<debug::ProfilingStageManager<debug::GPUProfilingStage>>();
		auto &gpuProfiler = get_cengine()->GetGPUProfiler();
		m_gpuProfilingStageManager->InitializeProfilingStageManager(gpuProfiler);
	});
	m_cbProfilingHandle = get_cengine()->AddProfilingHandler([this](bool profilingEnabled) {
		if(profilingEnabled == false) {
			m_profilingStageManager = nullptr;
			return;
		}
		auto &cpuProfiler = get_cengine()->GetProfiler();
		m_profilingStageManager = std::make_unique<debug::ProfilingStageManager<debug::ProfilingStage>>();
		m_profilingStageManager->InitializeProfilingStageManager(cpuProfiler);
	});

	m_renderQueueBuilder = std::make_unique<rendering::RenderQueueBuilder>();
	m_renderQueueWorkerManager = std::make_unique<rendering::RenderQueueWorkerManager>(math::clamp(cvWorkerThreadCount->GetInt(), 1, 20));

	m_globalShaderInputDataManager = std::make_unique<rendering::GlobalShaderInputDataManager>();

	auto &texManager = static_cast<material::CMaterialManager &>(static_cast<ClientState *>(GetNetworkState())->GetMaterialManager()).GetTextureManager();
	for(auto &tex : g_requiredGameTextures) {
		texManager.LoadAsset(tex); // Pre-loaded in ClientState constructor
		texManager.FlagAssetAsAlwaysInUse(tex, true);
	}
}

pragma::CGame::~CGame() { g_game = nullptr; }

void pragma::CGame::OnRemove()
{
	m_flags |= GameFlags::ClosingGame;
	m_renderQueueWorkerManager = nullptr;
	m_renderQueueBuilder = nullptr;
	get_cengine()->GetRenderContext().WaitIdle();
	gui::WGUI::GetInstance().SetFocusCallback(nullptr);
	gui::WGUI::GetInstance().SetUiMouseButtonCallback(nullptr);
	gui::WGUI::GetInstance().SetUiKeyboardCallback(nullptr);
	gui::WGUI::GetInstance().SetUiCharCallback(nullptr);
	gui::WGUI::GetInstance().SetUiScrollCallback(nullptr);
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

	debug::DebugRenderer::ClearObjects();

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

	for(auto &layer : get_cengine()->GetInputBindingLayers())
		layer->ClearLuaKeyMappings();
	const_cast<InputBindingLayer &>(get_cengine()->GetEffectiveInputBindingLayer()).ClearLuaKeyMappings();

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
	reset_lua_shaders();

	ClearSoundCache();

	Game::OnRemove();
}

pragma::rendering::GlobalShaderInputDataManager &pragma::CGame::GetGlobalShaderInputDataManager() { return *m_globalShaderInputDataManager; }
const pragma::rendering::GlobalShaderInputDataManager &pragma::CGame::GetGlobalShaderInputDataManager() const { return const_cast<CGame *>(this)->GetGlobalShaderInputDataManager(); }

void pragma::CGame::GetRegisteredEntities(std::vector<std::string> &classes, std::vector<std::string> &luaClasses) const
{
	client_entities::ClientEntityRegistry::Instance().GetRegisteredClassNames(classes);
	GetLuaRegisteredEntities(luaClasses);
}

void pragma::CGame::OnGameWorldShaderSettingsChanged(const rendering::GameWorldShaderSettings &newSettings, const rendering::GameWorldShaderSettings &oldSettings)
{
	if(newSettings.fxaaEnabled != oldSettings.fxaaEnabled || newSettings.bloomEnabled != oldSettings.bloomEnabled) {
		auto shader = get_cengine()->GetShaderManager().GetShader("pp_hdr");
		if(shader.valid())
			shader->ReloadPipelines();
	}
	if(newSettings.ssaoEnabled != oldSettings.ssaoEnabled)
		ReloadPrepassShaderPipelines();
	if(newSettings.shadowQuality != oldSettings.shadowQuality || newSettings.ssaoEnabled != oldSettings.ssaoEnabled || newSettings.bloomEnabled != oldSettings.bloomEnabled || newSettings.debugModeEnabled != oldSettings.debugModeEnabled || newSettings.iblEnabled != oldSettings.iblEnabled
	  || newSettings.dynamicLightingEnabled != oldSettings.dynamicLightingEnabled || newSettings.dynamicShadowsEnabled != oldSettings.dynamicShadowsEnabled)
		ReloadGameWorldShaderPipelines();
	if(newSettings.dynamicLightingEnabled != oldSettings.dynamicLightingEnabled) {
		auto shader = get_cengine()->GetShaderManager().GetShader("forwardp_light_culling");
		if(shader.valid())
			shader->ReloadPipelines();
	}
}

static void cmd_render_ibl_enabled(pragma::NetworkState *, const pragma::console::ConVar &, bool, bool enabled)
{
	auto *client = pragma::get_client_state();
	if(client == nullptr)
		return;
	client->UpdateGameWorldShaderSettings();
}
namespace {
	auto UVN = pragma::console::client::register_variable_listener<bool>("renderender_ibl_enabledr_bloom_enabled", &cmd_render_ibl_enabled);
}
namespace {
	auto UVN = pragma::console::client::register_variable_listener<bool>("render_dynamic_lighting_enabled", &cmd_render_ibl_enabled);
}
namespace {
	auto UVN = pragma::console::client::register_variable_listener<bool>("render_dynamic_shadows_enabled", &cmd_render_ibl_enabled);
}

static void cmd_render_queue_worker_thread_count(pragma::NetworkState *, const pragma::console::ConVar &, int, int val)
{
	if(pragma::get_cgame() == nullptr)
		return;
	val = pragma::math::clamp(val, 1, 20);
	pragma::get_cgame()->GetRenderQueueWorkerManager().SetWorkerCount(val);
}
namespace {
	auto UVN = pragma::console::client::register_variable_listener<int>("render_queue_worker_thread_count", &cmd_render_queue_worker_thread_count);
}

static void cmd_render_queue_worker_jobs_per_batch(pragma::NetworkState *, const pragma::console::ConVar &, int, int val)
{
	if(pragma::get_cgame() == nullptr)
		return;
	val = pragma::math::max(val, 1);
	pragma::get_cgame()->GetRenderQueueWorkerManager().SetJobsPerBatchCount(val);
}
namespace {
	auto UVN = pragma::console::client::register_variable_listener<int>("render_queue_worker_jobs_per_batch", &cmd_render_queue_worker_jobs_per_batch);
}

pragma::rendering::RenderQueueBuilder &pragma::CGame::GetRenderQueueBuilder() { return *m_renderQueueBuilder; }
pragma::rendering::RenderQueueWorkerManager &pragma::CGame::GetRenderQueueWorkerManager() { return *m_renderQueueWorkerManager; }

void pragma::CGame::UpdateTime()
{
	// TODO: This also has to be applied serverside?
	auto dt = get_cengine()->GetDeltaFrameTime();
	float timeScale = GetTimeScale();
	m_ctCur.UpdateByDelta(dt * timeScale);
	m_ctReal.UpdateByDelta(dt);
	m_tCur = CDouble(m_ctCur());
	m_tReal = CDouble(m_ctReal());
	m_tDelta = CDouble(m_tCur - m_tLast);
	m_tDeltaReal = CDouble(m_tReal - m_tLastReal);
}

bool pragma::CGame::StartGPUProfilingStage(const char *stage) { return m_gpuProfilingStageManager && m_gpuProfilingStageManager->StartProfilerStage(stage); }
bool pragma::CGame::StopGPUProfilingStage() { return m_gpuProfilingStageManager && m_gpuProfilingStageManager->StopProfilerStage(); }
pragma::debug::ProfilingStageManager<pragma::debug::GPUProfilingStage> *pragma::CGame::GetGPUProfilingStageManager() { return m_gpuProfilingStageManager.get(); }

pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage> *pragma::CGame::GetProfilingStageManager() { return m_profilingStageManager.get(); }
bool pragma::CGame::StartProfilingStage(const char *stage) { return m_profilingStageManager && m_profilingStageManager->StartProfilerStage(stage); }
bool pragma::CGame::StopProfilingStage() { return m_profilingStageManager && m_profilingStageManager->StopProfilerStage(); }

std::shared_ptr<pragma::EntityComponentManager> pragma::CGame::InitializeEntityComponentManager() { return pragma::util::make_shared<CEntityComponentManager>(); }

void pragma::CGame::OnReceivedRegisterNetEvent(NetPacket &packet)
{
	auto name = packet->ReadString();
	auto localId = SetupNetEvent(name);
	auto sharedId = packet->Read<NetEventId>();
	if(sharedId >= m_clientNetEventData.sharedNetEventIdToLocalId.size()) {
		if(m_clientNetEventData.sharedNetEventIdToLocalId.size() == m_clientNetEventData.sharedNetEventIdToLocalId.capacity())
			m_clientNetEventData.sharedNetEventIdToLocalId.reserve(m_clientNetEventData.sharedNetEventIdToLocalId.size() * 1.1f + 100);
		m_clientNetEventData.sharedNetEventIdToLocalId.resize(sharedId + 1, std::numeric_limits<NetEventId>::max());
	}
	m_clientNetEventData.sharedNetEventIdToLocalId[sharedId] = localId;
}

pragma::NetEventId pragma::CGame::SharedNetEventIdToLocal(NetEventId evId) const { return (evId < m_clientNetEventData.sharedNetEventIdToLocalId.size()) ? m_clientNetEventData.sharedNetEventIdToLocalId[evId] : std::numeric_limits<NetEventId>::max(); }

pragma::NetEventId pragma::CGame::LocalNetEventIdToShared(NetEventId evId) const { return (evId < m_clientNetEventData.localNetEventIdToSharedId.size()) ? m_clientNetEventData.localNetEventIdToSharedId[evId] : std::numeric_limits<NetEventId>::max(); }

pragma::NetEventId pragma::CGame::FindNetEvent(const std::string &name) const
{
	auto it = m_clientNetEventData.localNetEventIds.find(name);
	if(it == m_clientNetEventData.localNetEventIds.end())
		return std::numeric_limits<NetEventId>::max();
	return it->second;
}

pragma::NetEventId pragma::CGame::SetupNetEvent(const std::string &name)
{
	auto it = m_clientNetEventData.localNetEventIds.find(name);
	if(it != m_clientNetEventData.localNetEventIds.end())
		return it->second;
	m_clientNetEventData.localNetEventIds.insert(std::make_pair(name, m_clientNetEventData.nextLocalNetEventId++));
	return m_clientNetEventData.nextLocalNetEventId - 1;
}

std::shared_ptr<pragma::nav::Mesh> pragma::CGame::LoadNavMesh(const std::string &fname) { return nav::CMesh::Load(*this, fname); }

pragma::rendering::WorldEnvironment &pragma::CGame::GetWorldEnvironment() { return *m_worldEnvironment; }
const pragma::rendering::WorldEnvironment &pragma::CGame::GetWorldEnvironment() const { return const_cast<CGame *>(this)->GetWorldEnvironment(); }

void pragma::CGame::InitializeWorldEnvironment() { m_worldEnvironment = rendering::WorldEnvironment::Create(); }

void pragma::CGame::SetRenderClipPlane(const Vector4 &clipPlane) { m_clipPlane = clipPlane; }
const Vector4 &pragma::CGame::GetRenderClipPlane() const { return m_clipPlane; }

template<typename TCPPM>
TCPPM *pragma::CGame::CreateParticleTracer(const Vector3 &start, const Vector3 &end, float radius, const Color &col, float length, float speed, const std::string &material, float bloomScale)
{
	std::stringstream ssColor;
	ssColor << col.r << " " << col.g << " " << col.b << " " << col.a;
	std::unordered_map<std::string, std::string> values {{"maxparticles", "1"}, {"max_node_count", "2"}, {"emission_rate", "10000"}, {"material", material}, {"radius", std::to_string(radius)}, {"color", ssColor.str()}, {"sort_particles", "0"}, {"bloom_scale", std::to_string(bloomScale)}};
	auto *particle = ecs::CParticleSystemComponent::Create(values, nullptr);
	if(particle == nullptr)
		return nullptr;
	std::unordered_map<std::string, std::string> beamValues {{"node_start", "1"}, {"node_end", "2"}, {"curvature", "0.0"}};
	particle->AddRenderer("beam", beamValues);
	auto hParticle = particle->GetHandle<ecs::CParticleSystemComponent>();

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
		auto &tDelta = get_cgame()->DeltaTime();
		cStart += dir * static_cast<float>(speed * tDelta);
		auto dist = uvec::distance(cStart, end);
		length = math::min(static_cast<float>(length), dist);
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
	get_cgame()->AddCallback("Think", cb);
	return reinterpret_cast<TCPPM *>(particle);
}
template DLLCLIENT pragma::ecs::CParticleSystemComponent *pragma::CGame::CreateParticleTracer(const Vector3 &start, const Vector3 &end, float radius, const Color &col, float length, float speed, const std::string &material, float bloomScale);

void pragma::CGame::SetRenderModeEnabled(rendering::SceneRenderPass renderMode, bool bEnabled) { m_renderModesEnabled[math::to_integral(renderMode)] = bEnabled; }
void pragma::CGame::EnableRenderMode(rendering::SceneRenderPass renderMode) { SetRenderModeEnabled(renderMode, true); }
void pragma::CGame::DisableRenderMode(rendering::SceneRenderPass renderMode) { SetRenderModeEnabled(renderMode, false); }
bool pragma::CGame::IsRenderModeEnabled(rendering::SceneRenderPass renderMode) const { return m_renderModesEnabled[math::to_integral(renderMode)]; }

void pragma::CGame::InitializeLuaScriptWatcher() { m_scriptWatcher = std::make_unique<CLuaDirectoryWatcherManager>(this); }

pragma::material::Material *pragma::CGame::GetLoadMaterial() { return m_matLoad.get(); }
void pragma::CGame::OnEntityCreated(ecs::BaseEntity *ent)
{
	Game::OnEntityCreated(ent);
	if(typeid(*ent) == typeid(CGameEntity)) {
		m_entGame = ent->GetHandle();
		m_gameComponent = ent->GetComponent<CGameComponent>()->GetHandle();
	}
}

template<typename TCPPM>
const TCPPM *pragma::CGame::GetGameComponent() const
{
	return const_cast<CGame *>(this)->GetGameComponent<TCPPM>();
}
template DLLCLIENT const pragma::CGameComponent *pragma::CGame::GetGameComponent() const;

template<typename TCPPM>
TCPPM *pragma::CGame::GetGameComponent()
{
	return static_cast<TCPPM *>(m_gameComponent.get());
}
template DLLCLIENT pragma::CGameComponent *pragma::CGame::GetGameComponent();

template<typename TCPPM>
TCPPM *pragma::CGame::GetViewModel()
{
	if(m_viewModel.expired())
		return nullptr;
	return static_cast<TCPPM *>(m_viewModel.get());
}
template DLLCLIENT pragma::CViewModelComponent *pragma::CGame::GetViewModel<pragma::CViewModelComponent>();

template<typename TCPPM>
TCPPM *pragma::CGame::GetViewBody()
{
	if(m_viewBody.expired())
		return nullptr;
	return static_cast<TCPPM *>(m_viewBody.get());
}
template DLLCLIENT pragma::CViewBodyComponent *pragma::CGame::GetViewBody<pragma::CViewBodyComponent>();

static void shader_handler(pragma::material::Material *mat)
{
	if(mat->IsLoaded() == false)
		return;
	auto &shaderManager = pragma::get_cengine()->GetShaderManager();
	auto whShader = shaderManager.GetShader(mat->GetShaderIdentifier());
	if(whShader.expired())
		return;
	auto &cmat = *static_cast<pragma::material::CMaterial *>(mat);
	// Clear descriptor set in case the shader has changed; The new shader will re-create it appropriately
	cmat.SetDescriptorSetGroup(*whShader.get(), nullptr);

	auto *shader = dynamic_cast<pragma::ShaderTexturedBase *>(whShader.get());
	auto *pShader = dynamic_cast<prosper::Shader *>(shader);
	if(pShader == nullptr || pShader->IsValid() == false)
		return;
	mat->SetUserData(shader);
	shader->InitializeMaterialDescriptorSet(cmat, false);

	// TODO: Cache this
	auto *prepass = dynamic_cast<pragma::ShaderTexturedBase *>(pragma::get_cengine()->GetShader("prepass").get());
	if(prepass)
		prepass->InitializeMaterialDescriptorSet(cmat, false); // TODO: Only if this is a material with masked transparency?
}

void pragma::CGame::ReloadMaterialShader(material::CMaterial *mat)
{
	auto *shader = static_cast<ShaderTexturedBase *>(mat->GetUserData());
	if(shader == nullptr)
		return;
	shader->InitializeMaterialDescriptorSet(*mat, true);
}

void pragma::CGame::Initialize()
{
	Game::Initialize();
	auto *client = get_client_state();
	auto &materialManager = static_cast<material::CMaterialManager &>(client->GetMaterialManager());
	materialManager.SetShaderHandler(&shader_handler);
	CRenderComponent::InitializeBuffers();
	CLightComponent::InitializeBuffers();
	geometry::CModelSubMesh::InitializeBuffers();
	ecs::CParticleSystemComponent::InitializeBuffers();

	InitShaders();

	ecs::CParticleSystemComponent::Precache("impact");
	ecs::CParticleSystemComponent::Precache("muzzleflash");
	ecs::CParticleSystemComponent::Precache("explosion");

	// Initialize Scene (Has to be initialized AFTER shaders!)

	InitializeWorldEnvironment();
	auto *mat = client->LoadMaterial("loading", CallbackHandle {}, false, true);
	m_matLoad = mat ? mat->GetHandle() : nullptr;
}

static void render_debug_mode(pragma::NetworkState *, const pragma::console::ConVar &, int32_t, int32_t debugMode)
{
	auto *client = pragma::get_client_state();
	if(client == nullptr)
		return;
	client->UpdateGameWorldShaderSettings();
	pragma::ecs::EntityIterator entIt {*pragma::get_cgame(), pragma::ecs::EntityIterator::FilterFlags::Default | pragma::ecs::EntityIterator::FilterFlags::Pending};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CSceneComponent>>();
	for(auto *ent : entIt) {
		auto sceneC = ent->GetComponent<pragma::CSceneComponent>();
		sceneC->SetDebugMode(static_cast<pragma::SceneDebugMode>(debugMode));
	}
}
namespace {
	auto UVN = pragma::console::client::register_variable_listener<int32_t>("render_debug_mode", &render_debug_mode);
}

static void CVAR_CALLBACK_render_unlit(pragma::NetworkState *nw, const pragma::console::ConVar &cv, bool prev, bool val) { render_debug_mode(nw, cv, prev, pragma::math::to_integral(pragma::SceneDebugMode::Unlit)); }
namespace {
	auto UVN = pragma::console::client::register_variable_listener<bool>("render_unlit", &CVAR_CALLBACK_render_unlit);
}

void pragma::CGame::SetViewModelFOV(float fov) { *m_viewFov = fov; }
const pragma::util::PFloatProperty &pragma::CGame::GetViewModelFOVProperty() const { return m_viewFov; }
float pragma::CGame::GetViewModelFOV() const { return *m_viewFov; }
float pragma::CGame::GetViewModelFOVRad() const { return math::deg_to_rad(*m_viewFov); }
Mat4 pragma::CGame::GetViewModelProjectionMatrix() const
{
	auto *cam = GetPrimaryCamera<CCameraComponent>();
	auto aspectRatio = cam ? cam->GetAspectRatio() : 1.f;
	auto nearZ = cam ? cam->GetNearZ() : baseEnvCameraComponent::DEFAULT_NEAR_Z;
	auto farZ = cam ? cam->GetFarZ() : baseEnvCameraComponent::DEFAULT_FAR_Z;
	return BaseEnvCameraComponent::CalcProjectionMatrix(*m_viewFov, aspectRatio, nearZ, farZ);
}

template<typename TCPPM>
TCPPM *pragma::CGame::CreateCamera(float aspectRatio, float fov, float nearZ, float farZ)
{
	auto *cam = CreateEntity<CEnvCamera>();
	auto whCamComponent = cam ? cam->GetComponent<CCameraComponent>() : pragma::ComponentHandle<CCameraComponent> {};
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

template<typename TCPPM>
TCPPM *pragma::CGame::CreateCamera(uint32_t width, uint32_t height, float fov, float nearZ, float farZ)
{
	return CreateCamera<TCPPM>(width / static_cast<float>(height), fov, nearZ, farZ);
}
template DLLCLIENT pragma::CCameraComponent *pragma::CGame::CreateCamera(uint32_t width, uint32_t height, float fov, float nearZ, float farZ);

void pragma::CGame::InitializeGame() // Called by NET_cl_resourcecomplete
{
	Game::InitializeGame();
	SetupLua();

	m_hCbDrawFrame = get_cengine()->AddCallback("DrawFrame", FunctionCallback<void, std::reference_wrapper<std::shared_ptr<prosper::IPrimaryCommandBuffer>>>::Create([this](std::reference_wrapper<std::shared_ptr<prosper::IPrimaryCommandBuffer>> drawCmd) {
		auto baseDrawCmd = std::static_pointer_cast<prosper::ICommandBuffer>(drawCmd.get());
		CallLuaCallbacks<void, std::shared_ptr<prosper::ICommandBuffer>>("DrawFrame", baseDrawCmd);
	}));

	auto *client = get_client_state();
	auto &materialManager = static_cast<material::CMaterialManager &>(client->GetMaterialManager());
	if(m_surfaceMaterialManager)
		m_surfaceMaterialManager->Load("scripts/physics/materials.udm");

	auto resolution = get_cengine()->GetRenderResolution();
	get_cengine()->GetRenderContext().GetPipelineLoader().Flush();
	get_cengine()->GetRenderContext().SavePipelineCache();

	m_globalRenderSettingsBufferData = std::make_unique<rendering::GlobalRenderSettingsBufferData>();
	auto *scene = CSceneComponent::Create(CSceneComponent::CreateInfo {});
	if(!scene)
		throw std::runtime_error {"Failed to create scene."};
	scene->GetEntity().SetName("scene_game");
	m_scene = scene->GetHandle();
	scene->SetDebugMode(static_cast<SceneDebugMode>(GetConVarInt("render_debug_mode")));
	SetViewModelFOV(GetConVarFloat("cl_fov_viewmodel"));

	auto *entRenderer = CreateEntity<CRasterizationRenderer>();
	if(entRenderer) {
		auto rasterization = entRenderer->GetComponent<CRasterizationRendererComponent>();
		if(rasterization.valid()) {
			auto *renderer = rasterization->GetRendererComponent<CRendererComponent>();
			if(renderer) {
				scene->SetRenderer(renderer);
				rasterization->SetSSAOEnabled(GetConVarBool("cl_render_ssao"));
				scene->ReloadRenderTarget(static_cast<uint32_t>(resolution.x), static_cast<uint32_t>(resolution.y));
				scene->SetWorldEnvironment(GetWorldEnvironment());
			}
		}
		entRenderer->Spawn();
	}

	SetRenderScene(*scene);

	Resize(false);

	auto *cam = CreateCamera<CCameraComponent>(scene->GetWidth(), scene->GetHeight(), GetConVarFloat("cl_render_fov"), get_cengine()->GetNearZ(), get_cengine()->GetFarZ());
	if(cam) {
		auto toggleC = cam->GetEntity().GetComponent<CToggleComponent>();
		if(toggleC.valid())
			toggleC->TurnOn();
		scene->SetActiveCamera(*cam);
		m_primaryCamera = cam->GetHandle();

		cam->GetEntity().AddComponent<CObserverComponent>();
	}

	m_flags |= GameFlags::GameInitialized;
	CallCallbacks<void, Game *>("OnGameInitialized", this);
	for(auto *gmC : GetGamemodeComponents())
		gmC->OnGameInitialized();
}

void pragma::CGame::RequestResource(const std::string &fileName)
{
	Con::CCL << "[CGame] RequestResource '" << fileName << "'" << Con::endl;
	auto fName = fs::get_canonicalized_path(fileName);
	auto it = std::find(m_requestedResources.begin(), m_requestedResources.end(), fName);
	if(it != m_requestedResources.end())
		return;
	m_requestedResources.push_back(fName);
	NetPacket p;
	p->WriteString(fName);
	get_client_state()->SendPacket(networking::net_messages::server::QUERY_RESOURCE, p, networking::Protocol::SlowReliable);
	Con::CCL << "[CGame] Request sent!" << Con::endl;
}

void pragma::CGame::Resize(bool reloadRenderTarget)
{
	if(reloadRenderTarget)
		ReloadRenderFrameBuffer();
	auto *cam = GetRenderCamera<CCameraComponent>();
	if(cam != nullptr) {
		cam->SetAspectRatio(get_cengine()->GetWindow().GetAspectRatio());
		cam->UpdateMatrices();
	}

	CallLuaCallbacks("OnRenderTargetResized");
}

void pragma::CGame::PreGUIDraw() { CallLuaCallbacks<void>("PreGUIDraw"); }
void pragma::CGame::PostGUIDraw() { CallLuaCallbacks<void>("PostGUIDraw"); }
void pragma::CGame::PreGUIRecord() { CallLuaCallbacks<void>("PreGUIRecord"); }
void pragma::CGame::PostGUIRecord() { CallLuaCallbacks<void>("PostGUIRecord"); }
void pragma::CGame::SetDefaultGameRenderEnabled(bool enabled) { m_defaultGameRenderEnabled = enabled; }
bool pragma::CGame::IsDefaultGameRenderEnabled() const { return m_defaultGameRenderEnabled; }
uint32_t pragma::CGame::GetNumberOfScenesQueuedForRendering() const { return m_sceneRenderQueue.size(); }
pragma::rendering::DrawSceneInfo *pragma::CGame::GetQueuedSceneRenderInfo(uint32_t i) { return (i < m_sceneRenderQueue.size()) ? &m_sceneRenderQueue[i] : nullptr; }
void pragma::CGame::QueueForRendering(const rendering::DrawSceneInfo &drawSceneInfo) { m_sceneRenderQueue.push_back(drawSceneInfo); }
const std::vector<pragma::rendering::DrawSceneInfo> &pragma::CGame::GetQueuedRenderScenes() const { return m_sceneRenderQueue; }
template<typename TCPPM>
void pragma::CGame::SetRenderScene(TCPPM &scene)
{
	m_renderScene = scene.GetHandle();
}
template DLLCLIENT void pragma::CGame::SetRenderScene(CSceneComponent &scene);
void pragma::CGame::ResetRenderScene() { m_renderScene = m_scene; }
template<typename TCPPM>
TCPPM *pragma::CGame::GetRenderScene()
{
	return static_cast<CSceneComponent *>(m_renderScene.get());
}
template<typename TCPPM>
const TCPPM *pragma::CGame::GetRenderScene() const
{
	return const_cast<CGame *>(this)->GetRenderScene<CSceneComponent>();
}
template DLLCLIENT pragma::CSceneComponent *pragma::CGame::GetRenderScene();
template DLLCLIENT const pragma::CSceneComponent *pragma::CGame::GetRenderScene() const;
template<typename TCPPM>
TCPPM *pragma::CGame::GetRenderCamera() const
{
	if(m_renderScene.expired())
		return nullptr;
	return const_cast<CCameraComponent *>(static_cast<const CSceneComponent *>(m_renderScene.get())->GetActiveCamera().get());
}
template DLLCLIENT pragma::CCameraComponent *pragma::CGame::GetRenderCamera() const;
template<typename TCPPM>
void pragma::CGame::SetGameplayControlCamera(TCPPM &cam)
{
	m_controlCamera = cam.template GetHandle<BaseEntityComponent>();
	m_stateFlags &= ~StateFlags::DisableGamplayControlCamera;
}
template DLLCLIENT void pragma::CGame::SetGameplayControlCamera(CCameraComponent &);
void pragma::CGame::ResetGameplayControlCamera()
{
	m_controlCamera = pragma::ComponentHandle<BaseEntityComponent> {};
	m_stateFlags &= ~StateFlags::DisableGamplayControlCamera;
}
void pragma::CGame::ClearGameplayControlCamera()
{
	m_controlCamera = pragma::ComponentHandle<BaseEntityComponent> {};
	m_stateFlags |= StateFlags::DisableGamplayControlCamera;
}
template<typename TCPPM>
TCPPM *pragma::CGame::GetGameplayControlCamera()
{
	if(m_controlCamera.valid())
		return static_cast<CCameraComponent *>(m_controlCamera.get());
	if(math::is_flag_set(m_stateFlags, StateFlags::DisableGamplayControlCamera))
		return nullptr;
	return GetRenderCamera<TCPPM>();
}
template DLLCLIENT pragma::CCameraComponent *pragma::CGame::GetGameplayControlCamera();
template<typename TCPPM>
TCPPM *pragma::CGame::GetPrimaryCamera() const
{
	return const_cast<CCameraComponent *>(static_cast<const CCameraComponent *>(m_primaryCamera.get()));
}
template DLLCLIENT pragma::CCameraComponent *pragma::CGame::GetPrimaryCamera<pragma::CCameraComponent>() const;

void pragma::CGame::SetMaterialOverride(material::Material *mat) { m_matOverride = mat; }
pragma::material::Material *pragma::CGame::GetMaterialOverride() { return m_matOverride; }

void pragma::CGame::SetColorScale(const Vector4 &col) { m_colScale = col; }
Vector4 &pragma::CGame::GetColorScale() { return m_colScale; }
void pragma::CGame::SetAlphaScale(float a) { m_colScale.a = a; }
float pragma::CGame::GetAlphaScale() { return m_colScale.a; }

pragma::gui::types::WIBase *pragma::CGame::CreateGUIElement(std::string className, gui::types::WIBase *parent)
{
	auto *o = m_luaGUIElements.GetClassObject(className);
	auto &gui = gui::WGUI::GetInstance();
	gui::types::WIBase *el = nullptr;
	if(o != nullptr) {
		luabind::object r;
#ifndef LUABIND_NO_EXCEPTIONS
		try {
#endif
			// Object is created through Luabind using 'new'
			// but we steal it with our WILuaBaseHolder holder so it can get destroyed through
			// the WGUI library instead.
			r = (*o)();

			auto *elLua = luabind::object_cast<gui::types::WILuaBase *>(r);
			auto *holder = luabind::object_cast<LuaCore::WILuaBaseHolder *>(r);
			if(elLua && holder) {
				string::to_lower(className);
				elLua->SetupLua(r, className);
				gui::WGUI::GetInstance().RegisterElement(*elLua, className, parent);
				holder->SetHandle(pragma::util::weak_shared_handle_cast<gui::types::WIBase, gui::types::WILuaBase>(elLua->GetHandle()));
				el = elLua;
			}
			else {
				Con::CWAR << Con::PREFIX_CLIENT << "Unable to create lua GUI Element '" << className << "': Lua class is not derived from valid GUI base!" << Con::endl;
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
			Con::CWAR << Con::PREFIX_CLIENT << "Unable to create lua GUI Element '" << className << "'!" << Con::endl;
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
		string::to_lower(lclassName);
		auto basePath = "gui/" + lclassName;
		auto luaPath = Lua::find_script_file(basePath);
		if(luaPath && ExecuteLuaFile(*luaPath, nullptr, true)) {
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

static auto cvLODBias = pragma::console::get_client_con_var("cl_render_lod_bias");
void pragma::CGame::SetLODBias(int32_t bias) { get_client_state()->SetConVar("cl_render_lod_bias", std::to_string(bias)); }
int32_t pragma::CGame::GetLODBias() const { return cvLODBias->GetInt(); }
uint32_t pragma::CGame::GetLOD(float dist, uint32_t maxLod) const
{
	auto lod = CUInt32(dist / LOD_SWAP_DISTANCE) + GetLODBias();
	if(lod < 0)
		lod = 0;
	if(lod > maxLod)
		lod = maxLod;
	return static_cast<uint32_t>(lod);
}

template<typename TCPPM>
void pragma::CGame::CreateGiblet(const GibletCreateInfo &info, TCPPM **particle)
{
	if(particle != nullptr)
		*particle = nullptr;
	if(info.lifetime <= 0.f)
		return;
	auto *pt = ecs::CParticleSystemComponent::Create({{"maxparticles", "1"}, {"emission_rate", "10000"}, {"cast_shadows", "1"}, {"radius", std::to_string(info.scale)},
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
		*particle = reinterpret_cast<TCPPM *>(pt);
}
template DLLCLIENT void pragma::CGame::CreateGiblet(const GibletCreateInfo &info, ecs::CParticleSystemComponent **particle);

void pragma::CGame::CreateGiblet(const GibletCreateInfo &info) { CreateGiblet<ecs::CParticleSystemComponent>(info, nullptr); }

pragma::gui::types::WIBase *pragma::CGame::CreateGUIElement(std::string name, gui::WIHandle *hParent)
{
	string::to_lower(name);
	gui::types::WIBase *pParent = nullptr;
	if(hParent != nullptr && hParent->IsValid())
		pParent = hParent->get();
	return CreateGUIElement(name, pParent);
}
LuaGUIManager &pragma::CGame::GetLuaGUIManager() { return m_luaGUIElements; }
pragma::LuaShaderManager &pragma::CGame::GetLuaShaderManager() { return *m_luaShaderManager; }
pragma::cxxm_LuaParticleModifierManager &pragma::CGame::GetLuaParticleModifierManager() { return *static_cast<cxxm_LuaParticleModifierManager *>(m_luaParticleModifierManager.get()); }
pragma::LuaInputBindingLayerRegister &pragma::CGame::GetLuaInputBindingLayerRegister() { return *m_luaInputBindingLayerRegister; }

void pragma::CGame::SetUp()
{
	Game::SetUp();
	CListener *listener = CreateEntity<CListener>();
	m_listener = listener->GetComponent<CListenerComponent>()->GetHandle();

	CViewModel *vm = CreateEntity<CViewModel>();
	m_viewModel = vm->GetComponent<CViewModelComponent>()->GetHandle();

	CViewBody *body = CreateEntity<CViewBody>();
	m_viewBody = body->GetComponent<CViewBodyComponent>()->GetHandle();

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
	//Scene &scene = *pragma::get_cengine()->GetScene(0);
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

static auto cvAntiAliasing = pragma::console::get_client_con_var("cl_render_anti_aliasing");
static auto cvMsaaSamples = pragma::console::get_client_con_var("cl_render_msaa_samples");
uint32_t pragma::CGame::GetMSAASampleCount()
{
	auto bMsaaEnabled = static_cast<rendering::AntiAliasing>(cvAntiAliasing->GetInt()) == rendering::AntiAliasing::MSAA;
	unsigned int numSamples = bMsaaEnabled ? math::pow(2, cvMsaaSamples->GetInt()) : 0;
	rendering::ClampMSAASampleCount(&numSamples);
	return numSamples;
}
void pragma::CGame::ReloadRenderFrameBuffer()
{
	auto *scene = GetScene<CSceneComponent>();
	if(scene)
		scene->ReloadRenderTarget(scene->GetWidth(), scene->GetHeight());
}

void pragma::CGame::Think()
{
	Game::Think();
	auto *scene = GetRenderScene<CSceneComponent>();
	auto *cam = GetPrimaryCamera<CCameraComponent>();

	double tDelta = m_stateNetwork->DeltaTime();
	m_tServer += DeltaTime();
	if(m_gameComponent.valid())
		static_cast<CGameComponent *>(m_gameComponent.get())->UpdateFrame(cam);
	CallCallbacks<void>("Think");
	CallLuaCallbacks("Think");
	if(m_gameComponent.valid())
		static_cast<CGameComponent *>(m_gameComponent.get())->UpdateCamera(cam);

	if(scene)
		SetRenderScene(*scene);

	auto &info = debug::get_render_debug_info();
	info.Reset();

	PostThink();
}

static auto cvUpdateRate = pragma::console::get_client_con_var("cl_updaterate");
void pragma::CGame::Tick()
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

void pragma::CGame::ReloadGameWorldShaderPipelines() const
{
	if(math::is_flag_set(m_stateFlags, StateFlags::GameWorldShaderPipelineReloadRequired))
		return;
	math::set_flag(const_cast<CGame *>(this)->m_stateFlags, StateFlags::GameWorldShaderPipelineReloadRequired);

	auto cb = FunctionCallback<void>::Create(nullptr);
	static_cast<Callback<void> *>(cb.get())->SetFunction([this, cb]() mutable {
		cb.Remove();

		if(!math::is_flag_set(const_cast<CGame *>(this)->m_stateFlags, StateFlags::GameWorldShaderPipelineReloadRequired))
			return;
		math::set_flag(const_cast<CGame *>(this)->m_stateFlags, StateFlags::GameWorldShaderPipelineReloadRequired, false);
		auto &shaderManager = get_cengine()->GetShaderManager();
		for(auto &shader : shaderManager.GetShaders()) {
			auto *gameWorldShader = dynamic_cast<ShaderGameWorldLightingPass *>(shader.get());
			if(gameWorldShader == nullptr)
				continue;
			gameWorldShader->ReloadPipelines();
		}
		get_cengine()->GetRenderContext().GetPipelineLoader().Flush();
	});
	const_cast<CGame *>(this)->AddCallback("PreRenderScenes", cb);
}
void pragma::CGame::ReloadPrepassShaderPipelines() const
{
	if(math::is_flag_set(m_stateFlags, StateFlags::PrepassShaderPipelineReloadRequired))
		return;
	math::set_flag(const_cast<CGame *>(this)->m_stateFlags, StateFlags::PrepassShaderPipelineReloadRequired);

	auto cb = FunctionCallback<void>::Create(nullptr);
	static_cast<Callback<void> *>(cb.get())->SetFunction([this, cb]() mutable {
		cb.Remove();

		math::set_flag(const_cast<CGame *>(this)->m_stateFlags, StateFlags::PrepassShaderPipelineReloadRequired, false);
		auto &shader = GetGameShader(GameShader::Prepass);
		if(shader.valid())
			shader.get()->ReloadPipelines();
	});
	const_cast<CGame *>(this)->AddCallback("PreRenderScenes", cb);
}

static auto cvSimEnabled = pragma::console::get_client_con_var("cl_physics_simulation_enabled");
bool pragma::CGame::IsPhysicsSimulationEnabled() const { return cvSimEnabled->GetBool(); }

const pragma::util::WeakHandle<prosper::Shader> &pragma::CGame::GetGameShader(GameShader shader) const { return m_gameShaders.at(math::to_integral(shader)); }

LuaCallbackHandler &pragma::CGame::GetInputCallbackHandler() { return m_inputCallbackHandler; }

std::shared_ptr<pragma::geometry::ModelMesh> pragma::CGame::CreateModelMesh() const { return pragma::util::make_shared<geometry::CModelMesh>(); }
std::shared_ptr<pragma::geometry::ModelSubMesh> pragma::CGame::CreateModelSubMesh() const { return pragma::util::make_shared<geometry::CModelSubMesh>(); }

Float pragma::CGame::GetHDRExposure() const
{
	auto *renderer = GetScene<CSceneComponent>()->GetRenderer<CRendererComponent>();
	auto raster = renderer ? renderer->GetEntity().GetComponent<CRasterizationRendererComponent>() : pragma::ComponentHandle<CRasterizationRendererComponent> {};
	return raster.valid() ? raster->GetHDRExposure() : 0.f;
}
Float pragma::CGame::GetMaxHDRExposure() const
{
	auto *renderer = GetScene<CSceneComponent>()->GetRenderer<CRendererComponent>();
	auto raster = renderer ? renderer->GetEntity().GetComponent<CRasterizationRendererComponent>() : pragma::ComponentHandle<CRasterizationRendererComponent> {};
	return raster.valid() ? raster->GetMaxHDRExposure() : 0.f;
}
void pragma::CGame::SetMaxHDRExposure(Float exposure)
{
	auto *renderer = GetScene<CSceneComponent>()->GetRenderer<CRendererComponent>();
	auto raster = renderer ? renderer->GetEntity().GetComponent<CRasterizationRendererComponent>() : pragma::ComponentHandle<CRasterizationRendererComponent> {};
	if(raster.expired())
		return;
	raster->SetMaxHDRExposure(exposure);
}

void pragma::CGame::OnMapLoaded()
{
	Game::OnMapLoaded();

	auto *scene = GetRenderScene<CSceneComponent>();
	if(scene)
		scene->UpdateRenderData();

	// Update reflection probes
	// TODO: Make sure all map materials have been fully loaded before doing this!
	// pragma::CReflectionProbeComponent::BuildAllReflectionProbes(*this);
}

void pragma::CGame::InitializeMapEntities(asset::WorldData &worldData, std::vector<EntityHandle> &outEnts)
{
	auto &entityData = worldData.GetEntities();
	outEnts.reserve(entityData.size());

	std::unordered_map<uint32_t, EntityHandle> mapIndexToEntity;
	ecs::EntityIterator entIt {*this, ecs::EntityIterator::FilterFlags::Default | ecs::EntityIterator::FilterFlags::Pending};
	entIt.AttachFilter<TEntityIteratorFilterComponent<MapComponent>>();
	mapIndexToEntity.reserve(entIt.GetCount());
	for(auto *ent : entIt) {
		auto mapC = ent->GetComponent<MapComponent>();
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
			if(!math::is_flag_set(flags, asset::ComponentData::Flags::ClientsideOnly))
				continue;
			CreateMapComponent(ent, cType, *cData);
		}
	}

	for(auto &hEnt : outEnts) {
		if(hEnt.valid() == false)
			continue;
		auto &ent = *hEnt.get();
		if(ent.IsWorld()) {
			auto pWorldComponent = ent.GetComponent<CWorldComponent>();
			if(pWorldComponent.valid()) {
				auto *bspTree = worldData.GetBSPTree();
				if(bspTree)
					pWorldComponent->SetBSPTree(bspTree->shared_from_this(), worldData.GetClusterMeshIndices());
			}
		}

		auto &mdl = ent.GetModel();
		if(mdl == nullptr) {
			auto pRenderComponent = static_cast<ecs::CBaseEntity &>(ent).GetRenderComponent();
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

void pragma::CGame::InitializeWorldData(asset::WorldData &worldData)
{
	Game::InitializeWorldData(worldData);

	auto &texManager = static_cast<material::CMaterialManager &>(static_cast<ClientState *>(GetNetworkState())->GetMaterialManager()).GetTextureManager();
	auto texture = texManager.LoadAsset(worldData.GetLightmapAtlasTexturePath(GetMapName()));
	if(texture) {
		prosper::util::SamplerCreateInfo samplerCreateInfo {};
		auto sampler = get_cengine()->GetRenderContext().CreateSampler(samplerCreateInfo);
		texture->GetVkTexture()->SetSampler(*sampler);

		auto &tex = *static_cast<material::Texture *>(texture.get());
		auto lightmapAtlas = tex.GetVkTexture();
		//auto lightmapAtlas = pragma::CLightMapComponent::CreateLightmapTexture(img->GetWidth(),img->GetHeight(),static_cast<uint16_t*>(img->GetData()));
		auto *scene = GetScene<CSceneComponent>();
		auto *renderer = scene ? scene->GetRenderer<CRendererComponent>() : nullptr;
		if(renderer != nullptr) {
			if(lightmapAtlas != nullptr) {
				auto *entWorld = get_cgame()->GetWorld();
				auto lightMapC = entWorld ? entWorld->GetEntity().GetComponent<CLightMapComponent>() : pragma::ComponentHandle<CLightMapComponent> {};
				if(lightMapC.valid())
					CRasterizationRendererComponent::UpdateLightmap(*lightMapC);
			}
		}

		// Find map entities with lightmap uv sets
		ecs::EntityIterator entIt {*get_cgame(), ecs::EntityIterator::FilterFlags::Default | ecs::EntityIterator::FilterFlags::Pending};
		entIt.AttachFilter<TEntityIteratorFilterComponent<MapComponent>>();
		for(auto *ent : entIt)
			CLightMapReceiverComponent::SetupLightMapUvData(static_cast<ecs::CBaseEntity &>(*ent));

		// Generate lightmap uv buffers for all entities
		if(worldData.IsLegacyLightMapEnabled()) {
			std::vector<std::shared_ptr<prosper::IBuffer>> buffers {};
			auto globalLightmapUvBuffer = CLightMapComponent::GenerateLightmapUVBuffers(buffers);

			auto *world = get_cgame()->GetWorld();
			if(world && globalLightmapUvBuffer) {
				auto lightMapC = world->GetEntity().GetComponent<CLightMapComponent>();
				if(lightMapC.valid()) {
					// lightMapC->SetLightMapIntensity(worldData.GetLightMapIntensity());
					lightMapC->SetLightMapExposure(worldData.GetLightMapExposure());
					lightMapC->InitializeLightMapData(lightmapAtlas, globalLightmapUvBuffer, buffers);
					auto *scene = GetRenderScene<CSceneComponent>();
					if(scene)
						scene->SetLightMap(*lightMapC);
				}
			}
		}
	}

	auto &materialManager = static_cast<material::CMaterialManager &>(get_client_state()->GetMaterialManager());
	materialManager.ReloadMaterialShaders();
}

bool pragma::CGame::LoadMap(const std::string &map, const Vector3 &origin, std::vector<EntityHandle> *entities)
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

void pragma::CGame::BuildVMF(const char *)
{
	//Game::BuildVMF<CWorld,CPolyMesh,CPoly,CBrushMesh>(map);
}

static auto cvTimescale = pragma::console::get_client_con_var("host_timescale");
float pragma::CGame::GetTimeScale() { return cvTimescale->GetFloat(); }

void pragma::CGame::SetTimeScale(float t)
{
	Game::SetTimeScale(t);
	for(auto &rsnd : get_client_state()->GetSounds()) {
		auto &snd = static_cast<audio::CALSound &>(rsnd.get());
		snd.SetPitchModifier(t); // TODO Implement SetPitchModifier for SoundScripts
	}
}

bool pragma::CGame::IsServer() { return false; }
bool pragma::CGame::IsClient() { return true; }

void pragma::CGame::SetLocalPlayer(CPlayerComponent *pl)
{
	m_plLocal = pl->GetHandle<CPlayerComponent>();
	pl->SetLocalPlayer(true);

	auto *cam = GetPrimaryCamera<CCameraComponent>();
	if(cam) {
		auto observerC = cam->GetEntity().GetComponent<CObserverComponent>();
		auto observableC = pl->GetEntity().GetComponent<CObservableComponent>();
		if(observerC.valid() && observableC.valid())
			observerC->SetObserverTarget(observableC.get());
	}

	CallCallbacks<void, CPlayerComponent *>("OnLocalPlayerSpawned", pl);
	CallLuaCallbacks<void, luabind::object>("OnLocalPlayerSpawned", pl->GetLuaObject());
}

void pragma::CGame::OnReceivedPlayerInputResponse(uint8_t userInputId)
{
	if(m_userInputTracker.IsMessageInOrder(userInputId) == false)
		return;
	m_userInputTracker.CheckMessages(userInputId, m_lostPackets);
	//UpdateLatency(m_userInputTracker.messageTimestamps[userInputId]);
}

uint16_t pragma::CGame::GetLatency() const
{
	auto *cl = get_client_state()->GetClient();
	if(cl == nullptr)
		return 0;
	return cl->GetLatency();
}

pragma::rendering::RenderMask pragma::CGame::GetInclusiveRenderMasks() const { return m_inclusiveRenderMasks; }
pragma::rendering::RenderMask pragma::CGame::GetExclusiveRenderMasks() const { return m_exclusiveRenderMasks; }
bool pragma::CGame::IsInclusiveRenderMask(rendering::RenderMask mask) const { return math::is_flag_set(m_inclusiveRenderMasks, mask); }
bool pragma::CGame::IsExclusiveRenderMask(rendering::RenderMask mask) const { return math::is_flag_set(m_exclusiveRenderMasks, mask); }
pragma::rendering::RenderMask pragma::CGame::RegisterRenderMask(const std::string &name, bool inclusiveByDefault)
{
	constexpr auto highestAllowedMask = (static_cast<uint64_t>(1) << (sizeof(uint64_t) * 8 - 1));
	if(math::to_integral(m_nextCustomRenderMaskIndex) == highestAllowedMask)
		throw std::runtime_error {"Exceeded maximum allowed number of custom render masks!"};
	auto mask = GetRenderMask(name);
	if(mask.has_value())
		return *mask;
	auto id = m_nextCustomRenderMaskIndex;
	m_customRenderMasks.insert(std::make_pair(name, id));
	m_nextCustomRenderMaskIndex = static_cast<decltype(m_nextCustomRenderMaskIndex)>(math::to_integral(m_nextCustomRenderMaskIndex) << 1);
	if(inclusiveByDefault)
		m_inclusiveRenderMasks |= id;
	else
		m_exclusiveRenderMasks |= id;
	return id;
}
std::optional<pragma::rendering::RenderMask> pragma::CGame::GetRenderMask(const std::string &name)
{
	auto it = m_customRenderMasks.find(name);
	return (it != m_customRenderMasks.end()) ? it->second : std::optional<rendering::RenderMask> {};
}
const std::string *pragma::CGame::FindRenderMaskName(rendering::RenderMask mask) const
{
	auto it = std::find_if(m_customRenderMasks.begin(), m_customRenderMasks.end(), [mask](const std::pair<std::string, rendering::RenderMask> &pair) { return pair.second == mask; });
	if(it == m_customRenderMasks.end())
		return nullptr;
	return &it->first;
}

void pragma::CGame::SendUserInput()
{
	auto *pl = GetLocalPlayer();
	if(pl == nullptr)
		return;
	m_userInputTracker.messageTimestamps[m_userInputTracker.outMessageId] = RealTime();

	NetPacket p;
	p->Write<uint8_t>(m_userInputTracker.outMessageId++);
	//p->Write<uint8_t>(m_latency); // Deprecated: Let the networkmanager handle it
	auto &ent = pl->GetEntity();
	auto charComponent = ent.GetCharacterComponent();
	auto pTrComponent = ent.GetTransformComponent();
	auto orientation = charComponent.valid() ? charComponent->GetViewOrientation() : pTrComponent != nullptr ? pTrComponent->GetRotation() : uquat::identity();
	networking::write_quat(p, orientation);
	p->Write<Vector3>(pl->GetViewPos());

	auto *actionInputC = pl->GetActionInputController();
	auto actions = actionInputC ? actionInputC->GetActionInputs() : Action::None;
	p->Write<Action>(actions);
	auto bControllers = get_cengine()->GetControllersEnabled();
	p->Write<bool>(bControllers);
	if(bControllers == true) {
		auto actionValues = math::get_power_of_2_values(math::to_integral(actions));
		for(auto v : actionValues) {
			auto magnitude = 0.f;
			if(actionInputC)
				actionInputC->GetActionInputAxisMagnitude(static_cast<Action>(v));
			p->Write<float>(magnitude);
		}
	}
	get_client_state()->SendPacket(networking::net_messages::server::USERINPUT, p, networking::Protocol::FastUnreliable);
}

double &pragma::CGame::ServerTime() { return m_tServer; }
void pragma::CGame::SetServerTime(double t) { m_tServer = t; }

bool pragma::CGame::RunLua(const std::string &lua) { return Game::RunLua(lua, "lua_run_cl"); }

void pragma::CGame::UpdateLostPackets()
{
	auto &t = RealTime();
	while(!m_lostPackets.empty() && t - m_lostPackets.front() > 1.0)
		m_lostPackets.erase(m_lostPackets.begin());
}

uint32_t pragma::CGame::GetLostPacketCount()
{
	UpdateLostPackets();
	return static_cast<uint32_t>(m_lostPackets.size());
}

void pragma::CGame::ReceiveSnapshot(NetPacket &packet)
{
	//Con::CCL<<"Received snapshot.."<<Con::endl;
	//auto tOld = m_tServer;
	auto latency = GetLatency() / 2.f; // Latency is entire roundtrip; We need the time for one way
	auto tActivated = (util::clock::to_int(util::clock::get_duration_since_start()) - packet.GetTimeActivated()) / 1'000'000.0;
	//Con::CCL<<"Snapshot delay: "<<+latency<<"+ "<<tActivated<<" = "<<(latency +tActivated)<<Con::endl;
	auto tDelta = static_cast<float>((latency + tActivated) / 1'000.0);

	auto snapshotId = packet->Read<uint8_t>();
	m_tServer = packet->Read<double>();

	auto &t = RealTime();
	m_snapshotTracker.messageTimestamps[snapshotId] = m_tServer;
	if(m_snapshotTracker.IsMessageInOrder(snapshotId) == false)
		return; // Old snapshot; Just skip it (We're already received a newer snapshot, this one's out of order)
	m_snapshotTracker.CheckMessages(snapshotId, m_lostPackets, t);

	//std::cout<<"Received snapshot with "<<(m_tServer -tOld)<<" time difference to last snapshot"<<std::endl;
	const auto maxCorrectionDistance = math::pow2(10.f);
	unsigned int numEnts = packet->Read<unsigned int>();
	for(unsigned int i = 0; i < numEnts; i++) {
		ecs::CBaseEntity *ent = static_cast<ecs::CBaseEntity *>(networking::read_entity(packet));
		Vector3 pos = networking::read_vector(packet);
		Vector3 vel = networking::read_vector(packet);
		Vector3 angVel = networking::read_vector(packet);
		auto orientation = networking::read_quat(packet);
		auto entDataSize = packet->Read<UInt8>();
		if(ent != nullptr) {
			pos += vel * tDelta;
			if(uvec::length_sqr(angVel) > 0.0)
				orientation = uquat::create(EulerAngles(math::rad_to_deg(angVel.x), math::rad_to_deg(angVel.y), math::rad_to_deg(angVel.z)) * tDelta) * orientation; // TODO: Check if this is correct

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
			auto pVelComponent = ent->GetComponent<VelocityComponent>();
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

		auto flags = packet->Read<SnapshotFlags>();
		if((flags & SnapshotFlags::PhysicsData) != SnapshotFlags::None) {
			auto numObjs = packet->Read<uint8_t>();
			if(ent != nullptr) {
				auto pPhysComponent = ent->GetPhysicsComponent();
				physics::PhysObj *physObj = pPhysComponent != nullptr ? pPhysComponent->GetPhysicsObject() : nullptr;
				if(physObj != nullptr && !physObj->IsStatic()) {
					auto colObjs = physObj->GetCollisionObjects();
					auto numActualObjs = colObjs.size();
					for(auto i = decltype(numObjs) {0}; i < numObjs; ++i) {
						auto pos = packet->Read<Vector3>();
						auto rot = packet->Read<Quat>();
						auto vel = packet->Read<Vector3>();
						auto angVel = packet->Read<Vector3>();
						if(physObj->IsController()) {
							auto *physController = static_cast<physics::ControllerPhysObj *>(physObj);
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
									rot = uquat::create(EulerAngles(math::rad_to_deg(angVel.x), math::rad_to_deg(angVel.y), math::rad_to_deg(angVel.z)) * tDelta) * rot; // TODO: Check if this is correct
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

		if((flags & SnapshotFlags::ComponentData) != SnapshotFlags::None) {
			auto &componentManager = static_cast<CEntityComponentManager &>(GetEntityComponentManager());
			auto &svComponentToClComponentTable = componentManager.GetServerComponentIdToClientComponentIdTable();
			auto &componentTypes = componentManager.GetRegisteredComponentTypes();
			auto numComponents = packet->Read<uint8_t>();
			for(auto i = decltype(numComponents) {0}; i < numComponents; ++i) {
				auto svId = packet->Read<ComponentId>();
				auto componentSize = packet->Read<uint8_t>();
				auto componentEndOffset = packet->GetOffset() + componentSize;
				if(ent != nullptr && svId < svComponentToClComponentTable.size() && svComponentToClComponentTable.at(svId) != CEntityComponentManager::INVALID_COMPONENT) {
					auto clId = svComponentToClComponentTable.at(svId);
					if(clId >= componentTypes.size())
						throw std::runtime_error("Invalid client component type index " + std::to_string(clId) + "!");
					auto pComponent = ent->FindComponent(clId);
					if(pComponent.valid()) {
						auto *pSnapshotComponent = dynamic_cast<CBaseSnapshotComponent *>(pComponent.get());
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
		auto *plComponent = networking::read_player(packet);
		auto *pl = (plComponent != nullptr) ? static_cast<CPlayer *>(plComponent->GetBasePlayer()) : nullptr;
		auto orientation = networking::read_quat(packet);
		unsigned char numKeys = packet->Read<unsigned char>();
		for(int i = 0; i < numKeys; i++) {
			unsigned short key = packet->Read<unsigned short>();
			// UNUSED(key);
			char pressed = packet->Read<char>();
			int action = (pressed == 1) ? KEY_PRESS : KEY_RELEASE;
			// UNUSED(action);
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
		if(plComponent != nullptr && pl->IsCharacter()) {
			auto charComponent = pl->GetCharacterComponent();
			charComponent->SetViewOrientation(orientation);
		}
	}
}

static void set_action_input(pragma::Action action, bool b, bool bKeepMagnitude, const float *inMagnitude = nullptr)
{
	auto magnitude = 0.f;
	if(inMagnitude != nullptr)
		magnitude = *inMagnitude;
	else if(bKeepMagnitude == false)
		magnitude = (b == true) ? 1.f : 0.f;
	auto *pl = pragma::get_cgame()->GetLocalPlayer();
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
void pragma::CGame::SetActionInput(Action action, bool b, bool bKeepMagnitude) { set_action_input(action, b, bKeepMagnitude); }
void pragma::CGame::SetActionInput(Action action, bool b, float magnitude) { set_action_input(action, b, false, &magnitude); }

bool pragma::CGame::GetActionInput(Action action)
{
	auto *pl = GetLocalPlayer();
	if(pl == nullptr)
		return false;
	auto *actionInputC = pl->GetActionInputController();
	if(!actionInputC)
		return false;
	return actionInputC->GetActionInput(action);
}

void pragma::CGame::DrawLine(const Vector3 &start, const Vector3 &end, const Color &color, float duration) { debug::DebugRenderer::DrawLine(start, end, {color, duration}); }
void pragma::CGame::DrawBox(const Vector3 &origin, const Vector3 &start, const Vector3 &end, const EulerAngles &ang, const Color &colorOutline, const std::optional<Color> &fillColor, float duration)
{
	debug::DebugRenderInfo renderInfo {};
	renderInfo.SetOrigin(origin);
	renderInfo.SetRotation(uquat::create(ang));
	renderInfo.SetDuration(duration);
	if(fillColor) {
		renderInfo.SetColor(*fillColor);
		renderInfo.SetOutlineColor(colorOutline);
		debug::DebugRenderer::DrawBox(start, end, renderInfo);
	}
	else {
		renderInfo.SetColor(colorOutline);
		debug::DebugRenderer::DrawBox(start, end, renderInfo);
	}
}
void pragma::CGame::DrawPlane(const Vector3 &n, float dist, const Color &color, float duration) { debug::DebugRenderer::DrawPlane(n, dist, {color, duration}); }
void pragma::CGame::DrawMesh(const std::vector<Vector3> &meshVerts, const Color &color, const Color &colorOutline, float duration) { debug::DebugRenderer::DrawMesh(meshVerts, {color, colorOutline, duration}); }
static auto cvRenderPhysics = pragma::console::get_client_con_var("debug_physics_draw");
static auto cvSvRenderPhysics = pragma::console::get_client_con_var("sv_debug_physics_draw");
template<typename TCPPM>
void pragma::CGame::RenderDebugPhysics(std::shared_ptr<prosper::ICommandBuffer> &drawCmd, TCPPM &cam)
{
	if(cvRenderPhysics->GetBool()) {
		auto *physEnv = GetPhysicsEnvironment();
		auto *pVisualDebugger = physEnv ? physEnv->GetVisualDebugger() : nullptr;
		if(pVisualDebugger)
			static_cast<physics::CPhysVisualDebugger &>(*pVisualDebugger).Render(drawCmd, cam);
	}
	if(cvSvRenderPhysics->GetBool()) {
		// Serverside physics (singleplayer only)
		auto *svState = get_cengine()->GetServerNetworkState();
		auto *game = svState ? svState->GetGameState() : nullptr;
		auto *physEnv = game ? game->GetPhysicsEnvironment() : nullptr;
		auto *pVisualDebugger = physEnv ? physEnv->GetVisualDebugger() : nullptr;
		if(pVisualDebugger)
			static_cast<physics::CPhysVisualDebugger &>(*pVisualDebugger).Render(drawCmd, cam);
	}
}
template DLLCLIENT void pragma::CGame::RenderDebugPhysics<pragma::CCameraComponent>(std::shared_ptr<prosper::ICommandBuffer> &, CCameraComponent &);

bool pragma::CGame::LoadAuxEffects(const std::string &fname)
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
		string::to_lower(name);
		std::string type;
		pair.property["type"](type);
		audio::create_aux_effect(name, type, pair.property);
	}
	return true;
}
std::shared_ptr<pragma::audio::IEffect> pragma::CGame::GetAuxEffect(const std::string &name) { return get_cengine()->GetAuxEffect(name); }

bool pragma::CGame::SaveImage(prosper::IImage &image, const std::string &fileName, const image::TextureInfo &imageWriteInfo) const
{
	auto path = ufile::get_path_from_filename(fileName);
	fs::create_path(path);
	return prosper::util::save_texture(fileName, image, imageWriteInfo, [fileName](const std::string &err) { Con::CWAR << "Unable to save image '" << fileName << "': " << err << Con::endl; });
}

bool pragma::CGame::SaveImage(const std::vector<std::vector<const void *>> &imgLayerMipmapData, uint32_t width, uint32_t height, uint32_t szPerPixel, const std::string &fileName, const image::TextureInfo &imageWriteInfo, bool cubemap) const
{
	auto path = ufile::get_path_from_filename(fileName);
	fs::create_path(path);
	image::TextureSaveInfo texSaveInfo;
	texSaveInfo.texInfo = imageWriteInfo;
	texSaveInfo.width = width;
	texSaveInfo.height = height;
	texSaveInfo.szPerPixel = szPerPixel;
	texSaveInfo.cubemap = cubemap;
	return image::save_texture(fileName, imgLayerMipmapData, texSaveInfo, [fileName](const std::string &err) { Con::CWAR << "Unable to save image '" << fileName << "': " << err << Con::endl; });
}

bool pragma::CGame::SaveImage(image::ImageBuffer &imgBuffer, const std::string &fileName, const image::TextureInfo &imageWriteInfo, bool cubemap) const
{
	auto path = ufile::get_path_from_filename(fileName);
	fs::create_path(path);
	image::TextureSaveInfo texSaveInfo;
	texSaveInfo.texInfo = imageWriteInfo;
	texSaveInfo.cubemap = cubemap;
	return image::save_texture(fileName, imgBuffer, texSaveInfo, [fileName](const std::string &err) { Con::CWAR << "Unable to save image '" << fileName << "': " << err << Con::endl; });
}

static auto cvFriction = pragma::console::get_client_con_var("sv_friction");
Float pragma::CGame::GetFrictionScale() const { return cvFriction->GetFloat(); }
static auto cvRestitution = pragma::console::get_client_con_var("sv_restitution");
Float pragma::CGame::GetRestitutionScale() const { return cvRestitution->GetFloat(); }
