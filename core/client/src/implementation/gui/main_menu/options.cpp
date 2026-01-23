// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>
#include <rapidxml_print.hpp>

module pragma.client;

import :gui.main_menu_options;
import :gui.options_list;
import :gui.choice_list;
import :gui.slider;

import :audio;
import :client_state;
import :engine;
import :entities.components;
import :rendering.render_apis;
import pragma.pad;
import pragma.string.unicode;

pragma::gui::types::WIMainMenuOptions::WIMainMenuOptions() : WIMainMenuBase(), m_yOffset(128) {}

pragma::gui::types::WIMainMenuOptions::~WIMainMenuOptions()
{
	if(m_joystickStateChanged.IsValid())
		m_joystickStateChanged.Remove();
}

void pragma::gui::types::WIMainMenuOptions::ApplyWindowSize()
{
	WIDropDownMenu *resMenu = static_cast<WIDropDownMenu *>(m_hResolutionList.get());
	auto text = resMenu->GetText();
	std::vector<std::string> res;
	string::explode(std::string {text.cpp_str()}, "x", res);
	if(res.size() < 2)
		return;
	int w = string::to_int(res[0]);
	int h = string::to_int(res[1]);

	get_cengine()->GetWindow().SetResolution(Vector2i(w, h));
}

void pragma::gui::types::WIMainMenuOptions::ApplyOptions()
{
	if(m_hActive.IsValid())
		static_cast<WIOptionsList *>(m_hActive.get())->RunUpdateConVars();
	auto *client = get_client_state();
	if(m_hAntiAliasing.IsValid()) {
		auto *pChoice = static_cast<WIChoiceList *>(m_hAntiAliasing.get())->GetSelectedChoice();
		if(pChoice->value == "fxaa") {
			std::vector<std::string> argv {std::to_string(math::to_integral(rendering::AntiAliasing::FXAA))};
			client->RunConsoleCommand("cl_render_anti_aliasing", argv);
		}
		else if(string::substr(pChoice->value, 0, 4) == "msaa") {
			std::string mssaaSamples = "1";
			if(pChoice->value == "msaa2")
				mssaaSamples = "1";
			else if(pChoice->value == "msaa4")
				mssaaSamples = "2";
			else if(pChoice->value == "msaa8")
				mssaaSamples = "3";
			else if(pChoice->value == "msaa16")
				mssaaSamples = "4";
			else if(pChoice->value == "msaa32")
				mssaaSamples = "5";
			else if(pChoice->value == "msaa64")
				mssaaSamples = "6";
			std::vector<std::string> argv {mssaaSamples};
			client->RunConsoleCommand("cl_render_msaa_samples", argv);
			argv = {std::to_string(math::to_integral(rendering::AntiAliasing::MSAA))};
			client->RunConsoleCommand("cl_render_anti_aliasing", argv);
		}
		else {
			std::vector<std::string> argv {std::to_string(math::to_integral(rendering::AntiAliasing::None))};
			client->RunConsoleCommand("cl_render_anti_aliasing", argv);
		}
	}
	//ApplyWindowSize();
}
void pragma::gui::types::WIMainMenuOptions::CloseMessageBox()
{
	if(m_hMessageBox.IsValid())
		m_hMessageBox->Remove();
}
void pragma::gui::types::WIMainMenuOptions::Apply(platform::MouseButton button, platform::KeyState state, platform::Modifier)
{
	if(button != platform::MouseButton::Left || state != platform::KeyState::Press)
		return;
	ApplyOptions();
	CloseMessageBox();
	/*auto *pMessageBox = WIMessageBox::Create(
		pragma::locale::get_text("menu_apply_restart_required"),pragma::locale::get_text("menu_apply_restart_required_title"),
		WIMessageBox::Button::OK,
		[](WIMessageBox *pMessageBox,WIMessageBox::Button) {
			pMessageBox->RemoveSafely();
		}
	);
	m_hMessageBox = pMessageBox->GetHandle();*/
}

void pragma::gui::types::WIMainMenuOptions::ResetDefaults(platform::MouseButton button, platform::KeyState state, platform::Modifier)
{
	if(button != platform::MouseButton::Left || state != platform::KeyState::Press)
		return;
	CloseMessageBox();
	auto *pMessageBox = WIMessageBox::Create(locale::get_text("menu_reset_defaults_confirm"), locale::get_text("menu_reset_defaults_confirm_title"), WIMessageBox::Button::YESNO, [](WIMessageBox *pMessageBox, WIMessageBox::Button) { pMessageBox->RemoveSafely(); });
	m_hMessageBox = pMessageBox->GetHandle();
}

void pragma::gui::types::WIMainMenuOptions::CreateLabel(std::string text)
{
	WIHandle hLabel = CreateChild<WIText>();
	WIText *t = static_cast<WIText *>(hLabel.get());
	t->SetText(text);
	t->SizeToContents();
	t->SetPos(Vector2i(100, m_yOffset));
	t->SetColor(1.f, 1.f, 1.f, 1.f);
}

pragma::gui::types::WIDropDownMenu *pragma::gui::types::WIMainMenuOptions::CreateDropDownMenu(std::string text)
{
	CreateLabel(text);
	WIHandle hDropDown = CreateChild<WIDropDownMenu>();
	WIDropDownMenu *menu = static_cast<WIDropDownMenu *>(hDropDown.get());
	menu->SetPos(Vector2i(256, m_yOffset));
	menu->SetSize(200, 28);
	m_yOffset += menu->GetHeight() + 20;
	return menu;
}

pragma::gui::types::WICheckbox *pragma::gui::types::WIMainMenuOptions::CreateCheckbox(std::string text)
{
	CreateLabel(text);
	WIHandle hCheckBox = CreateChild<WICheckbox>();
	WICheckbox *checkBox = static_cast<WICheckbox *>(hCheckBox.get());
	checkBox->SetPos(Vector2i(256, m_yOffset));
	m_yOffset += checkBox->GetHeight() + 20;
	return checkBox;
}

static bool sortResolutions(pragma::platform::Monitor::VideoMode &a, pragma::platform::Monitor::VideoMode &b) { return ((a.width < b.width) || (a.width == b.width && a.height < b.height)) ? true : false; }
void pragma::gui::types::WIMainMenuOptions::SetActiveMenu(WIHandle &hMenu)
{
	if(hMenu.get() == m_hActive.get() && hMenu.IsValid())
		return;
	if(m_hActive.IsValid())
		m_hActive->SetVisible(false);
	if(!hMenu.IsValid())
		return;
	hMenu->SetVisible(true);
	m_hActive = hMenu;
}
void pragma::gui::types::WIMainMenuOptions::Initialize()
{
	WIMainMenuBase::Initialize();
	EnableThinking();
	AddMenuItem(locale::get_text("menu_options_general"), FunctionCallback<void, WIMainMenuElement *>::Create([this](WIMainMenuElement *) { ShowGeneralSettings(); }));
	AddMenuItem(locale::get_text("menu_options_video"), FunctionCallback<void, WIMainMenuElement *>::Create([this](WIMainMenuElement *) { ShowVideoSettings(); }));
	AddMenuItem(locale::get_text("menu_options_audio"), FunctionCallback<void, WIMainMenuElement *>::Create([this](WIMainMenuElement *) { ShowAudioSettings(); }));
	AddMenuItem(locale::get_text("menu_options_controls"), FunctionCallback<void, WIMainMenuElement *>::Create([this](WIMainMenuElement *) { ShowControlSettings(); }));
	AddMenuItem(locale::get_text("back"), FunctionCallback<void, WIMainMenuElement *>::Create([this](WIMainMenuElement *) {
		auto *mainMenu = dynamic_cast<WIMainMenu *>(GetParent());
		if(mainMenu == nullptr)
			return;
		mainMenu->OpenMainMenu();
	}));
}

void pragma::gui::types::WIMainMenuOptions::OnVisibilityChanged(bool bVisible)
{
	WIMainMenuBase::OnVisibilityChanged(bVisible);
	if(bVisible == false || m_bInitialized == true)
		return;
	m_bInitialized = true;
	// Initialize sub-menues once options have been opened for the first time
	InitializeGeneralSettings();
	InitializeVideoSettings();
	InitializeAudioSettings();
	InitializeControlSettings();
}

void pragma::gui::types::WIMainMenuOptions::ShowGeneralSettings() { SetActiveMenu(m_hGeneralSettings); }
void pragma::gui::types::WIMainMenuOptions::ShowVideoSettings() { SetActiveMenu(m_hVideoSettings); }
void pragma::gui::types::WIMainMenuOptions::ShowAudioSettings() { SetActiveMenu(m_hAudioSettings); }
void pragma::gui::types::WIMainMenuOptions::ShowControlSettings() { SetActiveMenu(m_hControlSettings); }

static std::string sliderTranslator(float f) { return std::to_string(CInt32(f * 100.f)); }
static void sliderInitializer(pragma::gui::types::WISlider *pSlider)
{
	pSlider->SetRange(0.f, 1.f, 0.01f);
	pSlider->SetValueTranslator(sliderTranslator);
	pSlider->SetPostFix("%");
}
void pragma::gui::types::WIMainMenuOptions::InitializeOptionsList(WIOptionsList *pList)
{
	pList->SetVisible(false);
	auto *pRow = pList->AddRow();
	auto &gui = WGUI::GetInstance();
	auto *buttonReset = gui.Create<WIButton>();
	buttonReset->SetText(locale::get_text("reset_defaults"));
	buttonReset->SizeToContents();
	buttonReset->SetAutoCenterToParent(true);
	buttonReset->AddCallback("OnMouseEvent",
	  FunctionCallback<util::EventReply, platform::MouseButton, platform::KeyState, platform::Modifier>::CreateWithOptionalReturn(
	    [this](util::EventReply *reply, platform::MouseButton button, platform::KeyState state, platform::Modifier mods) -> CallbackReturnType {
		    ResetDefaults(button, state, mods);
		    *reply = util::EventReply::Handled;
		    return CallbackReturnType::HasReturnValue;
	    }));
	m_hButtonReset = buttonReset->GetHandle();
	pRow->InsertElement(0, buttonReset);

	auto *buttonApply = gui.Create<WIButton>();
	buttonApply->SetText(locale::get_text("apply"));
	buttonApply->SizeToContents();
	buttonApply->SetAutoCenterToParent(true);
	buttonApply->AddCallback("OnMouseEvent",
	  FunctionCallback<util::EventReply, platform::MouseButton, platform::KeyState, platform::Modifier>::CreateWithOptionalReturn(
	    [this](util::EventReply *reply, platform::MouseButton button, platform::KeyState state, platform::Modifier mods) -> CallbackReturnType {
		    Apply(button, state, mods);
		    *reply = util::EventReply::Handled;
		    return CallbackReturnType::HasReturnValue;
	    }));
	m_hButtonApply = buttonApply->GetHandle();
	pRow->InsertElement(1, buttonApply);
	WIMainMenuBase::InitializeOptionsList(pList);
}
void pragma::gui::types::WIMainMenuOptions::InitializeGeneralSettings()
{
	m_hGeneralSettings = CreateChild<WIOptionsList>();
	m_hGeneralSettings->SetName("settings_general");
	auto *pList = static_cast<WIOptionsList *>(m_hGeneralSettings.get());
	auto title = locale::get_text("general_options");
	string::to_upper(title);
	pList->SetTitle(title);
	// Player Name
	auto *teName = pList->AddTextEntry(locale::get_text("player_name"), "playername");
	teName->SizeToContents();
	//
	// Language
	std::unordered_map<std::string, std::string> lanOptions {};
	for(auto &pair : locale::get_languages()) {
		auto &lanInfo = pair.second;
		auto enabled = true;
		if(lanInfo.configData)
			(*lanInfo.configData)["enabled"](enabled);
		if(!enabled)
			continue;
		lanOptions[lanInfo.displayName] = pair.first;
	}
	WIDropDownMenu *language = pList->AddDropDownMenu(locale::get_text("language"), lanOptions, "cl_language");
	//

	auto *pRowGameplay = pList->AddHeaderRow();
	pRowGameplay->SetValue(0, locale::get_text("gameplay_options"));

	// Physics Engine
	std::unordered_map<std::string, std::string> physEngines {};
	for(auto &engine : physics::IEnvironment::GetAvailablePhysicsEngines())
		physEngines.insert(std::make_pair(locale::get_text("physics_engine_" + engine), engine));
	auto *pPhysEngineList = pList->AddDropDownMenu(locale::get_text("physics_engine"), physEngines, "phys_engine");
	//

	// Audio Engine
	std::unordered_map<std::string, std::string> audioEngines {};
	for(auto &engine : audio::get_available_audio_apis())
		audioEngines.insert(std::make_pair(locale::get_text("audio_engine_" + engine), engine));
	auto *pAudioEngineList = pList->AddDropDownMenu(locale::get_text("audio_engine"), audioEngines, "audio_engine");
	//

	// Networking library
	std::unordered_map<std::string, std::string> netLibs {};
	for(auto &lib : networking::GetAvailableNetworkingModules())
		netLibs.insert(std::make_pair(locale::get_text("networking_library_" + lib), lib));
	auto *pNetLibList = pList->AddDropDownMenu(locale::get_text("networking_library"), netLibs, "net_library");
	//

	pList->AddToggleChoice(locale::get_text("enable_steamworks"), "steam_steamworks_enabled");

	// Lua memory usage
	auto *pRow = pList->AddHeaderRow();
	pRow->SetValue(0, locale::get_text("lua_memory_statistics"));
	auto fAddLuaMemorySlider = [pList](const std::string &title, uint32_t totalSize) {
		return pList
		  ->AddSlider(locale::get_text(title),
		    [totalSize](WISlider *pSlider) {
			    pSlider->SetRange(0, totalSize, 1);
			    pSlider->SetMouseInputEnabled(false);
			    pSlider->GetProgressProperty()->AddCallback([pSlider](std::reference_wrapper<const float> oldVal, std::reference_wrapper<const float> newVal) {
				    auto percent = newVal.get();
				    auto &col0 = colors::LimeGreen;
				    auto &col1 = colors::DarkOrange;
				    auto &col2 = colors::Crimson;
				    Color col {};
				    if(percent <= 0.75)
					    col = col0.Lerp(col1, percent / 0.75);
				    else
					    col = col1.Lerp(col2, math::min((percent - 0.75) / 0.25, 1.0));
				    pSlider->SetColor(col);
			    });
			    pSlider->SetValueTranslator([totalSize, pSlider](float f) -> std::string {
				    auto percent = f / static_cast<double>(totalSize);
				    return util::get_pretty_bytes(static_cast<uint64_t>(f)) + " / " + util::get_pretty_bytes(totalSize) + " (" + util::round_string(percent * 100.0, 2) + "%)";
			    });
		    })
		  ->GetHandle();
	};
	auto totalSize = 1 * 1'024 * 1'024 * 1'024; // LuaJIT has a memory limit of 1GiB when compiled for x64
	m_hLuaMemoryUsageGUI = fAddLuaMemorySlider("lua_memory_current_usage_gui", totalSize);
	m_hLuaMemoryUsageClient = fAddLuaMemorySlider("lua_memory_current_usage_client", totalSize);
	m_hLuaMemoryUsageServer = fAddLuaMemorySlider("lua_memory_current_usage_server", totalSize);
	//

	InitializeOptionsList(pList);
}
void pragma::gui::types::WIMainMenuOptions::Think(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd)
{
	WIMainMenuBase::Think(drawCmd);
	UpdateMemoryUsage();
}
void pragma::gui::types::WIMainMenuOptions::UpdateMemoryUsage()
{
	if(IsVisible() == false)
		return;
	auto t = util::Clock::now();
	if(std::chrono::duration_cast<std::chrono::seconds>(t - m_tLastMemoryUsageUpdate).count() < 1)
		return;
	m_tLastMemoryUsageUpdate = t;
	if(m_hGPUMemoryUsage.IsValid() && m_hActive.get() == m_hVideoSettings.get()) {
		/*auto *pSlider = static_cast<WISlider*>(m_hGPUMemoryUsage.get());
		auto &context = pragma::get_cengine()->GetRenderContext();
		auto &memManager = context.GetMemoryManager();
		auto stats = memManager.GetStatistics();
		auto allocatedMemory = 0ull;
		for(auto &heap : stats.memoryHeap)
			allocatedMemory += heap.usedBytes;
		pSlider->SetValue(allocatedMemory);*/ // PROSPER TODO
	}

	std::array<WIHandle, 3> luaMemUsageSliders = {m_hLuaMemoryUsageGUI, m_hLuaMemoryUsageClient, m_hLuaMemoryUsageServer};
	auto *svState = get_cengine()->GetServerNetworkState();
	auto *client = get_client_state();
	std::array<lua::State *, 3> luaMemUsageStates = {client->GetGUILuaState(), client->GetLuaState(), (svState != nullptr) ? svState->GetLuaState() : nullptr};
	for(auto i = decltype(luaMemUsageSliders.size()) {0}; i < luaMemUsageSliders.size(); ++i) {
		auto &hSlider = luaMemUsageSliders.at(i);
		if(hSlider.IsValid() == false)
			continue;
		auto *l = luaMemUsageStates.at(i);
		auto *pSlider = static_cast<WISlider *>(hSlider.get());
		auto allocatedMemory = (l != nullptr) ? (lua::gc(l, lua::GarbageCollectorTask::CurrentMemoryInUseInKb, -1) * 1'024 + lua::gc(l, lua::GarbageCollectorTask::CurrentMemoryRemainderBytes, -1)) : 0;
		pSlider->SetValue(allocatedMemory);
	}
}
void pragma::gui::types::WIMainMenuOptions::InitializeVideoSettings()
{
	m_hVideoSettings = CreateChild<WIOptionsList>();
	m_hVideoSettings->SetName("settings_video");
	auto *pList = static_cast<WIOptionsList *>(m_hVideoSettings.get());
	auto title = locale::get_text("video_options");
	string::to_upper(title);
	pList->SetTitle(title);

	// Preset
	auto *populateRenderPresets = +[](WIChoiceList *pList) {
		pList->AddChoice(locale::get_text("minimal"), "0");
		pList->AddChoice(locale::get_text("low"), "1");
		pList->AddChoice(locale::get_text("medium"), "2");
		pList->AddChoice(locale::get_text("high"), "3");
		pList->AddChoice(locale::get_text("very_high"), "4");
	};
	auto *pListPreset = pList->AddChoiceList(
	  locale::get_text("preset"),
	  populateRenderPresets,
	  "cl_render_preset");
	auto hThis = GetHandle();
	pListPreset->AddCallback("OnSelect", FunctionCallback<void, uint32_t, std::reference_wrapper<std::string>>::Create([hThis](uint32_t, std::reference_wrapper<std::string> value) mutable {
		if(hThis.IsValid() == false)
			return;
		auto *el = static_cast<WIMainMenuOptions *>(hThis.get());
		auto val = util::to_int(value.get());

		// Defaults; should match the convar values
		uint32_t textureQuality = 4;
		int32_t antiAliasing = 2;
		auto bSsao = true;
		auto bHdrr = true;
		auto bBloom = true;
		auto motionBlur = 0.f;
		uint32_t occlusionCulling = 4;
		auto bDoF = false;
		uint32_t presentMode = 2;
		uint32_t particleQuality = 3;
		uint32_t shaderQuality = 1;
		uint32_t shadowQuality = 8;
		uint32_t reflectionQuality = 2;
		auto bDynamicShadows = true;
		uint32_t shadowUpdateFrequency = 0;
		uint32_t pssmShadowUpdateFrequencyOffset = 0;
		uint32_t pssmSplitCount = 3;
		uint32_t mdlQuality = 0;
		uint32_t textureFiltering = 6;
		std::string shadowResolution = "1024";
		switch(val) {
		case 0:
			textureQuality = 0;
			//antiAliasing = 0; // off
			//bSsao = false;
			//bHdrr = false;
			//bBloom = false;
			//motionBlur = 0.f;
			//occlusionCulling = 4;
			//bDoF = false;
			//presentMode = pragma::math::min(static_cast<uint32_t>(1),static_cast<WIChoiceList*>(el->m_hPresentMode.get())->GetChoiceCount() -1);
			//particleQuality = 0;
			shaderQuality = 0;
			shadowQuality = 0;
			//reflectionQuality = 0;
			//bDynamicShadows = false;
			//shadowUpdateFrequency = 10;
			//pssmShadowUpdateFrequencyOffset = 10;
			//pssmSplitCount = 1;
			mdlQuality = 0;
			textureFiltering = 0;
			shadowResolution = "256";
			break;
		case 1:
			textureQuality = 1;
			//antiAliasing = 0; // off
			//bSsao = false;
			//bHdrr = true;
			//bBloom = true;
			//motionBlur = 0.f;
			//occlusionCulling = 4;
			//bDoF = false;
			//presentMode = pragma::math::min(static_cast<uint32_t>(1),static_cast<WIChoiceList*>(el->m_hPresentMode.get())->GetChoiceCount() -1);
			//particleQuality = 1;
			shaderQuality = 1;
			shadowQuality = 1;
			//reflectionQuality = 0;
			//bDynamicShadows = false;
			//shadowUpdateFrequency = 10;
			//pssmShadowUpdateFrequencyOffset = 6;
			//pssmSplitCount = 1;
			mdlQuality = 1;
			textureFiltering = 1;
			shadowResolution = "512";
			break;
		case 2:
			textureQuality = 3;
			//antiAliasing = 1; // fxaa
			//bSsao = false;
			//bHdrr = true;
			//bBloom = true;
			//motionBlur = 0.f;
			//occlusionCulling = 4;
			//bDoF = false;
			//presentMode = pragma::math::min(static_cast<uint32_t>(2),static_cast<WIChoiceList*>(el->m_hPresentMode.get())->GetChoiceCount() -1);
			//particleQuality = 1;
			shaderQuality = 2;
			shadowQuality = 2;
			//reflectionQuality = 1;
			//bDynamicShadows = true;
			//shadowUpdateFrequency = 5;
			//pssmShadowUpdateFrequencyOffset = 2;
			//pssmSplitCount = pragma::math::max(pragma::CShadowCSMComponent::MAX_CASCADE_COUNT -2,static_cast<uint32_t>(1));
			mdlQuality = 2;
			textureFiltering = 2;
			shadowResolution = "1024";
			break;
		case 3:
			textureQuality = 4;
			//antiAliasing = 1; //2;// msaa x2
			//bSsao = true;
			//bHdrr = true;
			//bBloom = true;
			//motionBlur = 1.f;
			//occlusionCulling = 4;
			//bDoF = true;
			//presentMode = static_cast<WIChoiceList*>(el->m_hPresentMode.get())->GetChoiceCount() -1;
			//particleQuality = 2;
			shaderQuality = 2;
			shadowQuality = 4;
			//reflectionQuality = 2;
			//bDynamicShadows = true;
			//shadowUpdateFrequency = 1;
			//pssmShadowUpdateFrequencyOffset = 2;
			//pssmSplitCount = pragma::math::max(pragma::CShadowCSMComponent::MAX_CASCADE_COUNT -1,static_cast<uint32_t>(1));
			mdlQuality = static_cast<WIChoiceList *>(el->m_hMdlQuality.get())->GetChoiceCount() - 1;
			textureFiltering = static_cast<WIChoiceList *>(el->m_hTextureFiltering.get())->GetChoiceCount() - 1;
			shadowResolution = "2048";
			break;
		default:
			/*textureQuality = static_cast<WIChoiceList*>(el->m_hTexQuality.get())->GetChoiceCount() -1;
				antiAliasing = static_cast<WIChoiceList*>(el->m_hAntiAliasing.get())->GetChoiceCount() -1; // max msaa
				bSsao = true;
				bHdrr = true;
				bBloom = true;
				motionBlur = 1.f;
				occlusionCulling = 4;
				bDoF = true;
				presentMode = static_cast<WIChoiceList*>(el->m_hPresentMode.get())->GetChoiceCount() -1;
				particleQuality = static_cast<WIChoiceList*>(el->m_hParticleQuality.get())->GetChoiceCount() -1;
				shaderQuality = static_cast<WIChoiceList*>(el->m_hShaderQuality.get())->GetChoiceCount() -1;
				shadowQuality = static_cast<WIChoiceList*>(el->m_hShadowQuality.get())->GetChoiceCount() -1;
				reflectionQuality = 2;
				bDynamicShadows = true;
				shadowUpdateFrequency = 0;
				pssmShadowUpdateFrequencyOffset = 0;
				pssmSplitCount = pragma::CShadowCSMComponent::MAX_CASCADE_COUNT;
				mdlQuality = static_cast<WIChoiceList*>(el->m_hMdlQuality.get())->GetChoiceCount() -1;
				textureFiltering = static_cast<WIChoiceList*>(el->m_hTextureFiltering.get())->GetChoiceCount() -1;
				shadowResolution = "2048";*/
			break;
		}

		if(get_cengine()->GetRenderContext().GetPhysicalDeviceVendor() == prosper::Vendor::Nvidia)
			antiAliasing = 0; // TODO: Anti-Aliasing causes crashes on some modern Nvidia GPUs (Something to do with depth-image) FIXME
		if(el->m_hTexQuality.IsValid())
			static_cast<WIChoiceList *>(el->m_hTexQuality.get())->SelectChoice(textureQuality);
		if(el->m_hAntiAliasing.IsValid())
			static_cast<WIChoiceList *>(el->m_hAntiAliasing.get())->SelectChoice(antiAliasing);
		if(el->m_hSsao.IsValid())
			static_cast<WICheckbox *>(el->m_hSsao.get())->SetChecked(bSsao);
		if(el->m_hdrr.IsValid())
			static_cast<WICheckbox *>(el->m_hdrr.get())->SetChecked(bHdrr);
		if(el->m_hBloom.IsValid())
			static_cast<WICheckbox *>(el->m_hBloom.get())->SetChecked(bBloom);
		if(el->m_hMotionBlur.IsValid())
			static_cast<WISlider *>(el->m_hMotionBlur.get())->SetValue(motionBlur);
		if(el->m_hOcclusionCulling.IsValid())
			static_cast<WIChoiceList *>(el->m_hOcclusionCulling.get())->SelectChoice(occlusionCulling);
		if(el->m_hDoF.IsValid())
			static_cast<WICheckbox *>(el->m_hDoF.get())->SetChecked(bDoF);
		if(el->m_hPresentMode.IsValid())
			static_cast<WIChoiceList *>(el->m_hPresentMode.get())->SelectChoice(presentMode);
		if(el->m_hParticleQuality.IsValid())
			static_cast<WIChoiceList *>(el->m_hParticleQuality.get())->SelectChoice(particleQuality);
		if(el->m_hShaderQuality.IsValid())
			static_cast<WIChoiceList *>(el->m_hShaderQuality.get())->SelectChoice(shaderQuality);
		if(el->m_hShadowQuality.IsValid())
			static_cast<WIChoiceList *>(el->m_hShadowQuality.get())->SelectChoice(shadowQuality);
		if(el->m_hReflectionQuality.IsValid())
			static_cast<WIChoiceList *>(el->m_hReflectionQuality.get())->SelectChoice(reflectionQuality);
		if(el->m_hDynamicShadows.IsValid())
			static_cast<WICheckbox *>(el->m_hDynamicShadows.get())->SetChecked(bDynamicShadows);
		if(el->m_hShadowUpdateFrequency.IsValid())
			static_cast<WISlider *>(el->m_hShadowUpdateFrequency.get())->SetValue(static_cast<float>(shadowUpdateFrequency));
		if(el->m_hPssmShadowUpdateFrequencyOffset.IsValid())
			static_cast<WISlider *>(el->m_hPssmShadowUpdateFrequencyOffset.get())->SetValue(static_cast<float>(pssmShadowUpdateFrequencyOffset));
		if(el->m_hPssmSplitCount.IsValid())
			static_cast<WISlider *>(el->m_hPssmSplitCount.get())->SetValue(static_cast<float>(pssmSplitCount));
		if(el->m_hMdlQuality.IsValid())
			static_cast<WIChoiceList *>(el->m_hMdlQuality.get())->SelectChoice(mdlQuality);
		if(el->m_hTextureFiltering.IsValid())
			static_cast<WIChoiceList *>(el->m_hTextureFiltering.get())->SelectChoice(textureFiltering);
		if(el->m_hShadowRes.IsValid())
			static_cast<WIChoiceList *>(el->m_hShadowRes.get())->SelectChoice(shadowResolution);
	}));
	//
	// Vertical Sync
	auto *vSync = pList->AddToggleChoice(locale::get_text("vertical_sync"), "cl_render_vsync_enabled");
	//
	// Resolution Menu
	// TODO Aspect Ratio
	auto *resMenu = pList->AddDropDownMenu(
	  locale::get_text("resolution"),
	  [](WIDropDownMenu *pMenu) {
		  auto &context = WGUI::GetInstance().GetContext();
		  auto &window = context.GetWindow();
		  auto *monitor = window->GetMonitor();
		  auto primaryMonitor = platform::get_primary_monitor();
		  if(monitor == nullptr)
			  monitor = &primaryMonitor;
		  //GLWindow *window = pragma::get_cengine()->GetWindow();
		  std::vector<platform::Monitor::VideoMode> modes;
		  if(monitor != nullptr) {
			  auto videoModes = monitor->GetSupportedVideoModes();
			  modes.reserve(videoModes.size());
			  for(auto it = videoModes.begin(); it != videoModes.end(); ++it) {
				  auto &videoMode = *it;
				  auto itMode = std::find_if(modes.begin(), modes.end(), [&videoMode](platform::Monitor::VideoMode &mode) { return (mode.width == videoMode.width && mode.height == videoMode.height) ? true : false; });
				  if(itMode == modes.end())
					  modes.push_back(videoMode);
			  }
			  std::sort(modes.begin(), modes.end(), &sortResolutions);
		  }
		  for(int i = 0; i < modes.size(); i++) {
			  const auto &videoMode = modes[i];
			  std::string res = std::to_string(videoMode.width);
			  res += "x";
			  res += std::to_string(videoMode.height);
			  pMenu->AddOption(res, res);
		  }
	  },
	  "cl_window_resolution");
	m_hResolutionList = resMenu->GetHandle();
	//
	// Supersampling
	// TODO
	//
	// Graphics API
	std::unordered_map<std::string, std::string> graphicsAPIs {};
	for(auto &api : rendering::get_available_graphics_apis())
		graphicsAPIs.insert(std::make_pair(locale::get_text("graphics_api_" + api), api));
	auto *pGraphicsAPIList = pList->AddDropDownMenu(locale::get_text("graphics_api"), graphicsAPIs, "render_api");
	//
	// Display
	WIDropDownMenu *display = pList->AddDropDownMenu(
	  locale::get_text("monitor"),
	  [](WIDropDownMenu *pMenu) {
		  auto monitors = platform::get_monitors();

		  std::vector<std::string> monitorNames(monitors.size());
		  std::vector<std::string> monitorOptionNames(monitors.size());

		  for(auto i = decltype(monitors.size()) {0}; i < monitors.size(); ++i) {
			  auto &monitor = monitors.at(i);
			  auto name = monitor.GetName();
			  monitorNames.at(i) = name;
			  auto numExists = 0;
			  auto idxFirst = -1;
			  for(auto j = 0; j < i; j++) {
				  if(monitorNames[j] == name) {
					  if(idxFirst == -1)
						  idxFirst = j;
					  numExists++;
				  }
			  }
			  if(numExists > 0) {
				  name += std::string(" #") + std::to_string(numExists + 1);
				  if(numExists == 1)
					  monitorOptionNames[idxFirst] += " #1";
			  }
			  monitorOptionNames[i] = name;
		  }
		  for(auto it = monitorOptionNames.begin(); it != monitorOptionNames.end(); ++it)
			  pMenu->AddOption(*it);
	  },
	  "cl_render_monitor");
	m_hDisplay = display->GetHandle();
	//
	// Device
	// TODO
#if 0
	auto *pDeviceMenu = pList->AddDropDownMenu(pragma::locale::get_text("physical_device"),[](WIDropDownMenu *pMenu) {
		auto deviceList = prosper::util::get_available_vendor_devices(pragma::get_cengine()->GetRenderContext());
		for(auto &devInfo : deviceList)
			pMenu->AddOption(devInfo.deviceName,std::to_string(pragma::math::to_integral(devInfo.vendor)) +"," +std::to_string(devInfo.deviceId));

		pMenu->SelectOption(0u);
		pMenu->SelectOption(client->GetConVarString("cl_gpu_device"));
	},"cl_gpu_device");
#endif
	//
	// Window Mode
	pList->AddChoiceList(locale::get_text("window_mode"), {locale::get_text("windowmode_fullscreen"), locale::get_text("windowmode_windowed"), locale::get_text("windowmode_noborder_window")}, "cl_render_window_mode");
	//
	// FPS Limit
	auto *populateMaxFps = +[](WIChoiceList *pList) {
		pList->AddChoice(locale::get_text("no_limit"), "-1");
		pList->AddChoice("30 FPS", "30");
		pList->AddChoice("60 FPS", "60");
		pList->AddChoice("90 FPS", "90");
		pList->AddChoice("120 FPS", "120");
	};
	pList->AddChoiceList(
	  locale::get_text("fps_limit"),
	  populateMaxFps,
	  "cl_max_fps");
	//
	// Texture Quality (Mipmap Level)
	auto *populateTextureQuality = +[](WIChoiceList *pList) {
		pList->AddChoice(locale::get_text("minimal"), "0");
		pList->AddChoice(locale::get_text("low"), "1");
		pList->AddChoice(locale::get_text("medium"), "2");
		pList->AddChoice(locale::get_text("high"), "3");
		pList->AddChoice(locale::get_text("very_high"), "4");
	};
	auto texList = pList->AddChoiceList(
	  locale::get_text("texture_quality"),
	  populateTextureQuality,
	  "cl_render_texture_quality");
	m_hTexQuality = texList->GetHandle();
	//
	// Texture Streaming
	auto *texStreaming = pList->AddToggleChoice(locale::get_text("texture_streaming"), "cl_material_streaming_enabled");
	//
	// Model Quality
	auto *populateModelQuality = +[](WIChoiceList *pList) {
		pList->AddChoice(locale::get_text("minimal"), "10000");
		pList->AddChoice(locale::get_text("low"), "3");
		pList->AddChoice(locale::get_text("medium"), "2");
		pList->AddChoice(locale::get_text("high"), "1");
		pList->AddChoice(locale::get_text("very_high"), "0");
	};
	auto *mdlQualityList = pList->AddChoiceList(
	  locale::get_text("model_quality"),
	  populateModelQuality,
	  "cl_render_lod_bias");
	m_hMdlQuality = mdlQualityList->GetHandle();
	//
	// Anti-Aliasing
	std::vector<std::pair<std::string, std::string>> aaChoices;
	auto maxSamples = rendering::GetMaxMSAASampleCount();
	aaChoices.push_back(std::make_pair("off", locale::get_text("off")));
	aaChoices.push_back(std::make_pair("fxaa", "FXAA"));
	int samples = 2;
	/*while(samples <= maxSamples)
	{
		std::string o = "MSAA x";
		o += std::to_string(samples);
		aaChoices.push_back(std::make_pair("msaa" +std::to_string(samples),o));
		samples *= 2;
	}*/
	auto *antiAlias = pList->AddChoiceList(locale::get_text("anti_aliasing"), aaChoices, "", "cl_render_anti_aliasing");
	m_hAntiAliasing = antiAlias->GetHandle();
	if(antiAlias != nullptr) {
		auto antiAliasingType = static_cast<rendering::AntiAliasing>(get_client_state()->GetConVarInt("cl_render_anti_aliasing"));
		switch(antiAliasingType) {
		case rendering::AntiAliasing::FXAA:
			antiAlias->SelectChoice("fxaa");
			break;
		case rendering::AntiAliasing::MSAA:
			{
				auto msaaSamples = get_client_state()->GetConVarInt("cl_render_msaa_samples");
				//antiAlias->SelectChoice("msaa" +std::to_string(static_cast<int32_t>(sqrt(msaaSamples))));
				break;
			}
		}
	}
	//
	// Tone Mapping
	auto *populateToneMapping = +[](WIChoiceList *pList) {
		pList->AddChoice(locale::get_text("default"), "-1");
		pList->AddChoice(locale::get_text("gamma_correction"), "0");
		pList->AddChoice(locale::get_text("tone_mapping_reinhard"), "1");
		pList->AddChoice(locale::get_text("tone_mapping_hejil_richard"), "2");
		pList->AddChoice(locale::get_text("tone_mapping_uncharted"), "3");
		pList->AddChoice(locale::get_text("tone_mapping_aces"), "4");
		pList->AddChoice(locale::get_text("tone_mapping_gran_turismo"), "5");
	};
	auto *pToneMappingList = pList->AddChoiceList(
	  locale::get_text("tone_mapping"),
	  populateToneMapping,
	  "cl_render_tone_mapping");
	//
	// SSAO
	m_hSsao = pList->AddToggleChoice(locale::get_text("ssao"), "cl_render_ssao")->GetHandle();
	//
	// HDRR
	m_hdrr = pList->AddToggleChoice(locale::get_text("hdrr"), "cl_render_hdrr")->GetHandle();
	//
	// Bloom
	m_hBloom = pList->AddToggleChoice(locale::get_text("bloom"), "cl_render_bloom")->GetHandle();
	//
	// Motion Blur
	m_hMotionBlur = pList->AddSlider(locale::get_text("motion_blur"), sliderInitializer, "cl_render_motion_blur")->GetHandle();
	//
	// Horizontal FOV
	pList->AddSlider(locale::get_text("horizontal_fov"), [](WISlider *pSlider) { pSlider->SetRange(60, 120); }, "cl_render_fov");
	//
	// Brightness
	pList->AddSlider(locale::get_text("brightness"), sliderInitializer, "cl_render_brightness");
	//
	// Contrast
	pList->AddSlider(locale::get_text("contrast"), sliderInitializer, "cl_render_contrast");
	//
	// Texture Filtering
	auto *populateTextureFiltering = +[](WIChoiceList *pList) {
		pList->AddChoice(locale::get_text("off"), "0");
		pList->AddChoice(locale::get_text("texfilter_bilinear_filtering"), "1");
		pList->AddChoice(locale::get_text("texfilter_trilinear_filtering"), "2");

		auto limits = get_cengine()->GetRenderContext().GetPhysicalDeviceLimits();
		std::vector<int> anisotropy;
		auto maxAnisotropy = limits.maxSamplerAnisotropy;
		if(maxAnisotropy >= 2.f) {
			anisotropy.push_back(2);
			if(maxAnisotropy >= 4.f) {
				anisotropy.push_back(4);
				if(maxAnisotropy >= 8.f) {
					anisotropy.push_back(8);
					if(maxAnisotropy >= 16.f)
						anisotropy.push_back(16);
				}
			}
		}
		auto start = 3;
		for(auto it = anisotropy.begin(); it != anisotropy.end(); ++it) {
			auto v = *it;
			pList->AddChoice(locale::get_text("texfilter_anisotropic_filtering") + " x" + std::to_string(v), std::to_string(start++));
		}
	};
	auto *texFilter = pList->AddChoiceList(
	  locale::get_text("texture_filtering"),
	  populateTextureFiltering,
	  "cl_render_texture_filtering");
	m_hTextureFiltering = texFilter->GetHandle();
	//
	// Occlusion Culling
	auto *populateOcclusionCulling = +[](WIChoiceList *pList) {
		pList->AddChoice(locale::get_text("disabled"), "0");
		pList->AddChoice(locale::get_text("bruteforce"), "1");
		pList->AddChoice(locale::get_text("chcplusplus"), "2");
		pList->AddChoice(locale::get_text("bsp_and_octree"), "4");
	};
	auto *occlusionCulling = pList->AddChoiceList(
	  locale::get_text("occlusion_culling"),
	  populateOcclusionCulling,
	  "cl_render_occlusion_culling");
	m_hOcclusionCulling = occlusionCulling->GetHandle();
	//
	// DOF
	m_hDoF = pList->AddToggleChoice(locale::get_text("depth_of_field"), "cl_render_depth_of_field")->GetHandle();
	//m_hDOF = dof->GetHandle();
	//
	// Present Mode
	auto *populatePresentMode = +[](WIChoiceList *pList) {
		auto limits = get_cengine()->GetRenderContext().GetPhysicalDeviceLimits();
		auto maxImageCount = limits.maxSurfaceImageCount;
		if(maxImageCount > 0) {
			if(get_cengine()->GetRenderContext().IsPresentationModeSupported(prosper::PresentModeKHR::Immediate))
				pList->AddChoice(locale::get_text("immediate"), "0");
			if(maxImageCount > 1) {
				if(get_cengine()->GetRenderContext().IsPresentationModeSupported(prosper::PresentModeKHR::Fifo))
					pList->AddChoice(locale::get_text("fifo"), "1");
				if(maxImageCount > 2) {
					if(get_cengine()->GetRenderContext().IsPresentationModeSupported(prosper::PresentModeKHR::Mailbox))
						pList->AddChoice(locale::get_text("mailbox"), "2");
				}
			}
		}
	};
	auto *presentMode = pList->AddChoiceList(
	  locale::get_text("present_mode"),
	  populatePresentMode,
	  "cl_render_present_mode");
	m_hPresentMode = presentMode->GetHandle();
	//
	// Particle Quality
	auto *populateRenderParticleQuality = +[](WIChoiceList *pList) {
		pList->AddChoice(locale::get_text("disabled"), "0");
		pList->AddChoice(locale::get_text("low"), "1");
		pList->AddChoice(locale::get_text("medium"), "2");
		pList->AddChoice(locale::get_text("high"), "3");
	};
	auto *particleQuality = pList->AddChoiceList(
	  locale::get_text("particle_quality"),
	  populateRenderParticleQuality,
	  "cl_render_particle_quality");
	m_hParticleQuality = particleQuality->GetHandle();
	//
	// Shader Quality
	auto *populateShaderQuality = +[](WIChoiceList *pList) {
		pList->AddChoice(locale::get_text("very_low"), "1");
		pList->AddChoice(locale::get_text("low"), "3");
		pList->AddChoice(locale::get_text("medium"), "6");
		pList->AddChoice(locale::get_text("high"), "8");
		pList->AddChoice(locale::get_text("very_high"), "10");
	};
	auto *shaderQuality = pList->AddChoiceList(
	  locale::get_text("shader_quality"),
	  populateShaderQuality,
	  "cl_render_shader_quality");
	m_hShaderQuality = shaderQuality->GetHandle();
	//
	// Reflection Quality
	auto *populateReflectionQuality = +[](WIChoiceList *pList) {
		pList->AddChoice(locale::get_text("static_only"), "0");
		pList->AddChoice(locale::get_text("dynamic"), "1");
		pList->AddChoice(locale::get_text("full"), "2");
	};
	auto *pReflectionQuality = pList->AddChoiceList(
	  locale::get_text("reflection_quality"),
	  populateReflectionQuality,
	  "cl_render_reflection_quality");
	m_hReflectionQuality = pReflectionQuality->GetHandle();
	//
	auto *pRow = pList->AddHeaderRow();
	pRow->SetValue(0, locale::get_text("lighting_and_shadows"));
	// Shadow Resolution
	auto *populateShadowResolution = +[](WIChoiceList *pList) {
		pList->AddChoice("256x256", "256");
		pList->AddChoice("512x512", "512");
		pList->AddChoice("1024x1024", "1024");
		pList->AddChoice("2048x2048", "2048");
		//pList->AddChoice(pragma::locale::get_text("very_high"),"4096"); // NOT supported! See "cl_render_shadow_resolution"-callback in c_shadowmap.cpp for more information.
	};
	auto *shadowRes = pList->AddChoiceList(
	  locale::get_text("shadow_resolution"),
	  populateShadowResolution,
	  "cl_render_shadow_resolution");
	m_hShadowRes = shadowRes->GetHandle();
	//
	// Shadow Quality
	auto *populateShadowQuality = +[](WIChoiceList *pList) {
		pList->AddChoice(locale::get_text("disabled"), "0");
		pList->AddChoice(locale::get_text("low"), "1");
		pList->AddChoice(locale::get_text("medium"), "2");
		pList->AddChoice(locale::get_text("high"), "3");
		pList->AddChoice(locale::get_text("very_high"), "4");
	};
	auto *pShadowQuality = pList->AddChoiceList(
	  locale::get_text("shadow_quality"),
	  populateShadowQuality,
	  "render_shadow_quality");
	m_hShadowQuality = pShadowQuality->GetHandle();
	//
	// Dynamic Shadows
	m_hDynamicShadows = pList->AddToggleChoice(locale::get_text("shadow_enable_dynamic"), "cl_render_shadow_dynamic")->GetHandle();
	//
	// Shadow Update Frequency
	m_hShadowUpdateFrequency = pList->AddSlider(locale::get_text("shadow_update_frequency"), [](WISlider *pSlider) { pSlider->SetRange(0, 10); }, "cl_render_shadow_update_frequency")->GetHandle();
	//
	// PSSM Shadow Update Frequency Offset
	m_hPssmShadowUpdateFrequencyOffset = pList->AddSlider(locale::get_text("shadow_pssm_update_frequency_offset"), [](WISlider *pSlider) { pSlider->SetRange(0, 10); }, "cl_render_shadow_pssm_update_frequency_offset")->GetHandle();
	//
	// PSSM Split Count
	m_hPssmSplitCount = pList->AddSlider(locale::get_text("shadow_pssm_split_count"), [](WISlider *pSlider) { pSlider->SetRange(1.f, static_cast<float>(CShadowCSMComponent::MAX_CASCADE_COUNT)); }, "cl_render_shadow_pssm_split_count")->GetHandle();
	//
	// TODO Restore defaults

	pRow = pList->AddHeaderRow();
	pRow->SetValue(0, locale::get_text("virtual_reality"));
	auto *pRowVr = pRow;
	// Virtual Reality
	auto *vrSupport = pList->AddToggleChoice(locale::get_text("vr_support_enable"), "cl_render_vr_enabled");
	/*auto *vrResMenu = pList->AddDropDownMenu(pragma::locale::get_text("vr_resolution"),[](WIDropDownMenu *pMenu) {
		std::vector<std::string> resolutions = {
			"128x128",
			"256x256"
		};
		for(auto &res : resolutions)
			pMenu->AddOption(res,res);
	},"cl_render_vr_resolution");
	vrResMenu->SizeToContents();*/
	//

	// Memory usage
	// TODO
#if 0
	pRow = pList->AddHeaderRow();
	pRow->SetValue(0,pragma::locale::get_text("gpu_memory_statistics"));
	m_hGPUMemoryUsage = pList->AddSlider(pragma::locale::get_text("gpu_memory_current_usage"),[](WISlider *pSlider) {
		auto memProps = prosper::util::get_physical_device_memory_properties(pragma::get_cengine()->GetRenderContext());
		if(memProps.has_value() == false)
			return;
		auto totalSize = 0ull;
		for(auto heapSize : memProps->heapSizes)
			totalSize += heapSize;
		pSlider->SetRange(0,totalSize,1);
		pSlider->SetMouseInputEnabled(false);
		pSlider->GetProgressProperty()->AddCallback([pSlider](std::reference_wrapper<const float> oldVal,std::reference_wrapper<const float> newVal) {
			auto percent = newVal.get();
			auto &col0 = colors::LimeGreen;
			auto &col1 = colors::DarkOrange;
			auto &col2 = colors::Crimson;
			Color col {};
			if(percent <= 0.75)
				col = col0.Lerp(col1,percent /0.75);
			else
				col = col1.Lerp(col2,pragma::math::min((percent -0.75) /0.25,1.0));
			pSlider->SetColor(col);
		});
		pSlider->SetValueTranslator([totalSize,pSlider](float f) -> std::string {
			auto percent = f /static_cast<double>(totalSize);
			return pragma::util::get_pretty_bytes(static_cast<uint64_t>(f)) +" / " +util::get_pretty_bytes(totalSize) +" (" +util::round_string(percent *100.0,2) +"%)";
		});
	})->GetHandle();
#endif
	//

	InitializeOptionsList(pList);

	if(get_client_state()->GetConVarInt("cl_render_preset") < 0) {
		pListPreset->SelectChoice(3);
		if(m_hButtonApply.IsValid()) {
			SetActiveMenu(m_hVideoSettings);
			m_hButtonApply->InjectMouseInput(platform::MouseButton::Left, platform::KeyState::Press, {});
			m_hButtonApply->InjectMouseInput(platform::MouseButton::Left, platform::KeyState::Release, {});
			WIHandle hMenu {};
			SetActiveMenu(hMenu);
		}
	}

	auto showAdvancedOptions = get_cengine()->IsDeveloperModeEnabled();
	if(showAdvancedOptions == false) {
		pList->GetRow("cl_render_vsync_enabled")->SetVisible(false);
		pList->GetRow("cl_material_streaming_enabled")->SetVisible(false);
		pList->GetRow("cl_render_anti_aliasing")->SetVisible(false);
		pList->GetRow("cl_render_ssao")->SetVisible(false);
		pList->GetRow("cl_render_hdrr")->SetVisible(false);
		pList->GetRow("cl_render_bloom")->SetVisible(false);
		pList->GetRow("cl_render_motion_blur")->SetVisible(false);
		pList->GetRow("cl_render_occlusion_culling")->SetVisible(false);
		pList->GetRow("cl_render_depth_of_field")->SetVisible(false);
		pList->GetRow("cl_render_present_mode")->SetVisible(false);
		pList->GetRow("cl_render_particle_quality")->SetVisible(false);
		pList->GetRow("cl_render_reflection_quality")->SetVisible(false);
		pList->GetRow("cl_render_shadow_resolution")->SetVisible(false);
		pList->GetRow("cl_render_shadow_dynamic")->SetVisible(false);
		pList->GetRow("cl_render_shadow_update_frequency")->SetVisible(false);
		pList->GetRow("cl_render_shadow_pssm_update_frequency_offset")->SetVisible(false);
		pList->GetRow("cl_render_shadow_pssm_split_count")->SetVisible(false);
		pList->GetRow("cl_render_vr_enabled")->SetVisible(false);
		pList->GetRow("render_shadow_quality")->SetVisible(false);
	}
}
void pragma::gui::types::WIMainMenuOptions::InitializeAudioSettings()
{
	m_hAudioSettings = CreateChild<WIOptionsList>();
	m_hAudioSettings->SetName("settings_audio");
	auto *pList = static_cast<WIOptionsList *>(m_hAudioSettings.get());
	auto title = locale::get_text("audio_options");
	string::to_upper(title);
	pList->SetTitle(title);
	// Audio Device
	/*WIDropDownMenu *audioDevice = pList->AddDropDownMenu(pragma::locale::get_text("audio_device"));
	auto alDevices = alcGetString(nullptr,ALC_DEVICE_SPECIFIER);
	UInt i = 0;
	while(alDevices[i] != '\0' && alDevices[i +1] != '\0')
	{
		auto device = std::string(&alDevices[i]);
		audioDevice->AddOption(device);
		i += device.length();
	}
	audioDevice->SizeToContents();*/
	//

	// Audio Device
	//WIDropDownMenu *captureDevice = pList->AddDropDownMenu(pragma::locale::get_text("capture_device"));
	//auto r = alcGetString(nullptr,ALC_CAPTURE_DEVICE_SPECIFIER);
	//

	// Master Volume
	pList->AddSlider(locale::get_text("master_volume"), sliderInitializer, "cl_audio_master_volume");
	//

	// Effects Volume
	pList->AddSlider(locale::get_text("effects_volume"), sliderInitializer, "cl_effects_volume");
	//

	// Music Volume
	pList->AddSlider(locale::get_text("music_volume"), sliderInitializer, "cl_music_volume");
	//

	// Voice Volume
	pList->AddSlider(locale::get_text("voice_volume"), sliderInitializer, "cl_voice_volume");
	//

	// GUI Volume
	pList->AddSlider(locale::get_text("gui_volume"), sliderInitializer, "cl_gui_volume");
	//

	// Sound Device
	/*auto *soundDeviceMenu = pList->AddDropDownMenu(pragma::locale::get_text("audio_device"),[](WIDropDownMenu *pMenu) {
		auto devices = pragma::audio::get_devices();
		for(auto &dev : devices)
			pMenu->AddOption(dev,dev); // TODO: ID?
	},"cl_audio_device");*/
	//

	// Enable HRTF
	pList->AddToggleChoice(locale::get_text("enable_hrtf"), "cl_audio_hrtf_enabled");
	//

	// Audio Streaming
	pList->AddToggleChoice(locale::get_text("audio_streaming"), "cl_audio_streaming_enabled");
	//

	// Enable Microphone
	pList->AddToggleChoice(locale::get_text("enable_microphone"), "");
	//

	// Play sounds when window is not active
	pList->AddToggleChoice(locale::get_text("play_sounds_if_inactive"), "cl_audio_always_play");
	//

	auto *pRow = pList->AddHeaderRow();
	pRow->SetValue(0, locale::get_text("steam_audio"));

	// Enable / Disable
	pList->AddToggleChoice(locale::get_text("enable_steam_audio"), "cl_steam_audio_enabled");
	//

	// Number of rays
	pList->AddSlider(locale::get_text("steam_audio_number_of_rays"), [](WISlider *pSlider) { pSlider->SetRange(1'024, 131'072, 1'024); }, "cl_steam_audio_number_of_rays");
	//

	// Number of bounces
	pList->AddSlider(locale::get_text("steam_audio_number_of_bounces"), [](WISlider *pSlider) { pSlider->SetRange(1, 32, 1); }, "cl_steam_audio_number_of_bounces");
	//

	// Ir duration
	pList->AddSlider(locale::get_text("steam_audio_ir_duration"), [](WISlider *pSlider) { pSlider->SetRange(0.5f, 4.0f, 0.1f); }, "cl_steam_audio_ir_duration");
	//

	// Ambisonics order
	pList->AddSlider(locale::get_text("steam_audio_ambisonics_order"), [](WISlider *pSlider) { pSlider->SetRange(0, 3, 1); }, "cl_steam_audio_ambisonics_order");
	//

	// Sound propagation delay
	pList->AddToggleChoice(locale::get_text("enable_propagation_delay"), "cl_steam_audio_propagation_delay_enabled");
	//

	InitializeOptionsList(pList);
}
void pragma::gui::types::WIMainMenuOptions::InitializeControlSettings()
{
	m_hControlSettings = CreateChild<WIOptionsList>();
	m_hControlSettings->SetName("settings_controls");
	auto *pList = static_cast<WIOptionsList *>(m_hControlSettings.get());
	auto title = locale::get_text("control_options");
	string::to_upper(title);
	pList->SetTitle(title);
	// Mouse Sensitivity
	pList->AddSlider(locale::get_text("mouse_sensitivity"), [](WISlider *pSlider) { pSlider->SetRange(0.f, 4.f, 0.f); }, "cl_mouse_sensitivity");
	//

	// Invert Y-Axis
	pList->AddToggleChoice(
	  locale::get_text("invert_y_axis"), "cl_mouse_pitch",
	  [](bool b) {
		  if(b == true)
			  return "1";
		  return "-1";
	  },
	  [](std::string str) {
		  if(util::to_float(str) > 0.f)
			  return true;
		  return false;
	  });
	//

	// Mouse Acceleration
	pList->AddSlider(locale::get_text("mouse_acceleration"), sliderInitializer, "cl_mouse_acceleration");
	//

	// Raw Input
	//pList->AddToggleChoice(pragma::locale::get_text("raw_input")); // TODO
	//

	// Confine Mouse Cursor
	//pList->AddToggleChoice(pragma::locale::get_text("confine_mouse_cursor")); // TODO
	//
	/*WIHandle m_hKeyBindings = pragma::gui::WGUI::Create<WIKeyBindingList>(pList)->GetHandle();
	WIKeyBindingList *t = m_hKeyBindings.get<WIKeyBindingList>();
	t->SetPos(500,64);
	t->SetSize(500,24 *t->GetRowCount());*/

	auto *pCheckbox = pList->AddToggleChoice(locale::get_text("enable_controllers"), "cl_controller_enabled");
	// Has to be applied immediately, otherwise controller keys can't be bound before applying the settings first
	pCheckbox->AddCallback("OnChange", FunctionCallback<void>::Create([pCheckbox]() { get_cengine()->SetControllersEnabled(pCheckbox->IsChecked()); }));

	auto *pRow = pList->AddHeaderRow();
	pRow->SetValue(0, locale::get_text("action"));
	pRow->SetValue(1, locale::get_text("key") + " 1");
	pRow->SetValue(2, locale::get_text("key") + " 2");

	pList->AddKeyBinding(locale::get_text("key_use"), "+use");

	pRow = pList->AddHeaderRow();
	pRow->SetValue(0, locale::get_text("movement"));
	pList->AddKeyBinding(locale::get_text("key_move_forward"), "+forward");
	pList->AddKeyBinding(locale::get_text("key_move_back"), "+backward");
	pList->AddKeyBinding(locale::get_text("key_move_left"), "+left");
	pList->AddKeyBinding(locale::get_text("key_move_right"), "+right");

	pList->AddKeyBinding(locale::get_text("key_turn_up"), "+turn_up");
	pList->AddKeyBinding(locale::get_text("key_turn_down"), "+turn_down");
	pList->AddKeyBinding(locale::get_text("key_turn_left"), "+turn_left");
	pList->AddKeyBinding(locale::get_text("key_turn_right"), "+turn_right");

	pList->AddKeyBinding(locale::get_text("key_sprint"), "+sprint");
	pList->AddKeyBinding(locale::get_text("key_walk"), "+walk");
	pList->AddKeyBinding(locale::get_text("key_strafe"), "+strafe");
	pList->AddKeyBinding(locale::get_text("key_jump"), "+jump");
	pList->AddKeyBinding(locale::get_text("key_duck"), "+crouch");
	pList->AddKeyBinding(locale::get_text("key_noclip"), "noclip");

	pRow = pList->AddHeaderRow();
	pRow->SetValue(0, locale::get_text("gameplay"));
	pRow->SetValue(1, "");
	pRow->SetValue(2, "");
	pList->AddKeyBinding(locale::get_text("key_suicide"), "suicide");
	pList->AddKeyBinding(locale::get_text("key_flashlight"), "flashlight");
	pList->AddKeyBinding(locale::get_text("key_toggle_thirdperson"), "thirdperson");

	pRow = pList->AddHeaderRow();
	pRow->SetValue(0, locale::get_text("combat"));
	pRow->SetValue(1, "");
	pRow->SetValue(2, "");

	pList->AddKeyBinding(locale::get_text("key_primary_attack"), "+attack");
	pList->AddKeyBinding(locale::get_text("key_secondary_attack"), "+attack2");
	pList->AddKeyBinding(locale::get_text("key_tertiary_attack"), "+attack3");
	pList->AddKeyBinding(locale::get_text("key_quaternary_attack"), "+attack4");
	pList->AddKeyBinding(locale::get_text("key_reload"), "+reload");
	pList->AddKeyBinding(locale::get_text("key_drop_weapon"), "drop");
	pList->AddKeyBinding(locale::get_text("key_next_weapon"), "next_weapon");
	pList->AddKeyBinding(locale::get_text("key_previous_weapon"), "previous_weapon");

	pRow = pList->AddHeaderRow();
	pRow->SetValue(0, locale::get_text("other"));
	pRow->SetValue(1, "");
	pRow->SetValue(2, "");

	pList->AddKeyBinding(locale::get_text("key_spraylogo"), "spray");
	pList->AddKeyBinding(locale::get_text("key_screenshot"), "screenshot");
	pList->AddKeyBinding(locale::get_text("key_quit"), "exit");
	pList->AddKeyBinding(locale::get_text("chat"), "chat");
	pList->AddKeyBinding(locale::get_text("key_toggle_vr_view"), "toggle cl_vr_hmd_view_enabled");
	pList->AddKeyBinding(locale::get_text("key_toggle_vr_mirror_window"), "toggle cl_vr_mirror_window_enabled");

	// Initialize custom control options from addons
	auto *upadManager = dynamic_cast<pad::PackageManager *>(fs::get_package_manager("upad"));
	auto &mountedAddons = AddonSystem::GetMountedAddons();
	for(auto &info : mountedAddons) {
		auto addonPath = info.GetAbsolutePath();
		fs::VFilePtr f = nullptr;
		std::string ext;
		if(ufile::get_extension(addonPath, &ext) == true && ext == "pad") {
			if(upadManager != nullptr)
				f = upadManager->OpenFile(info.GetLocalPath(), "menu.xml", false, fs::SearchFlags::All, fs::SearchFlags::All); // TODO: Absolute path
		}
		else {
			auto path = addonPath + "\\menu.xml";
			f = fs::open_system_file(path, fs::FileMode::Read);
		}
		if(f != nullptr) {
			auto content = f->ReadString();

			rapidxml::xml_document<> xmlDoc {};
			auto *pcontent = xmlDoc.allocate_string(content.c_str(), content.length() + 1); // +1 = null-terminator
			try {
				xmlDoc.parse<0>(pcontent);

				auto *nodeLocale = xmlDoc.first_node("locale");
				while(nodeLocale != nullptr) {
					locale::load(nodeLocale->value());

					nodeLocale = nodeLocale->next_sibling("locale");
				}

				auto *nodeGame = xmlDoc.first_node("game");
				if(nodeGame != nullptr) {
					auto *nodeMenu = nodeGame->first_node("menu");
					if(nodeMenu != nullptr) {
						auto *nodeOptions = nodeMenu->first_node("options");
						if(nodeOptions != nullptr) {
							auto *nodeControls = nodeOptions->first_node("controls");
							if(nodeControls != nullptr) {
								auto *nodeControl = nodeControls->first_node("control");
								while(nodeControl != nullptr) {
									auto *nodeName = nodeControl->first_node("name");
									if(nodeName != nullptr) {
										auto *attrHeader = nodeControl->first_attribute("header");
										auto bHeader = (attrHeader != nullptr && strcmp(attrHeader->value(), "true") == 0) ? true : false;
										if(bHeader == true) {
											pRow = pList->AddHeaderRow();
											pRow->SetValue(0, locale::get_text(nodeName->value()));
											pRow->SetValue(1, "");
											pRow->SetValue(2, "");
										}
										else {
											auto *nodeCmd = nodeControl->first_node("cmd");
											if(nodeCmd != nullptr)
												pList->AddKeyBinding(locale::get_text(nodeName->value()), nodeCmd->value());
										}
									}
									nodeControl = nodeControl->next_sibling("control");
								}
							}
						}
					}
				}
			}
			catch(const rapidxml::parse_error &err) {
				Con::CWAR << "Unable to parse 'menu.xml' for addon '" << info.GetLocalPath() << "': " << err.what() << "!" << Con::endl;
			}
		}
	}

	InitializeOptionsList(pList);
}
