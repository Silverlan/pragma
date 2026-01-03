// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:client_state;
export import :audio;
export import :game;
export import :gui;
export import :networking;
export import pragma.string.unicode;

#undef PlaySound

#pragma warning(push)
#pragma warning(disable : 4251)
export namespace pragma {
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
		ResourceDownload(fs::VFilePtrReal file, std::string name, unsigned int size)
		{
			this->file = file;
			this->name = name;
			this->size = size;
		}
		~ResourceDownload()
		{
			if(file != nullptr)
				file.reset();
		}
		fs::VFilePtrReal file;
		std::string name;
		unsigned int size;
	};

	struct DLLCLIENT LastConnectionInfo {
		std::optional<std::pair<std::string, uint16_t>> address = {};
		std::optional<uint64_t> steamId = {};
	};

	class DLLCLIENT ClientState : public NetworkState {
		// For internal use only! Not to be used directly!
	  public:
		virtual std::unordered_map<std::string, std::shared_ptr<console::PtrConVar>> &GetConVarPtrs() override;
		static console::ConVarHandle GetConVarHandle(std::string scvar);
		//
	  private:
		std::unique_ptr<networking::IClient> m_client;
		std::unique_ptr<ServerInfo> m_svInfo;
		std::unique_ptr<ResourceDownload> m_resDownload; // Current resource file being downloaded

		unsigned int GetServerMessageID(std::string identifier);
		unsigned int GetServerConVarID(std::string scmd);
		bool GetServerConVarIdentifier(uint32_t id, std::string &cvar);

		// Sound
		void InitializeSound(audio::CALSound &snd);
		std::vector<std::shared_ptr<audio::ALSound>> m_soundScripts; // 'Regular' sounds are already handled by sound engine, but we still have to take care of sound-scripts
		float m_volMaster;
		std::unordered_map<audio::ALSoundType, float> m_volTypes;

		gui::WIHandle m_hMainMenu;
		gui::WIHandle m_hFps;
		rendering::GameWorldShaderSettings m_worldShaderSettings {};
		LastConnectionInfo m_lastConnection {};
	  protected:
		std::shared_ptr<Lua::Interface> m_luaGUI = nullptr;
		void InitializeGUILua();
		std::vector<std::function<luabind::object(lua::State *, gui::types::WIBase &)>> m_guiLuaWrapperFactories;

		virtual void InitializeResourceManager() override;
		void StartResourceTransfer();
		void InitializeGameClient(bool singlePlayerLocalGame);
		void ResetGameClient();
		void DestroyClient();

		virtual void implFindSimilarConVars(const std::string &input, std::vector<SimilarCmdInfo> &similarCmds) const override;
		virtual material::Material *LoadMaterial(const std::string &path, bool precache, bool bReload) override;
	  public:
		ClientState();
		virtual ~ClientState() override;
		virtual bool IsClient() const override;
	  public:
		virtual bool ShouldRemoveSound(audio::ALSound &snd) override;
		material::Material *LoadMaterial(const std::string &path, const std::function<void(material::Material *)> &onLoaded, bool bReload, bool bLoadInstantly); // TODO
		material::MaterialHandle CreateMaterial(const std::string &path, const std::string &shader);
		material::MaterialHandle CreateMaterial(const std::string &shader);
		bool LoadGUILuaFile(std::string f);
		networking::IClient *GetClient();
		virtual NwStateType GetType() const override;
		virtual void Think() override;
		virtual void Tick() override;
		void Draw(rendering::DrawSceneInfo &drawSceneInfo);
		void Render(rendering::DrawSceneInfo &drawSceneInfo, std::shared_ptr<prosper::RenderTarget> &rt);
		virtual void Close() override;
		virtual console::ConVarMap *GetConVarMap() override;
		bool IsConnected() const;
		void AddGUILuaWrapperFactory(const std::function<luabind::object(lua::State *, gui::types::WIBase &)> &f);
		std::vector<std::function<luabind::object(lua::State *, gui::types::WIBase &)>> &GetGUILuaWrapperFactories();
		virtual material::MaterialManager &GetMaterialManager() override;
		virtual geometry::ModelSubMesh *CreateSubMesh() const override;
		virtual geometry::ModelMesh *CreateMesh() const override;
		virtual util::FileAssetManager *GetAssetManager(asset::Type type) override;
		virtual void Initialize() override;
		virtual std::string GetMessagePrefix() const override;

		rendering::GameWorldShaderSettings &GetGameWorldShaderSettings() { return m_worldShaderSettings; }
		const rendering::GameWorldShaderSettings &GetGameWorldShaderSettings() const { return const_cast<ClientState *>(this)->GetGameWorldShaderSettings(); }
		void UpdateGameWorldShaderSettings();

		gui::types::WIMainMenu *GetMainMenu();

		lua::State *GetGUILuaState();
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
		virtual bool RunConsoleCommand(std::string scmd, std::vector<std::string> &argv, BasePlayerComponent *pl = nullptr, KeyState pressState = KeyState::Press, float magnitude = 1.f, const std::function<bool(console::ConConf *, float &)> &callback = nullptr) override;
		virtual console::ConVar *SetConVar(std::string scmd, std::string value, bool bApplyIfEqual = false) override;
		// Sockets
		void Connect(std::string ip, std::string port = networking::DEFAULT_PORT_TCP);
		// Peer-to-peer only!
		void Connect(uint64_t steamId);
		networking::CLNetMessage *GetNetMessage(unsigned int ID);
		networking::ClientMessageMap *GetNetMessageMap();
		void SendUserInfo();

		void InitializeGUIModule();

		// Sound
		virtual void StopSounds() override;
		virtual void StopSound(std::shared_ptr<audio::ALSound> pSnd) override;
		bool PrecacheSound(std::string snd, std::pair<audio::ISoundBuffer *, audio::ISoundBuffer *> *buffers, audio::ALChannel mode = audio::ALChannel::Auto, bool bLoadInstantly = false);
		virtual bool PrecacheSound(std::string snd, audio::ALChannel mode = audio::ALChannel::Auto) override;
		virtual bool LoadSoundScripts(const char *file, bool bPrecache = false) override;
		virtual std::shared_ptr<audio::ALSound> CreateSound(std::string snd, audio::ALSoundType type, audio::ALCreateFlags flags = audio::ALCreateFlags::None) override;
		std::shared_ptr<audio::ALSound> CreateSound(audio::ISoundBuffer &buffer, audio::ALSoundType type);
		std::shared_ptr<audio::ALSound> CreateSound(audio::Decoder &decoder, audio::ALSoundType type);
		void IndexSound(std::shared_ptr<audio::ALSound> snd, unsigned int idx);
		std::shared_ptr<audio::ALSound> PlaySound(std::string snd, audio::ALSoundType type, audio::ALCreateFlags flags = audio::ALCreateFlags::None);
		std::shared_ptr<audio::ALSound> PlaySound(audio::ISoundBuffer &buffer, audio::ALSoundType type);
		std::shared_ptr<audio::ALSound> PlaySound(audio::Decoder &buffer, audio::ALSoundType type);
		std::shared_ptr<audio::ALSound> PlayWorldSound(audio::ISoundBuffer &buffer, audio::ALSoundType type, const Vector3 &pos);
		std::shared_ptr<audio::ALSound> PlayWorldSound(audio::Decoder &buffer, audio::ALSoundType type, const Vector3 &pos);
		std::shared_ptr<audio::ALSound> PlayWorldSound(std::string snd, audio::ALSoundType type, const Vector3 &pos);
		virtual std::shared_ptr<audio::ALSound> GetSoundByIndex(unsigned int idx) override;
		virtual void UpdateSounds() override;
		void SetMasterSoundVolume(float vol);
		float GetMasterSoundVolume();
		void SetSoundVolume(audio::ALSoundType type, float vol);
		float GetSoundVolume(audio::ALSoundType type);
		void UpdateSoundVolume();
		std::unordered_map<audio::ALSoundType, float> &GetSoundVolumes();

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

		void SendPacket(const std::string &name, NetPacket &packet, networking::Protocol protocol);
		void SendPacket(const std::string &name, NetPacket &packet);
		void SendPacket(const std::string &name, networking::Protocol protocol);

		LastConnectionInfo &GetLastConnectionInfo() { return m_lastConnection; }

		void Disconnect();
		// Game
		virtual bool IsMultiPlayer() const override;
		virtual bool IsSinglePlayer() const override;
		virtual void StartGame(bool singlePlayer) override;
		void StartNewGame(const std::string &gameMode);
		CGame *GetGameState();
		virtual void EndGame() override;
		virtual bool IsGameActive() override;

		void ShowFPSCounter(bool b);

		// Config
		Bool RawMouseInput(platform::MouseButton button, platform::KeyState state, platform::Modifier mods);
		Bool RawKeyboardInput(platform::Key key, int scanCode, platform::KeyState state, platform::Modifier mods, float magnitude = 1.f);
		Bool RawCharInput(unsigned int c);
		Bool RawScrollInput(Vector2 offset);

		Bool MouseInput(platform::MouseButton button, platform::KeyState state, platform::Modifier mods);
		Bool KeyboardInput(platform::Key key, int scanCode, platform::KeyState state, platform::Modifier mods, float magnitude = 1.f);
		Bool CharInput(unsigned int c);
		Bool ScrollInput(Vector2 offset);
		void OnFilesDropped(std::vector<std::string> &files);
		void OnDragEnter(prosper::Window &window);
		void OnDragExit(prosper::Window &window);
		bool OnWindowShouldClose(prosper::Window &window);
		void OnPreedit(prosper::Window &window, const string::Utf8String &preeditString, const std::vector<int> &blockSizes, int focusedBlock, int caret);
		void OnIMEStatusChanged(prosper::Window &window, bool imeEnabled);

		material::Material *LoadMaterial(const std::string &path, const std::function<void(material::Material *)> &onLoaded, bool bReload = false);
		material::Material *LoadMaterial(const std::string &path);

		void ReadEntityData(NetPacket &packet);
	};

	DLLCLIENT ClientState *get_client_state();
};
#pragma warning(pop)
