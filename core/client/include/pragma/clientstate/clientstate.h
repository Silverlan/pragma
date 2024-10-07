/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __CLIENTSTATE_H__
#define __CLIENTSTATE_H__

#include "pragma/c_enginedefinitions.h"
#include "pragma/clientdefinitions.h"
#include <pragma/networkstate/networkstate.h>
#include <pragma/networking/portinfo.h>
#include "pragma/rendering/game_world_shader_settings.hpp"
#include "pragma/game/c_game.h"
#include "pragma/audio/c_alsound.h"

#undef PlaySound

#pragma warning(push)
#pragma warning(disable : 4251)
struct DLLCLIENT ServerInfo {
  private:
	std::string m_downloadPath;
  public:
	ServerInfo() = default;
	~ServerInfo();
	std::string address;
	unsigned short portUDP;

	// Only used if server authentication is enabled.
	// Must be kept alive for the entire duration of the connection to the server!
	std::shared_ptr<void> authTokenHandle;

	const std::string &GetDownloadPath() const;
	void SetDownloadPath(const std::string &path);
};

struct DLLCLIENT ResourceDownload {
	ResourceDownload(VFilePtrReal file, std::string name, unsigned int size)
	{
		this->file = file;
		this->name = name;
		this->size = size;
	}
	~ResourceDownload()
	{
		if(file != NULL)
			file.reset();
	}
	VFilePtrReal file;
	std::string name;
	unsigned int size;
};

class CLNetMessage;
class ClientMessageMap;
enum class ClientEvent : int;
struct ALResource;
struct ALAudio;
class WIMainMenu;
class WILuaHandleWrapper;
enum class ALSoundType : Int32;
namespace al {
	class ISoundBuffer;
	class Decoder;
};
namespace pragma::networking {
	class IClient;
	enum class Protocol : uint8_t;
};
namespace prosper {
	class RenderTarget;
};
namespace pragma::string {
	class Utf8String;
};
class DLLCLIENT ClientState : public NetworkState {
	// For internal use only! Not to be used directly!
  public:
	virtual std::unordered_map<std::string, std::shared_ptr<PtrConVar>> &GetConVarPtrs() override;
	static ConVarHandle GetConVarHandle(std::string scvar);
	//
  private:
	std::unique_ptr<pragma::networking::IClient> m_client;
	std::unique_ptr<ServerInfo> m_svInfo;
	std::unique_ptr<ResourceDownload> m_resDownload; // Current resource file being downloaded

	unsigned int GetServerMessageID(std::string identifier);
	unsigned int GetServerConVarID(std::string scmd);
	bool GetServerConVarIdentifier(uint32_t id, std::string &cvar);

	// Sound
	void InitializeSound(CALSound &snd);
	std::vector<std::shared_ptr<ALSound>> m_soundScripts; // 'Regular' sounds are already handled by sound engine, but we still have to take care of sound-scripts
	float m_volMaster;
	std::unordered_map<ALSoundType, float> m_volTypes;

	WIHandle m_hMainMenu;
	WIHandle m_hFps;
	pragma::rendering::GameWorldShaderSettings m_worldShaderSettings {};
  protected:
	std::shared_ptr<Lua::Interface> m_luaGUI = nullptr;
	void InitializeGUILua();
	std::vector<std::function<luabind::object(lua_State *, WIBase &)>> m_guiLuaWrapperFactories;

	virtual void InitializeResourceManager() override;
	void StartResourceTransfer();
	void InitializeGameClient(bool singlePlayerLocalGame);
	void ResetGameClient();
	void DestroyClient();

	virtual void implFindSimilarConVars(const std::string &input, std::vector<SimilarCmdInfo> &similarCmds) const override;
	virtual Material *LoadMaterial(const std::string &path, bool precache, bool bReload) override;
  public:
	ClientState();
	virtual ~ClientState() override;
	virtual bool IsClient() const;
  public:
	virtual bool ShouldRemoveSound(ALSound &snd) override;
	Material *LoadMaterial(const std::string &path, const std::function<void(Material *)> &onLoaded, bool bReload, bool bLoadInstantly); // TODO
	msys::MaterialHandle CreateMaterial(const std::string &path, const std::string &shader);
	msys::MaterialHandle CreateMaterial(const std::string &shader);
	bool LoadGUILuaFile(std::string f);
	pragma::networking::IClient *GetClient();
	virtual NwStateType GetType() const override;
	void Think();
	void Tick();
	void Draw(util::DrawSceneInfo &drawSceneInfo);
	void Render(util::DrawSceneInfo &drawSceneInfo, std::shared_ptr<prosper::RenderTarget> &rt);
	virtual void Close() override;
	ConVarMap *GetConVarMap();
	bool IsConnected() const;
	void AddGUILuaWrapperFactory(const std::function<luabind::object(lua_State *, WIBase &)> &f);
	std::vector<std::function<luabind::object(lua_State *, WIBase &)>> &GetGUILuaWrapperFactories();
	virtual msys::MaterialManager &GetMaterialManager() override;
	virtual ModelSubMesh *CreateSubMesh() const override;
	virtual ModelMesh *CreateMesh() const override;
	virtual util::FileAssetManager *GetAssetManager(pragma::asset::Type type) override;
	virtual void Initialize() override;
	virtual std::string GetMessagePrefix() const override;

	pragma::rendering::GameWorldShaderSettings &GetGameWorldShaderSettings() { return m_worldShaderSettings; }
	const pragma::rendering::GameWorldShaderSettings &GetGameWorldShaderSettings() const { return const_cast<ClientState *>(this)->GetGameWorldShaderSettings(); }
	void UpdateGameWorldShaderSettings();

	WIMainMenu *GetMainMenu();

	lua_State *GetGUILuaState();
	Lua::Interface &GetGUILuaInterface();
	bool IsMainMenuOpen();
	void CloseMainMenu();
	void OpenMainMenu();
	void ToggleMainMenu();
	// Lua
	static void RegisterSharedLuaGlobals(Lua::Interface &lua);
	static void RegisterSharedLuaClasses(Lua::Interface &lua, bool bGUI = false);
	static void RegisterSharedLuaLibraries(Lua::Interface &lua, bool bGUI = false);
	static void RegisterVulkanLuaInterface(Lua::Interface &lua);
	// CVars
	void RegisterServerConVar(std::string scmd, unsigned int id);
	virtual bool RunConsoleCommand(std::string scmd, std::vector<std::string> &argv, pragma::BasePlayerComponent *pl = nullptr, KeyState pressState = KeyState::Press, float magnitude = 1.f, const std::function<bool(ConConf *, float &)> &callback = nullptr) override;
	ConVar *SetConVar(std::string scmd, std::string value, bool bApplyIfEqual = false);
	// Sockets
	void Connect(std::string ip, std::string port = sci::DEFAULT_PORT_TCP);
	// Peer-to-peer only!
	void Connect(uint64_t steamId);
	CLNetMessage *GetNetMessage(unsigned int ID);
	ClientMessageMap *GetNetMessageMap();
	void SendUserInfo();

	void InitializeGUIModule();

	// Sound
	virtual void StopSounds() override;
	virtual void StopSound(std::shared_ptr<ALSound> pSnd) override;
	bool PrecacheSound(std::string snd, std::pair<al::ISoundBuffer *, al::ISoundBuffer *> *buffers, ALChannel mode = ALChannel::Auto, bool bLoadInstantly = false);
	virtual bool PrecacheSound(std::string snd, ALChannel mode = ALChannel::Auto) override;
	virtual bool LoadSoundScripts(const char *file, bool bPrecache = false) override;
	virtual std::shared_ptr<ALSound> CreateSound(std::string snd, ALSoundType type, ALCreateFlags flags = ALCreateFlags::None) override;
	std::shared_ptr<ALSound> CreateSound(al::ISoundBuffer &buffer, ALSoundType type);
	std::shared_ptr<ALSound> CreateSound(al::Decoder &decoder, ALSoundType type);
	void IndexSound(std::shared_ptr<ALSound> snd, unsigned int idx);
	std::shared_ptr<ALSound> PlaySound(std::string snd, ALSoundType type, ALCreateFlags flags = ALCreateFlags::None);
	std::shared_ptr<ALSound> PlaySound(al::ISoundBuffer &buffer, ALSoundType type);
	std::shared_ptr<ALSound> PlaySound(al::Decoder &buffer, ALSoundType type);
	std::shared_ptr<ALSound> PlayWorldSound(al::ISoundBuffer &buffer, ALSoundType type, const Vector3 &pos);
	std::shared_ptr<ALSound> PlayWorldSound(al::Decoder &buffer, ALSoundType type, const Vector3 &pos);
	std::shared_ptr<ALSound> PlayWorldSound(std::string snd, ALSoundType type, const Vector3 &pos);
	virtual std::shared_ptr<ALSound> GetSoundByIndex(unsigned int idx) override;
	virtual void UpdateSounds() override;
	void SetMasterSoundVolume(float vol);
	float GetMasterSoundVolume();
	void SetSoundVolume(ALSoundType type, float vol);
	float GetSoundVolume(ALSoundType type);
	void UpdateSoundVolume();
	std::unordered_map<ALSoundType, float> &GetSoundVolumes();

	// Handles
	void LoadLuaCache(std::string cache, unsigned int cacheSize);
	void HandlePacket(NetPacket &packet);
	void HandleConnect();
	void HandleReceiveGameInfo(NetPacket &packet);
	void SetGameReady();
	void RequestServerInfo();

	void HandleClientStartResourceTransfer(NetPacket &packet);
	void HandleClientReceiveServerInfo(NetPacket &packet);
	void HandleClientResource(NetPacket &packet);
	void HandleClientResourceFragment(NetPacket &packet);

	void HandleLuaNetPacket(NetPacket &packet);

	void SendPacket(const std::string &name, NetPacket &packet, pragma::networking::Protocol protocol);
	void SendPacket(const std::string &name, NetPacket &packet);
	void SendPacket(const std::string &name, pragma::networking::Protocol protocol);

	void Disconnect();
	// Game
	virtual bool IsMultiPlayer() const override;
	virtual bool IsSinglePlayer() const override;
	virtual void StartGame(bool singlePlayer) override;
	void StartNewGame(const std::string &gameMode);
	CGame *GetGameState();
	void EndGame();
	bool IsGameActive();

	void ShowFPSCounter(bool b);

	// Config
	Bool RawMouseInput(GLFW::MouseButton button, GLFW::KeyState state, GLFW::Modifier mods);
	Bool RawKeyboardInput(GLFW::Key key, int scanCode, GLFW::KeyState state, GLFW::Modifier mods, float magnitude = 1.f);
	Bool RawCharInput(unsigned int c);
	Bool RawScrollInput(Vector2 offset);

	Bool MouseInput(GLFW::MouseButton button, GLFW::KeyState state, GLFW::Modifier mods);
	Bool KeyboardInput(GLFW::Key key, int scanCode, GLFW::KeyState state, GLFW::Modifier mods, float magnitude = 1.f);
	Bool CharInput(unsigned int c);
	Bool ScrollInput(Vector2 offset);
	void OnFilesDropped(std::vector<std::string> &files);
	void OnDragEnter(prosper::Window &window);
	void OnDragExit(prosper::Window &window);
	bool OnWindowShouldClose(prosper::Window &window);
	void OnPreedit(prosper::Window &window, const pragma::string::Utf8String &preeditString, const std::vector<int> &blockSizes, int focusedBlock, int caret);
	void OnIMEStatusChanged(prosper::Window &window, bool imeEnabled);

	Material *LoadMaterial(const std::string &path, const std::function<void(Material *)> &onLoaded, bool bReload = false);
	Material *LoadMaterial(const std::string &path);

	void ReadEntityData(NetPacket &packet);
};
#pragma warning(pop)

#endif
