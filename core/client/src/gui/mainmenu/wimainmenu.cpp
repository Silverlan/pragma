/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/gui/mainmenu/wimainmenu.h"
#include "pragma/gui/mainmenu/wimainmenu_newgame.h"
#include "pragma/gui/mainmenu/wimainmenu_loadgame.h"
#include "pragma/gui/mainmenu/wimainmenu_options.h"
#include "pragma/gui/mainmenu/wimainmenu_mods.hpp"
#include "pragma/gui/mainmenu/wimainmenu_credits.hpp"
#include "pragma/gui/wiloadscreen.h"
#include <wgui/types/witext.h>
#include "pragma/gui/wiconsole.hpp"
#include "pragma/gui/wiimageslideshow.h"
#include "pragma/gui/wiserverbrowser.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"
#include "pragma/localization.h"
#include <pragma/engine_version.h>
#include <pragma/audio/alsound_type.h>
#if WIMENU_ENABLE_PATREON_LOGO != 0
#include <shellapi.h>
#endif
#include <pragma/engine_info.hpp>

#define DLLSPEC_ISTEAMWORKS DLLNETWORK
#include <pragma/game/isteamworks.hpp>

extern DLLCLIENT CEngine *c_engine;
extern ClientState *client;
extern CGame *c_game;

WIMainMenu::WIMainMenu() : WIBase(), m_menuType(0), m_tOpen(0.0)
{
	SetKeyboardInputEnabled(true);
	SetZPos(1000);
}

WIMainMenu::~WIMainMenu()
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

util::EventReply WIMainMenu::KeyboardCallback(GLFW::Key key, int scanCode, GLFW::KeyState state, GLFW::Modifier mods)
{
	if(!m_hActive.IsValid())
		return util::EventReply::Handled;
	return m_hActive->KeyboardCallback(key, scanCode, state, mods);
}

void WIMainMenu::OnVisibilityChanged(bool bVisible)
{
	WIBase::OnVisibilityChanged(bVisible);
	if(c_game == NULL)
		return;
	if(bVisible == true) {
		double tCur = c_game->RealTime();
		m_tOpen = tCur;
		// Obsolete?
		/*m_cbBlur = c_game->AddCallback("RenderPostProcessing",FunctionCallback<void,unsigned int,unsigned int>::Create([this](unsigned int ppFBO,unsigned int) {
			double t = c_game->RealTime();
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

void WIMainMenu::PlayNextMenuTrack(bool newRound)
{
	if(m_menuSound != nullptr) {
		m_menuSound->Stop();
		m_menuSound = nullptr;
	}
	if(m_cbMenuTrack.IsValid())
		m_cbMenuTrack.Remove();
	if(m_menuTracks.empty()) {
		FileManager::FindFiles("sounds/ui/gamestartup*.*", &m_menuTracks, nullptr);
		newRound = true;
	}
	if(m_menuTracks.empty())
		return;
	auto next = umath::random(0, CUInt32(m_menuTracks.size() - 1));
	auto it = m_menuTracks.begin() + next;
	auto sound = *it;
	m_menuTracks.erase(it);
	if(client->PrecacheSound(std::string("ui/") + sound) == false || (m_menuSound = client->PlaySound(std::string("ui/") + sound, ALSoundType::GUI, ALCreateFlags::None)) == nullptr) {
		if(newRound == false)
			PlayNextMenuTrack(newRound);
	}
	else {
		m_menuSound->SetType(ALSoundType::Music | ALSoundType::GUI);
		// m_menuSound->SetPitch(0.4f);
		// m_menuSound->SetGain(0.2f);
		m_cbMenuTrack = FunctionCallback<void, ALState, ALState>::Create([this](ALState, ALState newstate) {
			if(newstate != ALState::Playing)
				this->PlayNextMenuTrack();
		});
		m_menuSound->AddCallback("OnStateChanged", m_cbMenuTrack);
	}
}

//#include "pragma/util/curl_query_handler.hpp"
void WIMainMenu::Initialize()
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
	FileManager::FindFiles("screenshots/*.tga", &imgFiles, nullptr);
	FileManager::FindFiles("screenshots/*.png", &imgFiles, nullptr);
	for(auto it = imgFiles.begin(); it != imgFiles.end(); it++)
		*it = "screenshots/" + *it;
	pImageSlideShow->SetImages(imgFiles);

	//std::shared_ptr<ALSound> PlaySound(std::string snd,int mode=AL_CHANNEL_AUTO,unsigned char priority=0);
	m_hMain = CreateChild<WIMainMenuBase>();
	WIMainMenuBase *menu = static_cast<WIMainMenuBase *>(m_hMain.get());
	menu->SetVisible(false);
	menu->SetSize(GetWidth(), GetHeight());
	menu->SetAnchor(0.f, 0.f, 1.f, 1.f);
	menu->AddMenuItem(Locale::GetText("menu_newgame"), FunctionCallback<>::Create([this]() { SetActiveMenu(m_hNewGame); }));
	menu->AddMenuItem(Locale::GetText("menu_find_servers"), FunctionCallback<>::Create([this]() {
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
	menu->AddMenuItem(Locale::GetText("menu_loadgame"), FunctionCallback<>::Create([this]() { SetActiveMenu(m_hLoad); }));
#endif
	menu->AddMenuItem(Locale::GetText("menu_options"), FunctionCallback<>::Create([this]() { SetActiveMenu(m_hOptions); }));
#if WIMENU_ENABLE_CREDITS_MENU != 0
	menu->AddMenuItem(Locale::GetText("menu_credits"), FunctionCallback<>::Create([this]() { SetActiveMenu(m_hCredits); }));
#endif
	/*menu->AddMenuItem(Locale::GetText("menu_addons"), FunctionCallback<>::Create([this]() {
		//SetActiveMenu(m_hMods);
		//ShellExecute(0,0,engine_info::get_modding_hub_url().c_str(),0,0,SW_SHOW);
		util::open_url_in_browser("steam://url/SteamWorkshopPage/" + std::to_string(engine_info::get_steam_app_id()));
	}));*/
#ifdef _DEBUG
	menu->AddMenuItem("Loadscreen", FunctionCallback<>::Create([this]() { SetActiveMenu(m_hLoadScreen); }));
#endif
	menu->AddMenuItem(Locale::GetText("menu_quit"), FunctionCallback<>::Create([]() { c_engine->ShutDown(); }));
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

#if WIMENU_ENABLE_CREDITS_MENU != 0
	m_hCredits = CreateChild<WIMainMenuCredits>();
	auto *pCredits = m_hCredits.get<WIMainMenuCredits>();
	pCredits->SetVisible(false);
	pCredits->SetSize(GetWidth(), GetHeight());
	pCredits->SetAnchor(0.f, 0.f, 1.f, 1.f);
	pCredits->SetKeyboardInputEnabled(true);
#endif

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
		version += " [" + ustring::substr(gitInfo->commitSha, 0, 7) + "]";
	auto *pVersion = static_cast<WIText *>(m_hVersion.get());
	pVersion->AddStyleClass("game_version");
	pVersion->SetColor(1.f, 1.f, 1.f, 1.f);
	pVersion->SetText(version);
	pVersion->SizeToContents();
	pVersion->SetName("engine_version");
	pVersion->SetPos(GetWidth() - pVersion->GetWidth() - 40, GetHeight() - pVersion->GetHeight() - 20);
	pVersion->SetAnchor(1.f, 1.f, 1.f, 1.f);

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
		pAttributes->SetColor(Color::Red);
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
	pAttributes->SetColor(Color::Lime);
	pAttributes->SetText("[" +c_engine->GetRenderContext().GetAPIAbbreviation() +"]");
	pAttributes->SizeToContents();*/

#if WIMENU_ENABLE_PATREON_LOGO != 0
	m_hPatreonIcon = CreateChild<WITexturedRect>();
	auto *pIcon = m_hPatreonIcon.get<WITexturedRect>();
	pIcon->SetMaterial("wgui/patreon_logo");
	pIcon->SetSize(64, 64);
	pIcon->SetMouseInputEnabled(true);
	pIcon->SetCursor(GLFW::Cursor::Shape::Hand);
	pIcon->AddCallback("OnMousePressed", FunctionCallback<util::EventReply>::CreateWithOptionalReturn([](util::EventReply *reply) -> CallbackReturnType {
		util::open_url_in_browser(engine_info::get_patreon_url());
		*reply = util::EventReply::Handled;
		return CallbackReturnType::HasReturnValue;
	}));
#endif

#if WIMENU_ENABLE_FMOD_LOGO != 0
	m_hFMODLogo = CreateChild<WITexturedRect>();
	auto *pIconFMod = m_hFMODLogo.get<WITexturedRect>();
	pIconFMod->SetMaterial("third_party/fmod_logo");
	pIconFMod->SetColor(Color::White);
	pIconFMod->SetSize(64, 32);
	pIconFMod->SetMouseInputEnabled(true);
#endif

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

void WIMainMenu::SetActiveMenu(WIHandle &hMenu)
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
void WIMainMenu::OpenMainMenu() { SetActiveMenu(m_hMain); }
void WIMainMenu::OpenNewGameMenu() { SetActiveMenu(m_hNewGame); }
void WIMainMenu::OpenLoadGameMenu() { SetActiveMenu(m_hLoad); }
void WIMainMenu::OpenOptionsMenu() { SetActiveMenu(m_hOptions); }
void WIMainMenu::OpenModsMenu() { SetActiveMenu(m_hMods); }
#if WIMENU_ENABLE_CREDITS_MENU != 0
void WIMainMenu::OpenCreditsMenu() { SetActiveMenu(m_hCredits); }
#endif
void WIMainMenu::OpenLoadScreen() { SetActiveMenu(m_hLoadScreen); }

void WIMainMenu::OnFocusGained()
{
	if(!m_hActive.IsValid())
		return;
	WIMainMenuBase *menu = static_cast<WIMainMenuBase *>(m_hActive.get());
	menu->RequestFocus();
}

void WIMainMenu::OnFocusKilled()
{
	if(!m_hActive.IsValid())
		return;
	WIMainMenuBase *menu = static_cast<WIMainMenuBase *>(m_hActive.get());
	menu->KillFocus(true);
}

void WIMainMenu::SetContinueMenu()
{
	if(m_menuType == 1 || !m_hMain.IsValid())
		return;
	m_menuType = 1;
	WIMainMenuBase *menu = static_cast<WIMainMenuBase *>(m_hMain.get());
	menu->AddMenuItem(0, Locale::GetText("menu_resumegame"), FunctionCallback<>::Create([]() { client->CloseMainMenu(); }));
	menu->AddMenuItem(1, Locale::GetText("menu_disconnect"), FunctionCallback<>::Create([]() { c_engine->EndGame(); }));
}

void WIMainMenu::SetNewGameMenu()
{
	if(m_menuType == 0 || !m_hMain.IsValid())
		return;
	m_menuType = 0;
	WIMainMenuBase *menu = static_cast<WIMainMenuBase *>(m_hMain.get());
	menu->RemoveMenuItem(1);
	menu->RemoveMenuItem(0);
}

void WIMainMenu::SetSize(int x, int y)
{
	WIBase::SetSize(x, y);
	uint32_t logoYBottom = 50;
#if WIMENU_ENABLE_PATREON_LOGO != 0
	if(m_hPatreonIcon.IsValid()) {
		auto *pIcon = m_hPatreonIcon.get();
		pIcon->SetPos(x - pIcon->GetWidth() - 20, y - pIcon->GetHeight() - 60);
		logoYBottom = y - pIcon->GetY();
	}
#endif
#if WIMENU_ENABLE_FMOD_LOGO != 0
	if(m_hFMODLogo.IsValid()) {
		auto *pIcon = m_hFMODLogo.get();
		pIcon->SetPos(x - pIcon->GetWidth() - 20, y - pIcon->GetHeight() - logoYBottom);
	}
#endif
}
