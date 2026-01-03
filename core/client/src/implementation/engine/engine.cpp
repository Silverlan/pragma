// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/console/helper.hpp"
#include <cassert>
#ifdef _WIN32

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

#endif

module pragma.client;

import :engine;
import util_zip;
import :assets;
import :client_state;
import :entities.components;
import :game;
import :gui;
import :model;
import :networking;
import :rendering.shader_graph;
import :rendering.shaders;
import pragma.gui;
import pragma.shadergraph;
import pragma.string.unicode;

#undef CreateWindow
#undef CreateFile
#undef max

extern "C" {
void DLLCLIENT RunCEngine(int argc, char *argv[])
{
	auto en = pragma::initialize_engine<pragma::CEngine>(argc, argv);
	if(en == nullptr)
		return;
	en->Release(); // Has to be called before object is actually destroyed, to make sure weak_ptr references are still valid
	en = nullptr;
}
}

//__declspec(dllimport) std::vector<void*> _vkImgPtrs;
decltype(pragma::CEngine::AXIS_PRESS_THRESHOLD) pragma::CEngine::AXIS_PRESS_THRESHOLD = 0.5f;
// If set to true, each joystick axes will be split into a positive and a negative axis, which
// can be bound individually
static const auto SEPARATE_JOYSTICK_AXES = true;

static pragma::CEngine *g_engine = nullptr;
pragma::CEngine::CEngine(int argc, char *argv[])
    : Engine(argc, argv), RenderContext(), m_nearZ(baseEnvCameraComponent::DEFAULT_NEAR_Z), //10.0f), //0.1f
      m_farZ(baseEnvCameraComponent::DEFAULT_FAR_Z), m_fps(0), m_tFPSTime(0.f), m_tLastFrame(util::Clock::now()), m_tDeltaFrameTime(0), m_audioAPI {"fmod"}
{
	g_engine = this;

	static auto registeredGlobals = false;
	if(!registeredGlobals) {
		registeredGlobals = true;
		pragma::console::register_shared_convars(*console::client::get_convar_map());
		register_client_launch_parameters(*GetLaunchParaMap());
		client_entities::register_entities();
		networking::register_client_net_messages();
		pts::register_particle_operators();
	}

	RegisterCallback<void, std::reference_wrapper<const platform::Joystick>, bool>("OnJoystickStateChanged");
	RegisterCallback<void, std::reference_wrapper<std::shared_ptr<prosper::IPrimaryCommandBuffer>>>("DrawFrame");
	RegisterCallback<void>("PreDrawGUI");
	RegisterCallback<void>("PostDrawGUI");
	RegisterCallback<void>("PreRecordGUI");
	RegisterCallback<void>("PostRecordGUI");
	RegisterCallback<void>("Draw");

	RegisterCallbackWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, platform::MouseButton, platform::KeyState, platform::Modifier>("OnMouseInput");
	RegisterCallbackWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, platform::Key, int, platform::KeyState, platform::Modifier, float>("OnKeyboardInput");
	RegisterCallbackWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, unsigned int>("OnCharInput");
	RegisterCallbackWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, Vector2>("OnScrollInput");
	RegisterCallbackWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, std::reference_wrapper<const platform::Joystick>, uint32_t, platform::KeyState>("OnJoystickButtonInput");
	RegisterCallbackWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, std::reference_wrapper<const platform::Joystick>, uint32_t, platform::Modifier, float, float>("OnJoystickAxisInput");

	AddProfilingHandler([this](bool profilingEnabled) {
		if(profilingEnabled == false) {
			m_profilingStageManager = nullptr;
			return;
		}
		auto &cpuProfiler = get_cengine()->GetProfiler();
		m_cpuProfilingStageManager = std::make_unique<debug::ProfilingStageManager<debug::ProfilingStage>>();
		m_cpuProfilingStageManager->InitializeProfilingStageManager(cpuProfiler);
	});
	m_coreInputBindingLayer = pragma::util::make_shared<CoreInputBindingLayer>();
	auto inputLayer = pragma::util::make_shared<InputBindingLayer>();
	inputLayer->identifier = "core";
	AddInputBindingLayer(inputLayer);

	{
		asset::AssetManager::ImporterInfo importerInfo {};
		importerInfo.name = "glTF";
		importerInfo.fileExtensions = {{"gltf", false}, {"glb", true}, {"vrm", true}}; // VRM is based on glTF ( https://vrm.dev/en/ )
		GetAssetManager().RegisterImporter(importerInfo, asset::Type::Model, [](Game &game, ufile::IFile &f, const std::optional<std::string> &mdlPath, std::string &errMsg) -> std::unique_ptr<asset::IAssetWrapper> {
			util::Path path {};
			if(mdlPath.has_value()) {
				path = util::Path::CreateFile(*mdlPath);
				path.PopBack();
			}
			auto mdl = asset::import_model(f, errMsg, path);
			if(mdl == nullptr)
				return nullptr;
			auto wrapper = std::make_unique<asset::ModelAssetWrapper>();
			wrapper->SetModel(*mdl);
			return wrapper;
		});
	}

	{
		asset::AssetManager::ImporterInfo importerInfo {};
		importerInfo.name = "fbx";
		importerInfo.fileExtensions = {{"fbx", true}};
		GetAssetManager().RegisterImporter(importerInfo, asset::Type::Model, [](Game &game, ufile::IFile &f, const std::optional<std::string> &mdlPath, std::string &errMsg) -> std::unique_ptr<asset::IAssetWrapper> {
			util::Path path {};
			if(mdlPath.has_value()) {
				path = util::Path::CreateFile(*mdlPath);
				path.PopBack();
			}
			auto result = asset::import_fbx(f, errMsg, path);
			if(!result)
				return nullptr;
			if(result->modelObjects.empty()) {
				errMsg = "No models found in model file!";
				return nullptr;
			}

			auto wrapper = std::make_unique<asset::ModelAssetWrapper>();
			wrapper->SetModel(*result->modelObjects.front());
			return wrapper;
		});
	}
}

void pragma::CEngine::Release()
{
	Close();
	Engine::Release();
	RenderContext::Release();
}

pragma::debug::GPUProfiler &pragma::CEngine::GetGPUProfiler() const { return *m_gpuProfiler; }
pragma::debug::ProfilingStageManager<pragma::debug::GPUProfilingStage> *pragma::CEngine::GetGPUProfilingStageManager() { return m_gpuProfilingStageManager.get(); }
pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage> *pragma::CEngine::GetProfilingStageManager() { return m_cpuProfilingStageManager.get(); }

static auto cvGPUProfiling = pragma::console::get_client_con_var("cl_gpu_timer_queries_enabled");
bool pragma::CEngine::IsGPUProfilingEnabled() const { return cvGPUProfiling->GetBool(); }

void pragma::CEngine::DumpDebugInformation(uzip::ZIPFile &zip) const
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
		ss << "Vendor ID: " << math::to_integral(deviceInfo.vendor);
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

	auto fWriteLuaTraceback = [&zip](lua::State *l, const std::string &identifier) {
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

	auto &context = get_cengine()->GetRenderContext();
	if(!context.IsClosed()) {
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
}

void pragma::CEngine::SetRenderResolution(std::optional<Vector2i> resolution)
{
	if(m_renderResolution == resolution)
		return;
	m_renderResolution = resolution;

	resolution = GetRenderResolution();
	OnRenderResolutionChanged(resolution->x, resolution->y);
}
Vector2i pragma::CEngine::GetRenderResolution() const
{
	if(m_renderResolution.has_value())
		return *m_renderResolution;
	return GetRenderContext().GetWindow()->GetSize();
}

double pragma::CEngine::GetFPS() const { return m_fps; }
double pragma::CEngine::GetFrameTime() const { return m_tFPSTime * 1'000.0; }
Double pragma::CEngine::GetDeltaFrameTime() const { return util::clock::to_seconds(m_tDeltaFrameTime); }

static auto cvFrameLimit = pragma::console::get_client_con_var("cl_max_fps");
float pragma::CEngine::GetFPSLimit() const { return cvFrameLimit->GetFloat(); }

unsigned int pragma::CEngine::GetStereoSourceCount() { return 0; }
unsigned int pragma::CEngine::GetMonoSourceCount() { return 0; }
unsigned int pragma::CEngine::GetStereoSource(unsigned int idx) { return 0; }
float pragma::CEngine::GetNearZ() { return m_nearZ; }
float pragma::CEngine::GetFarZ() { return m_farZ; }

bool pragma::CEngine::IsClientConnected()
{
	auto *cl = static_cast<ClientState *>(GetClientState());
	if(cl == nullptr)
		return false;
	return cl->IsConnected();
}

void pragma::CEngine::EndGame()
{
	Disconnect();
	auto *cl = GetClientState();
	if(cl != nullptr)
		cl->EndGame();
	Engine::EndGame();
}

void pragma::CEngine::Input(int key, platform::KeyState inputState, platform::KeyState pressState, platform::Modifier mods, float magnitude)
{
	if(inputState == platform::KeyState::Press || inputState == platform::KeyState::Release || inputState == platform::KeyState::Held) {
		auto &inputLayer = GetEffectiveInputBindingLayer();
		auto &keyMappings = inputLayer.GetKeyMappings();
		if((mods & platform::Modifier::AxisNegative) != platform::Modifier::None) {
			// We need to check if there are any keybindings with a command with the JoystickAxisSingle flag set,
			// in which case that keybinding has priority
			auto keyPositive = key - 1;
			auto it = keyMappings.find(CInt16(keyPositive));
			if(it != keyMappings.end() && const_cast<KeyBind &>(it->second).Execute(inputState, pressState, mods, magnitude) == true)
				return;
			mods &= ~platform::Modifier::AxisNegative;
		}
		auto it = keyMappings.find(CInt16(key));
		if(it != keyMappings.end())
			const_cast<KeyBind &>(it->second).Execute(inputState, pressState, mods, magnitude);
	}
}
void pragma::CEngine::Input(int key, platform::KeyState state, platform::Modifier mods, float magnitude) { Input(key, state, state, mods, magnitude); }
void pragma::CEngine::MouseInput(prosper::Window &window, platform::MouseButton button, platform::KeyState state, platform::Modifier mods)
{
	auto handled = false;
	if(CallCallbacksWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, platform::MouseButton, platform::KeyState, platform::Modifier>("OnMouseInput", handled, window, button, state, mods) == CallbackReturnType::HasReturnValue && handled == true)
		return;
	auto *client = get_client_state();
	if(client != nullptr && client->RawMouseInput(button, state, mods) == false)
		return;
	if(gui::WGUI::GetInstance().HandleMouseInput(window, button, state, mods))
		return;
	button = static_cast<platform::MouseButton>(math::to_integral(button) + math::to_integral(platform::Key::Last));
	if(client != nullptr && client->MouseInput(button, state, mods) == false)
		return;
	Input(static_cast<int>(button), state);
}
void pragma::CEngine::GetMappedKeys(const std::string &cvarName, std::vector<platform::Key> &keys, uint32_t maxKeys)
{
	if(maxKeys != std::numeric_limits<uint32_t>::max())
		keys.reserve(maxKeys);
	if(maxKeys == 0)
		return;
	std::string cmd;
	std::vector<std::string> argv {};
	string::get_sequence_commands(cvarName, [&cmd, &argv](std::string cmdOther, std::vector<std::string> argvOther) {
		cmd = cmdOther;
		argv = argvOther;
	});
	for(auto &layer : GetInputBindingLayers()) {
		auto &keyMappings = layer->GetKeyMappings();
		for(auto &pair : keyMappings) {
			auto &keyBind = pair.second;
			auto bFoundCvar = false;
			string::get_sequence_commands(keyBind.GetBind(), [&cmd, &argv, &bFoundCvar](std::string cmdOther, std::vector<std::string> &argvOther) {
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
				keys.push_back(static_cast<platform::Key>(pair.first));
				if(keys.size() == maxKeys)
					break;
			}
		}
	}
}
void pragma::CEngine::JoystickButtonInput(prosper::Window &window, const platform::Joystick &joystick, uint32_t key, platform::KeyState state)
{
	auto handled = false;
	if(CallCallbacksWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, std::reference_wrapper<const platform::Joystick>, uint32_t, platform::KeyState>("OnJoystickButtonInput", handled, window, joystick, key, state) == CallbackReturnType::HasReturnValue && handled == true)
		return;
	KeyboardInput(window, static_cast<platform::Key>(key), -1, state, {});
}
void pragma::CEngine::JoystickAxisInput(prosper::Window &window, const platform::Joystick &joystick, uint32_t axis, platform::Modifier mods, float newVal, float deltaVal)
{
	auto handled = false;
	if(CallCallbacksWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, std::reference_wrapper<const platform::Joystick>, uint32_t, platform::Modifier, float, float>("OnJoystickAxisInput", handled, window, joystick, axis, mods, newVal, deltaVal)
	    == CallbackReturnType::HasReturnValue
	  && handled == true)
		return;
	auto oldVal = newVal - deltaVal;
	auto key = static_cast<platform::Key>(axis);
	auto state = (IsValidAxisInput(newVal) == true) ? platform::KeyState::Press : platform::KeyState::Release;
	auto it = m_joystickKeyStates.find(key);
	auto oldState = (it == m_joystickKeyStates.end()) ? platform::KeyState::Release : it->second;
	if(state == platform::KeyState::Release && oldState == platform::KeyState::Release)
		return;
	if(state == platform::KeyState::Press && oldState == platform::KeyState::Press)
		state = platform::KeyState::Held;

	m_joystickKeyStates[key] = state;
	mods |= platform::Modifier::AxisInput;
	if(math::abs(newVal) > AXIS_PRESS_THRESHOLD) {
		if(math::abs(oldVal) <= AXIS_PRESS_THRESHOLD)
			mods |= platform::Modifier::AxisPress; // Axis represents actual button press
	}
	else if(math::abs(oldVal) > AXIS_PRESS_THRESHOLD)
		mods |= platform::Modifier::AxisRelease; // Axis represents actual button release
	KeyboardInput(window, key, -1, state, mods, newVal);
}
static auto cvAxisInputThreshold = pragma::console::get_client_con_var("cl_controller_axis_input_threshold");
bool pragma::CEngine::IsValidAxisInput(float axisInput) const
{
	if(!get_client_state())
		return false;
	return (math::abs(axisInput) > cvAxisInputThreshold->GetFloat()) ? true : false;
}

bool pragma::CEngine::GetInputButtonState(float axisInput, platform::Modifier mods, platform::KeyState &inOutState) const
{
	if(IsValidAxisInput(axisInput) == false) {
		if((mods & platform::Modifier::AxisInput) != platform::Modifier::None) {
			inOutState = platform::KeyState::Release;
			return true;
		}
		inOutState = platform::KeyState::Invalid;
		return false;
	}
	if((mods & platform::Modifier::AxisInput) == platform::Modifier::None)
		return true; // No need to change state

	if((mods & platform::Modifier::AxisPress) != platform::Modifier::None)
		inOutState = platform::KeyState::Press;
	else if((mods & platform::Modifier::AxisRelease) != platform::Modifier::None)
		inOutState = platform::KeyState::Release;
	else {
		inOutState = platform::KeyState::Invalid;
		return false; // Not an actual key press
	}
	return true;
}
void pragma::CEngine::KeyboardInput(prosper::Window &window, platform::Key key, int scanCode, platform::KeyState state, platform::Modifier mods, float magnitude)
{
	auto handled = false;
	if(CallCallbacksWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, platform::Key, int, platform::KeyState, platform::Modifier, float>("OnKeyboardInput", handled, window, key, scanCode, state, mods, magnitude) == CallbackReturnType::HasReturnValue && handled == true)
		return;
	auto *client = get_client_state();
	if(client != nullptr && client->RawKeyboardInput(key, scanCode, state, mods, magnitude) == false)
		return;
	if(key == platform::Key::Escape) // Escape key is hardcoded
	{
		if(client != nullptr) {
			if(state == platform::KeyState::Press)
				client->ToggleMainMenu();
			return;
		}
	}
	if(key == platform::Key::GraveAccent) {
		if(mods == platform::Modifier::None) {
			if(state == platform::KeyState::Press)
				ToggleConsole();
		}
		return;
	}
	auto buttonState = state;
	auto bValidButtonInput = GetInputButtonState(magnitude, mods, buttonState);
	if(bValidButtonInput == true) {
		if(gui::WGUI::GetInstance().HandleKeyboardInput(window, key, scanCode, buttonState, mods))
			return;
	}
	if(client != nullptr && client->KeyboardInput(key, scanCode, state, mods, magnitude) == false)
		return;
	auto ikey = math::to_integral(key);
	if(ikey >= math::to_integral(platform::Key::A) && ikey <= math::to_integral(platform::Key::Z))
		key = static_cast<platform::Key>(std::tolower(ikey));
	Input(math::to_integral(key), state, buttonState, mods, magnitude);
}
void pragma::CEngine::CharInput(prosper::Window &window, unsigned int c)
{
	auto handled = false;
	if(CallCallbacksWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, unsigned int>("OnCharInput", handled, window, c) == CallbackReturnType::HasReturnValue && handled == true)
		return;
	auto *client = get_client_state();
	if(client != nullptr && client->RawCharInput(c) == false)
		return;
	if(gui::WGUI::GetInstance().HandleCharInput(window, c))
		return;
	if(client != nullptr && client->CharInput(c) == false)
		return;
}
void pragma::CEngine::ScrollInput(prosper::Window &window, Vector2 offset)
{
	auto handled = false;
	if(CallCallbacksWithOptionalReturn<bool, std::reference_wrapper<prosper::Window>, Vector2>("OnScrollInput", handled, window, offset) == CallbackReturnType::HasReturnValue && handled == true)
		return;
	auto *client = get_client_state();
	if(client != nullptr && client->RawScrollInput(offset) == false)
		return;
	if(gui::WGUI::GetInstance().HandleScrollInput(window, offset))
		return;
	if(client != nullptr && client->ScrollInput(offset) == false)
		return;
	if(offset.y >= 0.f) {
		Input(GLFW_CUSTOM_KEY_SCRL_UP, platform::KeyState::Press);
		Input(GLFW_CUSTOM_KEY_SCRL_UP, platform::KeyState::Release);
	}
	else {
		Input(GLFW_CUSTOM_KEY_SCRL_DOWN, platform::KeyState::Press);
		Input(GLFW_CUSTOM_KEY_SCRL_DOWN, platform::KeyState::Release);
	}
}

void pragma::CEngine::OnWindowFocusChanged(prosper::Window &window, bool bFocused)
{
	math::set_flag(m_stateFlags, StateFlags::WindowFocused, bFocused);
	auto *client = get_client_state();
	if(client != nullptr)
		client->UpdateSoundVolume();
}

// Usually we don't allow opening external files, but we make an exception for files that have been dropped into Pragma.
static std::unordered_map<std::string, std::string> g_droppedFiles;
namespace pragma {
	DLLCLIENT const std::unordered_map<std::string, std::string> &get_dropped_files() { return g_droppedFiles; }
};
const std::vector<pragma::CEngine::DroppedFile> &pragma::CEngine::GetDroppedFiles() const { return m_droppedFiles; }
void pragma::CEngine::OnFilesDropped(prosper::Window &window, std::vector<std::string> &files)
{
	auto *client = get_client_state();
	if(client != nullptr)
		return;

	m_droppedFiles.reserve(files.size());
	auto addFile = [this](const std::string &fileName, const std::string &rootPath) {
		m_droppedFiles.push_back(DroppedFile {rootPath, fileName});
		auto path = util::Path::CreateFile(fileName).GetString();
		string::to_lower(path);
		g_droppedFiles.insert(std::make_pair(ufile::get_file_from_filename(path), path));
	};
	std::function<void(const std::vector<std::string> &, const std::optional<std::string> &)> addFiles = nullptr;
	addFiles = [this, &addFile, &addFiles](const std::vector<std::string> &files, const std::optional<std::string> &rootPath) {
		for(auto &f : files) {
			if(fs::is_system_file(f))
				addFile(f, rootPath ? *rootPath : ufile::get_path_from_filename(f));
			else if(fs::is_system_dir(f)) {
				auto subRootPath = rootPath;
				if(!subRootPath) {
					auto path = util::Path::CreatePath(f);
					path.PopBack();
					subRootPath = path.GetString();
				}
				std::vector<std::string> subFiles;
				std::vector<std::string> subDirs;
				fs::find_system_files(f + "/*", &subFiles, &subDirs);
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
	if(gui::WGUI::GetInstance().HandleFileDrop(window, droppedFileNames))
		return;
	client->OnFilesDropped(files);
}
void pragma::CEngine::OnDragEnter(prosper::Window &window)
{
	auto *client = get_client_state();
	if(client != nullptr)
		return;
	if(gui::WGUI::GetInstance().HandleFileDragEnter(window))
		return;
	client->OnDragEnter(window);
}
void pragma::CEngine::OnDragExit(prosper::Window &window)
{
	auto *client = get_client_state();
	if(client != nullptr)
		return;
	if(gui::WGUI::GetInstance().HandleFileDragExit(window))
		return;
	client->OnDragExit(window);
}
bool pragma::CEngine::OnWindowShouldClose(prosper::Window &window)
{
	auto *client = get_client_state();
	if(client != nullptr)
		return true;
	return client->OnWindowShouldClose(window);
}
void pragma::CEngine::OnPreedit(prosper::Window &window, const string::Utf8String &preeditString, const std::vector<int> &blockSizes, int focusedBlock, int caret)
{
	auto *client = get_client_state();
	if(client != nullptr)
		return;
	client->OnPreedit(window, preeditString, blockSizes, focusedBlock, caret);
}
void pragma::CEngine::OnIMEStatusChanged(prosper::Window &window, bool imeEnabled)
{
	auto *client = get_client_state();
	if(client != nullptr)
		return;
	gui::WGUI::GetInstance().HandleIMEStatusChanged(window, imeEnabled);
	client->OnIMEStatusChanged(window, imeEnabled);
}
bool pragma::CEngine::IsWindowFocused() const { return math::is_flag_set(m_stateFlags, StateFlags::WindowFocused); }

void pragma::CEngine::SetAssetMultiThreadedLoadingEnabled(bool enabled)
{
	Engine::SetAssetMultiThreadedLoadingEnabled(enabled);
	auto *cl = GetClientState();
	if(cl) {
		auto &mdlManager = cl->GetModelManager();
		mdlManager.GetLoader().SetMultiThreadingEnabled(enabled);
		auto &matManager = static_cast<material::CMaterialManager &>(cl->GetMaterialManager());
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
extern bool g_cpuRendering;
void register_game_shaders();

bool pragma::CEngine::IsWindowless() const { return g_windowless; }
bool pragma::CEngine::IsCPURenderingOnly() const { return g_cpuRendering; }
bool pragma::CEngine::IsClosed() const { return math::is_flag_set(m_stateFlags, StateFlags::CEClosed); }

void pragma::CEngine::HandleOpenGLFallback()
{
	if(string::compare(GetRenderAPI(), std::string {"opengl"}, false))
		return;
	auto *cl = static_cast<ClientState *>(GetClientState());
	if(!cl)
		return;
	auto msg = locale::get_text("prompt_fallback_to_opengl");
	if(pragma::debug::show_message_prompt(msg, debug::MessageBoxButtons::YesNo, util::get_program_name()) != debug::MessageBoxButton::Yes)
		return;
	cl->SetConVar("render_api", "opengl");
	SaveClientConfig();
	ShutDown();
	util::CommandInfo cmdInfo;
	cmdInfo.command = util::get_program_name();
	cmdInfo.absoluteCommandPath = false;
	pragma::util::start_process(cmdInfo);
}

std::optional<std::string> g_waylandLibdecorPlugin;
extern bool g_cli;
bool pragma::CEngine::Initialize(int argc, char *argv[])
{
	Engine::Initialize(argc, argv);
	SetCLIOnly(g_cli);

#ifdef __linux__
	auto xdgSessionType = util::get_env_variable("XDG_SESSION_TYPE");
	if(!xdgSessionType || *xdgSessionType != "x11") {
		if(!g_waylandLibdecorPlugin)
			g_waylandLibdecorPlugin = "gtk";
		if(g_waylandLibdecorPlugin) {
			// Note: Using cairo plugin with wayland will likely crash on startup
			if(*g_waylandLibdecorPlugin == "cairo")
				Con::CWAR << "Using libdecor cairo plugin may crash on startup!" << Con::endl;
			util::set_env_variable("GDK_BACKEND", "wayland");

			auto path = util::FilePath(util::get_program_path(), "modules/graphics/vulkan/libdecor/plugins", *g_waylandLibdecorPlugin);
			util::set_env_variable("LIBDECOR_PLUGIN_DIR", path.GetString());
		}
	}
#endif

	if(Lua::get_extended_lua_modules_enabled())
		RegisterConCommand("lc", [this](NetworkState *, BasePlayerComponent *, std::vector<std::string> &argv, float) { RunConsoleCommand("lua_run_cl", argv); });
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
		util::sleep_for_seconds(5);
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
		Con::CWAR << "Failed to load render API data: " << e.what() << Con::endl;
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
			if(fs::find_absolute_path(relFilePath, strFilePath))
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
		string::explode(*windowRes, "x", vals);
		if(vals.size() >= 2) {
			contextCreateInfo.width = util::to_int(vals[0]);
			contextCreateInfo.height = util::to_int(vals[1]);
		}
	}
	// SetResolution(Vector2i(contextCreateInfo.width,contextCreateInfo.height));

	auto renderRes = findCmdArg("cl_render_resolution");
	if(renderRes) {
		std::vector<std::string> vals;
		string::explode(*renderRes, "x", vals);
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
		auto monitors = platform::get_monitors();
		if(monitor < monitors.size() && monitor > 0)
			initialWindowSettings.monitor = monitors[monitor];
	}

	auto gpuDevice = findCmdArg("cl_gpu_device");
	if(gpuDevice) {
		auto device = *gpuDevice;
		std::vector<std::string> subStrings;
		string::explode(device, ",", subStrings);
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
		pragma::math::swap(tmp.r, tmp.b);
		auto hex = tmp.ToHexColorRGB();
		COLORREF hexCol = pragma::math::to_hex_number("0x" + hex);
		const DWORD ATTR_CAPTION_COLOR = 35; // See DWMWINDOWATTRIBUTE::DWMWA_CAPTION_COLOR, can't use the enum because it may not be available and there's no way to check for it
		DwmSetWindowAttribute(h, ATTR_CAPTION_COLOR, &hexCol, sizeof(hexCol));
	}
	if(g_borderColor.has_value()) {
		auto tmp = *g_borderColor;
		pragma::math::swap(tmp.r, tmp.b);
		auto hex = tmp.ToHexColorRGB();
		COLORREF hexCol = pragma::math::to_hex_number("0x" + hex);
		const DWORD ATTR_BORDER_COLOR = 34; // See DWMWINDOWATTRIBUTE::DWMWA_BORDER_COLOR, can't use the enum because it may not be available and there's no way to check for it
		DwmSetWindowAttribute(h, ATTR_BORDER_COLOR, &hexCol, sizeof(hexCol));
	}
#endif
#endif

	auto &shaderManager = GetRenderContext().GetShaderManager();
	shaderManager.RegisterShader("clear_color", [](prosper::IPrContext &context, const std::string &identifier) { return new ShaderClearColor(context, identifier); });
	shaderManager.RegisterShader("gradient", [](prosper::IPrContext &context, const std::string &identifier) { return new ShaderGradient(context, identifier); });

	shaderManager.GetShader("blur_horizontal");
	shaderManager.GetShader("blur_vertical");

	// Initialize Client Instance
	auto matManager = material::CMaterialManager::Create(GetRenderContext());
	matManager->SetImportDirectory("addons/converted/");
	InitializeAssetManager(*matManager);
	pragma::asset::update_extension_cache(asset::Type::Material);

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
	auto *lanInfo = locale::get_language_info();
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
		util::sleep_for_seconds(5);
		return false;
	};

	if(!FindFontSet(defaultFontSet)) {
		spdlog::error("Failed to find default font set '{}'!", defaultFontSet);
		fail();
		return false;
	}

	auto &fontSet = GetDefaultFontSet();
	auto &gui = gui::WGUI::Open(GetRenderContext(), matManager);
	RegisterUiElementTypes();
	gui.SetMaterialLoadHandler([this](const std::string &path) -> material::Material * { return GetClientState()->LoadMaterial(path); });
	auto *fontData = fontSet.FindFontFileCandidate(FontSetFlag::Sans | FontSetFlag::Bold);
	if(!fontData) {
		spdlog::error("Failed to determine default font for font set '{}'!", defaultFontSet);
		fail();
		return false;
	}
	auto r = gui.Initialize(GetRenderResolution(), fontData->fileName, {"source-han-sans/SourceHanSans-VF.ttf"});
	if(r != gui::WGUI::ResultCode::Ok) {
		Con::CERR << "Unable to initialize GUI library: ";
		switch(r) {
		case gui::WGUI::ResultCode::UnableToInitializeFontManager:
			Con::CERR << "Error initializing font manager!";
			break;
		case gui::WGUI::ResultCode::ErrorInitializingShaders:
			Con::CERR << "Error initializing shaders!";
			break;
		case gui::WGUI::ResultCode::FontNotFound:
			Con::CERR << "Font not found!";
			break;
		default:
			Con::COUT << "Unknown error!";
			break;
		}
		fail();
		return false;
	}
	gui::types::WIContextMenu::SetKeyBindHandler(
	  [this](platform::Key key, const std::string &cmd) -> std::string {
		  std::string keyStr;
		  auto b = KeyToText(math::to_integral(key), &keyStr);
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
		  std::vector<platform::Key> keys;
		  GetMappedKeys(cmd, keys);
		  if(keys.empty())
			  return {};
		  std::string strKey {};
		  KeyToText(math::to_integral(keys.front()), &strKey);
		  return strKey;
	  });
	gui::WITextTagLink::set_link_handler([this](const std::string &arg) {
		std::vector<std::string> args {};
		string::explode_whitespace(arg, args);
		if(args.empty())
			return;
		auto cmd = args.front();
		args.erase(args.begin());
		if(pragma::string::compare<std::string>(cmd, "url", false)) {
			if(args.empty() == false)
				util::open_url_in_browser(args.front());
			return;
		}
		RunConsoleCommand(cmd, args);
	});

	m_speedCam = 1600.0f;
	m_speedCamMouse = 0.2f;

	GetRenderContext().GetWindow().ReloadStagingRenderTarget();
	math::set_flag(m_stateFlags, StateFlags::FirstFrame);

	m_gpuProfiler = debug::GPUProfiler::Create<debug::GPUProfiler>();
	AddGPUProfilingHandler([this](bool profilingEnabled) {
		if(profilingEnabled == false) {
			m_gpuProfilingStageManager = nullptr;
			return;
		}
		m_gpuProfilingStageManager = std::make_unique<debug::ProfilingStageManager<debug::GPUProfilingStage>>();
		auto &gpuProfiler = *m_gpuProfiler;
		m_gpuProfilingStageManager->InitializeProfilingStageManager(gpuProfiler);
	});

	InitializeSoundEngine();

	OpenClientState();
	register_game_shaders(); // Preload game shaders

	if(math::is_flag_set(m_stateFlags, StateFlags::ConsoleOpen))
		OpenConsole(); // GUI Console mustn't be opened before client has been created!

#ifdef _WIN32
	if(GetRenderContext().IsValidationEnabled()) {
		if(pragma::util::is_process_running("bdcam.exe")) {
			auto r = MessageBox(nullptr, "Bandicam is running and vulkan validation mode is enabled. This is NOT recommended, as Bandicam will cause misleading validation errors! Press OK to continue anyway.", "Validation Warning", MB_OK | MB_OKCANCEL);
			if(r == IDCANCEL)
				ShutDown();
		}
	}
	else if(pragma::util::is_process_running("bdcam.exe")) {
		auto r = MessageBox(nullptr, "Detected Bandicam running in the background, this can cause crashes and/or freezing! Please close Bandicam and restart Pragma. You can restart Bandicam after Pragma has been started, but it mustn't be running before then.", "Bandicam Warning",
		  MB_OK | MB_OKCANCEL);
		if(r == IDCANCEL)
			ShutDown();
	}

#endif

	{
		auto regBase = pragma::util::make_shared<shadergraph::NodeRegistry>();
		//regBase->RegisterNode<pragma::shadergraph::BlackbodyNode>("blackbody");
		regBase->RegisterNode<shadergraph::BrightContrastNode>("bright_contrast");
		regBase->RegisterNode<shadergraph::ClampNode>("clamp");
		//regBase->RegisterNode<pragma::shadergraph::ColorNode>("color");
		regBase->RegisterNode<shadergraph::CombineXyzNode>("combine_xyz");
		regBase->RegisterNode<shadergraph::GammaNode>("gamma");
		//regBase->RegisterNode<pragma::shadergraph::GroupNode>("group");
		regBase->RegisterNode<shadergraph::EmissionNode>("emission");
		//regBase->RegisterNode<pragma::shadergraph::GroupInputNode>("group_input");
		//regBase->RegisterNode<pragma::shadergraph::GroupOutputNode>("group_output");
		regBase->RegisterNode<shadergraph::HsvNode>("hsv");
		regBase->RegisterNode<shadergraph::InvertNode>("invert");
		//regBase->RegisterNode<pragma::shadergraph::LightFalloffNode>("light_falloff");
		regBase->RegisterNode<shadergraph::MapRangeNode>("map_range_node");
		//regBase->RegisterNode<pragma::shadergraph::MappingNode>("mapping");
		regBase->RegisterNode<shadergraph::MathNode>("math");
		regBase->RegisterNode<shadergraph::MixNode>("mix");
		//regBase->RegisterNode<pragma::shadergraph::RgbRampNode>("rgb_ramp");
		regBase->RegisterNode<shadergraph::RgbToBwNode>("rgb_to_bw");
		regBase->RegisterNode<shadergraph::SepiaToneNode>("sepia_tone");
		regBase->RegisterNode<shadergraph::SeparateXyzNode>("separate_xyz");
		regBase->RegisterNode<shadergraph::ValueNode>("value");
		//regBase->RegisterNode<pragma::shadergraph::VectorDisplacementNode>("vector_displacement");
		//regBase->RegisterNode<pragma::shadergraph::VectorMapRangeNode>("vector_map_range");
		regBase->RegisterNode<shadergraph::VectorMathNode>("vector_math");
		//regBase->RegisterNode<pragma::shadergraph::VectorRotateNode>("vector_rotate");
		//regBase->RegisterNode<pragma::shadergraph::VectorTransformNode>("vector_transform");
		//regBase->RegisterNode<pragma::shadergraph::WavelengthNode>("wavelength");

		auto regScene = pragma::util::make_shared<shadergraph::NodeRegistry>();
		regScene->RegisterNode<rendering::shader_graph::SceneOutputNode>("output");
		regScene->RegisterNode<rendering::shader_graph::CameraNode>("camera");
		regScene->RegisterNode<rendering::shader_graph::FogNode>("fog");
		regScene->RegisterNode<rendering::shader_graph::LightmapNode>("lightmap");
		regScene->RegisterNode<rendering::shader_graph::ObjectNode>("object");
		regScene->RegisterNode<rendering::shader_graph::TimeNode>("time");
		regScene->RegisterNode<rendering::shader_graph::PbrNode>("pbr");
		regScene->RegisterNode<rendering::shader_graph::ToonNode>("toon");
		regScene->RegisterNode<rendering::shader_graph::ImageTextureNode>("image_texture");
		regScene->RegisterNode<rendering::shader_graph::MaterialTextureNode>("material_texture");
		regScene->RegisterNode<rendering::shader_graph::TextureCoordinateNode>("texture_coordinate");
		regScene->RegisterNode<rendering::shader_graph::VectorTransformNode>("vector_transform");
		regScene->RegisterNode<rendering::shader_graph::GeometryNode>("geometry");
		regScene->RegisterNode<rendering::shader_graph::InputParameterBooleanNode>("input_parameter_boolean");
		regScene->RegisterNode<rendering::shader_graph::InputParameterIntNode>("input_parameter_int");
		regScene->RegisterNode<rendering::shader_graph::InputParameterUIntNode>("input_parameter_uint");
		regScene->RegisterNode<rendering::shader_graph::InputParameterUInt16Node>("input_parameter_uint16");
		regScene->RegisterNode<rendering::shader_graph::InputParameterColorNode>("input_parameter_color");
		regScene->RegisterNode<rendering::shader_graph::InputParameterVectorNode>("input_parameter_vector");
		regScene->RegisterNode<rendering::shader_graph::InputParameterVector4Node>("input_parameter_vector4");
		regScene->RegisterNode<rendering::shader_graph::InputParameterPointNode>("input_parameter_point");
		regScene->RegisterNode<rendering::shader_graph::InputParameterNormalNode>("input_parameter_normal");
		regScene->RegisterNode<rendering::shader_graph::InputParameterPoint2Node>("input_parameter_point2");
		regScene->RegisterNode<rendering::shader_graph::InputParameterStringNode>("input_parameter_string");
		regScene->RegisterNode<rendering::shader_graph::InputParameterTransformNode>("input_parameter_transform");
		regScene->RegisterNode<rendering::shader_graph::InputParameterFloatNode>("input_parameter_float");
		regScene->RegisterNode<rendering::shader_graph::InputParameterTextureNode>("input_parameter_texture");

		std::vector<std::string> files;
		fs::find_files("scripts/shader_data/materials/*.udm", &files, nullptr);
		for(auto &f : files) {
			ufile::remove_extension_from_filename(f);
			rendering::shader_material::get_cache().Load(f);
		}

		auto &cache = rendering::shader_material::get_cache();
		for(auto &[name, mat] : cache.GetShaderMaterials()) {
			auto nodeName = name;
			nodeName = "sm_" + nodeName;
			auto node = pragma::util::make_shared<rendering::shader_graph::ShaderMaterialNode>(GString {nodeName}, *mat);
			regScene->RegisterNode(node);
		}

		regScene->AddChildRegistry(regBase);

		auto regPp = pragma::util::make_shared<shadergraph::NodeRegistry>();
		regPp->AddChildRegistry(regBase);

		m_shaderGraphManager = std::make_unique<rendering::ShaderGraphManager>();
		m_shaderGraphManager->RegisterGraphTypeManager("post_processing", regPp);
		m_shaderGraphManager->RegisterGraphTypeManager("object", regScene);
		//m_shaderGraphManager->RegisterGraphTypeManager("group", regGroup);

		m_shaderGraphManager->GetModuleManager().RegisterFactory("pbr", [](ShaderGraph &shader) -> std::unique_ptr<rendering::ShaderGraphModule> { return std::make_unique<rendering::shader_graph::PbrModule>(shader); });
		m_shaderGraphManager->GetModuleManager().RegisterFactory("toon", [](ShaderGraph &shader) -> std::unique_ptr<rendering::ShaderGraphModule> { return std::make_unique<rendering::shader_graph::ToonModule>(shader); });
		m_shaderGraphManager->GetModuleManager().RegisterFactory("image_texture", [](ShaderGraph &shader) -> std::unique_ptr<rendering::ShaderGraphModule> { return std::make_unique<rendering::shader_graph::ImageTextureModule>(shader); });
		m_shaderGraphManager->GetModuleManager().RegisterFactory("input_data", [](ShaderGraph &shader) -> std::unique_ptr<rendering::ShaderGraphModule> { return std::make_unique<rendering::shader_graph::InputDataModule>(shader); });

		// TODO: Load shader graphs from mounted addons
		for(auto &[type, man] : m_shaderGraphManager->GetShaderGraphTypeManagers()) {
			std::vector<std::string> sgFiles;
			fs::find_files(rendering::ShaderGraphManager::ROOT_GRAPH_PATH + type + std::string {"/*."} + shadergraph::Graph::EXTENSION_BINARY, &sgFiles, nullptr);
			fs::find_files(rendering::ShaderGraphManager::ROOT_GRAPH_PATH + type + std::string {"/*."} + shadergraph::Graph::EXTENSION_ASCII, &sgFiles, nullptr);
			for(auto &f : sgFiles) {
				ufile::remove_extension_from_filename(f, std::array<std::string, 2> {shadergraph::Graph::EXTENSION_BINARY, shadergraph::Graph::EXTENSION_ASCII});
				std::string err;
				auto graph = m_shaderGraphManager->LoadShader(f, err);
				if(!graph)
					spdlog::error("Failed to load shader graph '{}': {}", f, err);
			}
		}
	}

	return true;
}

void pragma::CEngine::RegisterUiElementTypes()
{
	auto &gui = gui::WGUI::GetInstance();
	gui.RegisterType<gui::types::WICheckbox>("WICheckbox");
	gui.RegisterType<gui::types::WIChoiceList>("WIChoiceList");
	gui.RegisterType<gui::types::WICommandLineEntry>("WICommandLineEntry");
	gui.RegisterType<gui::types::WIConsole>("WIConsole");
	gui.RegisterType<gui::types::WIContainer>("WIContainer");
	gui.RegisterType<gui::types::WIDetachable>("WIDetachable");
	gui.RegisterType<gui::types::WIFPS>("WIFPS");
	gui.RegisterType<gui::types::WIFrame>("WIFrame");
	gui.RegisterType<gui::types::WIGridPanel>("WIGridPanel");
	gui.RegisterType<gui::types::WIIcon>("WIIcon");
	gui.RegisterType<gui::types::WIImageSlideShow>("WIImageSlideShow");
	gui.RegisterType<gui::types::WILineGraph>("WILineGraph");
	gui.RegisterType<gui::types::WIMessageBox>("WIMessageBox");
	gui.RegisterType<gui::types::WINetGraph>("WINetGraph");
	gui.RegisterType<gui::types::WIOptionsList>("WIOptionsList");
	gui.RegisterType<gui::types::WIProgressBar>("WIProgressBar");
	gui.RegisterType<gui::types::WIScrollContainer>("WIScrollContainer");
	gui.RegisterType<gui::types::WIServerBrowser>("WIServerBrowser");
	gui.RegisterType<gui::types::WISilkIcon>("WISilkIcon");
	gui.RegisterType<gui::types::WISlider>("WISlider");
	gui.RegisterType<gui::types::WISnapArea>("WISnapArea");
	gui.RegisterType<gui::types::WITable>("WITable");
	gui.RegisterType<gui::types::WITableRow>("WITableRow");
	gui.RegisterType<gui::types::WITableCell>("WITableCell");
	gui.RegisterType<gui::types::WITexturedCubemap>("WITexturedCubemap");
	gui.RegisterType<gui::types::WITransformable>("WITransformable");
	gui.RegisterType<gui::types::WITreeList>("WITreeList");
	gui.RegisterType<gui::types::WITreeListElement>("WITreeListElement");
	gui.RegisterType<gui::types::WIDebugDepthTexture>("WIDebugDepthTexture");
	gui.RegisterType<gui::types::WIDebugHDRBloom>("WIDebugHDRBloom");
	gui.RegisterType<gui::types::WIDebugMipMaps>("WIDebugMipMaps");
	gui.RegisterType<gui::types::WIDebugMSAATexture>("WIDebugMSAATexture");
	gui.RegisterType<gui::types::WIDebugShadowMap>("WIDebugShadowMap");
	gui.RegisterType<gui::types::WIDebugSSAO>("WIDebugSSAO");
	gui.RegisterType<gui::types::WIMainMenuElement>("WIMainMenuElement");
}

const std::string &pragma::CEngine::GetDefaultFontSetName() const { return m_defaultFontSet; }
const FontSet &pragma::CEngine::GetDefaultFontSet() const
{
	auto *fs = FindFontSet(m_defaultFontSet);
	assert(fs != nullptr);
	return *fs;
}
const FontSet *pragma::CEngine::FindFontSet(const std::string &name) const
{
	auto it = m_fontSets.find(name);
	return (it != m_fontSets.end()) ? it->second.get() : nullptr;
}
void pragma::CEngine::LoadFontSets()
{
	std::vector<std::string> dirs;
	fs::find_files("fonts/*", nullptr, &dirs);
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
void pragma::CEngine::RunLaunchCommands()
{
	Engine::RunLaunchCommands();
	auto *cl = GetClientState();
	if(cl != nullptr)
		SetHRTFEnabled(cl->GetConVarBool("cl_audio_hrtf_enabled"));
}
void pragma::CEngine::ClearConsole()
{
	auto *pConsole = gui::types::WIConsole::GetConsole();
	if(pConsole == nullptr) {
		Engine::ClearConsole();
		return;
	}
	pConsole->Clear();
}
void pragma::CEngine::OpenConsole()
{
	switch(m_consoleType) {
	case ConsoleType::Terminal:
		Engine::OpenConsole();
		break;
	default:
		{
			if(gui::WGUI::IsOpen()) {
				auto *console = gui::types::WIConsole::Open();
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
	math::set_flag(m_stateFlags, StateFlags::ConsoleOpen, true);
	// Engine::OpenConsole();
}
void pragma::CEngine::CloseConsole()
{
	switch(m_consoleType) {
	case ConsoleType::Terminal:
		Engine::CloseConsole();
		break;
	default:
		{
			if(gui::WGUI::IsOpen())
				gui::types::WIConsole::Close();
			break;
		}
	}

	// Engine::CloseConsole();
	math::set_flag(m_stateFlags, StateFlags::ConsoleOpen, false);
}
void pragma::CEngine::SetConsoleType(ConsoleType type)
{
	if(type == m_consoleType)
		return;
	auto isOpen = IsConsoleOpen();
	CloseConsole();
	Engine::SetConsoleType(type);
	if(isOpen)
		OpenConsole();
}
pragma::CEngine::ConsoleType pragma::CEngine::GetConsoleType() const
{
	auto *pConsole = gui::types::WIConsole::GetConsole();
	auto *pFrame = pConsole ? pConsole->GetFrame() : nullptr;
	if(pFrame && pFrame->IsVisible())
		return pFrame->IsDetached() ? ConsoleType::GUIDetached : ConsoleType::GUI;
	return Engine::GetConsoleType();
}
bool pragma::CEngine::IsConsoleOpen() const
{
	switch(m_consoleType) {
	case ConsoleType::Terminal:
		return Engine::IsConsoleOpen();
	default:
		{
			auto *pConsole = gui::types::WIConsole::GetConsole();
			auto *pFrame = pConsole ? pConsole->GetFrame() : nullptr;
			return pFrame && pFrame->IsVisible();
		}
	}
	return false;
}
CallbackHandle pragma::CEngine::AddGPUProfilingHandler(const std::function<void(bool)> &handler)
{
	auto hCb = FunctionCallback<void, bool>::Create(handler);
	m_gpuProfileHandlers.push_back(hCb);
	return hCb;
}
void pragma::CEngine::SetGPUProfilingEnabled(bool bEnabled)
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

std::shared_ptr<prosper::Window> pragma::CEngine::CreateWindow(prosper::WindowSettings &settings)
{
	if(settings.width == 0 || settings.height == 0)
		return nullptr;
	auto &mainWindowCreateInfo = get_cengine()->GetRenderContext().GetWindow().GetWindowSettings();
	settings.flags = mainWindowCreateInfo.flags;
	settings.api = mainWindowCreateInfo.api;
	auto window = get_cengine()->GetRenderContext().CreateWindow(settings);
	if(!window)
		return nullptr;

	if(g_titleBarColor.has_value())
		(*window)->SetTitleBarColor(*g_titleBarColor);
	if(g_borderColor.has_value())
		(*window)->SetBorderColor(*g_borderColor);

	auto *pWindow = window.get();
	pWindow->GetStagingRenderTarget(); // This will initialize the staging target immediately
	(*pWindow)->SetWindowSizeCallback([pWindow](platform::Window &window, Vector2i size) {
		pWindow->ReloadStagingRenderTarget();
		auto *el = gui::WGUI::GetInstance().GetBaseElement(pWindow);
		if(el)
			el->SetSize(size);
	});
	InitializeWindowInputCallbacks(*pWindow);
	gui::WGUI::GetInstance().AddBaseElement(pWindow);
	return window;
}
void pragma::CEngine::InitializeWindowInputCallbacks(prosper::Window &window)
{
	window->SetKeyCallback([this, &window](platform::Window &glfwWindow, platform::Key key, int scanCode, platform::KeyState state, platform::Modifier mods) mutable { KeyboardInput(window, key, scanCode, state, mods); });
	window->SetMouseButtonCallback([this, &window](platform::Window &glfwWindow, platform::MouseButton button, platform::KeyState state, platform::Modifier mods) mutable { MouseInput(window, button, state, mods); });
	window->SetCharCallback([this, &window](platform::Window &glfwWindow, unsigned int c) mutable { CharInput(window, c); });
	window->SetScrollCallback([this, &window](platform::Window &glfwWindow, Vector2 offset) mutable { ScrollInput(window, offset); });
	window->SetFocusCallback([this, &window](platform::Window &glfwWindow, bool bFocused) mutable { OnWindowFocusChanged(window, bFocused); });
	window->SetDropCallback([this, &window](platform::Window &glfwWindow, std::vector<std::string> &files) mutable { OnFilesDropped(window, files); });
	window->SetDragEnterCallback([this, &window](platform::Window &glfwWindow) mutable { OnDragEnter(window); });
	window->SetDragExitCallback([this, &window](platform::Window &glfwWindow) mutable { OnDragExit(window); });
	window->SetOnShouldCloseCallback([this, &window](platform::Window &glfwWindow) -> bool { return OnWindowShouldClose(window); });
	window->SetPreeditCallback([this, &window](platform::Window &glfwWindow, int preedit_count, unsigned int *preedit_string, int block_count, int *block_sizes, int focused_block, int caret) {
		std::vector<int32_t> istr;
		istr.resize(preedit_count);
		for(auto i = decltype(preedit_count) {0u}; i < preedit_count; ++i)
			istr[i] = static_cast<int32_t>(preedit_string[i]);
		string::Utf8String preeditString {istr.data(), istr.size()};

		std::vector<int32_t> blockSizes;
		blockSizes.reserve(block_count);
		for(auto i = decltype(block_count) {0u}; i < block_count; ++i)
			blockSizes.push_back(block_sizes[i]);
		OnPreedit(window, preeditString, blockSizes, focused_block, caret);
	});
	window->SetIMEStatusCallback([this, &window](platform::Window &glfwWindow) { OnIMEStatusChanged(window, glfwWindow.IsIMEEnabled()); });
}
void pragma::CEngine::OnWindowResized(prosper::Window &window, Vector2i size)
{
	m_stateFlags |= StateFlags::WindowSizeChanged;
	m_tWindowResizeTime = util::Clock::now();
}

DLLCLIENT std::optional<std::string> g_customWindowIcon {};
void pragma::CEngine::OnWindowInitialized()
{
	RenderContext::OnWindowInitialized();
	auto &window = GetRenderContext().GetWindow();
	InitializeWindowInputCallbacks(window);
	window->SetWindowSizeCallback([this, &window](platform::Window &glfwWindow, Vector2i size) mutable { OnWindowResized(window, size); });

	if(g_customWindowIcon.has_value()) {
		auto imgBuf = image::load_image(*g_customWindowIcon, image::PixelFormat::LDR);
		if(imgBuf) {
			imgBuf->ToLDRFormat(image::Format::RGBA32);
			window->SetWindowIcon(imgBuf->GetWidth(), imgBuf->GetHeight(), static_cast<uint8_t *>(imgBuf->GetData()));
			platform::poll_events();
		}
	}
}
void pragma::CEngine::InitializeExternalArchiveManager() { pragma::util::initialize_external_archive_manager(GetClientState()); }
bool pragma::CEngine::StartProfilingStage(const char *stage) { return m_cpuProfilingStageManager && m_cpuProfilingStageManager->StartProfilerStage(stage); }
bool pragma::CEngine::StopProfilingStage() { return m_cpuProfilingStageManager && m_cpuProfilingStageManager->StopProfilerStage(); }
bool pragma::CEngine::StartGPUProfilingStage(const char *stage) { return m_gpuProfilingStageManager && m_gpuProfilingStageManager->StartProfilerStage(stage); }
bool pragma::CEngine::StopGPUProfilingStage() { return m_gpuProfilingStageManager && m_gpuProfilingStageManager->StopProfilerStage(); }
bool pragma::CEngine::GetControllersEnabled() const { return math::is_flag_set(m_stateFlags, StateFlags::ControllersEnabled); }
void pragma::CEngine::SetControllersEnabled(bool b)
{
	if(GetControllersEnabled() == b)
		return;
	math::set_flag(m_stateFlags, StateFlags::ControllersEnabled, b);
	if(b == false) {
		platform::set_joysticks_enabled(false);
		return;
	}
	platform::set_joysticks_enabled(true);
	platform::set_joystick_axis_threshold(0.01f);
	platform::set_joystick_button_callback([this](const platform::Joystick &joystick, uint32_t key, platform::KeyState oldState, platform::KeyState newState) {
		auto keyOffset = GLFW_CUSTOM_KEY_JOYSTICK_0_KEY_START + joystick.GetJoystickId() * GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT;
		JoystickButtonInput(GetWindow(), joystick, key + keyOffset, newState);
	});
	platform::set_joystick_axis_callback([this](const platform::Joystick &joystick, uint32_t axisId, float oldVal, float newVal) {
		m_rawInputJoystickMagnitude = newVal;
		auto mods = platform::Modifier::None;
		auto axisOffset = GLFW_CUSTOM_KEY_JOYSTICK_0_AXIS_START + joystick.GetJoystickId() * GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT;
		if(SEPARATE_JOYSTICK_AXES == true) {
			axisId *= 2;
			if(math::sign(oldVal) != math::sign(newVal)) {
				auto prevAxisId = axisId;
				auto prevMods = mods;
				if(oldVal < 0.f) {
					++prevAxisId;
					prevMods |= platform::Modifier::AxisNegative;
				}
				JoystickAxisInput(GetWindow(), joystick, prevAxisId + axisOffset, prevMods, 0.f, 0.f - oldVal);
				oldVal = 0.f;
			}
			if(newVal < 0.f) {
				oldVal = -oldVal;
				newVal = -newVal;
				++axisId;
				mods |= platform::Modifier::AxisNegative;
			}
		}
		JoystickAxisInput(GetWindow(), joystick, axisId + axisOffset, mods, newVal, newVal - oldVal);
	});
	platform::set_joystick_state_callback([this](const platform::Joystick &joystick, bool bConnected) { get_cengine()->CallCallbacks<void, std::reference_wrapper<const platform::Joystick>, bool>("OnJoystickStateChanged", std::ref(joystick), bConnected); });
}
namespace {
	auto UVN = pragma::console::client::register_variable_listener<bool>("cl_controller_enabled", +[](pragma::NetworkState *, const pragma::console::ConVar &, bool, bool newVal) { pragma::get_cengine()->SetControllersEnabled(newVal); });
}

float pragma::CEngine::GetRawJoystickAxisMagnitude() const { return m_rawInputJoystickMagnitude; }

std::unique_ptr<pragma::CEngine::ConVarInfoList> &pragma::CEngine::GetConVarConfig(NwStateType type)
{
	if(type == NwStateType::Client)
		return m_clConfig;
	return Engine::GetConVarConfig(type);
}
pragma::Engine::StateInstance &pragma::CEngine::GetStateInstance(NetworkState &nw)
{
	if(m_clInstance->state.get() == &nw)
		return *m_clInstance;
	return Engine::GetStateInstance(nw);
}
pragma::Engine::StateInstance &pragma::CEngine::GetClientStateInstance() { return *m_clInstance; }

pragma::util::WeakHandle<prosper::Shader> pragma::CEngine::ReloadShader(const std::string &name)
{
#ifdef _DEBUG
	bReload = true;
#endif
	/*	Con::CERR<<"Loading shader "<<name<<"..."<<Con::endl;
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
void pragma::CEngine::ReloadShaderPipelines()
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

pragma::CEngine::~CEngine() { m_audioAPILib = nullptr; }

pragma::CEngine *pragma::get_cengine() { return g_engine; }

void pragma::CEngine::HandleLocalHostPlayerClientPacket(NetPacket &p)
{
	auto *client = GetClientState();
	if(client == nullptr)
		return;
	auto *cl = static_cast<ClientState *>(client)->GetClient();
	if(cl == nullptr)
		return;
	cl->HandlePacket(p);
}

void pragma::CEngine::Connect(const std::string &ip, const std::string &port)
{
	auto *cl = static_cast<ClientState *>(GetClientState());
	if(cl == nullptr)
		return;
	cl->Disconnect();
	if(ip != "localhost")
		get_cengine()->CloseServerState();
	else {
		auto steamId = get_cengine()->GetServerSteamId();
		if(steamId.has_value()) {
			// Listen server is peer-to-peer; Connect via steam ID
			cl->Connect(*steamId);
			return;
		}
	}
	cl->Connect(ip, port);
}

void pragma::CEngine::Connect(uint64_t steamId)
{
	auto *cl = static_cast<ClientState *>(GetClientState());
	if(cl == nullptr)
		return;
	cl->Disconnect();
	get_cengine()->CloseServerState();
	cl->Connect(steamId);
}

void pragma::CEngine::Disconnect()
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

bool pragma::CEngine::IsMultiPlayer() const
{
	if(Engine::IsMultiPlayer())
		return true;
	auto *clState = static_cast<ClientState *>(GetClientState());
	if(clState == nullptr)
		return false;
	auto *cl = clState ? clState->GetClient() : nullptr;
	return cl && typeid(*cl) != typeid(networking::LocalClient);
}

void pragma::CEngine::StartDefaultGame(const std::string &map, bool singlePlayer)
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

void pragma::CEngine::StartDefaultGame(const std::string &map) { StartDefaultGame(map, true); }

Lua::Interface *pragma::CEngine::GetLuaInterface(lua::State *l)
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

bool pragma::CEngine::IsProgramInFocus() const
{
	for(auto &window : GetRenderContext().GetWindows()) {
		if((*window)->IsInFocus())
			return true;
	}
	return false;
}

pragma::NetworkState *pragma::CEngine::GetNetworkState(lua::State *l)
{
	auto *cl = static_cast<ClientState *>(GetClientState());
	if(cl == nullptr)
		return nullptr;
	if(cl->GetLuaState() == l || cl->GetGUILuaState() == l)
		return cl;
	return Engine::GetNetworkState(l);
}

void pragma::CEngine::Start() { Engine::Start(); }

void pragma::CEngine::Close()
{
	if(math::is_flag_set(m_stateFlags, StateFlags::CEClosed))
		return;
	math::set_flag(m_stateFlags, StateFlags::CEClosed);

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

	gui::WGUI::GetInstance().ClearSkins(); // Should be cleared before lua states are closed
	CloseClientState();
	m_auxEffects.clear();
	CloseSoundEngine(); // Has to be closed after client state (since clientstate may still have some references at this point)
	m_clInstance = nullptr;
	gui::WGUI::Close(); // Has to be closed after client state
	RenderContext::Release();
	g_engine = nullptr;

	Engine::Close();
}

void pragma::CEngine::OnClose()
{
	RenderContext::OnClose();
	// Clear all Vulkan resources before closing the context
	m_gpuProfiler = {};

	CRenderComponent::ClearBuffers();
	CLightComponent::ClearBuffers();
	geometry::CModelSubMesh::ClearBuffers();
	ecs::CParticleSystemComponent::ClearBuffers();
}

static auto cvFpsDecayFactor = pragma::console::get_client_con_var("cl_fps_decay_factor");
void pragma::CEngine::UpdateFPS(float t)
{
	auto weightRatio = cvFpsDecayFactor->GetFloat();
	m_tFPSTime = t * (1.0 - weightRatio) + m_tFPSTime * weightRatio;
	if(m_tFPSTime > 0.0)
		m_fps = 1.0 / m_tFPSTime;
}

static auto cvProfiling = pragma::console::get_engine_con_var("debug_profiling_enabled");
void pragma::CEngine::DrawFrame()
{
	auto primWindowCmd = GetWindow().GetDrawCommandBuffer();
	auto perfTimers = math::is_flag_set(m_stateFlags, StateFlags::EnableGpuPerformanceTimers);
	if(perfTimers) {
		auto n = math::to_integral(GPUTimer::Count);
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
	static_cast<material::CMaterialManager &>(*m_clInstance->materialManager).Poll(); // Requires active command buffer
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::get_domain().EndTask();
#endif

	StartProfilingStage("GUILogic");
	auto &gui = gui::WGUI::GetInstance();
	gui.Think(primWindowCmd);
	StopProfilingStage(); // GUILogic

	auto &stagingRt = GetRenderContext().GetWindow().GetStagingRenderTarget();
	if(math::is_flag_set(m_stateFlags, StateFlags::FirstFrame))
		math::set_flag(m_stateFlags, StateFlags::FirstFrame, false);
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

static auto cvHideGui = pragma::console::get_client_con_var("debug_hide_gui");
void pragma::CEngine::DrawScene(std::shared_ptr<prosper::RenderTarget> &rt)
{
	auto perfTimers = math::is_flag_set(m_stateFlags, StateFlags::EnableGpuPerformanceTimers);
	auto drawGui = !cvHideGui->GetBool();
	if(drawGui) {
		auto &rp = rt->GetRenderPass();
		auto &fb = rt->GetFramebuffer();
		StartProfilingStage("RecordGUI");
		StartProfilingStage("GUI");

		gui::WGUI::GetInstance().BeginDraw();
		CallCallbacks<void>("PreRecordGUI");
		if(get_cgame() != nullptr)
			get_cgame()->PreGUIRecord();
		auto &context = GetRenderContext();
		for(auto &window : context.GetWindows()) {
			if(!window || window->IsValid() == false || window->GetState() != prosper::Window::State::Active)
				continue;
			auto &windowRt = window->GetStagingRenderTarget();
			auto &swapCmdGroup = window->GetSwapCommandBufferGroup();
			swapCmdGroup.StartRecording(windowRt->GetRenderPass(), windowRt->GetFramebuffer());
			swapCmdGroup.Record([this, window](prosper::ISecondaryCommandBuffer &drawCmd) {
				auto &gui = gui::WGUI::GetInstance();
				StartProfilingStage("DrawGUI");
				gui.Draw(*window, drawCmd);
				StopProfilingStage();
			});
			swapCmdGroup.EndRecording();
		}
		CallCallbacks<void>("PostRecordGUI");
		if(get_cgame() != nullptr)
			get_cgame()->PostGUIRecord();

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
		rendering::DrawSceneInfo drawSceneInfo {};
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
		if(get_cgame() != nullptr)
			get_cgame()->PreGUIDraw();

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
		if(get_cgame() != nullptr)
			get_cgame()->PostGUIDraw();
		gui::WGUI::GetInstance().EndDraw();
		StopProfilingStage(); // ExecuteGUIDrawCalls
	}
}

uint32_t pragma::CEngine::GetPerformanceTimerIndex(uint32_t swapchainIdx, GPUTimer timer) const { return swapchainIdx * math::to_integral(GPUTimer::Count) + math::to_integral(timer); }
uint32_t pragma::CEngine::GetPerformanceTimerIndex(GPUTimer timer) const { return GetPerformanceTimerIndex(GetRenderContext().GetLastAcquiredPrimaryWindowSwapchainImageIndex(), timer); }

void pragma::CEngine::SetGpuPerformanceTimersEnabled(bool enabled)
{
	if(math::is_flag_set(m_stateFlags, StateFlags::EnableGpuPerformanceTimers) == enabled)
		return;
	math::set_flag(m_stateFlags, StateFlags::EnableGpuPerformanceTimers, enabled);

	if(enabled == false) {
		get_cengine()->GetRenderContext().KeepResourceAliveUntilPresentationComplete(m_gpuTimerPool);
		m_gpuTimerPool = nullptr;
		for(auto &t : m_gpuTimers) {
			get_cengine()->GetRenderContext().KeepResourceAliveUntilPresentationComplete(t);
			t = nullptr;
		}
		for(auto &t : m_gpuExecTimes)
			t = std::chrono::nanoseconds {0};
		return;
	}
	auto &context = GetRenderContext();
	auto numSwapchainImages = context.GetPrimaryWindowSwapchainImageCount();
	auto numTimers = math::to_integral(GPUTimer::Count) * numSwapchainImages;
	m_gpuTimerPool = context.CreateQueryPool(prosper::QueryType::Timestamp, numTimers * 2);
	m_gpuTimers.resize(numTimers);
	m_gpuExecTimes.resize(numTimers);
	for(auto &t : m_gpuTimers)
		t = m_gpuTimerPool->CreateTimerQuery(prosper::PipelineStageFlags::TopOfPipeBit, prosper::PipelineStageFlags::BottomOfPipeBit);
}
std::chrono::nanoseconds pragma::CEngine::GetGpuExecutionTime(uint32_t swapchainIdx, GPUTimer timer) const
{
	if(math::is_flag_set(m_stateFlags, StateFlags::EnableGpuPerformanceTimers) == false)
		return std::chrono::nanoseconds {0};
	return m_gpuExecTimes[GetPerformanceTimerIndex(swapchainIdx, timer)];
}

void pragma::CEngine::Think()
{
	platform::poll_joystick_events();

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
	if(cl != nullptr)
		cl->Think(); // Draw?

	StartProfilingStage("DrawFrame");

	RenderContext::DrawFrame();
	CallCallbacks("Draw");
	StopProfilingStage();    // DrawFrame
	platform::poll_events(); // Needs to be called AFTER rendering!
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

void pragma::CEngine::SetFixedFrameDeltaTimeInterpretation(std::optional<std::chrono::nanoseconds> frameDeltaTime) { m_fixedFrameDeltaTimeInterpretation = frameDeltaTime; }
void pragma::CEngine::SetFixedFrameDeltaTimeInterpretationByFPS(uint16_t fps) { SetFixedFrameDeltaTimeInterpretation(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds {1}) / fps); }
void pragma::CEngine::SetTickDeltaTimeTiedToFrameRate(bool tieToFrameRate) { math::set_flag(m_stateFlags, StateFlags::TickDeltaTimeTiedToFrameRate, tieToFrameRate); }

void pragma::CEngine::UpdateTickCount()
{
	if(math::is_flag_set(m_stateFlags, StateFlags::TickDeltaTimeTiedToFrameRate) == false) {
		Engine::UpdateTickCount();
		return;
	}
	m_ctTick.UpdateByDelta(util::clock::to_seconds(m_tDeltaFrameTime));
}

void pragma::CEngine::Tick()
{
	if(math::is_flag_set(m_stateFlags, StateFlags::WindowSizeChanged)) {
		auto t = util::Clock::now();
		auto dt = t - m_tWindowResizeTime;
		// If the window is being resized by the user, we don't want to update the resolution constantly,
		// so we add a small delay
		if(dt > std::chrono::milliseconds {250}) {
			auto &window = GetWindow();
			auto size = window.GetGlfwWindow().GetSize();
			if(size.x > 0 && size.y > 0) { // If either size is 0, the window is probably minimized and we don't need to update.
				math::set_flag(m_stateFlags, StateFlags::WindowSizeChanged, false);
				OnResolutionChanged(size.x, size.y);
			}
		}
	}

	locale::poll();
	ProcessConsoleInput();
	RunTickEvents();

	Engine::StartProfilingStage("Tick");
	// The client tick has to run BEFORE the server tick!!!
	// This is to avoid issues in singleplayer, where the client would use data it received from the server and apply the same calculations on the already modified data.
	StartProfilingStage("ClientTick");
	auto *cl = GetClientState();
	if(cl != nullptr)
		cl->Tick();
	StopProfilingStage(); // ClientTick

	Engine::StartProfilingStage("ServerTick");
	auto *sv = GetServerNetworkState();
	if(sv != nullptr)
		sv->Tick();
	Engine::StopProfilingStage(); // ServerTick
	Engine::StopProfilingStage(); // Tick

	UpdateParallelJobs();
}

bool pragma::CEngine::IsServerOnly() { return false; }

void pragma::CEngine::UseFullbrightShader(bool b) { math::set_flag(m_stateFlags, StateFlags::Fullbright, b); }

void pragma::CEngine::OnResolutionChanged(uint32_t width, uint32_t height)
{
	RenderContext::OnResolutionChanged(width, height);
	if(m_renderResolution.has_value() == false)
		OnRenderResolutionChanged(width, height);
}

void pragma::CEngine::OnRenderResolutionChanged(uint32_t width, uint32_t height)
{
	GetRenderContext().GetWindow().ReloadStagingRenderTarget();
	math::set_flag(m_stateFlags, StateFlags::FirstFrame, true);

	auto &wgui = gui::WGUI::GetInstance();
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

uint32_t pragma::CEngine::DoClearUnusedAssets(asset::Type type) const
{
	if(type == asset::Type::Texture || type == asset::Type::Material || type == asset::Type::Model)
		const_cast<CEngine *>(this)->GetRenderContext().WaitIdle();
	auto n = Engine::DoClearUnusedAssets(type);
	switch(type) {
	case asset::Type::Texture:
		{
			auto *cl = GetClientState();
			if(cl) {
				auto &texManager = static_cast<material::CMaterialManager &>(cl->GetMaterialManager()).GetTextureManager();
				if(!IsVerbose())
					n += texManager.ClearUnused();
				else {
					auto &cache = texManager.GetCache();

					std::unordered_map<material::Texture *, std::string> oldCache;
					for(auto &pair : cache) {
						auto asset = texManager.GetAsset(pair.second);
						if(!asset)
							continue;
						auto tex = material::TextureManager::GetAssetObject(*asset);
						oldCache[tex.get()] = tex->GetName();
					}

					n += texManager.ClearUnused();

					std::unordered_map<material::Texture *, std::string> newCache;
					for(auto &pair : cache) {
						auto asset = texManager.GetAsset(pair.second);
						if(!asset)
							continue;
						auto tex = material::TextureManager::GetAssetObject(*asset);
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

pragma::CEngine::DroppedFile::DroppedFile(const std::string &rootPath, const std::string &_fullPath) : fullPath(_fullPath)
{
	auto path = util::Path::CreateFile(fullPath);
	path.MakeRelative(rootPath);
	fileName = path.GetString();
}

namespace {
	auto UVN = pragma::console::client::register_variable_listener<int32_t>(
	  "cl_render_monitor", +[](pragma::NetworkState *, const pragma::console::ConVar &, int32_t, int32_t monitor) {
		  auto monitors = pragma::platform::get_monitors();
		  if(monitor < monitors.size() && monitor >= 0)
			  pragma::get_cengine()->GetWindow().SetMonitor(monitors[monitor]);
	  });
}
namespace {
	auto UVN = pragma::console::client::register_variable_listener<int32_t>(
	  "cl_render_window_mode", +[](pragma::NetworkState *, const pragma::console::ConVar &, int32_t, int32_t val) {
		  pragma::get_cengine()->GetWindow().SetWindowedMode(val != 0);
		  pragma::get_cengine()->GetWindow().SetNoBorder(val == 2);
	  });
}
namespace {
	auto UVN = pragma::console::client::register_variable_listener<std::string>(
	  "cl_window_resolution", +[](pragma::NetworkState *, const pragma::console::ConVar &, std::string, std::string val) {
		  std::vector<std::string> vals;
		  pragma::string::explode(val, "x", vals);
		  if(vals.size() < 2)
			  return;
		  auto x = pragma::util::to_int(vals[0]);
		  auto y = pragma::util::to_int(vals[1]);
		  Vector2i resolution(x, y);
		  pragma::get_cengine()->GetWindow().SetResolution(resolution);
		  auto *client = static_cast<pragma::ClientState *>(pragma::get_cengine()->GetClientState());
		  if(client != nullptr)
			  return;
		  auto &wgui = pragma::gui::WGUI::GetInstance();
		  auto *el = wgui.GetBaseElement();
		  if(el == nullptr)
			  return;
		  el->SetSize(resolution);
		  auto *menu = client->GetMainMenu();
		  if(menu == nullptr)
			  return;
		  menu->SetSize(x, y);
	  });
}
namespace {
	auto UVN = pragma::console::client::register_variable_listener<std::string>(
	  "cl_render_resolution", +[](pragma::NetworkState *, const pragma::console::ConVar &, std::string, std::string val) {
		  std::vector<std::string> vals;
		  pragma::string::explode(val, "x", vals);
		  if(vals.size() < 2) {
			  pragma::get_cengine()->SetRenderResolution({});
			  return;
		  }
		  auto x = pragma::util::to_int(vals[0]);
		  auto y = pragma::util::to_int(vals[1]);
		  Vector2i resolution(x, y);
		  pragma::get_cengine()->SetRenderResolution(resolution);
	  });
}
namespace {
	auto UVN = pragma::console::client::register_variable_listener<bool>(
	  "cl_gpu_timer_queries_enabled", +[](pragma::NetworkState *, const pragma::console::ConVar &, bool, bool enabled) {
		  if(pragma::get_cengine() == nullptr)
			  return;
		  pragma::get_cengine()->SetGPUProfilingEnabled(enabled);
	  });
}

static void dump_traceback_gui()
{
	auto *en = pragma::get_cengine();
	auto *state = en ? static_cast<pragma::ClientState *>(en->GetClientState()) : nullptr;
	auto *l = state ? state->GetGUILuaState() : nullptr;
	if(!l)
		return;
	Lua::PrintTraceback(l);
}
static void dump_stack_gui()
{
	auto *en = pragma::get_cengine();
	auto *state = en ? static_cast<pragma::ClientState *>(en->GetClientState()) : nullptr;
	auto *l = state ? state->GetGUILuaState() : nullptr;
	if(!l)
		return;
	Lua::StackDump(l);
}
namespace pragma::LuaCore::debug {
	// These are mainly used in the immediate window for debugging purposes
	DLLCLIENT void dump_traceback_gui() { ::dump_traceback_gui(); }
	DLLCLIENT void dump_stack_gui() { ::dump_stack_gui(); }
};
