/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include <pragma/engine.h>
#include "pragma/game/gamemode/gamemodemanager.h"
#include "pragma/game/gamemode/gamemode.h"
#include <pragma/game/game.h>
#include "luasystem.h"
#include "pragma/game/damageinfo.h"
#include "pragma/lua/luafunction_call.h"
#include "pragma/entities/components/base_player_component.hpp"
#include "pragma/entities/components/base_ai_component.hpp"

extern DLLENGINE Engine *engine;

void Lua::GameMode::GetName(lua_State *l,::GameMode &gm)
{
	Lua::PushString(l,gm.GetName());
}
void Lua::GameMode::GetIdentifier(lua_State *l,::GameMode &gm)
{
	Lua::PushString(l,gm.GetIdentifier());
}
void Lua::GameMode::GetClassName(lua_State *l,::GameMode &gm)
{
	Lua::PushString(l,gm.GetClassName());
}
void Lua::GameMode::GetAuthor(lua_State *l,::GameMode &gm)
{
	Lua::PushString(l,gm.GetAuthor());
}
void Lua::GameMode::GetVersion(lua_State *l,::GameMode &gm)
{
	auto version = gm.GetVersion();
	Lua::PushInt(l,version.major);
	Lua::PushInt(l,version.minor);
	Lua::PushInt(l,version.revision);
}

//////////////////////////////

GameMode::GameMode()
	: m_game(nullptr)
{}

GameMode::~GameMode()
{
	for(auto it=m_callbacks.begin();it!=m_callbacks.end();++it)
	{
		auto &hCallback = *it;
		if(hCallback.IsValid())
			hCallback.Remove();
	}
}

static const std::string empty_string;
const std::string &GameMode::GetName() const
{
	auto *info = m_game->GetGameMode();
	if(info == nullptr)
		return empty_string;
	return info->name;
}
const std::string &GameMode::GetIdentifier() const
{
	auto *info = m_game->GetGameMode();
	if(info == nullptr)
		return empty_string;
	return info->id;
}
const std::string &GameMode::GetClassName() const
{
	auto *info = m_game->GetGameMode();
	if(info == nullptr)
		return empty_string;
	return info->class_name;
}
const std::string &GameMode::GetAuthor() const
{
	auto *info = m_game->GetGameMode();
	if(info == nullptr)
		return empty_string;
	return info->author;
}
util::Version GameMode::GetVersion() const
{
	auto *info = m_game->GetGameMode();
	if(info == nullptr)
		return {};
	return info->version;
}

bool GameMode::IsInitialized() const {return m_bInitialized;}

void GameMode::Initialize(Game *game,luabind::object&)
{
	m_bInitialized = true;
	m_game = game;
	m_callbacks.push_back(game->AddCallback("Think",FunctionCallback<>::Create([this]() {
		Think();
	})));
	m_callbacks.push_back(game->AddCallback("Tick",FunctionCallback<>::Create([this]() {
		Tick();
	})));
	m_callbacks.push_back(game->AddCallback("OnEntityTakeDamage",FunctionCallback<void,BaseEntity*,std::reference_wrapper<DamageInfo>>::Create([this](BaseEntity *ent,std::reference_wrapper<DamageInfo> dmg) {
		OnEntityTakeDamage(ent->GetHandle(),dmg.get());
	})));
	m_callbacks.push_back(game->AddCallback("OnEntityTakenDamage",FunctionCallback<void,BaseEntity*,std::reference_wrapper<DamageInfo>,unsigned short,unsigned short>::Create([this](BaseEntity *ent,std::reference_wrapper<DamageInfo> dmg,unsigned short oldHealth,unsigned short newHealth) {
		OnEntityTakenDamage(ent->GetHandle(),dmg.get(),oldHealth,newHealth);
	})));
	m_callbacks.push_back(game->AddCallback("OnEntityHealthChanged",FunctionCallback<void,BaseEntity*,unsigned short,unsigned short>::Create([this](BaseEntity *ent,unsigned short oldHealth,unsigned short newHealth) {
		OnEntityHealthChanged(ent->GetHandle(),oldHealth,newHealth);
	})));
	m_callbacks.push_back(game->AddCallback("OnPlayerDeath",FunctionCallback<void,pragma::BasePlayerComponent*,DamageInfo*>::Create([this](pragma::BasePlayerComponent *victim,DamageInfo *dmgInfo) {
		auto hVictim = (victim != nullptr) ? victim->GetLuaObject() : luabind::object(m_game->GetLuaState(),luabind::nil);
		OnPlayerDeath(hVictim,dmgInfo);
	})));
	m_callbacks.push_back(game->AddCallback("OnPlayerSpawned",FunctionCallback<void,pragma::BasePlayerComponent*>::Create([this](pragma::BasePlayerComponent *pl) {
		OnPlayerSpawned(pl->GetLuaObject());
	})));
	m_callbacks.push_back(game->AddCallback("OnNPCDeath",FunctionCallback<void,pragma::BaseAIComponent*,DamageInfo*>::Create([this](pragma::BaseAIComponent *victim,DamageInfo *dmgInfo) {
		auto hVictim = (victim != nullptr) ? victim->GetLuaObject() : luabind::object(m_game->GetLuaState(),luabind::nil);
		OnNPCDeath(hVictim,dmgInfo);
	})));
	m_callbacks.push_back(game->AddCallback("OnActionInput",FunctionCallback<bool,pragma::BasePlayerComponent*,Action,bool>::CreateWithOptionalReturn([this](bool *r,pragma::BasePlayerComponent *pl,Action action,bool bPressed) {
		*r = OnActionInput(pl->GetLuaObject(),umath::to_integral(action),bPressed);
		return !(*r) ? CallbackReturnType::HasReturnValue : CallbackReturnType::NoReturnValue;
	})));
	m_callbacks.push_back(game->AddCallback("OnPlayerReady",FunctionCallback<void,pragma::BasePlayerComponent*>::Create([this](pragma::BasePlayerComponent *pl) {
		OnPlayerReady(pl->GetLuaObject());
	})));
	m_callbacks.push_back(game->AddCallback("OnPlayerDropped",FunctionCallback<void,pragma::BasePlayerComponent*,pragma::networking::DropReason>::Create([this](pragma::BasePlayerComponent *pl,pragma::networking::DropReason reason) {
		OnPlayerDropped(pl->GetLuaObject(),std::underlying_type_t<decltype(reason)>(reason));
	})));
	m_callbacks.push_back(game->AddCallback("OnPlayerJoined",FunctionCallback<void,pragma::BasePlayerComponent*>::Create([this](pragma::BasePlayerComponent *pl) {
		OnPlayerJoined(pl->GetLuaObject());
	})));
	m_callbacks.push_back(game->AddCallback("OnGameInitialized",FunctionCallback<void,Game*>::Create([this](Game*) {
		OnGameInitialized();
	})));
	m_callbacks.push_back(game->AddCallback("OnMapLoaded",FunctionCallback<void>::Create([this]() {
		OnMapInitialized();
	})));
	m_callbacks.push_back(game->AddCallback("OnGameReady",FunctionCallback<void>::Create([this]() {
		OnGameReady();
	})));
	game->CallLuaCallbacks<void,GameMode*>("OnGameModeInitialized",this);
}

void GameMode::Think() {}
void GameMode::Tick() {}
void GameMode::OnEntityTakeDamage(EntityHandle,DamageInfo&) {}
void GameMode::OnEntityTakenDamage(EntityHandle,DamageInfo&,unsigned short,unsigned short) {}
void GameMode::OnEntityHealthChanged(EntityHandle,unsigned short,unsigned short) {}
void GameMode::OnPlayerDeath(luabind::object hVictim,DamageInfo*) {}
void GameMode::OnPlayerSpawned(luabind::object) {}
void GameMode::OnNPCDeath(luabind::object,DamageInfo*) {}
bool GameMode::OnActionInput(luabind::object,uint32_t,bool) {return true;}
void GameMode::OnPlayerDropped(luabind::object hPlayer,int32_t) {}
void GameMode::OnPlayerReady(luabind::object hPlayer) {}
void GameMode::OnPlayerJoined(luabind::object hPlayer) {}
void GameMode::OnGameInitialized() {}
void GameMode::OnMapInitialized() {}
void GameMode::OnGameReady() {}

std::string Game::GetGameModeScriptDirectoryPath() const
{
	auto *info = GetGameMode();
	if(info == nullptr)
		return "";
	auto &id = info->id;
	return "gamemodes\\" +id;
}
std::string Game::GetGameModeScriptDirectoryNetworkPath() const
{
	auto path = GetGameModeScriptDirectoryPath();
	if(path.empty())
		return "";
	return path +'\\' +GetLuaNetworkDirectoryName();
}
std::string Game::GetGameModeScriptFilePath() const
{
	auto path = GetGameModeScriptDirectoryNetworkPath();
	if(path.empty())
		return "";
	return path +'\\' +GetLuaNetworkFileName();
}

void Game::ReloadGameModeScripts()
{
	auto fileName = GetGameModeScriptFilePath();
	if(fileName.empty())
		return;
	ExecuteLuaFile(fileName);
}

bool Game::InitializeGameMode()
{
	auto *info = GetGameMode();
	if(info == nullptr)
		return false;
	ReloadGameModeScripts();
	auto gameMode = info->class_name;

	std::vector<std::string> libs;
	ustring::explode(gameMode,".",libs);
	if(libs.empty())
		return false;
	auto o = luabind::globals(GetLuaState());
	auto idx = 1u;
	o = o[libs.front()];
	while(o && idx < libs.size())
		o = o[libs.at(idx++)];
	if(!o)
		return false;

#ifndef LUABIND_NO_EXCEPTIONS
	try
	{
#endif
		m_luaGameMode = std::make_unique<luabind::object>(o());
#ifndef LUABIND_NO_EXCEPTIONS
	}
	catch(luabind::error&)
	{
		Lua::HandleLuaError(GetLuaState());
		return false;
	}
#endif
	auto gm = luabind::object_cast<GameMode*>(*m_luaGameMode.get());
	if(gm != nullptr)
		gm->Initialize(this,*m_luaGameMode.get());
	return true;
}

bool Game::IsGameModeInitialized() const
{
	if(m_luaGameMode == nullptr)
		return false;
	auto gm = luabind::object_cast<GameMode*>(*m_luaGameMode.get());
	if(gm == nullptr)
		return false;
	return gm->IsInitialized();
}

///////////////////////////////////////////

GameModeWrapper::GameModeWrapper()
	: GameMode(),LuaObjectBase(luabind::object())
{}
GameModeWrapper::~GameModeWrapper()
{}
void GameModeWrapper::Initialize(Game *game,luabind::object &o)
{
	m_baseLuaObj = std::shared_ptr<luabind::object>(new luabind::object(o));
	GameMode::Initialize(game,o);
}

void GameModeWrapper::Think()
{
	CallLuaMember<void>("Think");
}
void GameModeWrapper::LThink() {}
void GameModeWrapper::default_Think(GameMode *ptr)
{
	return ptr->GameMode::Think();
}

void GameModeWrapper::Tick()
{
	CallLuaMember<void>("Tick");
}
void GameModeWrapper::LTick() {}
void GameModeWrapper::default_Tick(GameMode *ptr)
{
	return ptr->GameMode::Tick();
}

void GameModeWrapper::OnEntityTakeDamage(EntityHandle hEnt,DamageInfo &dmg)
{
	CallLuaMember<void,luabind::object,std::reference_wrapper<DamageInfo>>("OnEntityTakeDamage",*hEnt->GetLuaObject(),std::ref(dmg));
}
void GameModeWrapper::LOnEntityTakeDamage(EntityHandle,DamageInfo&) {}
void GameModeWrapper::default_OnEntityTakeDamage(GameMode *ptr,EntityHandle hEnt,DamageInfo &dmg)
{
	return ptr->GameMode::OnEntityTakeDamage(hEnt,dmg);
}

void GameModeWrapper::OnEntityTakenDamage(EntityHandle hEnt,DamageInfo &dmg,unsigned short oldHealth,unsigned short newHealth)
{
	CallLuaMember<void,luabind::object,std::reference_wrapper<DamageInfo>,unsigned short,unsigned short>("OnEntityTakenDamage",*hEnt->GetLuaObject(),std::ref(dmg),oldHealth,newHealth);
}
void GameModeWrapper::LOnEntityTakenDamage(EntityHandle,DamageInfo&,unsigned short,unsigned short) {}
void GameModeWrapper::default_OnEntityTakenDamage(GameMode *ptr,EntityHandle hEnt,DamageInfo &dmg,unsigned short oldHealth,unsigned short newHealth)
{
	return ptr->GameMode::OnEntityTakenDamage(hEnt,dmg,oldHealth,newHealth);
}

void GameModeWrapper::OnEntityHealthChanged(EntityHandle hEnt,unsigned short oldHealth,unsigned short newHealth)
{
	CallLuaMember<void,luabind::object,unsigned short,unsigned short>("OnEntityHealthChanged",*hEnt->GetLuaObject(),oldHealth,newHealth);
}
void GameModeWrapper::LOnEntityHealthChanged(EntityHandle hEnt,unsigned short,unsigned short) {}
void GameModeWrapper::default_OnEntityHealthChanged(GameMode *ptr,EntityHandle hEnt,unsigned short oldHealth,unsigned short newHealth)
{
	return ptr->GameMode::OnEntityHealthChanged(hEnt,oldHealth,newHealth);
}

void GameModeWrapper::OnPlayerDeath(luabind::object hVictim,DamageInfo *dmgInfo)
{
	CallLuaMember<void,luabind::object,DamageInfo*>("OnPlayerDeath",hVictim,dmgInfo);
}
void GameModeWrapper::LOnPlayerDeath(luabind::object hVictim,DamageInfo *dmgInfo) {}
void GameModeWrapper::default_OnPlayerDeath(GameMode *ptr,luabind::object hVictim,DamageInfo *dmgInfo)
{
	return ptr->GameMode::OnPlayerDeath(hVictim,dmgInfo);
}

void GameModeWrapper::OnPlayerSpawned(luabind::object hPlayer)
{
	CallLuaMember<void,luabind::object>("OnPlayerSpawned",hPlayer);
}
void GameModeWrapper::LOnPlayerSpawned(luabind::object hPlayer) {}
void GameModeWrapper::default_OnPlayerSpawned(GameMode *ptr,luabind::object hPlayer)
{
	return ptr->GameMode::OnPlayerSpawned(hPlayer);
}

void GameModeWrapper::OnNPCDeath(luabind::object hVictim,DamageInfo *dmgInfo)
{
	CallLuaMember<void,luabind::object,DamageInfo*>("OnNPCDeath",hVictim,dmgInfo);
}
void GameModeWrapper::LOnNPCDeath(luabind::object hVictim,DamageInfo *dmgInfo) {}
void GameModeWrapper::default_OnNPCDeath(GameMode *ptr,luabind::object hVictim,DamageInfo *dmgInfo)
{
	return ptr->GameMode::OnPlayerDeath(hVictim,dmgInfo);
}

bool GameModeWrapper::OnActionInput(luabind::object hPlayer,uint32_t action,bool bPressed)
{
	auto r = true;
	CallLuaMember<bool,luabind::object,uint32_t,bool>("OnActionInput",&r,hPlayer,action,bPressed);
	return r;
}
bool GameModeWrapper::LOnActionInput(luabind::object hPlayer,uint32_t,bool) {return true;}
bool GameModeWrapper::default_OnActionInput(GameMode *ptr,luabind::object hPlayer,uint32_t action,bool bPressed)
{
	return ptr->GameMode::OnActionInput(hPlayer,action,bPressed);
}

void GameModeWrapper::OnPlayerDropped(luabind::object hPlayer,int32_t reason)
{
	CallLuaMember<void,luabind::object,int32_t>("OnPlayerDropped",hPlayer,reason);
}
void GameModeWrapper::LOnPlayerDropped(luabind::object hPlayer,int32_t) {}
void GameModeWrapper::default_OnPlayerDropped(GameMode *ptr,luabind::object hPlayer,int32_t reason)
{
	ptr->GameMode::OnPlayerDropped(hPlayer,reason);
}

void GameModeWrapper::OnPlayerReady(luabind::object hPlayer)
{
	CallLuaMember<void,luabind::object>("OnPlayerReady",hPlayer);
}
void GameModeWrapper::LOnPlayerReady(luabind::object hPlayer) {}
void GameModeWrapper::default_OnPlayerReady(GameMode *ptr,luabind::object hPlayer)
{
	Con::cout<<"Pre OnPlayerReady"<<Con::endl;
	ptr->GameMode::OnPlayerReady(hPlayer);
	Con::cout<<"Post OnPlayerReady"<<Con::endl;
}

void GameModeWrapper::OnPlayerJoined(luabind::object hPlayer)
{
	CallLuaMember<void,luabind::object>("OnPlayerJoined",hPlayer);
}
void GameModeWrapper::LOnPlayerJoined(luabind::object hPlayer) {}
void GameModeWrapper::default_OnPlayerJoined(GameMode *ptr,luabind::object hPlayer)
{
	ptr->GameMode::OnPlayerJoined(hPlayer);
}

void GameModeWrapper::OnGameInitialized()
{
	CallLuaMember<void>("OnGameInitialized");
}
void GameModeWrapper::LOnGameInitialized() {}
void GameModeWrapper::default_OnGameInitialized(GameMode *ptr)
{
	ptr->GameMode::OnGameInitialized();
}

void GameModeWrapper::OnMapInitialized()
{
	CallLuaMember<void>("OnMapInitialized");
}
void GameModeWrapper::LOnMapInitialized() {}
void GameModeWrapper::default_OnMapInitialized(GameMode *ptr)
{
	ptr->GameMode::OnMapInitialized();
}

void GameModeWrapper::OnGameReady()
{
	CallLuaMember<void>("OnGameReady");
}
void GameModeWrapper::LOnGameReady() {}
void GameModeWrapper::default_OnGameReady(GameMode *ptr)
{
	ptr->GameMode::OnGameReady();
}
