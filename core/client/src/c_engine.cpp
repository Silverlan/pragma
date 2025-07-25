// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifdef _MSC_VER
namespace pragma::string {
	class Utf8String;
	class Utf8StringView;
	class Utf8StringArg;
};
#endif
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
#include "pragma/debug/debug_utils.hpp"
#include "pragma/model/c_modelmanager.h"
#include "pragma/networking/iclient.hpp"
#include "pragma/networking/local_client.hpp"
#include "pragma/rendering/c_sci_gpu_timer_manager.hpp"
#include <pragma/rendering/scene/util_draw_scene_info.hpp>
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/shader_graph/manager.hpp"
#include <pragma/entities/environment/lights/c_env_light.h>
#include <pragma/input/input_binding_layer.hpp>
#include <pragma/lua/lua_error_handling.hpp>
#include <cmaterialmanager.h>
#include <cmaterial_manager2.hpp>
#include <pragma/model/modelmanager.h>
#include <pragma/model/c_modelmesh.h>
#include <cctype>
#include <sharedutils/util_debug.h>
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
#include <pragma/logging.hpp>
#include <wgui/types/wicontextmenu.hpp>
#include <wgui/types/witext.h>
#include <wgui/types/witext_tags.hpp>
#include <wgui/types/wiroot.h>
#include <util_image.hpp>
#include <util_image_buffer.hpp>
#include <queries/prosper_query_pool.hpp>
#include <queries/prosper_timer_query.hpp>
#include <pragma/asset/util_asset.hpp>
#include <prosper_window.hpp>
#include <fsys/ifile.hpp>
#include <pragma/util/font_set.hpp>
#ifdef _WIN32

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

#endif

import util_zip;
import pragma.shadergraph;
import pragma.locale;
#ifndef _MSC_VER
import pragma.string.unicode;
#endif

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

#include "pragma/rendering/shader_graph/nodes/scene_output.hpp"
#include "pragma/rendering/shader_graph/nodes/shader_material.hpp"
#include "pragma/rendering/shader_graph/nodes/camera.hpp"
#include "pragma/rendering/shader_graph/nodes/fog.hpp"
#include "pragma/rendering/shader_graph/nodes/lightmap.hpp"
#include "pragma/rendering/shader_graph/nodes/object.hpp"
#include "pragma/rendering/shader_graph/nodes/time.hpp"
#include "pragma/rendering/shader_graph/nodes/pbr.hpp"
#include "pragma/rendering/shader_graph/nodes/toon.hpp"
#include "pragma/rendering/shader_graph/nodes/image_texture.hpp"
#include "pragma/rendering/shader_graph/nodes/texture_coordinate.hpp"
#include "pragma/rendering/shader_graph/nodes/vector_transform.hpp"
#include "pragma/rendering/shader_graph/nodes/geometry.hpp"
#include "pragma/rendering/shader_graph/nodes/material_texture.hpp"
#include "pragma/rendering/shader_graph/nodes/input_parameter.hpp"
#include "pragma/rendering/shader_graph/modules/pbr.hpp"
#include "pragma/rendering/shader_graph/modules/toon.hpp"
#include "pragma/rendering/shader_graph/modules/image_texture.hpp"
#include "pragma/rendering/shader_graph/modules/input_data.hpp"

CEngine::CEngine(int argc, char *argv[])
    : Engine(argc, argv), pragma::RenderContext(), m_nearZ(pragma::BaseEnvCameraComponent::DEFAULT_NEAR_Z), //10.0f), //0.1f
      m_farZ(pragma::BaseEnvCameraComponent::DEFAULT_FAR_Z), m_fps(0), m_tFPSTime(0.f), m_tLastFrame(util::Clock::now()), m_tDeltaFrameTime(0), m_audioAPI {"fmod"}
{
	c_engine = this;
	RegisterCallback<void, std::reference_wrapper<const pragma::platform::Joystick>, bool>("OnJoystickStateChanged");
	RegisterCallback<void, std::reference_wrapper<std::shared_ptr<prosper::IPrimaryCommandBuffer>>>("DrawFrame");
	RegisterCallback<void>("PreDrawGUI");
	RegisterCallback<void>("PostDrawGUI");
	RegisterCallback<void>("PreRecordGUI");
	RegisterCallback<void>("PostRecordGUI");
	RegisterCallback<void>("Draw");

	RegisterCallbackWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, pragma::platform::MouseButton, pragma::platform::KeyState, pragma::platform::Modifier>("OnMouseInput");
	RegisterCallbackWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, pragma::platform::Key, int, pragma::platform::KeyState, pragma::platform::Modifier, float>("OnKeyboardInput");
	RegisterCallbackWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, unsigned int>("OnCharInput");
	RegisterCallbackWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, Vector2>("OnScrollInput");
	RegisterCallbackWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, std::reference_wrapper<const pragma::platform::Joystick>, uint32_t, pragma::platform::KeyState>("OnJoystickButtonInput");
	RegisterCallbackWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, std::reference_wrapper<const pragma::platform::Joystick>, uint32_t, pragma::platform::Modifier, float, float>("OnJoystickAxisInput");

	AddProfilingHandler([this](bool profilingEnabled) {
		if(profilingEnabled == false) {
			m_profilingStageManager = nullptr;
			return;
		}
		auto &cpuProfiler = c_engine->GetProfiler();
		m_cpuProfilingStageManager = std::make_unique<pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage>>();
		m_cpuProfilingStageManager->InitializeProfilingStageManager(cpuProfiler);
	});
	m_coreInputBindingLayer = std::make_shared<CoreInputBindingLayer>();
	auto inputLayer = std::make_shared<InputBindingLayer>();
	inputLayer->identifier = "core";
	AddInputBindingLayer(inputLayer);

	{
		pragma::asset::AssetManager::ImporterInfo importerInfo {};
		importerInfo.name = "glTF";
		importerInfo.fileExtensions = {{"gltf", false}, {"glb", true}, {"vrm", true}}; // VRM is based on glTF ( https://vrm.dev/en/ )
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

	{
		pragma::asset::AssetManager::ImporterInfo importerInfo {};
		importerInfo.name = "fbx";
		importerInfo.fileExtensions = {{"fbx", true}};
		GetAssetManager().RegisterImporter(importerInfo, pragma::asset::Type::Model, [](Game &game, ufile::IFile &f, const std::optional<std::string> &mdlPath, std::string &errMsg) -> std::unique_ptr<pragma::asset::IAssetWrapper> {
			util::Path path {};
			if(mdlPath.has_value()) {
				path = util::Path::CreateFile(*mdlPath);
				path.PopBack();
			}
			auto result = pragma::asset::import_fbx(f, errMsg, path);
			if(!result)
				return nullptr;
			if(result->modelObjects.empty()) {
				errMsg = "No models found in model file!";
				return nullptr;
			}

			auto wrapper = std::make_unique<pragma::asset::ModelAssetWrapper>();
			wrapper->SetModel(*result->modelObjects.front());
			return wrapper;
		});
	}
}

void CEngine::Release()
{
	Close();
	Engine::Release();
	pragma::RenderContext::Release();
}

pragma::debug::GPUProfiler &CEngine::GetGPUProfiler() const { return *m_gpuProfiler; }
pragma::debug::ProfilingStageManager<pragma::debug::GPUProfilingStage> *CEngine::GetGPUProfilingStageManager() { return m_gpuProfilingStageManager.get(); }
pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage> *CEngine::GetProfilingStageManager() { return m_cpuProfilingStageManager.get(); }

static auto cvGPUProfiling = GetClientConVar("cl_gpu_timer_queries_enabled");
bool CEngine::IsGPUProfilingEnabled() const { return cvGPUProfiling->GetBool(); }

void CEngine::DumpDebugInformation(uzip::ZIPFile &zip) const
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

	std::stringstream engineInfo;
	engineInfo << "Render API: " << GetRenderAPI();
	zip.AddFile("engine_cl.txt", engineInfo.str());

	auto &context = c_engine->GetRenderContext();
	auto layers = context.DumpLayers();
	if(layers)
		zip.AddFile("prosper_layers.txt", *layers);

	auto extensions = context.DumpExtensions();
	if(extensions)
		zip.AddFile("prosper_extensions.txt", *extensions);

	auto limits = context.DumpLimits();
	if(limits)
		zip.AddFile("prosper_limits.txt", *limits);

	auto features = context.DumpFeatures();
	if(features)
		zip.AddFile("prosper_features.txt", *features);

	auto imageFormatProperties = context.DumpImageFormatProperties();
	if(imageFormatProperties)
		zip.AddFile("prosper_image_format_properties.txt", *imageFormatProperties);

	auto formatProperties = context.DumpFormatProperties();
	if(formatProperties)
		zip.AddFile("prosper_format_properties.txt", *formatProperties);
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

void CEngine::Input(int key, pragma::platform::KeyState inputState, pragma::platform::KeyState pressState, pragma::platform::Modifier mods, float magnitude)
{
	if(inputState == pragma::platform::KeyState::Press || inputState == pragma::platform::KeyState::Release || inputState == pragma::platform::KeyState::Held) {
		auto &inputLayer = GetEffectiveInputBindingLayer();
		auto &keyMappings = inputLayer.GetKeyMappings();
		if((mods & pragma::platform::Modifier::AxisNegative) != pragma::platform::Modifier::None) {
			// We need to check if there are any keybindings with a command with the JoystickAxisSingle flag set,
			// in which case that keybinding has priority
			auto keyPositive = key - 1;
			auto it = keyMappings.find(CInt16(keyPositive));
			if(it != keyMappings.end() && const_cast<KeyBind &>(it->second).Execute(inputState, pressState, mods, magnitude) == true)
				return;
			mods &= ~pragma::platform::Modifier::AxisNegative;
		}
		auto it = keyMappings.find(CInt16(key));
		if(it != keyMappings.end())
			const_cast<KeyBind &>(it->second).Execute(inputState, pressState, mods, magnitude);
	}
}
void CEngine::Input(int key, pragma::platform::KeyState state, pragma::platform::Modifier mods, float magnitude) { Input(key, state, state, mods, magnitude); }
void CEngine::MouseInput(prosper::Window &window, pragma::platform::MouseButton button, pragma::platform::KeyState state, pragma::platform::Modifier mods)
{
	auto handled = false;
	if(CallCallbacksWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, pragma::platform::MouseButton, pragma::platform::KeyState, pragma::platform::Modifier>("OnMouseInput", handled, window, button, state, mods) == CallbackReturnType::HasReturnValue && handled == true)
		return;
	if(client != nullptr && client->RawMouseInput(button, state, mods) == false)
		return;
	if(WGUI::GetInstance().HandleMouseInput(window, button, state, mods))
		return;
	button += static_cast<pragma::platform::MouseButton>(pragma::platform::Key::Last);
	if(client != nullptr && client->MouseInput(button, state, mods) == false)
		return;
	Input(static_cast<int>(button), state);
}
void CEngine::GetMappedKeys(const std::string &cvarName, std::vector<pragma::platform::Key> &keys, uint32_t maxKeys)
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
				keys.push_back(static_cast<pragma::platform::Key>(pair.first));
				if(keys.size() == maxKeys)
					break;
			}
		}
	}
}
void CEngine::JoystickButtonInput(prosper::Window &window, const pragma::platform::Joystick &joystick, uint32_t key, pragma::platform::KeyState state)
{
	auto handled = false;
	if(CallCallbacksWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, std::reference_wrapper<const pragma::platform::Joystick>, uint32_t, pragma::platform::KeyState>("OnJoystickButtonInput", handled, window, joystick, key, state) == CallbackReturnType::HasReturnValue
	  && handled == true)
		return;
	KeyboardInput(window, static_cast<pragma::platform::Key>(key), -1, state, {});
}
void CEngine::JoystickAxisInput(prosper::Window &window, const pragma::platform::Joystick &joystick, uint32_t axis, pragma::platform::Modifier mods, float newVal, float deltaVal)
{
	auto handled = false;
	if(CallCallbacksWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, std::reference_wrapper<const pragma::platform::Joystick>, uint32_t, pragma::platform::Modifier, float, float>("OnJoystickAxisInput", handled, window, joystick, axis, mods, newVal, deltaVal)
	    == CallbackReturnType::HasReturnValue
	  && handled == true)
		return;
	auto oldVal = newVal - deltaVal;
	auto key = static_cast<pragma::platform::Key>(axis);
	auto state = (IsValidAxisInput(newVal) == true) ? pragma::platform::KeyState::Press : pragma::platform::KeyState::Release;
	auto it = m_joystickKeyStates.find(key);
	auto oldState = (it == m_joystickKeyStates.end()) ? pragma::platform::KeyState::Release : it->second;
	if(state == pragma::platform::KeyState::Release && oldState == pragma::platform::KeyState::Release)
		return;
	if(state == pragma::platform::KeyState::Press && oldState == pragma::platform::KeyState::Press)
		state = pragma::platform::KeyState::Held;

	m_joystickKeyStates[key] = state;
	mods |= pragma::platform::Modifier::AxisInput;
	if(umath::abs(newVal) > AXIS_PRESS_THRESHOLD) {
		if(umath::abs(oldVal) <= AXIS_PRESS_THRESHOLD)
			mods |= pragma::platform::Modifier::AxisPress; // Axis represents actual button press
	}
	else if(umath::abs(oldVal) > AXIS_PRESS_THRESHOLD)
		mods |= pragma::platform::Modifier::AxisRelease; // Axis represents actual button release
	KeyboardInput(window, key, -1, state, mods, newVal);
}
static auto cvAxisInputThreshold = GetClientConVar("cl_controller_axis_input_threshold");
bool CEngine::IsValidAxisInput(float axisInput) const
{
	if(!client)
		return false;
	return (umath::abs(axisInput) > cvAxisInputThreshold->GetFloat()) ? true : false;
}

bool CEngine::GetInputButtonState(float axisInput, pragma::platform::Modifier mods, pragma::platform::KeyState &inOutState) const
{
	if(IsValidAxisInput(axisInput) == false) {
		if((mods & pragma::platform::Modifier::AxisInput) != pragma::platform::Modifier::None) {
			inOutState = pragma::platform::KeyState::Release;
			return true;
		}
		inOutState = pragma::platform::KeyState::Invalid;
		return false;
	}
	if((mods & pragma::platform::Modifier::AxisInput) == pragma::platform::Modifier::None)
		return true; // No need to change state

	if((mods & pragma::platform::Modifier::AxisPress) != pragma::platform::Modifier::None)
		inOutState = pragma::platform::KeyState::Press;
	else if((mods & pragma::platform::Modifier::AxisRelease) != pragma::platform::Modifier::None)
		inOutState = pragma::platform::KeyState::Release;
	else {
		inOutState = pragma::platform::KeyState::Invalid;
		return false; // Not an actual key press
	}
	return true;
}
void CEngine::KeyboardInput(prosper::Window &window, pragma::platform::Key key, int scanCode, pragma::platform::KeyState state, pragma::platform::Modifier mods, float magnitude)
{
	auto handled = false;
	if(CallCallbacksWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, pragma::platform::Key, int, pragma::platform::KeyState, pragma::platform::Modifier, float>("OnKeyboardInput", handled, window, key, scanCode, state, mods, magnitude) == CallbackReturnType::HasReturnValue
	  && handled == true)
		return;
	if(client != nullptr && client->RawKeyboardInput(key, scanCode, state, mods, magnitude) == false)
		return;
	if(key == pragma::platform::Key::Escape) // Escape key is hardcoded
	{
		if(client != nullptr) {
			if(state == pragma::platform::KeyState::Press)
				client->ToggleMainMenu();
			return;
		}
	}
	if(key == pragma::platform::Key::GraveAccent) {
		if(mods == pragma::platform::Modifier::None) {
			if(state == pragma::platform::KeyState::Press)
				ToggleConsole();
		}
		return;
	}
	auto buttonState = state;
	auto bValidButtonInput = GetInputButtonState(magnitude, mods, buttonState);
	if(bValidButtonInput == true) {
		if(WGUI::GetInstance().HandleKeyboardInput(window, key, scanCode, buttonState, mods))
			return;
	}
	if(client != nullptr && client->KeyboardInput(key, scanCode, state, mods, magnitude) == false)
		return;
	auto ikey = umath::to_integral(key);
	if(ikey >= umath::to_integral(pragma::platform::Key::A) && ikey <= umath::to_integral(pragma::platform::Key::Z))
		key = static_cast<pragma::platform::Key>(std::tolower(ikey));
	Input(umath::to_integral(key), state, buttonState, mods, magnitude);
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
		Input(GLFW_CUSTOM_KEY_SCRL_UP, pragma::platform::KeyState::Press);
		Input(GLFW_CUSTOM_KEY_SCRL_UP, pragma::platform::KeyState::Release);
	}
	else {
		Input(GLFW_CUSTOM_KEY_SCRL_DOWN, pragma::platform::KeyState::Press);
		Input(GLFW_CUSTOM_KEY_SCRL_DOWN, pragma::platform::KeyState::Release);
	}
}

void CEngine::OnWindowFocusChanged(prosper::Window &window, bool bFocused)
{
	umath::set_flag(m_stateFlags, StateFlags::WindowFocused, bFocused);
	if(client != nullptr)
		client->UpdateSoundVolume();
}

// Usually we don't allow opening external files, but we make an exception for files that have been dropped into Pragma.
static std::unordered_map<std::string, std::string> g_droppedFiles;
namespace pragma {
	DLLCLIENT const std::unordered_map<std::string, std::string> &get_dropped_files() { return g_droppedFiles; }
};
const std::vector<CEngine::DroppedFile> &CEngine::GetDroppedFiles() const { return m_droppedFiles; }
void CEngine::OnFilesDropped(prosper::Window &window, std::vector<std::string> &files)
{
	if(client == nullptr)
		return;

	m_droppedFiles.reserve(files.size());
	auto addFile = [this](const std::string &fileName, const std::string &rootPath) {
		m_droppedFiles.push_back(DroppedFile {rootPath, fileName});
		auto path = util::Path::CreateFile(fileName).GetString();
		ustring::to_lower(path);
		g_droppedFiles.insert(std::make_pair(ufile::get_file_from_filename(path), path));
	};
	std::function<void(const std::vector<std::string> &, const std::optional<std::string> &)> addFiles = nullptr;
	addFiles = [this, &addFile, &addFiles](const std::vector<std::string> &files, const std::optional<std::string> &rootPath) {
		for(auto &f : files) {
			if(filemanager::is_system_file(f))
				addFile(f, rootPath ? *rootPath : ufile::get_path_from_filename(f));
			else if(filemanager::is_system_dir(f)) {
				auto subRootPath = rootPath;
				if(!subRootPath) {
					auto path = util::Path::CreatePath(f);
					path.PopBack();
					subRootPath = path.GetString();
				}
				std::vector<std::string> subFiles;
				std::vector<std::string> subDirs;
				filemanager::find_system_files(f + "/*", &subFiles, &subDirs);
				for(auto &fileName : subFiles)
					addFile(util::Path::CreateFile(f, fileName).GetString(), *subRootPath);
				for(auto &subDir : subDirs)
					subDir = util::Path::CreatePath(f, subDir).GetString();
				addFiles(subDirs, subRootPath);
			}
		}
	};
	addFiles(files, {});

	util::ScopeGuard g {[this]() {
		m_droppedFiles.clear();
		m_droppedFiles.shrink_to_fit();
	}};

	std::vector<std::string> droppedFileNames;
	droppedFileNames.reserve(m_droppedFiles.size());
	for(auto &f : m_droppedFiles)
		droppedFileNames.push_back(f.fileName);
	if(WGUI::GetInstance().HandleFileDrop(window, droppedFileNames))
		return;
	client->OnFilesDropped(files);
}
void CEngine::OnDragEnter(prosper::Window &window)
{
	if(client == nullptr)
		return;
	if(WGUI::GetInstance().HandleFileDragEnter(window))
		return;
	client->OnDragEnter(window);
}
void CEngine::OnDragExit(prosper::Window &window)
{
	if(client == nullptr)
		return;
	if(WGUI::GetInstance().HandleFileDragExit(window))
		return;
	client->OnDragExit(window);
}
bool CEngine::OnWindowShouldClose(prosper::Window &window)
{
	if(client == nullptr)
		return true;
	return client->OnWindowShouldClose(window);
}
void CEngine::OnPreedit(prosper::Window &window, const pragma::string::Utf8String &preeditString, const std::vector<int> &blockSizes, int focusedBlock, int caret)
{
	if(client == nullptr)
		return;
	client->OnPreedit(window, preeditString, blockSizes, focusedBlock, caret);
}
void CEngine::OnIMEStatusChanged(prosper::Window &window, bool imeEnabled)
{
	if(client == nullptr)
		return;
	WGUI::GetInstance().HandleIMEStatusChanged(window, imeEnabled);
	client->OnIMEStatusChanged(window, imeEnabled);
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
extern std::optional<Color> g_titleBarColor;
extern std::optional<Color> g_borderColor;
extern bool g_windowless;
void register_game_shaders();

void CEngine::HandleOpenGLFallback()
{
	if(ustring::compare(GetRenderAPI(), std::string {"opengl"}, false))
		return;
	auto *cl = static_cast<ClientState *>(GetClientState());
	if(!cl)
		return;
	auto msg = pragma::locale::get_text("prompt_fallback_to_opengl");
	if(util::debug::show_message_prompt(msg, util::debug::MessageBoxButtons::YesNo, util::get_program_name()) != util::debug::MessageBoxButton::Yes)
		return;
	cl->SetConVar("render_api", "opengl");
	SaveClientConfig();
	ShutDown();
	util::CommandInfo cmdInfo;
	cmdInfo.command = util::get_program_name();
	cmdInfo.absoluteCommandPath = false;
	util::start_process(cmdInfo);
}

std::optional<std::string> g_waylandLibdecorPlugin;
bool CEngine::Initialize(int argc, char *argv[])
{
	Engine::Initialize(argc, argv);

#ifdef __linux__
	if(!g_waylandLibdecorPlugin)
		g_waylandLibdecorPlugin = "cairo";
	if(g_waylandLibdecorPlugin) {
		::util::set_env_variable("GDK_BACKEND", "wayland");

		auto path = util::FilePath(util::get_program_path(), "modules/graphics/vulkan/libdecor/plugins", *g_waylandLibdecorPlugin);
		::util::set_env_variable("LIBDECOR_PLUGIN_DIR", path.GetString());
	}
#endif

	if(Lua::get_extended_lua_modules_enabled())
		RegisterConCommand("lc", [this](NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float) { RunConsoleCommand("lua_run_cl", argv); });
	auto &cmds = *m_clConfig;
	auto findCmdArg = [&cmds](const std::string &cmd) -> std::optional<std::string> {
		auto *args = cmds.Find(cmd);
		if(args && !args->empty())
			return args->front();
		return {};
	};

	auto renderApi = findCmdArg("render_api");
	if(renderApi)
		SetRenderAPI(*renderApi);

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

	auto windowRes = findCmdArg("cl_window_resolution");
	prosper::IPrContext::CreateInfo contextCreateInfo {};
	contextCreateInfo.width = 1280;
	contextCreateInfo.height = 1024;
	contextCreateInfo.windowless = g_windowless;
	contextCreateInfo.enableDiagnostics = IsGfxDiagnosticsModeEnabled();

	std::shared_ptr<udm::Data> renderApiData {};
	try {
		renderApiData = udm::Data::Load("cfg/render_api.udm");
	}
	catch(const udm::Exception &e) {
		Con::cwar << "Failed to load render API data: " << e.what() << Con::endl;
	}
	if(renderApiData) {
		auto &renderAPI = GetRenderAPI();
		auto data = renderApiData->GetAssetData().GetData();
		for(auto &pair : data["all"]["extensions"].ElIt()) {
			auto availability = prosper::IPrContext::ExtensionAvailability::EnableIfAvailable;
			udm::to_enum_value<prosper::IPrContext::ExtensionAvailability>(pair.property, availability);
			contextCreateInfo.extensions[std::string {pair.key}] = availability;
		}
		for(auto &[key, prop] : data[renderAPI]["extensions"].ElIt()) {
			auto availability = prosper::IPrContext::ExtensionAvailability::EnableIfAvailable;
			udm::to_enum_value<prosper::IPrContext::ExtensionAvailability>(prop, availability);
			contextCreateInfo.extensions[std::string {key}] = availability;
		}

		auto findAbsoluteFilePath = [](const std::string &relFilePath) -> util::Path {
			std::string strFilePath;
			if(filemanager::find_absolute_path(relFilePath, strFilePath))
				return util::FilePath(strFilePath);
			return util::FilePath(util::get_program_path(), relFilePath);
		};

		std::vector<std::string> layers;
		auto getLayerData = [&](udm::LinkedPropertyWrapper udmLayers) {
			layers.reserve(layers.size() + udmLayers.GetSize());
			for(auto &layer : udmLayers) {
				std::string name;
				layer["name"] >> name;
				if(name.empty())
					continue;
				layers.push_back(name);
				auto settings = layer["settings"];
				contextCreateInfo.layerSettings.reserve(contextCreateInfo.layerSettings.size() + settings.GetSize());
				for(auto &[key, prop] : settings.ElIt()) {
					auto type = prop.GetType();
					prosper::LayerSetting setting {};
					setting.layerName = name;
					setting.settingName = key;
					if(type == udm::Type::Element) {
						std::string settingType;
						prop["type"] >> settingType;
						auto udmValues = prop["values"];
						if(udmValues) {
							auto *a = udmValues.GetValuePtr<udm::Array>();
							if(a) {
								auto size = a->GetSize();
								::udm::visit(a->GetValueType(), [a, size, &setting, &settingType, &findAbsoluteFilePath](auto tag) {
									using T = typename decltype(tag)::type;
									if constexpr(std::is_same_v<T, udm::Boolean> || std::is_same_v<T, udm::Int32> || std::is_same_v<T, udm::Int64> || std::is_same_v<T, udm::UInt32> || std::is_same_v<T, udm::UInt64> || std::is_same_v<T, udm::Float> || std::is_same_v<T, udm::Double>) {
										auto *values = new T[size];
										for(size_t i = 0; i < size; ++i)
											values[i] = a->GetValue<T>(i);
										setting.SetValues(size, values);
									}
									else if constexpr(std::is_same_v<T, udm::String>) {
										if(settingType == "file") {
											auto *values = new const char *[size];
											std::vector<util::Path> tmpPaths;
											tmpPaths.reserve(size);
											for(size_t i = 0; i < size; ++i) {
												auto filePath = findAbsoluteFilePath(a->GetValue<std::string>(i));
												tmpPaths.push_back(filePath);
												values[i] = filePath.GetString().c_str();
											}
											// SetValues copies the strings, so we can safely delete the temporary paths
											setting.SetValues(size, values);
										}
										else {
											auto *values = new const char *[size];
											for(size_t i = 0; i < size; ++i)
												values[i] = a->GetValue<std::string>(i).c_str();
											setting.SetValues(size, values);
										}
									}
									else
										throw std::invalid_argument {"Unsupported layer setting type " + std::string {magic_enum::enum_name(a->GetValueType())}};
								});
							}
						}
						else {
							auto udmValue = prop["value"];
							::udm::visit(udmValue.GetType(), [&udmValue, &setting, &settingType, &findAbsoluteFilePath](auto tag) {
								using T = typename decltype(tag)::type;
								if constexpr(std::is_same_v<T, udm::Boolean> || std::is_same_v<T, udm::Int32> || std::is_same_v<T, udm::Int64> || std::is_same_v<T, udm::UInt32> || std::is_same_v<T, udm::UInt64> || std::is_same_v<T, udm::Float> || std::is_same_v<T, udm::Double>)
									setting.SetValues(1, &udmValue.GetValue<T>());
								else if constexpr(std::is_same_v<T, udm::String>) {
									if(settingType == "file") {
										auto filePath = findAbsoluteFilePath(udmValue.GetValue<T>());
										auto *str = filePath.GetString().c_str();
										setting.SetValues(1, &str);
									}
									else {
										auto *str = udmValue.GetValue<T>().c_str();
										setting.SetValues(1, &str);
									}
								}
								else
									throw std::invalid_argument {"Unsupported layer setting type " + std::string {magic_enum::enum_name(udmValue.GetType())}};
							});
						}
					}
					else {
						::udm::visit(type, [&prop, &setting](auto tag) {
							using T = typename decltype(tag)::type;
							if constexpr(std::is_same_v<T, udm::Boolean> || std::is_same_v<T, udm::Int32> || std::is_same_v<T, udm::Int64> || std::is_same_v<T, udm::UInt32> || std::is_same_v<T, udm::UInt64> || std::is_same_v<T, udm::Float> || std::is_same_v<T, udm::Double>)
								setting.SetValues(1, &prop.GetValue<T>());
							else if constexpr(std::is_same_v<T, udm::String>) {
								auto *str = prop.GetValue<T>().c_str();
								setting.SetValues(1, &str);
							}
							else
								throw std::invalid_argument {"Unsupported layer setting type " + std::string {magic_enum::enum_name(prop.GetType())}};
						});
					}
					contextCreateInfo.layerSettings.push_back(setting);
				}
			}
		};
		getLayerData(data["all"]["layers"]);
		getLayerData(data[renderAPI]["layers"]);
		contextCreateInfo.layers = std::move(layers);
	}

	if(windowRes) {
		std::vector<std::string> vals;
		ustring::explode(*windowRes, "x", vals);
		if(vals.size() >= 2) {
			contextCreateInfo.width = util::to_int(vals[0]);
			contextCreateInfo.height = util::to_int(vals[1]);
		}
	}
	// SetResolution(Vector2i(contextCreateInfo.width,contextCreateInfo.height));

	auto renderRes = findCmdArg("cl_render_resolution");
	if(renderRes) {
		std::vector<std::string> vals;
		ustring::explode(*renderRes, "x", vals);
		if(vals.size() >= 2) {
			m_renderResolution = {util::to_int(vals[0]), util::to_int(vals[1])};
		}
	}
	//

	auto windowMode = findCmdArg("cl_render_window_mode");
	int mode = 1;
	if(windowMode)
		mode = util::to_int(*windowMode);
	auto &initialWindowSettings = GetRenderContext().GetInitialWindowSettings();
	initialWindowSettings.windowedMode = (mode != 0);
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

	auto renderMonitor = findCmdArg("cl_render_monitor");
	if(renderMonitor) {
		auto monitor = util::to_int(*renderMonitor);
		auto monitors = pragma::platform::get_monitors();
		if(monitor < monitors.size() && monitor > 0)
			initialWindowSettings.monitor = monitors[monitor];
	}

	auto gpuDevice = findCmdArg("cl_gpu_device");
	if(gpuDevice) {
		auto device = *gpuDevice;
		std::vector<std::string> subStrings;
		ustring::explode(device, ",", subStrings);
		if(subStrings.size() >= 2)
			contextCreateInfo.device = {static_cast<prosper::Vendor>(util::to_uint(subStrings.at(0))), util::to_uint(subStrings.at(1))};
	}

	auto presentMode = prosper::PresentModeKHR::Mailbox;
	auto renderPresentMode = findCmdArg("cl_render_present_mode");
	if(renderPresentMode) {
		auto mode = util::to_int(*renderPresentMode);
		if(mode == 0)
			presentMode = prosper::PresentModeKHR::Immediate;
		else if(mode == 1)
			presentMode = prosper::PresentModeKHR::Fifo;
		else
			presentMode = prosper::PresentModeKHR::Mailbox;
	}
	contextCreateInfo.presentMode = presentMode;

	GetRenderContext().Initialize(contextCreateInfo);

	auto &window = GetRenderContext().GetWindow();
	if(g_titleBarColor.has_value())
		window->SetTitleBarColor(*g_titleBarColor);
	if(g_borderColor.has_value())
		window->SetBorderColor(*g_borderColor);

#ifdef _WIN32
#if defined(WINVER) && (WINVER >= 0x0501)
	auto h = GetConsoleWindow();
	if(g_titleBarColor.has_value()) {
		auto tmp = *g_titleBarColor;
		umath::swap(tmp.r, tmp.b);
		auto hex = tmp.ToHexColorRGB();
		COLORREF hexCol = ::util::to_hex_number("0x" + hex);
		const DWORD ATTR_CAPTION_COLOR = 35; // See DWMWINDOWATTRIBUTE::DWMWA_CAPTION_COLOR, can't use the enum because it may not be available and there's no way to check for it
		DwmSetWindowAttribute(h, ATTR_CAPTION_COLOR, &hexCol, sizeof(hexCol));
	}
	if(g_borderColor.has_value()) {
		auto tmp = *g_borderColor;
		umath::swap(tmp.r, tmp.b);
		auto hex = tmp.ToHexColorRGB();
		COLORREF hexCol = ::util::to_hex_number("0x" + hex);
		const DWORD ATTR_BORDER_COLOR = 34; // See DWMWINDOWATTRIBUTE::DWMWA_BORDER_COLOR, can't use the enum because it may not be available and there's no way to check for it
		DwmSetWindowAttribute(h, ATTR_BORDER_COLOR, &hexCol, sizeof(hexCol));
	}
#endif
#endif

	auto &shaderManager = GetRenderContext().GetShaderManager();
	shaderManager.RegisterShader("clear_color", [](prosper::IPrContext &context, const std::string &identifier) { return new pragma::ShaderClearColor(context, identifier); });
	shaderManager.RegisterShader("gradient", [](prosper::IPrContext &context, const std::string &identifier) { return new pragma::ShaderGradient(context, identifier); });

	shaderManager.GetShader("blur_horizontal");
	shaderManager.GetShader("blur_vertical");

	// Initialize Client Instance
	auto matManager = msys::CMaterialManager::Create(GetRenderContext());
	matManager->SetImportDirectory("addons/converted/");
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
	defaultFontSet = "dejavu";
	auto *lanInfo = pragma::locale::get_language_info();
	if(lanInfo && lanInfo->configData) {
		std::vector<std::string> characterSetRequirements;
		(*lanInfo->configData)["font"]["characterSetRequirements"](characterSetRequirements);
		if(std::find(characterSetRequirements.begin(), characterSetRequirements.end(), "jp") != characterSetRequirements.end() || std::find(characterSetRequirements.begin(), characterSetRequirements.end(), "zh") != characterSetRequirements.end()) {
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
	auto r = gui.Initialize(GetRenderResolution(), fontData->fileName, {"source-han-sans/SourceHanSans-VF.ttf"});
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
	  [this](pragma::platform::Key key, const std::string &cmd) -> std::string {
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
		  std::vector<pragma::platform::Key> keys;
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
		m_gpuProfilingStageManager = std::make_unique<pragma::debug::ProfilingStageManager<pragma::debug::GPUProfilingStage>>();
		auto &gpuProfiler = *m_gpuProfiler;
		m_gpuProfilingStageManager->InitializeProfilingStageManager(gpuProfiler);
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

	{
		auto regBase = std::make_shared<pragma::shadergraph::NodeRegistry>();
		//regBase->RegisterNode<pragma::shadergraph::BlackbodyNode>("blackbody");
		regBase->RegisterNode<pragma::shadergraph::BrightContrastNode>("bright_contrast");
		regBase->RegisterNode<pragma::shadergraph::ClampNode>("clamp");
		//regBase->RegisterNode<pragma::shadergraph::ColorNode>("color");
		regBase->RegisterNode<pragma::shadergraph::CombineXyzNode>("combine_xyz");
		regBase->RegisterNode<pragma::shadergraph::GammaNode>("gamma");
		//regBase->RegisterNode<pragma::shadergraph::GroupNode>("group");
		regBase->RegisterNode<pragma::shadergraph::EmissionNode>("emission");
		//regBase->RegisterNode<pragma::shadergraph::GroupInputNode>("group_input");
		//regBase->RegisterNode<pragma::shadergraph::GroupOutputNode>("group_output");
		regBase->RegisterNode<pragma::shadergraph::HsvNode>("hsv");
		regBase->RegisterNode<pragma::shadergraph::InvertNode>("invert");
		//regBase->RegisterNode<pragma::shadergraph::LightFalloffNode>("light_falloff");
		regBase->RegisterNode<pragma::shadergraph::MapRangeNode>("map_range_node");
		//regBase->RegisterNode<pragma::shadergraph::MappingNode>("mapping");
		regBase->RegisterNode<pragma::shadergraph::MathNode>("math");
		regBase->RegisterNode<pragma::shadergraph::MixNode>("mix");
		//regBase->RegisterNode<pragma::shadergraph::RgbRampNode>("rgb_ramp");
		regBase->RegisterNode<pragma::shadergraph::RgbToBwNode>("rgb_to_bw");
		regBase->RegisterNode<pragma::shadergraph::SepiaToneNode>("sepia_tone");
		regBase->RegisterNode<pragma::shadergraph::SeparateXyzNode>("separate_xyz");
		regBase->RegisterNode<pragma::shadergraph::ValueNode>("value");
		//regBase->RegisterNode<pragma::shadergraph::VectorDisplacementNode>("vector_displacement");
		//regBase->RegisterNode<pragma::shadergraph::VectorMapRangeNode>("vector_map_range");
		regBase->RegisterNode<pragma::shadergraph::VectorMathNode>("vector_math");
		//regBase->RegisterNode<pragma::shadergraph::VectorRotateNode>("vector_rotate");
		//regBase->RegisterNode<pragma::shadergraph::VectorTransformNode>("vector_transform");
		//regBase->RegisterNode<pragma::shadergraph::WavelengthNode>("wavelength");

		auto regScene = std::make_shared<pragma::shadergraph::NodeRegistry>();
		regScene->RegisterNode<pragma::rendering::shader_graph::SceneOutputNode>("output");
		regScene->RegisterNode<pragma::rendering::shader_graph::CameraNode>("camera");
		regScene->RegisterNode<pragma::rendering::shader_graph::FogNode>("fog");
		regScene->RegisterNode<pragma::rendering::shader_graph::LightmapNode>("lightmap");
		regScene->RegisterNode<pragma::rendering::shader_graph::ObjectNode>("object");
		regScene->RegisterNode<pragma::rendering::shader_graph::TimeNode>("time");
		regScene->RegisterNode<pragma::rendering::shader_graph::PbrNode>("pbr");
		regScene->RegisterNode<pragma::rendering::shader_graph::ToonNode>("toon");
		regScene->RegisterNode<pragma::rendering::shader_graph::ImageTextureNode>("image_texture");
		regScene->RegisterNode<pragma::rendering::shader_graph::MaterialTextureNode>("material_texture");
		regScene->RegisterNode<pragma::rendering::shader_graph::TextureCoordinateNode>("texture_coordinate");
		regScene->RegisterNode<pragma::rendering::shader_graph::VectorTransformNode>("vector_transform");
		regScene->RegisterNode<pragma::rendering::shader_graph::GeometryNode>("geometry");
		regScene->RegisterNode<pragma::rendering::shader_graph::InputParameterBooleanNode>("input_parameter_boolean");
		regScene->RegisterNode<pragma::rendering::shader_graph::InputParameterIntNode>("input_parameter_int");
		regScene->RegisterNode<pragma::rendering::shader_graph::InputParameterUIntNode>("input_parameter_uint");
		regScene->RegisterNode<pragma::rendering::shader_graph::InputParameterUInt16Node>("input_parameter_uint16");
		regScene->RegisterNode<pragma::rendering::shader_graph::InputParameterColorNode>("input_parameter_color");
		regScene->RegisterNode<pragma::rendering::shader_graph::InputParameterVectorNode>("input_parameter_vector");
		regScene->RegisterNode<pragma::rendering::shader_graph::InputParameterVector4Node>("input_parameter_vector4");
		regScene->RegisterNode<pragma::rendering::shader_graph::InputParameterPointNode>("input_parameter_point");
		regScene->RegisterNode<pragma::rendering::shader_graph::InputParameterNormalNode>("input_parameter_normal");
		regScene->RegisterNode<pragma::rendering::shader_graph::InputParameterPoint2Node>("input_parameter_point2");
		regScene->RegisterNode<pragma::rendering::shader_graph::InputParameterStringNode>("input_parameter_string");
		regScene->RegisterNode<pragma::rendering::shader_graph::InputParameterTransformNode>("input_parameter_transform");
		regScene->RegisterNode<pragma::rendering::shader_graph::InputParameterFloatNode>("input_parameter_float");
		regScene->RegisterNode<pragma::rendering::shader_graph::InputParameterTextureNode>("input_parameter_texture");

		std::vector<std::string> files;
		filemanager::find_files("scripts/shader_data/materials/*.udm", &files, nullptr);
		for(auto &f : files) {
			ufile::remove_extension_from_filename(f);
			pragma::rendering::shader_material::get_cache().Load(f);
		}

		auto &cache = pragma::rendering::shader_material::get_cache();
		for(auto &[name, mat] : cache.GetShaderMaterials()) {
			auto nodeName = name;
			nodeName = "sm_" + nodeName;
			auto node = std::make_shared<pragma::rendering::shader_graph::ShaderMaterialNode>(pragma::GString {nodeName}, *mat);
			regScene->RegisterNode(node);
		}

		regScene->AddChildRegistry(regBase);

		auto regPp = std::make_shared<pragma::shadergraph::NodeRegistry>();
		regPp->AddChildRegistry(regBase);

		m_shaderGraphManager = std::make_unique<pragma::rendering::ShaderGraphManager>();
		m_shaderGraphManager->RegisterGraphTypeManager("post_processing", regPp);
		m_shaderGraphManager->RegisterGraphTypeManager("object", regScene);
		//m_shaderGraphManager->RegisterGraphTypeManager("group", regGroup);

		m_shaderGraphManager->GetModuleManager().RegisterFactory("pbr", [](pragma::ShaderGraph &shader) -> std::unique_ptr<pragma::rendering::ShaderGraphModule> { return std::make_unique<pragma::rendering::shader_graph::PbrModule>(shader); });
		m_shaderGraphManager->GetModuleManager().RegisterFactory("toon", [](pragma::ShaderGraph &shader) -> std::unique_ptr<pragma::rendering::ShaderGraphModule> { return std::make_unique<pragma::rendering::shader_graph::ToonModule>(shader); });
		m_shaderGraphManager->GetModuleManager().RegisterFactory("image_texture", [](pragma::ShaderGraph &shader) -> std::unique_ptr<pragma::rendering::ShaderGraphModule> { return std::make_unique<pragma::rendering::shader_graph::ImageTextureModule>(shader); });
		m_shaderGraphManager->GetModuleManager().RegisterFactory("input_data", [](pragma::ShaderGraph &shader) -> std::unique_ptr<pragma::rendering::ShaderGraphModule> { return std::make_unique<pragma::rendering::shader_graph::InputDataModule>(shader); });

		// TODO: Load shader graphs from mounted addons
		for(auto &[type, man] : m_shaderGraphManager->GetShaderGraphTypeManagers()) {
			std::vector<std::string> sgFiles;
			filemanager::find_files(pragma::rendering::ShaderGraphManager::ROOT_GRAPH_PATH + type + std::string {"/*."} + pragma::shadergraph::Graph::EXTENSION_BINARY, &sgFiles, nullptr);
			filemanager::find_files(pragma::rendering::ShaderGraphManager::ROOT_GRAPH_PATH + type + std::string {"/*."} + pragma::shadergraph::Graph::EXTENSION_ASCII, &sgFiles, nullptr);
			for(auto &f : sgFiles) {
				ufile::remove_extension_from_filename(f, std::array<std::string, 2> {pragma::shadergraph::Graph::EXTENSION_BINARY, pragma::shadergraph::Graph::EXTENSION_ASCII});
				std::string err;
				auto graph = m_shaderGraphManager->LoadShader(f, err);
				if(!graph)
					spdlog::error("Failed to load shader graph '{}': {}", f, err);
			}
		}
	}

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
	return (it != m_fontSets.end()) ? it->second.get() : nullptr;
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

						uint32_t fontSizeAdjustment;
						if(fontData["fontSizeAdjustment"](fontSizeAdjustment))
							fileData.fontSizeAdjustment = fontSizeAdjustment;

						fileData.fileName = dir + '/' + fileData.fileName;
						fontSet.fileData.push_back(fileData);
					}
					else {
						for(auto &udmFileData : fontData["files"]) {
							FontSetFileData fileData {};
							udmFileData["fileName"](fileData.fileName);

							uint32_t fontSizeAdjustment;
							if(fontData["fontSizeAdjustment"](fontSizeAdjustment))
								fileData.fontSizeAdjustment = fontSizeAdjustment;

							fileData.fileName = dir + '/' + fileData.fileName;
							udm::to_flags(udmFileData["features"], fileData.flags);
							fontSet.fileData.push_back(fileData);
						}
					}
					m_fontSets[dir] = std::make_unique<FontSet>(std::move(fontSet));
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
				if(console && m_consoleType == ConsoleType::GUIDetached && !console->IsExternallyOwned()) {
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
	if(settings.width == 0 || settings.height == 0)
		return nullptr;
	auto &mainWindowCreateInfo = c_engine->GetRenderContext().GetWindow().GetWindowSettings();
	settings.flags = mainWindowCreateInfo.flags;
	settings.api = mainWindowCreateInfo.api;
	auto window = c_engine->GetRenderContext().CreateWindow(settings);
	if(!window)
		return nullptr;

	if(g_titleBarColor.has_value())
		(*window)->SetTitleBarColor(*g_titleBarColor);
	if(g_borderColor.has_value())
		(*window)->SetBorderColor(*g_borderColor);

	auto *pWindow = window.get();
	pWindow->GetStagingRenderTarget(); // This will initialize the staging target immediately
	(*pWindow)->SetWindowSizeCallback([pWindow](pragma::platform::Window &window, Vector2i size) {
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
	window->SetKeyCallback([this, &window](pragma::platform::Window &glfwWindow, pragma::platform::Key key, int scanCode, pragma::platform::KeyState state, pragma::platform::Modifier mods) mutable { KeyboardInput(window, key, scanCode, state, mods); });
	window->SetMouseButtonCallback([this, &window](pragma::platform::Window &glfwWindow, pragma::platform::MouseButton button, pragma::platform::KeyState state, pragma::platform::Modifier mods) mutable { MouseInput(window, button, state, mods); });
	window->SetCharCallback([this, &window](pragma::platform::Window &glfwWindow, unsigned int c) mutable { CharInput(window, c); });
	window->SetScrollCallback([this, &window](pragma::platform::Window &glfwWindow, Vector2 offset) mutable { ScrollInput(window, offset); });
	window->SetFocusCallback([this, &window](pragma::platform::Window &glfwWindow, bool bFocused) mutable { OnWindowFocusChanged(window, bFocused); });
	window->SetDropCallback([this, &window](pragma::platform::Window &glfwWindow, std::vector<std::string> &files) mutable { OnFilesDropped(window, files); });
	window->SetDragEnterCallback([this, &window](pragma::platform::Window &glfwWindow) mutable { OnDragEnter(window); });
	window->SetDragExitCallback([this, &window](pragma::platform::Window &glfwWindow) mutable { OnDragExit(window); });
	window->SetOnShouldCloseCallback([this, &window](pragma::platform::Window &glfwWindow) -> bool { return OnWindowShouldClose(window); });
	window->SetPreeditCallback([this, &window](pragma::platform::Window &glfwWindow, int preedit_count, unsigned int *preedit_string, int block_count, int *block_sizes, int focused_block, int caret) {
		std::vector<int32_t> istr;
		istr.resize(preedit_count);
		for(auto i = decltype(preedit_count) {0u}; i < preedit_count; ++i)
			istr[i] = static_cast<int32_t>(preedit_string[i]);
		pragma::string::Utf8String preeditString {istr.data(), istr.size()};

		std::vector<int32_t> blockSizes;
		blockSizes.reserve(block_count);
		for(auto i = decltype(block_count) {0u}; i < block_count; ++i)
			blockSizes.push_back(block_sizes[i]);
		OnPreedit(window, preeditString, blockSizes, focused_block, caret);
	});
	window->SetIMEStatusCallback([this, &window](pragma::platform::Window &glfwWindow) { OnIMEStatusChanged(window, glfwWindow.IsIMEEnabled()); });
}
void CEngine::OnWindowResized(prosper::Window &window, Vector2i size)
{
	m_stateFlags |= StateFlags::WindowSizeChanged;
	m_tWindowResizeTime = util::Clock::now();
}

DLLCLIENT std::optional<std::string> g_customWindowIcon {};
void CEngine::OnWindowInitialized()
{
	pragma::RenderContext::OnWindowInitialized();
	auto &window = GetRenderContext().GetWindow();
	InitializeWindowInputCallbacks(window);
	window->SetWindowSizeCallback([this, &window](pragma::platform::Window &glfwWindow, Vector2i size) mutable { OnWindowResized(window, size); });

	if(g_customWindowIcon.has_value()) {
		auto imgBuf = uimg::load_image(*g_customWindowIcon, uimg::PixelFormat::LDR);
		if(imgBuf) {
			imgBuf->ToLDRFormat(uimg::Format::RGBA32);
			window->SetWindowIcon(imgBuf->GetWidth(), imgBuf->GetHeight(), static_cast<uint8_t *>(imgBuf->GetData()));
			pragma::platform::poll_events();
		}
	}
}
void CEngine::InitializeExternalArchiveManager() { util::initialize_external_archive_manager(GetClientState()); }
bool CEngine::StartProfilingStage(const char *stage) { return m_cpuProfilingStageManager && m_cpuProfilingStageManager->StartProfilerStage(stage); }
bool CEngine::StopProfilingStage() { return m_cpuProfilingStageManager && m_cpuProfilingStageManager->StopProfilerStage(); }
bool CEngine::StartGPUProfilingStage(const char *stage) { return m_gpuProfilingStageManager && m_gpuProfilingStageManager->StartProfilerStage(stage); }
bool CEngine::StopGPUProfilingStage() { return m_gpuProfilingStageManager && m_gpuProfilingStageManager->StopProfilerStage(); }
bool CEngine::GetControllersEnabled() const { return umath::is_flag_set(m_stateFlags, StateFlags::ControllersEnabled); }
void CEngine::SetControllersEnabled(bool b)
{
	if(GetControllersEnabled() == b)
		return;
	umath::set_flag(m_stateFlags, StateFlags::ControllersEnabled, b);
	if(b == false) {
		pragma::platform::set_joysticks_enabled(false);
		return;
	}
	pragma::platform::set_joysticks_enabled(true);
	pragma::platform::set_joystick_axis_threshold(0.01f);
	pragma::platform::set_joystick_button_callback([this](const pragma::platform::Joystick &joystick, uint32_t key, pragma::platform::KeyState oldState, pragma::platform::KeyState newState) {
		auto keyOffset = GLFW_CUSTOM_KEY_JOYSTICK_0_KEY_START + joystick.GetJoystickId() * GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT;
		JoystickButtonInput(GetWindow(), joystick, key + keyOffset, newState);
	});
	pragma::platform::set_joystick_axis_callback([this](const pragma::platform::Joystick &joystick, uint32_t axisId, float oldVal, float newVal) {
		m_rawInputJoystickMagnitude = newVal;
		auto mods = pragma::platform::Modifier::None;
		auto axisOffset = GLFW_CUSTOM_KEY_JOYSTICK_0_AXIS_START + joystick.GetJoystickId() * GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT;
		if(SEPARATE_JOYSTICK_AXES == true) {
			axisId *= 2;
			if(umath::sign(oldVal) != umath::sign(newVal)) {
				auto prevAxisId = axisId;
				auto prevMods = mods;
				if(oldVal < 0.f) {
					++prevAxisId;
					prevMods |= pragma::platform::Modifier::AxisNegative;
				}
				JoystickAxisInput(GetWindow(), joystick, prevAxisId + axisOffset, prevMods, 0.f, 0.f - oldVal);
				oldVal = 0.f;
			}
			if(newVal < 0.f) {
				oldVal = -oldVal;
				newVal = -newVal;
				++axisId;
				mods |= pragma::platform::Modifier::AxisNegative;
			}
		}
		JoystickAxisInput(GetWindow(), joystick, axisId + axisOffset, mods, newVal, newVal - oldVal);
	});
	pragma::platform::set_joystick_state_callback([this](const pragma::platform::Joystick &joystick, bool bConnected) { c_engine->CallCallbacks<void, std::reference_wrapper<const pragma::platform::Joystick>, bool>("OnJoystickStateChanged", std::ref(joystick), bConnected); });
}
REGISTER_CONVAR_CALLBACK_CL(cl_controller_enabled, [](NetworkState *state, const ConVar &cv, bool oldVal, bool newVal) { c_engine->SetControllersEnabled(newVal); });

float CEngine::GetRawJoystickAxisMagnitude() const { return m_rawInputJoystickMagnitude; }

std::unique_ptr<CEngine::ConVarInfoList> &CEngine::GetConVarConfig(NwStateType type)
{
	if(type == NwStateType::Client)
		return m_clConfig;
	return Engine::GetConVarConfig(type);
}
Engine::StateInstance &CEngine::GetStateInstance(NetworkState &nw)
{
	if(m_clInstance->state.get() == &nw)
		return *m_clInstance;
	return Engine::GetStateInstance(nw);
}
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
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::get_domain().BeginTask("Reloading shader " + name);
#endif
	whShader.get()->Initialize(true);
	auto nummPipelines = whShader->GetPipelineCount();
	for(auto i = decltype(nummPipelines) {0u}; i < nummPipelines; ++i)
		whShader->GetPipelineInfo(i); // Force immediate reload
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::get_domain().EndTask();
#endif
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
CGame *pragma::get_client_game() { return c_game; }

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

	WGUI::GetInstance().ClearSkins(); // Should be cleared before lua states are closed
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
	StartGPUProfilingStage("Frame");

	auto ptrDrawCmd = std::dynamic_pointer_cast<prosper::IPrimaryCommandBuffer>(primWindowCmd);
	CallCallbacks<void, std::reference_wrapper<std::shared_ptr<prosper::IPrimaryCommandBuffer>>>("DrawFrame", std::ref(ptrDrawCmd));

#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::get_domain().BeginTask("poll_material_manager");
#endif
	static_cast<msys::CMaterialManager &>(*m_clInstance->materialManager).Poll(); // Requires active command buffer
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::get_domain().EndTask();
#endif

	StartProfilingStage("GUILogic");
	auto &gui = WGUI::GetInstance();
	gui.Think(primWindowCmd);
	StopProfilingStage(); // GUILogic

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

	StopGPUProfilingStage(); // Frame
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
		StartProfilingStage("RecordGUI");
		StartProfilingStage("GUI");

		WGUI::GetInstance().BeginDraw();
		CallCallbacks<void>("PreRecordGUI");
		if(c_game != nullptr)
			c_game->PreGUIRecord();
		auto &context = GetRenderContext();
		for(auto &window : context.GetWindows()) {
			if(!window || window->IsValid() == false || window->GetState() != prosper::Window::State::Active)
				continue;
			auto &windowRt = window->GetStagingRenderTarget();
			auto &swapCmdGroup = window->GetSwapCommandBufferGroup();
			swapCmdGroup.StartRecording(windowRt->GetRenderPass(), windowRt->GetFramebuffer());
			swapCmdGroup.Record([this, window](prosper::ISecondaryCommandBuffer &drawCmd) {
				auto &gui = WGUI::GetInstance();
				StartProfilingStage("DrawGUI");
				gui.Draw(*window, drawCmd);
				StopProfilingStage();
			});
			swapCmdGroup.EndRecording();
		}
		CallCallbacks<void>("PostRecordGUI");
		if(c_game != nullptr)
			c_game->PostGUIRecord();

		StopProfilingStage(); // GUI
		StopProfilingStage(); // RecordGUI
	}

	auto *cl = static_cast<ClientState *>(GetClientState());
	auto tStart = util::Clock::now();
	if(cl != nullptr) {
		StartProfilingStage("RecordScene");
		StartGPUProfilingStage("DrawScene");

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

		StopGPUProfilingStage(); // DrawScene
		StopProfilingStage();    // RecordScene
	}

	if(drawGui) {
		StartProfilingStage("ExecuteGUIDrawCalls");
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
		CallCallbacks<void>("PostDrawGUI");
		if(c_game != nullptr)
			c_game->PostGUIDraw();
		WGUI::GetInstance().EndDraw();
		StopProfilingStage(); // ExecuteGUIDrawCalls
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
	pragma::platform::poll_joystick_events();

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

	StartProfilingStage("DrawFrame");

	pragma::RenderContext::DrawFrame();
	CallCallbacks("Draw");
	StopProfilingStage();            // DrawFrame
	pragma::platform::poll_events(); // Needs to be called AFTER rendering!
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
	if(umath::is_flag_set(m_stateFlags, StateFlags::WindowSizeChanged)) {
		auto t = util::Clock::now();
		auto dt = t - m_tWindowResizeTime;
		// If the window is being resized by the user, we don't want to update the resolution constantly,
		// so we add a small delay
		if(dt > std::chrono::milliseconds {250}) {
			auto &window = GetWindow();
			auto size = window.GetGlfwWindow().GetSize();
			if(size.x > 0 && size.y > 0) { // If either size is 0, the window is probably minimized and we don't need to update.
				umath::set_flag(m_stateFlags, StateFlags::WindowSizeChanged, false);
				OnResolutionChanged(size.x, size.y);
			}
		}
	}

	pragma::locale::poll();
	ProcessConsoleInput();
	RunTickEvents();

	Engine::StartProfilingStage("Tick");
	// The client tick has to run BEFORE the server tick!!!
	// This is to avoid issues in singleplayer, where the client would use data it received from the server and apply the same calculations on the already modified data.
	StartProfilingStage("ClientTick");
	auto *cl = GetClientState();
	if(cl != NULL)
		cl->Tick();
	StopProfilingStage(); // ClientTick

	Engine::StartProfilingStage("ServerTick");
	auto *sv = GetServerNetworkState();
	if(sv != NULL)
		sv->Tick();
	Engine::StopProfilingStage(); // ServerTick
	Engine::StopProfilingStage(); // Tick

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
	umath::set_flag(m_stateFlags, StateFlags::FirstFrame, true);

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

CEngine::DroppedFile::DroppedFile(const std::string &rootPath, const std::string &_fullPath) : fullPath(_fullPath)
{
	auto path = util::Path::CreateFile(fullPath);
	path.MakeRelative(rootPath);
	fileName = path.GetString();
}

REGISTER_CONVAR_CALLBACK_CL(cl_render_monitor, [](NetworkState *, const ConVar &, int32_t, int32_t monitor) {
	auto monitors = pragma::platform::get_monitors();
	if(monitor < monitors.size() && monitor >= 0)
		c_engine->GetWindow().SetMonitor(monitors[monitor]);
})

REGISTER_CONVAR_CALLBACK_CL(cl_render_window_mode, [](NetworkState *, const ConVar &, int32_t, int32_t val) {
	c_engine->GetWindow().SetWindowedMode(val != 0);
	c_engine->GetWindow().SetNoBorder(val == 2);
})

REGISTER_CONVAR_CALLBACK_CL(cl_window_resolution, [](NetworkState *, const ConVar &, std::string, std::string val) {
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

REGISTER_CONVAR_CALLBACK_CL(cl_render_resolution, [](NetworkState *, const ConVar &, std::string, std::string val) {
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

REGISTER_CONVAR_CALLBACK_CL(cl_gpu_timer_queries_enabled, [](NetworkState *, const ConVar &, bool, bool enabled) {
	if(c_engine == nullptr)
		return;
	c_engine->SetGPUProfilingEnabled(enabled);
})
