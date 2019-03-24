#ifndef __CLIENTSTATE_H__
#define __CLIENTSTATE_H__

#include "pragma/c_enginedefinitions.h"
#include "pragma/clientdefinitions.h"
#include <pragma/networkstate/networkstate.h>
#include <wgui/wihandle.h>
#include <pragma/networking/portinfo.h>
#include "pragma/game/c_game.h"
#include "pragma/audio/c_alsound.h"

#undef PlaySound

namespace boost{namespace system{
	class error_code;
}}

#pragma warning(push)
#pragma warning(disable : 4251)
struct DLLCLIENT ServerInfo
{
private:
	std::string m_downloadPath;
public:
	ServerInfo()=default;
	~ServerInfo();
	std::string address;
	unsigned short portUDP;
	const std::string &GetDownloadPath() const;
	void SetDownloadPath(const std::string &path);
};

struct DLLCLIENT ResourceDownload
{
	ResourceDownload(VFilePtrReal file,std::string name,unsigned int size)
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
/*
struct DLLCLIENT ALBuffer
{
	ALBuffer()
		: mono(-1),stereo(-1)
	{}
	~ALBuffer()
	{
		if(mono > 0)
		{
			unsigned int umono = mono;
			alDeleteBuffers(1,&umono);
		}
		if(stereo > 0)
		{
			unsigned int ustereo = stereo;
			alDeleteBuffers(1,&ustereo);
		}
	}
	int mono;
	int stereo;
};
*/
class CLNetMessage;
class ClientMessageMap;
class WVClient;
enum class ClientEvent : int;
struct ALResource;
struct ALAudio;
class WIMainMenu;
struct WILuaHandleWrapper;
enum class ALSoundType : Int32;
namespace al {class SoundBuffer;class Decoder;};
class DLLCLIENT ClientState
	: public NetworkState
{
// For internal use only! Not to be used directly!
public:
	virtual std::unordered_map<std::string,std::shared_ptr<PtrConVar>> &GetConVarPtrs() override;
	static ConVarHandle GetConVarHandle(std::string scvar);
//
private:
	CGame *m_game;
	std::unique_ptr<WVClient> m_client;
	std::unique_ptr<ServerInfo> m_svInfo;
	std::unique_ptr<ResourceDownload> m_resDownload; // Current resource file being downloaded

	unsigned int GetServerMessageID(std::string identifier);
	unsigned int GetServerConVarID(std::string scmd);
	bool GetServerConVarIdentifier(uint32_t id,std::string &cvar);

	// Sound
	void InitializeSound(CALSound &snd);
	std::vector<std::shared_ptr<ALSound>> m_soundScripts; // 'Regular' sounds are already handled by sound engine, but we still have to take care of sound-scripts
	float m_volMaster;
	std::unordered_map<Int32,float> m_volTypes; // c++14 std::unordered_map<ALSoundType,float> m_volTypes;

	WIHandle m_hMainMenu;
	WIHandle m_hFps;
protected:
	std::shared_ptr<Lua::Interface> m_luaGUI = nullptr;
	void InitializeGUILua();
	std::vector<std::function<luabind::object(lua_State*,WIBase&)>> m_guiLuaWrapperFactories;
	
	virtual void InitializeResourceManager() override;

	void SendPacketTCP(unsigned int ID,NetPacket &packet);
	void SendPacketUDP(unsigned int ID,NetPacket &packet);
	void StartResourceTransfer();

	virtual void implFindSimilarConVars(const std::string &input,std::vector<SimilarCmdInfo> &similarCmds) const override;
public:
	ClientState();
	virtual ~ClientState() override;
	virtual bool IsClient() const;
public:
	virtual bool ShouldRemoveSound(ALSound &snd) override;
	virtual Material *LoadMaterial(const std::string &path,bool bReload=false) override;
	Material *LoadMaterial(const std::string &path,const std::function<void(Material*)> &onLoaded,bool bReload,bool bLoadInstantly); // TODO
	Material *CreateMaterial(const std::string &path,const std::string &shader);
	Material *CreateMaterial(const std::string &shader);
	bool LoadGUILuaFile(std::string f);
	WVClient *GetClient();
	void Think();
	void Tick();
	void Draw(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,std::shared_ptr<prosper::RenderTarget> &rt);//const Vulkan::RenderPass &renderPass,const Vulkan::Framebuffer &framebuffer,const Vulkan::CommandBuffer &drawCmd); // prosper TODO
	void Render(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,std::shared_ptr<prosper::RenderTarget> &rt);//const Vulkan::RenderPass &renderPass,const Vulkan::Framebuffer &framebuffer,const Vulkan::CommandBuffer &drawCmd); // prosper TODO
	virtual void Close() override;
	ConVarMap *GetConVarMap();
	bool IsConnected() const;
	void AddGUILuaWrapperFactory(const std::function<luabind::object(lua_State*,WIBase&)> &f);
	std::vector<std::function<luabind::object(lua_State*,WIBase&)>> &GetGUILuaWrapperFactories();
	virtual MaterialManager &GetMaterialManager() override;
	virtual ModelSubMesh *CreateSubMesh() const override;
	virtual ModelMesh *CreateMesh() const override;
	virtual void Initialize() override;
	virtual Lua::ErrorColorMode GetLuaErrorColorMode() override;

	WIMainMenu *GetMainMenu();

	lua_State *GetGUILuaState();
	Lua::Interface &GetGUILuaInterface();
	bool IsMainMenuOpen();
	void CloseMainMenu();
	void OpenMainMenu();
	void ToggleMainMenu();
	// Lua
	static void RegisterSharedLuaGlobals(Lua::Interface &lua);
	static void RegisterSharedLuaClasses(Lua::Interface &lua,bool bGUI=false);
	static void RegisterSharedLuaLibraries(Lua::Interface &lua,bool bGUI=false);
	static void RegisterVulkanLuaInterface(Lua::Interface &lua);
	// CVars
	void RegisterServerConVar(std::string scmd,unsigned int id);
	virtual bool RunConsoleCommand(std::string scmd,std::vector<std::string> &argv,pragma::BasePlayerComponent *pl=nullptr,KeyState pressState=KeyState::Press,float magnitude=1.f,const std::function<bool(ConConf*,float&)> &callback=nullptr) override;
	ConVar *SetConVar(std::string scmd,std::string value,bool bApplyIfEqual=false);
	// Sockets
	unsigned short GetTCPPort();
	unsigned short GetUDPPort();
	void Connect(std::string ip,std::string port=sci::DEFAULT_PORT_TCP);
	CLNetMessage *GetNetMessage(unsigned int ID);
	ClientMessageMap *GetNetMessageMap();
	void SendUserInfo();

	void InitializeGUIModule();

	// Saves the currently bound frame buffer as a .tga-file. The texture has to be bound to GL_TEXTURE_2D
	bool SaveFrameBufferAsTGA(const char *name,int x,int y,int w,int h);
	// Saves the currently bound frame buffer as a .tga-file
	bool SaveFrameBufferAsTGA(const char *name,int x,int y,int w,int h,unsigned int format);

	bool SaveTextureAsTGA(const char *name);
	bool SaveTextureAsTGA(const char *name,unsigned int target,int level=0);
	bool SavePixelsAsTGA(const char *name,unsigned char *pixels,unsigned int w,unsigned int h);

	// Sound
	virtual void StopSounds() override;
	virtual void StopSound(std::shared_ptr<ALSound> pSnd) override;
	bool PrecacheSound(std::string snd,std::pair<al::SoundBuffer*,al::SoundBuffer*> *buffers,ALChannel mode=ALChannel::Auto,bool bLoadInstantly=false);
	virtual bool PrecacheSound(std::string snd,ALChannel mode=ALChannel::Auto) override;
	virtual bool LoadSoundScripts(const char *file,bool bPrecache=false) override;
	virtual std::shared_ptr<ALSound> CreateSound(std::string snd,ALSoundType type,ALCreateFlags flags=ALCreateFlags::None) override;
	std::shared_ptr<ALSound> CreateSound(al::SoundBuffer &buffer,ALSoundType type);
	std::shared_ptr<ALSound> CreateSound(al::Decoder &decoder,ALSoundType type);
	void IndexSound(std::shared_ptr<ALSound> snd,unsigned int idx);
	std::shared_ptr<ALSound> PlaySound(std::string snd,ALSoundType type,ALCreateFlags flags=ALCreateFlags::None);
	std::shared_ptr<ALSound> PlaySound(al::SoundBuffer &buffer,ALSoundType type);
	std::shared_ptr<ALSound> PlaySound(al::Decoder &buffer,ALSoundType type);
	std::shared_ptr<ALSound> PlayWorldSound(al::SoundBuffer &buffer,ALSoundType type,const Vector3 &pos);
	std::shared_ptr<ALSound> PlayWorldSound(al::Decoder &buffer,ALSoundType type,const Vector3 &pos);
	std::shared_ptr<ALSound> PlayWorldSound(std::string snd,ALSoundType type,const Vector3 &pos);
	virtual std::shared_ptr<ALSound> GetSoundByIndex(unsigned int idx) override;
	virtual void UpdateSounds() override;
	void SetMasterSoundVolume(float vol);
	float GetMasterSoundVolume();
	void SetSoundVolume(ALSoundType type,float vol);
	float GetSoundVolume(ALSoundType type);
	void UpdateSoundVolume();
	std::unordered_map<Int32,float> &GetSoundVolumes();

	// Handles
	void LoadLuaCache(std::string cache,unsigned int cacheSize);
	void HandlePacket(NetPacket &packet);
	void HandleConnect();
	void HandleReceiveGameInfo(NetPacket &packet);
	void RequestServerInfo();

	void HandleClientReceiveServerInfo(NetPacket &packet);
	void HandleClientResource(NetPacket &packet);
	void HandleClientResourceFragment(NetPacket &packet);

	void HandleLuaNetPacket(NetPacket &packet);

	void SendPacketTCP(const std::string &name,NetPacket &packet);
	void SendPacketUDP(const std::string &name,NetPacket &packet);

	void SendPacketTCP(const std::string &name);
	void SendPacketUDP(const std::string &name);
	virtual bool IsTCPOpen() const override;
	virtual bool IsUDPOpen() const override;
	void Disconnect();
	// Game
	virtual bool IsMultiPlayer() const override;
	virtual bool IsSinglePlayer() const override;
	virtual void StartGame() override;
	void StartGame(const std::string &gameMode);
	CGame *GetGameState();
	void EndGame();
	bool IsGameActive();

	void ShowFPSCounter(bool b);

	// Config
	Bool RawMouseInput(GLFW::MouseButton button,GLFW::KeyState state,GLFW::Modifier mods);
	Bool RawKeyboardInput(GLFW::Key key,int scanCode,GLFW::KeyState state,GLFW::Modifier mods,float magnitude=1.f);
	Bool RawCharInput(unsigned int c);
	Bool RawScrollInput(Vector2 offset);

	Bool MouseInput(GLFW::MouseButton button,GLFW::KeyState state,GLFW::Modifier mods);
	Bool KeyboardInput(GLFW::Key key,int scanCode,GLFW::KeyState state,GLFW::Modifier mods,float magnitude=1.f);
	Bool CharInput(unsigned int c);
	Bool ScrollInput(Vector2 offset);
	void OnFilesDropped(std::vector<std::string> &files);

	Material *LoadMaterial(const std::string &path,const std::function<void(Material*)> &onLoaded,bool bReload=false);
	Material *LoadMaterial(const std::string &path,bool bLoadInstantly,bool bReload);

	void ReadEntityData(NetPacket &packet);
};
#pragma warning(pop)

#endif
