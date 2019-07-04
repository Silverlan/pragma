#include "stdafx_engine.h"

// Link Libraries
#pragma comment(lib,"vfilesystem.lib")
#pragma comment(lib,"Winmm.lib")
#pragma comment(lib,"shared.lib")
#pragma comment(lib,"server.lib")
#ifdef USE_LUAJIT
	#pragma comment(lib,"lua51.lib")
#else
	#pragma comment(lib,"lua530.lib")
#endif
#pragma comment(lib,"luabind.lib")
#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"bz2.lib")
#pragma comment(lib,"sharedutils.lib")
#pragma comment(lib,"mathutil.lib")
#pragma comment(lib,"luasystem.lib")
#pragma comment(lib,"materialsystem.lib")
#pragma comment(lib,"util_zip.lib")
#pragma comment(lib,"util_pad.lib")
#pragma comment(lib,"util_versioned_archive.lib")
#pragma comment(lib,"util_pragma_doc.lib")
//

#include "pragma/engine.h"
#include <pragma/serverstate/serverstate.h>
#include "pragma/physics/physxcallbacks.h"
#include <pragma/console/convarhandle.h>
#include "luasystem.h"
#include <networkmanager/nwm_packet.h>
#include <pragma/networking/wvlocalclient.h>
#include <pragma/console/convars.h>
#include "pragma/console/engine_cvar.h"
#include "pragma/engine_version.h"
#include "pragma/console/cvar.h"
#include "pragma/debug/debug_performance_profiler.hpp"
#include <sharedutils/util.h>
#include <util_zip.h>
#include <pragma/game/game_resources.hpp>
#include <util_pad.hpp>
#include <pragma/addonsystem/addonsystem.h>
#include <pragma/model/animation/activities.h>
#include <pragma/model/animation/animation_event.h>

decltype(Engine::DEFAULT_TICK_RATE) Engine::DEFAULT_TICK_RATE = ENGINE_DEFAULT_TICK_RATE;

extern "C"
{
	void DLLENGINE RunEngine(int argc,char *argv[])
	{
		auto en = InitializeServer(argc,argv);
		en = nullptr;
	}
}

static std::unordered_map<std::string,std::shared_ptr<PtrConVar>> *conVarPtrs = NULL;
std::unordered_map<std::string,std::shared_ptr<PtrConVar>> &Engine::GetConVarPtrs() {return *conVarPtrs;}
ConVarHandle Engine::GetConVarHandle(std::string scvar)
{
	if(conVarPtrs == NULL)
	{
		static std::unordered_map<std::string,std::shared_ptr<PtrConVar>> ptrs;
		conVarPtrs = &ptrs;
	}
	return CVarHandler::GetConVarHandle(*conVarPtrs,scvar);
}

DLLENGINE Engine *engine = NULL;
extern DLLSERVER ServerState *server;

Engine::Engine(int,char*[])
	: CVarHandler(),m_bRunning(true),m_bInitialized(false),
	m_console(nullptr),m_consoleThread(nullptr),
	m_logFile(nullptr),
	m_tickRate(Engine::DEFAULT_TICK_RATE)
{
	m_lastTick = static_cast<long long>(m_ctTick());
	engine = this;

	// Link package system to file system
	m_padPackageManager = upad::link_to_file_system();

	pragma::register_engine_animation_events();
	pragma::register_engine_activities();

	RegisterCallback<void>("Think");

	Con::set_output_callback([this](const std::string_view &output,Con::MessageFlags flags,const Color *color) {
		if(m_bRecordConsoleOutput == false)
			return;
		m_consoleOutputMutex.lock();
			m_consoleOutput.push({std::string{output},flags,color ? std::make_shared<Color>(*color) : nullptr});
		m_consoleOutputMutex.unlock();
	});
	
	m_cpuProfiler = pragma::debug::CPUProfiler::Create<pragma::debug::CPUProfiler>();
	AddProfilingHandler([this](bool profilingEnabled) {
		if(profilingEnabled == false)
		{
			m_profilingStageManager = nullptr;
			return;
		}
		auto stageFrame = pragma::debug::ProfilingStage::Create(*m_cpuProfiler,"Frame");
		m_profilingStageManager = std::make_unique<pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage,CPUProfilingPhase>>();
		m_profilingStageManager->InitializeProfilingStageManager(*m_cpuProfiler,{
			stageFrame,
			pragma::debug::ProfilingStage::Create(*m_cpuProfiler,"Think",stageFrame.get()),
			pragma::debug::ProfilingStage::Create(*m_cpuProfiler,"Tick",stageFrame.get())
		});
		static_assert(umath::to_integral(CPUProfilingPhase::Count) == 3u,"Added new profiling phase, but did not create associated profiling stage!");
	});
}

void Engine::ClearConsole()
{
	std::system("cls");
}

std::optional<Engine::ConsoleOutput> Engine::PollConsoleOutput()
{
	if(m_bRecordConsoleOutput == false)
		return {};
	m_consoleOutputMutex.lock();
		if(m_consoleOutput.empty())
		{
			m_consoleOutputMutex.unlock();
			return {};
		}
		auto r = m_consoleOutput.front();
		m_consoleOutput.pop();
	m_consoleOutputMutex.unlock();
	return r;
}
void Engine::SetRecordConsoleOutput(bool record)
{
	if(record == m_bRecordConsoleOutput)
		return;
	m_consoleOutputMutex.lock();
		m_bRecordConsoleOutput = record;
		m_consoleOutput = {};
	m_consoleOutputMutex.unlock();
}

CallbackHandle Engine::AddProfilingHandler(const std::function<void(bool)> &handler)
{
	auto hCb = FunctionCallback<void,bool>::Create(handler);
	m_profileHandlers.push_back(hCb);
	return hCb;
}

void Engine::SetProfilingEnabled(bool bEnabled)
{
	for(auto it=m_profileHandlers.begin();it!=m_profileHandlers.end();)
	{
		auto &hCb = *it;
		if(hCb.IsValid() == false)
		{
			it = m_profileHandlers.erase(it);
			continue;
		}
		hCb(bEnabled);
		++it;
	}
}

upad::PackageManager *Engine::GetPADPackageManager() const {return m_padPackageManager;}

void Engine::Close()
{
	m_bRunning = false;
	CloseServerState();

	CloseConsole();
	EndLogging();

	util::close_external_archive_manager();
	Con::set_output_callback(nullptr);
}

void Engine::ClearCache()
{
	FileManager::RemoveDirectory("cache");
}

ServerState *Engine::GetServerState() const
{
	if(m_svInstance == nullptr)
		return nullptr;
	return static_cast<ServerState*>(m_svInstance->state.get());
}

NetworkState *Engine::GetServerNetworkState() const {return static_cast<NetworkState*>(GetServerState());}

void Engine::EndGame()
{
	auto *sv = GetServerState();
	if(sv != nullptr)
		sv->EndGame();
	CloseServer();
}

void Engine::SetMountExternalGameResources(bool b)
{
	m_bMountExternalGameResources = b;
	if(b == true)
		InitializeExternalArchiveManager();
}
bool Engine::ShouldMountExternalGameResources() const {return m_bMountExternalGameResources;}

pragma::debug::CPUProfiler &Engine::GetProfiler() const {return *m_cpuProfiler;}
pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage,Engine::CPUProfilingPhase> *Engine::GetProfilingStageManager() {return m_profilingStageManager.get();}
bool Engine::StartProfilingStage(CPUProfilingPhase stage)
{
	return m_profilingStageManager && m_profilingStageManager->StartProfilerStage(stage);
}
bool Engine::StopProfilingStage(CPUProfilingPhase stage)
{
	return m_profilingStageManager && m_profilingStageManager->StopProfilerStage(stage);
}

void Engine::Tick()
{
	m_ctTick.Update();
	ProcessConsoleInput();

	StartProfilingStage(CPUProfilingPhase::Tick);
	StartProfilingStage(CPUProfilingPhase::ServerTick);
	auto *sv = GetServerState();
	if(sv != NULL)
		sv->Tick();
	StopProfilingStage(CPUProfilingPhase::ServerTick);
	StopProfilingStage(CPUProfilingPhase::Tick);
}

ConVarMap *Engine::GetConVarMap() {return console_system::engine::get_convar_map();}

Engine::StateInstance &Engine::GetServerStateInstance() {return *m_svInstance;}

void Engine::SetVerbose(bool bVerbose) {m_bVerbose = bVerbose;}
bool Engine::IsVerbose() const {return m_bVerbose;}

void Engine::Release()
{
	Close();
}

bool Engine::Initialize(int argc,char *argv[],bool bRunLaunchCommands)
{
	CVarHandler::Initialize();
	RegisterConsoleCommands();

	// Initialize Server Instance
	auto matManager = std::make_shared<MaterialManager>();
	auto *matErr = matManager->Load("error");
	m_svInstance = std::unique_ptr<StateInstance>(new StateInstance{matManager,matErr});
	//
	InitLaunchOptions(argc,argv);
	if(!IsServerOnly())
		LoadConfig();

	auto cacheVersion = util::Version::FromString(GetConVarString("cache_version"));
	auto curVersion = get_engine_version();
	if(curVersion != cacheVersion)
	{
		SetConVar("cache_version",curVersion.ToString());
		ClearCache();
	}
	ServerState *server = OpenServerState();
	if(server != nullptr && IsServerOnly())
		LoadConfig();
	if(bRunLaunchCommands == true)
		RunLaunchCommands();

	return true;
}

void Engine::InitializeExternalArchiveManager() {util::initialize_external_archive_manager(GetServerState());}

bool Engine::Initialize(int argc,char *argv[])
{
	return Initialize(argc,argv,true);
}

void Engine::RunLaunchCommands()
{
	for(auto it=m_launchCommands.rbegin();it!=m_launchCommands.rend();++it)
	{
		auto &cmd = *it;
		RunConsoleCommand(cmd.command,cmd.args);
	}
	m_launchCommands.clear();
}

Lua::Interface *Engine::GetLuaInterface(lua_State *l)
{
	auto *sv = GetServerState();
	if(sv == nullptr)
		return nullptr;
	auto *sg = sv->GetGameState();
	if(sg == nullptr)
		return nullptr;
	if(sg->GetLuaState() == l)
		return &sg->GetLuaInterface();
	return nullptr;
}

NetworkState *Engine::GetNetworkState(lua_State *l)
{
	auto *sv = GetServerState();
	if(sv == NULL)
		return NULL;
	if(sv->GetLuaState() == l)
		return sv;
	return NULL;
}

bool Engine::IsMultiPlayer() const
{
	auto *sv = GetServerState();
	if(sv == nullptr)
		return false;
	return sv->IsMultiPlayer();
}
bool Engine::IsSinglePlayer() const
{
	auto *sv = GetServerState();
	if(sv == nullptr)
		return false;
	return sv->IsSinglePlayer();
}

void Engine::StartServer()
{
	auto *sv = GetServerState();
	if(sv == NULL)
		return;
	sv->StartServer();
}

void Engine::CloseServer()
{
	auto *sv = GetServerState();
	if(sv == NULL)
		return;
	sv->CloseServer();
}

bool Engine::IsClientConnected() {return false;}

bool Engine::IsServerRunning()
{
	auto *sv = GetServerState();
	if(sv == NULL)
		return false;
	return sv->IsServerRunning();
}

void Engine::LoadMap(const char *map)
{
	EndGame();
	auto *sv = GetServerState();
	if(sv == NULL)
		return;
	sv->LoadMap(map);
}

static auto cvRemoteDebugging = GetConVar("sh_lua_remote_debugging");
int32_t Engine::GetRemoteDebugging() const {return cvRemoteDebugging->GetInt();}

extern std::string __lp_map;
extern std::string __lp_gamemode;
static auto cvMountExternalResources = GetConVar("sh_mount_external_game_resources");
void Engine::Start()
{
	if(cvMountExternalResources->GetBool() == true)
		SetMountExternalGameResources(true);
	if(!__lp_gamemode.empty())
	{
		std::vector<std::string> argv = {__lp_gamemode};
		RunConsoleCommand("sv_gamemode",argv);
	}
	if(!__lp_map.empty())
		LoadMap(__lp_map.c_str());

	InvokeConVarChangeCallbacks("steam_steamworks_enabled");

	//const double FRAMES_PER_SECOND = GetTickRate();
	//const double SKIP_TICKS = 1000 /FRAMES_PER_SECOND;
	const int MAX_FRAMESKIP = 5;
	long long nextTick = GetTickCount();
	int loops;
	do {
		StartProfilingStage(CPUProfilingPhase::Think);
		Think();
		StopProfilingStage(CPUProfilingPhase::Think);

		loops = 0;
		auto tickRate = GetTickRate();
		auto skipTicks = static_cast<long long>(1'000 /tickRate);

		auto t = GetTickCount();
		while(t > nextTick && loops < MAX_FRAMESKIP)
		{
			Tick();

			m_lastTick = static_cast<long long>(m_ctTick());
			nextTick += skipTicks;//SKIP_TICKS);
			loops++;
		}
		if(t > nextTick)
			nextTick = t; // This should only happen after loading times
	}
	while(IsRunning());
	Close();
}

void Engine::UpdateTickCount()
{
	m_ctTick.Update();
}

void Engine::DumpDebugInformation(ZIPFile &zip) const
{
	std::stringstream engineInfo;
	engineInfo<<"System: ";
	if(util::is_windows_system())
		engineInfo<<"Windows";
	else
		engineInfo<<"Linux";
	if(util::is_x64_system())
		engineInfo<<" x64";
	else
		engineInfo<<" x86";
	if(engine != nullptr)
		engineInfo<<"\nEngine Version: "<<get_pretty_engine_version();
	auto *nw = static_cast<NetworkState*>(GetServerState());
	if(nw == nullptr)
		nw = GetClientState();
	if(nw != nullptr)
	{
		auto *game = nw->GetGameState();
		if(game != nullptr)
		{
			auto &mapInfo = game->GetMapInfo();
			engineInfo<<"\nMap: "<<mapInfo.name<<" ("<<mapInfo.fileName<<")";
		}
	}
	zip.AddFile("engine.txt",engineInfo.str());
}

const long long &Engine::GetLastTick() const {return m_lastTick;}

long long Engine::GetDeltaTick() const {return GetTickCount() -m_lastTick;}

void Engine::Think()
{
	AddonSystem::Poll(); // Required for dynamic mounting of addons
	UpdateTickCount();
	CallCallbacks<void>("Think");
	auto *sv = GetServerState();
	if(sv != NULL)
		sv->Think();
}

ServerState *Engine::OpenServerState()
{
	CloseServerState();
	m_svInstance->state = std::make_unique<ServerState>();
	auto *sv = GetServerState();
	sv->Initialize();
	return sv;
}

void Engine::CloseServerState()
{
	auto *sv = GetServerState();
	if(sv == nullptr)
		return;
	sv->Close();
	m_svInstance->state = nullptr;
	server = nullptr;
}

NetworkState *Engine::GetClientState() const {return NULL;}

NetworkState *Engine::GetActiveState() {return GetServerState();}

bool Engine::IsActiveState(NetworkState *state) {return state == GetActiveState();}

void Engine::AddLaunchConVar(std::string cvar,std::string val) {m_launchCommands.push_back({cvar,{val}});}

void Engine::ShutDown() {m_bRunning = false;}

void Engine::HandleLocalPlayerClientPacket(NetPacket &p) {}
void Engine::HandleLocalPlayerServerPacket(NetPacket &p)
{
	if(server == nullptr)
		return;
	server->HandlePacket(*server->GetLocalClient(),p);
}

Engine::~Engine()
{
	engine = nullptr;
	if(m_bRunning == true)
		throw std::runtime_error("Engine has to be closed before it can be destroyed!");
}

REGISTER_ENGINE_CONVAR_CALLBACK(debug_profiling_enabled,[](NetworkState*,ConVar*,bool,bool enabled) {
	if(engine == nullptr)
		return;
	engine->SetProfilingEnabled(enabled);
});
