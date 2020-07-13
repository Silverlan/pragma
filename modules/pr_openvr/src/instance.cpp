#include "stdafx_openvr.h"
#include "vr_instance.hpp"
#include "vr_eye.hpp"
#include "vr_controller_state.hpp"
#include "wvmodule.h"
#include "vrincludes.h"
#include <sharedutils/scope_guard.h>
#include <array>
#include <GLFW/glfw3.h>
#include <image/prosper_render_target.hpp>
#include <prosper_context.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_fence.hpp>
#include <shader/prosper_shader.hpp>
#include <prosper_util.hpp>
#include <pragma/c_engine.h>
#include <pragma/iscene.h>
#include <pragma/game/c_game.h>
#include <pragma/entities/environment/c_env_camera.h>
#include <sharedutils/util_string.h>
#ifdef _DEBUG
#include <iostream>
#endif

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

using namespace openvr;

#pragma optimize("",off)
std::string openvr::to_string(vr::VREvent_t ev)
{
	switch(ev.eventType)
	{
		case vr::EVREventType::VREvent_None:
			return "None";
		case vr::EVREventType::VREvent_TrackedDeviceActivated:
			return "Tracked device activated";
		case vr::EVREventType::VREvent_TrackedDeviceDeactivated:
			return "Tracked device deactivated";
		case vr::EVREventType::VREvent_TrackedDeviceUpdated:
			return "Tracked device updated";
		case vr::EVREventType::VREvent_TrackedDeviceUserInteractionStarted:
			return "Tracked device user interaction started";
		case vr::EVREventType::VREvent_TrackedDeviceUserInteractionEnded:
			return "Tracked device user interaction ended";
		case vr::EVREventType::VREvent_IpdChanged:
			return "Ipd changed";
		case vr::EVREventType::VREvent_EnterStandbyMode:
			return "Enter standby mode";
		case vr::EVREventType::VREvent_LeaveStandbyMode:
			return "Leave standby mode";
		case vr::EVREventType::VREvent_TrackedDeviceRoleChanged:
			return "Tracked device role changed";
		case vr::EVREventType::VREvent_ButtonPress:
			return "Button press";
		case vr::EVREventType::VREvent_ButtonUnpress:
			return "Button unpress";
		case vr::EVREventType::VREvent_ButtonTouch:
			return "Button touch";
		case vr::EVREventType::VREvent_ButtonUntouch:
			return "Button untouch";
		case vr::EVREventType::VREvent_MouseMove:
			return "Mouse move";
		case vr::EVREventType::VREvent_MouseButtonDown:
			return "Mouse button down";
		case vr::EVREventType::VREvent_MouseButtonUp:
			return "Mouse button up";
		case vr::EVREventType::VREvent_FocusEnter:
			return "Focus enter";
		case vr::EVREventType::VREvent_FocusLeave:
			return "Focus leave";
		case vr::EVREventType::VREvent_ScrollDiscrete:
			return "ScrollDiscrete";
		case vr::EVREventType::VREvent_ScrollSmooth:
			return "ScrollSmooth";
		case vr::EVREventType::VREvent_TouchPadMove:
			return "Touch pad move";
		case vr::EVREventType::VREvent_InputFocusCaptured:
			return "Input focus captured";
		case vr::EVREventType::VREvent_InputFocusReleased:
			return "Input focus released";
		case vr::EVREventType::VREvent_SceneApplicationChanged:
			return "Scene application changed";
		case vr::EVREventType::VREvent_SceneFocusChanged:
			return "Scene focus changed";
		case vr::EVREventType::VREvent_HideRenderModels:
			return "Hide render models";
		case vr::EVREventType::VREvent_ShowRenderModels:
			return "Show render models";
		case vr::EVREventType::VREvent_OverlayShown:
			return "Overlay shown";
		case vr::EVREventType::VREvent_OverlayHidden:
			return "Overlay hidden";
		case vr::EVREventType::VREvent_DashboardActivated:
			return "Dashboard activated";
		case vr::EVREventType::VREvent_DashboardDeactivated:
			return "Dashboard deactivated";
		case vr::EVREventType::VREvent_DashboardRequested:
			return "Dashboard requested";
		case vr::EVREventType::VREvent_ResetDashboard:
			return "Reset dashboard";
		case vr::EVREventType::VREvent_RenderToast:
			return "Render toast";
		case vr::EVREventType::VREvent_ImageLoaded:
			return "Image loaded";
		case vr::EVREventType::VREvent_ShowKeyboard:
			return "Show keyboard";
		case vr::EVREventType::VREvent_HideKeyboard:
			return "Hide keyboard";
		case vr::EVREventType::VREvent_OverlayGamepadFocusGained:
			return "Overlay gamepad focus gained";
		case vr::EVREventType::VREvent_OverlayGamepadFocusLost:
			return "Overlay gamepad focus lost";
		case vr::EVREventType::VREvent_OverlaySharedTextureChanged:
			return "Overlay shared texture changed";
		case vr::EVREventType::VREvent_Notification_Shown:
			return "Notification shown";
		case vr::EVREventType::VREvent_Notification_Hidden:
			return "Notification hidden";
		case vr::EVREventType::VREvent_Notification_BeginInteraction:
			return "Begin interaction";
		case vr::EVREventType::VREvent_Notification_Destroyed:
			return "Notification destroyed";
		case vr::EVREventType::VREvent_Quit:
			return "Quit";
		case vr::EVREventType::VREvent_ProcessQuit:
			return "Process quit";
		case vr::EVREventType::VREvent_QuitAcknowledged:
			return "Quit acknowledged";
		case vr::EVREventType::VREvent_ChaperoneDataHasChanged:
			return "Chaperone data has changed";
		case vr::EVREventType::VREvent_ChaperoneUniverseHasChanged:
			return "Chaperone universe has changed";
		case vr::EVREventType::VREvent_ChaperoneTempDataHasChanged:
			return "Chaperone temp data has changed";
		case vr::EVREventType::VREvent_ChaperoneSettingsHaveChanged:
			return "Chaperone settings have changed";
		case vr::EVREventType::VREvent_SeatedZeroPoseReset:
			return "Seated zero pose reset";
		case vr::EVREventType::VREvent_BackgroundSettingHasChanged:
			return "Background setting has changed";
		case vr::EVREventType::VREvent_CameraSettingsHaveChanged:
			return "Camera settings have changed";
		case vr::EVREventType::VREvent_StatusUpdate:
			return "Status update";
		case vr::EVREventType::VREvent_MCImageUpdated:
			return "MC image updated";
		case vr::EVREventType::VREvent_FirmwareUpdateStarted:
			return "Firmware update started";
		case vr::EVREventType::VREvent_FirmwareUpdateFinished:
			return "Firmware update finished";
		case vr::EVREventType::VREvent_KeyboardClosed:
			return "Keyboard closed";
		case vr::EVREventType::VREvent_KeyboardCharInput:
			return "Keyboard char input";
		case vr::EVREventType::VREvent_KeyboardDone:
			return "Keyboard done";
		case vr::EVREventType::VREvent_Compositor_ChaperoneBoundsShown:
			return "Composition chaperone bounds shown";
		case vr::EVREventType::VREvent_Compositor_ChaperoneBoundsHidden:
			return "Compositor chaperone bounds hidden";
		case vr::EVREventType::VREvent_TrackedCamera_StartVideoStream:
			return "Tracked camera start video stream";
		case vr::EVREventType::VREvent_TrackedCamera_StopVideoStream:
			return "Tracked camera stop video stream";
		case vr::EVREventType::VREvent_TrackedCamera_PauseVideoStream:
			return "Tracked camera paused video stream";
		case vr::EVREventType::VREvent_TrackedCamera_ResumeVideoStream:
			return "Tracked camera resume video stream";
		case vr::EVREventType::VREvent_PerformanceTest_EnableCapture:
			return "Performance test enable capture";
		case vr::EVREventType::VREvent_PerformanceTest_DisableCapture:
			return "Performance test disable capture";
		case vr::EVREventType::VREvent_PerformanceTest_FidelityLevel:
			return "Performance test fidelity level";
	}
	return "Invalid";
}

std::string openvr::to_string(vr::VRCompositorError err)
{
	switch(err)
	{
		case vr::VRCompositorError::VRCompositorError_None:
			return "None";
		case vr::VRCompositorError::VRCompositorError_IncompatibleVersion:
			return "Incompatible version";
		case vr::VRCompositorError::VRCompositorError_DoNotHaveFocus:
			return "Do not have focus";
		case vr::VRCompositorError::VRCompositorError_InvalidTexture:
			return "Invalid texture";
		case vr::VRCompositorError::VRCompositorError_IsNotSceneApplication:
			return "Is not scene application";
		case vr::VRCompositorError::VRCompositorError_TextureIsOnWrongDevice:
			return "Texture is on wrong device";
		case vr::VRCompositorError::VRCompositorError_TextureUsesUnsupportedFormat:
			return "Texture uses unsupported format";
		case vr::VRCompositorError::VRCompositorError_SharedTexturesNotSupported:
			return "Shared textures not supported";
		case vr::VRCompositorError::VRCompositorError_IndexOutOfRange:
			return "Index out of range";
	}
	return "Invalid";
}

std::string openvr::to_string(vr::ETrackedPropertyError err)
{
	switch(err)
	{
		case vr::ETrackedPropertyError::TrackedProp_Success:
			return "Success";
		case vr::ETrackedPropertyError::TrackedProp_WrongDataType:
			return "Wrong data type";
		case vr::ETrackedPropertyError::TrackedProp_WrongDeviceClass:
			return "Wrong device class";
		case vr::ETrackedPropertyError::TrackedProp_BufferTooSmall:
			return "Buffer too small";
		case vr::ETrackedPropertyError::TrackedProp_UnknownProperty:
			return "Unknown property";
		case vr::ETrackedPropertyError::TrackedProp_InvalidDevice:
			return "Invalid device";
		case vr::ETrackedPropertyError::TrackedProp_CouldNotContactServer:
			return "Could not contact server";
		case vr::ETrackedPropertyError::TrackedProp_ValueNotProvidedByDevice:
			return "Value not provided by device";
		case vr::ETrackedPropertyError::TrackedProp_StringExceedsMaximumLength:
			return "String exceeds maximum length";
		case vr::ETrackedPropertyError::TrackedProp_NotYetAvailable:
			return "Not yet available";
	}
	return "Invalid";
}

std::string openvr::to_string(vr::EVRInitError err)
{
	return vr::VR_GetVRInitErrorAsEnglishDescription(err);
}

//////////////////////////////////

Instance::Instance(vr::IVRSystem *system,uint32_t width,uint32_t height,vr::IVRRenderModels *i,vr::IVRCompositor *compositor
#ifdef USE_OPENGL_OFFSCREEN_CONTEXT
	,GLFWwindow *window
#endif
)
	: m_system{system},m_renderInterface{i},m_compositor{compositor},
	m_width(width),m_height(height)
#ifdef USE_OPENGL_OFFSCREEN_CONTEXT
	,m_window{window}
#endif
{
	m_leftEye = std::make_unique<Eye>(this,vr::EVREye::Eye_Left);
	m_rightEye = std::make_unique<Eye>(this,vr::EVREye::Eye_Right);
	auto *shaderFlip = IState::get_shader("screen_flip_y");
	m_hShaderFlip = (shaderFlip != nullptr) ? shaderFlip->GetHandle() : util::WeakHandle<prosper::Shader>{};
	m_cbThink = m_cbDraw = IState::add_callback(IState::Callback::Think,FunctionCallback<void>::Create([this]() {
		PollEvents();
	}));

	auto renderAPI = c_engine->GetRenderContext().GetAPIIdentifier();
	if(ustring::compare(renderAPI,"OpenGL"))
		m_renderAPI = RenderAPI::OpenGL;
	else if(ustring::compare(renderAPI,"Vulkan"))
		m_renderAPI = RenderAPI::Vulkan;
#ifdef _DEBUG
	m_compositor->ShowMirrorWindow();
#endif
}
Instance::~Instance()
{
	vr::VR_Shutdown();
	if(m_cbThink.IsValid())
		m_cbThink.Remove();
	if(m_cbDraw.IsValid())
		m_cbDraw.Remove();
	if(m_cbDrawGame.IsValid())
		m_cbDrawGame.Remove();
#ifdef USE_OPENGL_OFFSCREEN_CONTEXT
	glfwDestroyWindow(m_window);
#endif
}
void Instance::PollEvents()
{
	vr::VREvent_t event;
	while(m_system->PollNextEvent(&event,sizeof(event)))
		ProcessEvent(event);

	vr::VRControllerState_t state {};
	auto *sys = GetSystemInterface();
	for(auto i=decltype(vr::k_unMaxTrackedDeviceCount){0};i<vr::k_unMaxTrackedDeviceCount;++i)
	{
		if(sys->GetControllerState(i,&state,sizeof(vr::VRControllerState_t)) == false)
			continue;
		auto it = m_controllerStates.find(i);
		if(it == m_controllerStates.end())
		{
			it = m_controllerStates.insert(std::make_pair(i,ControllerState{})).first;
			it->second.SetStateChangeCallback(std::bind(&Instance::OnControllerStateChanged,this,i,std::placeholders::_1,std::placeholders::_2));
		}
		it->second.UpdateState(state);
	}
}
void Instance::OnControllerStateChanged(uint32_t controllerId,uint32_t key,GLFW::KeyState state)
{
	if(m_controllerStateCallback == nullptr)
		return;
	m_controllerStateCallback(controllerId,key,state);
}
void Instance::SetControllerStateCallback(const std::function<void(uint32_t,uint32_t,GLFW::KeyState)> &callback) {m_controllerStateCallback = callback;}
bool Instance::InitializeScene()
{
	if(m_cbDraw.IsValid())
		m_cbDraw.Remove();
	if(m_cbDrawGame.IsValid())
		m_cbDrawGame.Remove();
	//RegisterCallbackWithOptionalReturn<>("DrawScene");
	m_cbDraw = IState::add_callback(IState::Callback::EngineDraw,FunctionCallback<void>::Create([this]() {
		if(m_bHmdViewEnabled == false)
			return;
		DrawScene();
		return;
	}));
	m_cbDrawGame = IState::add_callback(IState::Callback::DrawScene,FunctionCallback<
		bool,std::reference_wrapper<std::shared_ptr<prosper::IPrimaryCommandBuffer>>,prosper::IImage*
	>::CreateWithOptionalReturn([this](bool *ret,std::reference_wrapper<std::shared_ptr<prosper::IPrimaryCommandBuffer>> drawCmd,prosper::IImage *img) -> CallbackReturnType {
		if(m_bHmdViewEnabled == false)
			return CallbackReturnType::NoReturnValue;
#if LOPENVR_VERBOSE == 1
		std::cout<<"[VR] Drawing scene..."<<std::endl;
#endif
		//DrawScene(drawCmd.get());


		/*auto &oldScene = GetRenderScene();
		static std::shared_ptr<Scene> newScene = nullptr;
		if(newScene == nullptr)
			newScene = Scene::Create(45.f,oldScene->GetViewFOV(),oldScene->GetAspectRatio(),oldScene->GetZNear(),oldScene->GetZFar());
		newScene->camera.SetPos(oldScene->camera.GetPos());
		newScene->camera.SetForward(oldScene->camera.GetForward());
		newScene->camera.SetUp(oldScene->camera.GetUp());
		newScene->camera.UpdateViewMatrix();
		SetRenderScene(newScene);
		Render(rp.get(),fb.get(),cmdBuffer);
		SetRenderScene(nullptr);*/
		//return true;
		*ret = true; // Override default rendering
		return CallbackReturnType::HasReturnValue; // Required to make it take our return value into account
	}));
	auto &context = IState::get_render_context();
	auto w = m_width;
	auto h = m_height;
#if LOPENVR_VERBOSE == 1
		std::cout<<"[VR] Initializing eyes..."<<std::endl;
#endif
	return (m_leftEye->Initialize(w,h) == true && m_rightEye->Initialize(w,h) == true) ? true : false;
}
void Instance::ClearScene()
{
	m_leftEye->vkRenderTarget = nullptr;
	m_rightEye->vkRenderTarget = nullptr;
}
uint32_t Instance::GetWidth() const {return m_width;}
uint32_t Instance::GetHeight() const {return m_height;}
const openvr::Eye &Instance::GetLeftEye() const {return *m_leftEye;}
const openvr::Eye &Instance::GetRightEye() const {return *m_rightEye;}

Mat4 openvr::steam_vr_matrix_to_engine_matrix(const vr::HmdMatrix34_t &matPose)
{
	return Mat4(
		matPose.m[0][0],matPose.m[1][0],matPose.m[2][0],0.f,
		matPose.m[0][1],matPose.m[1][1],matPose.m[2][1],0.f,
		matPose.m[0][2],matPose.m[1][2],matPose.m[2][2],0.f,
		matPose.m[0][3],matPose.m[1][3],matPose.m[2][3],1.f
	);
}

const Mat4 &Instance::GetHMDPoseMatrix() const {return m_hmdPoseMatrix;}

void Instance::UpdateHMDPoses()
{
	//auto t = std::chrono::high_resolution_clock::now();
	m_compositor->WaitGetPoses(m_trackedPoses.data(),m_trackedPoses.size(),nullptr,0);
	//auto tDelta = std::chrono::high_resolution_clock::now() -t;
	//std::cout<<"Time passed: "<<(std::chrono::duration_cast<std::chrono::nanoseconds>(tDelta).count() /1'000'000.0)<<"ms"<<std::endl;

	auto validPoseCount = 0;
	auto nDevice = 0u;
	for(auto &pose : m_trackedPoses)
	{
		if(pose.bPoseIsValid == false)
		{
			++nDevice;
			continue;
		}
		validPoseCount++;
		m_poseTransforms.at(nDevice) = steam_vr_matrix_to_engine_matrix(pose.mDeviceToAbsoluteTracking);
		if(m_trackedDeviceClasses.at(nDevice) == 0)
			m_trackedDeviceClasses.at(nDevice) = m_system->GetTrackedDeviceClass(nDevice);
		++nDevice;
	}
	auto &hmdPose = m_trackedPoses.at(vr::k_unTrackedDeviceIndex_Hmd);
	if(hmdPose.bPoseIsValid == true)
	{
		m_hmdPoseMatrix = m_poseTransforms.at(vr::k_unTrackedDeviceIndex_Hmd);
		m_hmdPoseMatrix = glm::inverse(m_hmdPoseMatrix);
	}
}

static bool check_error(vr::EVRCompositorError err)
{
	if(err == vr::EVRCompositorError::VRCompositorError_None)
		return true;
	std::cout<<"[VR] Eye submit error";
#ifdef USE_OPENGL_OFFSCREEN_CONTEXT
	std::cout<<" (Texture: "<<eye->texture<<")";
#endif
	std::cout<<": "<<openvr::to_string(err)<<std::endl;
	return false;
};

#include <glm/gtx/matrix_decompose.hpp>
#include <iostream>
void Instance::DrawScene()
{
	/*static std::array<vr::TrackedDevicePose_t,vr::k_unMaxTrackedDeviceCount> renderPoses {};
	auto t = std::chrono::high_resolution_clock::now();
	m_compositor->WaitGetPoses(renderPoses.data(),renderPoses.size(),nullptr,0);
	auto tDelta = std::chrono::high_resolution_clock::now() -t;
	std::cout<<"Time passed: "<<(std::chrono::duration_cast<std::chrono::nanoseconds>(tDelta).count() /1'000'000.0)<<"ms"<<std::endl;


	// Change Begin
	// for somebody asking for the default figure out the time from now to photons.
	float fSecondsSinceLastVsync;

	m_system->GetTimeSinceLastVsync(&fSecondsSinceLastVsync, NULL);

	float fDisplayFrequency = m_system->GetFloatTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_DisplayFrequency_Float);
	float fFrameDuration = 1.f / fDisplayFrequency;
	float fVsyncToPhotons = m_system->GetFloatTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SecondsFromVsyncToPhotons_Float);

	float fPredictedSecondsFromNow = (fFrameDuration *2.f) - fSecondsSinceLastVsync + fVsyncToPhotons;

	m_system->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseStanding, fPredictedSecondsFromNow, renderPoses.data(), vr::k_unMaxTrackedDeviceCount);




	for(auto i=decltype(renderPoses.size()){0};i<renderPoses.size();++i)
	{
		auto &pose = renderPoses.at(i);
		if(pose.bPoseIsValid == false)
			continue;
		m_poseTransforms.at(i) = steam_vr_matrix_to_engine_matrix(pose.mDeviceToAbsoluteTracking);
	}
	Mat4 matHmd {};
	auto &hmdPose = renderPoses.at(vr::k_unTrackedDeviceIndex_Hmd);
	if(hmdPose.bPoseIsValid == true)
	{
		matHmd = m_poseTransforms.at(vr::k_unTrackedDeviceIndex_Hmd);
		matHmd = glm::inverse(matHmd);
	}*/
	/*
	float fSecondsSinceLastVsync;

	m_system->GetTimeSinceLastVsync(&fSecondsSinceLastVsync, NULL);

	float fDisplayFrequency = m_system->GetFloatTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_DisplayFrequency_Float);
	float fFrameDuration = 1.f / fDisplayFrequency;
	float fVsyncToPhotons = m_system->GetFloatTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SecondsFromVsyncToPhotons_Float);

	float fPredictedSecondsFromNow = (fFrameDuration *2.f) - fSecondsSinceLastVsync + fVsyncToPhotons;

	m_system->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseStanding, fPredictedSecondsFromNow, renderPoses.data(), vr::k_unMaxTrackedDeviceCount);
	



	for(auto i=decltype(renderPoses.size()){0};i<renderPoses.size();++i)
	{
		auto &pose = renderPoses.at(i);
		if(pose.bPoseIsValid == false)
			continue;
		m_poseTransforms.at(i) = steam_vr_matrix_to_engine_matrix(pose.mDeviceToAbsoluteTracking);
	}
	Mat4 matHmd {};
	auto &hmdPose = renderPoses.at(vr::k_unTrackedDeviceIndex_Hmd);
	if(hmdPose.bPoseIsValid == true)
	{
		matHmd = m_poseTransforms.at(vr::k_unTrackedDeviceIndex_Hmd);
		matHmd = glm::inverse(matHmd);
	}
	*/

	auto gameScene = IState::get_render_scene();
	auto camGame = gameScene.GetInternalScene().GetActiveCamera();
	if(camGame.expired())
		return;
	camGame->UpdateViewMatrix();
	std::array<Eye*,2> eyes = {m_leftEye.get(),m_rightEye.get()};
	for(auto *eye : eyes)
	{
		auto &rt = eye->vkRenderTarget;
		auto &vkTexture = rt->GetTexture();
		auto &vkImg = vkTexture.GetImage();
		auto &cam = eye->camera;
		if(cam.expired())
			continue;
		cam->SetViewMatrix(camGame->GetViewMatrix());
		auto mViewCam = cam->GetViewMatrix();
		auto mView = m_hmdPoseMatrix *eye->GetEyeViewMatrix(*cam);
		cam->SetViewMatrix(mView);

		auto scene = c_game->GetRenderScene();
		auto *renderer = scene->GetRenderer();
		scene->SetRenderer(*renderer);

		// TODO
		//auto mProj = eye->GetEyeProjectionMatrix(scene.GetZNear(),scene.GetZFar());
		//scene.SetProjectionMatrix(reinterpret_cast<IMat4&>(mProj));

		//scene.CullLightSources();

		auto drawCmdInfo = StartRecording();
		if(drawCmdInfo.has_value())
		{
#ifdef USE_VULKAN
			drawCmdInfo->commandBuffer->RecordImageBarrier(eye->vkRenderTarget->GetTexture().GetImage(),prosper::ImageLayout::TransferSrcOptimal,prosper::ImageLayout::ColorAttachmentOptimal);
			//prosper::util::record_image_barrier(**drawCmdInfo.commandBuffer,**eye->vkRenderTarget->GetTexture()->GetImage(),Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL);
			//prosper::util::record_clear_image(**drawCmdInfo.commandBuffer,**vkImg,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,std::array<float,4>{1.f,0.f,0.f,1.f});
			//prosper::util::record_image_barrier(**drawCmdInfo.commandBuffer,**eye->vkRenderTarget->GetTexture()->GetImage(),Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);

#endif
			IState::draw_scene(scene,drawCmdInfo->commandBuffer,rt);
			eye->UpdateImage(vkImg);

#ifdef USE_VULKAN
			drawCmdInfo->commandBuffer->RecordImageBarrier(eye->vkRenderTarget->GetTexture().GetImage(),prosper::ImageLayout::ColorAttachmentOptimal,prosper::ImageLayout::TransferSrcOptimal);
#endif

			StopRecording();
		}
		// Reset old view matrix
		cam->SetViewMatrix(mViewCam);
	}
	for(auto *eye : eyes)
		check_error(m_compositor->Submit(eye->eye,&eye->vrTexture));

	UpdateHMDPoses();
}
std::optional<Instance::CommandBufferInfo> Instance::StartRecording()
{
	StopRecording(); // Note: Recording should already have been stopped by the user at this point, this is just a failsafe!
	if(m_commandBuffers.empty() == false && m_commandBuffers.back().fence->IsSet() == true)
	{
		auto cmdBufferInfo = m_commandBuffers.back();
		cmdBufferInfo.commandBuffer->Reset(true);
		cmdBufferInfo.fence->Reset();
		m_commandBuffers.pop_back();
		if(cmdBufferInfo.commandBuffer->StartRecording() == false)
			return {};
		m_activeCommandBuffer = cmdBufferInfo;
		return m_activeCommandBuffer;
	}
	auto &context = const_cast<prosper::IPrContext&>(IState::get_render_context());
	uint32_t universalQueueFamilyIndex;
	auto cmdBuffer = context.AllocatePrimaryLevelCommandBuffer(prosper::QueueFamilyType::Universal,universalQueueFamilyIndex);
	auto fence = context.CreateFence();
	if(cmdBuffer->StartRecording() == false)
		return {};
	m_activeCommandBuffer = CommandBufferInfo{cmdBuffer,fence};
	return m_activeCommandBuffer;
}
void Instance::StopRecording()
{
	if(m_activeCommandBuffer.has_value() == false)
		return;
	m_activeCommandBuffer->commandBuffer->StopRecording();
	check_error(m_compositor->SubmitExplicitTimingData());
	m_activeCommandBuffer->commandBuffer->GetContext().SubmitCommandBuffer(*m_activeCommandBuffer->commandBuffer,false,m_activeCommandBuffer->fence.get());
	m_commandBuffers.push_front(*m_activeCommandBuffer);

	m_activeCommandBuffer = {};
}
vr::IVRSystem *Instance::GetSystemInterface() {return m_system;}
vr::IVRRenderModels *Instance::GetRenderInterface() {return m_renderInterface;}
vr::IVRCompositor *Instance::GetCompositorInterface() {return m_compositor;}

void Instance::FadeToColor(Color col,float tFade,bool bBackground)
{
	m_compositor->FadeToColor(tFade,static_cast<float>(col.r) /255.f,static_cast<float>(col.g) /255.f,static_cast<float>(col.b) /255.f,static_cast<float>(col.a) /255.f,bBackground);
}
void Instance::FadeGrid(float tFade,bool bFadeIn) {m_compositor->FadeGrid(tFade,bFadeIn);}
void Instance::ShowMirrorWindow() {m_compositor->ShowMirrorWindow();}
void Instance::HideMirrorWindow() {m_compositor->HideMirrorWindow();}
bool Instance::IsMirrorWindowVisible() const {return m_compositor->IsMirrorWindowVisible();}
void Instance::SetHmdViewEnabled(bool b) {m_bHmdViewEnabled = b;}
bool Instance::IsHmdViewEnabled() const {return m_bHmdViewEnabled;}

void Instance::ProcessEvent(vr::VREvent_t ev)
{
#ifdef _DEBUG
	std::cout<<"[VR] Event: "<<openvr::to_string(ev)<<std::endl;
#endif
	auto &data = ev.data;
	if(ev.eventType == vr::EVREventType::VREvent_ButtonPress)
	{
		auto &keyboard = data.keyboard;
		//keyboard.
	}
	/*switch(ev.eventType)
	{
	case vr::VREvent_TrackedDeviceActivated:
		{
			//SetupRenderModelForTrackedDevice( event.trackedDeviceIndex );
#ifdef _DEBUG
			std::cout<<"Device "<<ev.trackedDeviceIndex<<" attached!"<<std::endl;
#endif
		}
		break;
	case vr::VREvent_TrackedDeviceDeactivated:
		{
#ifdef _DEBUG
			std::cout<<"Device "<<ev.trackedDeviceIndex<<" deactivated!"<<std::endl;
#endif
		}
		break;
	case vr::VREvent_TrackedDeviceUpdated:
		{
#ifdef _DEBUG
			std::cout<<"Device "<<ev.trackedDeviceIndex<<" updated!"<<std::endl;
#endif
		}
		break;
	}*/
}
std::string Instance::GetTrackedDeviceString(vr::TrackedDeviceProperty prop,vr::TrackedPropertyError *peError) const
{
	auto unRequiredBufferLen = m_system->GetStringTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd,prop,nullptr,0,peError);
	if(unRequiredBufferLen == 0)
		return "";
	std::vector<char> r(unRequiredBufferLen);
	unRequiredBufferLen = m_system->GetStringTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd,prop,r.data(),unRequiredBufferLen,peError);
	return std::string{r.data(),r.size()};
}
bool Instance::GetTrackedDeviceBool(vr::TrackedDeviceProperty prop,vr::TrackedPropertyError *peError) const
{
	return m_system->GetBoolTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd,prop,peError);
}
float Instance::GetTrackedDeviceFloat(vr::TrackedDeviceProperty prop,vr::TrackedPropertyError *peError) const
{
	return m_system->GetFloatTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd,prop,peError);
}
int32_t Instance::GetTrackedDeviceInt32(vr::TrackedDeviceProperty prop,vr::TrackedPropertyError *peError) const
{
	return m_system->GetInt32TrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd,prop,peError);
}
uint64_t Instance::GetTrackedDeviceUInt64(vr::TrackedDeviceProperty prop,vr::TrackedPropertyError *peError) const
{
	return m_system->GetUint64TrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd,prop,peError);
}
glm::mat3x4 Instance::GetTrackedDeviceMatrix34(vr::TrackedDeviceProperty prop,vr::TrackedPropertyError *peError) const
{
	auto m = m_system->GetMatrix34TrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd,prop,peError);
	glm::mat3x4 r{};
	for(int32_t i=0;i<3;++i)
	{
		for(int32_t j=0;j<4;++j)
			r[i][j] = m.m[i][j];
	}
	return r;
}
std::unique_ptr<Instance> Instance::Create(vr::EVRInitError *err,std::vector<std::string> &reqInstanceExtensions,std::vector<std::string> &reqDeviceExtensions)
{
	if(vr::VR_IsHmdPresent() == false)
	{
		if(err != nullptr)
			*err = vr::EVRInitError::VRInitError_Init_HmdNotFound;
		return nullptr;
	}
	auto *ivr = vr::VR_Init(err,vr::EVRApplicationType::VRApplication_Scene);
	if(ivr == nullptr)
		return nullptr;
	ScopeGuard guard{[]() {
		vr::VR_Shutdown();
	}};
	auto *pRenderModels = static_cast<vr::IVRRenderModels*>(vr::VR_GetGenericInterface(vr::IVRRenderModels_Version,err));
	if(pRenderModels == nullptr)
		return nullptr;
	auto *pCompositor = vr::VRCompositor();
	if(pCompositor == nullptr)
	{
		if(err != nullptr)
			*err = vr::EVRInitError::VRInitError_Compositor_Failed;
		return nullptr;
	}

#ifdef USE_VULKAN
	auto instanceExtLen = pCompositor->GetVulkanInstanceExtensionsRequired(nullptr,0);
	std::string instanceExt;
	if(instanceExtLen > 0)
	{
		instanceExt.resize(instanceExtLen);
		pCompositor->GetVulkanInstanceExtensionsRequired(const_cast<char*>(instanceExt.data()),0);
	}
	auto &vkContext = IState::get_render_context();
	auto *vkDevice = static_cast<VkPhysicalDevice_T*>(vkContext.GetInternalPhysicalDevice());
	auto deviceExtLen = pCompositor->GetVulkanDeviceExtensionsRequired(vkDevice,nullptr,0);
	std::string deviceExt;
	if(deviceExtLen > 0)
	{
		deviceExt.resize(deviceExtLen);
		pCompositor->GetVulkanDeviceExtensionsRequired(vkDevice,const_cast<char*>(deviceExt.data()),0);
	}
	pCompositor->SetExplicitTimingMode(vr::EVRCompositorTimingMode::VRCompositorTimingMode_Explicit_RuntimePerformsPostPresentHandoff);

	ustring::explode(instanceExt," ",reqInstanceExtensions);
	ustring::explode(deviceExt," ",reqDeviceExtensions);
	std::cout<<"[VR] Instance Extensions Required: "<<instanceExt<<std::endl;
	std::cout<<"[VR] Device Extensions Required: "<<deviceExt<<std::endl;

	for(auto &ext : reqInstanceExtensions)
	{
		if(vkContext.IsInstanceExtensionEnabled(ext) == false)
		{
			Con::cerr<<"[VR] ERROR: Required instance extension '"<<ext<<"' is not enabled!"<<Con::endl;
			break;
		}
	}
	for(auto &ext : reqDeviceExtensions)
	{
		if(vkContext.IsDeviceExtensionEnabled(ext) == false)
		{
			Con::cerr<<"[VR] ERROR: Required device extension '"<<ext<<"' is not enabled!"<<Con::endl;
			break;
		}
	}
#endif

	auto width = 1280u;
	auto height = 1024u;
	ivr->GetRecommendedRenderTargetSize(&width,&height);

#ifdef USE_OPENGL_OFFSCREEN_CONTEXT
	// Initialize GLFW Offscreen Context
	glfwSetErrorCallback([](int error, const char* description) {
		std::cout<<"GLFW ERROR: "<<error<<"; "<<description<<std::endl;
	}); // TODO
	if(glfwInit() == GL_FALSE)
	{
		if(err != nullptr)
			*err = vr::EVRInitError::VRInitError_Unknown;
		return nullptr;
	}
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE,GL_FALSE);
	glfwWindowHint(GLFW_VISIBLE,GL_FALSE);
	glfwWindowHint(GLFW_CLIENT_API,GLFW_OPENGL_API);
	
	auto *window = glfwCreateWindow(width,height,"Sciolyte VR Offscreen Context",nullptr,nullptr);
	if(window == nullptr)
	{
		if(err != nullptr)
			*err = vr::EVRInitError::VRInitError_Unknown;
		return nullptr;
	}
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	auto r = glewInit();
	if(r != GLEW_OK)
	{
		if(err != nullptr)
			*err = vr::EVRInitError::VRInitError_Unknown;
		return nullptr;
	}
	//
#endif

	guard.dismiss();
	auto instance = std::unique_ptr<Instance>{new Instance{ivr,width,height,pRenderModels,pCompositor
#ifdef USE_OPENGL_OFFSCREEN_CONTEXT
		,window
#endif
	}};
	return instance;
}

std::string Instance::GetTrackingSystemName(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceString(vr::TrackedDeviceProperty::Prop_TrackingSystemName_String,peError);}
std::string Instance::GetModelNumber(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceString(vr::TrackedDeviceProperty::Prop_ModelNumber_String,peError);}
std::string Instance::GetSerialNumber(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceString(vr::TrackedDeviceProperty::Prop_SerialNumber_String,peError);}
std::string Instance::GetRenderModelName(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceString(vr::TrackedDeviceProperty::Prop_RenderModelName_String,peError);}
std::string Instance::GetManufacturerName(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceString(vr::TrackedDeviceProperty::Prop_ManufacturerName_String,peError);}
std::string Instance::GetTrackingFirmwareVersion(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceString(vr::TrackedDeviceProperty::Prop_TrackingFirmwareVersion_String,peError);}
std::string Instance::GetHardwareRevision(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceString(vr::TrackedDeviceProperty::Prop_HardwareRevision_String,peError);}
std::string Instance::GetAllWirelessDongleDescriptions(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceString(vr::TrackedDeviceProperty::Prop_AllWirelessDongleDescriptions_String,peError);}
std::string Instance::GetConnectedWirelessDongle(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceString(vr::TrackedDeviceProperty::Prop_ConnectedWirelessDongle_String,peError);}
std::string Instance::GetFirmwareManualUpdateURL(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceString(vr::TrackedDeviceProperty::Prop_Firmware_ManualUpdateURL_String,peError);}
std::string Instance::GetFirmwareProgrammingTarget(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceString(vr::TrackedDeviceProperty::Prop_Firmware_ProgrammingTarget_String,peError);}
std::string Instance::GetDisplayMCImageLeft(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceString(vr::TrackedDeviceProperty::Prop_DisplayMCImageLeft_String,peError);}
std::string Instance::GetDisplayMCImageRight(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceString(vr::TrackedDeviceProperty::Prop_DisplayMCImageRight_String,peError);}
std::string Instance::GetDisplayGCImage(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceString(vr::TrackedDeviceProperty::Prop_DisplayGCImage_String,peError);}
std::string Instance::GetCameraFirmwareDescription(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceString(vr::TrackedDeviceProperty::Prop_CameraFirmwareDescription_String,peError);}
std::string Instance::GetAttachedDeviceId(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceString(vr::TrackedDeviceProperty::Prop_AttachedDeviceId_String,peError);}
std::string Instance::GetModelLabel(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceString(vr::TrackedDeviceProperty::Prop_ModeLabel_String,peError);}

bool Instance::WillDriftInYaw(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceBool(vr::TrackedDeviceProperty::Prop_WillDriftInYaw_Bool,peError);}
bool Instance::DeviceIsWireless(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceBool(vr::TrackedDeviceProperty::Prop_DeviceIsWireless_Bool,peError);}
bool Instance::DeviceIsCharging(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceBool(vr::TrackedDeviceProperty::Prop_DeviceIsCharging_Bool,peError);}
bool Instance::FirmwareUpdateAvailable(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceBool(vr::TrackedDeviceProperty::Prop_Firmware_UpdateAvailable_Bool,peError);}
bool Instance::FirmwareManualUpdate(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceBool(vr::TrackedDeviceProperty::Prop_Firmware_ManualUpdate_Bool,peError);}
bool Instance::BlockServerShutdown(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceBool(vr::TrackedDeviceProperty::Prop_BlockServerShutdown_Bool,peError);}
bool Instance::CanUnifyCoordinateSystemWithHmd(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceBool(vr::TrackedDeviceProperty::Prop_CanUnifyCoordinateSystemWithHmd_Bool,peError);}
bool Instance::ContainsProximitySensor(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceBool(vr::TrackedDeviceProperty::Prop_ContainsProximitySensor_Bool,peError);}
bool Instance::DeviceProvidesBatteryStatus(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceBool(vr::TrackedDeviceProperty::Prop_DeviceProvidesBatteryStatus_Bool,peError);}
bool Instance::DeviceCanPowerOff(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceBool(vr::TrackedDeviceProperty::Prop_DeviceCanPowerOff_Bool,peError);}
bool Instance::HasCamera(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceBool(vr::TrackedDeviceProperty::Prop_HasCamera_Bool,peError);}
bool Instance::ReportsTimeSinceVSync(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceBool(vr::TrackedDeviceProperty::Prop_ReportsTimeSinceVSync_Bool,peError);}
bool Instance::IsOnDesktop(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceBool(vr::TrackedDeviceProperty::Prop_IsOnDesktop_Bool,peError);}

float Instance::GetDeviceBatteryPercentage(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceFloat(vr::TrackedDeviceProperty::Prop_DeviceBatteryPercentage_Float,peError);}
float Instance::GetSecondsFromVsyncToPhotons(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceFloat(vr::TrackedDeviceProperty::Prop_SecondsFromVsyncToPhotons_Float,peError);}
float Instance::GetDisplayFrequency(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceFloat(vr::TrackedDeviceProperty::Prop_DisplayFrequency_Float,peError);}
float Instance::GetUserIpdMeters(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceFloat(vr::TrackedDeviceProperty::Prop_UserIpdMeters_Float,peError);}
float Instance::GetDisplayMCOffset(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceFloat(vr::TrackedDeviceProperty::Prop_DisplayMCOffset_Float,peError);}
float Instance::GetDisplayMCScale(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceFloat(vr::TrackedDeviceProperty::Prop_DisplayMCScale_Float,peError);}
float Instance::GetDisplayGCBlackClamp(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceFloat(vr::TrackedDeviceProperty::Prop_DisplayGCBlackClamp_Float,peError);}
float Instance::GetDisplayGCOffset(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceFloat(vr::TrackedDeviceProperty::Prop_DisplayGCOffset_Float,peError);}
float Instance::GetDisplayGCScale(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceFloat(vr::TrackedDeviceProperty::Prop_DisplayGCScale_Float,peError);}
float Instance::GetDisplayGCPrescale(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceFloat(vr::TrackedDeviceProperty::Prop_DisplayGCPrescale_Float,peError);}
float Instance::GetLensCenterLeftU(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceFloat(vr::TrackedDeviceProperty::Prop_LensCenterLeftU_Float,peError);}
float Instance::GetLensCenterLeftV(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceFloat(vr::TrackedDeviceProperty::Prop_LensCenterLeftV_Float,peError);}
glm::vec2 Instance::GetLensCenterLeftUV(vr::TrackedPropertyError *peError) const
{
	glm::vec2 uv{};
	uv.x = GetLensCenterLeftU(peError);
	if(peError != nullptr && *peError != vr::TrackedPropertyError::TrackedProp_Success)
		return uv;
	uv.y = GetLensCenterLeftV(peError);
	return uv;
}
float Instance::GetLensCenterRightU(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceFloat(vr::TrackedDeviceProperty::Prop_LensCenterRightU_Float,peError);}
float Instance::GetLensCenterRightV(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceFloat(vr::TrackedDeviceProperty::Prop_LensCenterRightV_Float,peError);}
glm::vec2 Instance::GetLensCenterRightUV(vr::TrackedPropertyError *peError) const
{
	glm::vec2 uv{};
	uv.x = GetLensCenterRightU(peError);
	if(peError != nullptr && *peError != vr::TrackedPropertyError::TrackedProp_Success)
		return uv;
	uv.y = GetLensCenterRightV(peError);
	return uv;
}
float Instance::GetUserHeadToEyeDepthMeters(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceFloat(vr::TrackedDeviceProperty::Prop_UserHeadToEyeDepthMeters_Float,peError);}
float Instance::GetFieldOfViewLeftDegrees(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceFloat(vr::TrackedDeviceProperty::Prop_FieldOfViewLeftDegrees_Float,peError);}
float Instance::GetFieldOfViewRightDegrees(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceFloat(vr::TrackedDeviceProperty::Prop_FieldOfViewRightDegrees_Float,peError);}
float Instance::GetFieldOfViewTopDegrees(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceFloat(vr::TrackedDeviceProperty::Prop_FieldOfViewTopDegrees_Float,peError);}
float Instance::GetFieldOfViewBottomDegrees(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceFloat(vr::TrackedDeviceProperty::Prop_FieldOfViewBottomDegrees_Float,peError);}
float Instance::GetTrackingRangeMinimumMeters(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceFloat(vr::TrackedDeviceProperty::Prop_TrackingRangeMinimumMeters_Float,peError);}
float Instance::GetTrackingRangeMaximumMeters(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceFloat(vr::TrackedDeviceProperty::Prop_TrackingRangeMaximumMeters_Float,peError);}

glm::mat3x4 Instance::GetStatusDisplayTransform(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceMatrix34(vr::TrackedDeviceProperty::Prop_StatusDisplayTransform_Matrix34,peError);}
glm::mat3x4 Instance::GetCameraToHeadTransform(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceMatrix34(vr::TrackedDeviceProperty::Prop_CameraToHeadTransform_Matrix34,peError);}

uint64_t Instance::GetHardwareRevisionNumber(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceUInt64(vr::TrackedDeviceProperty::Prop_HardwareRevision_Uint64,peError);}
uint64_t Instance::GetFirmwareVersion(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceUInt64(vr::TrackedDeviceProperty::Prop_FirmwareVersion_Uint64,peError);}
uint64_t Instance::GetFPGAVersion(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceUInt64(vr::TrackedDeviceProperty::Prop_FPGAVersion_Uint64,peError);}
uint64_t Instance::GetVRCVersion(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceUInt64(vr::TrackedDeviceProperty::Prop_VRCVersion_Uint64,peError);}
uint64_t Instance::GetRadioVersion(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceUInt64(vr::TrackedDeviceProperty::Prop_RadioVersion_Uint64,peError);}
uint64_t Instance::GetDongleVersion(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceUInt64(vr::TrackedDeviceProperty::Prop_DongleVersion_Uint64,peError);}
uint64_t Instance::GetCurrentUniverseId(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceUInt64(vr::TrackedDeviceProperty::Prop_CurrentUniverseId_Uint64,peError);}
uint64_t Instance::GetPreviousUniverseId(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceUInt64(vr::TrackedDeviceProperty::Prop_PreviousUniverseId_Uint64,peError);}
uint64_t Instance::GetDisplayFirmwareVersion(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceUInt64(vr::TrackedDeviceProperty::Prop_DisplayFirmwareVersion_Uint64,peError);}
uint64_t Instance::GetCameraFirmwareVersion(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceUInt64(vr::TrackedDeviceProperty::Prop_CameraFirmwareVersion_Uint64,peError);}
uint64_t Instance::GetDisplayFPGAVersion(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceUInt64(vr::TrackedDeviceProperty::Prop_DisplayFPGAVersion_Uint64,peError);}
uint64_t Instance::GetDisplayBootloaderVersion(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceUInt64(vr::TrackedDeviceProperty::Prop_DisplayBootloaderVersion_Uint64,peError);}
uint64_t Instance::GetDisplayHardwareVersion(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceUInt64(vr::TrackedDeviceProperty::Prop_DisplayHardwareVersion_Uint64,peError);}
uint64_t Instance::GetAudioFirmwareVersion(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceUInt64(vr::TrackedDeviceProperty::Prop_AudioFirmwareVersion_Uint64,peError);}
uint64_t Instance::GetSupportedButtons(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceUInt64(vr::TrackedDeviceProperty::Prop_SupportedButtons_Uint64,peError);}

int32_t Instance::GetDeviceClass(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceInt32(vr::TrackedDeviceProperty::Prop_DeviceClass_Int32,peError);}
int32_t Instance::GetDisplayMCType(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceInt32(vr::TrackedDeviceProperty::Prop_DisplayMCType_Int32,peError);}
int32_t Instance::GetEdidVendorID(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceInt32(vr::TrackedDeviceProperty::Prop_EdidVendorID_Int32,peError);}
int32_t Instance::GetEdidProductID(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceInt32(vr::TrackedDeviceProperty::Prop_EdidProductID_Int32,peError);}
int32_t Instance::GetDisplayGCType(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceInt32(vr::TrackedDeviceProperty::Prop_DisplayGCType_Int32,peError);}
int32_t Instance::GetCameraCompatibilityMode(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceInt32(vr::TrackedDeviceProperty::Prop_CameraCompatibilityMode_Int32,peError);}
int32_t Instance::GetAxis0Type(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceInt32(vr::TrackedDeviceProperty::Prop_Axis0Type_Int32,peError);}
int32_t Instance::GetAxis1Type(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceInt32(vr::TrackedDeviceProperty::Prop_Axis1Type_Int32,peError);}
int32_t Instance::GetAxis2Type(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceInt32(vr::TrackedDeviceProperty::Prop_Axis2Type_Int32,peError);}
int32_t Instance::GetAxis3Type(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceInt32(vr::TrackedDeviceProperty::Prop_Axis3Type_Int32,peError);}
int32_t Instance::GetAxis4Type(vr::TrackedPropertyError *peError) const {return GetTrackedDeviceInt32(vr::TrackedDeviceProperty::Prop_Axis4Type_Int32,peError);}

bool Instance::CanRenderScene() const {return m_compositor->CanRenderScene();}
void Instance::ClearLastSubmittedFrame() const {m_compositor->ClearLastSubmittedFrame();}
void Instance::ClearSkyboxOverride() const {m_compositor->ClearSkyboxOverride();}
vr::ETrackingUniverseOrigin Instance::GetTrackingSpace() const {return m_compositor->GetTrackingSpace();}
void Instance::SetTrackingSpace(vr::ETrackingUniverseOrigin space) const {m_compositor->SetTrackingSpace(space);}
vr::Compositor_CumulativeStats Instance::GetCumulativeStats() const
{
	vr::Compositor_CumulativeStats stats;
	m_compositor->GetCumulativeStats(&stats,sizeof(stats));
	return stats;
}
vr::EVRCompositorError Instance::SetSkyboxOverride(const std::vector<prosper::IImage*> &images) const
{
	auto renderAPI = c_engine->GetRenderContext().GetAPIIdentifier();
	std::vector<vr::Texture_t> imgTexData(images.size());
	for(auto i=decltype(images.size()){0};i<images.size();++i)
	{
		auto &img = images.at(i);
		auto &texData = imgTexData.at(i);
		texData.eColorSpace = vr::EColorSpace::ColorSpace_Auto;
		texData.handle = const_cast<void*>(img->GetInternalHandle());
		switch(m_renderAPI)
		{
		case RenderAPI::OpenGL:
			texData.eType = vr::ETextureType::TextureType_OpenGL;
			break;
		case RenderAPI::Vulkan:
			texData.eType = vr::ETextureType::TextureType_Vulkan;
			break;
		}
	}
	return m_compositor->SetSkyboxOverride(imgTexData.data(),imgTexData.size());
}
vr::EVRCompositorError Instance::SetSkyboxOverride(prosper::IImage &img) const {return SetSkyboxOverride({&img});}
vr::EVRCompositorError Instance::SetSkyboxOverride(prosper::IImage &img,prosper::IImage &img2) const {return SetSkyboxOverride({&img,&img2});}
vr::EVRCompositorError Instance::SetSkyboxOverride(
	prosper::IImage &front,prosper::IImage &back,prosper::IImage &left,
	prosper::IImage &right,prosper::IImage &top,prosper::IImage &bottom
) const
{
	return SetSkyboxOverride({&front,&back,&left,&right,&top,&bottom});
}
void Instance::CompositorBringToFront() const {m_compositor->CompositorBringToFront();}
void Instance::CompositorDumpImages() const {m_compositor->CompositorDumpImages();}
void Instance::CompositorGoToBack() const {m_compositor->CompositorGoToBack();}
void Instance::ForceInterleavedReprojectionOn(bool b) const {m_compositor->ForceInterleavedReprojectionOn(b);}
void Instance::ForceReconnectProcess() const {m_compositor->ForceReconnectProcess();}
float Instance::GetFrameTimeRemaining() const {return m_compositor->GetFrameTimeRemaining();}
bool Instance::IsFullscreen() const {return m_compositor->IsFullscreen();}
bool Instance::ShouldAppRenderWithLowResources() const {return m_compositor->ShouldAppRenderWithLowResources();}
void Instance::SuspendRendering(bool b) const {m_compositor->SuspendRendering(b);}
//#include <chrono>
bool Instance::GetPoseTransform(uint32_t deviceIdx,vr::TrackedDevicePose_t &pose,Mat4 &m) const
{
	float fSecondsSinceLastVsync;
	m_system->GetTimeSinceLastVsync(&fSecondsSinceLastVsync,NULL);

	auto fDisplayFrequency = m_system->GetFloatTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd,vr::Prop_DisplayFrequency_Float);
	auto fFrameDuration = 1.f /fDisplayFrequency;
	auto fVsyncToPhotons = m_system->GetFloatTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd,vr::Prop_SecondsFromVsyncToPhotons_Float);

	auto fPredictedSecondsFromNow = fFrameDuration -fSecondsSinceLastVsync +fVsyncToPhotons;
	static std::array<vr::TrackedDevicePose_t,vr::k_unMaxTrackedDeviceCount> poseTransforms;
	m_system->GetDeviceToAbsoluteTrackingPose(vr::ETrackingUniverseOrigin::TrackingUniverseSeated,fPredictedSecondsFromNow,poseTransforms.data(),poseTransforms.size());
	if(deviceIdx >= poseTransforms.size())
		return false;
	pose = poseTransforms.at(deviceIdx);
	if(pose.bDeviceIsConnected == false || pose.bPoseIsValid == false || pose.eTrackingResult != vr::ETrackingResult::TrackingResult_Running_OK)
		return false;
	m = steam_vr_matrix_to_engine_matrix(pose.mDeviceToAbsoluteTracking);
	if(std::isnan(m[0][0]))
		return false;
	return true;
}

#pragma optimize("",on)
