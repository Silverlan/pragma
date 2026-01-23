// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/console/helper.hpp"
#include <cassert>

module pragma.client;

import :console.commands;

const std::string OPENVR_MODULE_PATH = "openvr/pr_openvr";

static std::string lastMessage = "";
static Color lastColor = colors::White;
static pragma::gui::WIHandle hHmdViewMessage = {};
static pragma::gui::WIHandle hHmdViewText = {};
static void update_text()
{
	if(hHmdViewText.IsValid() == false || hHmdViewMessage.IsValid() == false)
		return;
	auto *pBg = hHmdViewMessage.get();
	auto *pText = static_cast<pragma::gui::types::WIText *>(hHmdViewText.get());
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
	auto &wgui = pragma::gui::WGUI::GetInstance();
	auto *bg = wgui.Create<pragma::gui::types::WIRect>();
	bg->SetAutoAlignToParent(true);
	bg->SetColor(colors::Black);
	//bg->SetVisible(false);
	hHmdViewMessage = bg->GetHandle();

	auto *pText = wgui.Create<pragma::gui::types::WIText>(bg);
	hHmdViewText = pText->GetHandle();

	update_text();
}

static void cl_render_vr_enabled(bool b)
{
	if(b == false)
		return;
	pragma::locale::load("vr.txt");
	std::string err;
	auto *client = pragma::get_client_state();
	if(client->InitializeLibrary(OPENVR_MODULE_PATH, &err) == nullptr) {
		lastMessage = pragma::locale::get_text("vr_msg_error_load", std::vector<std::string> {err});
		Con::CERR << lastMessage << Con::endl;
		lastColor = colors::Red;
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
		lastMessage = pragma::locale::get_text("vr_msg_error_init", std::vector<std::string> {err});
		Con::CERR << lastMessage << Con::endl;
		lastColor = colors::Red;
		show_hmd_message();
	}
	else {
		/* // Obsolete: All extensions are enabled by default by Anvil
		auto &dev = pragma::get_cengine()->GetDevice();
		auto fCheckExtension = [&dev](const std::string &ext,bool instanceExtension=false) -> bool {
			if((instanceExtension == false && dev.is_extension_enabled(ext) == true) || (instanceExtension == true && dev.get_parent_instance()->is_instance_extension_enabled(ext)))
				return true;
			WIMessageBox::ShowMessageBox(
				pragma::locale::get_text("unsupported_extension") +": " +ext,pragma::locale::get_text("unsupported_extension"),
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

		lastMessage = pragma::locale::get_text("vr_msg_switch_display");
		lastColor = colors::White;
	}
}
namespace {
	auto UVN = pragma::console::client::register_variable_listener<bool>("cl_render_vr_enabled", +[](pragma::NetworkState *, const pragma::console::ConVar &, bool, bool b) { cl_render_vr_enabled(b); });
}

/*REGISTER_CONVAR_CALLBACK_CL(cl_render_vr_resolution,[](pragma::NetworkState*,ConVar*,std::string,std::string val) {
	
})*/

#if 0
static auto cvHmdViewEnabled = pragma::console::get_client_con_var("cl_vr_hmd_view_enabled");
static void draw_vr(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,prosper::RenderTarget &rt)
{
	if(pragma::get_cgame() == nullptr || hHmdViewMessage.IsValid() == false || pragma::get_cgame()->IsInMainRenderPass() == false)
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

	//auto *scene = pragma::get_cgame()->GetRenderScene();
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
	auto *client = pragma::get_client_state();
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

	auto *fSetControllerStateCallback = dllHandle->FindSymbolAddress<void(*)(const std::function<void(uint32_t,uint32_t,pragma::platform::KeyState)>&)>("openvr_set_controller_state_callback");
	if(fSetControllerStateCallback != nullptr)
	{
		if(val == true)
		{
			fSetControllerStateCallback([](uint32_t controllerId,uint32_t key,pragma::platform::KeyState state) {
				//Con::CERR<<"Controller Key State: "<<controllerId<<","<<key<<","<<pragma::math::to_integral(state)<<Con::endl;
				auto joystick = pragma::platform::Joystick::Create(controllerId +GLFW_MAX_JOYSTICK_COUNT); // TODO
				auto keyOffset = GLFW_CUSTOM_KEY_JOYSTICK_0_KEY_START +joystick->GetJoystickId() *GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT;
				pragma::get_cengine()->JoystickButtonInput(pragma::get_cengine()->GetWindow(),*joystick,key +keyOffset,state);
			});
		}
		else
			fSetControllerStateCallback(nullptr);
	}
}
namespace {
	auto UVN = pragma::console::client::register_variable_listener<bool>("cl_vr_hmd_view_enabled", +[](pragma::NetworkState *, const pragma::console::ConVar &, bool, bool val) { cl_vr_hmd_view_enabled(val); });
}
#endif

static void cl_vr_mirror_window_enabled(bool val)
{
	auto dllHandle = pragma::get_client_state()->GetLibraryModule(OPENVR_MODULE_PATH);
	if(dllHandle == nullptr)
		return;
	auto *fSetMirrorWindowEnabled = dllHandle->FindSymbolAddress<void (*)(bool)>("openvr_set_mirror_window_enabled");
	if(fSetMirrorWindowEnabled == nullptr)
		return;
	fSetMirrorWindowEnabled(val);
}
namespace {
	auto UVN = pragma::console::client::register_variable_listener<bool>("cl_vr_mirror_window_enabled", +[](pragma::NetworkState *, const pragma::console::ConVar &, bool, bool val) { cl_vr_mirror_window_enabled(val); });
}
