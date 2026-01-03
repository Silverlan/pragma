// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:server_state;

export import :audio;
export import :entities.components;
export import :networking;
export import :game;
export import pragma.shared;
import pragma.wms;

namespace pragma {
#pragma warning(push)
#pragma warning(disable : 4251)
	export class DLLSERVER ServerState : public NetworkState {
		// For internal use only! Not to be used directly!
	  public:
		virtual std::unordered_map<std::string, std::shared_ptr<console::PtrConVar>> &GetConVarPtrs() override;
		static console::ConVarHandle GetConVarHandle(std::string scvar);
		//
	  private:
		std::unordered_map<std::string, console::ConCommand *> m_luaConCommands;
		unsigned int m_conCommandID;
		std::unique_ptr<networking::IServer> m_server;
		std::shared_ptr<networking::IServerClient> m_localClient = {};

		// Handles the connection to the master server
		std::unique_ptr<networking::MasterServerRegistration> m_serverReg;

		ChronoTimePoint m_tNextWMSConnect;
		unsigned int m_alsoundID;

		std::deque<unsigned int> m_alsoundIndex;
		// We need to keep shared pointer references to all serverside sounds (Network state only keeps references)
		std::vector<std::shared_ptr<audio::ALSound>> m_serverSounds;
	  protected:
		virtual void implFindSimilarConVars(const std::string &input, std::vector<SimilarCmdInfo> &similarCmds) const override;
		virtual material::Material *LoadMaterial(const std::string &path, bool precache, bool bReload) override;
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
		void SendResourceFile(const std::string &f, const std::vector<networking::IServerClient *> &clients);
		void SendResourceFile(const std::string &f);
		void SendRoughModel(const std::string &f, const std::vector<networking::IServerClient *> &clients);
		void SendRoughModel(const std::string &f);
		void SendSoundSourceToClient(audio::SALSound &sound, bool sendFullUpdate, const networking::ClientRecipientFilter *rf = nullptr);
		// ConVars
		virtual console::ConVar *SetConVar(std::string scmd, std::string value, bool bApplyIfEqual = false) override;
		// Sound
		virtual std::shared_ptr<audio::ALSound> CreateSound(std::string snd, audio::ALSoundType type, audio::ALCreateFlags flags = audio::ALCreateFlags::None) override;
		virtual std::shared_ptr<audio::ALSound> GetSoundByIndex(unsigned int idx) override;
		virtual void UpdateSounds() override;
		virtual bool PrecacheSound(std::string snd, audio::ALChannel mode = audio::ALChannel::Auto) override;
		virtual void StopSounds() override;
		virtual void StopSound(std::shared_ptr<audio::ALSound> pSnd) override;
		// Game
		virtual void StartGame(bool singlePlayer) override;
		virtual void ChangeLevel(const std::string &map) override;

		void UpdatePlayerScore(SPlayerComponent &pl, int32_t score);
		void UpdatePlayerName(SPlayerComponent &pl, const std::string &name);

		virtual std::string GetMessagePrefix() const override;
		void OnClientConVarChanged(BasePlayerComponent &pl, std::string cvar, std::string value);
		// if 'wasAuthenticationSuccessful' is not set, no authentication was required
		void OnClientAuthenticated(networking::IServerClient &session, std::optional<bool> wasAuthenticationSuccessful);
		SPlayerComponent *GetPlayer(const networking::IServerClient &session);

		virtual bool IsMultiPlayer() const override;
		virtual bool IsSinglePlayer() const override;
		void StartServer(bool singlePlayer);
		void CloseServer();
		networking::IServerClient *GetLocalClient();
		void InitResourceTransfer(networking::IServerClient &session);
		void HandleServerNextResource(networking::IServerClient &session);
		void HandleServerResourceStart(networking::IServerClient &session, NetPacket &packet);
		void HandleServerResourceFragment(networking::IServerClient &session);
		void HandleLuaNetPacket(networking::IServerClient &session, NetPacket &packet);
		bool HandlePacket(networking::IServerClient &session, NetPacket &packet);
		void ReceiveUserInput(networking::IServerClient &session, NetPacket &packet);
		bool ConnectLocalHostPlayerClient();
	  public:
		static ServerState *Get();
		ServerState();
		virtual ~ServerState() override;
		virtual material::MaterialManager &GetMaterialManager() override;
		virtual geometry::ModelSubMesh *CreateSubMesh() const override;
		virtual geometry::ModelMesh *CreateMesh() const override;
		virtual bool IsServer() const override;
		virtual console::ConVarMap *GetConVarMap() override;
		networking::ServerMessageMap *GetNetMessageMap();
		networking::SVNetMessage *GetNetMessage(unsigned int ID);
		unsigned int GetClientMessageID(std::string identifier);
		virtual console::ConCommand *CreateConCommand(const std::string &scmd, LuaFunction fc, console::ConVarFlags flags = console::ConVarFlags::None, const std::string &help = "") override;
		void GetLuaConCommands(std::unordered_map<std::string, console::ConCommand *> **cmds);

		bool IsClientAuthenticationRequired() const;
		void SetServerInterface(std::unique_ptr<networking::IServer> iserver);

		// Game
		SGame *GetGameState();
		virtual void EndGame() override;
		virtual bool IsGameActive() override;

		// Sound
		virtual bool LoadSoundScripts(const char *file, bool bPrecache = false) override;

		void SendPacket(const std::string &name, NetPacket &packet, networking::Protocol protocol, const networking::ClientRecipientFilter &rf);
		void SendPacket(const std::string &name, NetPacket &packet, networking::Protocol protocol);
		void SendPacket(const std::string &name, NetPacket &packet);
		void SendPacket(const std::string &name, networking::Protocol protocol);

		networking::IServer *GetServer();
		networking::MasterServerRegistration *GetMasterServerRegistration();
		bool IsServerRunning() const;
		void DropClient(networking::IServerClient &session, networking::DropReason reason = networking::DropReason::Disconnected);
	};
#pragma warning(pop)
	constexpr inline uint32_t FSYS_SEARCH_CACHE = 8'192;
};
