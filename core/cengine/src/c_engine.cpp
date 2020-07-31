/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_cengine.h"
#include "pragma/c_engine.h"
#include <wgui/wgui.h>
#include "cmaterialmanager.h"
#include "pragma/console/c_cvar.h"
#include <pragma/serverstate/serverstate.h>
#include <texturemanager/texturemanager.h>
#include "pragma/gui/wiimageslideshow.h"
#include "pragma/gui/mainmenu/wimainmenu.h"
#include "pragma/gui/wiconsole.hpp"
#include "pragma/gui/wiframe.h"
#include <pragma/console/convars.h>
#include "pragma/console/engine_cvar.h"
#include "pragma/networking/iclient.hpp"
#include "pragma/networking/local_client.hpp"
#include "pragma/rendering/c_sci_gpu_timer_manager.hpp"
#include <pragma/rendering/scene/util_draw_scene_info.hpp>
#include <pragma/entities/environment/lights/c_env_light.h>
#include <cmaterialmanager.h>
#include <pragma/model/c_modelmesh.h>
#include <cctype>
#include <sharedutils/util_debug.h>
#include <util_zip.h>
#include <pragma/input/inputhelper.h>
#include <fsys/directory_watcher.h>
#include <pragma/game/game_resources.hpp>
#include <sharedutils/util_file.h>
#include <pragma/engine_info.hpp>
#include <prosper_util.hpp>
#include <image/prosper_render_target.hpp>
#include <debug/prosper_debug.hpp>
#include <prosper_command_buffer.hpp>
#include <pragma/entities/environment/env_camera.h>
#include <pragma/entities/components/c_render_component.hpp>
#include <pragma/entities/environment/effects/c_env_particle_system.h>
#include <pragma/rendering/shaders/image/c_shader_clear_color.hpp>
#include <pragma/rendering/shaders/image/c_shader_gradient.hpp>
#include <wgui/types/wicontextmenu.hpp>
#include <wgui/types/witext.h>
#include <wgui/types/witext_tags.hpp>

extern "C"
{
	void DLLCENGINE RunCEngine(int argc,char *argv[])
	{
		auto en = InitializeEngine<CEngine>(argc,argv);
		if(en == nullptr)
			return;
		en->Release(); // Has to be called before object is actually destroyed, to make sure weak_ptr references are still valid
		en = nullptr;
	}
}

DLLCENGINE CEngine *c_engine = NULL;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;
//__declspec(dllimport) std::vector<void*> _vkImgPtrs;
decltype(CEngine::AXIS_PRESS_THRESHOLD) CEngine::AXIS_PRESS_THRESHOLD = 0.5f;

// If set to true, each joystick axes will be split into a positive and a negative axis, which
// can be bound individually
#pragma optimize("",off)
static const auto SEPARATE_JOYSTICK_AXES = true;
CEngine::CEngine(int argc,char* argv[])
	: Engine(argc,argv),pragma::RenderContext(),
	m_nearZ(pragma::BaseEnvCameraComponent::DEFAULT_NEAR_Z),//10.0f), //0.1f
	m_farZ(pragma::BaseEnvCameraComponent::DEFAULT_FAR_Z),
	m_fps(0),m_tFPSTime(0.f),
	m_tLastFrame(util::Clock::now()),m_tDeltaFrameTime(0)
{
	c_engine = this;

	RegisterCallback<void,std::reference_wrapper<const GLFW::Joystick>,bool>("OnJoystickStateChanged");
	RegisterCallback<void,std::reference_wrapper<std::shared_ptr<prosper::IPrimaryCommandBuffer>>>("DrawFrame");
	RegisterCallback<void,std::reference_wrapper<std::shared_ptr<prosper::IPrimaryCommandBuffer>>>("PreDrawGUI");
	RegisterCallback<void,std::reference_wrapper<std::shared_ptr<prosper::IPrimaryCommandBuffer>>>("PostDrawGUI");
	RegisterCallback<void>("Draw");

	RegisterCallbackWithOptionalReturn<bool,std::reference_wrapper<GLFW::Window>,GLFW::MouseButton,GLFW::KeyState,GLFW::Modifier>("OnMouseInput");
	RegisterCallbackWithOptionalReturn<bool,std::reference_wrapper<GLFW::Window>,GLFW::Key,int,GLFW::KeyState,GLFW::Modifier,float>("OnKeyboardInput");
	RegisterCallbackWithOptionalReturn<bool,std::reference_wrapper<GLFW::Window>,unsigned int>("OnCharInput");
	RegisterCallbackWithOptionalReturn<bool,std::reference_wrapper<GLFW::Window>,Vector2>("OnScrollInput");
	RegisterCallbackWithOptionalReturn<bool,std::reference_wrapper<GLFW::Window>,std::reference_wrapper<const GLFW::Joystick>,uint32_t,GLFW::KeyState>("OnJoystickButtonInput");
	RegisterCallbackWithOptionalReturn<bool,std::reference_wrapper<GLFW::Window>,std::reference_wrapper<const GLFW::Joystick>,uint32_t,GLFW::Modifier,float,float>("OnJoystickAxisInput");

	AddProfilingHandler([this](bool profilingEnabled) {
		if(profilingEnabled == false)
		{
			m_profilingStageManager = nullptr;
			return;
		}
		auto &cpuProfiler = c_engine->GetProfiler();
		m_cpuProfilingStageManager = std::make_unique<pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage,CPUProfilingPhase>>();
		auto stageFrame = pragma::debug::ProfilingStage::Create(cpuProfiler,"DrawFrame",&Engine::GetProfilingStageManager()->GetProfilerStage(Engine::CPUProfilingPhase::Think));
		m_cpuProfilingStageManager->InitializeProfilingStageManager(cpuProfiler,{
			stageFrame,
			pragma::debug::ProfilingStage::Create(cpuProfiler,"GUI",stageFrame.get()),
			pragma::debug::ProfilingStage::Create(cpuProfiler,"ClientTick",&Engine::GetProfilingStageManager()->GetProfilerStage(Engine::CPUProfilingPhase::Tick))
		});
		static_assert(umath::to_integral(CPUProfilingPhase::Count) == 3u,"Added new profiling phase, but did not create associated profiling stage!");
	});
}

void CEngine::Release()
{
	Close();
	Engine::Release();
	pragma::RenderContext::Release();
}

pragma::debug::GPUProfiler &CEngine::GetGPUProfiler() const {return *m_gpuProfiler;}
pragma::debug::ProfilingStageManager<pragma::debug::GPUProfilingStage,CEngine::GPUProfilingPhase> *CEngine::GetGPUProfilingStageManager() {return m_gpuProfilingStageManager.get();}
pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage,CEngine::CPUProfilingPhase> *CEngine::GetProfilingStageManager() {return m_cpuProfilingStageManager.get();}

static auto cvGPUProfiling = GetClientConVar("cl_gpu_timer_queries_enabled");
bool CEngine::IsGPUProfilingEnabled() const
{
	return cvGPUProfiling->GetBool();
}

void CEngine::DumpDebugInformation(ZIPFile &zip) const
{
	Engine::DumpDebugInformation(zip);
	std::stringstream ss;
	// TODO
#if 0
	auto deviceInfo = prosper::util::get_vendor_device_info(GetRenderContext());
	ss<<"Vulkan API Version: "<<deviceInfo.apiVersion<<"\n";
	ss<<"Device Name: "<<deviceInfo.deviceName<<"\n";
	ss<<"Device Type: "<<prosper::util::to_string(deviceInfo.deviceType)<<"\n";
	ss<<"Driver Version: "<<deviceInfo.driverVersion<<"\n";
	ss<<"Vendor: "<<prosper::util::to_string(deviceInfo.vendor)<<"\n";
	ss<<"Vendor ID: "<<umath::to_integral(deviceInfo.vendor);
	zip.AddFile("gpu.txt",ss.str());
#endif
	
	ss.str(std::string());
	ss.clear();
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

void CEngine::InitializeStagingTarget()
{
	c_engine->GetRenderContext().WaitIdle();
	auto resolution = GetRenderResolution();
	prosper::util::ImageCreateInfo createInfo {};
	createInfo.usage = prosper::ImageUsageFlags::TransferDstBit | prosper::ImageUsageFlags::TransferSrcBit | prosper::ImageUsageFlags::ColorAttachmentBit;
	createInfo.format = prosper::Format::R8G8B8A8_UNorm;
	createInfo.width = resolution.x;
	createInfo.height = resolution.y;
	createInfo.postCreateLayout = prosper::ImageLayout::ColorAttachmentOptimal;
	auto stagingImg = GetRenderContext().CreateImage(createInfo);
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	auto stagingTex = GetRenderContext().CreateTexture({},*stagingImg,imgViewCreateInfo);

	auto rp = GetRenderContext().CreateRenderPass(
		prosper::util::RenderPassCreateInfo{{{prosper::Format::R8G8B8A8_UNorm,prosper::ImageLayout::ColorAttachmentOptimal,prosper::AttachmentLoadOp::DontCare,
			prosper::AttachmentStoreOp::Store,prosper::SampleCountFlags::e1Bit,prosper::ImageLayout::ColorAttachmentOptimal
		}}}
		//prosper::util::RenderPassCreateInfo{vk::Format::eD32Sfloat,vk::ImageLayout::eDepthStencilAttachmentOptimal,vk::AttachmentLoadOp::eClear}
	);
	umath::set_flag(m_stateFlags,StateFlags::FirstFrame);
	m_stagingRenderTarget = GetRenderContext().CreateRenderTarget({stagingTex},rp);//,finalDepthTex},rp);
	m_stagingRenderTarget->SetDebugName("engine_staging_rt");
	// Vulkan TODO: Resize when window resolution was changed
}

const std::shared_ptr<prosper::RenderTarget> &CEngine::GetStagingRenderTarget() const {return m_stagingRenderTarget;}

void CEngine::SetRenderResolution(std::optional<Vector2i> resolution)
{
	if(m_renderResolution == resolution)
		return;
	m_renderResolution = resolution;

	resolution = GetRenderResolution();
	OnRenderResolutionChanged(resolution->x,resolution->y);
}
Vector2i CEngine::GetRenderResolution() const
{
	if(m_renderResolution.has_value())
		return *m_renderResolution;
	auto &windowCreateInfo = GetRenderContext().GetWindowCreationInfo();
	return Vector2i{windowCreateInfo.width,windowCreateInfo.height};
}

UInt32 CEngine::GetFPS() const {return m_fps;}
UInt32 CEngine::GetFrameTime() const {return CUInt32(m_tFPSTime *1000.f);}
Double CEngine::GetDeltaFrameTime() const {return util::clock::to_seconds(m_tDeltaFrameTime);}

static auto cvFrameLimit = GetClientConVar("cl_max_fps");
float CEngine::GetFPSLimit() const {return cvFrameLimit->GetFloat();}

unsigned int CEngine::GetStereoSourceCount() {return 0;}
unsigned int CEngine::GetMonoSourceCount() {return 0;}
unsigned int CEngine::GetStereoSource(unsigned int idx) {return 0;}
float CEngine::GetNearZ() {return m_nearZ;}
float CEngine::GetFarZ() {return m_farZ;}

bool CEngine::IsClientConnected()
{
	auto *cl = static_cast<ClientState*>(GetClientState());
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

void CEngine::Input(int key,GLFW::KeyState inputState,GLFW::KeyState pressState,GLFW::Modifier mods,float magnitude)
{
	if(inputState == GLFW::KeyState::Press || inputState == GLFW::KeyState::Release || inputState == GLFW::KeyState::Held)
	{
		if((mods &GLFW::Modifier::AxisNegative) != GLFW::Modifier::None)
		{
			// We need to check if there are any keybindings with a command with the JoystickAxisSingle flag set,
			// in which case that keybinding has priority
			auto keyPositive = key -1;
			auto it = m_keyMappings.find(CInt16(keyPositive));
			if(it != m_keyMappings.end() && it->second.Execute(inputState,pressState,mods,magnitude) == true)
				return;
			mods &= ~GLFW::Modifier::AxisNegative;
		}
		auto it = m_keyMappings.find(CInt16(key));
		if(it != m_keyMappings.end())
			it->second.Execute(inputState,pressState,mods,magnitude);
	}
}
void CEngine::Input(int key,GLFW::KeyState state,GLFW::Modifier mods,float magnitude) {Input(key,state,state,mods,magnitude);}
void CEngine::MouseInput(GLFW::Window &window,GLFW::MouseButton button,GLFW::KeyState state,GLFW::Modifier mods)
{
	auto handled = false;
	if(CallCallbacksWithOptionalReturn<bool,std::reference_wrapper<GLFW::Window>,GLFW::MouseButton,GLFW::KeyState,GLFW::Modifier>("OnMouseInput",handled,window,button,state,mods) == CallbackReturnType::HasReturnValue && handled == true)
		return;
	if(client != nullptr && client->RawMouseInput(button,state,mods) == false)
		return;
	if(WGUI::GetInstance().HandleMouseInput(window,button,state,mods))
		return;
	button += static_cast<GLFW::MouseButton>(GLFW::Key::Last);
	if(client != nullptr && client->MouseInput(button,state,mods) == false)
		return;
	Input(static_cast<int>(button),state);
}
void CEngine::GetMappedKeys(const std::string &cvarName,std::vector<GLFW::Key> &keys,uint32_t maxKeys)
{
	if(maxKeys != std::numeric_limits<uint32_t>::max())
		keys.reserve(maxKeys);
	if(maxKeys == 0)
		return;
	std::string cmd;
	std::vector<std::string> argv {};
	ustring::get_sequence_commands(cvarName,[&cmd,&argv](std::string cmdOther,std::vector<std::string> argvOther) {
		cmd = cmdOther;
		argv = argvOther;
	});
	auto &keyMappings = GetKeyMappings();
	for(auto &pair : keyMappings)
	{
		auto &keyBind = pair.second;
		auto bFoundCvar = false;
		ustring::get_sequence_commands(keyBind.GetBind(),[&cmd,&argv,&bFoundCvar](std::string cmdOther,std::vector<std::string> &argvOther) {
			if(cmdOther == "toggle" && argvOther.empty() == false)
				cmdOther += " " +argvOther.front();
			if(cmdOther == cmd && argv.size() == argvOther.size())
			{
				auto bDiscrepancy = false;
				for(auto i=decltype(argv.size()){0};i<argv.size();++i)
				{
					if(argv.at(i) == argvOther.at(i))
						continue;
					bDiscrepancy = true;
					break;
				}
				if(bDiscrepancy == false)
					bFoundCvar = true;
			}
		});
		if(bFoundCvar == true)
		{
			if(keys.size() == keys.capacity())
				keys.reserve(keys.size() +10);
			keys.push_back(static_cast<GLFW::Key>(pair.first));
			if(keys.size() == maxKeys)
				break;
		}
	}
}
void CEngine::JoystickButtonInput(GLFW::Window &window,const GLFW::Joystick &joystick,uint32_t key,GLFW::KeyState state)
{
	auto handled = false;
	if(CallCallbacksWithOptionalReturn<bool,std::reference_wrapper<GLFW::Window>,std::reference_wrapper<const GLFW::Joystick>,uint32_t,GLFW::KeyState>("OnJoystickButtonInput",handled,window,joystick,key,state) == CallbackReturnType::HasReturnValue && handled == true)
		return;
	KeyboardInput(window,static_cast<GLFW::Key>(key),-1,state,{});
}
void CEngine::JoystickAxisInput(GLFW::Window &window,const GLFW::Joystick &joystick,uint32_t axis,GLFW::Modifier mods,float newVal,float deltaVal)
{
	auto handled = false;
	if(CallCallbacksWithOptionalReturn<bool,std::reference_wrapper<GLFW::Window>,std::reference_wrapper<const GLFW::Joystick>,uint32_t,GLFW::Modifier,float,float>("OnJoystickAxisInput",handled,window,joystick,axis,mods,newVal,deltaVal) == CallbackReturnType::HasReturnValue && handled == true)
		return;
	auto oldVal = newVal -deltaVal;
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
	if(umath::abs(newVal) > AXIS_PRESS_THRESHOLD)
	{
		if(umath::abs(oldVal) <= AXIS_PRESS_THRESHOLD)
			mods |= GLFW::Modifier::AxisPress; // Axis represents actual button press
	}
	else if(umath::abs(oldVal) > AXIS_PRESS_THRESHOLD)
		mods |= GLFW::Modifier::AxisRelease; // Axis represents actual button release
	KeyboardInput(window,key,-1,state,mods,newVal);
}
static auto cvAxisInputThreshold = GetClientConVar("cl_controller_axis_input_threshold");
bool CEngine::IsValidAxisInput(float axisInput) const
{
	return (umath::abs(axisInput) > cvAxisInputThreshold->GetFloat()) ? true : false;
}

bool CEngine::GetInputButtonState(float axisInput,GLFW::Modifier mods,GLFW::KeyState &inOutState) const
{
	if(IsValidAxisInput(axisInput) == false)
	{
		if((mods &GLFW::Modifier::AxisInput) != GLFW::Modifier::None)
		{
			inOutState = GLFW::KeyState::Release;
			return true;
		}
		inOutState = GLFW::KeyState::Invalid;
		return false;
	}
	if((mods &GLFW::Modifier::AxisInput) == GLFW::Modifier::None)
		return true; // No need to change state

	if((mods &GLFW::Modifier::AxisPress) != GLFW::Modifier::None)
		inOutState = GLFW::KeyState::Press;
	else if((mods &GLFW::Modifier::AxisRelease) != GLFW::Modifier::None)
		inOutState = GLFW::KeyState::Release;
	else
	{
		inOutState = GLFW::KeyState::Invalid;
		return false; // Not an actual key press
	}
	return true;
}
void CEngine::KeyboardInput(GLFW::Window &window,GLFW::Key key,int scanCode,GLFW::KeyState state,GLFW::Modifier mods,float magnitude)
{
	auto handled = false;
	if(CallCallbacksWithOptionalReturn<bool,std::reference_wrapper<GLFW::Window>,GLFW::Key,int,GLFW::KeyState,GLFW::Modifier,float>("OnKeyboardInput",handled,window,key,scanCode,state,mods,magnitude) == CallbackReturnType::HasReturnValue && handled == true)
		return;
	if(client != nullptr && client->RawKeyboardInput(key,scanCode,state,mods,magnitude) == false)
		return;
	if(key == GLFW::Key::Escape) // Escape key is hardcoded
	{
		if(client != nullptr)
		{
			if(state == GLFW::KeyState::Press)
				client->ToggleMainMenu();
			return;
		}
	}
	auto buttonState = state;
	auto bValidButtonInput = GetInputButtonState(magnitude,mods,buttonState);
	if(bValidButtonInput == true)
	{
		if(WGUI::GetInstance().HandleKeyboardInput(window,key,scanCode,buttonState,mods))
			return;
	}
	if(client != nullptr && client->KeyboardInput(key,scanCode,state,mods,magnitude) == false)
		return;
	key = static_cast<GLFW::Key>(std::tolower(static_cast<int>(key)));
	Input(static_cast<int>(key),state,buttonState,mods,magnitude);
}
void CEngine::CharInput(GLFW::Window &window,unsigned int c)
{
	auto handled = false;
	if(CallCallbacksWithOptionalReturn<bool,std::reference_wrapper<GLFW::Window>,unsigned int>("OnCharInput",handled,window,c) == CallbackReturnType::HasReturnValue && handled == true)
		return;
	if(client != nullptr && client->RawCharInput(c) == false)
		return;
	if(WGUI::GetInstance().HandleCharInput(window,c))
		return;
	if(c == '`')
	{
		if(IsConsoleOpen())
			CloseConsole();
		else
			OpenConsole();
		return;
	}
	if(client != nullptr && client->CharInput(c) == false)
		return;
}
void CEngine::ScrollInput(GLFW::Window &window,Vector2 offset)
{
	auto handled = false;
	if(CallCallbacksWithOptionalReturn<bool,std::reference_wrapper<GLFW::Window>,Vector2>("OnScrollInput",handled,window,offset) == CallbackReturnType::HasReturnValue && handled == true)
		return;
	if(client != nullptr && client->RawScrollInput(offset) == false)
		return;
	if(WGUI::GetInstance().HandleScrollInput(window,offset))
		return;
	if(client != nullptr && client->ScrollInput(offset) == false)
		return;
	if(offset.y >= 0.f)
	{
		Input(GLFW_CUSTOM_KEY_SCRL_UP,GLFW::KeyState::Press);
		Input(GLFW_CUSTOM_KEY_SCRL_UP,GLFW::KeyState::Release);
	}
	else
	{
		Input(GLFW_CUSTOM_KEY_SCRL_DOWN,GLFW::KeyState::Press);
		Input(GLFW_CUSTOM_KEY_SCRL_DOWN,GLFW::KeyState::Release);
	}
}

void CEngine::OnWindowFocusChanged(GLFW::Window &window,bool bFocused)
{
	umath::set_flag(m_stateFlags,StateFlags::WindowFocused,bFocused);
	if(client != nullptr)
		client->UpdateSoundVolume();
}
void CEngine::OnFilesDropped(GLFW::Window &window,std::vector<std::string> &files)
{
	if(client == nullptr)
		return;
	client->OnFilesDropped(files);
}
bool CEngine::IsWindowFocused() const {return umath::is_flag_set(m_stateFlags,StateFlags::WindowFocused);}

bool CEngine::Initialize(int argc,char *argv[])
{
	Engine::Initialize(argc,argv,false);

	auto &cmds = *m_preloadedConfig.get();

	auto *cviRenderAPI = cmds.find("render_api");
	if(cviRenderAPI && cviRenderAPI->argv.empty() == false)
		SetRenderAPI(cviRenderAPI->argv.front());

	// Initialize Window context
	try
	{
		InitializeRenderAPI();
	}
	catch(const std::runtime_error &err)
	{
		Con::cerr<<"ERROR: Unable to initialize graphics API: "<<err.what()<<Con::endl;
		std::this_thread::sleep_for(std::chrono::seconds(5));
		Close();
		return false;
	}

	auto res = cmds.find("cl_window_resolution");
	prosper::IPrContext::CreateInfo contextCreateInfo {};
	contextCreateInfo.width = 1280;
	contextCreateInfo.height = 1024;
	if(res != nullptr && !res->argv.empty())
	{
		std::vector<std::string> vals;
		ustring::explode(res->argv[0],"x",vals);
		if(vals.size() >= 2)
		{
			contextCreateInfo.width = util::to_int(vals[0]);
			contextCreateInfo.height = util::to_int(vals[1]);
		}
	}
	// SetResolution(Vector2i(contextCreateInfo.width,contextCreateInfo.height));

	res = cmds.find("cl_render_resolution");
	if(res != nullptr && !res->argv.empty())
	{
		std::vector<std::string> vals;
		ustring::explode(res->argv[0],"x",vals);
		if(vals.size() >= 2)
		{
			m_renderResolution = {
				util::to_int(vals[0]),
				util::to_int(vals[1])
			};
		}
	}
	//

	res = cmds.find("cl_render_window_mode");
	int mode = 0;
	if(res != nullptr && !res->argv.empty())
		mode = util::to_int(res->argv[0]);
	if(mode == 0)
		SetWindowedMode(false);
	else
		SetWindowedMode(true);
	GetRenderContext().GetWindowCreationInfo().decorated = ((mode == 2) ? false : true);

	res = cmds.find("cl_render_monitor");
	if(res != nullptr && !res->argv.empty())
	{
		auto monitor = util::to_int(res->argv[0]);
		auto monitors = GLFW::get_monitors();
		if(monitor < monitors.size() && monitor > 0)
			SetMonitor(monitors[monitor]);
	}

	res = cmds.find("cl_gpu_device");
	if(res != nullptr && !res->argv.empty())
	{
		auto device = res->argv[0];
		std::vector<std::string> subStrings;
		ustring::explode(device,",",subStrings);
		if(subStrings.size() >= 2)
			contextCreateInfo.device = {static_cast<prosper::Vendor>(util::to_uint(subStrings.at(0))),util::to_uint(subStrings.at(1))};
	}

	auto presentMode = prosper::PresentModeKHR::Mailbox;
	res = cmds.find("cl_render_present_mode");
	if(res != nullptr && !res->argv.empty())
	{
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
	shaderManager.RegisterShader("clear_color",[](prosper::IPrContext &context,const std::string &identifier) {return new pragma::ShaderClearColor(context,identifier);});
	shaderManager.RegisterShader("gradient",[](prosper::IPrContext &context,const std::string &identifier) {return new pragma::ShaderGradient(context,identifier);});

	// Initialize Client Instance
	auto matManager = std::make_shared<CMaterialManager>(this->GetRenderContext());
	matManager->GetTextureManager().SetTextureFileHandler([this](const std::string &fpath) -> VFilePtr {
		if(FileManager::Exists(fpath) == false)
		{
			auto &formats = MaterialManager::get_supported_image_formats();
			auto *cl = GetClientState();
			auto path = fpath;
			ufile::remove_extension_from_filename(path);
			for(auto &format : formats)
			{
				if(util::port_file(cl,path +'.' +format.extension) == true)
					break;
			}
		}
		return nullptr;
	});
	matManager->SetTextureImporter([this](const std::string &fpath,const std::string &outputPath) -> VFilePtr {
		if(FileManager::Exists(fpath) == false)
		{
			auto &formats = MaterialManager::get_supported_image_formats();
			auto *cl = GetClientState();
			auto path = fpath;
			ufile::remove_extension_from_filename(path);
			for(auto &format : formats)
			{
				if(util::port_file(cl,path +'.' +format.extension,outputPath +'.' +format.extension) == true)
					break;
			}
		}
		return nullptr;
	});
	auto *matErr = matManager->Load("error",nullptr,nullptr,false,nullptr,true);
	m_clInstance = std::unique_ptr<StateInstance>(new StateInstance{matManager,matErr});
	//

	auto &gui = WGUI::Open(GetRenderContext(),matManager);
	gui.SetMaterialLoadHandler([this](const std::string &path) -> Material* {
		return GetClientState()->LoadMaterial(path);
	});
	auto r = gui.Initialize(GetRenderResolution());
	if(r != WGUI::ResultCode::Ok)
	{
		Con::cerr<<"ERROR: Unable to initialize GUI library: ";
		switch(r)
		{
			case WGUI::ResultCode::UnableToInitializeFontManager:
				Con::cerr<<"Error initializing font manager!";
				break;
			case WGUI::ResultCode::ErrorInitializingShaders:
				Con::cerr<<"Error initializing shaders!";
				break;
			case WGUI::ResultCode::FontNotFound:
				Con::cerr<<"Font not found!";
				break;
			default:
				Con::cout<<"Unknown error!";
				break;
		}
		matManager = nullptr;
		Close();
		Release();
		std::this_thread::sleep_for(std::chrono::seconds(5));
		return false;
	}
	WIContextMenu::SetKeyBindHandler([this](GLFW::Key key,const std::string &cmd) -> std::string {
		std::string keyStr;
		auto b = KeyToText(umath::to_integral(key),&keyStr);
		short c;
		if(StringToKey(keyStr,&c))
			MapKey(c,cmd);
		return keyStr;
	},[this](const std::string &cmd) -> std::optional<std::string> {
		std::vector<GLFW::Key> keys;
		GetMappedKeys(cmd,keys);
		if(keys.empty())
			return {};
		std::string strKey {};
		KeyToText(umath::to_integral(keys.front()),&strKey);
		return strKey;
	});
	WITextTagLink::set_link_handler([this](const std::string &arg) {
		std::vector<std::string> args {};
		ustring::explode_whitespace(arg,args);
		if(args.empty())
			return;
		auto cmd = args.front();
		args.erase(args.begin());
		if(ustring::compare(cmd,"url",false))
		{
			if(args.empty() == false)
				util::open_url_in_browser(args.front());
			return;
		}
		RunConsoleCommand(cmd,args);
	});

	m_speedCam = 1600.0f;
	m_speedCamMouse = 0.2f;
	
	InitializeStagingTarget();

	m_gpuProfiler = pragma::debug::GPUProfiler::Create<pragma::debug::GPUProfiler>();
	AddGPUProfilingHandler([this](bool profilingEnabled) {
		if(profilingEnabled == false)
		{
			m_gpuProfilingStageManager = nullptr;
			return;
		}
		m_gpuProfilingStageManager = std::make_unique<pragma::debug::ProfilingStageManager<pragma::debug::GPUProfilingStage,GPUProfilingPhase>>();
		auto &gpuProfiler = *m_gpuProfiler;
		const auto defaultStage = prosper::PipelineStageFlags::BottomOfPipeBit;
		auto stageFrame = pragma::debug::GPUProfilingStage::Create(gpuProfiler,"Frame",defaultStage);
		m_gpuProfilingStageManager->InitializeProfilingStageManager(gpuProfiler,{
			stageFrame,
			pragma::debug::GPUProfilingStage::Create(gpuProfiler,"DrawScene",defaultStage,stageFrame.get()),
			pragma::debug::GPUProfilingStage::Create(gpuProfiler,"GUI",defaultStage,stageFrame.get())
		});
		static_assert(umath::to_integral(GPUProfilingPhase::Count) == 3u,"Added new profiling phase, but did not create associated profiling stage!");
	});

	InitializeSoundEngine();

	auto *cl = OpenClientState();

	if(umath::is_flag_set(m_stateFlags,StateFlags::ConsoleOpen))
		OpenConsole(); // GUI Console mustn't be opened before client has been created!

	RunLaunchCommands();
	if(cl != nullptr)
		SetHRTFEnabled(cl->GetConVarBool("cl_audio_hrtf_enabled"));

#ifdef _WIN32
	if(GetRenderContext().IsValidationEnabled())
	{
		if(util::is_process_running("bdcam.exe"))
		{
			auto r = MessageBox(NULL,"Bandicam is running and vulkan validation mode is enabled. This is NOT recommended, as Bandicam will cause misleading validation errors! Press OK to continue anyway.","Validation Warning",MB_OK | MB_OKCANCEL);
			if(r == IDCANCEL)
				ShutDown();
		}
	}
#endif
	return true;
}
void CEngine::ClearConsole()
{
	auto *pConsole = WIConsole::GetConsole();
	if(pConsole == nullptr)
	{
		Engine::ClearConsole();
		return;
	}
	pConsole->Clear();
}
void CEngine::OpenConsole()
{
	if(WGUI::IsOpen())
		WIConsole::Open();
	umath::set_flag(m_stateFlags,StateFlags::ConsoleOpen,true);
	// Engine::OpenConsole();
}
void CEngine::CloseConsole()
{
	WIConsole::Close();
	// Engine::CloseConsole();
	umath::set_flag(m_stateFlags,StateFlags::ConsoleOpen,false);
}
bool CEngine::IsConsoleOpen() const
{
	auto *pConsole = WIConsole::GetConsole();
	auto *pFrame = pConsole ? pConsole->GetFrame() : nullptr;
	return pFrame && pFrame->IsVisible();
	// Engine::IsConsoleOpen();
}
CallbackHandle CEngine::AddGPUProfilingHandler(const std::function<void(bool)> &handler)
{
	auto hCb = FunctionCallback<void,bool>::Create(handler);
	m_gpuProfileHandlers.push_back(hCb);
	return hCb;
}
void CEngine::SetGPUProfilingEnabled(bool bEnabled)
{
	for(auto it=m_gpuProfileHandlers.begin();it!=m_gpuProfileHandlers.end();)
	{
		auto &hCb = *it;
		if(hCb.IsValid() == false)
		{
			it = m_gpuProfileHandlers.erase(it);
			continue;
		}
		hCb(bEnabled);
		++it;
	}
}
void CEngine::OnWindowInitialized()
{
	pragma::RenderContext::OnWindowInitialized();

	auto &window = GetWindow();
	window.SetKeyCallback(std::bind(&CEngine::KeyboardInput,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4,std::placeholders::_5,1.f));
	window.SetMouseButtonCallback(std::bind(&CEngine::MouseInput,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4));
	window.SetCharCallback(std::bind(&CEngine::CharInput,this,std::placeholders::_1,std::placeholders::_2));
	window.SetScrollCallback(std::bind(&CEngine::ScrollInput,this,std::placeholders::_1,std::placeholders::_2));
	window.SetFocusCallback(std::bind(&CEngine::OnWindowFocusChanged,this,std::placeholders::_1,std::placeholders::_2));
	window.SetDropCallback(std::bind(&CEngine::OnFilesDropped,this,std::placeholders::_1,std::placeholders::_2));
}
void CEngine::InitializeExternalArchiveManager() {util::initialize_external_archive_manager(GetClientState());}
bool CEngine::StartProfilingStage(CPUProfilingPhase stage)
{
	return m_cpuProfilingStageManager && m_cpuProfilingStageManager->StartProfilerStage(stage);
}
bool CEngine::StopProfilingStage(CPUProfilingPhase stage)
{
	return m_cpuProfilingStageManager && m_cpuProfilingStageManager->StopProfilerStage(stage);
}
bool CEngine::StartProfilingStage(GPUProfilingPhase stage)
{
	return m_gpuProfilingStageManager && m_gpuProfilingStageManager->StartProfilerStage(stage);
}
bool CEngine::StopProfilingStage(GPUProfilingPhase stage)
{
	return m_gpuProfilingStageManager && m_gpuProfilingStageManager->StopProfilerStage(stage);
}
bool CEngine::GetControllersEnabled() const {return umath::is_flag_set(m_stateFlags,StateFlags::ControllersEnabled);}
void CEngine::SetControllersEnabled(bool b)
{
	if(GetControllersEnabled() == b)
		return;
	umath::set_flag(m_stateFlags,StateFlags::ControllersEnabled,b);
	if(b == false)
	{
		GLFW::set_joysticks_enabled(false);
		return;
	}
	GLFW::set_joysticks_enabled(true);
	GLFW::set_joystick_axis_threshold(0.01f);
	GLFW::set_joystick_button_callback([this](const GLFW::Joystick &joystick,uint32_t key,GLFW::KeyState oldState,GLFW::KeyState newState) {
		auto keyOffset = GLFW_CUSTOM_KEY_JOYSTICK_0_KEY_START +joystick.GetJoystickId() *GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT;
		JoystickButtonInput(GetWindow(),joystick,key +keyOffset,newState);
	});
	GLFW::set_joystick_axis_callback([this](const GLFW::Joystick &joystick,uint32_t axisId,float oldVal,float newVal) {
		m_rawInputJoystickMagnitude = newVal;
		auto mods = GLFW::Modifier::None;
		auto axisOffset = GLFW_CUSTOM_KEY_JOYSTICK_0_AXIS_START +joystick.GetJoystickId() *GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT;
		if(SEPARATE_JOYSTICK_AXES == true)
		{
			axisId *= 2;
			if(umath::sign(oldVal) != umath::sign(newVal))
			{
				auto prevAxisId = axisId;
				auto prevMods = mods;
				if(oldVal < 0.f)
				{
					++prevAxisId;
					prevMods |= GLFW::Modifier::AxisNegative;
				}
				JoystickAxisInput(GetWindow(),joystick,prevAxisId +axisOffset,prevMods,0.f,0.f -oldVal);
				oldVal = 0.f;
			}
			if(newVal < 0.f)
			{
				oldVal = -oldVal;
				newVal = -newVal;
				++axisId;
				mods |= GLFW::Modifier::AxisNegative;
			}
		}
		JoystickAxisInput(GetWindow(),joystick,axisId +axisOffset,mods,newVal,newVal -oldVal);
	});
	GLFW::set_joystick_state_callback([this](const GLFW::Joystick &joystick,bool bConnected) {
		c_engine->CallCallbacks<void,std::reference_wrapper<const GLFW::Joystick>,bool>("OnJoystickStateChanged",std::ref(joystick),bConnected);
	});
}
REGISTER_CONVAR_CALLBACK_CL(cl_controller_enabled,[](NetworkState *state,ConVar *cv,bool oldVal,bool newVal) {
	c_engine->SetControllersEnabled(newVal);
});

float CEngine::GetRawJoystickAxisMagnitude() const {return m_rawInputJoystickMagnitude;}

Engine::StateInstance &CEngine::GetClientStateInstance() {return *m_clInstance;}

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
	if(whShader.expired())
	{
		if(IsVerbose())
			Con::cwar<<"WARNING: No shader found with name '"<<name<<"'!"<<Con::endl;
		return {};
	}
	if(IsVerbose() == true)
		Con::cout<<"Reloading shader "<<name<<"..."<<Con::endl;
	whShader.get()->Initialize(true);
	return whShader;
}
void CEngine::ReloadShaderPipelines()
{
	GetRenderContext().WaitIdle();
	if(IsVerbose() == true)
		Con::cout<<"Reloading shaders"<<Con::endl;
	auto &shaderManager = GetRenderContext().GetShaderManager();
	auto &shaders = shaderManager.GetShaders();
	for(auto &pair : shaders)
		pair.second->Initialize(true);
}

CEngine::~CEngine() {}

void CEngine::HandleLocalHostPlayerClientPacket(NetPacket &p)
{
	auto *client = GetClientState();
	if(client == nullptr)
		return;
	auto *cl = static_cast<ClientState*>(client)->GetClient();
	if(cl == nullptr)
		return;
	cl->HandlePacket(p);
}

void CEngine::Connect(const std::string &ip,const std::string &port)
{
	auto *cl = static_cast<ClientState*>(GetClientState());
	if(cl == NULL)
		return;
	cl->Disconnect();
	if(ip != "localhost")
		c_engine->CloseServerState();
	else
	{
		auto steamId = c_engine->GetServerSteamId();
		if(steamId.has_value())
		{
			// Listen server is peer-to-peer; Connect via steam ID
			cl->Connect(*steamId);
			return;
		}
	}
	cl->Connect(ip,port);
}

void CEngine::Connect(uint64_t steamId)
{
	auto *cl = static_cast<ClientState*>(GetClientState());
	if(cl == NULL)
		return;
	cl->Disconnect();
	c_engine->CloseServerState();
	cl->Connect(steamId);
}

void CEngine::Disconnect()
{
	auto *cl = static_cast<ClientState*>(GetClientState());
	if(cl == nullptr)
		return;
	if(cl->IsGameActive())
	{
		cl->Disconnect();
		OpenServerState();
	}
	cl->OpenMainMenu();
}

bool CEngine::IsMultiPlayer() const
{
	if(Engine::IsMultiPlayer())
		return true;
	auto *clState = static_cast<ClientState*>(GetClientState());
	if(clState == nullptr)
		return false;
	auto *cl = clState ? clState->GetClient() : nullptr;
	return cl && typeid(*cl) != typeid(pragma::networking::LocalClient);
}

void CEngine::StartDefaultGame(const std::string &map,bool singlePlayer)
{
	EndGame();
	StartNewGame(map.c_str(),singlePlayer);
	Connect("localhost");
}

void CEngine::StartDefaultGame(const std::string &map) {StartDefaultGame(map,true);}

Lua::Interface *CEngine::GetLuaInterface(lua_State *l)
{
	auto *cl = static_cast<ClientState*>(GetClientState());
	if(cl != nullptr)
	{
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
	auto *cl = static_cast<ClientState*>(GetClientState());
	if(cl == NULL)
		return NULL;
	if(cl->GetLuaState() == l || cl->GetGUILuaState() == l)
		return cl;
	return Engine::GetNetworkState(l);
}

void CEngine::Start()
{
	Engine::Start();
}

void CEngine::Close()
{
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
	m_stagingRenderTarget = nullptr;
	m_gpuProfiler = {};

	pragma::CRenderComponent::ClearBuffers();
	pragma::CLightComponent::ClearBuffers();
	CModelSubMesh::ClearBuffers();
	pragma::CParticleSystemComponent::ClearBuffers();
}

void CEngine::UpdateFPS(float t)
{
	const auto weightRatio = 0.8f;
	m_tFPSTime = t *(1.f -weightRatio) +m_tFPSTime *weightRatio;
	if(m_tFPSTime > 0.f)
		m_fps = CUInt32(1.f /m_tFPSTime);
}

static CVar cvProfiling = GetEngineConVar("debug_profiling_enabled");
void CEngine::DrawFrame(prosper::IPrimaryCommandBuffer &drawCmd,uint32_t n_current_swapchain_image)
{
	m_gpuProfiler->Reset();
	StartProfilingStage(GPUProfilingPhase::Frame);

	auto ptrDrawCmd = std::dynamic_pointer_cast<prosper::IPrimaryCommandBuffer>(drawCmd.shared_from_this());
	CallCallbacks<void,std::reference_wrapper<std::shared_ptr<prosper::IPrimaryCommandBuffer>>>("DrawFrame",std::ref(ptrDrawCmd));

	static_cast<CMaterialManager&>(*m_clInstance->materialManager).Update(); // Requires active command buffer

	StartProfilingStage(CPUProfilingPhase::GUI);
	auto &gui = WGUI::GetInstance();
	gui.Think();
	StopProfilingStage(CPUProfilingPhase::GUI);

	auto &stagingRt = m_stagingRenderTarget;
	if(umath::is_flag_set(m_stateFlags,StateFlags::FirstFrame))
		umath::set_flag(m_stateFlags,StateFlags::FirstFrame,false);
	else
	{
		drawCmd.RecordImageBarrier(
			stagingRt->GetTexture().GetImage(),
			prosper::ImageLayout::TransferSrcOptimal,prosper::ImageLayout::ColorAttachmentOptimal
		);
	}

	DrawScene(ptrDrawCmd,stagingRt);

	auto &finalImg = stagingRt->GetTexture().GetImage();
	drawCmd.RecordImageBarrier(
		finalImg,
		prosper::ImageLayout::ColorAttachmentOptimal,prosper::ImageLayout::TransferSrcOptimal
	);

	drawCmd.RecordPresentImage(finalImg,n_current_swapchain_image);

	StopProfilingStage(GPUProfilingPhase::Frame);
}

void CEngine::DrawScene(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,std::shared_ptr<prosper::RenderTarget> &rt)
{
	auto bProfiling = cvProfiling->GetBool();
	auto *cl = static_cast<ClientState*>(GetClientState());
	auto tStart = util::Clock::now();
	if(cl != nullptr)
	{
		StartProfilingStage(GPUProfilingPhase::DrawScene);

		util::DrawSceneInfo drawSceneInfo {};
		drawSceneInfo.commandBuffer = drawCmd;
		cl->Render(drawSceneInfo,rt);

		StopProfilingStage(GPUProfilingPhase::DrawScene);
	}

	CallCallbacks<void,std::reference_wrapper<std::shared_ptr<prosper::IPrimaryCommandBuffer>>>("PreDrawGUI",std::ref(drawCmd));
	if(c_game != nullptr)
		c_game->PreGUIDraw(drawCmd);

	drawCmd->RecordBeginRenderPass(*rt);
	
	StartProfilingStage(GPUProfilingPhase::GUI);
		auto &gui = WGUI::GetInstance();
		gui.Draw();
	StopProfilingStage(GPUProfilingPhase::GUI);

	drawCmd->RecordEndRenderPass();
	CallCallbacks<void,std::reference_wrapper<std::shared_ptr<prosper::IPrimaryCommandBuffer>>>("PostDrawGUI",std::ref(drawCmd));

	if(c_game != nullptr)
		c_game->PostGUIDraw(drawCmd);
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
		tDelta = tNow -m_tLastFrame;
	else
		tDelta = *m_fixedFrameDeltaTimeInterpretation;
	auto maxFps = GetFPSLimit();
	if(maxFps >= 0.f && (maxFps == 0.f || std::chrono::duration_cast<std::chrono::nanoseconds>(tDelta).count() /1'000'000.0 < 1'000.0 /maxFps))
	{
		if(maxFps < 1.f)
		{
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
	UpdateFPS(static_cast<float>(std::chrono::duration_cast<std::chrono::nanoseconds>(m_tDeltaFrameTime).count() /sToNs));
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
	if(GetWindow().ShouldClose())
		ShutDown();

	GetRenderContext().EndFrame();
}

void CEngine::SetFixedFrameDeltaTimeInterpretation(std::optional<std::chrono::nanoseconds> frameDeltaTime)
{
	m_fixedFrameDeltaTimeInterpretation = frameDeltaTime;
}
void CEngine::SetFixedFrameDeltaTimeInterpretationByFPS(uint16_t fps)
{
	SetFixedFrameDeltaTimeInterpretation(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds{1}) /fps);
}
void CEngine::SetTickDeltaTimeTiedToFrameRate(bool tieToFrameRate)
{
	umath::set_flag(m_stateFlags,StateFlags::TickDeltaTimeTiedToFrameRate,tieToFrameRate);
}

void CEngine::UpdateTickCount()
{
	if(umath::is_flag_set(m_stateFlags,StateFlags::TickDeltaTimeTiedToFrameRate) == false)
	{
		Engine::UpdateTickCount();
		return;
	}
	m_ctTick.UpdateByDelta(util::clock::to_seconds(m_tDeltaFrameTime));
}

void CEngine::Tick()
{
	ProcessConsoleInput();
	Engine::StartProfilingStage(Engine::CPUProfilingPhase::Tick);
	// The client tick has to run BEFORE the server tick!!!
	// This is to avoid issues in singleplayer, where the client would use data it received from the server and apply the same calculations on the already modified data.
	StartProfilingStage(CPUProfilingPhase::ClientTick);
	auto *cl = GetClientState();
	if(cl != NULL)
		cl->Tick();
	StopProfilingStage(CPUProfilingPhase::ClientTick);

	Engine::StartProfilingStage(Engine::CPUProfilingPhase::ServerTick);
	auto *sv = GetServerState();
	if(sv != NULL)
		sv->Tick();
	Engine::StopProfilingStage(Engine::CPUProfilingPhase::ServerTick);
	Engine::StopProfilingStage(Engine::CPUProfilingPhase::Tick);

	UpdateParallelJobs();
}

bool CEngine::IsServerOnly() {return false;}

void CEngine::UseFullbrightShader(bool b) {umath::set_flag(m_stateFlags,StateFlags::Fullbright,b);}

void CEngine::OnResolutionChanged(uint32_t width,uint32_t height)
{
	RenderContext::OnResolutionChanged(width,height);
	if(m_renderResolution.has_value() == false)
		OnRenderResolutionChanged(width,height);
}

void CEngine::OnRenderResolutionChanged(uint32_t width,uint32_t height)
{
	InitializeStagingTarget();

	auto &wgui = WGUI::GetInstance();
	auto *baseEl = wgui.GetBaseElement();
	if(baseEl != nullptr)
		baseEl->SetSize(width,height);

	auto *cl = GetClientState();
	if(cl == nullptr)
		return;
	auto *game = static_cast<CGame*>(cl->GetGameState());
	if(game == nullptr)
		return;
	game->Resize();
}

REGISTER_CONVAR_CALLBACK_CL(cl_render_monitor,[](NetworkState*,ConVar*,int32_t,int32_t monitor) {
	auto monitors = GLFW::get_monitors();
	if(monitor < monitors.size() && monitor >= 0)
		c_engine->SetMonitor(monitors[monitor]);
})

REGISTER_CONVAR_CALLBACK_CL(cl_render_window_mode,[](NetworkState*,ConVar*,int32_t,int32_t val) {
	c_engine->SetWindowedMode(val != 0);
	c_engine->SetNoBorder(val == 2);
})

REGISTER_CONVAR_CALLBACK_CL(cl_window_resolution,[](NetworkState*,ConVar*,std::string,std::string val) {
	std::vector<std::string> vals;
	ustring::explode(val,"x",vals);
	if(vals.size() < 2)
		return;
	auto x = util::to_int(vals[0]);
	auto y = util::to_int(vals[1]);
	Vector2i resolution(x,y);
	c_engine->SetResolution(resolution);
	auto *client = static_cast<ClientState*>(c_engine->GetClientState());
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
	menu->SetSize(x,y);
})

REGISTER_CONVAR_CALLBACK_CL(cl_render_resolution,[](NetworkState*,ConVar*,std::string,std::string val) {
	std::vector<std::string> vals;
	ustring::explode(val,"x",vals);
	if(vals.size() < 2)
	{
		c_engine->SetRenderResolution({});
		return;
	}
	auto x = util::to_int(vals[0]);
	auto y = util::to_int(vals[1]);
	Vector2i resolution(x,y);
	c_engine->SetRenderResolution(resolution);
})

REGISTER_CONVAR_CALLBACK_CL(cl_gpu_timer_queries_enabled,[](NetworkState*,ConVar*,bool,bool enabled) {
	if(c_engine == nullptr)
		return;
	c_engine->SetGPUProfilingEnabled(enabled);
})
#pragma optimize("",on)
