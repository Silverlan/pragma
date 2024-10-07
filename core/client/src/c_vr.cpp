/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"

namespace pragma::string {
	class Utf8String;
	class Utf8StringView;
	class Utf8StringArg;
};

#include "pragma/clientstate/clientstate.h"
#include "pragma/c_engine.h"
#include "pragma/gui/wimessagebox.h"
#include "pragma/console/c_cvar.h"
#include <pragma/localization.h>
#include <wgui/types/wirect.h>
#include <wgui/types/witext.h>
#include <sharedutils/scope_guard.h>
#include <sharedutils/util_library.hpp>
#include <pragma/input/inputhelper.h>
#include <prosper_util.hpp>
#include <prosper_command_buffer.hpp>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;
const std::string OPENVR_MODULE_PATH = "openvr/pr_openvr";

static std::string lastMessage = "";
static Color lastColor = Color::White;
static WIHandle hHmdViewMessage = {};
static WIHandle hHmdViewText = {};
static void update_text()
{
	if(hHmdViewText.IsValid() == false || hHmdViewMessage.IsValid() == false)
		return;
	auto *pBg = hHmdViewMessage.get();
	auto *pText = static_cast<WIText *>(hHmdViewText.get());
	pText->SetText(lastMessage);
	pText->SetColor(lastColor);
	pText->SizeToContents();
	pText->SetPos(pBg->GetWidth() * 0.5f - pText->GetWidth() * 0.5f, pBg->GetHeight() * 0.5f - pText->GetHeight() * 0.5f);
}
static void show_hmd_message(bool bInit = false)
{
	if(hHmdViewText.IsValid() == true) {
		update_text();
		return;
	}
	if(hHmdViewMessage.IsValid() == true || bInit == false)
		return;
	auto &wgui = WGUI::GetInstance();
	auto *bg = wgui.Create<WIRect>();
	bg->SetAutoAlignToParent(true);
	bg->SetColor(Color::Black);
	//bg->SetVisible(false);
	hHmdViewMessage = bg->GetHandle();

	auto *pText = wgui.Create<WIText>(bg);
	hHmdViewText = pText->GetHandle();

	update_text();
}

static void cl_render_vr_enabled(bool b)
{
	if(b == false)
		return;
	Locale::Load("vr.txt");
	std::string err;
	if(client->InitializeLibrary(OPENVR_MODULE_PATH, &err) == nullptr) {
		lastMessage = Locale::GetText("vr_msg_error_load", std::vector<std::string> {err});
		Con::cerr << lastMessage << Con::endl;
		lastColor = Color::Red;
		show_hmd_message();
		return;
	}
	auto dllHandle = client->GetLibraryModule(OPENVR_MODULE_PATH);
	assert(dllHandle != nullptr);
	if(dllHandle == nullptr)
		return;
	std::vector<std::string> reqInstanceExtensions;
	std::vector<std::string> reqDeviceExtensions;
	auto *fInitialize = dllHandle->FindSymbolAddress<bool (*)(std::string &, std::vector<std::string> &, std::vector<std::string> &)>("openvr_initialize");
	if(fInitialize(err, reqInstanceExtensions, reqDeviceExtensions) == false) {
		lastMessage = Locale::GetText("vr_msg_error_init", std::vector<std::string> {err});
		Con::cerr << lastMessage << Con::endl;
		lastColor = Color::Red;
		show_hmd_message();
	}
	else {
		/* // Obsolete: All extensions are enabled by default by Anvil
		auto &dev = c_engine->GetDevice();
		auto fCheckExtension = [&dev](const std::string &ext,bool instanceExtension=false) -> bool {
			if((instanceExtension == false && dev.is_extension_enabled(ext) == true) || (instanceExtension == true && dev.get_parent_instance()->is_instance_extension_enabled(ext)))
				return true;
			WIMessageBox::ShowMessageBox(
				Locale::GetText("unsupported_extension") +": " +ext,Locale::GetText("unsupported_extension"),
				WIMessageBox::Button::OK,nullptr,[](WIMessageBox *pMessageBox,WIMessageBox::Button button) {
					pMessageBox->RemoveSafely();
				}
			);
			return false;
		};
		for(auto &ext : reqInstanceExtensions)
		{
			if(fCheckExtension(ext,true) == false)
				return;
		}
		for(auto &ext : reqDeviceExtensions)
		{
			if(fCheckExtension(ext) == false)
				return;
		}*/

		lastMessage = Locale::GetText("vr_msg_switch_display");
		lastColor = Color::White;
	}
}
REGISTER_CONVAR_CALLBACK_CL(cl_render_vr_enabled, [](NetworkState *, const ConVar &, bool, bool b) { cl_render_vr_enabled(b); })

/*REGISTER_CONVAR_CALLBACK_CL(cl_render_vr_resolution,[](NetworkState*,ConVar*,std::string,std::string val) {
	
})*/

#if 0
static auto cvHmdViewEnabled = GetClientConVar("cl_vr_hmd_view_enabled");
static void draw_vr(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,prosper::RenderTarget &rt)
{
	if(c_game == nullptr || hHmdViewMessage.IsValid() == false || c_game->IsInMainRenderPass() == false)
		return;
	auto *bg = hHmdViewMessage.get();
	ScopeGuard sgBg {};
	if(cvHmdViewEnabled->GetBool() == true)
	{
		bg->SetVisible(true);

		sgBg = [bg]() {
			bg->SetVisible(false);
		};
	}
	else
		bg->SetVisible(false);

	//auto *scene = c_game->GetRenderScene();
	//auto &tex = scene->GetRenderTexture();
	//scene->BeginRendering();
	//auto &img = tex->GetImage();
	//img->SetDrawLayout(vk::ImageLayout::ePresentSrcKHR); // prosper TODO
	WGUI::GetInstance().Think();

	auto extents = rt.GetTexture().GetImage().GetExtents();
	prosper::ClearValue clearVal {prosper::ClearDepthStencilValue{1.f}};
	if(drawCmd->RecordBeginRenderPass(rt,&clearVal) == false)
		return;

		bg->Draw(extents.width,extents.height);

	drawCmd->RecordEndRenderPass();
	//*ret = true;
	//return true;
}

static void cl_vr_hmd_view_enabled(bool val)
{
	static CallbackHandle cbPreRender = {};
	auto dllHandle = client->GetLibraryModule(OPENVR_MODULE_PATH);
	if(dllHandle == nullptr)
		return;
	auto *fSetHmdViewEnabled = dllHandle->FindSymbolAddress<void(*)(bool)>("openvr_set_hmd_view_enabled");
	if(fSetHmdViewEnabled == nullptr)
		return;
	fSetHmdViewEnabled(val);
	if(val == true)
	{
		show_hmd_message(true);
		if(cbPreRender.IsValid() == true)
			cbPreRender.Remove();
		cbPreRender = client->AddCallback("PostRender",FunctionCallback<
			void,std::reference_wrapper<std::shared_ptr<prosper::IPrimaryCommandBuffer>>,
			std::reference_wrapper<std::shared_ptr<prosper::RenderTarget>>
		>::Create([](std::reference_wrapper<std::shared_ptr<prosper::IPrimaryCommandBuffer>> drawCmd,std::reference_wrapper<std::shared_ptr<prosper::RenderTarget>> rt) {
			draw_vr(drawCmd.get(),*rt.get());
		}));
	}
	else
	{
		if(hHmdViewMessage.IsValid() == true)
			hHmdViewMessage->Remove();
		if(cbPreRender.IsValid() == true)
			cbPreRender.Remove();
	}

	auto *fSetControllerStateCallback = dllHandle->FindSymbolAddress<void(*)(const std::function<void(uint32_t,uint32_t,GLFW::KeyState)>&)>("openvr_set_controller_state_callback");
	if(fSetControllerStateCallback != nullptr)
	{
		if(val == true)
		{
			fSetControllerStateCallback([](uint32_t controllerId,uint32_t key,GLFW::KeyState state) {
				//Con::cerr<<"Controller Key State: "<<controllerId<<","<<key<<","<<umath::to_integral(state)<<Con::endl;
				auto joystick = GLFW::Joystick::Create(controllerId +GLFW_MAX_JOYSTICK_COUNT); // TODO
				auto keyOffset = GLFW_CUSTOM_KEY_JOYSTICK_0_KEY_START +joystick->GetJoystickId() *GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT;
				c_engine->JoystickButtonInput(c_engine->GetWindow(),*joystick,key +keyOffset,state);
			});
		}
		else
			fSetControllerStateCallback(nullptr);
	}
}
REGISTER_CONVAR_CALLBACK_CL(cl_vr_hmd_view_enabled,[](NetworkState*,ConVar*,bool,bool val) {
	cl_vr_hmd_view_enabled(val);
})
#endif

static void cl_vr_mirror_window_enabled(bool val)
{
	auto dllHandle = client->GetLibraryModule(OPENVR_MODULE_PATH);
	if(dllHandle == nullptr)
		return;
	auto *fSetMirrorWindowEnabled = dllHandle->FindSymbolAddress<void (*)(bool)>("openvr_set_mirror_window_enabled");
	if(fSetMirrorWindowEnabled == nullptr)
		return;
	fSetMirrorWindowEnabled(val);
}
REGISTER_CONVAR_CALLBACK_CL(cl_vr_mirror_window_enabled, [](NetworkState *, const ConVar &, bool, bool val) { cl_vr_mirror_window_enabled(val); })
