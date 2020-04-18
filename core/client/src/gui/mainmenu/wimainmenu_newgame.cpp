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

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;


WIMainMenuNewGame::WIMainMenuNewGame()
	: WIMainMenuBase()
{}

WIMainMenuNewGame::~WIMainMenuNewGame()
{
	if(m_cbMapListReload.IsValid())
		m_cbMapListReload.Remove();
}

void WIMainMenuNewGame::OnStartGame(GLFW::MouseButton button,GLFW::KeyState state,GLFW::Modifier)
{
	if(button != GLFW::MouseButton::Left || state != GLFW::KeyState::Press)
		return;
	auto *pOptionsList = m_hControlSettings.get<WIOptionsList>();
	std::string map;
	std::string serverName;
	std::string gameMode;
	std::string rconPassword;
	UInt32 maxPlayers = 1;
	if(m_hMapList.IsValid())
	{
		auto *pMap = m_hMapList.get<WIDropDownMenu>();
		map = pMap->GetValue();
	}
	if(m_hServerName.IsValid())
	{
		auto *pServerName = m_hServerName.get<WITextEntry>();
		serverName = pServerName->GetText();
	}
	if(m_hGameMode.IsValid())
	{
		auto *pGameMode = m_hGameMode.get<WIDropDownMenu>();
		gameMode = pGameMode->GetText();
	}
	if(m_hRconPassword.IsValid())
	{
		auto *pRconPassword = m_hRconPassword.get<WITextEntry>();
		rconPassword = pRconPassword->GetText();
	}
	if(m_hMaxPlayers.IsValid())
	{
		auto *pSlider = m_hMaxPlayers.get<WISlider>();
		maxPlayers = CUInt32(pSlider->GetValue());
	}
	if(map.empty())
		return;
	c_engine->EndGame();
	pOptionsList->RunUpdateConVars(false);
	c_engine->StartDefaultGame(map,maxPlayers <= 1);
}

void WIMainMenuNewGame::Initialize()
{
	WIMainMenuBase::Initialize();
	AddMenuItem(Locale::GetText("back"),FunctionCallback<void,WIMainMenuElement*>::Create([this](WIMainMenuElement*) {
		auto *mainMenu = dynamic_cast<WIMainMenu*>(GetParent());
		if(mainMenu == nullptr)
			return;
		mainMenu->OpenMainMenu();
	}));

	InitializeGameSettings();
}

void WIMainMenuNewGame::InitializeOptionsList(WIOptionsList *pList)
{
	auto *pRow = pList->AddRow();
	pRow->SetValue(0,"");
	
	auto *buttonStart = WGUI::GetInstance().Create<WIButton>();
	buttonStart->SetText(Locale::GetText("start_game"));
	buttonStart->SizeToContents();
	buttonStart->SetAutoCenterToParent(true);
	buttonStart->AddCallback("OnMouseEvent",FunctionCallback<util::EventReply,GLFW::MouseButton,GLFW::KeyState,GLFW::Modifier>::CreateWithOptionalReturn(
		[this](util::EventReply *reply,GLFW::MouseButton button,GLFW::KeyState state,GLFW::Modifier mods) -> CallbackReturnType {
		OnStartGame(button,state,mods);
		*reply = util::EventReply::Handled;
		return CallbackReturnType::HasReturnValue;
	}));
	pRow->InsertElement(1,buttonStart);
	WIMainMenuBase::InitializeOptionsList(pList);
}

void WIMainMenuNewGame::ReloadMapList()
{
	if(m_hMapList.IsValid() == false)
		return;
	auto *pMap = static_cast<WIDropDownMenu*>(m_hMapList.get());
	pMap->ClearOptions();
	std::vector<std::string> files;
	FileManager::FindFiles("maps/*.wld",&files,nullptr);
	std::sort(files.begin(),files.end());
	for(unsigned int i=0;i<files.size();i++)
	{
		auto fName = files[i].substr(0,files[i].length() -4);
		auto displayName = fName;
		auto f = FileManager::OpenFile((std::string("maps/") +fName +".txt").c_str(),"r");
		if(f != nullptr)
		{
			auto root = ds::System::ReadData(f);
			if(root != nullptr)
			{
				auto block = root->GetBlock(fName.c_str(),0);
				if(block != nullptr)
				{
					auto name = block->GetString("name");
					auto author = block->GetString("author");
					std::stringstream newName;
					if(!name.empty())
						newName<<name;
					else
						newName<<fName;
					if(!author.empty())
						newName<<" ("<<author<<")";
					displayName = newName.str();
				}
			}
		}
		pMap->AddOption(displayName,fName);
	}
	pMap->SizeToContents();
	//
}

#include <wgui/types/witext.h>
void WIMainMenuNewGame::InitializeGameSettings()
{
	auto *pList = InitializeOptionsList();
	auto title = Locale::GetText("game_settings");
	ustring::to_upper(title);
	pList->SetTitle(title);

	// Map
	auto *pMap = pList->AddDropDownMenu(Locale::GetText("map"));
	pMap->SetEditable(true);
	pMap->SetName("map");
	m_hMapList = pMap->GetHandle();
	ReloadMapList();
	auto &resourceWatcher = client->GetResourceWatcher();
	if(m_cbMapListReload.IsValid())
		m_cbMapListReload.Remove();
	m_cbMapListReload = resourceWatcher.AddChangeCallback(EResourceWatcherCallbackType::Map,[this](std::reference_wrapper<const std::string> fileName,std::reference_wrapper<const std::string> ext) {
		ReloadMapList();
	});

	// Game Mode
	auto &gameModes = GameModeManager::GetGameModes();
	std::unordered_map<std::string,std::string> gameModeOptions;
	for(auto it=gameModes.begin();it!=gameModes.end();++it)
	{
		auto &info = it->second;
		gameModeOptions[info.name] = it->first;
	}
	auto *pGameMode = pList->AddDropDownMenu(Locale::GetText("gamemode"),gameModeOptions,"sv_gamemode");
	m_hGameMode = pGameMode->GetHandle();
	//
	// Server Name
	auto *pServerName = pList->AddTextEntry(Locale::GetText("server_name"),"sv_servername");
	m_hServerName = pServerName->GetHandle();
	//
	// RCON Password
	auto *pPassword = pList->AddTextEntry(Locale::GetText("server_password"),"sv_password");
	pPassword->SetInputHidden(true);
	m_hRconPassword = pPassword->GetHandle();
	//
	// Player Count
	auto *pMaxPlayers = pList->AddSlider(Locale::GetText("max_players"),[](WISlider *pSlider) {
		pSlider->SetRange(1.f,50.f,1.f);
	},"sv_maxplayers");
	m_hMaxPlayers = pMaxPlayers;
	//
	InitializeOptionsList(pList);
}

