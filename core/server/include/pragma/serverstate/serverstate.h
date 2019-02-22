#ifndef __SERVERSTATE_H__
#define __SERVERSTATE_H__
#include "pragma/serverdefinitions.h"
#include <pragma/networkstate/networkstate.h>
#include "pragma/game/s_game.h"
#include <pragma/input/inkeys.h>
#include <sharedutils/chronoutil.h>
#include "wmserverdata.h"

#define FSYS_SEARCH_CACHE 8'192

class SVNetMessage;
class ServerMessageMap;
class WVServer;
class WVLocalClient;
class ClientSessionInfo;
namespace nwm {class RecipientFilter;};
namespace pragma {class SPlayerComponent;};
struct Resource;
enum class ServerEvent : int;
#pragma warning(push)
#pragma warning(disable : 4251)
class DLLSERVER ServerState
	: public NetworkState
{
// For internal use only! Not to be used directly!
public:
	virtual std::unordered_map<std::string,std::shared_ptr<PtrConVar>> &GetConVarPtrs() override;
	static ConVarHandle GetConVarHandle(std::string scvar);
//
private:
	SGame *m_game = nullptr;
	std::unordered_map<std::string,ConCommand*> m_luaConCommands;
	unsigned int m_conCommandID;
	std::unique_ptr<WVServer> m_server;
	ChronoTimePoint m_tNextWMSConnect;
	std::unique_ptr<WVLocalClient> m_local; // SinglePlayer or local player on a listen server
	unsigned int m_alsoundID;

	std::deque<unsigned int> m_alsoundIndex;
	// We need to keep shared pointer references to all serverside sounds (Network state only keeps references)
	std::vector<std::shared_ptr<ALSound>> m_serverSounds;
protected:
	virtual void implFindSimilarConVars(const std::string &input,std::vector<SimilarCmdInfo> &similarCmds) const override;
	virtual void InitializeResourceManager() override;
	void ClearConCommands();
	void OnMasterServerRegistered(bool b,std::string reason);
	void RegisterServerInfo();
	WMServerData m_serverData;
public:
	virtual void Initialize() override;
	virtual void Think() override;
	virtual void Tick() override;
	virtual void Close() override;
	WMServerData &GetServerData();
	void SendResourceFile(const std::string &f,const std::vector<WVServerClient*> &clients);
	void SendResourceFile(const std::string &f);
	void SendRoughModel(const std::string &f,const std::vector<WVServerClient*> &clients);
	void SendRoughModel(const std::string &f);
	// ConVars
	virtual ConVar *SetConVar(std::string scmd,std::string value,bool bApplyIfEqual=false) override;
	// Sound
	virtual std::shared_ptr<ALSound> CreateSound(std::string snd,ALSoundType type,ALCreateFlags flags=ALCreateFlags::None) override;
	virtual std::shared_ptr<ALSound> GetSoundByIndex(unsigned int idx) override;
	virtual void UpdateSounds() override;
	virtual bool PrecacheSound(std::string snd,ALChannel mode=ALChannel::Auto) override;
	virtual void StopSounds() override;
	virtual void StopSound(std::shared_ptr<ALSound> pSnd) override;
	// Game
	virtual void StartGame() override;
	virtual void LoadMap(const char *map,bool bDontReload=false) override;

	virtual Lua::ErrorColorMode GetLuaErrorColorMode() override;
	void OnClientConVarChanged(pragma::BasePlayerComponent &pl,std::string cvar,std::string value);
	pragma::SPlayerComponent *GetPlayer(WVServerClient *session);

	virtual bool IsMultiPlayer() const override;
	virtual bool IsSinglePlayer() const override;
	void StartServer();
	void CloseServer();
	WVLocalClient *GetLocalClient();
	void InitResourceTransfer(WVServerClient *session);
	void HandleServerNextResource(WVServerClient *session);
	void HandleServerResourceStart(WVServerClient *session,NetPacket &packet);
	void HandleServerResourceFragment(WVServerClient *session);
	void HandleLuaNetPacket(WVServerClient *session,NetPacket &packet);
	bool HandlePacket(WVServerClient *session,NetPacket &packet);
	void ReceiveUserInput(WVServerClient *session,NetPacket &packet);
	void BroadcastTCP(unsigned int ID,NetPacket &packet);
	void BroadcastUDP(unsigned int ID,NetPacket &packet);
	void SendPacketTCP(unsigned int ID,NetPacket &packet,const nwm::RecipientFilter &rp);
	void SendPacketUDP(unsigned int ID,NetPacket &packet,const nwm::RecipientFilter &rp);
public:
	ServerState();
	virtual ~ServerState() override;
	virtual MaterialManager &GetMaterialManager() override;
	virtual ModelSubMesh *CreateSubMesh() const override;
	virtual ModelMesh *CreateMesh() const override;
	virtual bool IsServer() const override;
	virtual ConVarMap *GetConVarMap() override;
	ServerMessageMap *GetNetMessageMap();
	SVNetMessage *GetNetMessage(unsigned int ID);
	unsigned int GetClientMessageID(std::string identifier);
	virtual ConCommand *CreateConCommand(const std::string &scmd,LuaFunction fc,ConVarFlags flags=ConVarFlags::None,const std::string &help="") override;
	void GetLuaConCommands(std::unordered_map<std::string,ConCommand*> **cmds);

	// Game
	virtual SGame *GetGameState() override;
	virtual void EndGame() override;
	virtual bool IsGameActive() override;
	virtual Material *LoadMaterial(const std::string &path,bool bReload=false) override;

	// Sound
	virtual bool LoadSoundScripts(const char *file,bool bPrecache=false) override;

	void BroadcastTCP(const std::string &name,NetPacket &packet);
	void BroadcastUDP(const std::string &name,NetPacket &packet);
	void SendPacketTCP(const std::string &name,NetPacket &packet,const nwm::RecipientFilter &rp);
	void SendPacketUDP(const std::string &name,NetPacket &packet,const nwm::RecipientFilter &rp);
	void SendPacketTCP(const std::string &name,NetPacket &packet,WVServerClient *client);
	void SendPacketUDP(const std::string &name,NetPacket &packet,WVServerClient *client);
	void BroadcastTCP(const std::string &name);
	void BroadcastUDP(const std::string &name);
	void SendPacketTCP(const std::string &name,const nwm::RecipientFilter &rp);
	void SendPacketUDP(const std::string &name,const nwm::RecipientFilter &rp);
	void SendPacketTCP(const std::string &name,WVServerClient *client);
	void SendPacketUDP(const std::string &name,WVServerClient *client);

	WVServer *GetServer();
	bool IsServerRunning() const;
	void DropClient(WVServerClient *session);
	unsigned short GetTCPPort();
	unsigned short GetUDPPort();
	virtual bool IsTCPOpen() const override;
	virtual bool IsUDPOpen() const override;
};
#pragma warning(pop)
#endif
