/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_cengine.h"
#include "pragma/c_engine.h"
#include <wgui/wgui.h>
#include "cmaterialmanager.h"
#include "pragma/console/c_cvar.h"
#include <texturemanager/texturemanager.h>
#include "pragma/gui/wiimageslideshow.h"
#include "pragma/gui/mainmenu/wimainmenu.h"
#include "pragma/gui/wiconsole.hpp"
#include "pragma/gui/wiframe.h"
#include "pragma/asset/c_util_model.hpp"
#include <pragma/engine_init.hpp>
#include <pragma/console/convars.h>
#include "pragma/console/engine_cvar.h"
#include "pragma/model/c_modelmanager.h"
#include "pragma/networking/iclient.hpp"
#include "pragma/networking/local_client.hpp"
#include "pragma/rendering/c_sci_gpu_timer_manager.hpp"
#include <pragma/rendering/scene/util_draw_scene_info.hpp>
#include <pragma/entities/environment/lights/c_env_light.h>
#include <pragma/input/input_binding_layer.hpp>
#include <pragma/lua/lua_error_handling.hpp>
#include <cmaterialmanager.h>
#include <cmaterial_manager2.hpp>
#include <pragma/model/modelmanager.h>
#include <pragma/model/c_modelmesh.h>
#include <cctype>
#include <sharedutils/util_debug.h>
#include <util_zip.h>
#include <pragma/input/inputhelper.h>
#include <fsys/directory_watcher.h>
#include <pragma/game/game_resources.hpp>
#include <pragma/debug/intel_vtune.hpp>
#include <sharedutils/util_file.h>
#include <pragma/engine_info.hpp>
#include <prosper_util.hpp>
#include <shader/prosper_pipeline_loader.hpp>
#include <image/prosper_render_target.hpp>
#include <debug/prosper_debug.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_swap_command_buffer.hpp>
#include <pragma/entities/environment/env_camera.h>
#include <pragma/entities/components/c_render_component.hpp>
#include <pragma/entities/environment/effects/c_env_particle_system.h>
#include <pragma/rendering/shaders/image/c_shader_clear_color.hpp>
#include <pragma/rendering/shaders/image/c_shader_gradient.hpp>
#include <pragma/localization.h>
#include <pragma/logging.hpp>
#include <wgui/types/wicontextmenu.hpp>
#include <wgui/types/witext.h>
#include <wgui/types/witext_tags.hpp>
#include <util_image.hpp>
#include <util_image_buffer.hpp>
#include <queries/prosper_query_pool.hpp>
#include <queries/prosper_timer_query.hpp>
#include <pragma/asset/util_asset.hpp>
#include <prosper_window.hpp>
#include <fsys/ifile.hpp>

extern "C" {
void DLLCLIENT RunCEngine(int argc, char *argv[])
{
	auto en = InitializeEngine<CEngine>(argc, argv);
	if(en == nullptr)
		return;
	en->Release(); // Has to be called before object is actually destroyed, to make sure weak_ptr references are still valid
	en = nullptr;
}
}

DLLCLIENT CEngine *c_engine = NULL;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;
//__declspec(dllimport) std::vector<void*> _vkImgPtrs;
decltype(CEngine::AXIS_PRESS_THRESHOLD) CEngine::AXIS_PRESS_THRESHOLD = 0.5f;
// If set to true, each joystick axes will be split into a positive and a negative axis, which
// can be bound individually
static const auto SEPARATE_JOYSTICK_AXES = true;

CEngine::CEngine(int argc, char *argv[])
    : Engine(argc, argv), pragma::RenderContext(), m_nearZ(pragma::BaseEnvCameraComponent::DEFAULT_NEAR_Z), //10.0f), //0.1f
      m_farZ(pragma::BaseEnvCameraComponent::DEFAULT_FAR_Z), m_fps(0), m_tFPSTime(0.f), m_tLastFrame(util::Clock::now()), m_tDeltaFrameTime(0), m_audioAPI {"fmod"}
{
	c_engine = this;

	RegisterCallback<void, std::reference_wrapper<const GLFW::Joystick>, bool>("OnJoystickStateChanged");
	RegisterCallback<void, std::reference_wrapper<std::shared_ptr<prosper::IPrimaryCommandBuffer>>>("DrawFrame");
	RegisterCallback<void>("PreDrawGUI");
	RegisterCallback<void>("PostDrawGUI");
	RegisterCallback<void>("Draw");

	RegisterCallbackWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, GLFW::MouseButton, GLFW::KeyState, GLFW::Modifier>("OnMouseInput");
	RegisterCallbackWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, GLFW::Key, int, GLFW::KeyState, GLFW::Modifier, float>("OnKeyboardInput");
	RegisterCallbackWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, unsigned int>("OnCharInput");
	RegisterCallbackWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, Vector2>("OnScrollInput");
	RegisterCallbackWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, std::reference_wrapper<const GLFW::Joystick>, uint32_t, GLFW::KeyState>("OnJoystickButtonInput");
	RegisterCallbackWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, std::reference_wrapper<const GLFW::Joystick>, uint32_t, GLFW::Modifier, float, float>("OnJoystickAxisInput");

	AddProfilingHandler([this](bool profilingEnabled) {
		if(profilingEnabled == false) {
			m_profilingStageManager = nullptr;
			return;
		}
		auto &cpuProfiler = c_engine->GetProfiler();
		m_cpuProfilingStageManager = std::make_unique<pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage, CPUProfilingPhase>>();
		auto stageFrame = pragma::debug::ProfilingStage::Create(cpuProfiler, "DrawFrame", &Engine::GetProfilingStageManager()->GetProfilerStage(Engine::CPUProfilingPhase::Think));
		m_cpuProfilingStageManager->InitializeProfilingStageManager(cpuProfiler,
		  {stageFrame, pragma::debug::ProfilingStage::Create(cpuProfiler, "GUI", stageFrame.get()), pragma::debug::ProfilingStage::Create(cpuProfiler, "ClientTick", &Engine::GetProfilingStageManager()->GetProfilerStage(Engine::CPUProfilingPhase::Tick))});
		static_assert(umath::to_integral(CPUProfilingPhase::Count) == 3u, "Added new profiling phase, but did not create associated profiling stage!");
	});
	m_coreInputBindingLayer = std::make_shared<CoreInputBindingLayer>();
	auto inputLayer = std::make_shared<InputBindingLayer>();
	inputLayer->identifier = "core";
	AddInputBindingLayer(inputLayer);

	pragma::asset::AssetManager::ImporterInfo importerInfo {};
	importerInfo.name = "glTF";
	importerInfo.fileExtensions = {{"gltf", false}, {"glb", true}};
	GetAssetManager().RegisterImporter(importerInfo, pragma::asset::Type::Model, [](Game &game, ufile::IFile &f, const std::optional<std::string> &mdlPath, std::string &errMsg) -> std::unique_ptr<pragma::asset::IAssetWrapper> {
		util::Path path {};
		if(mdlPath.has_value()) {
			path = util::Path::CreateFile(*mdlPath);
			path.PopBack();
		}
		auto mdl = pragma::asset::import_model(f, errMsg, path);
		if(mdl == nullptr)
			return nullptr;
		auto wrapper = std::make_unique<pragma::asset::ModelAssetWrapper>();
		wrapper->SetModel(*mdl);
		return wrapper;
	});
}

void CEngine::Release()
{
	Close();
	Engine::Release();
	pragma::RenderContext::Release();
}

pragma::debug::GPUProfiler &CEngine::GetGPUProfiler() const { return *m_gpuProfiler; }
pragma::debug::ProfilingStageManager<pragma::debug::GPUProfilingStage, CEngine::GPUProfilingPhase> *CEngine::GetGPUProfilingStageManager() { return m_gpuProfilingStageManager.get(); }
pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage, CEngine::CPUProfilingPhase> *CEngine::GetProfilingStageManager() { return m_cpuProfilingStageManager.get(); }

static auto cvGPUProfiling = GetClientConVar("cl_gpu_timer_queries_enabled");
bool CEngine::IsGPUProfilingEnabled() const { return cvGPUProfiling->GetBool(); }

void CEngine::DumpDebugInformation(ZIPFile &zip) const
{
	Engine::DumpDebugInformation(zip);

	auto &renderContext = GetRenderContext();

	// Disabled because 'DumpMemoryBudget' can cause stack corruption?
	//auto budget = renderContext.DumpMemoryBudget();
	//if(budget.has_value())
	//	zip.AddFile("mem_budget.txt",*budget);

	auto stats = renderContext.DumpMemoryStats();
	if(stats.has_value())
		zip.AddFile("mem_stats.txt", *stats);

	auto printDeviceInfo = [](std::stringstream &ss, const prosper::util::VendorDeviceInfo &deviceInfo) {
		ss << "Vulkan API Version: " << deviceInfo.apiVersion << "\n";
		ss << "Device Name: " << deviceInfo.deviceName << "\n";
		ss << "Device Type: " << prosper::util::to_string(deviceInfo.deviceType) << "\n";
		ss << "Device ID: " << deviceInfo.deviceId << "\n";
		ss << "Driver Version: " << deviceInfo.driverVersion << "\n";
		ss << "Vendor: " << prosper::util::to_string(deviceInfo.vendor) << "\n";
		ss << "Vendor ID: " << umath::to_integral(deviceInfo.vendor);
	};
	auto deviceInfo = renderContext.GetVendorDeviceInfo();
	if(deviceInfo.has_value()) {
		std::stringstream ss;
		printDeviceInfo(ss, *deviceInfo);
		zip.AddFile("gpu.txt", ss.str());
		ss.str(std::string());
		ss.clear();
	}

	auto deviceInfos = renderContext.GetAvailableVendorDevices();
	if(deviceInfos.has_value()) {
		std::stringstream ss;
		size_t idx = 1;
		for(auto &devInfo : *deviceInfos) {
			ss << "GPU " << idx++ << ":\n";
			printDeviceInfo(ss, devInfo);
			ss << "\n";
		}
		zip.AddFile("available_gpus.txt", ss.str());
		ss.str(std::string());
		ss.clear();
	}

	auto fWriteLuaTraceback = [&zip](lua_State *l, const std::string &identifier) {
		if(!l)
			return;
		std::stringstream ss;
		if(!Lua::PrintTraceback(l, ss))
			return;
		zip.AddFile("lua_traceback_" + identifier + ".txt", ss.str());
	};
	if(GetClientState())
		fWriteLuaTraceback(static_cast<ClientState *>(GetClientState())->GetGUILuaState(), "gui");

#if 0
	prosper::debug::dump_layers(c_engine->GetRenderContext(),ss);
	zip.AddFile("vk_layers.txt",ss.str());

	ss.str(std::string());
	ss.clear();
	prosper::debug::dump_extensions(c_engine->GetRenderContext(),ss);
	zip.AddFile("vk_extensions.txt",ss.str());

	ss.str(std::string());
	ss.clear();
	prosper::debug::dump_limits(c_engine->GetRenderContext(),ss);
	zip.AddFile("vk_limits.txt",ss.str());

	ss.str(std::string());
	ss.clear();
	prosper::debug::dump_features(c_engine->GetRenderContext(),ss);
	zip.AddFile("vk_features.txt",ss.str());

	ss.str(std::string());
	ss.clear();
	prosper::debug::dump_image_format_properties(c_engine->GetRenderContext(),ss);
	zip.AddFile("vk_image_format_properties.txt",ss.str());

	ss.str(std::string());
	ss.clear();
	prosper::debug::dump_format_properties(c_engine->GetRenderContext(),ss);
	zip.AddFile("vk_format_properties.txt",ss.str());
#endif
}

void CEngine::SetRenderResolution(std::optional<Vector2i> resolution)
{
	if(m_renderResolution == resolution)
		return;
	m_renderResolution = resolution;

	resolution = GetRenderResolution();
	OnRenderResolutionChanged(resolution->x, resolution->y);
}
Vector2i CEngine::GetRenderResolution() const
{
	if(m_renderResolution.has_value())
		return *m_renderResolution;
	return GetRenderContext().GetWindow()->GetSize();
}

double CEngine::GetFPS() const { return m_fps; }
double CEngine::GetFrameTime() const { return m_tFPSTime * 1'000.0; }
Double CEngine::GetDeltaFrameTime() const { return util::clock::to_seconds(m_tDeltaFrameTime); }

static auto cvFrameLimit = GetClientConVar("cl_max_fps");
float CEngine::GetFPSLimit() const { return cvFrameLimit->GetFloat(); }

unsigned int CEngine::GetStereoSourceCount() { return 0; }
unsigned int CEngine::GetMonoSourceCount() { return 0; }
unsigned int CEngine::GetStereoSource(unsigned int idx) { return 0; }
float CEngine::GetNearZ() { return m_nearZ; }
float CEngine::GetFarZ() { return m_farZ; }

bool CEngine::IsClientConnected()
{
	auto *cl = static_cast<ClientState *>(GetClientState());
	if(cl == nullptr)
		return false;
	return cl->IsConnected();
}

void CEngine::EndGame()
{
	Disconnect();
	auto *cl = GetClientState();
	if(cl != nullptr)
		cl->EndGame();
	Engine::EndGame();
}

void CEngine::Input(int key, GLFW::KeyState inputState, GLFW::KeyState pressState, GLFW::Modifier mods, float magnitude)
{
	if(inputState == GLFW::KeyState::Press || inputState == GLFW::KeyState::Release || inputState == GLFW::KeyState::Held) {
		auto &inputLayer = GetEffectiveInputBindingLayer();
		auto &keyMappings = inputLayer.GetKeyMappings();
		if((mods & GLFW::Modifier::AxisNegative) != GLFW::Modifier::None) {
			// We need to check if there are any keybindings with a command with the JoystickAxisSingle flag set,
			// in which case that keybinding has priority
			auto keyPositive = key - 1;
			auto it = keyMappings.find(CInt16(keyPositive));
			if(it != keyMappings.end() && const_cast<KeyBind &>(it->second).Execute(inputState, pressState, mods, magnitude) == true)
				return;
			mods &= ~GLFW::Modifier::AxisNegative;
		}
		auto it = keyMappings.find(CInt16(key));
		if(it != keyMappings.end())
			const_cast<KeyBind &>(it->second).Execute(inputState, pressState, mods, magnitude);
	}
}
void CEngine::Input(int key, GLFW::KeyState state, GLFW::Modifier mods, float magnitude) { Input(key, state, state, mods, magnitude); }
void CEngine::MouseInput(prosper::Window &window, GLFW::MouseButton button, GLFW::KeyState state, GLFW::Modifier mods)
{
	auto handled = false;
	if(CallCallbacksWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, GLFW::MouseButton, GLFW::KeyState, GLFW::Modifier>("OnMouseInput", handled, window, button, state, mods) == CallbackReturnType::HasReturnValue && handled == true)
		return;
	if(client != nullptr && client->RawMouseInput(button, state, mods) == false)
		return;
	if(WGUI::GetInstance().HandleMouseInput(window, button, state, mods))
		return;
	button += static_cast<GLFW::MouseButton>(GLFW::Key::Last);
	if(client != nullptr && client->MouseInput(button, state, mods) == false)
		return;
	Input(static_cast<int>(button), state);
}
void CEngine::GetMappedKeys(const std::string &cvarName, std::vector<GLFW::Key> &keys, uint32_t maxKeys)
{
	if(maxKeys != std::numeric_limits<uint32_t>::max())
		keys.reserve(maxKeys);
	if(maxKeys == 0)
		return;
	std::string cmd;
	std::vector<std::string> argv {};
	ustring::get_sequence_commands(cvarName, [&cmd, &argv](std::string cmdOther, std::vector<std::string> argvOther) {
		cmd = cmdOther;
		argv = argvOther;
	});
	for(auto &layer : GetInputBindingLayers()) {
		auto &keyMappings = layer->GetKeyMappings();
		for(auto &pair : keyMappings) {
			auto &keyBind = pair.second;
			auto bFoundCvar = false;
			ustring::get_sequence_commands(keyBind.GetBind(), [&cmd, &argv, &bFoundCvar](std::string cmdOther, std::vector<std::string> &argvOther) {
				if(cmdOther == "toggle" && argvOther.empty() == false)
					cmdOther += " " + argvOther.front();
				if(cmdOther == cmd && argv.size() == argvOther.size()) {
					auto bDiscrepancy = false;
					for(auto i = decltype(argv.size()) {0}; i < argv.size(); ++i) {
						if(argv.at(i) == argvOther.at(i))
							continue;
						bDiscrepancy = true;
						break;
					}
					if(bDiscrepancy == false)
						bFoundCvar = true;
				}
			});
			if(bFoundCvar == true) {
				if(keys.size() == keys.capacity())
					keys.reserve(keys.size() + 10);
				keys.push_back(static_cast<GLFW::Key>(pair.first));
				if(keys.size() == maxKeys)
					break;
			}
		}
	}
}
void CEngine::JoystickButtonInput(prosper::Window &window, const GLFW::Joystick &joystick, uint32_t key, GLFW::KeyState state)
{
	auto handled = false;
	if(CallCallbacksWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, std::reference_wrapper<const GLFW::Joystick>, uint32_t, GLFW::KeyState>("OnJoystickButtonInput", handled, window, joystick, key, state) == CallbackReturnType::HasReturnValue && handled == true)
		return;
	KeyboardInput(window, static_cast<GLFW::Key>(key), -1, state, {});
}
void CEngine::JoystickAxisInput(prosper::Window &window, const GLFW::Joystick &joystick, uint32_t axis, GLFW::Modifier mods, float newVal, float deltaVal)
{
	auto handled = false;
	if(CallCallbacksWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, std::reference_wrapper<const GLFW::Joystick>, uint32_t, GLFW::Modifier, float, float>("OnJoystickAxisInput", handled, window, joystick, axis, mods, newVal, deltaVal) == CallbackReturnType::HasReturnValue
	  && handled == true)
		return;
	auto oldVal = newVal - deltaVal;
	auto key = static_cast<GLFW::Key>(axis);
	auto state = (IsValidAxisInput(newVal) == true) ? GLFW::KeyState::Press : GLFW::KeyState::Release;
	auto it = m_joystickKeyStates.find(key);
	auto oldState = (it == m_joystickKeyStates.end()) ? GLFW::KeyState::Release : it->second;
	if(state == GLFW::KeyState::Release && oldState == GLFW::KeyState::Release)
		return;
	if(state == GLFW::KeyState::Press && oldState == GLFW::KeyState::Press)
		state = GLFW::KeyState::Held;

	m_joystickKeyStates[key] = state;
	mods |= GLFW::Modifier::AxisInput;
	if(umath::abs(newVal) > AXIS_PRESS_THRESHOLD) {
		if(umath::abs(oldVal) <= AXIS_PRESS_THRESHOLD)
			mods |= GLFW::Modifier::AxisPress; // Axis represents actual button press
	}
	else if(umath::abs(oldVal) > AXIS_PRESS_THRESHOLD)
		mods |= GLFW::Modifier::AxisRelease; // Axis represents actual button release
	KeyboardInput(window, key, -1, state, mods, newVal);
}
static auto cvAxisInputThreshold = GetClientConVar("cl_controller_axis_input_threshold");
bool CEngine::IsValidAxisInput(float axisInput) const
{
	if(!client)
		return false;
	return (umath::abs(axisInput) > cvAxisInputThreshold->GetFloat()) ? true : false;
}

bool CEngine::GetInputButtonState(float axisInput, GLFW::Modifier mods, GLFW::KeyState &inOutState) const
{
	if(IsValidAxisInput(axisInput) == false) {
		if((mods & GLFW::Modifier::AxisInput) != GLFW::Modifier::None) {
			inOutState = GLFW::KeyState::Release;
			return true;
		}
		inOutState = GLFW::KeyState::Invalid;
		return false;
	}
	if((mods & GLFW::Modifier::AxisInput) == GLFW::Modifier::None)
		return true; // No need to change state

	if((mods & GLFW::Modifier::AxisPress) != GLFW::Modifier::None)
		inOutState = GLFW::KeyState::Press;
	else if((mods & GLFW::Modifier::AxisRelease) != GLFW::Modifier::None)
		inOutState = GLFW::KeyState::Release;
	else {
		inOutState = GLFW::KeyState::Invalid;
		return false; // Not an actual key press
	}
	return true;
}
void CEngine::KeyboardInput(prosper::Window &window, GLFW::Key key, int scanCode, GLFW::KeyState state, GLFW::Modifier mods, float magnitude)
{
	auto handled = false;
	if(CallCallbacksWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, GLFW::Key, int, GLFW::KeyState, GLFW::Modifier, float>("OnKeyboardInput", handled, window, key, scanCode, state, mods, magnitude) == CallbackReturnType::HasReturnValue && handled == true)
		return;
	if(client != nullptr && client->RawKeyboardInput(key, scanCode, state, mods, magnitude) == false)
		return;
	if(key == GLFW::Key::Escape) // Escape key is hardcoded
	{
		if(client != nullptr) {
			if(state == GLFW::KeyState::Press)
				client->ToggleMainMenu();
			return;
		}
	}
	auto buttonState = state;
	auto bValidButtonInput = GetInputButtonState(magnitude, mods, buttonState);
	if(bValidButtonInput == true) {
		if(WGUI::GetInstance().HandleKeyboardInput(window, key, scanCode, buttonState, mods))
			return;
	}
	if(client != nullptr && client->KeyboardInput(key, scanCode, state, mods, magnitude) == false)
		return;
	key = static_cast<GLFW::Key>(std::tolower(static_cast<int>(key)));
	Input(static_cast<int>(key), state, buttonState, mods, magnitude);
}
void CEngine::CharInput(prosper::Window &window, unsigned int c)
{
	auto handled = false;
	if(CallCallbacksWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, unsigned int>("OnCharInput", handled, window, c) == CallbackReturnType::HasReturnValue && handled == true)
		return;
	if(client != nullptr && client->RawCharInput(c) == false)
		return;
	if(WGUI::GetInstance().HandleCharInput(window, c))
		return;
	if(c == '`') {
		if(IsConsoleOpen())
			CloseConsole();
		else
			OpenConsole();
		return;
	}
	if(client != nullptr && client->CharInput(c) == false)
		return;
}
void CEngine::ScrollInput(prosper::Window &window, Vector2 offset)
{
	auto handled = false;
	if(CallCallbacksWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, Vector2>("OnScrollInput", handled, window, offset) == CallbackReturnType::HasReturnValue && handled == true)
		return;
	if(client != nullptr && client->RawScrollInput(offset) == false)
		return;
	if(WGUI::GetInstance().HandleScrollInput(window, offset))
		return;
	if(client != nullptr && client->ScrollInput(offset) == false)
		return;
	if(offset.y >= 0.f) {
		Input(GLFW_CUSTOM_KEY_SCRL_UP, GLFW::KeyState::Press);
		Input(GLFW_CUSTOM_KEY_SCRL_UP, GLFW::KeyState::Release);
	}
	else {
		Input(GLFW_CUSTOM_KEY_SCRL_DOWN, GLFW::KeyState::Press);
		Input(GLFW_CUSTOM_KEY_SCRL_DOWN, GLFW::KeyState::Release);
	}
}

void CEngine::OnWindowFocusChanged(prosper::Window &window, bool bFocused)
{
	umath::set_flag(m_stateFlags, StateFlags::WindowFocused, bFocused);
	if(client != nullptr)
		client->UpdateSoundVolume();
}
void CEngine::OnFilesDropped(prosper::Window &window, std::vector<std::string> &files)
{
	if(client == nullptr)
		return;
	client->OnFilesDropped(files);
}
bool CEngine::IsWindowFocused() const { return umath::is_flag_set(m_stateFlags, StateFlags::WindowFocused); }

void CEngine::SetAssetMultiThreadedLoadingEnabled(bool enabled)
{
	Engine::SetAssetMultiThreadedLoadingEnabled(enabled);
	auto *cl = GetClientState();
	if(cl) {
		auto &mdlManager = cl->GetModelManager();
		mdlManager.GetLoader().SetMultiThreadingEnabled(enabled);
		auto &matManager = static_cast<msys::CMaterialManager &>(cl->GetMaterialManager());
		matManager.GetLoader().SetMultiThreadingEnabled(enabled);
		auto &texManager = matManager.GetTextureManager();
		texManager.GetLoader().SetMultiThreadingEnabled(enabled);
	}
}

extern std::optional<bool> g_launchParamWindowedMode;
extern std::optional<int> g_launchParamRefreshRate;
extern std::optional<bool> g_launchParamNoBorder;
extern std::optional<uint32_t> g_launchParamWidth;
extern std::optional<uint32_t> g_launchParamHeight;
void register_game_shaders();
bool CEngine::Initialize(int argc, char *argv[])
{
	Engine::Initialize(argc, argv);
	if(Lua::get_extended_lua_modules_enabled()) {
		RegisterConCommand("lc", [this](NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float) { RunConsoleCommand("lua_run_cl", argv); });
	}
	auto &cmds = *m_preloadedConfig.get();

	auto *cviRenderAPI = cmds.find("render_api");
	if(cviRenderAPI && cviRenderAPI->argv.empty() == false)
		SetRenderAPI(cviRenderAPI->argv.front());

	// Initialize Window context
	try {
		InitializeRenderAPI();
	}
	catch(const std::runtime_error &err) {
		spdlog::error("Unable to initialize graphics API: {}", err.what());
		std::this_thread::sleep_for(std::chrono::seconds(5));
		Close();
		return false;
	}

	auto res = cmds.find("cl_window_resolution");
	prosper::IPrContext::CreateInfo contextCreateInfo {};
	contextCreateInfo.width = 1280;
	contextCreateInfo.height = 1024;
	if(res != nullptr && !res->argv.empty()) {
		std::vector<std::string> vals;
		ustring::explode(res->argv[0], "x", vals);
		if(vals.size() >= 2) {
			contextCreateInfo.width = util::to_int(vals[0]);
			contextCreateInfo.height = util::to_int(vals[1]);
		}
	}
	// SetResolution(Vector2i(contextCreateInfo.width,contextCreateInfo.height));

	res = cmds.find("cl_render_resolution");
	if(res != nullptr && !res->argv.empty()) {
		std::vector<std::string> vals;
		ustring::explode(res->argv[0], "x", vals);
		if(vals.size() >= 2) {
			m_renderResolution = {util::to_int(vals[0]), util::to_int(vals[1])};
		}
	}
	//

	res = cmds.find("cl_render_window_mode");
	int mode = 0;
	if(res != nullptr && !res->argv.empty())
		mode = util::to_int(res->argv[0]);
	auto &initialWindowSettings = GetRenderContext().GetInitialWindowSettings();
	initialWindowSettings.windowedMode = (mode == 0);
	initialWindowSettings.decorated = ((mode == 2) ? false : true);

	if(g_launchParamWindowedMode.has_value())
		initialWindowSettings.windowedMode = *g_launchParamWindowedMode;
	if(g_launchParamRefreshRate.has_value())
		initialWindowSettings.refreshRate = *g_launchParamRefreshRate;
	if(g_launchParamNoBorder.has_value())
		initialWindowSettings.decorated = !*g_launchParamNoBorder;
	if(g_launchParamWidth.has_value())
		initialWindowSettings.width = *g_launchParamWidth;
	if(g_launchParamHeight.has_value())
		initialWindowSettings.height = *g_launchParamHeight;

	res = cmds.find("cl_render_monitor");
	if(res != nullptr && !res->argv.empty()) {
		auto monitor = util::to_int(res->argv[0]);
		auto monitors = GLFW::get_monitors();
		if(monitor < monitors.size() && monitor > 0)
			initialWindowSettings.monitor = monitors[monitor];
	}

	res = cmds.find("cl_gpu_device");
	if(res != nullptr && !res->argv.empty()) {
		auto device = res->argv[0];
		std::vector<std::string> subStrings;
		ustring::explode(device, ",", subStrings);
		if(subStrings.size() >= 2)
			contextCreateInfo.device = {static_cast<prosper::Vendor>(util::to_uint(subStrings.at(0))), util::to_uint(subStrings.at(1))};
	}

	auto presentMode = prosper::PresentModeKHR::Mailbox;
	res = cmds.find("cl_render_present_mode");
	if(res != nullptr && !res->argv.empty()) {
		auto mode = util::to_int(res->argv[0]);
		if(mode == 0)
			presentMode = prosper::PresentModeKHR::Immediate;
		else if(mode == 1)
			presentMode = prosper::PresentModeKHR::Fifo;
		else
			presentMode = prosper::PresentModeKHR::Mailbox;
	}
	contextCreateInfo.presentMode = presentMode;

	GetRenderContext().Initialize(contextCreateInfo);

	auto &shaderManager = GetRenderContext().GetShaderManager();
	shaderManager.RegisterShader("clear_color", [](prosper::IPrContext &context, const std::string &identifier) { return new pragma::ShaderClearColor(context, identifier); });
	shaderManager.RegisterShader("gradient", [](prosper::IPrContext &context, const std::string &identifier) { return new pragma::ShaderGradient(context, identifier); });

	shaderManager.GetShader("blur_horizontal");
	shaderManager.GetShader("blur_vertical");

	// Initialize Client Instance
	auto matManager = msys::CMaterialManager::Create(GetRenderContext());
	matManager->SetImportDirectory("addons/converted/materials");
	InitializeAssetManager(*matManager);
	pragma::asset::update_extension_cache(pragma::asset::Type::Material);

	auto &texManager = matManager->GetTextureManager();
	InitializeAssetManager(texManager);
	texManager.GetCallbacks().onAssetReloaded = [](const std::string &assetName) {
		// TODO: Reload all materials using this texture
		std::cout << "";
	};

	auto matErr = matManager->LoadAsset("error");
	m_clInstance = std::unique_ptr<StateInstance>(new StateInstance {matManager, matErr.get()});
	//

	LoadFontSets();
	auto &defaultFontSet = m_defaultFontSet;
	defaultFontSet = "vera";
	auto *lanInfo = Locale::GetLanguageInfo();
	if(lanInfo && lanInfo->configData) {
		std::vector<std::string> characterSetRequirements;
		(*lanInfo->configData)["font"]["characterSetRequirements"](characterSetRequirements);
		if(std::find(characterSetRequirements.begin(), characterSetRequirements.end(), "jp") != characterSetRequirements.end()) {
			std::string sourceHanSans = "source-han-sans";
			if(FindFontSet(sourceHanSans))
				defaultFontSet = sourceHanSans;
		}
	}

	auto fail = [&]() {
		matManager = nullptr;
		Close();
		Release();
		std::this_thread::sleep_for(std::chrono::seconds(5));
		return false;
	};

	if(!FindFontSet(defaultFontSet)) {
		spdlog::error("Failed to find default font set '{}'!", defaultFontSet);
		fail();
		return false;
	}

	auto &fontSet = GetDefaultFontSet();
	auto &gui = WGUI::Open(GetRenderContext(), matManager);
	gui.SetMaterialLoadHandler([this](const std::string &path) -> Material * { return GetClientState()->LoadMaterial(path); });
	auto *fontData = fontSet.FindFontFileCandidate(FontSetFlag::Sans | FontSetFlag::Bold);
	if(!fontData) {
		spdlog::error("Failed to determine default font for font set '{}'!", defaultFontSet);
		fail();
		return false;
	}
	auto r = gui.Initialize(GetRenderResolution(), fontData->fileName);
	if(r != WGUI::ResultCode::Ok) {
		Con::cerr << "Unable to initialize GUI library: ";
		switch(r) {
		case WGUI::ResultCode::UnableToInitializeFontManager:
			Con::cerr << "Error initializing font manager!";
			break;
		case WGUI::ResultCode::ErrorInitializingShaders:
			Con::cerr << "Error initializing shaders!";
			break;
		case WGUI::ResultCode::FontNotFound:
			Con::cerr << "Font not found!";
			break;
		default:
			Con::cout << "Unknown error!";
			break;
		}
		fail();
		return false;
	}
	WIContextMenu::SetKeyBindHandler(
	  [this](GLFW::Key key, const std::string &cmd) -> std::string {
		  std::string keyStr;
		  auto b = KeyToText(umath::to_integral(key), &keyStr);
		  short c;
		  if(StringToKey(keyStr, &c)) {
			  auto bindingLayer = GetCoreInputBindingLayer();
			  if(bindingLayer)
				  bindingLayer->MapKey(c, cmd);
		  }
		  SetInputBindingsDirty();
		  return keyStr;
	  },
	  [this](const std::string &cmd) -> std::optional<std::string> {
		  std::vector<GLFW::Key> keys;
		  GetMappedKeys(cmd, keys);
		  if(keys.empty())
			  return {};
		  std::string strKey {};
		  KeyToText(umath::to_integral(keys.front()), &strKey);
		  return strKey;
	  });
	WITextTagLink::set_link_handler([this](const std::string &arg) {
		std::vector<std::string> args {};
		ustring::explode_whitespace(arg, args);
		if(args.empty())
			return;
		auto cmd = args.front();
		args.erase(args.begin());
		if(ustring::compare<std::string>(cmd, "url", false)) {
			if(args.empty() == false)
				util::open_url_in_browser(args.front());
			return;
		}
		RunConsoleCommand(cmd, args);
	});

	m_speedCam = 1600.0f;
	m_speedCamMouse = 0.2f;

	GetRenderContext().GetWindow().ReloadStagingRenderTarget();
	umath::set_flag(m_stateFlags, StateFlags::FirstFrame);

	m_gpuProfiler = pragma::debug::GPUProfiler::Create<pragma::debug::GPUProfiler>();
	AddGPUProfilingHandler([this](bool profilingEnabled) {
		if(profilingEnabled == false) {
			m_gpuProfilingStageManager = nullptr;
			return;
		}
		m_gpuProfilingStageManager = std::make_unique<pragma::debug::ProfilingStageManager<pragma::debug::GPUProfilingStage, GPUProfilingPhase>>();
		auto &gpuProfiler = *m_gpuProfiler;
		const auto defaultStage = prosper::PipelineStageFlags::BottomOfPipeBit;
		auto stageFrame = pragma::debug::GPUProfilingStage::Create(gpuProfiler, "Frame", defaultStage);
		m_gpuProfilingStageManager->InitializeProfilingStageManager(gpuProfiler, {stageFrame, pragma::debug::GPUProfilingStage::Create(gpuProfiler, "DrawScene", defaultStage, stageFrame.get()), pragma::debug::GPUProfilingStage::Create(gpuProfiler, "GUI", defaultStage, stageFrame.get())});
		static_assert(umath::to_integral(GPUProfilingPhase::Count) == 3u, "Added new profiling phase, but did not create associated profiling stage!");
	});

	InitializeSoundEngine();

	OpenClientState();
	register_game_shaders(); // Preload game shaders

	if(umath::is_flag_set(m_stateFlags, StateFlags::ConsoleOpen))
		OpenConsole(); // GUI Console mustn't be opened before client has been created!

#ifdef _WIN32
	if(GetRenderContext().IsValidationEnabled()) {
		if(util::is_process_running("bdcam.exe")) {
			auto r = MessageBox(NULL, "Bandicam is running and vulkan validation mode is enabled. This is NOT recommended, as Bandicam will cause misleading validation errors! Press OK to continue anyway.", "Validation Warning", MB_OK | MB_OKCANCEL);
			if(r == IDCANCEL)
				ShutDown();
		}
	}
	else if(util::is_process_running("bdcam.exe")) {
		auto r = MessageBox(NULL, "Detected Bandicam running in the background, this can cause crashes and/or freezing! Please close Bandicam and restart Pragma. You can restart Bandicam after Pragma has been started, but it mustn't be running before then.", "Bandicam Warning",
		  MB_OK | MB_OKCANCEL);
		if(r == IDCANCEL)
			ShutDown();
	}
#endif
	return true;
}
const std::string &CEngine::GetDefaultFontSetName() const { return m_defaultFontSet; }
const FontSet &CEngine::GetDefaultFontSet() const
{
	auto *fs = FindFontSet(m_defaultFontSet);
	assert(fs != nullptr);
	return *fs;
}
const FontSet *CEngine::FindFontSet(const std::string &name) const
{
	auto it = m_fontSets.find(name);
	return (it != m_fontSets.end()) ? &it->second : nullptr;
}
void CEngine::LoadFontSets()
{
	std::vector<std::string> dirs;
	filemanager::find_files("fonts/*", nullptr, &dirs);
	for(auto &dir : dirs) {
		try {
			auto data = udm::Data::Load("fonts/" + dir + "/font.udm");
			if(data) {
				auto fontData = data->GetAssetData().GetData()[dir];
				if(fontData) {
					FontSet fontSet {};
					if(fontData["fileName"]) {
						FontSetFileData fileData {};
						fontData["fileName"](fileData.fileName);
						fileData.fileName = dir + '/' + fileData.fileName;
						fontSet.fileData.push_back(fileData);
					}
					else {
						for(auto &udmFileData : fontData["files"]) {
							FontSetFileData fileData {};
							udmFileData["fileName"](fileData.fileName);
							fileData.fileName = dir + '/' + fileData.fileName;
							udm::to_flags(udmFileData["features"], fileData.flags);
							fontSet.fileData.push_back(fileData);
						}
					}
					m_fontSets[dir] = std::move(fontSet);
				}
			}
		}
		catch(const udm::Exception &e) {
		}
	}
}
void CEngine::RunLaunchCommands()
{
	Engine::RunLaunchCommands();
	auto *cl = GetClientState();
	if(cl != nullptr)
		SetHRTFEnabled(cl->GetConVarBool("cl_audio_hrtf_enabled"));
}
void CEngine::ClearConsole()
{
	auto *pConsole = WIConsole::GetConsole();
	if(pConsole == nullptr) {
		Engine::ClearConsole();
		return;
	}
	pConsole->Clear();
}
void CEngine::OpenConsole()
{
	switch(m_consoleType) {
	case ConsoleType::Terminal:
		Engine::OpenConsole();
		break;
	default:
		{
			if(WGUI::IsOpen()) {
				auto *console = WIConsole::Open();
				if(console && m_consoleType == ConsoleType::GUIDetached) {
					console->Update();
					auto *frame = console->GetFrame();
					if(frame) {
						frame->Update();
						frame->Detach();
					}
				}
			}
			break;
		}
	}
	umath::set_flag(m_stateFlags, StateFlags::ConsoleOpen, true);
	// Engine::OpenConsole();
}
void CEngine::CloseConsole()
{
	switch(m_consoleType) {
	case ConsoleType::Terminal:
		Engine::CloseConsole();
		break;
	default:
		{
			if(WGUI::IsOpen())
				WIConsole::Close();
			break;
		}
	}

	// Engine::CloseConsole();
	umath::set_flag(m_stateFlags, StateFlags::ConsoleOpen, false);
}
void CEngine::SetConsoleType(ConsoleType type)
{
	if(type == m_consoleType)
		return;
	auto isOpen = IsConsoleOpen();
	CloseConsole();
	Engine::SetConsoleType(type);
	if(isOpen)
		OpenConsole();
}
CEngine::ConsoleType CEngine::GetConsoleType() const
{
	auto *pConsole = WIConsole::GetConsole();
	auto *pFrame = pConsole ? pConsole->GetFrame() : nullptr;
	if(pFrame && pFrame->IsVisible())
		return pFrame->IsDetached() ? ConsoleType::GUIDetached : ConsoleType::GUI;
	return Engine::GetConsoleType();
}
bool CEngine::IsConsoleOpen() const
{
	switch(m_consoleType) {
	case ConsoleType::Terminal:
		return Engine::IsConsoleOpen();
	default:
		{
			auto *pConsole = WIConsole::GetConsole();
			auto *pFrame = pConsole ? pConsole->GetFrame() : nullptr;
			return pFrame && pFrame->IsVisible();
		}
	}
	return false;
}
CallbackHandle CEngine::AddGPUProfilingHandler(const std::function<void(bool)> &handler)
{
	auto hCb = FunctionCallback<void, bool>::Create(handler);
	m_gpuProfileHandlers.push_back(hCb);
	return hCb;
}
void CEngine::SetGPUProfilingEnabled(bool bEnabled)
{
	for(auto it = m_gpuProfileHandlers.begin(); it != m_gpuProfileHandlers.end();) {
		auto &hCb = *it;
		if(hCb.IsValid() == false) {
			it = m_gpuProfileHandlers.erase(it);
			continue;
		}
		hCb(bEnabled);
		++it;
	}
}
std::shared_ptr<prosper::Window> CEngine::CreateWindow(prosper::WindowSettings &settings)
{
	auto &mainWindowCreateInfo = c_engine->GetRenderContext().GetWindow().GetWindowSettings();
	settings.flags = mainWindowCreateInfo.flags;
	settings.api = mainWindowCreateInfo.api;
	auto window = c_engine->GetRenderContext().CreateWindow(settings);
	if(!window)
		return nullptr;
	auto *pWindow = window.get();
	pWindow->GetStagingRenderTarget(); // This will initialize the staging target immediately
	(*pWindow)->SetWindowSizeCallback([pWindow](GLFW::Window &window, Vector2i size) {
		pWindow->ReloadStagingRenderTarget();
		auto *el = ::WGUI::GetInstance().GetBaseElement(pWindow);
		if(el)
			el->SetSize(size);
	});
	InitializeWindowInputCallbacks(*pWindow);
	WGUI::GetInstance().AddBaseElement(pWindow);
	return window;
}
void CEngine::InitializeWindowInputCallbacks(prosper::Window &window)
{
	window->SetKeyCallback([this, &window](GLFW::Window &glfwWindow, GLFW::Key key, int scanCode, GLFW::KeyState state, GLFW::Modifier mods) mutable { KeyboardInput(window, key, scanCode, state, mods); });
	window->SetMouseButtonCallback([this, &window](GLFW::Window &glfwWindow, GLFW::MouseButton button, GLFW::KeyState state, GLFW::Modifier mods) mutable { MouseInput(window, button, state, mods); });
	window->SetCharCallback([this, &window](GLFW::Window &glfwWindow, unsigned int c) mutable { CharInput(window, c); });
	window->SetScrollCallback([this, &window](GLFW::Window &glfwWindow, Vector2 offset) mutable { ScrollInput(window, offset); });
	window->SetFocusCallback([this, &window](GLFW::Window &glfwWindow, bool bFocused) mutable { OnWindowFocusChanged(window, bFocused); });
	window->SetDropCallback([this, &window](GLFW::Window &glfwWindow, std::vector<std::string> &files) mutable { OnFilesDropped(window, files); });
}
DLLCLIENT std::optional<std::string> g_customWindowIcon {};
void CEngine::OnWindowInitialized()
{
	pragma::RenderContext::OnWindowInitialized();
	auto &window = GetRenderContext().GetWindow();
	InitializeWindowInputCallbacks(window);

	if(g_customWindowIcon.has_value()) {
		auto imgBuf = uimg::load_image(*g_customWindowIcon, uimg::PixelFormat::LDR);
		if(imgBuf) {
			imgBuf->ToLDRFormat(uimg::Format::RGBA32);
			window->SetWindowIcon(imgBuf->GetWidth(), imgBuf->GetHeight(), static_cast<uint8_t *>(imgBuf->GetData()));
			GLFW::poll_events();
		}
	}
}
void CEngine::InitializeExternalArchiveManager() { util::initialize_external_archive_manager(GetClientState()); }
bool CEngine::StartProfilingStage(CPUProfilingPhase stage) { return m_cpuProfilingStageManager && m_cpuProfilingStageManager->StartProfilerStage(stage); }
bool CEngine::StopProfilingStage(CPUProfilingPhase stage) { return m_cpuProfilingStageManager && m_cpuProfilingStageManager->StopProfilerStage(stage); }
bool CEngine::StartProfilingStage(GPUProfilingPhase stage) { return m_gpuProfilingStageManager && m_gpuProfilingStageManager->StartProfilerStage(stage); }
bool CEngine::StopProfilingStage(GPUProfilingPhase stage) { return m_gpuProfilingStageManager && m_gpuProfilingStageManager->StopProfilerStage(stage); }
bool CEngine::GetControllersEnabled() const { return umath::is_flag_set(m_stateFlags, StateFlags::ControllersEnabled); }
void CEngine::SetControllersEnabled(bool b)
{
	if(GetControllersEnabled() == b)
		return;
	umath::set_flag(m_stateFlags, StateFlags::ControllersEnabled, b);
	if(b == false) {
		GLFW::set_joysticks_enabled(false);
		return;
	}
	GLFW::set_joysticks_enabled(true);
	GLFW::set_joystick_axis_threshold(0.01f);
	GLFW::set_joystick_button_callback([this](const GLFW::Joystick &joystick, uint32_t key, GLFW::KeyState oldState, GLFW::KeyState newState) {
		auto keyOffset = GLFW_CUSTOM_KEY_JOYSTICK_0_KEY_START + joystick.GetJoystickId() * GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT;
		JoystickButtonInput(GetWindow(), joystick, key + keyOffset, newState);
	});
	GLFW::set_joystick_axis_callback([this](const GLFW::Joystick &joystick, uint32_t axisId, float oldVal, float newVal) {
		m_rawInputJoystickMagnitude = newVal;
		auto mods = GLFW::Modifier::None;
		auto axisOffset = GLFW_CUSTOM_KEY_JOYSTICK_0_AXIS_START + joystick.GetJoystickId() * GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT;
		if(SEPARATE_JOYSTICK_AXES == true) {
			axisId *= 2;
			if(umath::sign(oldVal) != umath::sign(newVal)) {
				auto prevAxisId = axisId;
				auto prevMods = mods;
				if(oldVal < 0.f) {
					++prevAxisId;
					prevMods |= GLFW::Modifier::AxisNegative;
				}
				JoystickAxisInput(GetWindow(), joystick, prevAxisId + axisOffset, prevMods, 0.f, 0.f - oldVal);
				oldVal = 0.f;
			}
			if(newVal < 0.f) {
				oldVal = -oldVal;
				newVal = -newVal;
				++axisId;
				mods |= GLFW::Modifier::AxisNegative;
			}
		}
		JoystickAxisInput(GetWindow(), joystick, axisId + axisOffset, mods, newVal, newVal - oldVal);
	});
	GLFW::set_joystick_state_callback([this](const GLFW::Joystick &joystick, bool bConnected) { c_engine->CallCallbacks<void, std::reference_wrapper<const GLFW::Joystick>, bool>("OnJoystickStateChanged", std::ref(joystick), bConnected); });
}
REGISTER_CONVAR_CALLBACK_CL(cl_controller_enabled, [](NetworkState *state, ConVar *cv, bool oldVal, bool newVal) { c_engine->SetControllersEnabled(newVal); });

float CEngine::GetRawJoystickAxisMagnitude() const { return m_rawInputJoystickMagnitude; }

Engine::StateInstance &CEngine::GetClientStateInstance() { return *m_clInstance; }

::util::WeakHandle<prosper::Shader> CEngine::ReloadShader(const std::string &name)
{
#ifdef _DEBUG
	bReload = true;
#endif
	/*	Con::cerr<<"Loading shader "<<name<<"..."<<Con::endl;
#ifndef _DEBUG
#error ""
#endif*/
	GetRenderContext().WaitIdle();
	auto whShader = GetRenderContext().GetShader(name);
	if(whShader.expired()) {
		spdlog::warn("No shader found with name '{}'!", name);
		return {};
	}
	spdlog::info("Reloading shader {}...", name);
	whShader.get()->Initialize(true);
	auto nummPipelines = whShader->GetPipelineCount();
	for(auto i = decltype(nummPipelines) {0u}; i < nummPipelines; ++i)
		whShader->GetPipelineInfo(i); // Force immediate reload
	return whShader;
}
void CEngine::ReloadShaderPipelines()
{
	GetRenderContext().WaitIdle();
	spdlog::info("Reloading shaders");
	auto &shaderManager = GetRenderContext().GetShaderManager();
	auto &shaders = shaderManager.GetShaders();
	for(auto &shader : shaders) {
		if(shader == nullptr)
			continue;
		shader->Initialize(true);
	}
}

CEngine::~CEngine() { m_audioAPILib = nullptr; }

CEngine *pragma::get_cengine() { return c_engine; }
ClientState *pragma::get_client_state() { return client; }

void CEngine::HandleLocalHostPlayerClientPacket(NetPacket &p)
{
	auto *client = GetClientState();
	if(client == nullptr)
		return;
	auto *cl = static_cast<ClientState *>(client)->GetClient();
	if(cl == nullptr)
		return;
	cl->HandlePacket(p);
}

void CEngine::Connect(const std::string &ip, const std::string &port)
{
	auto *cl = static_cast<ClientState *>(GetClientState());
	if(cl == NULL)
		return;
	cl->Disconnect();
	if(ip != "localhost")
		c_engine->CloseServerState();
	else {
		auto steamId = c_engine->GetServerSteamId();
		if(steamId.has_value()) {
			// Listen server is peer-to-peer; Connect via steam ID
			cl->Connect(*steamId);
			return;
		}
	}
	cl->Connect(ip, port);
}

void CEngine::Connect(uint64_t steamId)
{
	auto *cl = static_cast<ClientState *>(GetClientState());
	if(cl == NULL)
		return;
	cl->Disconnect();
	c_engine->CloseServerState();
	cl->Connect(steamId);
}

void CEngine::Disconnect()
{
	auto *cl = static_cast<ClientState *>(GetClientState());
	if(cl == nullptr)
		return;
	if(cl->IsGameActive()) {
		cl->Disconnect();
		OpenServerState();
	}
	cl->OpenMainMenu();
}

bool CEngine::IsMultiPlayer() const
{
	if(Engine::IsMultiPlayer())
		return true;
	auto *clState = static_cast<ClientState *>(GetClientState());
	if(clState == nullptr)
		return false;
	auto *cl = clState ? clState->GetClient() : nullptr;
	return cl && typeid(*cl) != typeid(pragma::networking::LocalClient);
}

void CEngine::StartDefaultGame(const std::string &map, bool singlePlayer)
{
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::get_domain().BeginTask("Load map " + map);
#endif
	EndGame();
	StartNewGame(map.c_str(), singlePlayer);
	Connect("localhost");
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::get_domain().EndTask();
#endif
}

void CEngine::StartDefaultGame(const std::string &map) { StartDefaultGame(map, true); }

Lua::Interface *CEngine::GetLuaInterface(lua_State *l)
{
	auto *cl = static_cast<ClientState *>(GetClientState());
	if(cl != nullptr) {
		if(cl->GetGUILuaState() == l)
			return &cl->GetGUILuaInterface();
		auto *cg = cl->GetGameState();
		if(cg != nullptr && cg->GetLuaState() == l)
			return &cg->GetLuaInterface();
	}
	return Engine::GetLuaInterface(l);
}

NetworkState *CEngine::GetNetworkState(lua_State *l)
{
	auto *cl = static_cast<ClientState *>(GetClientState());
	if(cl == NULL)
		return NULL;
	if(cl->GetLuaState() == l || cl->GetGUILuaState() == l)
		return cl;
	return Engine::GetNetworkState(l);
}

void CEngine::Start() { Engine::Start(); }

void CEngine::Close()
{
	if(umath::is_flag_set(m_stateFlags, StateFlags::CEClosed))
		return;
	umath::set_flag(m_stateFlags, StateFlags::CEClosed);

	auto closeSecondaryWindows = [this]() {
		auto &renderContext = GetRenderContext();
		auto &primWindow = renderContext.GetWindow();
		auto &windows = GetRenderContext().GetWindows();
		// Close all windows that aren't the primary window
		for(auto &w : windows) {
			if(w.get() == &primWindow)
				continue;
			w->Close();
		}
		for(auto it = windows.begin(); it != windows.end();) {
			auto &w = *it;
			if(w.get() == &primWindow) {
				++it;
				continue;
			}
			it = windows.erase(it);
		}
	};
	closeSecondaryWindows();

	CloseClientState();
	m_auxEffects.clear();
	CloseSoundEngine(); // Has to be closed after client state (since clientstate may still have some references at this point)
	m_clInstance = nullptr;
	WGUI::Close(); // Has to be closed after client state
	c_engine = nullptr;
	pragma::RenderContext::Release();

	Engine::Close();
}

void CEngine::OnClose()
{
	pragma::RenderContext::OnClose();
	// Clear all Vulkan resources before closing the context
	m_gpuProfiler = {};

	pragma::CRenderComponent::ClearBuffers();
	pragma::CLightComponent::ClearBuffers();
	CModelSubMesh::ClearBuffers();
	pragma::CParticleSystemComponent::ClearBuffers();
}

static auto cvFpsDecayFactor = GetClientConVar("cl_fps_decay_factor");
void CEngine::UpdateFPS(float t)
{
	auto weightRatio = cvFpsDecayFactor->GetFloat();
	m_tFPSTime = t * (1.0 - weightRatio) + m_tFPSTime * weightRatio;
	if(m_tFPSTime > 0.0)
		m_fps = 1.0 / m_tFPSTime;
}

static CVar cvProfiling = GetEngineConVar("debug_profiling_enabled");
void CEngine::DrawFrame()
{
	auto primWindowCmd = GetWindow().GetDrawCommandBuffer();
	auto perfTimers = umath::is_flag_set(m_stateFlags, StateFlags::EnableGpuPerformanceTimers);
	if(perfTimers) {
		auto n = umath::to_integral(GPUTimer::Count);
		for(auto i = decltype(n) {0u}; i < n; ++i) {
			auto idx = GetPerformanceTimerIndex(static_cast<GPUTimer>(i));
			m_gpuTimers[idx]->QueryResult(m_gpuExecTimes[idx]);
		}

		auto idx = GetPerformanceTimerIndex(GPUTimer::Frame);
		m_gpuTimers[idx]->Begin(*primWindowCmd);
	}
	m_gpuProfiler->Reset();
	StartProfilingStage(GPUProfilingPhase::Frame);

	auto ptrDrawCmd = std::dynamic_pointer_cast<prosper::IPrimaryCommandBuffer>(primWindowCmd);
	CallCallbacks<void, std::reference_wrapper<std::shared_ptr<prosper::IPrimaryCommandBuffer>>>("DrawFrame", std::ref(ptrDrawCmd));

#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::get_domain().BeginTask("poll_material_manager");
#endif
	static_cast<msys::CMaterialManager &>(*m_clInstance->materialManager).Poll(); // Requires active command buffer
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::get_domain().EndTask();
#endif

	StartProfilingStage(CPUProfilingPhase::GUI);
	auto &gui = WGUI::GetInstance();
	gui.Think();
	StopProfilingStage(CPUProfilingPhase::GUI);

	auto &stagingRt = GetRenderContext().GetWindow().GetStagingRenderTarget();
	if(umath::is_flag_set(m_stateFlags, StateFlags::FirstFrame))
		umath::set_flag(m_stateFlags, StateFlags::FirstFrame, false);
	else {
		primWindowCmd->RecordImageBarrier(stagingRt->GetTexture().GetImage(), prosper::ImageLayout::TransferSrcOptimal, prosper::ImageLayout::ColorAttachmentOptimal);
	}

	DrawScene(stagingRt);

	if(perfTimers) {
		auto idx = GetPerformanceTimerIndex(GPUTimer::Present);
		m_gpuTimers[idx]->Begin(*primWindowCmd);
	}
	for(auto &window : GetRenderContext().GetWindows()) {
		if(window->IsValid() == false || window->GetState() != prosper::Window::State::Active)
			continue;
		auto &finalImg = window->GetStagingRenderTarget()->GetTexture().GetImage();
		primWindowCmd->RecordImageBarrier(finalImg, prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::TransferSrcOptimal);

		primWindowCmd->RecordPresentImage(finalImg, *window);
	}

	if(perfTimers) {
		auto idx = GetPerformanceTimerIndex(GPUTimer::Present);
		m_gpuTimers[idx]->End(*primWindowCmd);
	}

	StopProfilingStage(GPUProfilingPhase::Frame);
	if(perfTimers) {
		auto idx = GetPerformanceTimerIndex(GPUTimer::Frame);
		m_gpuTimers[idx]->End(*primWindowCmd);
	}
}

static auto cvHideGui = GetClientConVar("debug_hide_gui");
void CEngine::DrawScene(std::shared_ptr<prosper::RenderTarget> &rt)
{
	auto perfTimers = umath::is_flag_set(m_stateFlags, StateFlags::EnableGpuPerformanceTimers);
	auto drawGui = !cvHideGui->GetBool();
	if(drawGui) {
		auto &rp = rt->GetRenderPass();
		auto &fb = rt->GetFramebuffer();
		StartProfilingStage(GPUProfilingPhase::GUI);

		WGUI::GetInstance().SetLockedForDrawing(true);
		auto &context = GetRenderContext();
		for(auto &window : context.GetWindows()) {
			if(!window || window->IsValid() == false || window->GetState() != prosper::Window::State::Active)
				continue;
			auto &windowRt = window->GetStagingRenderTarget();
			auto &swapCmdGroup = window->GetSwapCommandBufferGroup();
			swapCmdGroup.StartRecording(windowRt->GetRenderPass(), windowRt->GetFramebuffer());
			swapCmdGroup.Record([window](prosper::ISecondaryCommandBuffer &drawCmd) {
				auto &gui = WGUI::GetInstance();
				gui.Draw(*window, drawCmd);
			});
			swapCmdGroup.EndRecording();
		}

		StopProfilingStage(GPUProfilingPhase::GUI);
	}

	auto *cl = static_cast<ClientState *>(GetClientState());
	auto tStart = util::Clock::now();
	if(cl != nullptr) {
		StartProfilingStage(GPUProfilingPhase::DrawScene);

		auto &window = GetWindow();
		auto &drawCmd = window.GetDrawCommandBuffer();
		if(perfTimers) {
			auto idx = GetPerformanceTimerIndex(GPUTimer::Scene);
			m_gpuTimers[idx]->Begin(*drawCmd);
		}
		util::DrawSceneInfo drawSceneInfo {};
		drawSceneInfo.commandBuffer = drawCmd;
		cl->Render(drawSceneInfo, rt);
		if(perfTimers) {
			auto idx = GetPerformanceTimerIndex(GPUTimer::Scene);
			m_gpuTimers[idx]->End(*drawCmd);
		}

		StopProfilingStage(GPUProfilingPhase::DrawScene);
	}

	if(drawGui) {
		CallCallbacks<void>("PreDrawGUI");
		if(c_game != nullptr)
			c_game->PreGUIDraw();

		auto &primWindowCmd = GetWindow().GetDrawCommandBuffer();
		if(perfTimers) {
			auto idx = GetPerformanceTimerIndex(GPUTimer::GUI);
			m_gpuTimers[idx]->Begin(*primWindowCmd);
		}
		for(auto &window : GetRenderContext().GetWindows()) {
			if(!window || !window->IsValid() || window->GetState() != prosper::Window::State::Active)
				continue;
			auto &rt = window->GetStagingRenderTarget();
			auto &drawCmd = window->GetDrawCommandBuffer();

			static std::vector<prosper::ClearValue> clearVals = {prosper::ClearValue {}, prosper::ClearValue {prosper::ClearDepthStencilValue {0.f, 0}}};

			drawCmd->RecordBeginRenderPass(*rt, clearVals, prosper::IPrimaryCommandBuffer::RenderPassFlags::SecondaryCommandBuffers);
			window->GetSwapCommandBufferGroup().ExecuteCommands(*drawCmd);
			drawCmd->RecordEndRenderPass();
		}
		if(perfTimers) {
			auto idx = GetPerformanceTimerIndex(GPUTimer::GUI);
			m_gpuTimers[idx]->End(*primWindowCmd);
		}
		WGUI::GetInstance().SetLockedForDrawing(false);
		CallCallbacks<void>("PostDrawGUI");

		if(c_game != nullptr)
			c_game->PostGUIDraw();
	}
}

uint32_t CEngine::GetPerformanceTimerIndex(uint32_t swapchainIdx, GPUTimer timer) const { return swapchainIdx * umath::to_integral(GPUTimer::Count) + umath::to_integral(timer); }
uint32_t CEngine::GetPerformanceTimerIndex(GPUTimer timer) const { return GetPerformanceTimerIndex(GetRenderContext().GetLastAcquiredPrimaryWindowSwapchainImageIndex(), timer); }

void CEngine::SetGpuPerformanceTimersEnabled(bool enabled)
{
	if(umath::is_flag_set(m_stateFlags, StateFlags::EnableGpuPerformanceTimers) == enabled)
		return;
	umath::set_flag(m_stateFlags, StateFlags::EnableGpuPerformanceTimers, enabled);

	if(enabled == false) {
		c_engine->GetRenderContext().KeepResourceAliveUntilPresentationComplete(m_gpuTimerPool);
		m_gpuTimerPool = nullptr;
		for(auto &t : m_gpuTimers) {
			c_engine->GetRenderContext().KeepResourceAliveUntilPresentationComplete(t);
			t = nullptr;
		}
		for(auto &t : m_gpuExecTimes)
			t = std::chrono::nanoseconds {0};
		return;
	}
	auto &context = GetRenderContext();
	auto numSwapchainImages = context.GetPrimaryWindowSwapchainImageCount();
	auto numTimers = umath::to_integral(GPUTimer::Count) * numSwapchainImages;
	m_gpuTimerPool = context.CreateQueryPool(prosper::QueryType::Timestamp, numTimers * 2);
	m_gpuTimers.resize(numTimers);
	m_gpuExecTimes.resize(numTimers);
	for(auto &t : m_gpuTimers)
		t = m_gpuTimerPool->CreateTimerQuery(prosper::PipelineStageFlags::TopOfPipeBit, prosper::PipelineStageFlags::BottomOfPipeBit);
}
std::chrono::nanoseconds CEngine::GetGpuExecutionTime(uint32_t swapchainIdx, GPUTimer timer) const
{
	if(umath::is_flag_set(m_stateFlags, StateFlags::EnableGpuPerformanceTimers) == false)
		return std::chrono::nanoseconds {0};
	return m_gpuExecTimes[GetPerformanceTimerIndex(swapchainIdx, timer)];
}

#include <prosper_util.hpp>
#include <buffers/prosper_buffer.hpp>
#include <buffers/prosper_dynamic_resizable_buffer.hpp>
void CEngine::Think()
{
	GLFW::poll_joystick_events();

	auto tNow = util::Clock::now();

	std::chrono::nanoseconds tDelta;
	if(m_fixedFrameDeltaTimeInterpretation.has_value() == false)
		tDelta = tNow - m_tLastFrame;
	else
		tDelta = *m_fixedFrameDeltaTimeInterpretation;
	auto maxFps = GetFPSLimit();
	if(maxFps >= 0.f && (maxFps == 0.f || std::chrono::duration_cast<std::chrono::nanoseconds>(tDelta).count() / 1'000'000.0 < 1'000.0 / maxFps)) {
		if(maxFps < 1.f) {
			// FPS < 1 are a special case, used for testing only.
			// Since it may take a while before the next frame is executed,
			// we need to process console input here, otherwise we may have
			// to wait a long time before our console inputs are
			// executed (indefinitely if max fps is 0).
			ProcessConsoleInput();
		}
		return;
	}
	m_tDeltaFrameTime = tDelta;

	m_tLastFrame = tNow;
	const auto sToNs = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(1)).count());
	UpdateFPS(static_cast<float>(std::chrono::duration_cast<std::chrono::nanoseconds>(m_tDeltaFrameTime).count() / sToNs));
	//auto tStart = std::chrono::high_resolution_clock::now();

	Engine::Think();

	auto *cl = GetClientState();
	if(cl != NULL)
		cl->Think(); // Draw?

	StartProfilingStage(CPUProfilingPhase::DrawFrame);

	pragma::RenderContext::DrawFrame();
	CallCallbacks("Draw");
	StopProfilingStage(CPUProfilingPhase::DrawFrame);
	GLFW::poll_events(); // Needs to be called AFTER rendering!
	auto &windows = GetRenderContext().GetWindows();
	for(auto it = windows.begin(); it != windows.end();) {
		auto &window = *it;
		if(window.use_count() == 1) {
			window->Close();
			it = windows.erase(it);
			continue;
		}
		if(!window->IsValid()) {
			++it;
			continue;
		}
		if((*window)->ShouldClose() == false) {
			window->UpdateWindow();
			++it;
			continue;
		}
		if(window.get() == &GetRenderContext().GetWindow()) {
			ShutDown();
			return;
		}
		window->Close();
		++it;
	}
	GetRenderContext().CloseWindowsScheduledForClosing();
}

void CEngine::SetFixedFrameDeltaTimeInterpretation(std::optional<std::chrono::nanoseconds> frameDeltaTime) { m_fixedFrameDeltaTimeInterpretation = frameDeltaTime; }
void CEngine::SetFixedFrameDeltaTimeInterpretationByFPS(uint16_t fps) { SetFixedFrameDeltaTimeInterpretation(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds {1}) / fps); }
void CEngine::SetTickDeltaTimeTiedToFrameRate(bool tieToFrameRate) { umath::set_flag(m_stateFlags, StateFlags::TickDeltaTimeTiedToFrameRate, tieToFrameRate); }

void CEngine::UpdateTickCount()
{
	if(umath::is_flag_set(m_stateFlags, StateFlags::TickDeltaTimeTiedToFrameRate) == false) {
		Engine::UpdateTickCount();
		return;
	}
	m_ctTick.UpdateByDelta(util::clock::to_seconds(m_tDeltaFrameTime));
}

void CEngine::Tick()
{
	Locale::Poll();
	ProcessConsoleInput();
	RunTickEvents();

	Engine::StartProfilingStage(Engine::CPUProfilingPhase::Tick);
	// The client tick has to run BEFORE the server tick!!!
	// This is to avoid issues in singleplayer, where the client would use data it received from the server and apply the same calculations on the already modified data.
	StartProfilingStage(CPUProfilingPhase::ClientTick);
	auto *cl = GetClientState();
	if(cl != NULL)
		cl->Tick();
	StopProfilingStage(CPUProfilingPhase::ClientTick);

	Engine::StartProfilingStage(Engine::CPUProfilingPhase::ServerTick);
	auto *sv = GetServerNetworkState();
	if(sv != NULL)
		sv->Tick();
	Engine::StopProfilingStage(Engine::CPUProfilingPhase::ServerTick);
	Engine::StopProfilingStage(Engine::CPUProfilingPhase::Tick);

	UpdateParallelJobs();
}

bool CEngine::IsServerOnly() { return false; }

void CEngine::UseFullbrightShader(bool b) { umath::set_flag(m_stateFlags, StateFlags::Fullbright, b); }

void CEngine::OnResolutionChanged(uint32_t width, uint32_t height)
{
	RenderContext::OnResolutionChanged(width, height);
	if(m_renderResolution.has_value() == false)
		OnRenderResolutionChanged(width, height);
}

void CEngine::OnRenderResolutionChanged(uint32_t width, uint32_t height)
{
	GetRenderContext().GetWindow().ReloadStagingRenderTarget();

	auto &wgui = WGUI::GetInstance();
	auto *baseEl = wgui.GetBaseElement();
	if(baseEl != nullptr)
		baseEl->SetSize(width, height);

	auto *cl = GetClientState();
	if(cl == nullptr)
		return;
	auto *game = static_cast<CGame *>(cl->GetGameState());
	if(game == nullptr)
		return;
	game->Resize(true);
}

uint32_t CEngine::DoClearUnusedAssets(pragma::asset::Type type) const
{
	if(type == pragma::asset::Type::Texture || type == pragma::asset::Type::Material || type == pragma::asset::Type::Model)
		const_cast<CEngine *>(this)->GetRenderContext().WaitIdle();
	auto n = Engine::DoClearUnusedAssets(type);
	switch(type) {
	case pragma::asset::Type::Texture:
		{
			auto *cl = GetClientState();
			if(cl) {
				auto &texManager = static_cast<msys::CMaterialManager &>(cl->GetMaterialManager()).GetTextureManager();
				if(!IsVerbose())
					n += texManager.ClearUnused();
				else {
					auto &cache = texManager.GetCache();

					std::unordered_map<Texture *, std::string> oldCache;
					for(auto &pair : cache) {
						auto asset = texManager.GetAsset(pair.second);
						if(!asset)
							continue;
						auto tex = msys::TextureManager::GetAssetObject(*asset);
						oldCache[tex.get()] = tex->GetName();
					}

					n += texManager.ClearUnused();

					std::unordered_map<Texture *, std::string> newCache;
					for(auto &pair : cache) {
						auto asset = texManager.GetAsset(pair.second);
						if(!asset)
							continue;
						auto tex = msys::TextureManager::GetAssetObject(*asset);
						newCache[tex.get()] = tex->GetName();
					}

					for(auto &pair : oldCache) {
						auto it = newCache.find(pair.first);
						if(it != newCache.end())
							continue;
						spdlog::info("Texture {} was cleared from cache!", pair.second);
					}
				}
			}
			break;
		}
	}
	return n;
}

REGISTER_CONVAR_CALLBACK_CL(cl_render_monitor, [](NetworkState *, ConVar *, int32_t, int32_t monitor) {
	auto monitors = GLFW::get_monitors();
	if(monitor < monitors.size() && monitor >= 0)
		c_engine->GetWindow().SetMonitor(monitors[monitor]);
})

REGISTER_CONVAR_CALLBACK_CL(cl_render_window_mode, [](NetworkState *, ConVar *, int32_t, int32_t val) {
	c_engine->GetWindow().SetWindowedMode(val != 0);
	c_engine->GetWindow().SetNoBorder(val == 2);
})

REGISTER_CONVAR_CALLBACK_CL(cl_window_resolution, [](NetworkState *, ConVar *, std::string, std::string val) {
	std::vector<std::string> vals;
	ustring::explode(val, "x", vals);
	if(vals.size() < 2)
		return;
	auto x = util::to_int(vals[0]);
	auto y = util::to_int(vals[1]);
	Vector2i resolution(x, y);
	c_engine->GetWindow().SetResolution(resolution);
	auto *client = static_cast<ClientState *>(c_engine->GetClientState());
	if(client == nullptr)
		return;
	auto &wgui = WGUI::GetInstance();
	auto *el = wgui.GetBaseElement();
	if(el == nullptr)
		return;
	el->SetSize(resolution);
	auto *menu = client->GetMainMenu();
	if(menu == nullptr)
		return;
	menu->SetSize(x, y);
})

REGISTER_CONVAR_CALLBACK_CL(cl_render_resolution, [](NetworkState *, ConVar *, std::string, std::string val) {
	std::vector<std::string> vals;
	ustring::explode(val, "x", vals);
	if(vals.size() < 2) {
		c_engine->SetRenderResolution({});
		return;
	}
	auto x = util::to_int(vals[0]);
	auto y = util::to_int(vals[1]);
	Vector2i resolution(x, y);
	c_engine->SetRenderResolution(resolution);
})

REGISTER_CONVAR_CALLBACK_CL(cl_gpu_timer_queries_enabled, [](NetworkState *, ConVar *, bool, bool enabled) {
	if(c_engine == nullptr)
		return;
	c_engine->SetGPUProfilingEnabled(enabled);
})
