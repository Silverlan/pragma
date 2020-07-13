/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_ENGINE_H__
#define __C_ENGINE_H__

#include "pragma/c_enginedefinitions.h"
#include <pragma/engine.h>
#include "pragma/launchparameters.h"
#include "pragma/rendering/c_render_context.hpp"
#include "pragma/rendering/c_sci_gpu_timer_manager.hpp"
#include "pragma/input/c_keybind.h"
#include <sharedutils/util_clock.hpp>
#include <unordered_map>

class ClientState;
namespace GLFW {class Joystick;};
namespace al {class SoundSystem;class Effect;};
namespace prosper {class RenderTarget;};
#pragma warning(push)
#pragma warning(disable : 4251)
class DLLCENGINE CEngine
	: public Engine,public pragma::RenderContext
{
public:
	CEngine(int argc,char* argv[]);
	virtual ~CEngine() override;
	virtual void Release() override;
	static const unsigned int MAX_STEREO_SOURCES = 64;
	// Threshold at which axis value represents a key press
	static const float AXIS_PRESS_THRESHOLD;

	enum class GPUProfilingPhase : uint32_t
	{
		Frame = 0u,
		DrawScene,
		GUI,

		Count
	};
	enum class CPUProfilingPhase : uint32_t
	{
		DrawFrame = 0u,
		GUI,
		ClientTick,

		Count
	};
private:
	struct DLLCENGINE ConVarInfo
	{
		std::string cvar;
		std::vector<std::string> argv;
	};
	struct DLLCENGINE ConVarInfoList
	{
		std::vector<ConVarInfo> cvars;
		ConVarInfo *find(const std::string &cmd);
	};
public:
	enum class StateFlags : uint32_t
	{
		None = 0u,
		ControllersEnabled = 1u,
		Fullbright = ControllersEnabled<<1u,
		WindowFocused = Fullbright<<1u,
		FirstFrame = WindowFocused<<1u,
		UniformBlocksInitialized = FirstFrame<<1u,
		VulkanValidationEnabled = UniformBlocksInitialized<<1u,
		ConsoleOpen = VulkanValidationEnabled<<1u,
		TickDeltaTimeTiedToFrameRate = ConsoleOpen<<1u
	};

	using pragma::RenderContext::DrawFrame;
	
	// Debug
	virtual void DumpDebugInformation(ZIPFile &zip) const override;
	pragma::debug::ProfilingStageManager<pragma::debug::GPUProfilingStage,GPUProfilingPhase> *GetGPUProfilingStageManager();
	pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage,CPUProfilingPhase> *GetProfilingStageManager();
	bool StartProfilingStage(CPUProfilingPhase stage);
	bool StopProfilingStage(CPUProfilingPhase stage);
	bool StartProfilingStage(GPUProfilingPhase stage);
	bool StopProfilingStage(GPUProfilingPhase stage);
	CallbackHandle AddGPUProfilingHandler(const std::function<void(bool)> &handler);
	void SetGPUProfilingEnabled(bool bEnabled);

	virtual bool Initialize(int argc,char *argv[]) override;
	StateInstance &GetClientStateInstance();
	virtual void Start() override;
	virtual void Close() override;
	virtual void ClearConsole() override;
	virtual void OpenConsole() override;
	virtual void CloseConsole() override;
	virtual bool IsConsoleOpen() const override;
	void UseFullbrightShader(bool b);
	float GetFPSLimit() const;
	void StartDefaultGame(const std::string &map,bool singlePlayer);
	virtual void StartDefaultGame(const std::string &map) override;
	virtual void EndGame() override;
	virtual bool IsClientConnected() override;
	bool IsWindowFocused() const;
	bool IsValidAxisInput(float axisInput) const;
	void GetMappedKeys(const std::string &cmd,std::vector<GLFW::Key> &keys,uint32_t maxKeys=1);
	// Returns true if the input is a valid button input state (pressed or released)
	// If the input is an axis input, inOutState may change to represent actual button state
	bool GetInputButtonState(float axisInput,GLFW::Modifier mods,GLFW::KeyState &inOutState) const;

	const std::shared_ptr<prosper::RenderTarget> &GetStagingRenderTarget() const;

	void SetRenderResolution(std::optional<Vector2i> resolution);
	Vector2i GetRenderResolution() const;

	// Debug
	pragma::debug::GPUProfiler &GetGPUProfiler() const;
	bool IsGPUProfilingEnabled() const;

	// Config
	void LoadClientConfig();
	void SaveClientConfig();
	bool ExecConfig(const std::string &cfg,std::vector<ConVarInfo> &cmds);
	void ExecCommands(ConVarInfoList &cmds);
	void SetControllersEnabled(bool b);
	bool GetControllersEnabled() const;

	// Sound
	const al::SoundSystem *GetSoundSystem() const;
	al::SoundSystem *GetSoundSystem();
	al::SoundSystem *InitializeSoundEngine();
	void CloseSoundEngine();
	void SetHRTFEnabled(bool b);
	unsigned int GetStereoSourceCount();
	unsigned int GetMonoSourceCount();
	unsigned int GetStereoSource(unsigned int idx);
	template<class TEfxProperties>
		std::shared_ptr<al::Effect> CreateAuxEffect(const std::string &name,const TEfxProperties &props);
	std::shared_ptr<al::Effect> GetAuxEffect(const std::string &name);
	// Lua
	virtual NetworkState *GetNetworkState(lua_State *l) override;
	virtual Lua::Interface *GetLuaInterface(lua_State *l) override;

	float GetNearZ();
	float GetFarZ();
	// Input
	void MouseInput(GLFW::Window &window,GLFW::MouseButton button,GLFW::KeyState state,GLFW::Modifier mods);
	void KeyboardInput(GLFW::Window &window,GLFW::Key key,int scanCode,GLFW::KeyState state,GLFW::Modifier mods,float magnitude=1.f);
	void CharInput(GLFW::Window &window,unsigned int c);
	void ScrollInput(GLFW::Window &window,Vector2 offset);
	void OnWindowFocusChanged(GLFW::Window &window,bool bFocus);
	void OnFilesDropped(GLFW::Window &window,std::vector<std::string> &files);
	void JoystickButtonInput(GLFW::Window &window,const GLFW::Joystick &joystick,uint32_t key,GLFW::KeyState state);
	void JoystickAxisInput(GLFW::Window &window,const GLFW::Joystick &joystick,uint32_t axis,GLFW::Modifier mods,float newVal,float deltaVal);
	float GetRawJoystickAxisMagnitude() const;
	// Util
	virtual bool IsServerOnly() override;
	// Convars
	virtual ConConf *GetConVar(const std::string &cv) override;
	virtual bool RunConsoleCommand(std::string cmd,std::vector<std::string> &argv,KeyState pressState=KeyState::Press,float magnitude=1.f,const std::function<bool(ConConf*,float&)> &callback=nullptr) override;
	// ClientState
	virtual NetworkState *GetClientState() const override;
	ClientState *OpenClientState();
	void CloseClientState();
	void Connect(const std::string &ip,const std::string &port="29150");
	// Peer-to-peer only
	void Connect(uint64_t steamId);
	void Disconnect();
	virtual bool IsMultiPlayer() const override;
	virtual void HandleLocalHostPlayerClientPacket(NetPacket &p) override;
	// KeyMappings
	void MapKey(short c,std::string cmd);
	void MapKey(short c,int function);
	void AddKeyMapping(short c,std::string cmd);
	void RemoveKeyMapping(short c,std::string cmd);
	void ClearLuaKeyMappings();
	void UnmapKey(short c);
	const std::unordered_map<short,KeyBind> &GetKeyMappings() const;
	void ClearKeyMappings();

	// Shaders
	::util::WeakHandle<prosper::Shader> ReloadShader(const std::string &name);
	void ReloadShaderPipelines();
	//

	Double GetDeltaFrameTime() const;
	UInt32 GetFPS() const;
	UInt32 GetFrameTime() const;

	// If specified, 'frameDeltaTime' will be used as the delta time between
	// frames, regardless of the actual delta time. This can be used to make the game think
	// it's running at a different framerate than it actually is.
	void SetFixedFrameDeltaTimeInterpretation(std::optional<std::chrono::nanoseconds> frameDeltaTime);
	void SetFixedFrameDeltaTimeInterpretationByFPS(uint16_t fps);
	// If set to true, the tick-rate will become dependent on the frame-rate. A lower frame-rate
	// will cause the game to slow down, a faster frame-rate to speed up. This should only be
	// used for offline rendering (i.e. recording).
	void SetTickDeltaTimeTiedToFrameRate(bool tieToFrameRate);
protected:
	void DrawScene(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,std::shared_ptr<prosper::RenderTarget> &rt);
	void WriteClientConfig(VFilePtrReal f);
	void PreloadClientConfig();
	void OnRenderResolutionChanged(uint32_t width,uint32_t height);
	virtual void DrawFrame(prosper::IPrimaryCommandBuffer &drawCmd,uint32_t swapchainImageIdx) override;
	virtual void OnResolutionChanged(uint32_t w,uint32_t h) override;
	virtual void OnClose() override;
	virtual void UpdateTickCount() override;
	virtual void OnWindowInitialized() override;
	virtual void LoadConfig() override;
	virtual void InitializeExternalArchiveManager() override;

	virtual void RegisterConsoleCommands() override;
	void InitializeStagingTarget();
private:
	// Sound
	std::shared_ptr<al::SoundSystem> m_soundSystem = nullptr;

	// FPS
	UInt32 m_fps;
	Float m_tFPSTime;
	util::Clock::time_point m_tLastFrame;
	util::Clock::duration m_tDeltaFrameTime;
	std::optional<std::chrono::nanoseconds> m_fixedFrameDeltaTimeInterpretation = {};

	std::unordered_map<std::string,std::shared_ptr<al::Effect>> m_auxEffects;

	std::unique_ptr<pragma::debug::ProfilingStageManager<pragma::debug::GPUProfilingStage,GPUProfilingPhase>> m_gpuProfilingStageManager = nullptr;
	std::unique_ptr<pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage,CPUProfilingPhase>> m_cpuProfilingStageManager = nullptr;

	StateFlags m_stateFlags = StateFlags::FirstFrame;
	float m_speedCam;
	float m_speedCamMouse;
	float m_nearZ,m_farZ;
	std::unique_ptr<StateInstance> m_clInstance;
	std::shared_ptr<prosper::RenderTarget> m_stagingRenderTarget = nullptr;
	std::optional<Vector2i> m_renderResolution = {};

	std::shared_ptr<pragma::debug::GPUProfiler> m_gpuProfiler;
	std::vector<CallbackHandle> m_gpuProfileHandlers = {};

	float m_rawInputJoystickMagnitude = 0.f;
	std::unordered_map<GLFW::Key,GLFW::KeyState> m_joystickKeyStates;
	std::unordered_map<short,KeyBind> m_keyMappings;
	std::unique_ptr<ConVarInfoList> m_preloadedConfig;

	virtual void Think() override;
	virtual void Tick() override;
	void Input(int key,GLFW::KeyState state,GLFW::Modifier mods={},float magnitude=1.f);
	void Input(int key,GLFW::KeyState inputState,GLFW::KeyState pressState,GLFW::Modifier mods,float magnitude=1.f);
	void UpdateFPS(float t);
	void MapKey(short c,std::unordered_map<std::string,std::vector<std::string>> &binds);
};
REGISTER_BASIC_BITWISE_OPERATORS(CEngine::StateFlags)

template<class TEfxProperties>
	std::shared_ptr<al::Effect> CEngine::CreateAuxEffect(const std::string &name,const TEfxProperties &props)
{
	auto lname = name;
	ustring::to_lower(lname);
	auto effect = GetAuxEffect(lname);
	if(effect != nullptr)
		return effect;
	auto *soundSys = GetSoundSystem();
	if(soundSys == nullptr)
		return nullptr;
	try
	{
		effect = soundSys->CreateEffect(props);
	}
	catch(const std::runtime_error &e)
	{
		Con::cwar<<"WARNING: Unable to create auxiliary effect '"<<name<<"': "<<e.what()<<Con::endl;
		return nullptr;
	}
	m_auxEffects.insert(decltype(m_auxEffects)::value_type(name,effect));
	return effect;
}
#pragma warning(pop)

#endif
