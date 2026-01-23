// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "util_enum_flags.hpp"

export module pragma.client:game;

export import :core.lua_input_binding_layer_register;
export import :entities.base_entity;
export import :entities.components.player;
export import :rendering.enums;
export import :rendering.game_world_shader_settings;
export import :rendering.global_render_settings_buffer_data;
export import :rendering.global_shader_input_manager;
export import :rendering.gpu_profiler;
export import :rendering.render_queue_worker;
export import :rendering.shaders.lua;
export import :scripting.lua.gui_manager;
export import pragma.soundsystem;
export import pragma.platform;
export import pragma.string.unicode;

export {
	inline constexpr auto LOD_SWAP_DISTANCE = 500.f;
	inline constexpr auto LOD_SWAP_DISTANCE_SQR = pragma::math::pow2(LOD_SWAP_DISTANCE);

	struct DLLCLIENT KeyAction {
		KeyAction(int key, int action)
		{
			this->key = key;
			this->action = action;
		}
		int key, action;
	};
};

// These are only used for the transition phase to c++20 modules to resolve
// some predeclaration issues. Once the transition is complete, these types should be
// reverted to the actual types.
export namespace pragma {
	struct cxxm_LuaParticleModifierManager {};
	using cxxm_LuaParticleModifierManager_vp = void;
};

#pragma warning(push)
#pragma warning(disable : 4251)
export namespace pragma {
	namespace ecs {
		class CBaseEntity;
	}
	class DLLCLIENT CGame : public Game {
	  public:
		CGame(NetworkState *state);
		virtual ~CGame() override;
		virtual void OnRemove() override;

		enum class SoundCacheFlags : uint32_t {
			None = 0,
			BakeConvolution = 1,
			BakeParametric = BakeConvolution << 1,
			SaveProbeBoxes = BakeParametric << 1,

			BakeAll = BakeConvolution | BakeParametric,
			All = BakeAll | SaveProbeBoxes
		};

		enum class StateFlags : uint32_t { None = 0u, GameWorldShaderPipelineReloadRequired = 1u, PrepassShaderPipelineReloadRequired = GameWorldShaderPipelineReloadRequired << 1u, DisableGamplayControlCamera = PrepassShaderPipelineReloadRequired << 1u };

		// List of generic shaders used by the rendering pipeline for direct access to m_gameShaders
		enum class GameShader {
			Shadow = 0u,
			ShadowSpot,
			ShadowTransparent,
			ShadowTransparentSpot,
			ShadowCSM,
			ShadowCSMTransparent,

			PPFog,
			PPFXAA,
			PPTonemapping,
			PPDoF,

			Debug,
			DebugTexture,
			DebugVertex,

			Prepass,
			Pbr,

			Count
		};

		void ReloadSoundCache(bool bReloadBakedCache = false, SoundCacheFlags cacheFlags = SoundCacheFlags::All, float spacing = 1'024.f);
		void ClearSoundCache();

		void SetRenderClipPlane(const Vector4 &clipPlane);
		const Vector4 &GetRenderClipPlane() const;

		void ReloadMaterialShader(material::CMaterial *mat);
		void SetRenderModeEnabled(rendering::SceneRenderPass renderMode, bool bEnabled);
		void EnableRenderMode(rendering::SceneRenderPass renderMode);
		void DisableRenderMode(rendering::SceneRenderPass renderMode);
		bool IsRenderModeEnabled(rendering::SceneRenderPass renderMode) const;

		rendering::RenderMask RegisterRenderMask(const std::string &name, bool inclusiveByDefault = true);
		rendering::RenderMask GetInclusiveRenderMasks() const;
		rendering::RenderMask GetExclusiveRenderMasks() const;
		bool IsInclusiveRenderMask(rendering::RenderMask mask) const;
		bool IsExclusiveRenderMask(rendering::RenderMask mask) const;
		std::optional<rendering::RenderMask> GetRenderMask(const std::string &name);
		const std::string *FindRenderMaskName(rendering::RenderMask mask) const;

		std::shared_ptr<prosper::IPrimaryCommandBuffer> GetCurrentDrawCommandBuffer() const;
		virtual void InitializeLua() override;
		virtual void SetupLua() override;
		virtual void SetUp() override;
		virtual bool IsServer() override;
		virtual bool IsClient() override;
		bool IsInMainRenderPass() const;
		virtual void RegisterLua() override;
		virtual void RegisterLuaLibraries() override;
		virtual void RegisterLuaClasses() override;
		void HandleLuaNetPacket(NetPacket &packet);
		void SendUserInput();
		template<typename TCPPM>
		void CreateGiblet(const GibletCreateInfo &info, TCPPM **particle);
		virtual BaseEntityComponent *CreateLuaEntityComponent(ecs::BaseEntity &ent, std::string classname) override;
		virtual void CreateGiblet(const GibletCreateInfo &info) override;
		virtual std::shared_ptr<geometry::ModelMesh> CreateModelMesh() const override;
		virtual std::shared_ptr<geometry::ModelSubMesh> CreateModelSubMesh() const override;
		virtual void GetRegisteredEntities(std::vector<std::string> &classes, std::vector<std::string> &luaClasses) const override;

		bool StartGPUProfilingStage(const char *stage);
		bool StopGPUProfilingStage();
		debug::ProfilingStageManager<debug::GPUProfilingStage> *GetGPUProfilingStageManager();

		std::shared_ptr<audio::IEffect> GetAuxEffect(const std::string &name);

		debug::ProfilingStageManager<debug::ProfilingStage> *GetProfilingStageManager();
		bool StartProfilingStage(const char *stage);
		bool StopProfilingStage();

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

		// Game
		virtual void Think() override;
		virtual void Tick() override;
		virtual bool LoadMap(const std::string &map, const Vector3 &origin = {}, std::vector<EntityHandle> *entities = nullptr) override;
		void BuildVMF(const char *map);
		virtual double &ServerTime() override;
		void SetServerTime(double t);
		// Entities
		virtual ecs::CBaseEntity *CreateLuaEntity(std::string classname, bool bLoadIfNotExists = false) override;
		ecs::CBaseEntity *CreateLuaEntity(std::string classname, unsigned int idx, bool bLoadIfNotExists = false);
		virtual ecs::CBaseEntity *GetEntity(unsigned int idx) override;
		virtual ecs::CBaseEntity *GetEntityByLocalIndex(uint32_t idx) override;
		ecs::CBaseEntity *GetEntityByClientIndex(unsigned int idx);
		virtual void GetPlayers(std::vector<ecs::BaseEntity *> *ents) override;
		virtual void GetNPCs(std::vector<ecs::BaseEntity *> *ents) override;
		virtual void GetWeapons(std::vector<ecs::BaseEntity *> *ents) override;
		virtual void GetVehicles(std::vector<ecs::BaseEntity *> *ents) override;

		virtual void GetPlayers(std::vector<EntityHandle> *ents) override;
		virtual void GetNPCs(std::vector<EntityHandle> *ents) override;
		virtual void GetWeapons(std::vector<EntityHandle> *ents) override;
		virtual void GetVehicles(std::vector<EntityHandle> *ents) override;

		template<typename TCPPM>
		TCPPM *GetGameComponent();
		template<typename TCPPM>
		const TCPPM *GetGameComponent() const;
		virtual ecs::CBaseEntity *CreateEntity(std::string classname) override;
		template<class T>
		T *CreateEntity()
		{
			if(math::is_flag_set(m_flags, GameFlags::ClosingGame))
				return nullptr;
			return CreateEntity<T>(GetFreeEntityIndex());
		}
		template<class T>
		T *CreateEntity(unsigned int idx)
		{
			if(math::is_flag_set(m_flags, GameFlags::ClosingGame))
				return nullptr;
			T *ent = new T();
			SetupEntity(ent, idx);
			return ent;
		}
		virtual void RemoveEntity(ecs::BaseEntity *ent) override;
		template<typename TCPPM>
		TCPPM *GetListener();
		CPlayerComponent *GetLocalPlayer();
		void GetPrimaryCameraRenderMask(rendering::RenderMask &inclusionMask, rendering::RenderMask &exclusionMask) const;
		void SetLocalPlayer(CPlayerComponent *pl);
		virtual void SpawnEntity(ecs::BaseEntity *ent) override;
		void GetEntities(std::vector<ecs::CBaseEntity *> **ents);
		virtual void GetEntities(std::vector<ecs::BaseEntity *> **ents) override;
		void GetSharedEntities(std::vector<ecs::CBaseEntity *> **ents);
		void GetSharedEntities(std::vector<ecs::BaseEntity *> **ents);

		// Sockets
		void ReceiveSnapshot(NetPacket &packet);

		virtual Float GetFrictionScale() const override;
		virtual Float GetRestitutionScale() const override;

		template<typename TCPPM>
		TCPPM *CreateParticleTracer(const Vector3 &start, const Vector3 &end, float radius = game::bulletInfo::DEFAULT_TRACER_RADIUS, const Color &col = game::bulletInfo::DEFAULT_TRACER_COLOR, float length = game::bulletInfo::DEFAULT_TRACER_LENGTH,
		  float speed = game::bulletInfo::DEFAULT_TRACER_SPEED, const std::string &material = std::string {game::bulletInfo::DEFAULT_TRACER_MATERIAL}, float bloomScale = game::bulletInfo::DEFAULT_TRACER_BLOOM);

		virtual bool IsPhysicsSimulationEnabled() const override;

		// Debug
		virtual void DrawLine(const Vector3 &start, const Vector3 &end, const Color &color, float duration = 0.f) override;
		virtual void DrawBox(const Vector3 &origin, const Vector3 &start, const Vector3 &end, const EulerAngles &ang, const Color &colorOutline, const std::optional<Color> &fillColor, float duration = 0.f) override;
		virtual void DrawPlane(const Vector3 &n, float dist, const Color &color, float duration = 0.f) override;
		virtual void DrawMesh(const std::vector<Vector3> &meshVerts, const Color &color, const Color &colorOutline, float duration = 0.f) override;
		template<typename TCPPM>
		void RenderDebugPhysics(std::shared_ptr<prosper::ICommandBuffer> &drawCmd, TCPPM &cam);

		using Game::LoadNavMesh;

		void OnReceivedRegisterNetEvent(NetPacket &packet);
		virtual NetEventId FindNetEvent(const std::string &name) const override;
		virtual NetEventId SetupNetEvent(const std::string &name) override;
		NetEventId SharedNetEventIdToLocal(NetEventId evId) const;
		NetEventId LocalNetEventIdToShared(NetEventId evId) const;
		std::unordered_map<std::string, NetEventId> &GetLocalNetEventIds() { return m_clientNetEventData.localNetEventIds; }
		std::vector<NetEventId> &GetSharedNetEventIdToLocal() { return m_clientNetEventData.sharedNetEventIdToLocalId; }
		std::vector<NetEventId> &GetLocalNetEventIdToShared() { return m_clientNetEventData.localNetEventIdToSharedId; }

		void SetLODBias(int32_t bias);
		int32_t GetLODBias() const;
		uint32_t GetLOD(float dist, uint32_t maxLod = std::numeric_limits<uint32_t>::max()) const;
		LuaCallbackHandler &GetInputCallbackHandler();
		uint32_t GetMSAASampleCount();
		void SetMaterialOverride(material::Material *mat);
		material::Material *GetMaterialOverride();
		void SetColorScale(const Vector4 &col);
		Vector4 &GetColorScale();
		void SetAlphaScale(float a);
		float GetAlphaScale();
		LuaGUIManager &GetLuaGUIManager();
		LuaShaderManager &GetLuaShaderManager();
		cxxm_LuaParticleModifierManager &GetLuaParticleModifierManager();
		LuaInputBindingLayerRegister &GetLuaInputBindingLayerRegister();
		material::Material *GetLoadMaterial();
		virtual bool RunLua(const std::string &lua) override;
		virtual bool InvokeEntityEvent(BaseEntityComponent &component, uint32_t eventId, int32_t argsIdx, bool bInject) override;
		void OnReceivedPlayerInputResponse(uint8_t userInputId);
		uint16_t GetLatency() const;
		// Returns the number of lost snapshot packets within the last second
		uint32_t GetLostPacketCount();

		template<typename TCPPM>
		TCPPM *CreateCamera(uint32_t width, uint32_t height, float fov, float nearZ, float farZ);
		template<typename TCPPM>
		TCPPM *CreateCamera(float aspectRatio, float fov, float nearZ, float farZ);
		template<typename TCPPM>
		TCPPM *GetPrimaryCamera() const;
		template<typename TCPPM>
		const TCPPM *GetScene() const;
		template<typename TCPPM>
		TCPPM *GetScene();
		const rendering::WorldEnvironment &GetWorldEnvironment() const;
		rendering::WorldEnvironment &GetWorldEnvironment();

		virtual float GetTimeScale() override;
		virtual void SetTimeScale(float t) override;
		void RequestResource(const std::string &fileName);

		void UpdateEntityModel(ecs::CBaseEntity *ent);
		template<typename TCPPM>
		TCPPM *GetViewModel();
		template<typename TCPPM>
		TCPPM *GetViewBody();
		void ReloadRenderFrameBuffer();

		void RenderScenes(rendering::DrawSceneInfo &drawSceneInfo);
		void RenderScene(const rendering::DrawSceneInfo &drawSceneInfo);

		// GUI
		void PreGUIDraw();
		void PostGUIDraw();
		void PreGUIRecord();
		void PostGUIRecord();

		// HDR
		void Resize(bool reloadRenderTarget);
		Float GetHDRExposure() const;
		Float GetMaxHDRExposure() const;
		void SetMaxHDRExposure(Float exposure);

		gui::Element *CreateGUIElement(std::string name, gui::Element *parent = nullptr);
		gui::Element *CreateGUIElement(std::string name, gui::WIHandle *hParent);
		template<class TElement>
		TElement *CreateGUIElement(gui::Element *parent = nullptr)
		{
			TElement *p = gui::WGUI::GetInstance().Create<TElement>(parent);
			if(p == nullptr)
				return nullptr;
			//InitializeGUIElement(p);
			return p;
		}
		template<class TElement>
		TElement *CreateGUIElement(gui::WIHandle *hParent)
		{
			gui::Element *pParent = nullptr;
			if(hParent != nullptr && hParent->IsValid())
				pParent = hParent->get();
			return CreateGUIElement<TElement>(pParent);
		}
		virtual void Initialize() override;
		virtual void InitializeGame() override;

		void SetViewModelFOV(float fov);
		const util::PFloatProperty &GetViewModelFOVProperty() const;
		float GetViewModelFOV() const;
		float GetViewModelFOVRad() const;
		Mat4 GetViewModelProjectionMatrix() const;

		// Shaders
		const util::WeakHandle<prosper::Shader> &GetGameShader(GameShader shader) const;

		// Lights
		BaseEnvLightDirectionalComponent *GetEnvironmentLightSource() const;
		void UpdateEnvironmentLightSource();

		// Inputs
		void SetActionInput(Action action, bool b, bool bKeepMagnitude);
		void SetActionInput(Action action, bool b, float magnitude = 1.f);
		bool GetActionInput(Action action);

		// Util
		bool SaveImage(prosper::IImage &image, const std::string &fileName, const image::TextureInfo &imageWriteInfo) const;
		bool SaveImage(const std::vector<std::vector<const void *>> &imgLayerMipmapData, uint32_t width, uint32_t height, uint32_t szPerPixel, const std::string &fileName, const image::TextureInfo &imageWriteInfo, bool cubemap = false) const;
		bool SaveImage(image::ImageBuffer &imgBuffer, const std::string &fileName, const image::TextureInfo &imageWriteInfo, bool cubemap = false) const;

		virtual std::string GetLuaNetworkDirectoryName() const override;
		virtual std::string GetLuaNetworkFileName() const override;

		void SetDefaultGameRenderEnabled(bool enabled);
		bool IsDefaultGameRenderEnabled() const;

		uint32_t GetNumberOfScenesQueuedForRendering() const;
		rendering::DrawSceneInfo *GetQueuedSceneRenderInfo(uint32_t i);
		void QueueForRendering(const rendering::DrawSceneInfo &drawSceneInfo);
		void RenderScenes(const std::vector<rendering::DrawSceneInfo> &drawSceneInfos);
		template<typename TCPPM>
		void SetRenderScene(TCPPM &scene);
		void ResetRenderScene();
		template<typename TCPPM>
		TCPPM *GetRenderScene();
		template<typename TCPPM>
		const TCPPM *GetRenderScene() const;
		template<typename TCPPM>
		TCPPM *GetRenderCamera() const;
		template<typename TCPPM>
		void SetGameplayControlCamera(TCPPM &cam);
		void ClearGameplayControlCamera();
		void ResetGameplayControlCamera();
		template<typename TCPPM>
		TCPPM *GetGameplayControlCamera();

		rendering::GlobalShaderInputDataManager &GetGlobalShaderInputDataManager();
		const rendering::GlobalShaderInputDataManager &GetGlobalShaderInputDataManager() const;

		rendering::RenderQueueBuilder &GetRenderQueueBuilder();
		rendering::RenderQueueWorkerManager &GetRenderQueueWorkerManager();
		prosper::IDescriptorSet &GetGlobalRenderSettingsDescriptorSet();
		rendering::GlobalRenderSettingsBufferData &GetGlobalRenderSettingsBufferData();
		void ReloadGameWorldShaderPipelines() const;
		void ReloadPrepassShaderPipelines() const;
		void OnGameWorldShaderSettingsChanged(const rendering::GameWorldShaderSettings &newSettings, const rendering::GameWorldShaderSettings &oldSettings);

		// For internal use only!
		const std::vector<rendering::DrawSceneInfo> &GetQueuedRenderScenes() const;
	  protected:
		virtual void RegisterLuaEntityComponents(luabind::module_ &gameMod) override;
		virtual void OnMapLoaded() override;
		virtual void InitializeWorldData(asset::WorldData &worldData) override;
		virtual void InitializeMapEntities(asset::WorldData &worldData, std::vector<EntityHandle> &outEnts) override;

		template<class T>
		void GetPlayers(std::vector<T *> *ents);
		template<class T>
		void GetNPCs(std::vector<T *> *ents);
		template<class T>
		void GetWeapons(std::vector<T *> *ents);
		template<class T>
		void GetVehicles(std::vector<T *> *ents);

		virtual void UpdateTime() override;
		virtual void OnEntityCreated(ecs::BaseEntity *ent) override;
		virtual unsigned int GetFreeEntityIndex() override;
		virtual void SetupEntity(ecs::BaseEntity *ent, unsigned int idx) override;
		virtual void InitializeLuaScriptWatcher() override;
		virtual std::shared_ptr<EntityComponentManager> InitializeEntityComponentManager() override;
	  private:
		std::queue<gui::WIHandle> m_luaGUIObjects = {};
		double m_tLastClientUpdate = 0.0;
		std::array<bool, math::to_integral(rendering::SceneRenderPass::Count)> m_renderModesEnabled;
		CallbackHandle m_hCbDrawFrame = {};

		struct {
			std::unordered_map<std::string, NetEventId> localNetEventIds {};
			NetEventId nextLocalNetEventId = 0;
			std::vector<NetEventId> sharedNetEventIdToLocalId {};
			std::vector<NetEventId> localNetEventIdToSharedId {};
		} m_clientNetEventData {};

		CallbackHandle m_cbGPUProfilingHandle = {};
		std::unique_ptr<debug::ProfilingStageManager<debug::GPUProfilingStage>> m_gpuProfilingStageManager;
		CallbackHandle m_cbProfilingHandle = {};
		std::unique_ptr<debug::ProfilingStageManager<debug::ProfilingStage>> m_profilingStageManager;

		struct DLLCLIENT MessagePacketTracker {
			MessagePacketTracker();
			uint8_t lastInMessageId;
			uint8_t outMessageId;
			// Looks for lost packets and updates 'lastInMessageId'
			void CheckMessages(uint8_t messageId, std::vector<double> &lostPacketTimestamps, const double &tCur);
			void CheckMessages(uint8_t messageId, std::vector<double> &lostPacketTimestamps);
			bool IsMessageInOrder(uint8_t messageId) const;
			std::array<double, std::numeric_limits<decltype(lastInMessageId)>::max() + 1> messageTimestamps;
		};
		MessagePacketTracker m_snapshotTracker;
		MessagePacketTracker m_userInputTracker;
		std::vector<double> m_lostPackets;
		void UpdateLostPackets();

		std::vector<std::string> m_requestedResources;

		LuaGUIManager m_luaGUIElements = {};
		std::shared_ptr<LuaShaderManager> m_luaShaderManager = nullptr;
		std::shared_ptr<cxxm_LuaParticleModifierManager_vp> m_luaParticleModifierManager = nullptr;
		double m_tServer = 0.0;
		LuaCallbackHandler m_inputCallbackHandler;
		std::unique_ptr<LuaInputBindingLayerRegister> m_luaInputBindingLayerRegister {};

		// Shaders
		void InitShaders();
		void LoadLuaShaders();
		void LoadLuaShader(std::string file);
		material::MaterialHandle m_matLoad = {};

		// FOV used for view-models
		util::PFloatProperty m_viewFov = nullptr;

		// Sound
		bool LoadAuxEffects(const std::string &fname);

		// Render
		bool m_defaultGameRenderEnabled = true;
		std::vector<rendering::DrawSceneInfo> m_sceneRenderQueue {};
		std::shared_ptr<rendering::RenderQueueBuilder> m_renderQueueBuilder = nullptr;
		std::shared_ptr<rendering::RenderQueueWorkerManager> m_renderQueueWorkerManager = nullptr;
		std::unique_ptr<rendering::GlobalShaderInputDataManager> m_globalShaderInputDataManager;
		Vector4 m_clipPlane = {};
		Vector4 m_colScale = {};
		material::Material *m_matOverride = nullptr;
		bool m_bMainRenderPass = true;
		std::weak_ptr<prosper::IPrimaryCommandBuffer> m_currentDrawCmd = {};
		ComponentHandle<BaseEntityComponent> m_controlCamera {};

		std::array<util::WeakHandle<prosper::Shader>, math::to_integral(GameShader::Count)> m_gameShaders = {};
		StateFlags m_stateFlags = StateFlags::None;
		void RenderScenePresent(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, prosper::Texture &texPostHdr, prosper::IImage *optOutImage, uint32_t layerId = 0u);

		std::unique_ptr<rendering::GlobalRenderSettingsBufferData> m_globalRenderSettingsBufferData;

		// Scene
		util::TWeakSharedHandle<BaseEntityComponent> m_scene = pragma::util::TWeakSharedHandle<BaseEntityComponent> {};
		std::shared_ptr<rendering::WorldEnvironment> m_worldEnvironment = nullptr;

		void OnEnvironmentLightSourceChanged(BaseEnvLightDirectionalComponent *oldSource, BaseEnvLightDirectionalComponent *newSource);

		void UpdateShaderTimeData();

		std::unordered_map<std::string, rendering::RenderMask> m_customRenderMasks;
		rendering::RenderMask m_inclusiveRenderMasks = rendering::RenderMask::None;
		rendering::RenderMask m_exclusiveRenderMasks = rendering::RenderMask::None;
		rendering::RenderMask m_nextCustomRenderMaskIndex = static_cast<rendering::RenderMask>(1);
		rendering::RenderMask m_firstPersonRenderMask = rendering::RenderMask::None;
		rendering::RenderMask m_thirdPersonRenderMask = rendering::RenderMask::None;

		// Entities
		ComponentHandle<BaseEntityComponent> m_gameComponent;
		std::vector<ecs::CBaseEntity *> m_ents;
		std::vector<ecs::CBaseEntity *> m_shEnts;
		std::vector<ecs::CBaseEntity *> m_entsOccluded;
		std::vector<ecs::BaseEntity *> m_shBaseEnts;
		util::TWeakSharedHandle<BaseEnvLightDirectionalComponent> m_hEnvLight = pragma::util::TWeakSharedHandle<BaseEnvLightDirectionalComponent> {};
		util::TWeakSharedHandle<BaseEntityComponent> m_listener = pragma::util::TWeakSharedHandle<BaseEntityComponent> {};
		util::TWeakSharedHandle<CPlayerComponent> m_plLocal = pragma::util::TWeakSharedHandle<CPlayerComponent> {};
		util::TWeakSharedHandle<BaseEntityComponent> m_viewModel = pragma::util::TWeakSharedHandle<BaseEntityComponent> {};
		util::TWeakSharedHandle<BaseEntityComponent> m_viewBody = pragma::util::TWeakSharedHandle<BaseEntityComponent> {};
		util::TWeakSharedHandle<BaseEntityComponent> m_primaryCamera = pragma::util::TWeakSharedHandle<BaseEntityComponent> {};

		util::TWeakSharedHandle<BaseEntityComponent> m_renderScene = pragma::util::TWeakSharedHandle<BaseEntityComponent> {};

		// Map
		virtual std::shared_ptr<nav::Mesh> LoadNavMesh(const std::string &fname) override;

		// Entities
		void GetOccludedEntities(std::vector<ecs::CBaseEntity *> &entsOccluded);

		virtual void InitializeEntityComponents(EntityComponentManager &componentManager) override;
		void InitializeWorldEnvironment();
	};
}
export {
	REGISTER_ENUM_FLAGS(pragma::CGame::SoundCacheFlags)
}
export {
	REGISTER_ENUM_FLAGS(pragma::CGame::GameShader)
}
export {
	REGISTER_ENUM_FLAGS(pragma::CGame::StateFlags)
}
export {
	namespace pragma {
		DLLCLIENT CGame *get_cgame();
	};
}
#pragma warning(pop)

export {
	namespace pragma {
		DLLCLIENT CGame *get_client_game();
	};
};
