#include "stdafx_client.h"
#include "pragma/clientstate/clientutil.h"
#include "pragma/game/c_game.h"
#include <pragma/console/convars.h>
#include "pragma/networking/netmessages.h"
#include "cmaterialmanager.h"
#include "pragma/rendering/shaders/c_shader.h"
#include "pragma/model/c_modelmanager.h"
#include "pragma/lua/classes/c_ldef_wgui.h"
#include "pragma/gui/mainmenu/wimainmenu.h"
#include "pragma/c_engine.h"
#include "pragma/physics/pxvisualizer.h"
#include "pragma/console/convarhandle.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/audio/c_soundscript.h"
#include "luasystem.h"
#include "pragma/gui/wgui_luainterface.h"
#include "pragma/lua/libraries/c_lgui.h"
#include "pragma/lua/libraries/c_lengine.h"
#include "pragma/util/c_resource_watcher.hpp"
#include "pragma/networking/wvclient.h"
#include "pragma/console/c_cvar.h"
#include "pragma/gui/wifps.h"
#include <texturemanager/texturemanager.h>
#include "pragma/lua/classes/c_lwibase.h"
#include <pragma/lua/lua_error_handling.hpp>
#include <luasystem_file.h>
#include <pragma/networking/resources.h>
#include <pragma/engine_version.h>
#include <luainterface.hpp>
#include <alsoundsystem.hpp>
#include <sharedutils/util_shaderinfo.hpp>
#include <prosper_util.hpp>
#include <prosper_command_buffer.hpp>

static std::unordered_map<std::string,std::shared_ptr<PtrConVar>> *conVarPtrs = NULL;
std::unordered_map<std::string,std::shared_ptr<PtrConVar>> &ClientState::GetConVarPtrs() {return *conVarPtrs;}
ConVarHandle ClientState::GetConVarHandle(std::string scvar)
{
	if(conVarPtrs == NULL)
	{
		static std::unordered_map<std::string,std::shared_ptr<PtrConVar>> ptrs;
		conVarPtrs = &ptrs;
	}
	return NetworkState::GetConVarHandle(*conVarPtrs,scvar);
}

DLLCENGINE CEngine *c_engine;
DLLCLIENT ClientState *client = NULL;
extern CGame *c_game;

WIHandle *WGUIHandleFactory(WIBase *el)
{
	// Class specific handles have to also be defined in CGame::InitializeGUIElement!
	const std::type_info &info = typeid(*el);
	if(info == typeid(WIShape))
		return new WIShapeHandle(new PtrWI(el));
	else if(info == typeid(WITexturedShape) || info == typeid(WITexturedRect))
		return new WITexturedShapeHandle(new PtrWI(el));
	else if(info == typeid(WIText))
		return new WITextHandle(new PtrWI(el));
	else if(info == typeid(WITextEntry))
		return new WITextEntryHandle(new PtrWI(el));
	else if(info == typeid(WIOutlinedRect))
		return new WIOutlinedRectHandle(new PtrWI(el));
	else if(info == typeid(WILine))
		return new WILineHandle(new PtrWI(el));
	else if(info == typeid(WIRoundedRect))
		return new WIRoundedRectHandle(new PtrWI(el));
	else if(info == typeid(WIRoundedTexturedRect))
		return new WIRoundedTexturedRectHandle(new PtrWI(el));
	else if(info == typeid(WIScrollBar))
		return new WIScrollBarHandle(new PtrWI(el));
	else if(info == typeid(WISilkIcon))
		return new WISilkIconHandle(new PtrWI(el));
	else if(info == typeid(WIDropDownMenu))
		return new WIDropDownMenuHandle(new PtrWI(el));
	else if(info == typeid(WICheckbox))
		return new WICheckboxHandle(new PtrWI(el));
	else if(info == typeid(WIButton))
		return new WIButtonHandle(new PtrWI(el));
	return NULL;
}

ClientState::ClientState()
	: NetworkState(),m_client(nullptr),m_svInfo(nullptr),m_resDownload(nullptr),
	m_game(NULL),m_volMaster(1.f),
	m_hMainMenu(),m_luaGUI(NULL)
{
	client = this;
	m_soundScriptManager = std::make_unique<CSoundScriptManager>();
	auto &gui = WGUI::GetInstance();
	gui.SetHandleFactory(WGUIHandleFactory);
	gui.SetCreateCallback(WGUILuaInterface::InitializeGUIElement);
	//CVarHandler::Initialize();
	FileManager::AddCustomMountDirectory("downloads",static_cast<fsys::SearchFlags>(FSYS_SEARCH_RESOURCES));

	RegisterCallback<void,CGame*>("OnGameStart");
	RegisterCallback<void,CGame*>("EndGame");


	RegisterCallback<void>("Draw");
	RegisterCallback<
		void,std::reference_wrapper<std::shared_ptr<prosper::PrimaryCommandBuffer>>,
		std::reference_wrapper<std::shared_ptr<prosper::RenderTarget>>
	>("PreRender");
	RegisterCallback<
			void,std::reference_wrapper<std::shared_ptr<prosper::PrimaryCommandBuffer>>,
			std::reference_wrapper<std::shared_ptr<prosper::RenderTarget>>
	>("PostRender");
	RegisterCallback<void,std::reference_wrapper<NetPacket>>("OnReceivePacket");
	RegisterCallback<void,std::reference_wrapper<NetPacket>>("OnSendPacketTCP");
	RegisterCallback<void,std::reference_wrapper<NetPacket>>("OnSendPacketUDP");
}

ClientState::~ClientState()
{
	Disconnect();
	FileManager::RemoveCustomMountDirectory("downloads");
}

static auto cvSteamAudioEnabled = GetClientConVar("cl_steam_audio_enabled");
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
	c_engine->LoadClientConfig();
	InitializeGUILua();
	auto &gui = WGUI::GetInstance();
	m_hMainMenu = gui.Create<WIMainMenu>()->GetHandle();

#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys != nullptr)
		soundSys->SetSteamAudioEnabled(cvSteamAudioEnabled->GetBool()); // See also CGame::ReloadSoundCache
#endif
}

void ClientState::ShowFPSCounter(bool b)
{
	if(b == true)
	{
		if(m_hFps.IsValid())
			return;
		auto *pFps = WGUI::GetInstance().Create<WIFPS>();
		if(pFps == nullptr)
			return;
		pFps->SetPos(10,10);
		pFps->SetZPos(2000);
		m_hFps = pFps->GetHandle();
		return;
	}
	if(!m_hFps.IsValid())
		return;
	m_hFps->Remove();
}

REGISTER_CONVAR_CALLBACK_CL(cl_show_fps,[](NetworkState*,ConVar*,bool,bool val) {
	if(client == nullptr)
		return;
	client->ShowFPSCounter(val);
});

lua_State *ClientState::GetGUILuaState() {return (m_luaGUI != nullptr) ? m_luaGUI->GetState() : nullptr;}
Lua::Interface &ClientState::GetGUILuaInterface() {return *m_luaGUI;}

void ClientState::InitializeGUILua()
{
	m_luaGUI = std::make_shared<Lua::Interface>();
	m_luaGUI->Open();
	m_luaGUI->SetIdentifier("gui");
	Lua::initialize_lua_state(GetGUILuaInterface());

	Lua::RegisterLibrary(GetGUILuaState(),"util",{});
	NetworkState::RegisterSharedLuaClasses(GetGUILuaInterface());
	NetworkState::RegisterSharedLuaLibraries(GetGUILuaInterface());
	ClientState::RegisterSharedLuaClasses(*m_luaGUI,true);
	ClientState::RegisterSharedLuaLibraries(*m_luaGUI,true);
	NetworkState::RegisterSharedLuaGlobals(GetGUILuaInterface());
	ClientState::RegisterSharedLuaGlobals(*m_luaGUI);

	Lua::RegisterLibrary(GetGUILuaState(),"time",{
		{"last_think",Lua_gui_LastThink},
		{"real_time",Lua_gui_RealTime},
		{"delta_time",Lua_gui_DeltaTime}
	});

	Lua::RegisterLibrary(GetGUILuaState(),"engine",{
		{"create_font",&Lua::engine::create_font},
		{"get_font",&Lua::engine::get_font}
	});

	WGUILuaInterface::Initialize();

	Lua::ExecuteFiles(GetGUILuaState(),"autorun\\gui\\",Lua::HandleTracebackError,[this](Lua::StatusCode code,const std::string &luaFile) {
		Lua::HandleSyntaxError(GetGUILuaState(),code,luaFile);
	});
}

void ClientState::AddGUILuaWrapperFactory(const std::function<luabind::object(lua_State*,WIBase*)> &f) {m_guiLuaWrapperFactories.push_back(f);}
std::vector<std::function<luabind::object(lua_State*,WIBase*)>> &ClientState::GetGUILuaWrapperFactories() {return m_guiLuaWrapperFactories;}

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
	auto w = c_engine->GetWindowWidth();
	auto h = c_engine->GetWindowHeight();
	auto &window = c_engine->GetWindow();
	window.SetCursorPos(Vector2i(w /2,h /2));
	window.SetCursorInputMode(GLFW::CursorMode::Hidden);
}
void ClientState::OpenMainMenu()
{
	WIMainMenu *menu = GetMainMenu();
	if(menu == NULL)
		return;
	auto &window = c_engine->GetWindow();
	window.SetCursorInputMode(GLFW::CursorMode::Normal);
	menu->SetVisible(true);
}
void ClientState::ToggleMainMenu()
{
	WIMainMenu *menu = GetMainMenu();
	if(menu == NULL)
		return;
	if(menu->IsVisible())
	{
		if(!IsGameActive())
			return;
		CloseMainMenu();
	}
	else
		OpenMainMenu();
}

void ClientState::Close()
{
	c_engine->SaveClientConfig();
	NetworkState::Close();

	if(m_hMainMenu.IsValid())
		m_hMainMenu->Remove();

	auto *state = m_luaGUI->GetState();
	Lua::gui::clear_lua_callbacks(state);
	WGUILuaInterface::ClearGUILuaObjects(WGUI::GetInstance().GetBaseElement());
	auto identifier = m_luaGUI->GetIdentifier();
	TerminateLuaModules(state);
	m_luaGUI = nullptr;
	DeregisterLuaModules(state,identifier); // Has to be called AFTER Lua instance has been released!
	std::unordered_map<std::string,std::shared_ptr<PtrConVar>> &conVarPtrs = GetConVarPtrs();
	std::unordered_map<std::string,std::shared_ptr<PtrConVar>>::iterator itHandles;
	for(itHandles=conVarPtrs.begin();itHandles!=conVarPtrs.end();itHandles++)
		itHandles->second->set(NULL);
	StopSounds();
	client = NULL;
	CModelManager::Clear();
	GetMaterialManager().ClearUnused();
	pragma::CParticleSystemComponent::ClearCache();
#ifdef PHYS_ENGINE_PHYSX
	PxVisualizer::ClearScene();
#endif
}

void ClientState::implFindSimilarConVars(const std::string &input,std::vector<SimilarCmdInfo> &similarCmds) const
{
	NetworkState::implFindSimilarConVars(input,similarCmds);

	auto *clMap = console_system::client::get_convar_map();
	NetworkState::FindSimilarConVars(input,clMap->GetConVars(),similarCmds);

	auto *svMap = console_system::server::get_convar_map();
	NetworkState::FindSimilarConVars(input,svMap->GetConVars(),similarCmds);
}

void ClientState::RegisterServerConVar(std::string scmd,unsigned int id)
{
	std::unordered_map<std::string,unsigned int>::iterator i = m_conCommandIDs.find(scmd);
	if(i != m_conCommandIDs.end())
		return;
	m_conCommandIDs.insert(std::unordered_map<std::string,unsigned int>::value_type(scmd,id));
}

bool ClientState::RunConsoleCommand(std::string scmd,std::vector<std::string> &argv,pragma::BasePlayerComponent *pl,KeyState pressState,float magnitude,const std::function<bool(ConConf*,float&)> &callback)
{
	auto *clMap = console_system::client::get_convar_map();
	auto conVarCl = clMap->GetConVar(scmd);
	auto bUseClientside = (conVarCl != nullptr) ? (conVarCl->GetType() != ConType::Variable || argv.empty()) : false; // Console commands are ALWAYS executed clientside, if they exist clientside
	if(bUseClientside == false)
	{
		auto *svMap = console_system::server::get_convar_map();
		auto conVarSv = svMap->GetConVar(scmd);
		if(conVarSv == nullptr)
		{
			auto it = m_conCommandIDs.find(scmd);
			if(it == m_conCommandIDs.end()) // No serverside command exists
				bUseClientside = true;
		}
		else if(callback != nullptr && callback(conVarSv.get(),magnitude) == false)
			return true;
	}
	if(bUseClientside == true)
		return NetworkState::RunConsoleCommand(scmd,argv,pl,pressState,magnitude,callback);

	NetPacket p;
	p->WriteString(scmd);
	p->Write<uint8_t>(static_cast<uint8_t>(pressState));
	p->Write<float>(magnitude);
	p->Write<unsigned char>(CUChar(argv.size()));
	for(unsigned char i=0;i<argv.size();i++)
		p->WriteString(argv[i]);
	SendPacketTCP("cmd_call",p);
	return true;
}

ConVar *ClientState::SetConVar(std::string scmd,std::string value,bool bApplyIfEqual)
{
	ConVar *cvar = NetworkState::SetConVar(scmd,value,bApplyIfEqual);
	if(cvar == NULL)
		return NULL;
	auto flags = cvar->GetFlags();
	if(((flags &ConVarFlags::Userinfo) == ConVarFlags::Userinfo))
	{
		NetPacket p;
		p->WriteString(scmd);
		p->WriteString(cvar->GetString());
		SendPacketTCP("cvar_set",p);
	}
	return cvar;
}

void ClientState::Draw(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,std::shared_ptr<prosper::RenderTarget> &rt)//const Vulkan::RenderPass &renderPass,const Vulkan::Framebuffer &framebuffer,const Vulkan::CommandBuffer &drawCmd); // prosper TODO
{
	static auto skip = false;
	if(m_game != nullptr && skip == false)
		m_game->RenderScenes(drawCmd,rt);
	/*else // If game is NULL, that means render target has not been used in any render pass and we must transition the image layout ourselves
	{
		auto img = rt->GetTexture().lock()->GetImage().lock();
		prosper::util::record_image_barrier(
			*drawCmd,*img,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,vk::PipelineStageFlagBits::eTransfer,
			Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL,
			vk::AccessFlagBits::eColorAttachmentWrite,vk::AccessFlagBits::eTransferRead
		);
	}*/
	static auto bDraw = false;
	if(bDraw == false)
	{
		bDraw = true;
		CallCallbacks("Draw"); // Don't call this more than once to prevent infinite loops
		bDraw = false;
	}
}

void ClientState::Render(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,std::shared_ptr<prosper::RenderTarget> &rt)
{
	CallCallbacks<
		void,std::reference_wrapper<std::shared_ptr<prosper::PrimaryCommandBuffer>>,
		std::reference_wrapper<std::shared_ptr<prosper::RenderTarget>>
	>("PreRender",std::ref(drawCmd),std::ref(rt));
	if(m_game != nullptr)
	{
		m_game->CallCallbacks<
			void,std::reference_wrapper<std::shared_ptr<prosper::PrimaryCommandBuffer>>,
			std::reference_wrapper<std::shared_ptr<prosper::RenderTarget>>
		>("PreRender",std::ref(drawCmd),std::ref(rt));
		m_game->CallLuaCallbacks("PreRender");
	}
	Draw(drawCmd,rt);
	if(m_game != nullptr)
	{
		m_game->CallCallbacks<
			void,std::reference_wrapper<std::shared_ptr<prosper::PrimaryCommandBuffer>>,
			std::reference_wrapper<std::shared_ptr<prosper::RenderTarget>>
		>("PostRender",std::ref(drawCmd),std::ref(rt));
		m_game->CallLuaCallbacks("PostRender");
	}
	CallCallbacks<
			void,std::reference_wrapper<std::shared_ptr<prosper::PrimaryCommandBuffer>>,
			std::reference_wrapper<std::shared_ptr<prosper::RenderTarget>>
	>("PostRender",std::ref(drawCmd),std::ref(rt));
}

void ClientState::Think()
{
	NetworkState::Think();
	if(m_client != nullptr && m_client->IsActive())
	{
		m_client->PollEvents();
		if(m_client->IsDisconnected() == true)
			Disconnect();
	}
}

void ClientState::Tick()
{
	NetworkState::Tick();

}

void ClientState::EndGame()
{
	if(!IsGameActive())
		return;
	CallCallbacks<void,CGame*>("EndGame",m_game);
	m_game->CallCallbacks<void>("EndGame");
	delete m_game;
	m_game = NULL;
	c_game = NULL;
	NetworkState::EndGame();
	m_conCommandIDs.clear();
	if(m_hMainMenu.IsValid())
	{
		WIMainMenu *menu = m_hMainMenu.get<WIMainMenu>();
		menu->SetNewGameMenu();
	}
}

bool ClientState::IsGameActive() {return m_game != NULL;}

CGame *ClientState::GetGameState() {return m_game;}

void ClientState::HandleLuaNetPacket(NetPacket &packet)
{
	if(!IsGameActive())
		return;
	CGame *game = static_cast<CGame*>(GetGameState());
	game->HandleLuaNetPacket(packet);
}

bool ClientState::ShouldRemoveSound(ALSound &snd) {return (NetworkState::ShouldRemoveSound(snd) && snd.GetIndex() == 0) ? true : false;}

std::shared_ptr<ALSound> ClientState::GetSoundByIndex(unsigned int idx)
{
	auto *snd = CALSound::FindByServerIndex(idx);
	if(snd == nullptr)
		return nullptr;
	return snd->downcasted_shared_from_this<ALSound>();
}

void ClientState::Disconnect()
{
	if(m_client == nullptr)
		return;
	m_client->Disconnect();
	m_client = nullptr;
}

bool ClientState::IsConnected() const {return (m_client != nullptr) ? true : false;}

CLNetMessage *ClientState::GetNetMessage(unsigned int ID)
{
	ClientMessageMap *map = GetClientMessageMap();
	return map->GetNetMessage(ID);
}

extern DLLNETWORK ClientMessageMap *g_NetMessagesCl;
ClientMessageMap *ClientState::GetNetMessageMap() {return g_NetMessagesCl;}

unsigned short ClientState::GetTCPPort() {return CUChar(GetConVarInt("cl_port_tcp"));}
unsigned short ClientState::GetUDPPort() {return CUChar(GetConVarInt("cl_port_udp"));}

bool ClientState::IsClient() const {return true;}

bool ClientState::LoadGUILuaFile(std::string f)
{
	f = FileManager::GetNormalizedPath(f);
	return (Lua::LoadFile(GetGUILuaState(),f) == Lua::StatusCode::Ok) ? true : false;
}

void ClientState::SendUserInfo()
{
	Con::ccl<<"[CLIENT] Sending user info..."<<Con::endl;

	NetPacket packet;
	auto &version = get_engine_version();
	packet->Write<util::Version>(version);

	if(client->IsUDPOpen())
	{
		packet->Write<unsigned char>(1);
		packet->Write<unsigned short>(client->GetUDPPort());
	}
	else
		packet->Write<unsigned char>((unsigned char)(0));
	packet->WriteString(GetConVarString("playername"));

	auto &convars = client->GetConVars();
	unsigned int numUserInfo = 0;
	auto sz = packet->GetOffset();
	packet->Write<unsigned int>((unsigned int)(0));
	for(auto &pair : convars)
	{
		auto &cv = pair.second;
		if(cv->GetType() == ConType::Var)
		{
			auto *cvar = static_cast<ConVar*>(cv.get());
			if((cvar->GetFlags() &ConVarFlags::Userinfo) == ConVarFlags::Userinfo && cvar->GetString() != cvar->GetDefault())
			{
				packet->WriteString(pair.first);
				packet->WriteString(cvar->GetString());
				numUserInfo++;
			}
		}
	}
	packet->Write<unsigned int>(numUserInfo,&sz);
	client->SendPacketTCP("clientinfo",packet);
}

Lua::ErrorColorMode ClientState::GetLuaErrorColorMode() {return Lua::ErrorColorMode::Magenta;}

void ClientState::StartGame() {StartGame("");}
void ClientState::StartGame(const std::string &gameMode)
{
	NetworkState::StartGame();
	m_game = new CGame(this);
	m_game->SetGameMode(gameMode);
	CallCallbacks<void,CGame*>("OnGameStart",m_game);
	m_game->Initialize();
	//if(!IsConnected())
	//	RequestServerInfo(); // Deprecated; Now handled through NET_cl_map_ready
	CloseMainMenu();
	if(m_hMainMenu.IsValid())
	{
		WIMainMenu *menu = m_hMainMenu.get<WIMainMenu>();
		menu->SetContinueMenu();
	}
}

ConVarMap *ClientState::GetConVarMap() {return console_system::client::get_convar_map();}

bool ClientState::IsMultiPlayer() const {return c_engine->IsMultiPlayer();}
bool ClientState::IsSinglePlayer() const {return c_engine->IsSinglePlayer();}

MaterialManager &ClientState::GetMaterialManager() {return *c_engine->GetClientStateInstance().materialManager;}
ModelSubMesh *ClientState::CreateSubMesh() const {return new CModelSubMesh;}
ModelMesh *ClientState::CreateMesh() const {return new CModelMesh;}

static auto cvMatStreaming = GetClientConVar("cl_material_streaming_enabled");
Material *ClientState::LoadMaterial(const std::string &path,bool bReload)
{
	return LoadMaterial(path,nullptr,bReload,!cvMatStreaming->GetBool());
}

static void init_shader(Material *mat)
{
	if(mat == nullptr)
		return;
	auto *info = mat->GetShaderInfo();
	if(info != nullptr)
	{
		auto shader = c_engine->GetShader(info->GetIdentifier());
		const_cast<util::ShaderInfo*>(info)->SetShader(std::make_shared<::util::WeakHandle<prosper::Shader>>(shader));
	}
}
Material *ClientState::CreateMaterial(const std::string &path,const std::string &shader)
{
	auto *mat = GetMaterialManager().CreateMaterial(path,shader);
	if(mat == nullptr)
		return mat;
	static_cast<CMaterial*>(mat)->SetOnLoadedCallback(std::bind(init_shader,mat));
	return mat;
}

Material *ClientState::CreateMaterial(const std::string &shader)
{
	auto *mat = GetMaterialManager().CreateMaterial(shader);
	if(mat == nullptr)
		return mat;
	static_cast<CMaterial*>(mat)->SetOnLoadedCallback(std::bind(init_shader,mat));
	return mat;
}

Material *ClientState::LoadMaterial(const std::string &path,const std::function<void(Material*)> &onLoaded,bool bReload,bool bLoadInstantly)
{
	//bLoadInstantly = true;
	auto bShaderInitialized = std::make_shared<bool>(false);

	bool bFirstTimeError;
	auto *mat = static_cast<CMaterialManager&>(GetMaterialManager()).Load(path,[onLoaded,bShaderInitialized](Material *mat) mutable {
		if(bShaderInitialized.use_count() > 1) // Callback has been called immediately
			init_shader(mat);
		bShaderInitialized = nullptr;
		if(onLoaded != nullptr)
			onLoaded(mat);
		// Material has been fully loaded!
	},nullptr,bReload,&bFirstTimeError,bLoadInstantly);
	if(bFirstTimeError == true)
	{
		static auto bSkipPort = false;
		if(bSkipPort == false)
		{
			bSkipPort = true;
			auto b = PortMaterial(path,[this,&mat,&onLoaded,bLoadInstantly](const std::string &path,bool bReload) -> Material* {
				return mat = LoadMaterial(path,onLoaded,bReload,bLoadInstantly);
			});
			bSkipPort = false;
			if(b == true)
				return mat;
		}
		c_game->RequestResource(path);
		Con::cwar<<"WARNING: Unable to load material '"<<path<<"': File not found!"<<Con::endl;
	}
	else if(bShaderInitialized.use_count() > 1)
		init_shader(mat);
	return mat;
}
Material *ClientState::LoadMaterial(const std::string &path,const std::function<void(Material*)> &onLoaded,bool bReload) {return LoadMaterial(path,onLoaded,bReload,!cvMatStreaming->GetBool());}
Material *ClientState::LoadMaterial(const std::string &path,bool bLoadInstantly,bool bReload) {return LoadMaterial(path,CallbackHandle(),bReload,bLoadInstantly);}

WVClient *ClientState::GetClient() {return m_client.get();}

void ClientState::InitializeResourceManager() {m_resourceWatcher = std::make_unique<CResourceWatcherManager>(this);}

bool ClientState::IsTCPOpen() const {return (IsConnected() && m_client->HasTCPConnection()) ? true : false;}
bool ClientState::IsUDPOpen() const {return (IsConnected() && m_client->HasUDPConnection()) ? true : false;}

void ClientState::InitializeGUIModule()
{
#ifdef _WIN32
	if(m_lastModuleHandle == nullptr)
		return;
	auto *l = GetGUILuaState();
	if(l != nullptr)
		InitializeDLLModule(l,m_lastModuleHandle);
	l = GetLuaState();
	if(l != nullptr)
		InitializeDLLModule(l,m_lastModuleHandle);
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

bool ClientState::GetServerConVarIdentifier(uint32_t id,std::string &cvar)
{
	auto *map = console_system::server::get_convar_map();
	std::string *pCvar = nullptr;
	auto r = map->GetConVarIdentifier(id,&pCvar);
	if(r == true)
		cvar = *pCvar;
	return r;
}

REGISTER_CONVAR_CALLBACK_CL(sv_tickrate,[](NetworkState*,ConVar*,int,int val) {
	if(val < 0)
		val = 0;
	c_engine->SetTickRate(val);
});
