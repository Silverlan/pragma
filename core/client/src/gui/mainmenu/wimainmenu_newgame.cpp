/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientutil.h"
#include "pragma/c_engine.h"
#include "pragma/gui/mainmenu/wimainmenu_newgame.h"
#include <wgui/types/widropdownmenu.h>
#include <wgui/types/wibutton.h>
#include <fsys/filesystem.h>
#include "pragma/gui/mainmenu/wimainmenu.h"
#include <wgui/types/witextentry.h>
#include "pragma/gui/wislider.h"
#include "pragma/gui/wioptionslist.h"
#include "pragma/localization.h"
#include "pragma/game/gamemode/gamemodemanager.h"
#include <pragma/util/resource_watcher.h>
#include <pragma/game/game_resources.hpp>
#include <pragma/asset/util_asset.hpp>
#include <sharedutils/util_library.hpp>
#include <sharedutils/util_file.h>
#include <wgui/types/witext.h>
#include <fsys/ifile.hpp>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;

WIMainMenuNewGame::WIMainMenuNewGame() : WIMainMenuBase() {}

WIMainMenuNewGame::~WIMainMenuNewGame()
{
	if(m_cbMapListReload.IsValid())
		m_cbMapListReload.Remove();
}

void WIMainMenuNewGame::OnStartGame(GLFW::MouseButton button, GLFW::KeyState state, GLFW::Modifier)
{
	if(button != GLFW::MouseButton::Left || state != GLFW::KeyState::Press)
		return;
	auto *pOptionsList = static_cast<WIOptionsList *>(m_hControlSettings.get());
	std::string map;
	pragma::string::Utf8String serverName;
	std::string gameMode;
	pragma::string::Utf8String rconPassword;
	UInt32 maxPlayers = 1;
	if(m_hMapList.IsValid()) {
		auto *pMap = static_cast<WIDropDownMenu *>(m_hMapList.get());
		map = pMap->GetValue();
	}
	if(m_hServerName.IsValid()) {
		auto *pServerName = static_cast<WITextEntry *>(m_hServerName.get());
		serverName = pServerName->GetText().to_str();
	}
	if(m_hGameMode.IsValid()) {
		auto *pGameMode = static_cast<WIDropDownMenu *>(m_hGameMode.get());
		gameMode = pGameMode->GetText().cpp_str();
	}
	if(m_hRconPassword.IsValid()) {
		auto *pRconPassword = static_cast<WITextEntry *>(m_hRconPassword.get());
		rconPassword = pRconPassword->GetText().to_str();
	}
	if(m_hMaxPlayers.IsValid()) {
		auto *pSlider = static_cast<WISlider *>(m_hMaxPlayers.get());
		maxPlayers = CUInt32(pSlider->GetValue());
	}
	if(map.empty())
		return;
	c_engine->EndGame();
	pOptionsList->RunUpdateConVars(false);
	c_engine->StartDefaultGame(map, maxPlayers <= 1);
}

void WIMainMenuNewGame::Initialize()
{
	WIMainMenuBase::Initialize();
	AddMenuItem(Locale::GetText("back"), FunctionCallback<void, WIMainMenuElement *>::Create([this](WIMainMenuElement *) {
		auto *mainMenu = dynamic_cast<WIMainMenu *>(GetParent());
		if(mainMenu == nullptr)
			return;
		mainMenu->OpenMainMenu();
	}));

	InitializeGameSettings();
	EnableThinking();
}

void WIMainMenuNewGame::Think(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd)
{
	WIMainMenuBase::Think(drawCmd);
	DisableThinking();

	// We'll delay loading the map list until actually needed,
	// since it requires the archive library to be fully initialized,
	// which can take some time.
	ReloadMapList();
}

void WIMainMenuNewGame::InitializeOptionsList(WIOptionsList *pList)
{
	auto *pRow = pList->AddRow();
	pRow->SetValue(0, "");

	auto *buttonStart = WGUI::GetInstance().Create<WIButton>();
	buttonStart->SetText(Locale::GetText("start_game"));
	buttonStart->SizeToContents();
	buttonStart->SetAutoCenterToParent(true);
	buttonStart->AddCallback("OnMouseEvent", FunctionCallback<util::EventReply, GLFW::MouseButton, GLFW::KeyState, GLFW::Modifier>::CreateWithOptionalReturn([this](util::EventReply *reply, GLFW::MouseButton button, GLFW::KeyState state, GLFW::Modifier mods) -> CallbackReturnType {
		OnStartGame(button, state, mods);
		*reply = util::EventReply::Handled;
		return CallbackReturnType::HasReturnValue;
	}));
	pRow->InsertElement(1, buttonStart);
	WIMainMenuBase::InitializeOptionsList(pList);
}

void WIMainMenuNewGame::ReloadMapList()
{
	if(m_hMapList.IsValid() == false)
		return;
	auto *pMap = static_cast<WIDropDownMenu *>(m_hMapList.get());
	pMap->ClearOptions();
	auto exts = pragma::asset::get_supported_extensions(pragma::asset::Type::Map, pragma::asset::FormatType::All);
	std::vector<std::string> files;
	for(auto &ext : exts)
		filemanager::find_files("maps/*." + ext, &files, nullptr);

	std::unordered_set<std::string> uniqueFiles;
	std::unordered_set<std::string> nativeFiles;
	uniqueFiles.reserve(files.size());
	for(auto &f : files) {
		ufile::remove_extension_from_filename(f, exts);
		nativeFiles.insert(f);
		uniqueFiles.insert(std::move(f));
	}

	if(c_engine->GetConVarBool("sh_mount_external_game_resources")) {
		auto dllHandle = util::initialize_external_archive_manager(client);
		if(dllHandle) {
			auto *fFindFiles = dllHandle->FindSymbolAddress<void (*)(const std::string &, std::vector<std::string> *, std::vector<std::string> *)>("find_files");
			if(fFindFiles) {
				std::vector<std::string> extFiles {};
				std::vector<std::string> extDirs {};
				fFindFiles("maps/*.bsp", &extFiles, &extDirs);

				uniqueFiles.reserve(uniqueFiles.size() + extFiles.size());
				for(auto &extFile : extFiles) {
					ufile::remove_extension_from_filename(extFile, exts);
					uniqueFiles.insert(std::move(extFile));
				}
			}
		}
	}
	files.clear();
	files.reserve(uniqueFiles.size());
	for(auto &f : uniqueFiles)
		files.push_back(f);

	// Remove duplicates
	if(files.size() > 1) {
		for(auto it = files.begin(); it != files.end() - 1;) {
			auto itNext = it + 1;
			if(ustring::compare(*it, *itNext, false) == false) {
				++it;
				continue;
			}
			it = files.erase(it);
		}
	}

	std::vector<std::string> nativeMaps;
	nativeMaps.reserve(files.size());
	for(auto it = files.begin(); it != files.end();) {
		auto &f = *it;
		if(nativeFiles.find(f) != nativeFiles.end()) {
			nativeMaps.push_back(std::move(f));
			it = files.erase(it);
			continue;
		}
		++it;
	}

	std::sort(nativeMaps.begin(), nativeMaps.end());
	std::sort(files.begin(), files.end());

	auto fAddMaps = [pMap](const std::vector<std::string> &files, const std::optional<Color> &color = {}) {
		for(unsigned int i = 0; i < files.size(); i++) {
			auto &fName = files[i];
			auto displayName = fName;
			auto f = FileManager::OpenFile((std::string("maps/") + fName + ".txt").c_str(), "r");
			if(f != nullptr) {
				fsys::File fp {f};
				auto root = ds::System::ReadData(fp);
				if(root != nullptr) {
					auto block = root->GetBlock(fName.c_str(), 0);
					if(block != nullptr) {
						auto name = block->GetString("name");
						auto author = block->GetString("author");
						std::stringstream newName;
						if(!name.empty())
							newName << name;
						else
							newName << fName;
						if(!author.empty())
							newName << " (" << author << ")";
						displayName = newName.str();
					}
				}
			}
			auto *opt = pMap->AddOption(displayName, fName);
			auto *el = opt ? opt->GetTextElement() : nullptr;
			if(el && color.has_value())
				el->SetColor(*color);
		}
	};
	// Show native maps (i.e. maps that don't have to be imported) first
	fAddMaps(nativeMaps);
	fAddMaps(files, Color {200, 0, 0, 255});
}

void WIMainMenuNewGame::InitializeGameSettings()
{
	auto *pList = InitializeOptionsList();
	auto title = Locale::GetText("game_settings");
	ustring::to_upper(title);
	pList->SetTitle(title);

	// Game Mode
	auto &gameModes = GameModeManager::GetGameModes();
	std::unordered_map<std::string, std::string> gameModeOptions;
	for(auto it = gameModes.begin(); it != gameModes.end(); ++it) {
		auto &info = it->second;
		gameModeOptions[info.name] = it->first;
	}
	auto *pGameMode = pList->AddDropDownMenu(Locale::GetText("gamemode"), gameModeOptions, "sv_gamemode");
	pGameMode->AddCallback("OnValueChanged", FunctionCallback<void>::Create([pGameMode, this]() {
		auto val = pGameMode->GetOptionValue(pGameMode->GetSelectedOption());
		auto &gameModes = GameModeManager::GetGameModes();
		auto it = gameModes.find(val);
		if(it == gameModes.end())
			return;
		auto &gmInfo = it->second;
		if(!gmInfo.initial_map.empty()) {
			auto *dropDownMenu = static_cast<WIDropDownMenu *>(m_hMapList.get());
			if(dropDownMenu->GetSelectedOption() == -1)
				dropDownMenu->SelectOption(gmInfo.initial_map);
		}
	}));
	m_hGameMode = pGameMode->GetHandle();
	//

	// Map
	auto *pMap = pList->AddDropDownMenu(Locale::GetText("map"));
	// pMap->SetEditable(true);
	pMap->SetName("map");
	m_hMapList = pMap->GetHandle();

	auto &resourceWatcher = client->GetResourceWatcher();
	if(m_cbMapListReload.IsValid())
		m_cbMapListReload.Remove();
	m_cbMapListReload = resourceWatcher.AddChangeCallback(EResourceWatcherCallbackType::Map, [this](std::reference_wrapper<const std::string> fileName, std::reference_wrapper<const std::string> ext) { ReloadMapList(); });

	// Server Name
	auto *pServerName = pList->AddTextEntry(Locale::GetText("server_name"), "sv_servername");
	m_hServerName = pServerName->GetHandle();
	//
	// RCON Password
	auto *pPassword = pList->AddTextEntry(Locale::GetText("server_password"), "sv_password");
	pPassword->SetInputHidden(true);
	m_hRconPassword = pPassword->GetHandle();
	//
	// Player Count
	auto *pMaxPlayers = pList->AddSlider(
	  Locale::GetText("max_players"), [](WISlider *pSlider) { pSlider->SetRange(1.f, 50.f, 1.f); }, "sv_maxplayers");
	m_hMaxPlayers = pMaxPlayers;
	//
	InitializeOptionsList(pList);
}
