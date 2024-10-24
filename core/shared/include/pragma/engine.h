/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __ENGINE_H__
#define __ENGINE_H__
#include "pragma/definitions.h"
#include "pragma/lua/luaapi.h"
#include "pragma/console/cvar_handler.h"
#include "pragma/console/debugconsole.h"
#include <sharedutils/chronotime.h>
#include <fsys/vfileptr.h>
#include "pragma/debug/mdump.h"
#include "pragma/input/key_state.hpp"
#include "pragma/engine_info.hpp"
#include "pragma/iserverstate.hpp"
#include "pragma/types.hpp"
#include <materialmanager.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <sharedutils/callback_handler.h>
#include <sharedutils/scope_guard.h>
#include <sharedutils/util_parallel_job.hpp>

#ifdef _DEBUG
#define ENGINE_DEFAULT_TICK_RATE 33
#else
#define ENGINE_DEFAULT_TICK_RATE 60
#endif

class NetworkState;
class ServerState;
class VFilePtrInternalReal;
class PtrConVar;
class NetPacket;
namespace uzip {
	class ZIPFile;
};
class ConVarMap;
struct Color;
namespace Con {
	enum class MessageFlags : uint8_t;
};
namespace pragma::pad {
	class PackageManager;
};
namespace util {
	class ParallelJobWrapper;
	class FileAssetManager;
};
namespace pragma::asset {
	class AssetManager;
};
namespace pragma::debug {
	class CPUProfiler;
	template<class TProfilingStage>
	class ProfilingStageManager;
	class ProfilingStage;
};
enum class NwStateType : uint8_t { Client = 0, Server, Count };
class DLLNETWORK Engine : public CVarHandler, public CallbackHandler {
  public:
	static const uint32_t DEFAULT_TICK_RATE;
	// For internal use only! Not to be used directly!
  private:
	// Note: m_libServer needs to be the first member, to ensure it's destroyed last!
	mutable std::shared_ptr<util::Library> m_libServer = nullptr;
	mutable pragma::IServerState m_iServerState;
  public:
	struct DLLNETWORK ConVarInfoList {
		using ConVarArgs = std::vector<std::string>;
		struct DLLNETWORK ConVarInfo {
			std::string cmd;
			ConVarArgs args;
		};
		ConVarArgs *Find(const std::string &cmd);
		void Add(const std::string &cmd, const ConVarArgs &args);
		std::vector<ConVarInfo> &GetConVars() { return m_cvars; }
		const std::vector<ConVarInfo> &GetConVars() const { return m_cvars; }
	  private:
		std::vector<ConVarInfo> m_cvars;
		// Only contains last convar, used for fast lookups
		std::unordered_map<std::string, ConVarArgs> m_cvarMap;
	};

	virtual std::unordered_map<std::string, std::shared_ptr<PtrConVar>> &GetConVarPtrs();
	static ConVarHandle GetConVarHandle(std::string scvar);
	//
	class DLLNETWORK StateInstance {
	  public:
		~StateInstance();
		StateInstance(const std::shared_ptr<msys::MaterialManager> &matManager, Material *matErr);
		std::shared_ptr<msys::MaterialManager> materialManager;
		std::unique_ptr<NetworkState> state;
	};
	struct DLLNETWORK ConsoleOutput {
		std::string output;
		Con::MessageFlags messageFlags;
		std::shared_ptr<Color> color;
	};
	enum class ConsoleType : uint8_t { None = 0, Terminal, GUI, GUIDetached };

	virtual std::unique_ptr<ConVarInfoList> &GetConVarConfig(NwStateType type);
  protected:
	bool ExecConfig(const std::string &cfg, const std::function<void(std::string &, std::vector<std::string> &)> &callback);
	bool ExecConfig(const std::string &cfg, ConVarInfoList &infoList);
	void ExecCommands(ConVarInfoList &cmds);
	void PreloadConfig(StateInstance &instance, const std::string &configName);
	virtual void PreloadConfig(NwStateType type, const std::string &configName);
	std::unique_ptr<StateInstance> m_svInstance;
	std::unique_ptr<ConVarInfoList> m_svConfig;
	bool m_bMountExternalGameResources = true;

	std::atomic<bool> m_bRecordConsoleOutput = false;
	std::mutex m_consoleOutputMutex = {};
  public:
	Engine(int argc, char *argv[]);
	virtual ~Engine();

	enum class StateFlags : uint32_t {
		None = 0u,
		Verbose = 1u,
		Running = Verbose << 1u,
		Initialized = Running << 1u,
		DeveloperMode = Initialized << 1u,
		Closed = DeveloperMode << 1u,
		MultiThreadedAssetLoadingEnabled = Closed << 1u,
		RunUpdaterOnClose = MultiThreadedAssetLoadingEnabled << 1u,
		ConsoleSubsystem = RunUpdaterOnClose << 1u,
		CLIOnly = ConsoleSubsystem << 1u,
		NonInteractiveMode = CLIOnly << 1u,
	};
  public:
	DEBUGCONSOLE;
	virtual bool Initialize(int argc, char *argv[]);
	virtual void Start();
	void AddLaunchConVar(std::string cvar, std::string val);
	virtual void DumpDebugInformation(uzip::ZIPFile &zip) const;
	static std::unique_ptr<uzip::ZIPFile> GenerateEngineDump(const std::string &baseName, std::string &outZipFileName, std::string &outErr);
	virtual void Close();
	virtual void Release();
	virtual void ClearConsole();
	virtual void HandleOpenGLFallback() {};
	void ClearCache();

	void SetRunUpdaterOnClose(bool run);
	bool ShouldRunUpdaterOnClose() const;

	uint32_t ClearUnusedAssets(pragma::asset::Type type, bool verbose = false) const;
	uint32_t ClearUnusedAssets(const std::vector<pragma::asset::Type> &types, bool verbose = false) const;
	virtual void SetAssetMultiThreadedLoadingEnabled(bool enabled);
	void UpdateAssetMultiThreadedLoadingEnabled();

	// Debug
	pragma::debug::CPUProfiler &GetProfiler() const;
	pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage> *GetProfilingStageManager();
	CallbackHandle AddProfilingHandler(const std::function<void(bool)> &handler);
	void SetProfilingEnabled(bool bEnabled);
	bool StartProfilingStage(const char *stage);
	bool StopProfilingStage();

	pragma::pad::PackageManager *GetPADPackageManager() const;

	void SetVerbose(bool bVerbose);
	bool IsVerbose() const;

	void SetConsoleSubsystem(bool consoleSubsystem);
	bool IsConsoleSubsystem() const;

	void SetDeveloperMode(bool devMode);
	bool IsDeveloperModeEnabled() const;

	void SetNonInteractiveMode(bool nonInteractiveMode);
	bool IsNonInteractiveMode() const;

	void SetCLIOnly(bool cliOnly);
	bool IsCLIOnly() const;

	// Console
	void ConsoleInput(const std::string_view &line);
	void ProcessConsoleInput(const std::string_view &line, KeyState pressState = KeyState::Press, float magnitude = 1.f);
	// Lua
	virtual NetworkState *GetNetworkState(lua_State *l);
	virtual Lua::Interface *GetLuaInterface(lua_State *l);

	// Log
	void StartLogging();
	void EndLogging();
	void WriteToLog(const std::string &str);
	// Config
	virtual void LoadConfig();
	void LoadServerConfig();
	void SaveServerConfig();
	void SaveEngineConfig();
	// Util
	bool IsRunning();
	std::string GetDate(const std::string &format = "%Y-%m-%d %X");
	uint64_t GetTickCount() const;
	double GetTickTime() const;
	const long long &GetLastTick() const;
	long long GetDeltaTick() const;
	UInt32 GetTickRate() const;
	void SetTickRate(UInt32 tickRate);
	bool IsGameActive();
	virtual bool IsServerOnly();
	virtual bool IsClientConnected();
	virtual void EndGame();
	// Convars
	virtual ConVarMap *GetConVarMap() override;
	virtual std::string GetConVarString(const std::string &cv);
	virtual int GetConVarInt(const std::string &cv);
	virtual float GetConVarFloat(const std::string &cv);
	virtual bool GetConVarBool(const std::string &cv);
	virtual ConConf *GetConVar(const std::string &cv);
	template<class T>
	T *GetConVar(const std::string &cv);
	virtual bool RunConsoleCommand(std::string cmd, std::vector<std::string> &argv, KeyState pressState = KeyState::Press, float magnitude = 1.f, const std::function<bool(ConConf *, float &)> &callback = nullptr);
	// NetState
	virtual NetworkState *GetActiveState();

	virtual StateInstance &GetStateInstance(NetworkState &nw);
	StateInstance &GetServerStateInstance();

	std::optional<ConsoleOutput> PollConsoleOutput();
	void SetRecordConsoleOutput(bool record);
	virtual void SetConsoleType(ConsoleType type);
	virtual ConsoleType GetConsoleType() const;

	virtual bool IsMultiPlayer() const;
	bool IsSinglePlayer() const;
	bool IsActiveState(NetworkState *state);
	bool IsServerRunning();
	// ServerState
	ServerState *GetServerState() const;
	// Same as GetServerState, but returns base pointer
	NetworkState *GetServerNetworkState() const;
	ServerState *OpenServerState();
	virtual NetworkState *GetClientState() const;
	void CloseServerState();
	void StartServer(bool singlePlayer);
	void CloseServer();
	// Returns the steam id of the active listen server
	std::optional<uint64_t> GetServerSteamId() const;

	std::thread::id GetMainThreadId() const;
	void InitializeAssetManager(util::FileAssetManager &assetManager) const;

	virtual void StartNewGame(const std::string &map, bool singlePlayer);

	// When run in game-client: Starts a new single-player game, loads the specified map and connects the local client to the local server.
	// When run in dedicated-server: Starts a new game, loads the specified map and automatically starts a listener server.
	virtual void StartDefaultGame(const std::string &map);

	const pragma::IServerState &GetServerStateInterface() const;

	// Config
	bool ExecConfig(const std::string &cfg);

	void HandleLocalHostPlayerServerPacket(NetPacket &p);
	virtual void HandleLocalHostPlayerClientPacket(NetPacket &p);
	bool ConnectLocalHostPlayerClient();

	void SetMountExternalGameResources(bool b);
	bool ShouldMountExternalGameResources() const;

	int32_t GetRemoteDebugging() const;

	void ShutDown();
	void AddParallelJob(const util::ParallelJobWrapper &job, const std::string &jobName);

	void LockResourceWatchers();
	void UnlockResourceWatchers();
	util::ScopeGuard ScopeLockResourceWatchers();
	void PollResourceWatchers();

	pragma::asset::AssetManager &GetAssetManager();
	const pragma::asset::AssetManager &GetAssetManager() const;

	void AddTickEvent(const std::function<void()> &ev);

	// For internal use only
	void SetReplicatedConVar(const std::string &cvar, const std::string &val);
  protected:
	void UpdateParallelJobs();
	bool RunEngineConsoleCommand(std::string cmd, std::vector<std::string> &argv, KeyState pressState = KeyState::Press, float magnitude = 1.f, const std::function<bool(ConConf *, float &)> &callback = nullptr);
	void WriteServerConfig(VFilePtrReal f);
	void WriteEngineConfig(VFilePtrReal f);
	void RestoreConVarsForUnknownCommands(VFilePtrReal f, const ConVarInfoList &origCvarValues, const std::map<std::string, std::shared_ptr<ConConf>> &stateConVars);
	void RegisterSharedConsoleCommands(ConVarMap &map);
	void RunTickEvents();
	virtual uint32_t DoClearUnusedAssets(pragma::asset::Type type) const;
	virtual void RegisterConsoleCommands();
	virtual void UpdateTickCount();
	struct DLLNETWORK LaunchCommand {
		LaunchCommand(const std::string &cmd, const std::vector<std::string> &args);
		std::string command;
		std::vector<std::string> args;
	};
	std::vector<LaunchCommand> m_launchCommands;
	virtual void RunLaunchCommands();
	virtual void InitializeExternalArchiveManager();

	// Console
	struct ConsoleInstance {
		ConsoleInstance();
		~ConsoleInstance();
		std::unique_ptr<DebugConsole> console;
		std::unique_ptr<std::thread> consoleThread;
	};
	std::unique_ptr<ConsoleInstance> m_consoleInfo = nullptr;
	ConsoleType m_consoleType = ConsoleType::Terminal;
	std::queue<std::string> m_consoleInput;
	std::mutex m_consoleInputMutex;

	std::queue<ConsoleOutput> m_consoleOutput = {};
	void ProcessConsoleInput(KeyState pressState = KeyState::Press);

	std::thread::id m_mainThreadId;
	unsigned int m_tickRate;
	ChronoTime m_ctTick;
	long long m_lastTick;
	uint64_t m_tickCount = 0;
	std::shared_ptr<VFilePtrInternalReal> m_logFile;
	std::unique_ptr<pragma::asset::AssetManager> m_assetManager;

	struct JobInfo {
		util::ParallelJobWrapper job = {};
		std::string name = "";
		float lastProgress = 0.f;
		std::optional<float> timeRemaining = {};
		std::chrono::steady_clock::time_point lastProgressUpdate = {};
		std::chrono::steady_clock::time_point lastNotification = {};
		std::chrono::seconds notificationFrequency = std::chrono::seconds {10};
	};
	// Background tasks that usually take a long time to complete and run on a separate thread
	std::vector<JobInfo> m_parallelJobs {};
	std::mutex m_parallelJobMutex = {};

	std::shared_ptr<pragma::debug::CPUProfiler> m_cpuProfiler;
	std::vector<CallbackHandle> m_profileHandlers = {};

	std::queue<std::function<void()>> m_tickEventQueue;
	std::mutex m_tickEventQueueMutex;
	StateFlags m_stateFlags;
	mutable pragma::pad::PackageManager *m_padPackageManager = nullptr;
	std::unique_ptr<pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage>> m_profilingStageManager;

	std::unordered_map<std::string, std::function<void(int, char *[])>> m_launchOptions;

	void InitLaunchOptions(int argc, char *argv[]);
	virtual void Think();
	virtual void Tick();
};
REGISTER_BASIC_BITWISE_OPERATORS(Engine::StateFlags)

namespace pragma {
	DLLNETWORK Engine *get_engine();
	DLLNETWORK ServerState *get_server_state();
};

template<class T>
T *Engine::GetConVar(const std::string &scvar)
{
	ConConf *cv = GetConVar(scvar);
	if(cv == NULL)
		return NULL;
	return static_cast<T *>(cv);
}

#endif
