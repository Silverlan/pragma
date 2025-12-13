// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:network_state;

export import :assets.common;
export import :audio.enums;
export import :audio.sound_script_manager;
export import :console.cvar_handler;
export import :debug.performance_profiler;
export import :engine.enums;
export import :input.enums;
export import :map.map_info;

export import :types;
export import :util.resource_watcher;

export namespace pragma {
	class DLLNETWORK NetworkState : public util::CallbackHandler, public console::CVarHandler {
		// For internal use only! Not to be used directly!
	  protected:
		static console::ConVarHandle GetConVarHandle(std::unordered_map<std::string, std::shared_ptr<console::PtrConVar>> &ptrs, std::string scvar);
	  public:
		virtual std::unordered_map<std::string, std::shared_ptr<console::PtrConVar>> &GetConVarPtrs() = 0;
	  public:
		// Internal
		std::vector<CallbackHandle> &GetLuaEnumRegisterCallbacks();
		void TerminateLuaModules(lua::State *l);
		void DeregisterLuaModules(void *l, const std::string &identifier);
		virtual bool ShouldRemoveSound(audio::ALSound &snd);

		virtual NwStateType GetType() const = 0;

		// Assets
		const asset::ModelManager &GetModelManager() const;
		asset::ModelManager &GetModelManager();

		// Debug
		debug::ProfilingStageManager<debug::ProfilingStage> *GetProfilingStageManager();
		bool StartProfilingStage(const char *stage);
		bool StopProfilingStage();

		util::ResourceWatcherManager &GetResourceWatcher();

		// Textures
		material::Material *LoadMaterial(const std::string &path, bool bReload = false);
		material::Material *PrecacheMaterial(const std::string &path);
		bool PortMaterial(const std::string &path);
		MapInfo *GetMapInfo();
		std::string GetMap();
		virtual void Close();
		virtual void Initialize() override;
		virtual void Think();
		virtual void Tick();
		// Lua
		lua::State *GetLuaState();
		virtual std::string GetMessagePrefix() const = 0;
		static void RegisterSharedLuaGlobals(Lua::Interface &lua);
		static void RegisterSharedLuaClasses(Lua::Interface &lua);
		static void RegisterSharedLuaLibraries(Lua::Interface &lua);
		// Time
		double &RealTime();
		double &DeltaTime();
		double &LastThink();
		void WriteToLog(std::string str);

		void AddThinkCallback(CallbackHandle callback);
		void AddTickCallback(CallbackHandle callback);

		void InitializeLuaModules(lua::State *l);
		virtual std::shared_ptr<util::Library> InitializeLibrary(std::string library, std::string *err = nullptr, lua::State *l = nullptr);
		bool UnloadLibrary(const std::string &library);
		std::shared_ptr<util::Library> LoadLibraryModule(const std::string &lib, const std::vector<std::string> &additionalSearchDirectories = {}, std::string *err = nullptr);
		std::shared_ptr<util::Library> GetLibraryModule(const std::string &lib) const;

		std::unordered_map<std::string, unsigned int> &GetConCommandIDs();

		// Sound
		std::vector<audio::ALSoundRef> m_sounds;
		std::unordered_map<std::string, std::shared_ptr<audio::SoundCacheInfo>> m_soundsPrecached;
		void UpdateSounds(std::vector<std::shared_ptr<audio::ALSound>> &sounds);
	  public:
		NetworkState();
		virtual ~NetworkState();
		virtual bool IsServer() const;
		virtual bool IsClient() const;
		virtual bool IsMultiPlayer() const = 0;
		virtual bool IsSinglePlayer() const = 0;
		bool CheatsEnabled() const;
		virtual material::MaterialManager &GetMaterialManager() = 0;
		virtual geometry::ModelSubMesh *CreateSubMesh() const = 0;
		virtual geometry::ModelMesh *CreateMesh() const = 0;
		virtual util::FileAssetManager *GetAssetManager(asset::Type type);

		void TranslateConsoleCommand(std::string &cmd);
		void SetConsoleCommandOverride(const std::string &src, const std::string &dst);
		void ClearConsoleCommandOverride(const std::string &src);
		void ClearConsoleCommandOverrides();

		// Game
		virtual void StartGame(bool singlePlayer);
		virtual void EndGame();
		Game *GetGameState();
		virtual bool IsGameActive();
		virtual void StartNewGame(const std::string &map, bool singlePlayer);
		virtual void ChangeLevel(const std::string &map);

		// Sound
		float GetSoundDuration(std::string snd);
		virtual std::shared_ptr<audio::ALSound> CreateSound(std::string snd, audio::ALSoundType type, audio::ALCreateFlags flags = audio::ALCreateFlags::None) = 0;
		virtual void UpdateSounds() = 0;
		virtual bool PrecacheSound(std::string snd, audio::ALChannel mode = audio::ALChannel::Auto) = 0;
		virtual void StopSounds() = 0;
		virtual void StopSound(std::shared_ptr<audio::ALSound> pSnd) = 0;
		virtual std::shared_ptr<audio::ALSound> GetSoundByIndex(unsigned int idx) = 0;
		const std::vector<audio::ALSoundRef> &GetSounds() const;
		std::vector<audio::ALSoundRef> &GetSounds();

		audio::SoundScriptManager *GetSoundScriptManager();
		audio::SoundScript *FindSoundScript(const char *name);
		virtual bool LoadSoundScripts(const char *file, bool bPrecache = false);
		Bool IsSoundPrecached(const std::string &snd) const;

		// ConVars
		virtual console::ConVarMap *GetConVarMap() override;
		virtual bool RunConsoleCommand(std::string scmd, std::vector<std::string> &argv, BasePlayerComponent *pl = nullptr, KeyState pressState = KeyState::Press, float magnitude = 1.f, const std::function<bool(console::ConConf *, float &)> &callback = nullptr);
		virtual console::ConVar *SetConVar(std::string scmd, std::string value, bool bApplyIfEqual = false) override;

		void CallOnNextTick(const std::function<void()> &f);

		console::ConVar *CreateConVar(const std::string &scmd, udm::Type type, const std::string &value, console::ConVarFlags flags, const std::string &help = "");
		console::ConVar *RegisterConVar(const std::string &scmd, const std::shared_ptr<console::ConVar> &cvar);
		void UnregisterConVar(const std::string &scmd);
		virtual console::ConCommand *CreateConCommand(const std::string &scmd, LuaFunction fc, console::ConVarFlags flags = console::ConVarFlags::None, const std::string &help = "");
	  protected:
		virtual material::Material *LoadMaterial(const std::string &path, bool precache, bool bReload);

		static UInt8 STATE_COUNT;
		std::unique_ptr<MapInfo> m_mapInfo;
		bool m_bTCPOnly;
		bool m_bTerminateSocket;
		std::vector<CallbackHandle> m_luaEnumRegisterCallbacks;
		std::unique_ptr<util::ResourceWatcherManager> m_resourceWatcher;

		std::unordered_map<std::string, std::string> m_conOverrides;
		ChronoTime m_ctReal;
		double m_tReal;
		double m_tDelta;
		double m_tLast;

		std::unique_ptr<Game, void (*)(Game *)> m_game = std::unique_ptr<Game, void (*)(Game *)> {nullptr, [](Game *) {}};
		std::shared_ptr<asset::ModelManager> m_modelManager = nullptr;
		std::unique_ptr<audio::SoundScriptManager> m_soundScriptManager;
		std::unordered_set<std::string> m_missingSoundCache;
		std::vector<CallbackHandle> m_thinkCallbacks;
		std::vector<CallbackHandle> m_tickCallbacks;
		std::queue<std::function<void()>> m_tickCallQueue;
		CallbackHandle m_cbProfilingHandle = {};
		std::unique_ptr<debug::ProfilingStageManager<debug::ProfilingStage>> m_profilingStageManager;

		// Library handles are stored as shared_ptrs of shared_ptr because we need the
		// use count of each library in the network states to determine when to detach
		// the library (use count = 0 => not used in any network state => detach),
		// but this doesn't work if shared_ptr instances exist outside of the
		// network states.
		std::vector<std::shared_ptr<std::shared_ptr<util::Library>>> m_libHandles;
		std::shared_ptr<util::Library> m_lastModuleHandle = nullptr;
		struct DLLNETWORK LibraryInfo {
			std::shared_ptr<std::shared_ptr<util::Library>> library;
			bool loadedServerside = false;
			bool loadedClientside = false;
			bool WasLoadedInState(const NetworkState &nw) const { return (nw.IsClient() && loadedClientside) || (!nw.IsClient() && loadedServerside); }
		};
		static std::unordered_map<std::string, LibraryInfo> s_loadedLibraries;
		std::unordered_map<lua::State *, std::vector<std::shared_ptr<util::Library>>> m_initializedLibraries;

		void InitializeDLLModule(lua::State *l, std::shared_ptr<util::Library> module);

		virtual void InitializeResourceManager();
		void ClearGameConVars();
		virtual void implFindSimilarConVars(const std::string &input, std::vector<SimilarCmdInfo> &similarCmds) const override;
	};

	DLLNETWORK bool check_cheats(const std::string &scmd, NetworkState *state);
};
