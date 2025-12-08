// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.client:engine;

export import :client_state;
export import :rendering.render_context;
export import :rendering.shader_graph.manager;
export import pragma.string.unicode;
export import pragma.shared;

#pragma warning(push)
#pragma warning(disable : 4251)
export namespace pragma {
	class DLLCLIENT CEngine : public pragma::Engine, public rendering::RenderContext {
	  public:
		CEngine(int argc, char *argv[]);
		virtual ~CEngine() override;
		virtual void Release() override;
		static const unsigned int MAX_STEREO_SOURCES = 64;
		// Threshold at which axis value represents a key press
		static const float AXIS_PRESS_THRESHOLD;
	  public:
		enum class StateFlags : uint32_t {
			None = 0u,
			ControllersEnabled = 1u,
			Fullbright = ControllersEnabled << 1u,
			WindowFocused = Fullbright << 1u,
			FirstFrame = WindowFocused << 1u,
			UniformBlocksInitialized = FirstFrame << 1u,
			VulkanValidationEnabled = UniformBlocksInitialized << 1u,
			ConsoleOpen = VulkanValidationEnabled << 1u,
			TickDeltaTimeTiedToFrameRate = ConsoleOpen << 1u,
			EnableGpuPerformanceTimers = TickDeltaTimeTiedToFrameRate << 1u,
			CEClosed = EnableGpuPerformanceTimers << 1u,
			InputBindingsDirty = CEClosed << 1u,
			WindowSizeChanged = InputBindingsDirty << 1u,
		};
		enum class GPUTimer : uint32_t {
			GUI = 0,
			Scene,
			Frame,
			Present,

			Count
		};
		struct DLLCLIENT DroppedFile {
			DroppedFile(const std::string &rootPath, const std::string &fullPath);
			std::string fullPath;
			std::string fileName;
		};

		using rendering::RenderContext::DrawFrame;
		virtual void SetAssetMultiThreadedLoadingEnabled(bool enabled) override;

		virtual bool IsProgramInFocus() const override;

		// Debug
		virtual void DumpDebugInformation(uzip::ZIPFile &zip) const override;
		pragma::debug::ProfilingStageManager<pragma::debug::GPUProfilingStage> *GetGPUProfilingStageManager();
		pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage> *GetProfilingStageManager();
		bool StartProfilingStage(const char *stage);
		bool StopProfilingStage();
		bool StartGPUProfilingStage(const char *stage);
		bool StopGPUProfilingStage();
		CallbackHandle AddGPUProfilingHandler(const std::function<void(bool)> &handler);
		void SetGPUProfilingEnabled(bool bEnabled);

		bool IsWindowless() const;
		bool IsCPURenderingOnly() const;
		bool IsClosed() const;

		virtual bool Initialize(int argc, char *argv[]) override;
		virtual StateInstance &GetStateInstance(pragma::NetworkState &nw) override;
		StateInstance &GetClientStateInstance();
		const std::string &GetDefaultFontSetName() const;
		const FontSet &GetDefaultFontSet() const;
		const FontSet *FindFontSet(const std::string &name) const;
		const std::unordered_map<std::string, std::unique_ptr<FontSet>> &GetFontSets() const { return m_fontSets; }
		virtual void Start() override;
		virtual void Close() override;
		virtual void ClearConsole() override;
		virtual void OpenConsole() override;
		virtual void CloseConsole() override;
		virtual bool IsConsoleOpen() const override;
		virtual void SetConsoleType(ConsoleType type) override;
		virtual ConsoleType GetConsoleType() const override;
		void UseFullbrightShader(bool b);
		float GetFPSLimit() const;
		void StartDefaultGame(const std::string &map, bool singlePlayer);
		virtual void StartDefaultGame(const std::string &map) override;
		virtual void EndGame() override;
		virtual bool IsClientConnected() override;
		const std::vector<DroppedFile> &GetDroppedFiles() const;
		bool IsWindowFocused() const;
		bool IsValidAxisInput(float axisInput) const;
		void GetMappedKeys(const std::string &cmd, std::vector<pragma::platform::Key> &keys, uint32_t maxKeys = 1);
		// Returns true if the input is a valid button input state (pressed or released)
		// If the input is an axis input, inOutState may change to represent actual button state
		bool GetInputButtonState(float axisInput, pragma::platform::Modifier mods, pragma::platform::KeyState &inOutState) const;

		virtual void HandleOpenGLFallback() override;

		void SetRenderResolution(std::optional<Vector2i> resolution);
		Vector2i GetRenderResolution() const;

		// Debug
		pragma::debug::GPUProfiler &GetGPUProfiler() const;
		bool IsGPUProfilingEnabled() const;

		// Config
		void LoadClientConfig();
		void SaveClientConfig();
		void SetControllersEnabled(bool b);
		bool GetControllersEnabled() const;

		// Sound
		void SetAudioAPI(const std::string &audioAPI) { m_audioAPI = audioAPI; }
		const std::string &GetAudioAPI() const { return m_audioAPI; }
		const al::ISoundSystem *GetSoundSystem() const;
		al::ISoundSystem *GetSoundSystem();
		al::ISoundSystem *InitializeSoundEngine();
		void CloseSoundEngine();
		void SetHRTFEnabled(bool b);
		unsigned int GetStereoSourceCount();
		unsigned int GetMonoSourceCount();
		unsigned int GetStereoSource(unsigned int idx);
		template<class TEfxProperties>
		std::shared_ptr<al::IEffect> CreateAuxEffect(const std::string &name, const TEfxProperties &props)
		{
			auto lname = name;
			ustring::to_lower(lname);
			auto effect = GetAuxEffect(lname);
			if(effect != nullptr)
				return effect;
			auto *soundSys = GetSoundSystem();
			if(soundSys == nullptr)
				return nullptr;
			try {
				effect = soundSys->CreateEffect(props);
			}
			catch(const std::runtime_error &e) {
				Con::cwar << "Unable to create auxiliary effect '" << name << "': " << e.what() << Con::endl;
				return nullptr;
			}
			m_auxEffects.insert(decltype(m_auxEffects)::value_type(name, effect));
			return effect;
		}
		std::shared_ptr<al::IEffect> GetAuxEffect(const std::string &name);
		// Lua
		virtual pragma::NetworkState *GetNetworkState(lua::State *l) override;
		virtual Lua::Interface *GetLuaInterface(lua::State *l) override;

		float GetNearZ();
		float GetFarZ();
		// Input
		void MouseInput(prosper::Window &window, pragma::platform::MouseButton button, pragma::platform::KeyState state, pragma::platform::Modifier mods);
		void KeyboardInput(prosper::Window &window, pragma::platform::Key key, int scanCode, pragma::platform::KeyState state, pragma::platform::Modifier mods, float magnitude = 1.f);
		void CharInput(prosper::Window &window, unsigned int c);
		void ScrollInput(prosper::Window &window, Vector2 offset);
		void OnWindowFocusChanged(prosper::Window &window, bool bFocus);
		void OnFilesDropped(prosper::Window &window, std::vector<std::string> &files);
		void OnDragEnter(prosper::Window &window);
		void OnDragExit(prosper::Window &window);
		void OnWindowResized(prosper::Window &window, Vector2i size);
		bool OnWindowShouldClose(prosper::Window &window);
		void JoystickButtonInput(prosper::Window &window, const pragma::platform::Joystick &joystick, uint32_t key, pragma::platform::KeyState state);
		void JoystickAxisInput(prosper::Window &window, const pragma::platform::Joystick &joystick, uint32_t axis, pragma::platform::Modifier mods, float newVal, float deltaVal);
		void OnPreedit(prosper::Window &window, const pragma::string::Utf8String &preeditString, const std::vector<int> &blockSizes, int focusedBlock, int caret);
		void OnIMEStatusChanged(prosper::Window &window, bool imeEnabled);
		float GetRawJoystickAxisMagnitude() const;
		// Util
		virtual bool IsServerOnly() override;
		// Convars
		virtual pragma::console::ConConf *GetConVar(const std::string &cv) override;
		virtual bool RunConsoleCommand(std::string cmd, std::vector<std::string> &argv, KeyState pressState = KeyState::Press, float magnitude = 1.f, const std::function<bool(pragma::console::ConConf *, float &)> &callback = nullptr) override;
		// ClientState
		virtual pragma::NetworkState *GetClientState() const override;
		pragma::ClientState *OpenClientState();
		void CloseClientState();
		void Connect(const std::string &ip, const std::string &port = "29150");
		// Peer-to-peer only
		void Connect(uint64_t steamId);
		void Disconnect();
		virtual bool IsMultiPlayer() const override;
		virtual void HandleLocalHostPlayerClientPacket(NetPacket &p) override;

		void AddInputBindingLayer(const std::shared_ptr<InputBindingLayer> &layer);
		std::vector<std::shared_ptr<InputBindingLayer>> GetInputBindingLayers();
		const std::vector<std::shared_ptr<InputBindingLayer>> GetInputBindingLayers() const { return const_cast<CEngine *>(this)->GetInputBindingLayers(); }
		std::shared_ptr<InputBindingLayer> GetInputBindingLayer(const std::string &name);
		const std::shared_ptr<InputBindingLayer> GetInputBindingLayer(const std::string &name) const { return const_cast<CEngine *>(this)->GetInputBindingLayer(name); }
		bool RemoveInputBindingLayer(const std::string &name);
		std::shared_ptr<InputBindingLayer> GetCoreInputBindingLayer();
		const std::shared_ptr<InputBindingLayer> GetCoreInputBindingLayer() const { return const_cast<CEngine *>(this)->GetCoreInputBindingLayer(); }
		void SetInputBindingsDirty();
		const InputBindingLayer &GetEffectiveInputBindingLayer();

		// Shaders
		::util::WeakHandle<prosper::Shader> ReloadShader(const std::string &name);
		void ReloadShaderPipelines();
		pragma::rendering::ShaderGraphManager &GetShaderGraphManager() { return *m_shaderGraphManager; }
		const pragma::rendering::ShaderGraphManager &GetShaderGraphManager() const { return const_cast<CEngine *>(this)->GetShaderGraphManager(); }
		//

		Double GetDeltaFrameTime() const;
		double GetFPS() const;
		double GetFrameTime() const;

		// If specified, 'frameDeltaTime' will be used as the delta time between
		// frames, regardless of the actual delta time. This can be used to make the game think
		// it's running at a different framerate than it actually is.
		void SetFixedFrameDeltaTimeInterpretation(std::optional<std::chrono::nanoseconds> frameDeltaTime);
		void SetFixedFrameDeltaTimeInterpretationByFPS(uint16_t fps);
		// If set to true, the tick-rate will become dependent on the frame-rate. A lower frame-rate
		// will cause the game to slow down, a faster frame-rate to speed up. This should only be
		// used for offline rendering (i.e. recording).
		void SetTickDeltaTimeTiedToFrameRate(bool tieToFrameRate);

		void InitializeWindowInputCallbacks(prosper::Window &window);
		std::shared_ptr<prosper::Window> CreateWindow(prosper::WindowSettings &settings);

		void SetGpuPerformanceTimersEnabled(bool enabled);
		std::chrono::nanoseconds GetGpuExecutionTime(uint32_t swapchainIdx, GPUTimer timer) const;

		virtual std::unique_ptr<ConVarInfoList> &GetConVarConfig(pragma::NwStateType type) override;
	  protected:
		friend CoreInputBindingLayer;
		void DrawScene(std::shared_ptr<prosper::RenderTarget> &rt);
		void WriteClientConfig(VFilePtrReal f);
		void OnRenderResolutionChanged(uint32_t width, uint32_t height);
		void LoadFontSets();
		void UpdateDirtyInputBindings();
		void RegisterUiElementTypes();
		uint32_t GetPerformanceTimerIndex(uint32_t swapchainIdx, GPUTimer timer) const;
		uint32_t GetPerformanceTimerIndex(GPUTimer timer) const;
		virtual uint32_t DoClearUnusedAssets(pragma::asset::Type type) const override;
		virtual void RunLaunchCommands() override;
		virtual void DrawFrame() override;
		virtual void OnResolutionChanged(uint32_t w, uint32_t h) override;
		virtual void OnClose() override;
		virtual void UpdateTickCount() override;
		virtual void OnWindowInitialized() override;
		virtual void LoadConfig() override;
		virtual void InitializeExternalArchiveManager() override;
		virtual void PreloadConfig(pragma::NwStateType type, const std::string &configName) override;

		virtual void RegisterConsoleCommands() override;
	  private:
		// Sound
		std::shared_ptr<util::Library> m_audioAPILib = nullptr;
		std::shared_ptr<al::ISoundSystem> m_soundSystem = nullptr;
		std::string m_audioAPI;

		// FPS
		double m_fps;
		double m_tFPSTime;
		util::Clock::time_point m_tLastFrame;
		util::Clock::duration m_tDeltaFrameTime;
		util::Clock::time_point m_tWindowResizeTime;
		std::optional<std::chrono::nanoseconds> m_fixedFrameDeltaTimeInterpretation = {};

		std::unordered_map<std::string, std::shared_ptr<al::IEffect>> m_auxEffects;

		std::unique_ptr<pragma::debug::ProfilingStageManager<pragma::debug::GPUProfilingStage>> m_gpuProfilingStageManager;
		std::unique_ptr<pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage>> m_cpuProfilingStageManager;

		StateFlags m_stateFlags = StateFlags::FirstFrame;
		float m_speedCam;
		float m_speedCamMouse;
		float m_nearZ, m_farZ;
		std::unique_ptr<StateInstance> m_clInstance;
		std::unique_ptr<ConVarInfoList> m_clConfig;
		std::unique_ptr<pragma::rendering::ShaderGraphManager> m_shaderGraphManager;
		std::optional<Vector2i> m_renderResolution = {};

		std::shared_ptr<pragma::debug::GPUProfiler> m_gpuProfiler;
		std::vector<CallbackHandle> m_gpuProfileHandlers = {};

		std::string m_defaultFontSet;
		std::unordered_map<std::string, std::unique_ptr<FontSet>> m_fontSets;
		float m_rawInputJoystickMagnitude = 0.f;
		std::unordered_map<pragma::platform::Key, pragma::platform::KeyState> m_joystickKeyStates;

		std::vector<std::shared_ptr<InputBindingLayer>> m_inputBindingLayers;
		std::shared_ptr<InputBindingLayer> m_coreInputBindingLayer;

		std::shared_ptr<prosper::IQueryPool> m_gpuTimerPool = nullptr;
		std::vector<std::shared_ptr<prosper::TimerQuery>> m_gpuTimers;
		std::vector<std::chrono::nanoseconds> m_gpuExecTimes {};

		std::vector<DroppedFile> m_droppedFiles = {}; // Only contains files during OnFilesDropped-call

		virtual void Think() override;
		virtual void Tick() override;
		void Input(int key, pragma::platform::KeyState state, pragma::platform::Modifier mods = {}, float magnitude = 1.f);
		void Input(int key, pragma::platform::KeyState inputState, pragma::platform::KeyState pressState, pragma::platform::Modifier mods, float magnitude = 1.f);
		void UpdateFPS(float t);
	};

	DLLCLIENT CEngine *get_cengine();
};
#pragma warning(pop)

export {REGISTER_ENUM_FLAGS(pragma::CEngine::StateFlags)}

namespace pragma {
	void register_client_launch_parameters(LaunchParaMap &map);
}
