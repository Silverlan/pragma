// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/console/helper.hpp"

module pragma.client;

import :client_state;
import :audio;
import :console.register_commands;
import :engine;
import :entities.components;
import :gui;
import :model;
import :networking;
import :scripting.lua;
import :util;
// import pragma.scripting.lua;

#undef GetMessage

static std::unordered_map<std::string, std::shared_ptr<pragma::console::PtrConVar>> *conVarPtrs = nullptr;
std::unordered_map<std::string, std::shared_ptr<pragma::console::PtrConVar>> &pragma::ClientState::GetConVarPtrs() { return *conVarPtrs; }
pragma::console::ConVarHandle pragma::ClientState::GetConVarHandle(std::string scvar)
{
	if(conVarPtrs == nullptr) {
		static std::unordered_map<std::string, std::shared_ptr<console::PtrConVar>> ptrs;
		conVarPtrs = &ptrs;
	}
	return NetworkState::GetConVarHandle(*conVarPtrs, scvar);
}

static pragma::ClientState *g_client = nullptr;

pragma::ClientState *pragma::get_client_state() { return g_client; }

std::vector<std::string> &get_required_game_textures();
pragma::ClientState::ClientState() : NetworkState(), m_client(nullptr), m_svInfo(nullptr), m_resDownload(nullptr), m_volMaster(1.f), m_hMainMenu(), m_luaGUI(nullptr)
{
	g_client = this;
	m_soundScriptManager = std::make_unique<audio::CSoundScriptManager>();

	m_modelManager = std::make_unique<asset::CModelManager>(*this);
	// m_modelManager->SetVerbose(true);
	get_cengine()->InitializeAssetManager(*m_modelManager);
	pragma::asset::update_extension_cache(asset::Type::Model);

	auto &gui = gui::WGUI::GetInstance();
	// gui.SetCreateCallback(WGUILuaInterface::InitializeGUIElement);
	//CVarHandler::Initialize();
	fs::add_custom_mount_directory("downloads", static_cast<fs::SearchFlags>(networking::FSYS_SEARCH_RESOURCES));

	RegisterCallback<void, CGame *>("OnGameStart");
	RegisterCallback<void, CGame *>("EndGame");
	RegisterCallback<void, material::CMaterial *>("OnMaterialLoaded");

	RegisterCallback<void>("Draw");
	RegisterCallback<void, std::reference_wrapper<const rendering::DrawSceneInfo>, std::reference_wrapper<std::shared_ptr<prosper::RenderTarget>>>("PreRender");
	RegisterCallback<void, std::reference_wrapper<const rendering::DrawSceneInfo>, std::reference_wrapper<std::shared_ptr<prosper::RenderTarget>>>("PostRender");
	RegisterCallback<void, std::reference_wrapper<NetPacket>>("OnReceivePacket");
	RegisterCallback<void, std::reference_wrapper<NetPacket>>("OnSendPacketTCP");
	RegisterCallback<void, std::reference_wrapper<NetPacket>>("OnSendPacketUDP");

	auto &texManager = static_cast<material::CMaterialManager &>(GetMaterialManager()).GetTextureManager();
	for(auto &tex : get_required_game_textures())
		texManager.PreloadAsset(tex);
}

pragma::ClientState::~ClientState()
{
	Disconnect();
	fs::remove_custom_mount_directory("downloads");

	get_cengine()->GetSoundSystem()->SetOnReleaseSoundCallback(nullptr);

	ClearCommands();
}

void pragma::ClientState::UpdateGameWorldShaderSettings()
{
	auto oldSettings = m_worldShaderSettings;
	m_worldShaderSettings.shadowQuality = static_cast<rendering::GameWorldShaderSettings::ShadowQuality>(GetConVarInt("render_shadow_quality"));
	m_worldShaderSettings.ssaoEnabled = GetConVarBool("cl_render_ssao");
	m_worldShaderSettings.bloomEnabled = GetConVarBool("render_bloom_enabled");
	m_worldShaderSettings.debugModeEnabled = GetConVarBool("render_debug_mode") || GetConVarBool("render_unlit");
	m_worldShaderSettings.fxaaEnabled = static_cast<rendering::AntiAliasing>(GetConVarInt("cl_render_anti_aliasing")) == rendering::AntiAliasing::FXAA;
	m_worldShaderSettings.iblEnabled = GetConVarBool("render_ibl_enabled");
	m_worldShaderSettings.dynamicLightingEnabled = GetConVarBool("render_dynamic_lighting_enabled");
	m_worldShaderSettings.dynamicShadowsEnabled = GetConVarBool("render_dynamic_shadows_enabled");
	if(m_worldShaderSettings == oldSettings)
		return;

	auto *game = GetGameState();
	if(game)
		static_cast<CGame *>(game)->OnGameWorldShaderSettingsChanged(m_worldShaderSettings, oldSettings);
}

void pragma::ClientState::InitializeGameClient(bool singlePlayerLocalGame)
{
	DestroyClient();

	networking::ClientEventInterface eventInterface {};
	eventInterface.onConnected = [this]() { HandleConnect(); };
	eventInterface.onDisconnected = []() {};
	eventInterface.onConnectionClosed = []() {};
	//eventInterface.onPacketSent = [](pragma::networking::Protocol protocol,NetPacket &packet) {
	//};
	eventInterface.handlePacket = [this](NetPacket &packet) { HandlePacket(packet); };
	if(singlePlayerLocalGame == false) {
		auto netLibName = GetConVarString("net_library");
		auto netModPath = networking::GetNetworkingModuleLocation(netLibName, false);
		std::string err;
		auto dllHandle = InitializeLibrary(netModPath, &err);
		if(dllHandle) {
			auto *fInitNetLib = dllHandle->FindSymbolAddress<void (*)(NetworkState &, std::unique_ptr<networking::IClient> &)>("initialize_game_client");
			if(fInitNetLib != nullptr)
				fInitNetLib(*this, m_client);
			else
				spdlog::error("Unable to initialize networking system '{}': Function 'initialize_game_client' not found in module!", netLibName);
		}
		else
			spdlog::error("Unable to initialize networking system '{}': {}", netLibName, err);
		if(m_client != nullptr)
			ResetGameClient();
	}
	else
		m_client = std::make_unique<networking::LocalClient>();
	if(m_client)
		m_client->SetEventInterface(eventInterface);
}
void pragma::ClientState::ResetGameClient()
{
	//m_client = std::make_unique<pragma::networking::LocalClient>();
}

static auto cvSteamAudioEnabled = pragma::console::get_client_con_var("cl_steam_audio_enabled");

void pragma::ClientState::Initialize()
{
	/*Con::CWAR<<"Client NetMessages:"<<Con::endl;
	auto *mapMsgs = GetNetMessageMap();
	std::unordered_map<std::string,unsigned int> *msgs;
	mapMsgs->GetNetMessages(&msgs);
	std::unordered_map<std::string,unsigned int>::iterator i;
	for(i=msgs->begin();i!=msgs->end();i++)
	{
		Con::COUT<<"Name: "<<i->first<<Con::endl;
	}*/
	NetworkState::Initialize();

	get_cengine()->GetSoundSystem()->SetOnReleaseSoundCallback([this](const audio::SoundSource &snd) {
		auto it = std::find_if(m_sounds.begin(), m_sounds.end(), [&snd](const audio::ALSoundRef &sndOther) { return (static_cast<audio::CALSound *>(&sndOther.get()) == &snd) ? true : false; });
		if(it == m_sounds.end())
			return;
		m_sounds.erase(it);
	});

	get_cengine()->LoadClientConfig();
	InitializeGUILua();
	auto &gui = gui::WGUI::GetInstance();
	m_hMainMenu = gui.Create<gui::types::WIMainMenu>()->GetHandle();

	UpdateGameWorldShaderSettings();

#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
	auto *soundSys = pragma::get_cengine()->GetSoundSystem();
	if(soundSys != nullptr)
		soundSys->SetSteamAudioEnabled(cvSteamAudioEnabled->GetBool()); // See also CGame::ReloadSoundCache
#endif
}

void pragma::ClientState::ShowFPSCounter(bool b)
{
	if(b == true) {
		if(m_hFps.IsValid())
			return;
		auto *pFps = gui::WGUI::GetInstance().Create<gui::types::WIFPS>();
		if(pFps == nullptr)
			return;
		pFps->SetPos(10, 10);
		pFps->SetZPos(2000);
		m_hFps = pFps->GetHandle();
		return;
	}
	if(!m_hFps.IsValid())
		return;
	m_hFps->Remove();
}

namespace {
	auto UVN = pragma::console::client::register_variable_listener<bool>(
	  "cl_show_fps", +[](pragma::NetworkState *, const pragma::console::ConVar &, bool, bool val) {
		  auto *client = pragma::get_client_state();
		  if(client == nullptr)
			  return;
		  client->ShowFPSCounter(val);
	  });
}

lua::State *pragma::ClientState::GetGUILuaState() { return (m_luaGUI != nullptr) ? m_luaGUI->GetState() : nullptr; }
Lua::Interface &pragma::ClientState::GetGUILuaInterface() { return *m_luaGUI; }

//__declspec(dllimport) void test_lua_policies(lua::State *l);
std::optional<std::vector<std::string>> g_autoExecScripts {};

void pragma::ClientState::InitializeGUILua()
{
	m_luaGUI = pragma::util::make_shared<Lua::Interface>();
	m_luaGUI->Open();
	m_luaGUI->SetIdentifier("gui");
	Lua::initialize_lua_state(GetGUILuaInterface());

	auto utilMod = luabind::module(m_luaGUI->GetState(), "util");
	Lua::util::register_shared_generic(m_luaGUI->GetState(), utilMod);
	NetworkState::RegisterSharedLuaClasses(GetGUILuaInterface());
	NetworkState::RegisterSharedLuaLibraries(GetGUILuaInterface());
	RegisterSharedLuaClasses(*m_luaGUI, true);
	RegisterSharedLuaLibraries(*m_luaGUI, true);
	NetworkState::RegisterSharedLuaGlobals(GetGUILuaInterface());
	RegisterSharedLuaGlobals(*m_luaGUI);
	Lua::register_shared_client_state(m_luaGUI->GetState());
	Lua::udm::register_library(*m_luaGUI);
	auto modAsset = luabind::module_(m_luaGUI->GetState(), "asset");
	Lua::asset_client::register_library(*m_luaGUI, modAsset);

	auto timeMod = luabind::module(m_luaGUI->GetState(), "time");
	timeMod[(luabind::def("last_think", &Lua::gui::LastThink), luabind::def("real_time", &Lua::gui::RealTime), luabind::def("delta_time", &Lua::gui::DeltaTime))];

	auto enMod = luabind::module(m_luaGUI->GetState(), "engine");
	enMod[luabind::def("poll_console_output", &Lua::engine::poll_console_output)];
	Lua::engine::register_library(GetGUILuaState());

	// Testing
	/*{

		test_lua_policies(GetGUILuaState());

		auto *l = m_luaGUI->GetState();
		auto *el = pragma::gui::WGUI::GetInstance().Create<WIRect>();
		auto hEl = el->GetHandle();
		auto hElCast = pragma::util::weak_shared_handle_cast<WIBase,WIShape>(hEl);
		auto o = luabind::object{l,hElCast};

		auto *h1 = luabind::object_cast<pragma::util::TWeakSharedHandle<WIShape>*>(o);
		auto *h2 = luabind::object_cast<WIShape*>(o);
		auto *h3 = luabind::object_cast<WIBase*>(o);

		luabind::globals(l)["testObject"] = hEl;
		luabind::globals(l)["testObjectCast"] = hElCast;
		Lua::RunString(l,"print(\"Valid: \",testObject:IsValid())","test");
		Lua::RunString(l,"print(\"Valid: \",testObjectCast:IsValid())","test");

		el->Remove();
		h1 = luabind::object_cast<pragma::util::TWeakSharedHandle<WIShape>*>(o);
		h2 = luabind::object_cast<WIShape*>(o);
		h3 = luabind::object_cast<WIBase*>(o);

		Lua::RunString(l,"print(\"Valid: \",testObject:IsValid())","test");

		Lua::RunString(l,"print(\"Valid: \",testObjectCast:IsValid())","test");
	}*/

	gui::WGUILuaInterface::Initialize();

	scripting::lua_core::execute_files_in_directory(GetGUILuaState(), "autorun/gui/");
	if(g_autoExecScripts.has_value()) {
		for(auto &f : *g_autoExecScripts)
			scripting::lua_core::execute_file(GetGUILuaState(), f);
	}
}

void pragma::ClientState::AddGUILuaWrapperFactory(const std::function<luabind::object(lua::State *, gui::types::WIBase &)> &f) { m_guiLuaWrapperFactories.push_back(f); }
std::vector<std::function<luabind::object(lua::State *, pragma::gui::types::WIBase &)>> &pragma::ClientState::GetGUILuaWrapperFactories() { return m_guiLuaWrapperFactories; }

pragma::gui::types::WIMainMenu *pragma::ClientState::GetMainMenu()
{
	if(!m_hMainMenu.IsValid())
		return nullptr;
	return m_hMainMenu.get<gui::types::WIMainMenu>();
}

bool pragma::ClientState::IsMainMenuOpen()
{
	gui::types::WIMainMenu *menu = GetMainMenu();
	if(menu == nullptr)
		return false;
	return menu->IsVisible();
}
void pragma::ClientState::CloseMainMenu()
{
	gui::types::WIMainMenu *menu = GetMainMenu();
	if(menu == nullptr || !menu->IsVisible())
		return;
	menu->SetVisible(false);
}
void pragma::ClientState::OpenMainMenu()
{
	gui::types::WIMainMenu *menu = GetMainMenu();
	if(menu == nullptr)
		return;
	auto &window = get_cengine()->GetWindow();
	window->SetCursorInputMode(platform::CursorMode::Normal);
	menu->SetVisible(true);
}
void pragma::ClientState::ToggleMainMenu()
{
	gui::types::WIMainMenu *menu = GetMainMenu();
	if(menu == nullptr)
		return;
	if(menu->IsVisible()) {
		if(!IsGameActive())
			return;
		CloseMainMenu();
	}
	else
		OpenMainMenu();
}

pragma::NwStateType pragma::ClientState::GetType() const { return NwStateType::Client; }

void pragma::ClientState::Close()
{
	get_cengine()->GetRenderContext().GetPipelineLoader().Stop();
	get_cengine()->SaveClientConfig();
	NetworkState::Close();

	if(m_hMainMenu.IsValid())
		m_hMainMenu->Remove();

	auto *state = m_luaGUI->GetState();
	Lua::gui::clear_lua_callbacks(state);
	auto *guiBaseEl = gui::WGUI::GetInstance().GetBaseElement();
	if(guiBaseEl != nullptr)
		gui::WGUILuaInterface::ClearGUILuaObjects(*guiBaseEl);
	auto identifier = m_luaGUI->GetIdentifier();
	TerminateLuaModules(state);
	m_luaGUI = nullptr;
	DeregisterLuaModules(state, identifier); // Has to be called AFTER Lua instance has been released!
	std::unordered_map<std::string, std::shared_ptr<console::PtrConVar>> &conVarPtrs = GetConVarPtrs();
	std::unordered_map<std::string, std::shared_ptr<console::PtrConVar>>::iterator itHandles;
	for(itHandles = conVarPtrs.begin(); itHandles != conVarPtrs.end(); itHandles++)
		itHandles->second->set(nullptr);
	StopSounds();
	g_client = nullptr;
	m_modelManager->Clear();
	GetMaterialManager().ClearUnused();
	ecs::CParticleSystemComponent::ClearCache();
}

void pragma::ClientState::implFindSimilarConVars(const std::string &input, std::vector<SimilarCmdInfo> &similarCmds) const
{
	NetworkState::implFindSimilarConVars(input, similarCmds);

	auto *clMap = console::client::get_convar_map();
	FindSimilarConVars(input, clMap->GetConVars(), similarCmds);

	auto *svMap = console::server::get_convar_map();
	FindSimilarConVars(input, svMap->GetConVars(), similarCmds);
}

void pragma::ClientState::RegisterServerConVar(std::string scmd, unsigned int id)
{
	std::unordered_map<std::string, unsigned int>::iterator i = m_conCommandIDs.find(scmd);
	if(i != m_conCommandIDs.end())
		return;
	m_conCommandIDs.insert(std::unordered_map<std::string, unsigned int>::value_type(scmd, id));
}

bool pragma::ClientState::RunConsoleCommand(std::string scmd, std::vector<std::string> &argv, BasePlayerComponent *pl, KeyState pressState, float magnitude, const std::function<bool(console::ConConf *, float &)> &callback)
{
	auto *clMap = console::client::get_convar_map();
	auto conVarCl = clMap->GetConVar(scmd);
	auto bUseClientside = (conVarCl != nullptr) ? (conVarCl->GetType() != console::ConType::Variable || argv.empty()) : false; // Console commands are ALWAYS executed clientside, if they exist clientside
	if(bUseClientside == false) {
		auto *svMap = console::server::get_convar_map();
		auto conVarSv = svMap->GetConVar(scmd);
		if(conVarSv == nullptr) {
			auto it = m_conCommandIDs.find(scmd);
			if(it == m_conCommandIDs.end()) // No serverside command exists
				bUseClientside = true;
		}
		else if(callback != nullptr && callback(conVarSv.get(), magnitude) == false)
			return true;
	}
	if(bUseClientside == true)
		return NetworkState::RunConsoleCommand(scmd, argv, pl, pressState, magnitude, callback);

	if(m_client == nullptr) {
		// No client exists and this is probably a serverside command.
		// In this case, we'll redirect the command to the server directly
		// (if this is a locally hosted game)
		auto *svState = get_cengine()->GetServerNetworkState();
		if(svState)
			svState->RunConsoleCommand(scmd, argv, nullptr, pressState, magnitude, nullptr);
		return true;
	}
	NetPacket p;
	p->WriteString(scmd);
	p->Write<uint8_t>(static_cast<uint8_t>(pressState));
	p->Write<float>(magnitude);
	p->Write<unsigned char>(CUChar(argv.size()));
	for(unsigned char i = 0; i < argv.size(); i++)
		p->WriteString(argv[i]);
	SendPacket(networking::net_messages::server::CMD_CALL, p, networking::Protocol::SlowReliable);
	return true;
}

pragma::console::ConVar *pragma::ClientState::SetConVar(std::string scmd, std::string value, bool bApplyIfEqual)
{
	console::ConVar *cvar = NetworkState::SetConVar(scmd, value, bApplyIfEqual);
	if(cvar == nullptr)
		return nullptr;
	auto flags = cvar->GetFlags();
	if(((flags & console::ConVarFlags::Userinfo) == console::ConVarFlags::Userinfo)) {
		NetPacket p;
		p->WriteString(scmd);
		p->WriteString(cvar->GetString());
		SendPacket(networking::net_messages::server::CVAR_SET, p, networking::Protocol::SlowReliable);
	}
	return cvar;
}

void pragma::ClientState::Draw(rendering::DrawSceneInfo &drawSceneInfo) //const Vulkan::RenderPass &renderPass,const Vulkan::Framebuffer &framebuffer,const Vulkan::CommandBuffer &drawCmd); // prosper TODO
{
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::get_domain().BeginTask("draw_game_scenes");
#endif
	if(m_game != nullptr)
		GetGameState()->RenderScenes(drawSceneInfo);
	/*else // If game is NULL, that means render target has not been used in any render pass and we must transition the image layout ourselves
	{
		auto img = rt->GetTexture().lock()->GetImage().lock();
		.RecordImageBarrier(
			*drawCmd,*img,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,vk::PipelineStageFlagBits::eTransfer,
			prosper::ImageLayout::ColorAttachmentOptimal,prosper::ImageLayout::TransferSrcOptimal,
			vk::AccessFlagBits::eColorAttachmentWrite,vk::AccessFlagBits::eTransferRead
		);
	}*/
	CallCallbacks("Draw"); // Don't call this more than once to prevent infinite loops
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::get_domain().EndTask();
#endif
}

void pragma::ClientState::Render(rendering::DrawSceneInfo &drawSceneInfo, std::shared_ptr<prosper::RenderTarget> &rt)
{
	auto &drawCmd = drawSceneInfo.commandBuffer;
	drawSceneInfo.outputImage = rt->GetTexture().GetImage().shared_from_this();
	CallCallbacks<void, std::reference_wrapper<const rendering::DrawSceneInfo>, std::reference_wrapper<std::shared_ptr<prosper::RenderTarget>>>("PreRender", std::ref(drawSceneInfo), std::ref(rt));
	if(m_game != nullptr) {
		auto &context = get_cengine()->GetRenderContext();
		context.GetPipelineLoader().Flush(); // Make sure all shaders have been loaded and initialized

		m_game->CallCallbacks<void, std::reference_wrapper<const rendering::DrawSceneInfo>, std::reference_wrapper<std::shared_ptr<prosper::RenderTarget>>>("PreRender", std::ref(drawSceneInfo), std::ref(rt));
		m_game->CallLuaCallbacks("PreRender");
	}
	Draw(drawSceneInfo);
	if(m_game != nullptr) {
		m_game->CallCallbacks<void, std::reference_wrapper<const rendering::DrawSceneInfo>, std::reference_wrapper<std::shared_ptr<prosper::RenderTarget>>>("PostRender", std::ref(drawSceneInfo), std::ref(rt));
		m_game->CallLuaCallbacks("PostRender");
	}
	CallCallbacks<void, std::reference_wrapper<const rendering::DrawSceneInfo>, std::reference_wrapper<std::shared_ptr<prosper::RenderTarget>>>("PostRender", std::ref(drawSceneInfo), std::ref(rt));
}

void pragma::ClientState::Think()
{
	NetworkState::Think();
	if(m_client != nullptr && m_client->IsRunning()) {
		networking::Error err;
		m_client->PollEvents(err);
		if(m_client->IsDisconnected() == true)
			Disconnect();
	}
}

void pragma::ClientState::Tick() { NetworkState::Tick(); }

void pragma::ClientState::EndGame()
{
	if(!IsGameActive())
		return;
	CallCallbacks<void, CGame *>("EndGame", GetGameState());
	m_game->CallCallbacks<void>("EndGame");

	// Game::OnRemove requires that NetworkState::GetGameState returns
	// a valid game instance, but this is not the case if we destroy
	// m_game directly. Instead we move it into a temporary
	// variable and destroy that instead.
	// TODO: This is really ugly, do it another way!
	auto game = std::move(m_game);
	m_game = {game.get(), [](Game *) {}};
	game = nullptr;
	m_game = nullptr;

	NetworkState::EndGame();
	m_conCommandIDs.clear();
	if(m_hMainMenu.IsValid()) {
		gui::types::WIMainMenu *menu = m_hMainMenu.get<gui::types::WIMainMenu>();
		menu->SetNewGameMenu();
	}
}

bool pragma::ClientState::IsGameActive() { return m_game != nullptr; }

pragma::CGame *pragma::ClientState::GetGameState() { return static_cast<CGame *>(NetworkState::GetGameState()); }

void pragma::ClientState::HandleLuaNetPacket(NetPacket &packet)
{
	if(!IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(GetGameState());
	game->HandleLuaNetPacket(packet);
}

bool pragma::ClientState::ShouldRemoveSound(audio::ALSound &snd) { return (NetworkState::ShouldRemoveSound(snd) && snd.GetIndex() == 0) ? true : false; }

std::shared_ptr<pragma::audio::ALSound> pragma::ClientState::GetSoundByIndex(unsigned int idx)
{
	auto *snd = audio::CALSound::FindByServerIndex(idx);
	if(snd == nullptr)
		return nullptr;
	return snd->downcasted_shared_from_this<audio::ALSound>();
}

void pragma::ClientState::Disconnect()
{
	if(m_client != nullptr) {
		networking::Error err;
		if(m_client->Disconnect(err) == false)
			Con::CWAR << "Unable to disconnect from server: " << err.GetMessage() << Con::endl;
		DestroyClient();
	}
}

void pragma::ClientState::DestroyClient()
{
	m_client = nullptr;
	m_svInfo = nullptr;
}

bool pragma::ClientState::IsConnected() const { return (m_client != nullptr) ? true : false; }

pragma::networking::CLNetMessage *pragma::ClientState::GetNetMessage(unsigned int ID)
{
	auto *map = networking::get_client_message_map();
	return map->GetNetMessage(ID);
}

pragma::networking::ClientMessageMap *pragma::ClientState::GetNetMessageMap() { return networking::get_client_message_map(); }

bool pragma::ClientState::IsClient() const { return true; }

bool pragma::ClientState::LoadGUILuaFile(std::string f)
{
	f = fs::get_normalized_path(f);
	return (Lua::LoadFile(GetGUILuaState(), f) == Lua::StatusCode::Ok) ? true : false;
}

void pragma::ClientState::SendUserInfo()
{
	Con::CCL << "Sending user info..." << Con::endl;

	NetPacket packet;
	auto &version = get_engine_version();
	packet->Write<util::Version>(version);

	auto udpPort = m_client->GetLocalUDPPort();
	if(IsConnected() && udpPort.has_value()) {
		packet->Write<unsigned char>(1);
		packet->Write<unsigned short>(*udpPort);
	}
	else
		packet->Write<unsigned char>((unsigned char)(0));

	auto name = GetConVarString("playername");
	auto libSteamworks = GetLibraryModule("steamworks/pr_steamworks");
	if(libSteamworks) {
		auto *fGetClientName = libSteamworks->FindSymbolAddress<void (*)(std::string &)>("pr_steamworks_get_client_steam_name");
		if(fGetClientName)
			fGetClientName(name);
	}
	// TODO: Allow client to override steam user name?
	packet->WriteString(name);

	auto *client = get_client_state();
	auto &convars = client->GetConVars();
	unsigned int numUserInfo = 0;
	auto sz = packet->GetOffset();
	packet->Write<unsigned int>((unsigned int)(0));
	for(auto &pair : convars) {
		auto &cv = pair.second;
		if(cv->GetType() == console::ConType::Var) {
			auto *cvar = static_cast<console::ConVar *>(cv.get());
			if((cvar->GetFlags() & console::ConVarFlags::Userinfo) == console::ConVarFlags::Userinfo && cvar->GetString() != cvar->GetDefault()) {
				packet->WriteString(pair.first);
				packet->WriteString(cvar->GetString());
				numUserInfo++;
			}
		}
	}
	packet->Write<unsigned int>(numUserInfo, &sz);
	client->SendPacket(networking::net_messages::server::CLIENTINFO, packet, networking::Protocol::SlowReliable);
}

std::string pragma::ClientState::GetMessagePrefix() const { return std::string {Con::PREFIX_CLIENT}; }

void pragma::ClientState::StartGame(bool) { StartNewGame(""); }
void pragma::ClientState::StartNewGame(const std::string &gameMode)
{
	EndGame();
	m_game = {new CGame {this}, [](Game *game) {
		          game->OnRemove();
		          delete game;
	          }};
	m_game->SetGameMode(gameMode);
	CallCallbacks<void, CGame *>("OnGameStart", GetGameState());
	m_game->Initialize();
	m_game->OnInitialized();
	//if(!IsConnected())
	//	RequestServerInfo(); // Deprecated; Now handled through NET_cl_map_ready
	CloseMainMenu();
	if(m_hMainMenu.IsValid()) {
		gui::types::WIMainMenu *menu = m_hMainMenu.get<gui::types::WIMainMenu>();
		menu->SetContinueMenu();
	}
}

pragma::console::ConVarMap *pragma::ClientState::GetConVarMap() { return console::client::get_convar_map(); }

bool pragma::ClientState::IsMultiPlayer() const { return get_cengine()->IsMultiPlayer(); }
bool pragma::ClientState::IsSinglePlayer() const { return get_cengine()->IsSinglePlayer(); }

pragma::material::MaterialManager &pragma::ClientState::GetMaterialManager() { return *get_cengine()->GetClientStateInstance().materialManager; }
pragma::geometry::ModelSubMesh *pragma::ClientState::CreateSubMesh() const { return new geometry::CModelSubMesh; }
pragma::geometry::ModelMesh *pragma::ClientState::CreateMesh() const { return new geometry::CModelMesh; }

static auto cvMatStreaming = pragma::console::get_client_con_var("cl_material_streaming_enabled");
pragma::material::Material *pragma::ClientState::LoadMaterial(const std::string &path, bool precache, bool bReload)
{
	if(spdlog::get_level() <= spdlog::level::debug)
		spdlog::debug("Loading material '{}'...", path);
	return LoadMaterial(path, nullptr, bReload, !precache /*!cvMatStreaming->GetBool()*/);
}

static void init_shader(pragma::material::Material *mat)
{
	if(mat == nullptr)
		return;
	auto *info = mat->GetShaderInfo();
	if(info != nullptr) {
		auto shader = pragma::get_cengine()->GetShader(info->GetIdentifier());
		const_cast<pragma::util::ShaderInfo *>(info)->SetShader(pragma::util::make_shared<pragma::util::WeakHandle<prosper::Shader>>(shader));
	}
}
pragma::material::MaterialHandle pragma::ClientState::CreateMaterial(const std::string &path, const std::string &shader)
{
	auto settings = datasystem::create_data_settings({});
	auto mat = GetMaterialManager().CreateMaterial(path, shader, pragma::util::make_shared<datasystem::Block>(*settings));
	if(mat == nullptr)
		return {};
	static_cast<material::CMaterial *>(mat.get())->SetOnLoadedCallback(std::bind(init_shader, mat.get()));
	return mat->GetHandle();
}

pragma::material::MaterialHandle pragma::ClientState::CreateMaterial(const std::string &shader)
{
	auto settings = datasystem::create_data_settings({});
	auto mat = GetMaterialManager().CreateMaterial(shader, pragma::util::make_shared<datasystem::Block>(*settings));
	if(mat == nullptr)
		return {};
	static_cast<material::CMaterial *>(mat.get())->SetOnLoadedCallback(std::bind(init_shader, mat.get()));
	return mat->GetHandle();
}

pragma::util::FileAssetManager *pragma::ClientState::GetAssetManager(asset::Type type)
{
	switch(type) {
	case asset::Type::Texture:
		{
			auto &matManager = static_cast<material::CMaterialManager &>(GetMaterialManager());
			return &matManager.GetTextureManager();
		}
	}
	return NetworkState::GetAssetManager(type);
}

pragma::material::Material *pragma::ClientState::LoadMaterial(const std::string &path, const std::function<void(material::Material *)> &onLoaded, bool bReload, bool bLoadInstantly)
{
	auto &matManager = GetMaterialManager();
	auto success = true;
	material::Material *mat = nullptr;
	if(!bLoadInstantly) {
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
		debug::get_domain().BeginTask("preload_material");
#endif
		success = matManager.PreloadAsset(path);
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
		debug::get_domain().EndTask();
#endif
		return nullptr;
	}
	else if(bReload) {
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
		debug::get_domain().BeginTask("load_material");
#endif
		auto asset = matManager.ReloadAsset(path);
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
		debug::get_domain().EndTask();
#endif
		success = (asset != nullptr);
		mat = asset.get();
	}
	else {
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
		debug::get_domain().BeginTask("load_material");
#endif
		util::FileAssetManager::PreloadResult result {};
		auto asset = matManager.LoadAsset(path, nullptr, &result);
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
		debug::get_domain().EndTask();
#endif
		success = (asset != nullptr);
		mat = asset.get();
		if(!result && !result.firstTimeError)
			return nullptr;
	}

	//bLoadInstantly = true;
	auto bShaderInitialized = pragma::util::make_shared<bool>(false);

	bool bFirstTimeError;
	auto loadInfo = std::make_unique<material::MaterialLoadInfo>();
	auto pmat = static_cast<material::CMaterialManager &>(GetMaterialManager()).LoadAsset(path, std::move(loadInfo));
	mat = pmat.get();
	if(!mat) {
		static auto bSkipPort = false;
		if(bSkipPort == false) {
			bSkipPort = true;
			auto b = PortMaterial(path);
			if(b) {
				// Note: Porting the material may have also ported the texture files.
				// This would've happened AFTER the material has been loaded, however, which
				// means the material's texture references would be invalid.
				// We'll force the material to reload here, to make sure the texture references
				// are up-to-date.
				mat = LoadMaterial(path, onLoaded, true, bLoadInstantly);
			}
			bSkipPort = false;
			if(b == true)
				return mat;
		}
		if(get_cgame())
			get_cgame()->RequestResource(path);
		spdlog::warn("Unable to load material '{}': File not found!", path);
	}
	else {
		{
			// TODO: bShaderInitialized should never be null
			if(bShaderInitialized == nullptr || bShaderInitialized.use_count() > 1) // Callback has been called immediately
				init_shader(mat);
			bShaderInitialized = nullptr;
			CallCallbacks<void, material::CMaterial *>("OnMaterialLoaded", static_cast<material::CMaterial *>(mat));
			if(onLoaded != nullptr)
				onLoaded(mat);
			// Material has been fully loaded!

			std::string ext;
			if(pragma::string::compare<std::string>(mat->GetShaderIdentifier(), "eye", false) && ufile::get_extension(mat->GetName(), &ext) && pragma::string::compare<std::string>(ext, "vmt", false)) {
				// Material was loaded from a VMT and uses the eye shader. In this case we have to save the material as WMI, otherwise
				// we may run into a loop where the eye material would be loaded over and over again because it involves decomposing the eye
				// textures, which triggers the resource watcher.
				// This is a bit of a hack, but it'll do for now. TODO: Do this in a better way!
				auto matName = mat->GetName();
				ufile::remove_extension_from_filename(matName);
				auto savePath = pragma::asset::relative_path_to_absolute_path(matName, asset::Type::Material, util::CONVERT_PATH);
				std::string err;
				mat->Save(savePath.GetString(), err);
			}
		};

		if(bShaderInitialized.use_count() > 1)
			init_shader(mat);
	}
	return mat;
}
pragma::material::Material *pragma::ClientState::LoadMaterial(const std::string &path) { return LoadMaterial(path, nullptr, false); }
pragma::material::Material *pragma::ClientState::LoadMaterial(const std::string &path, const std::function<void(material::Material *)> &onLoaded, bool bReload) { return LoadMaterial(path, onLoaded, bReload, !cvMatStreaming->GetBool()); }

pragma::networking::IClient *pragma::ClientState::GetClient() { return m_client.get(); }

void pragma::ClientState::InitializeResourceManager() { m_resourceWatcher = std::make_unique<util::CResourceWatcherManager>(this); }

void pragma::ClientState::InitializeGUIModule()
{
#ifdef _WIN32
	if(m_lastModuleHandle == nullptr)
		return;
	auto *l = GetGUILuaState();
	if(l != nullptr)
		InitializeDLLModule(l, m_lastModuleHandle);
	l = GetLuaState();
	if(l != nullptr)
		InitializeDLLModule(l, m_lastModuleHandle);
#endif
}

unsigned int pragma::ClientState::GetServerMessageID(std::string identifier)
{
	auto *map = networking::get_server_message_map();
	return map->GetNetMessageID(identifier);
}

unsigned int pragma::ClientState::GetServerConVarID(std::string scmd)
{
	auto *map = console::server::get_convar_map();
	return map->GetConVarID(scmd);
}

bool pragma::ClientState::GetServerConVarIdentifier(uint32_t id, std::string &cvar)
{
	auto *map = console::server::get_convar_map();
	std::string *pCvar = nullptr;
	auto r = map->GetConVarIdentifier(id, &pCvar);
	if(r == true)
		cvar = *pCvar;
	return r;
}

namespace {
	auto UVN = pragma::console::client::register_variable_listener<int>(
	  "sv_tickrate", +[](pragma::NetworkState *, const pragma::console::ConVar &, int, int val) {
		  if(val < 0)
			  val = 0;
		  pragma::get_cengine()->SetTickRate(val);
	  });
}
