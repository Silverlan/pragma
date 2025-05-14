/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientutil.h"
#include "pragma/game/c_game.h"
#include <pragma/console/convars.h>
#include "pragma/networking/netmessages.h"
#include "cmaterialmanager.h"
#include <cmaterial_manager2.hpp>
#include "pragma/model/c_modelmanager.h"
#include "pragma/lua/classes/c_ldef_wgui.h"
#include "pragma/lua/libraries/c_lglobal.h"
#include "pragma/gui/mainmenu/wimainmenu.h"
#include "pragma/c_engine.h"
#include "pragma/console/convarhandle.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/audio/c_soundscript.h"
#include "luasystem.h"
#include "pragma/gui/wgui_luainterface.h"
#include "pragma/lua/libraries/c_lgui.h"
#include "pragma/lua/libraries/c_lengine.h"
#include "pragma/util/c_resource_watcher.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/console/c_cvar.h"
#include "pragma/gui/wifps.h"
#include <pragma/asset/util_asset.hpp>
#include <pragma/game/game_resources.hpp>
#include <pragma/lua/libraries/lengine.h>
#include <texturemanager/texturemanager.h>
#include "pragma/lua/classes/c_lwibase.h"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include "pragma/networking/local_client.hpp"
#include <pragma/lua/lua_error_handling.hpp>
#include <pragma/lua/libraries/lutil.hpp>
#include <pragma/lua/libraries/ludm.hpp>
#include <luasystem_file.h>
#include <pragma/debug/intel_vtune.hpp>
#include <pragma/networking/enums.hpp>
#include <pragma/networking/error.hpp>
#include <pragma/networking/resources.h>
#include <pragma/networking/networking_modules.hpp>
#include <pragma/engine_version.h>
#include <pragma/logging.hpp>
#include <luainterface.hpp>
#include <alsoundsystem.hpp>
#include <shader/prosper_pipeline_loader.hpp>
#include <sharedutils/util_shaderinfo.hpp>
#include <sharedutils/util_library.hpp>
#include <prosper_util.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_window.hpp>
#include <wgui/types/wiroot.h>


static std::unordered_map<std::string, std::shared_ptr<PtrConVar>> *conVarPtrs = NULL;
std::unordered_map<std::string, std::shared_ptr<PtrConVar>> &ClientState::GetConVarPtrs() { return *conVarPtrs; }
ConVarHandle ClientState::GetConVarHandle(std::string scvar)
{
	if(conVarPtrs == NULL) {
		static std::unordered_map<std::string, std::shared_ptr<PtrConVar>> ptrs;
		conVarPtrs = &ptrs;
	}
	return NetworkState::GetConVarHandle(*conVarPtrs, scvar);
}

extern DLLCLIENT CEngine *c_engine;
DLLCLIENT ClientState *client = NULL;
extern CGame *c_game;

std::vector<std::string> &get_required_game_textures();
ClientState::ClientState() : NetworkState(), m_client(nullptr), m_svInfo(nullptr), m_resDownload(nullptr), m_volMaster(1.f), m_hMainMenu(), m_luaGUI(NULL)
{
	client = this;
	m_soundScriptManager = std::make_unique<CSoundScriptManager>();

	m_modelManager = std::make_unique<pragma::asset::CModelManager>(*this);
	// m_modelManager->SetVerbose(true);
	c_engine->InitializeAssetManager(*m_modelManager);
	pragma::asset::update_extension_cache(pragma::asset::Type::Model);

	auto &gui = WGUI::GetInstance();
	// gui.SetCreateCallback(WGUILuaInterface::InitializeGUIElement);
	//CVarHandler::Initialize();
	FileManager::AddCustomMountDirectory("downloads", static_cast<fsys::SearchFlags>(FSYS_SEARCH_RESOURCES));

	RegisterCallback<void, CGame *>("OnGameStart");
	RegisterCallback<void, CGame *>("EndGame");
	RegisterCallback<void, CMaterial *>("OnMaterialLoaded");

	RegisterCallback<void>("Draw");
	RegisterCallback<void, std::reference_wrapper<const util::DrawSceneInfo>, std::reference_wrapper<std::shared_ptr<prosper::RenderTarget>>>("PreRender");
	RegisterCallback<void, std::reference_wrapper<const util::DrawSceneInfo>, std::reference_wrapper<std::shared_ptr<prosper::RenderTarget>>>("PostRender");
	RegisterCallback<void, std::reference_wrapper<NetPacket>>("OnReceivePacket");
	RegisterCallback<void, std::reference_wrapper<NetPacket>>("OnSendPacketTCP");
	RegisterCallback<void, std::reference_wrapper<NetPacket>>("OnSendPacketUDP");

	auto &texManager = static_cast<msys::CMaterialManager &>(GetMaterialManager()).GetTextureManager();
	for(auto &tex : get_required_game_textures())
		texManager.PreloadAsset(tex);
}

ClientState::~ClientState()
{
	Disconnect();
	FileManager::RemoveCustomMountDirectory("downloads");

	c_engine->GetSoundSystem()->SetOnReleaseSoundCallback(nullptr);

	ClearCommands();
}

void ClientState::UpdateGameWorldShaderSettings()
{
	auto oldSettings = m_worldShaderSettings;
	m_worldShaderSettings.shadowQuality = static_cast<pragma::rendering::GameWorldShaderSettings::ShadowQuality>(GetConVarInt("render_shadow_quality"));
	m_worldShaderSettings.ssaoEnabled = GetConVarBool("cl_render_ssao");
	m_worldShaderSettings.bloomEnabled = GetConVarBool("render_bloom_enabled");
	m_worldShaderSettings.debugModeEnabled = GetConVarBool("render_debug_mode") || GetConVarBool("render_unlit");
	m_worldShaderSettings.fxaaEnabled = static_cast<pragma::rendering::AntiAliasing>(GetConVarInt("cl_render_anti_aliasing")) == pragma::rendering::AntiAliasing::FXAA;
	m_worldShaderSettings.iblEnabled = GetConVarBool("render_ibl_enabled");
	m_worldShaderSettings.dynamicLightingEnabled = GetConVarBool("render_dynamic_lighting_enabled");
	m_worldShaderSettings.dynamicShadowsEnabled = GetConVarBool("render_dynamic_shadows_enabled");
	if(m_worldShaderSettings == oldSettings)
		return;

	auto *game = GetGameState();
	if(game)
		static_cast<CGame *>(game)->OnGameWorldShaderSettingsChanged(m_worldShaderSettings, oldSettings);
}

void ClientState::InitializeGameClient(bool singlePlayerLocalGame)
{
	DestroyClient();

	pragma::networking::ClientEventInterface eventInterface {};
	eventInterface.onConnected = [this]() { HandleConnect(); };
	eventInterface.onDisconnected = []() {};
	eventInterface.onConnectionClosed = []() {};
	//eventInterface.onPacketSent = [](pragma::networking::Protocol protocol,NetPacket &packet) {
	//};
	eventInterface.handlePacket = [this](NetPacket &packet) { HandlePacket(packet); };
	if(singlePlayerLocalGame == false) {
		auto netLibName = GetConVarString("net_library");
		auto netModPath = pragma::networking::GetNetworkingModuleLocation(netLibName, false);
		std::string err;
		auto dllHandle = InitializeLibrary(netModPath, &err);
		if(dllHandle) {
			auto *fInitNetLib = dllHandle->FindSymbolAddress<void (*)(NetworkState &, std::unique_ptr<pragma::networking::IClient> &)>("initialize_game_client");
			if(fInitNetLib != nullptr)
				fInitNetLib(*this, m_client);
			else
				spdlog::error("Unable to initialize networking system '{}': Function 'initialize_game_client' not found in module!", netLibName);
		}
		else
			spdlog::error("Unable to initialize networking system '{}': {}", netLibName, err);
		if(m_client == nullptr)
			ResetGameClient();
	}
	else
		m_client = std::make_unique<pragma::networking::LocalClient>();
	if(m_client)
		m_client->SetEventInterface(eventInterface);
}
void ClientState::ResetGameClient()
{
	//m_client = std::make_unique<pragma::networking::LocalClient>();
}

static auto cvSteamAudioEnabled = GetClientConVar("cl_steam_audio_enabled");
import pragma.string.unicode;
void ClientState::Initialize()
{
	/*Con::cwar<<"Client NetMessages:"<<Con::endl;
	ClientMessageMap *mapMsgs = GetNetMessageMap();
	std::unordered_map<std::string,unsigned int> *msgs;
	mapMsgs->GetNetMessages(&msgs);
	std::unordered_map<std::string,unsigned int>::iterator i;
	for(i=msgs->begin();i!=msgs->end();i++)
	{
		Con::cout<<"Name: "<<i->first<<Con::endl;
	}*/
	NetworkState::Initialize();

	c_engine->GetSoundSystem()->SetOnReleaseSoundCallback([this](const al::SoundSource &snd) {
		auto it = std::find_if(m_sounds.begin(), m_sounds.end(), [&snd](const ALSoundRef &sndOther) { return (static_cast<CALSound *>(&sndOther.get()) == &snd) ? true : false; });
		if(it == m_sounds.end())
			return;
		m_sounds.erase(it);
	});

	c_engine->LoadClientConfig();
	InitializeGUILua();
	auto &gui = WGUI::GetInstance();
	m_hMainMenu = gui.Create<WIMainMenu>()->GetHandle();

	UpdateGameWorldShaderSettings();

#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys != nullptr)
		soundSys->SetSteamAudioEnabled(cvSteamAudioEnabled->GetBool()); // See also CGame::ReloadSoundCache
#endif
}

void ClientState::ShowFPSCounter(bool b)
{
	if(b == true) {
		if(m_hFps.IsValid())
			return;
		auto *pFps = WGUI::GetInstance().Create<WIFPS>();
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

REGISTER_CONVAR_CALLBACK_CL(cl_show_fps, [](NetworkState *, const ConVar &, bool, bool val) {
	if(client == nullptr)
		return;
	client->ShowFPSCounter(val);
});

lua_State *ClientState::GetGUILuaState() { return (m_luaGUI != nullptr) ? m_luaGUI->GetState() : nullptr; }
Lua::Interface &ClientState::GetGUILuaInterface() { return *m_luaGUI; }

//__declspec(dllimport) void test_lua_policies(lua_State *l);
std::optional<std::vector<std::string>> g_autoExecScripts {};

#include "pragma/lua/policies/gui_element_policy.hpp"
void ClientState::InitializeGUILua()
{
	m_luaGUI = std::make_shared<Lua::Interface>();
	m_luaGUI->Open();
	m_luaGUI->SetIdentifier("gui");
	Lua::initialize_lua_state(GetGUILuaInterface());

	auto utilMod = luabind::module(m_luaGUI->GetState(), "util");
	Lua::util::register_shared_generic(m_luaGUI->GetState(), utilMod);
	NetworkState::RegisterSharedLuaClasses(GetGUILuaInterface());
	NetworkState::RegisterSharedLuaLibraries(GetGUILuaInterface());
	ClientState::RegisterSharedLuaClasses(*m_luaGUI, true);
	ClientState::RegisterSharedLuaLibraries(*m_luaGUI, true);
	NetworkState::RegisterSharedLuaGlobals(GetGUILuaInterface());
	ClientState::RegisterSharedLuaGlobals(*m_luaGUI);
	Lua::register_shared_client_state(m_luaGUI->GetState());
	Lua::udm::register_library(*m_luaGUI);
	auto modAsset = luabind::module_(m_luaGUI->GetState(), "asset");
	Lua::asset_client::register_library(*m_luaGUI, modAsset);

	auto timeMod = luabind::module(m_luaGUI->GetState(), "time");
	timeMod[luabind::def("last_think", &Lua::gui::LastThink), luabind::def("real_time", &Lua::gui::RealTime), luabind::def("delta_time", &Lua::gui::DeltaTime)];

	auto enMod = luabind::module(m_luaGUI->GetState(), "engine");
	enMod[luabind::def("poll_console_output", &Lua::engine::poll_console_output)];
	Lua::engine::register_library(GetGUILuaState());

	// Testing
	/*{

		test_lua_policies(GetGUILuaState());

		auto *l = m_luaGUI->GetState();
		auto *el = WGUI::GetInstance().Create<WIRect>();
		auto hEl = el->GetHandle();
		auto hElCast = util::weak_shared_handle_cast<WIBase,WIShape>(hEl);
		auto o = luabind::object{l,hElCast};

		auto *h1 = luabind::object_cast<util::TWeakSharedHandle<WIShape>*>(o);
		auto *h2 = luabind::object_cast<WIShape*>(o);
		auto *h3 = luabind::object_cast<WIBase*>(o);

		luabind::globals(l)["testObject"] = hEl;
		luabind::globals(l)["testObjectCast"] = hElCast;
		Lua::RunString(l,"print(\"Valid: \",testObject:IsValid())","test");
		Lua::RunString(l,"print(\"Valid: \",testObjectCast:IsValid())","test");

		el->Remove();
		h1 = luabind::object_cast<util::TWeakSharedHandle<WIShape>*>(o);
		h2 = luabind::object_cast<WIShape*>(o);
		h3 = luabind::object_cast<WIBase*>(o);

		Lua::RunString(l,"print(\"Valid: \",testObject:IsValid())","test");

		Lua::RunString(l,"print(\"Valid: \",testObjectCast:IsValid())","test");
	}*/

	WGUILuaInterface::Initialize();

	Lua::ExecuteFiles(GetGUILuaState(), "autorun/gui/", Lua::HandleTracebackError, [this](Lua::StatusCode code, const std::string &luaFile) { Lua::HandleSyntaxError(GetGUILuaState(), code, luaFile); });
	if(g_autoExecScripts.has_value()) {
		for(auto &f : *g_autoExecScripts)
			Lua::ExecuteFile(GetGUILuaState(), f, Lua::HandleTracebackError);
	}
}

void ClientState::AddGUILuaWrapperFactory(const std::function<luabind::object(lua_State *, WIBase &)> &f) { m_guiLuaWrapperFactories.push_back(f); }
std::vector<std::function<luabind::object(lua_State *, WIBase &)>> &ClientState::GetGUILuaWrapperFactories() { return m_guiLuaWrapperFactories; }

WIMainMenu *ClientState::GetMainMenu()
{
	if(!m_hMainMenu.IsValid())
		return NULL;
	return m_hMainMenu.get<WIMainMenu>();
}

bool ClientState::IsMainMenuOpen()
{
	WIMainMenu *menu = GetMainMenu();
	if(menu == NULL)
		return false;
	return menu->IsVisible();
}
void ClientState::CloseMainMenu()
{
	WIMainMenu *menu = GetMainMenu();
	if(menu == NULL || !menu->IsVisible())
		return;
	menu->SetVisible(false);
	auto w = c_engine->GetRenderContext().GetWindowWidth();
	auto h = c_engine->GetRenderContext().GetWindowHeight();
	auto &window = c_engine->GetWindow();
	if(window->IsFocused())
		window->SetCursorPos(Vector2i(w / 2, h / 2));
	window->SetCursorInputMode(pragma::platform::CursorMode::Disabled);
}
void ClientState::OpenMainMenu()
{
	WIMainMenu *menu = GetMainMenu();
	if(menu == NULL)
		return;
	auto &window = c_engine->GetWindow();
	window->SetCursorInputMode(pragma::platform::CursorMode::Normal);
	menu->SetVisible(true);
}
void ClientState::ToggleMainMenu()
{
	WIMainMenu *menu = GetMainMenu();
	if(menu == NULL)
		return;
	if(menu->IsVisible()) {
		if(!IsGameActive())
			return;
		CloseMainMenu();
	}
	else
		OpenMainMenu();
}

NwStateType ClientState::GetType() const { return NwStateType::Client; }

void ClientState::Close()
{
	c_engine->GetRenderContext().GetPipelineLoader().Stop();
	c_engine->SaveClientConfig();
	NetworkState::Close();

	if(m_hMainMenu.IsValid())
		m_hMainMenu->Remove();

	auto *state = m_luaGUI->GetState();
	Lua::gui::clear_lua_callbacks(state);
	auto *guiBaseEl = WGUI::GetInstance().GetBaseElement();
	if(guiBaseEl != nullptr)
		WGUILuaInterface::ClearGUILuaObjects(*guiBaseEl);
	auto identifier = m_luaGUI->GetIdentifier();
	TerminateLuaModules(state);
	m_luaGUI = nullptr;
	DeregisterLuaModules(state, identifier); // Has to be called AFTER Lua instance has been released!
	std::unordered_map<std::string, std::shared_ptr<PtrConVar>> &conVarPtrs = GetConVarPtrs();
	std::unordered_map<std::string, std::shared_ptr<PtrConVar>>::iterator itHandles;
	for(itHandles = conVarPtrs.begin(); itHandles != conVarPtrs.end(); itHandles++)
		itHandles->second->set(NULL);
	StopSounds();
	client = NULL;
	m_modelManager->Clear();
	GetMaterialManager().ClearUnused();
	pragma::CParticleSystemComponent::ClearCache();
}

void ClientState::implFindSimilarConVars(const std::string &input, std::vector<SimilarCmdInfo> &similarCmds) const
{
	NetworkState::implFindSimilarConVars(input, similarCmds);

	auto *clMap = console_system::client::get_convar_map();
	NetworkState::FindSimilarConVars(input, clMap->GetConVars(), similarCmds);

	auto *svMap = console_system::server::get_convar_map();
	NetworkState::FindSimilarConVars(input, svMap->GetConVars(), similarCmds);
}

void ClientState::RegisterServerConVar(std::string scmd, unsigned int id)
{
	std::unordered_map<std::string, unsigned int>::iterator i = m_conCommandIDs.find(scmd);
	if(i != m_conCommandIDs.end())
		return;
	m_conCommandIDs.insert(std::unordered_map<std::string, unsigned int>::value_type(scmd, id));
}

bool ClientState::RunConsoleCommand(std::string scmd, std::vector<std::string> &argv, pragma::BasePlayerComponent *pl, KeyState pressState, float magnitude, const std::function<bool(ConConf *, float &)> &callback)
{
	auto *clMap = console_system::client::get_convar_map();
	auto conVarCl = clMap->GetConVar(scmd);
	auto bUseClientside = (conVarCl != nullptr) ? (conVarCl->GetType() != ConType::Variable || argv.empty()) : false; // Console commands are ALWAYS executed clientside, if they exist clientside
	if(bUseClientside == false) {
		auto *svMap = console_system::server::get_convar_map();
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
		auto *svState = c_engine->GetServerNetworkState();
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
	SendPacket("cmd_call", p, pragma::networking::Protocol::SlowReliable);
	return true;
}

ConVar *ClientState::SetConVar(std::string scmd, std::string value, bool bApplyIfEqual)
{
	ConVar *cvar = NetworkState::SetConVar(scmd, value, bApplyIfEqual);
	if(cvar == NULL)
		return NULL;
	auto flags = cvar->GetFlags();
	if(((flags & ConVarFlags::Userinfo) == ConVarFlags::Userinfo)) {
		NetPacket p;
		p->WriteString(scmd);
		p->WriteString(cvar->GetString());
		SendPacket("cvar_set", p, pragma::networking::Protocol::SlowReliable);
	}
	return cvar;
}

void ClientState::Draw(util::DrawSceneInfo &drawSceneInfo) //const Vulkan::RenderPass &renderPass,const Vulkan::Framebuffer &framebuffer,const Vulkan::CommandBuffer &drawCmd); // prosper TODO
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

void ClientState::Render(util::DrawSceneInfo &drawSceneInfo, std::shared_ptr<prosper::RenderTarget> &rt)
{
	auto &drawCmd = drawSceneInfo.commandBuffer;
	drawSceneInfo.outputImage = rt->GetTexture().GetImage().shared_from_this();
	CallCallbacks<void, std::reference_wrapper<const util::DrawSceneInfo>, std::reference_wrapper<std::shared_ptr<prosper::RenderTarget>>>("PreRender", std::ref(drawSceneInfo), std::ref(rt));
	if(m_game != nullptr) {
		auto &context = c_engine->GetRenderContext();
		context.GetPipelineLoader().Flush(); // Make sure all shaders have been loaded and initialized

		m_game->CallCallbacks<void, std::reference_wrapper<const util::DrawSceneInfo>, std::reference_wrapper<std::shared_ptr<prosper::RenderTarget>>>("PreRender", std::ref(drawSceneInfo), std::ref(rt));
		m_game->CallLuaCallbacks("PreRender");
	}
	Draw(drawSceneInfo);
	if(m_game != nullptr) {
		m_game->CallCallbacks<void, std::reference_wrapper<const util::DrawSceneInfo>, std::reference_wrapper<std::shared_ptr<prosper::RenderTarget>>>("PostRender", std::ref(drawSceneInfo), std::ref(rt));
		m_game->CallLuaCallbacks("PostRender");
	}
	CallCallbacks<void, std::reference_wrapper<const util::DrawSceneInfo>, std::reference_wrapper<std::shared_ptr<prosper::RenderTarget>>>("PostRender", std::ref(drawSceneInfo), std::ref(rt));
}

void ClientState::Think()
{
	NetworkState::Think();
	if(m_client != nullptr && m_client->IsRunning()) {
		pragma::networking::Error err;
		m_client->PollEvents(err);
		if(m_client->IsDisconnected() == true)
			Disconnect();
	}
}

void ClientState::Tick() { NetworkState::Tick(); }

void ClientState::EndGame()
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

	c_game = nullptr;
	NetworkState::EndGame();
	m_conCommandIDs.clear();
	if(m_hMainMenu.IsValid()) {
		WIMainMenu *menu = m_hMainMenu.get<WIMainMenu>();
		menu->SetNewGameMenu();
	}
}

bool ClientState::IsGameActive() { return m_game != nullptr; }

CGame *ClientState::GetGameState() { return static_cast<CGame *>(NetworkState::GetGameState()); }

void ClientState::HandleLuaNetPacket(NetPacket &packet)
{
	if(!IsGameActive())
		return;
	CGame *game = static_cast<CGame *>(GetGameState());
	game->HandleLuaNetPacket(packet);
}

bool ClientState::ShouldRemoveSound(ALSound &snd) { return (NetworkState::ShouldRemoveSound(snd) && snd.GetIndex() == 0) ? true : false; }

std::shared_ptr<ALSound> ClientState::GetSoundByIndex(unsigned int idx)
{
	auto *snd = CALSound::FindByServerIndex(idx);
	if(snd == nullptr)
		return nullptr;
	return snd->downcasted_shared_from_this<ALSound>();
}

void ClientState::Disconnect()
{
	if(m_client != nullptr) {
		pragma::networking::Error err;
		if(m_client->Disconnect(err) == false)
			Con::cwar << "Unable to disconnect from server: " << err.GetMessage() << Con::endl;
		DestroyClient();
	}
}

void ClientState::DestroyClient()
{
	m_client = nullptr;
	m_svInfo = nullptr;
}

bool ClientState::IsConnected() const { return (m_client != nullptr) ? true : false; }

CLNetMessage *ClientState::GetNetMessage(unsigned int ID)
{
	ClientMessageMap *map = GetClientMessageMap();
	return map->GetNetMessage(ID);
}

extern DLLNETWORK ClientMessageMap *g_NetMessagesCl;
ClientMessageMap *ClientState::GetNetMessageMap() { return g_NetMessagesCl; }

bool ClientState::IsClient() const { return true; }

bool ClientState::LoadGUILuaFile(std::string f)
{
	f = FileManager::GetNormalizedPath(f);
	return (Lua::LoadFile(GetGUILuaState(), f) == Lua::StatusCode::Ok) ? true : false;
}

void ClientState::SendUserInfo()
{
	Con::ccl << "Sending user info..." << Con::endl;

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

	auto &convars = client->GetConVars();
	unsigned int numUserInfo = 0;
	auto sz = packet->GetOffset();
	packet->Write<unsigned int>((unsigned int)(0));
	for(auto &pair : convars) {
		auto &cv = pair.second;
		if(cv->GetType() == ConType::Var) {
			auto *cvar = static_cast<ConVar *>(cv.get());
			if((cvar->GetFlags() & ConVarFlags::Userinfo) == ConVarFlags::Userinfo && cvar->GetString() != cvar->GetDefault()) {
				packet->WriteString(pair.first);
				packet->WriteString(cvar->GetString());
				numUserInfo++;
			}
		}
	}
	packet->Write<unsigned int>(numUserInfo, &sz);
	client->SendPacket("clientinfo", packet, pragma::networking::Protocol::SlowReliable);
}

std::string ClientState::GetMessagePrefix() const { return std::string {Con::PREFIX_CLIENT}; }

void ClientState::StartGame(bool) { StartNewGame(""); }
void ClientState::StartNewGame(const std::string &gameMode)
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
		WIMainMenu *menu = m_hMainMenu.get<WIMainMenu>();
		menu->SetContinueMenu();
	}
}

ConVarMap *ClientState::GetConVarMap() { return console_system::client::get_convar_map(); }

bool ClientState::IsMultiPlayer() const { return c_engine->IsMultiPlayer(); }
bool ClientState::IsSinglePlayer() const { return c_engine->IsSinglePlayer(); }

msys::MaterialManager &ClientState::GetMaterialManager() { return *c_engine->GetClientStateInstance().materialManager; }
ModelSubMesh *ClientState::CreateSubMesh() const { return new CModelSubMesh; }
ModelMesh *ClientState::CreateMesh() const { return new CModelMesh; }

static auto cvMatStreaming = GetClientConVar("cl_material_streaming_enabled");
Material *ClientState::LoadMaterial(const std::string &path, bool precache, bool bReload)
{
	if(spdlog::get_level() <= spdlog::level::debug)
		spdlog::debug("Loading material '{}'...", path);
	return LoadMaterial(path, nullptr, bReload, !precache /*!cvMatStreaming->GetBool()*/);
}

static void init_shader(Material *mat)
{
	if(mat == nullptr)
		return;
	auto *info = mat->GetShaderInfo();
	if(info != nullptr) {
		auto shader = c_engine->GetShader(info->GetIdentifier());
		const_cast<util::ShaderInfo *>(info)->SetShader(std::make_shared<::util::WeakHandle<prosper::Shader>>(shader));
	}
}
msys::MaterialHandle ClientState::CreateMaterial(const std::string &path, const std::string &shader)
{
	auto settings = ds::create_data_settings({});
	auto mat = GetMaterialManager().CreateMaterial(path, shader, std::make_shared<ds::Block>(*settings));
	if(mat == nullptr)
		return mat;
	static_cast<CMaterial *>(mat.get())->SetOnLoadedCallback(std::bind(init_shader, mat.get()));
	return mat;
}

msys::MaterialHandle ClientState::CreateMaterial(const std::string &shader)
{
	auto settings = ds::create_data_settings({});
	auto mat = GetMaterialManager().CreateMaterial(shader, std::make_shared<ds::Block>(*settings));
	if(mat == nullptr)
		return mat;
	static_cast<CMaterial *>(mat.get())->SetOnLoadedCallback(std::bind(init_shader, mat.get()));
	return mat;
}

util::FileAssetManager *ClientState::GetAssetManager(pragma::asset::Type type)
{
	switch(type) {
	case pragma::asset::Type::Texture:
		{
			auto &matManager = static_cast<msys::CMaterialManager &>(GetMaterialManager());
			return &matManager.GetTextureManager();
		}
	}
	return NetworkState::GetAssetManager(type);
}

Material *ClientState::LoadMaterial(const std::string &path, const std::function<void(Material *)> &onLoaded, bool bReload, bool bLoadInstantly)
{
	auto &matManager = GetMaterialManager();
	auto success = true;
	Material *mat = nullptr;
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
	auto bShaderInitialized = std::make_shared<bool>(false);

	bool bFirstTimeError;
	auto loadInfo = std::make_unique<msys::MaterialLoadInfo>();
	loadInfo->onLoaded = [this, bShaderInitialized, onLoaded](util::Asset &asset) mutable {
		// TODO: bShaderInitialized should never be null, but for some reason is!
		auto mat = msys::CMaterialManager::GetAssetObject(asset);
		if(!mat)
			return;
		if(bShaderInitialized == nullptr || bShaderInitialized.use_count() > 1) // Callback has been called immediately
			init_shader(mat.get());
		bShaderInitialized = nullptr;
		CallCallbacks<void, CMaterial *>("OnMaterialLoaded", static_cast<CMaterial *>(mat.get()));
		if(onLoaded != nullptr)
			onLoaded(mat.get());
		// Material has been fully loaded!

		std::string ext;
		if(ustring::compare<std::string>(mat->GetShaderIdentifier(), "eye", false) && ufile::get_extension(mat->GetName(), &ext) && ustring::compare<std::string>(ext, "vmt", false)) {
			// Material was loaded from a VMT and uses the eye shader. In this case we have to save the material as WMI, otherwise
			// we may run into a loop where the eye material would be loaded over and over again because it involves decomposing the eye
			// textures, which triggers the resource watcher.
			// This is a bit of a hack, but it'll do for now. TODO: Do this in a better way!
			auto matName = mat->GetName();
			ufile::remove_extension_from_filename(matName);
			auto savePath = pragma::asset::relative_path_to_absolute_path(matName, pragma::asset::Type::Material, util::CONVERT_PATH);
			std::string err;
			mat->Save(savePath.GetString(), err);
		}
	};
	auto pmat = static_cast<msys::CMaterialManager &>(GetMaterialManager()).LoadAsset(path, std::move(loadInfo));
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
		if(c_game)
			c_game->RequestResource(path);
		spdlog::warn("Unable to load material '{}': File not found!", path);
	}
	else if(bShaderInitialized.use_count() > 1)
		init_shader(mat);
	return mat;
}
Material *ClientState::LoadMaterial(const std::string &path) { return LoadMaterial(path, nullptr, false); }
Material *ClientState::LoadMaterial(const std::string &path, const std::function<void(Material *)> &onLoaded, bool bReload) { return LoadMaterial(path, onLoaded, bReload, !cvMatStreaming->GetBool()); }

pragma::networking::IClient *ClientState::GetClient() { return m_client.get(); }

void ClientState::InitializeResourceManager() { m_resourceWatcher = std::make_unique<CResourceWatcherManager>(this); }

void ClientState::InitializeGUIModule()
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

unsigned int ClientState::GetServerMessageID(std::string identifier)
{
	ServerMessageMap *map = GetServerMessageMap();
	return map->GetNetMessageID(identifier);
}

unsigned int ClientState::GetServerConVarID(std::string scmd)
{
	ConVarMap *map = console_system::server::get_convar_map();
	return map->GetConVarID(scmd);
}

bool ClientState::GetServerConVarIdentifier(uint32_t id, std::string &cvar)
{
	auto *map = console_system::server::get_convar_map();
	std::string *pCvar = nullptr;
	auto r = map->GetConVarIdentifier(id, &pCvar);
	if(r == true)
		cvar = *pCvar;
	return r;
}

REGISTER_CONVAR_CALLBACK_CL(sv_tickrate, [](NetworkState *, const ConVar &, int, int val) {
	if(val < 0)
		val = 0;
	c_engine->SetTickRate(val);
});
