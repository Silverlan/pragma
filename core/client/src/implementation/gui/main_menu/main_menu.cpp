// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

#define DLLSPEC_ISTEAMWORKS DLLNETWORK

#undef PlaySound

import :gui.main_menu;

import :client_state;
import :engine;
import :game;

pragma::gui::types::WIMainMenu::WIMainMenu() : WIBase(), m_menuType(0), m_tOpen(0.0)
{
	SetKeyboardInputEnabled(true);
	SetZPos(1000);
}

pragma::gui::types::WIMainMenu::~WIMainMenu()
{
	if(m_cbBlur.IsValid())
		m_cbBlur.Remove();
	if(m_cbOnGameStart.IsValid())
		m_cbOnGameStart.Remove();
	if(m_cbOnGameEnd.IsValid())
		m_cbOnGameEnd.Remove();
	if(m_hServerBrowser.IsValid())
		m_hServerBrowser->Remove();
	if(m_cbMenuTrack.IsValid())
		m_cbMenuTrack.Remove();
	if(m_menuSound != nullptr)
		m_menuSound->Stop();

	if(m_cbOnSteamworksInit.IsValid())
		m_cbOnSteamworksInit.Remove();
	if(m_cbOnSteamworksShutdown.IsValid())
		m_cbOnSteamworksShutdown.Remove();
}

pragma::util::EventReply pragma::gui::types::WIMainMenu::KeyboardCallback(platform::Key key, int scanCode, platform::KeyState state, platform::Modifier mods)
{
	if(!m_hActive.IsValid())
		return util::EventReply::Handled;
	return m_hActive->KeyboardCallback(key, scanCode, state, mods);
}

void pragma::gui::types::WIMainMenu::OnVisibilityChanged(bool bVisible)
{
	WIBase::OnVisibilityChanged(bVisible);
	if(get_cgame() == nullptr)
		return;
	if(bVisible == true) {
		double tCur = get_cgame()->RealTime();
		m_tOpen = tCur;
		// Obsolete?
		/*m_cbBlur = pragma::get_cgame()->AddCallback("RenderPostProcessing",FunctionCallback<void,unsigned int,unsigned int>::Create([this](unsigned int ppFBO,unsigned int) {
			double t = pragma::get_cgame()->RealTime();
			double tDelta = t -m_tOpen;
			float blurSize = (CFloat(tDelta) /0.1f) *3.f;
			if(blurSize > 2.f)
				blurSize = 2.f;
			RenderBlur(ppFBO,blurSize,GetWidth(),GetHeight());
		}));*/
		return;
	}
	if(m_cbBlur.IsValid())
		m_cbBlur.Remove();
}

void pragma::gui::types::WIMainMenu::PlayNextMenuTrack(bool newRound)
{
	if(m_menuSound != nullptr) {
		m_menuSound->Stop();
		m_menuSound = nullptr;
	}
	if(m_cbMenuTrack.IsValid())
		m_cbMenuTrack.Remove();
	if(m_menuTracks.empty()) {
		fs::find_files("sounds/ui/gamestartup*.*", &m_menuTracks, nullptr);
		newRound = true;
	}
	if(m_menuTracks.empty())
		return;
	auto next = math::random(0, CUInt32(m_menuTracks.size() - 1));
	auto it = m_menuTracks.begin() + next;
	auto sound = *it;
	m_menuTracks.erase(it);
	auto *client = get_client_state();
	if(client->PrecacheSound(std::string("ui/") + sound) == false || (m_menuSound = client->PlaySound(std::string("ui/") + sound, audio::ALSoundType::GUI, audio::ALCreateFlags::None)) == nullptr) {
		if(newRound == false)
			PlayNextMenuTrack(newRound);
	}
	else {
		m_menuSound->SetType(audio::ALSoundType::Music | audio::ALSoundType::GUI);
		// m_menuSound->SetPitch(0.4f);
		// m_menuSound->SetGain(0.2f);
		m_cbMenuTrack = FunctionCallback<void, audio::ALState, audio::ALState>::Create([this](audio::ALState, audio::ALState newstate) {
			if(newstate != audio::ALState::Playing)
				this->PlayNextMenuTrack();
		});
		m_menuSound->AddCallback("OnStateChanged", m_cbMenuTrack);
	}
}

//#include "pragma/util/curl_query_handler.hpp"
void pragma::gui::types::WIMainMenu::Initialize()
{
	WIBase::Initialize();

	/*pragma::CurlQueryHandler curlQueryHandler {};
	curlQueryHandler.AddRequest("https://wiki.pragma-engine.com/",{},[](int32_t,const std::string &response) {
		std::cout<<"Complete!"<<std::endl;
		std::cout<<response<<std::endl;
	},[](int64_t,int64_t,int64_t,int64_t) {
		std::cout<<"Progress..."<<std::endl;
	});
	curlQueryHandler.StartDownload();
	while(curlQueryHandler.IsComplete() == false);*/

	m_hPragmaLogo = CreateChild<WITexturedRect>();
	auto *pTex = static_cast<WITexturedRect *>(m_hPragmaLogo.get());
	pTex->SetMaterial("wgui/pragma_logo");
	pTex->SizeToTexture();
	pTex->SetPos(192, 80);
	pTex->SetHeight(64, true);
	pTex->SetZPos(1'000);

	SetSize(1024, 768);
	m_hBg = CreateChild<WIRect>();
	WIRect *bg = static_cast<WIRect *>(m_hBg.get());
	bg->SetColor(0, 0, 0, 0.5f);
	bg->SetSize(GetSize());
	bg->SetAnchor(0.f, 0.f, 1.f, 1.f);

	bg->SetMouseInputEnabled(true);
	bg->SetMouseMovementCheckEnabled(true);
	//bg->AddCallback("OnCursorEntered",FunctionCallback<>::Create([]() {
	//	std::cout<<"ENTERED!"<<std::endl;
	//}));

	m_hBgSlideShow = CreateChild<WIImageSlideShow>();
	auto *pImageSlideShow = static_cast<WIImageSlideShow *>(m_hBgSlideShow.get());
	pImageSlideShow->SetSize(GetWidth(), GetHeight());
	pImageSlideShow->SetAnchor(0.f, 0.f, 1.f, 1.f);
	pImageSlideShow->SetColor(0.75f, 0.75f, 0.75f, 1.f);
	std::vector<std::string> imgFiles;
	fs::find_files("screenshots/*.tga", &imgFiles, nullptr);
	fs::find_files("screenshots/*.png", &imgFiles, nullptr);
	for(auto it = imgFiles.begin(); it != imgFiles.end(); it++)
		*it = "screenshots/" + *it;
	pImageSlideShow->SetImages(imgFiles);

	//std::shared_ptr<pragma::audio::ALSound> PlaySound(std::string snd,int mode=AL_CHANNEL_AUTO,unsigned char priority=0);
	m_hMain = CreateChild<WIMainMenuBase>();
	WIMainMenuBase *menu = static_cast<WIMainMenuBase *>(m_hMain.get());
	menu->SetVisible(false);
	menu->SetSize(GetWidth(), GetHeight());
	menu->SetAnchor(0.f, 0.f, 1.f, 1.f);
	menu->AddMenuItem(locale::get_text("menu_newgame"), FunctionCallback<>::Create([this]() { SetActiveMenu(m_hNewGame); }));
	menu->AddMenuItem(locale::get_text("menu_find_servers"), FunctionCallback<>::Create([this]() {
		if(m_hServerBrowser.IsValid())
			m_hServerBrowser->Remove();
		m_hServerBrowser = CreateChild<WIServerBrowser>();
		auto *sb = static_cast<WIServerBrowser *>(m_hServerBrowser.get());
		sb->SetKeyboardInputEnabled(true);
		sb->SetMouseInputEnabled(true);
		sb->SetPos(200, 200);
		sb->RequestFocus();
	}));
#ifdef _DEBUG
	menu->AddMenuItem(pragma::locale::get_text("menu_loadgame"), FunctionCallback<>::Create([this]() { SetActiveMenu(m_hLoad); }));
#endif
	menu->AddMenuItem(locale::get_text("menu_options"), FunctionCallback<>::Create([this]() { SetActiveMenu(m_hOptions); }));
	/*menu->AddMenuItem(pragma::locale::get_text("menu_addons"), FunctionCallback<>::Create([this]() {
		//SetActiveMenu(m_hMods);
		//ShellExecute(0,0,engine_info::get_modding_hub_url().c_str(),0,0,SW_SHOW);
		pragma::util::open_url_in_browser("steam://url/SteamWorkshopPage/" + std::to_string(engine_info::get_steam_app_id()));
	}));*/
#ifdef _DEBUG
	menu->AddMenuItem("Loadscreen", FunctionCallback<>::Create([this]() { SetActiveMenu(m_hLoadScreen); }));
#endif
	menu->AddMenuItem(locale::get_text("menu_quit"), FunctionCallback<>::Create([]() { get_cengine()->ShutDown(); }));
	menu->SetKeyboardInputEnabled(true);

	m_hNewGame = CreateChild<WIMainMenuNewGame>();
	WIMainMenuNewGame *newGame = static_cast<WIMainMenuNewGame *>(m_hNewGame.get());
	newGame->SetVisible(false);
	newGame->SetSize(GetWidth(), GetHeight());
	newGame->SetAnchor(0.f, 0.f, 1.f, 1.f);
	newGame->SetKeyboardInputEnabled(true);

	m_hOptions = CreateChild<WIMainMenuOptions>();
	WIMainMenuOptions *options = static_cast<WIMainMenuOptions *>(m_hOptions.get());
	options->SetVisible(false);
	options->SetSize(GetWidth(), GetHeight());
	options->SetAnchor(0.f, 0.f, 1.f, 1.f);
	options->SetKeyboardInputEnabled(true);

	m_hMods = CreateChild<WIMainMenuMods>();
	auto *pMods = static_cast<WIMainMenuMods *>(m_hMods.get());
	pMods->SetVisible(false);
	pMods->SetSize(GetWidth(), GetHeight());
	pMods->SetAnchor(0.f, 0.f, 1.f, 1.f);
	pMods->SetKeyboardInputEnabled(true);

	m_hLoad = CreateChild<WIMainMenuLoadGame>();
	WIMainMenuLoadGame *loadGame = static_cast<WIMainMenuLoadGame *>(m_hLoad.get());
	loadGame->SetVisible(false);
	loadGame->SetSize(GetWidth(), GetHeight());
	loadGame->SetAnchor(0.f, 0.f, 1.f, 1.f);
	loadGame->SetKeyboardInputEnabled(true);

	m_hLoadScreen = CreateChild<WILoadScreen>();
	auto *pLoadScreen = static_cast<WILoadScreen *>(m_hLoadScreen.get());
	pLoadScreen->SetVisible(false);
	pLoadScreen->SetSize(GetWidth(), GetHeight());
	pLoadScreen->SetAnchor(0.f, 0.f, 1.f, 1.f);
	pLoadScreen->SetKeyboardInputEnabled(true);

	m_hVersion = CreateChild<WIText>();

	auto version = get_pretty_engine_version();
	auto gitInfo = engine_info::get_git_info();
	if(gitInfo.has_value())
		version += " [" + string::substr(gitInfo->commitSha, 0, 7) + "]";
	auto *pVersion = static_cast<WIText *>(m_hVersion.get());
	pVersion->AddStyleClass("game_version");
	pVersion->SetColor(1.f, 1.f, 1.f, 1.f);
	pVersion->SetText(version);
	pVersion->SizeToContents();
	pVersion->SetName("engine_version");
	pVersion->SetPos(GetWidth() - pVersion->GetWidth() - 40, GetHeight() - pVersion->GetHeight() - 20);
	pVersion->SetAnchor(1.f, 1.f, 1.f, 1.f);

	auto *client = get_client_state();
	m_cbOnSteamworksInit = client->AddCallback("OnSteamworksInitialized", FunctionCallback<void, std::reference_wrapper<struct ISteamworks>>::Create([this](std::reference_wrapper<struct ISteamworks> isteamworks) {
		if(m_hVersion.IsValid() == false || isteamworks.get().get_build_id == nullptr)
			return;
		m_hBuild = CreateChild<WIText>();
		auto *pBuildId = static_cast<WIText *>(m_hBuild.get());
		pBuildId->AddStyleClass("game_version");
		pBuildId->SetColor(1.f, 1.f, 1.f, 1.f);
		pBuildId->SetText("Build: " + std::to_string(isteamworks.get().get_build_id()));
		pBuildId->SizeToContents();
		if(m_hVersion.IsValid()) {
			pBuildId->SetPos(m_hVersion->GetX() /*m_hVersion->GetRight() -pBuildId->GetWidth()*/, m_hVersion->GetY() - m_hVersion->GetHeight() - 5);
			pBuildId->SetAnchor(1.f, 1.f, 1.f, 1.f);
		}
	}));
	m_cbOnSteamworksShutdown = client->AddCallback("OnSteamworksShutdown", FunctionCallback<void>::Create([this]() {
		if(m_hBuild.IsValid())
			m_hBuild->Remove();
	}));

	if(Lua::get_extended_lua_modules_enabled() == true) {
		m_hVersionAttributes = CreateChild<WIText>();
		auto *pAttributes = static_cast<WIText *>(m_hVersionAttributes.get());
		pAttributes->AddStyleClass("game_version");
		pAttributes->SetColor(colors::Red);
		pAttributes->SetText("[D]");
		pAttributes->SizeToContents();
		if(m_hVersion.IsValid()) {
			pAttributes->SetPos(m_hVersion->GetRight() + 4, m_hVersion->GetY());
			pAttributes->SetAnchor(1.f, 1.f, 1.f, 1.f);
		}
	}

	/*m_hRenderAPI = CreateChild<WIText>();
	auto *pAttributes = m_hRenderAPI.get<WIText>();
	pAttributes->AddStyleClass("game_version");
	pAttributes->SetColor(colors::Lime);
	pAttributes->SetText("[" +pragma::get_cengine()->GetRenderContext().GetAPIAbbreviation() +"]");
	pAttributes->SizeToContents();*/

	/*WIHandle hConsole = CreateChild<WIConsole>();
	WIConsole *console = hConsole.get<WIConsole>();
	console->SetSize(256,512);
	console->SetPos(600,200);*/

	TrapFocus(true);
	RequestFocus();
	OpenMainMenu();

	m_cbOnGameStart = client->AddCallback("OnGameStart", FunctionCallback<void, CGame *>::Create([this](CGame *) {
		if(m_menuSound != nullptr) {
			m_menuSound->FadeOut(5.f);
			m_menuSound = nullptr;
			if(m_cbMenuTrack.IsValid())
				m_cbMenuTrack.Remove();
		}
		if(!m_hBgSlideShow.IsValid())
			return;
		m_hBgSlideShow->SetVisible(false);
	}));
	m_cbOnGameEnd = client->AddCallback("EndGame", FunctionCallback<void, CGame *>::Create([this](CGame *) {
		PlayNextMenuTrack();
		if(!m_hBgSlideShow.IsValid())
			return;
		m_hBgSlideShow->SetVisible(true);
	}));
	PlayNextMenuTrack();

	auto *pParent = GetParent();
	if(pParent != nullptr)
		SetSize(pParent->GetWidth(), pParent->GetHeight());
	SetAnchor(0.f, 0.f, 1.f, 1.f);
}

void pragma::gui::types::WIMainMenu::SetActiveMenu(WIHandle &hMenu)
{
	if(hMenu.get() == m_hActive.get() && hMenu.IsValid())
		return;
	if(m_hActive.IsValid())
		m_hActive->SetVisible(false);
	if(m_hPragmaLogo.IsValid())
		m_hPragmaLogo->SetVisible(hMenu.get() == m_hMain.get());
	if(!hMenu.IsValid())
		return;
	hMenu->SetVisible(true);
	//hMenu->TrapFocus(true);
	hMenu->RequestFocus();
	m_hActive = hMenu;
}
void pragma::gui::types::WIMainMenu::OpenMainMenu() { SetActiveMenu(m_hMain); }
void pragma::gui::types::WIMainMenu::OpenNewGameMenu() { SetActiveMenu(m_hNewGame); }
void pragma::gui::types::WIMainMenu::OpenLoadGameMenu() { SetActiveMenu(m_hLoad); }
void pragma::gui::types::WIMainMenu::OpenOptionsMenu() { SetActiveMenu(m_hOptions); }
void pragma::gui::types::WIMainMenu::OpenModsMenu() { SetActiveMenu(m_hMods); }

void pragma::gui::types::WIMainMenu::OpenLoadScreen() { SetActiveMenu(m_hLoadScreen); }

void pragma::gui::types::WIMainMenu::OnFocusGained()
{
	if(!m_hActive.IsValid())
		return;
	WIMainMenuBase *menu = static_cast<WIMainMenuBase *>(m_hActive.get());
	menu->RequestFocus();
}

void pragma::gui::types::WIMainMenu::OnFocusKilled()
{
	if(!m_hActive.IsValid())
		return;
	WIMainMenuBase *menu = static_cast<WIMainMenuBase *>(m_hActive.get());
	menu->KillFocus(true);
}

void pragma::gui::types::WIMainMenu::SetContinueMenu()
{
	if(m_menuType == 1 || !m_hMain.IsValid())
		return;
	m_menuType = 1;
	WIMainMenuBase *menu = static_cast<WIMainMenuBase *>(m_hMain.get());
	menu->AddMenuItem(0, locale::get_text("menu_resumegame"), FunctionCallback<>::Create([]() { get_client_state()->CloseMainMenu(); }));
	menu->AddMenuItem(1, locale::get_text("menu_disconnect"), FunctionCallback<>::Create([]() { get_cengine()->EndGame(); }));
}

void pragma::gui::types::WIMainMenu::SetNewGameMenu()
{
	if(m_menuType == 0 || !m_hMain.IsValid())
		return;
	m_menuType = 0;
	WIMainMenuBase *menu = static_cast<WIMainMenuBase *>(m_hMain.get());
	menu->RemoveMenuItem(1);
	menu->RemoveMenuItem(0);
}

void pragma::gui::types::WIMainMenu::SetSize(int x, int y)
{
	WIBase::SetSize(x, y);
	uint32_t logoYBottom = 50;
}
