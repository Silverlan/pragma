/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __NETWORKSTATE_H__
#define __NETWORKSTATE_H__
#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
#include <pragma/audio/alenums.hpp>
#include <sharedutils/callback_handler.h>
#include <pragma/console/cvar_handler.h>
#include <sharedutils/chronotime.h>
#include "pragma/lua/luafunction.h"
#include <pragma/console/fcvar.h>
#include <sharedutils/util_cpu_profiler.hpp>
#include <pragma/input/key_state.hpp>
#include <unordered_set>
#ifdef __linux__
#include "pragma/audio/soundscript.h"
#endif

#define GLFW_RELEASE 0
#define GLFW_PRESS 1

#define CHECK_CHEATS(scmd, state, ret)                                                                                                                                                                                                                                                           \
	{                                                                                                                                                                                                                                                                                            \
		if(state->CheatsEnabled() == false) {                                                                                                                                                                                                                                                    \
			Con::cout << "Can't use cheat cvar " << scmd << " in multiplayer, unless the server has sv_cheats set to 1." << Con::endl;                                                                                                                                                           \
			return ret;                                                                                                                                                                                                                                                                          \
		}                                                                                                                                                                                                                                                                                        \
	}

class ConConf;
class ConVar;
class ConVarHandle;
class PtrConVar;
class ConCommand;
class ConVarMap;
class TCallback;
class Game;
struct MapInfo;
class CvarCallback;
class SoundScriptManager;
class SoundScript;
class Material;
class Model;
class ModelMesh;
class ModelSubMesh;
namespace msys {
	class MaterialManager;
};
class ResourceWatcherManager;
class ALSound;
enum class ALSoundType : int32_t;
enum class NwStateType : uint8_t;
namespace Lua {
	enum class ErrorColorMode : uint32_t;
	class Interface;
};
namespace util {
	class Library;
	class FileAssetManager;
};
namespace pragma {
	namespace asset {
		class EntityData;
		class WorldData;
		class ModelManager;
		enum class Type : uint8_t;
	};
	namespace debug {
		class CPUProfiler;
		template<class TProfilingStage>
		class ProfilingStageManager;
		class ProfilingStage;
	};
};
using ALSoundRef = std::reference_wrapper<ALSound>;
class DLLNETWORK NetworkState : public CallbackHandler, public CVarHandler {
	// For internal use only! Not to be used directly!
  protected:
	static ConVarHandle GetConVarHandle(std::unordered_map<std::string, std::shared_ptr<PtrConVar>> &ptrs, std::string scvar);
  public:
	virtual std::unordered_map<std::string, std::shared_ptr<PtrConVar>> &GetConVarPtrs() = 0;
  public:
	// Internal
	std::vector<CallbackHandle> &GetLuaEnumRegisterCallbacks();
	void TerminateLuaModules(lua_State *l);
	void DeregisterLuaModules(void *l, const std::string &identifier);
	virtual bool ShouldRemoveSound(ALSound &snd);

	virtual NwStateType GetType() const = 0;

	// Assets
	const pragma::asset::ModelManager &GetModelManager() const;
	pragma::asset::ModelManager &GetModelManager();

	// Debug
	pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage> *GetProfilingStageManager();
	bool StartProfilingStage(const char *stage);
	bool StopProfilingStage();

	ResourceWatcherManager &GetResourceWatcher();

	// Textures
	Material *LoadMaterial(const std::string &path, bool bReload = false);
	Material *PrecacheMaterial(const std::string &path);
	bool PortMaterial(const std::string &path);
	MapInfo *GetMapInfo();
	std::string GetMap();
	virtual void Close();
	virtual void Initialize() override;
	virtual void Think();
	virtual void Tick();
	// Lua
	lua_State *GetLuaState();
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

	void InitializeLuaModules(lua_State *l);
	virtual std::shared_ptr<util::Library> InitializeLibrary(std::string library, std::string *err = nullptr, lua_State *l = nullptr);
	bool UnloadLibrary(const std::string &library);
	std::shared_ptr<util::Library> LoadLibraryModule(const std::string &lib, const std::vector<std::string> &additionalSearchDirectories = {}, std::string *err = nullptr);
	std::shared_ptr<util::Library> GetLibraryModule(const std::string &lib) const;

	std::unordered_map<std::string, unsigned int> &GetConCommandIDs();

	// Sound
	std::vector<ALSoundRef> m_sounds;
	std::unordered_map<std::string, std::shared_ptr<SoundCacheInfo>> m_soundsPrecached;
	void UpdateSounds(std::vector<std::shared_ptr<ALSound>> &sounds);
  public:
	NetworkState();
	virtual ~NetworkState();
	virtual bool IsServer() const;
	virtual bool IsClient() const;
	virtual bool IsMultiPlayer() const = 0;
	virtual bool IsSinglePlayer() const = 0;
	bool CheatsEnabled() const;
	virtual msys::MaterialManager &GetMaterialManager() = 0;
	virtual ModelSubMesh *CreateSubMesh() const = 0;
	virtual ModelMesh *CreateMesh() const = 0;
	virtual util::FileAssetManager *GetAssetManager(pragma::asset::Type type);

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
	virtual std::shared_ptr<ALSound> CreateSound(std::string snd, ALSoundType type, ALCreateFlags flags = ALCreateFlags::None) = 0;
	virtual void UpdateSounds() = 0;
	virtual bool PrecacheSound(std::string snd, ALChannel mode = ALChannel::Auto) = 0;
	virtual void StopSounds() = 0;
	virtual void StopSound(std::shared_ptr<ALSound> pSnd) = 0;
	virtual std::shared_ptr<ALSound> GetSoundByIndex(unsigned int idx) = 0;
	const std::vector<ALSoundRef> &GetSounds() const;
	std::vector<ALSoundRef> &GetSounds();

	SoundScriptManager *GetSoundScriptManager();
	SoundScript *FindSoundScript(const char *name);
	virtual bool LoadSoundScripts(const char *file, bool bPrecache = false);
	Bool IsSoundPrecached(const std::string &snd) const;

	// ConVars
	virtual ConVarMap *GetConVarMap() override;
	virtual bool RunConsoleCommand(std::string scmd, std::vector<std::string> &argv, pragma::BasePlayerComponent *pl = nullptr, KeyState pressState = KeyState::Press, float magnitude = 1.f, const std::function<bool(ConConf *, float &)> &callback = nullptr);
	virtual ConVar *SetConVar(std::string scmd, std::string value, bool bApplyIfEqual = false) override;

	void CallOnNextTick(const std::function<void()> &f);

	ConVar *CreateConVar(const std::string &scmd, udm::Type type, const std::string &value, ConVarFlags flags, const std::string &help = "");
	ConVar *RegisterConVar(const std::string &scmd, const std::shared_ptr<ConVar> &cvar);
	void UnregisterConVar(const std::string &scmd);
	virtual ConCommand *CreateConCommand(const std::string &scmd, LuaFunction fc, ConVarFlags flags = ConVarFlags::None, const std::string &help = "");
  protected:
	virtual Material *LoadMaterial(const std::string &path, bool precache, bool bReload);

	static UInt8 STATE_COUNT;
	std::unique_ptr<MapInfo> m_mapInfo;
	bool m_bTCPOnly;
	bool m_bTerminateSocket;
	std::vector<CallbackHandle> m_luaEnumRegisterCallbacks;
	std::unique_ptr<ResourceWatcherManager> m_resourceWatcher;

	std::unordered_map<std::string, std::string> m_conOverrides;
	ChronoTime m_ctReal;
	double m_tReal;
	double m_tDelta;
	double m_tLast;

	std::unique_ptr<Game, void (*)(Game *)> m_game = std::unique_ptr<Game, void (*)(Game *)> {nullptr, [](Game *) {}};
	std::shared_ptr<pragma::asset::ModelManager> m_modelManager = nullptr;
	std::unique_ptr<SoundScriptManager> m_soundScriptManager;
	std::unordered_set<std::string> m_missingSoundCache;
	std::vector<CallbackHandle> m_thinkCallbacks;
	std::vector<CallbackHandle> m_tickCallbacks;
	std::queue<std::function<void()>> m_tickCallQueue;
	CallbackHandle m_cbProfilingHandle = {};
	std::unique_ptr<pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage>> m_profilingStageManager;

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
	std::unordered_map<lua_State *, std::vector<std::shared_ptr<util::Library>>> m_initializedLibraries;

	void InitializeDLLModule(lua_State *l, std::shared_ptr<util::Library> module);

	virtual void InitializeResourceManager();
	void ClearGameConVars();
	virtual void implFindSimilarConVars(const std::string &input, std::vector<SimilarCmdInfo> &similarCmds) const override;
};

#endif
