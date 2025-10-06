// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include <sharedutils/chronoutil.h>
#include "wmserverdata.h"

export module pragma.server.server_state;

export import pragma.server.audio;
export import pragma.server.entities.components;
export import pragma.server.networking;
export import pragma.server.game;
export import pragma.shared;

#pragma warning(push)
#pragma warning(disable : 4251)
export class DLLSERVER ServerState : public NetworkState {
	// For internal use only! Not to be used directly!
  public:
	virtual std::unordered_map<std::string, std::shared_ptr<PtrConVar>> &GetConVarPtrs() override;
	static ConVarHandle GetConVarHandle(std::string scvar);
	//
  private:
	std::unordered_map<std::string, ConCommand *> m_luaConCommands;
	unsigned int m_conCommandID;
	std::unique_ptr<pragma::networking::IServer> m_server;
	std::shared_ptr<pragma::networking::IServerClient> m_localClient = {};

	// Handles the connection to the master server
	std::unique_ptr<pragma::networking::MasterServerRegistration> m_serverReg;

	ChronoTimePoint m_tNextWMSConnect;
	unsigned int m_alsoundID;

	std::deque<unsigned int> m_alsoundIndex;
	// We need to keep shared pointer references to all serverside sounds (Network state only keeps references)
	std::vector<std::shared_ptr<ALSound>> m_serverSounds;
  protected:
	virtual void implFindSimilarConVars(const std::string &input, std::vector<SimilarCmdInfo> &similarCmds) const override;
	virtual Material *LoadMaterial(const std::string &path, bool precache, bool bReload) override;
	virtual void InitializeResourceManager() override;
	void ClearConCommands();
	void OnMasterServerRegistered(bool b, std::string reason);
	void RegisterServerInfo();
	void InitializeGameServer(bool singlePlayerLocalGame);
	void ResetGameServer();
	WMServerData m_serverData;
  public:
	using NetworkState::LoadMaterial;
	virtual void Initialize() override;
	virtual void Think() override;
	virtual void Tick() override;
	virtual void Close() override;
	virtual NwStateType GetType() const override;
	WMServerData &GetServerData();
	void SendResourceFile(const std::string &f, const std::vector<pragma::networking::IServerClient *> &clients);
	void SendResourceFile(const std::string &f);
	void SendRoughModel(const std::string &f, const std::vector<pragma::networking::IServerClient *> &clients);
	void SendRoughModel(const std::string &f);
	void SendSoundSourceToClient(SALSound &sound, bool sendFullUpdate, const pragma::networking::ClientRecipientFilter *rf = nullptr);
	// ConVars
	virtual ConVar *SetConVar(std::string scmd, std::string value, bool bApplyIfEqual = false) override;
	// Sound
	virtual std::shared_ptr<ALSound> CreateSound(std::string snd, ALSoundType type, ALCreateFlags flags = ALCreateFlags::None) override;
	virtual std::shared_ptr<ALSound> GetSoundByIndex(unsigned int idx) override;
	virtual void UpdateSounds() override;
	virtual bool PrecacheSound(std::string snd, ALChannel mode = ALChannel::Auto) override;
	virtual void StopSounds() override;
	virtual void StopSound(std::shared_ptr<ALSound> pSnd) override;
	// Game
	virtual void StartGame(bool singlePlayer) override;
	virtual void ChangeLevel(const std::string &map) override;

	void UpdatePlayerScore(pragma::SPlayerComponent &pl, int32_t score);
	void UpdatePlayerName(pragma::SPlayerComponent &pl, const std::string &name);

	virtual std::string GetMessagePrefix() const override;
	void OnClientConVarChanged(pragma::BasePlayerComponent &pl, std::string cvar, std::string value);
	// if 'wasAuthenticationSuccessful' is not set, no authentication was required
	void OnClientAuthenticated(pragma::networking::IServerClient &session, std::optional<bool> wasAuthenticationSuccessful);
	pragma::SPlayerComponent *GetPlayer(const pragma::networking::IServerClient &session);

	virtual bool IsMultiPlayer() const override;
	virtual bool IsSinglePlayer() const override;
	void StartServer(bool singlePlayer);
	void CloseServer();
	pragma::networking::IServerClient *GetLocalClient();
	void InitResourceTransfer(pragma::networking::IServerClient &session);
	void HandleServerNextResource(pragma::networking::IServerClient &session);
	void HandleServerResourceStart(pragma::networking::IServerClient &session, NetPacket &packet);
	void HandleServerResourceFragment(pragma::networking::IServerClient &session);
	void HandleLuaNetPacket(pragma::networking::IServerClient &session, NetPacket &packet);
	bool HandlePacket(pragma::networking::IServerClient &session, NetPacket &packet);
	void ReceiveUserInput(pragma::networking::IServerClient &session, NetPacket &packet);
	bool ConnectLocalHostPlayerClient();
  public:
	static ServerState *Get();
	ServerState();
	virtual ~ServerState() override;
	virtual msys::MaterialManager &GetMaterialManager() override;
	virtual ModelSubMesh *CreateSubMesh() const override;
	virtual ModelMesh *CreateMesh() const override;
	virtual bool IsServer() const override;
	virtual ConVarMap *GetConVarMap() override;
	ServerMessageMap *GetNetMessageMap();
	SVNetMessage *GetNetMessage(unsigned int ID);
	unsigned int GetClientMessageID(std::string identifier);
	virtual ConCommand *CreateConCommand(const std::string &scmd, LuaFunction fc, ConVarFlags flags = ConVarFlags::None, const std::string &help = "") override;
	void GetLuaConCommands(std::unordered_map<std::string, ConCommand *> **cmds);

	bool IsClientAuthenticationRequired() const;
	void SetServerInterface(std::unique_ptr<pragma::networking::IServer> iserver);

	// Game
	SGame *GetGameState();
	virtual void EndGame() override;
	virtual bool IsGameActive() override;

	// Sound
	virtual bool LoadSoundScripts(const char *file, bool bPrecache = false) override;

	void SendPacket(const std::string &name, NetPacket &packet, pragma::networking::Protocol protocol, const pragma::networking::ClientRecipientFilter &rf);
	void SendPacket(const std::string &name, NetPacket &packet, pragma::networking::Protocol protocol);
	void SendPacket(const std::string &name, NetPacket &packet);
	void SendPacket(const std::string &name, pragma::networking::Protocol protocol);

	pragma::networking::IServer *GetServer();
	pragma::networking::MasterServerRegistration *GetMasterServerRegistration();
	bool IsServerRunning() const;
	void DropClient(pragma::networking::IServerClient &session, pragma::networking::DropReason reason = pragma::networking::DropReason::Disconnected);
};
#pragma warning(pop)
export {
	namespace pragma {
	    constexpr inline uint32_t FSYS_SEARCH_CACHE = 8'192;
	};
};
