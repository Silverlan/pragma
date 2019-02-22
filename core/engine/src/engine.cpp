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
#pragma comment(lib,"bzip2.lib")
#pragma comment(lib,"util.lib")
#pragma comment(lib,"mathutil.lib")
#pragma comment(lib,"luasystem.lib")
#pragma comment(lib,"materialsystem.lib")
#pragma comment(lib,"util_zip.lib")
#pragma comment(lib,"pad.lib")
#pragma comment(lib,"util_pragma_doc.lib")
#ifdef _DEBUG
#ifdef PHYS_ENGINE_BULLET
	#pragma comment(lib,"BulletDynamics_Debug.lib")
	#pragma comment(lib,"BulletCollision_Debug.lib")
	#pragma comment(lib,"BulletSoftBody_Debug.lib")
	#pragma comment(lib,"LinearMath_Debug.lib")
#endif
#ifdef PHYS_ENGINE_PHYSX
	#pragma comment(lib,"PhysX3CHECKED_x86.lib")
	#pragma comment(lib,"PhysX3CommonCHECKED_x86.lib")
	#pragma comment(lib,"PhysX3CookingCHECKED_x86.lib")
	#pragma comment(lib,"PhysX3ExtensionsCHECKED.lib")
	#pragma comment(lib,"PhysX3VehicleCHECKED.lib")
	#pragma comment(lib,"PhysXVisualDebuggerSDKCHECKED.lib")
#endif
#else
#ifdef PHYS_ENGINE_BULLET
	#pragma comment(lib,"BulletDynamics_RelWithDebugInfo.lib")
	#pragma comment(lib,"BulletCollision_RelWithDebugInfo.lib")
	#pragma comment(lib,"BulletSoftBody_RelWithDebugInfo.lib")
	#pragma comment(lib,"LinearMath_RelWithDebugInfo.lib")
#endif
#ifdef PHYS_ENGINE_PHYSX
	#pragma comment(lib,"PhysX3_x86.lib")
	#pragma comment(lib,"PhysX3Common_x86.lib")
	#pragma comment(lib,"PhysX3Cooking_x86.lib")
	#pragma comment(lib,"PhysX3Extensions.lib")
	#pragma comment(lib,"PhysX3Vehicle.lib")
	//#pragma comment(lib,"PhysXVisualDebuggerSDK.lib")
#endif
#endif
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
#include "pragma/util/profiling_stages.h"
#include "pragma/engine_version.h"
#include "pragma/console/cvar.h"
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
#ifdef PHYS_ENGINE_PHYSX
	m_physics(NULL),m_pxFoundation(NULL),m_pxCooking(NULL),
#ifdef _DEBUG
	m_pxPvdConnection(NULL),
#endif
#endif
	m_tickRate(Engine::DEFAULT_TICK_RATE)
{
	m_lastTick = static_cast<long long>(m_ctTick());
	engine = this;

	// Link package system to file system
	m_padPackageManager = upad::link_to_file_system();

	pragma::register_engine_animation_events();
	pragma::register_engine_activities();

	RegisterCallback<void>("Think");
}

upad::PackageManager *Engine::GetPADPackageManager() const {return m_padPackageManager;}

void Engine::Close()
{
	m_bRunning = false;
	CloseServerState();

#if PHYS_ENGINE_PHYSX
#ifdef _DEBUG
	ClosePVDConnection();
#endif
	physx::PxCloseVehicleSDK();
	m_physics->release();
	m_pxCooking->release();
	m_pxFoundation->release();
#endif

	CloseConsole();
	EndLogging();

	util::close_external_archive_manager();
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

static CVar cvProfiling = GetEngineConVar("debug_profiling_enabled");
bool Engine::IsProfilingEnabled() const
{
	return cvProfiling->GetBool();
}
void Engine::Tick()
{
	m_ctTick.Update();
	ProcessConsoleInput();
	auto *sv = GetServerState();
	if(sv != NULL)
	{
		auto bProfiling = cvProfiling->GetBool();
		if(bProfiling == true)
			StartStageProfiling(umath::to_integral(ProfilingStage::ServerStateTick));
		sv->Tick();
		if(bProfiling == true)
			EndStageProfiling(umath::to_integral(ProfilingStage::ServerStateTick));
	}
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
	// Initialize Server Instance
	auto matManager = std::make_shared<MaterialManager>();
	auto *matErr = matManager->Load("error");
	m_svInstance = std::unique_ptr<StateInstance>(new StateInstance{matManager,matErr});
	//
	InitLaunchOptions(argc,argv);
#ifdef PHYS_ENGINE_PHYSX
	Con::cout<<"Initializing PhysX..."<<Con::endl;
	static WVPxErrorCallback gDefaultErrorCallback;
	static physx::PxDefaultAllocator gDefaultAllocatorCallback;

	m_pxFoundation = PxCreateFoundation(PX_PHYSICS_VERSION,gDefaultAllocatorCallback,gDefaultErrorCallback);
	if(m_pxFoundation == NULL)
	{
		Con::cerr<<"ERROR: Unable to create PhysX foundation!"<<Con::endl;
		exit(EXIT_FAILURE);
	}
	physx::PxTolerancesScale scale;
	scale.length = 190.5f;
	scale.mass = 1000.f;
	scale.speed = 180.f;
	m_physics = PxCreatePhysics(PX_PHYSICS_VERSION,*m_pxFoundation,scale);
	if(m_physics == NULL)
	{
		Con::cerr<<"ERROR: Unable to create top-level PhysX Object!"<<Con::endl;
		exit(EXIT_FAILURE);
	}
	m_pxCooking = PxCreateCooking(PX_PHYSICS_VERSION,*m_pxFoundation,physx::PxCookingParams(scale));
	if(m_pxCooking == NULL)
	{
		Con::cerr<<"ERROR: Unable to initialize PhysX Cooking!"<<Con::endl;
		exit(EXIT_FAILURE);
	}
	if(!physx::PxInitVehicleSDK(*m_physics))
	{
		Con::cerr<<"ERROR: Unable to initialize PhysX Vehicle SDK!"<<Con::endl;
		exit(EXIT_FAILURE);
	}
	physx::PxVehicleSetBasisVectors(physx::PxVec3(0.f,1.f,0.f),physx::PxVec3(0.f,0.f,1.f));
	physx::PxVehicleSetUpdateMode(physx::PxVehicleUpdateMode::Enum::eACCELERATION);
#ifdef _DEBUG
	OpenPVDConnection();
#endif
#endif
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

#ifdef PHYS_ENGINE_BULLET

#elif PHYS_ENGINE_PHYSX
#ifdef _DEBUG
void Engine::OpenPVDConnection()
{
	const char *host = "127.0.0.1";
	int port = 5425;
	unsigned int timeout = 1;
	OpenPVDConnection(host,port,timeout);
}

struct PVDConnectionHandler : public physx::debugger::comm::PvdConnectionHandler
{
    virtual void onPvdSendClassDescriptions( physx::debugger::comm::PvdConnection& inFactory ) override
    {
        // send your custom PVD class descriptions from here
        // this then allows PVD to correctly identify and represent
        // custom data that is sent from your application to a PvdConnection.
        // example in JointConnectionHandler
    }
    virtual void onPvdConnected( physx::debugger::comm::PvdConnection &con ) override
    {
		Con::cwar<<"Successfully connected to PVD!"<<Con::endl;
        // do something when successfully connected
        // e.g. enable contact and constraint visualization
    }
    virtual void onPvdDisconnected( physx::debugger::comm::PvdConnection&) override
    {
		Con::cwar<<"Disconnected from PVD!"<<Con::endl;
        // handle disconnection
        // e.g. disable contact and constraint visualization
    }
};

void Engine::OpenPVDConnection(const char *host,int port,unsigned int timeout)
{
	if(m_physics->getPvdConnectionManager() == NULL || m_pxPvdConnection != NULL)
		return;
	physx::PxVisualDebuggerConnectionFlags flags = physx::PxVisualDebuggerConnectionFlag::eDEBUG;
	m_pxPvdConnection = physx::PxVisualDebuggerExt::createConnection(m_physics->getPvdConnectionManager(),host,port,timeout,flags);

	PVDConnectionHandler pvdConnectionHandler;
	if(m_physics->getPvdConnectionManager())
		m_physics->getPvdConnectionManager()->addHandler(pvdConnectionHandler);

	m_physics->getVisualDebugger()->setVisualizeConstraints(true);
	m_physics->getVisualDebugger()->setVisualDebuggerFlag(physx::PxVisualDebuggerFlag::eTRANSMIT_CONTACTS,true);
}
void Engine::ClosePVDConnection()
{
	if(m_pxPvdConnection == NULL)
		return;
	if(m_pxPvdConnection->isConnected())
		m_pxPvdConnection->disconnect();
	m_pxPvdConnection->release();
	m_pxPvdConnection = NULL;
}
#endif

physx::PxPhysics *Engine::GetPhysics() {return m_physics;}
physx::PxCooking *Engine::GetCookingLibrary() {return m_pxCooking;}

#endif

void Engine::HandleLocalPlayerServerPacket(NetPacket &p)
{
	if(server == nullptr)
		return;
	server->HandlePacket(server->GetLocalClient(),p);
}
void Engine::HandleLocalPlayerClientPacket(NetPacket&) {}

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
		auto bProfiling = cvProfiling->GetBool();
		if(bProfiling == true)
			StartStageProfiling(umath::to_integral(ProfilingStage::EngineLoop));

		Think();
		loops = 0;
		auto tickRate = GetTickRate();
		auto skipTicks = static_cast<long long>(1000 /tickRate);
		const long long &t = GetTickCount();
		while(t > nextTick && loops < MAX_FRAMESKIP)
		{
			Tick();
			m_lastTick = static_cast<long long>(m_ctTick());
			nextTick += skipTicks;//SKIP_TICKS);
			loops++;
		}
		if(t > nextTick)
			nextTick = t; // This should only happen after loading times

		if(bProfiling == true)
			EndStageProfiling(umath::to_integral(ProfilingStage::EngineLoop));
	}
	while(IsRunning());
	Close();
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

const long long &Engine::GetLastTick() {return m_lastTick;}

long long Engine::GetDeltaTick() {return GetTickCount() -m_lastTick;}

void Engine::Think()
{
	AddonSystem::Poll(); // Required for dynamic mounting of addons
	m_ctTick.Update();
	CallCallbacks<void>("Think");
	auto *sv = GetServerState();
	if(sv != NULL)
	{
		auto bProfiling = cvProfiling->GetBool();
		if(bProfiling == true)
			StartStageProfiling(umath::to_integral(ProfilingStage::ServerStateThink));
		sv->Think();
		if(bProfiling == true)
			EndStageProfiling(umath::to_integral(ProfilingStage::ServerStateThink));
	}
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

Engine::~Engine()
{
	engine = nullptr;
	if(m_bRunning == true)
		throw std::runtime_error("Engine has to be closed before it can be destroyed!");
}

