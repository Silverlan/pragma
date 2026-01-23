// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :gui.main_menu_new_game;
import :gui.options_list;
import :gui.slider;

import :client_state;
import :engine;
import pragma.gui;
import pragma.string.unicode;

pragma::gui::types::WIMainMenuNewGame::WIMainMenuNewGame() : WIMainMenuBase() {}

pragma::gui::types::WIMainMenuNewGame::~WIMainMenuNewGame()
{
	if(m_cbMapListReload.IsValid())
		m_cbMapListReload.Remove();
}

void pragma::gui::types::WIMainMenuNewGame::OnStartGame(platform::MouseButton button, platform::KeyState state, platform::Modifier)
{
	if(button != platform::MouseButton::Left || state != platform::KeyState::Press)
		return;
	auto *pOptionsList = static_cast<WIOptionsList *>(m_hControlSettings.get());
	std::string map;
	string::Utf8String serverName;
	std::string gameMode;
	string::Utf8String rconPassword;
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
	get_cengine()->EndGame();
	pOptionsList->RunUpdateConVars(false);
	get_cengine()->StartDefaultGame(map, maxPlayers <= 1);
}

void pragma::gui::types::WIMainMenuNewGame::Initialize()
{
	WIMainMenuBase::Initialize();
	AddMenuItem(locale::get_text("back"), FunctionCallback<void, WIMainMenuElement *>::Create([this](WIMainMenuElement *) {
		auto *mainMenu = dynamic_cast<WIMainMenu *>(GetParent());
		if(mainMenu == nullptr)
			return;
		mainMenu->OpenMainMenu();
	}));

	InitializeGameSettings();
	EnableThinking();
}

void pragma::gui::types::WIMainMenuNewGame::Think(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd)
{
	WIMainMenuBase::Think(drawCmd);
	DisableThinking();

	// We'll delay loading the map list until actually needed,
	// since it requires the archive library to be fully initialized,
	// which can take some time.
	ReloadMapList();
}

void pragma::gui::types::WIMainMenuNewGame::InitializeOptionsList(WIOptionsList *pList)
{
	auto *pRow = pList->AddRow();
	pRow->SetValue(0, "");

	auto *buttonStart = WGUI::GetInstance().Create<WIButton>();
	buttonStart->SetText(locale::get_text("start_game"));
	buttonStart->SizeToContents();
	buttonStart->SetAutoCenterToParent(true);
	buttonStart->AddCallback("OnMouseEvent",
	  FunctionCallback<util::EventReply, platform::MouseButton, platform::KeyState, platform::Modifier>::CreateWithOptionalReturn([this](util::EventReply *reply, platform::MouseButton button, platform::KeyState state, platform::Modifier mods) -> CallbackReturnType {
		  OnStartGame(button, state, mods);
		  *reply = util::EventReply::Handled;
		  return CallbackReturnType::HasReturnValue;
	  }));
	pRow->InsertElement(1, buttonStart);
	WIMainMenuBase::InitializeOptionsList(pList);
}

void pragma::gui::types::WIMainMenuNewGame::ReloadMapList()
{
	if(m_hMapList.IsValid() == false)
		return;
	auto *pMap = static_cast<WIDropDownMenu *>(m_hMapList.get());
	pMap->ClearOptions();
	auto exts = pragma::asset::get_supported_extensions(asset::Type::Map, asset::FormatType::All);
	std::vector<std::string> files;
	for(auto &ext : exts)
		fs::find_files("maps/*." + ext, &files, nullptr);

	std::unordered_set<std::string> uniqueFiles;
	std::unordered_set<std::string> nativeFiles;
	uniqueFiles.reserve(files.size());
	for(auto &f : files) {
		ufile::remove_extension_from_filename(f, exts);
		nativeFiles.insert(f);
		uniqueFiles.insert(std::move(f));
	}

	if(get_cengine()->GetConVarBool("sh_mount_external_game_resources")) {
		auto dllHandle = pragma::util::initialize_external_archive_manager(get_client_state());
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
			if(string::compare(*it, *itNext, false) == false) {
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
			auto f = pragma::fs::open_file((std::string("maps/") + fName + ".txt").c_str(), fs::FileMode::Read);
			if(f != nullptr) {
				fs::File fp {f};
				auto root = datasystem::System::ReadData(fp);
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

void pragma::gui::types::WIMainMenuNewGame::InitializeGameSettings()
{
	auto *pList = InitializeOptionsList();
	auto title = locale::get_text("game_settings");
	string::to_upper(title);
	pList->SetTitle(title);

	// Game Mode
	auto &gameModes = game::GameModeManager::GetGameModes();
	std::unordered_map<std::string, std::string> gameModeOptions;
	for(auto it = gameModes.begin(); it != gameModes.end(); ++it) {
		auto &info = it->second;
		gameModeOptions[info.name] = it->first;
	}
	auto *pGameMode = pList->AddDropDownMenu(locale::get_text("gamemode"), gameModeOptions, "sv_gamemode");
	pGameMode->AddCallback("OnValueChanged", FunctionCallback<void>::Create([pGameMode, this]() {
		auto val = pGameMode->GetOptionValue(pGameMode->GetSelectedOption());
		auto &gameModes = game::GameModeManager::GetGameModes();
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
	auto *pMap = pList->AddDropDownMenu(locale::get_text("map"));
	// pMap->SetEditable(true);
	pMap->SetName("map");
	m_hMapList = pMap->GetHandle();

	auto &resourceWatcher = get_client_state()->GetResourceWatcher();
	if(m_cbMapListReload.IsValid())
		m_cbMapListReload.Remove();
#ifdef WINDOWS_CLANG_COMPILER_FIX
	m_cbMapListReload = resourceWatcher.AddChangeCallback(util::EResourceWatcherCallbackType::createFromEnum(util::EResourceWatcherCallbackType::E::Map), [this](std::reference_wrapper<const std::string> fileName, std::reference_wrapper<const std::string> ext) { ReloadMapList(); });
#else
	m_cbMapListReload = resourceWatcher.AddChangeCallback(util::eResourceWatcherCallbackType::Map, [this](std::reference_wrapper<const std::string> fileName, std::reference_wrapper<const std::string> ext) { ReloadMapList(); });
#endif

	// Server Name
	auto *pServerName = pList->AddTextEntry(locale::get_text("server_name"), "sv_servername");
	m_hServerName = pServerName->GetHandle();
	//
	// RCON Password
	auto *pPassword = pList->AddTextEntry(locale::get_text("server_password"), "sv_password");
	pPassword->SetInputHidden(true);
	m_hRconPassword = pPassword->GetHandle();
	//
	// Player Count
	auto *pMaxPlayers = pList->AddSlider(locale::get_text("max_players"), [](WISlider *pSlider) { pSlider->SetRange(1.f, 50.f, 1.f); }, "sv_maxplayers");
	m_hMaxPlayers = pMaxPlayers;
	//
	InitializeOptionsList(pList);
}
