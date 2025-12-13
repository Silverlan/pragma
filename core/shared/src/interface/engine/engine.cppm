// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:engine;

export import :assets.manager;
export import :console.convar;
export import :console.convar_handle;
export import :console.cvar_handler;
export import :console.debug_console;
export import :debug.performance_profiler;
export import :engine.enums;
export import :engine.info;
export import :engine.launch_para_map;
export import :engine.version;
export import :input.enums;
export import :util.server_state_interface;
export import pragma.pad;
export import util_zip;

export {
#ifdef _DEBUG
	constexpr uint32_t ENGINE_DEFAULT_TICK_RATE = 33;
#else
	constexpr uint32_t ENGINE_DEFAULT_TICK_RATE = 60;
#endif

	namespace pragma {
		class NetworkState;
		class DLLNETWORK Engine : public console::CVarHandler, public util::CallbackHandler {
		  public:
			static const uint32_t DEFAULT_TICK_RATE;
			// For internal use only! Not to be used directly!
		  private:
			// Note: m_libServer needs to be the first member, to ensure it's destroyed last!
			mutable std::shared_ptr<util::Library> m_libServer = nullptr;
			mutable IServerState m_iServerState;
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

			virtual std::unordered_map<std::string, std::shared_ptr<console::PtrConVar>> &GetConVarPtrs() override;
			static console::ConVarHandle GetConVarHandle(std::string scvar);
			//
			class DLLNETWORK StateInstance {
			  public:
				~StateInstance();
				StateInstance(const std::shared_ptr<material::MaterialManager> &matManager, material::Material *matErr);
				std::shared_ptr<material::MaterialManager> materialManager;
				std::unique_ptr<NetworkState> state;
			};
			struct DLLNETWORK ConsoleOutput {
				std::string output;
				console::MessageFlags messageFlags;
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
			static Engine *Get();
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
				UseLinenoise = NonInteractiveMode << 1u, // Linux only
				ManagedByPackageManager = UseLinenoise << 1u,
				Sandboxed = ManagedByPackageManager << 1u,
			};
		  public:
			virtual void OpenConsole();
			virtual void CloseConsole();
			void ToggleConsole();
			virtual bool IsConsoleOpen() const;
			console::DebugConsole *GetConsole();

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

			virtual bool IsProgramInFocus() const;

			void SetRunUpdaterOnClose(bool run);
			bool ShouldRunUpdaterOnClose() const;

			uint32_t ClearUnusedAssets(asset::Type type, bool verbose = false) const;
			uint32_t ClearUnusedAssets(const std::vector<asset::Type> &types, bool verbose = false) const;
			virtual void SetAssetMultiThreadedLoadingEnabled(bool enabled);
			void UpdateAssetMultiThreadedLoadingEnabled();

			// Debug
			debug::CPUProfiler &GetProfiler() const;
			debug::ProfilingStageManager<debug::ProfilingStage> *GetProfilingStageManager();
			CallbackHandle AddProfilingHandler(const std::function<void(bool)> &handler);
			void SetProfilingEnabled(bool bEnabled);
			bool StartProfilingStage(const char *stage);
			bool StopProfilingStage();

			pad::PackageManager *GetPADPackageManager() const;

			void SetVerbose(bool bVerbose);
			bool IsVerbose() const;

			void SetConsoleSubsystem(bool consoleSubsystem);
			bool IsConsoleSubsystem() const;

			void SetDeveloperMode(bool devMode);
			bool IsDeveloperModeEnabled() const;

			void SetNonInteractiveMode(bool nonInteractiveMode);
			bool IsNonInteractiveMode() const;

			void SetLinenoiseEnabled(bool enabled);
			bool IsLinenoiseEnabled() const;

			void SetManagedByPackageManager(bool isManaged);
			bool IsManagedByPackageManager() const;

			void SetSandboxed(bool sandboxed);
			bool IsSandboxed() const;

			void SetCLIOnly(bool cliOnly);
			bool IsCLIOnly() const;

			// Console
			void ConsoleInput(const std::string_view &line, bool printLine = true);
			void ProcessConsoleInput(const std::string_view &line, KeyState pressState = KeyState::Press, float magnitude = 1.f);
			// Lua
			virtual NetworkState *GetNetworkState(lua::State *l);
			virtual Lua::Interface *GetLuaInterface(lua::State *l);

			// Log
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
			virtual console::ConVarMap *GetConVarMap() override;
			virtual std::string GetConVarString(const std::string &cv);
			virtual int GetConVarInt(const std::string &cv);
			virtual float GetConVarFloat(const std::string &cv);
			virtual bool GetConVarBool(const std::string &cv);
			virtual console::ConConf *GetConVar(const std::string &cv);
			template<class T>
			T *GetConVar(const std::string &cv);
			virtual bool RunConsoleCommand(std::string cmd, std::vector<std::string> &argv, KeyState pressState = KeyState::Press, float magnitude = 1.f, const std::function<bool(console::ConConf *, float &)> &callback = nullptr);
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
			// Same as GetServerState, but returns base pointer
			NetworkState *GetServerNetworkState() const;
			NetworkState *OpenServerState();
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

			const IServerState &GetServerStateInterface() const;

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

			asset::AssetManager &GetAssetManager();
			const asset::AssetManager &GetAssetManager() const;

			void AddTickEvent(const std::function<void()> &ev);

			// For internal use only
			void SetReplicatedConVar(const std::string &cvar, const std::string &val);
		  protected:
			void UpdateParallelJobs();
			bool RunEngineConsoleCommand(std::string cmd, std::vector<std::string> &argv, KeyState pressState = KeyState::Press, float magnitude = 1.f, const std::function<bool(console::ConConf *, float &)> &callback = nullptr);
			void WriteServerConfig(fs::VFilePtrReal f);
			void WriteEngineConfig(fs::VFilePtrReal f);
			void RestoreConVarsForUnknownCommands(fs::VFilePtrReal f, const ConVarInfoList &origCvarValues, const std::map<std::string, std::shared_ptr<console::ConConf>> &stateConVars);
			void RegisterSharedConsoleCommands(console::ConVarMap &map);
			void RunTickEvents();
			virtual uint32_t DoClearUnusedAssets(asset::Type type) const;
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
				std::unique_ptr<console::DebugConsole> console;
				std::unique_ptr<std::thread> consoleThread;
			};
			std::unique_ptr<ConsoleInstance> m_consoleInfo = nullptr;
			ConsoleType m_consoleType = ConsoleType::Terminal;

			struct ConsoleInputInfo {
				std::string line;
				bool printLine = false;
			};
			std::queue<ConsoleInputInfo> m_consoleInput;
			std::mutex m_consoleInputMutex;

			std::queue<ConsoleOutput> m_consoleOutput = {};
			void ProcessConsoleInput(KeyState pressState = KeyState::Press);

			std::thread::id m_mainThreadId;
			unsigned int m_tickRate;
			ChronoTime m_ctTick;
			long long m_lastTick;
			uint64_t m_tickCount = 0;
			std::shared_ptr<fs::VFilePtrInternalReal> m_logFile;
			std::unique_ptr<asset::AssetManager> m_assetManager;

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

			std::shared_ptr<debug::CPUProfiler> m_cpuProfiler;
			std::vector<CallbackHandle> m_profileHandlers = {};

			std::queue<std::function<void()>> m_tickEventQueue;
			std::mutex m_tickEventQueueMutex;
			StateFlags m_stateFlags;
			mutable pad::PackageManager *m_padPackageManager = nullptr;
			std::unique_ptr<debug::ProfilingStageManager<debug::ProfilingStage>> m_profilingStageManager;

			std::unordered_map<std::string, std::function<void(int, char *[])>> m_launchOptions;

			void InitLaunchOptions(int argc, char *argv[]);
			virtual void Think();
			virtual void Tick();
		};
		using namespace pragma::math::scoped_enum::bitwise;
	}
	REGISTER_ENUM_FLAGS(pragma::Engine::StateFlags)

	namespace pragma {
		DLLNETWORK Engine *get_engine();
		DLLNETWORK NetworkState *get_server_state();

		template<class T>
		T *Engine::GetConVar(const std::string &scvar)
		{
			console::ConConf *cv = GetConVar(scvar);
			if(cv == nullptr)
				return nullptr;
			return static_cast<T *>(cv);
		}
	};
};

export namespace pragma {
	class LaunchParaMap;
	void register_launch_parameters(LaunchParaMap &map);
}
