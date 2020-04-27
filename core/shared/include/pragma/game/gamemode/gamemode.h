/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __GAMEMODE_H__
#define __GAMEMODE_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
#include "pragma/lua/luaobjectbase.h"

#undef GetClassName

class GameMode;
namespace Lua
{
	namespace GameMode
	{
		DLLNETWORK void GetName(lua_State *l,::GameMode &gm);
		DLLNETWORK void GetIdentifier(lua_State *l,::GameMode &gm);
		DLLNETWORK void GetClassName(lua_State *l,::GameMode &gm);
		DLLNETWORK void GetAuthor(lua_State *l,::GameMode &gm);
		DLLNETWORK void GetVersion(lua_State *l,::GameMode &gm);
	};
};

class DLLNETWORK GameMode
{
private:
	std::vector<CallbackHandle> m_callbacks;
	Game *m_game;
	bool m_bInitialized = false;
public:
	GameMode();
	bool IsInitialized() const;
	virtual ~GameMode();
	virtual void Think();
	virtual void Tick();
	virtual void Initialize(Game *game,luabind::object &o);
	virtual void OnEntityTakeDamage(EntityHandle hEnt,DamageInfo &dmg);
	virtual void OnEntityTakenDamage(EntityHandle hEnt,DamageInfo &dmg,unsigned short oldHealth,unsigned short newHealth);
	virtual void OnEntityHealthChanged(EntityHandle hEnt,unsigned short oldHealth,unsigned short newHealth);
	virtual void OnPlayerDeath(luabind::object hVictim,DamageInfo *dmgInfo);
	virtual void OnPlayerSpawned(luabind::object hPlayer);
	virtual void OnNPCDeath(luabind::object hVictim,DamageInfo *dmgInfo);
	virtual bool OnActionInput(luabind::object hPlayer,uint32_t action,bool bPressed);
	virtual void OnPlayerDropped(luabind::object hPlayer,int32_t reason);
	virtual void OnPlayerReady(luabind::object hPlayer);
	virtual void OnPlayerJoined(luabind::object hPlayer);
	virtual void OnGameInitialized();
	virtual void OnMapInitialized();
	virtual void OnGameReady();
	const std::string &GetName() const;
	const std::string &GetIdentifier() const;
	const std::string &GetClassName() const;
	const std::string &GetAuthor() const;
	util::Version GetVersion() const;
};

struct DLLNETWORK GameModeWrapper
	: GameMode,luabind::wrap_base,LuaObjectBase
{
	GameModeWrapper();
	virtual ~GameModeWrapper();

	virtual void Think() override;
	void LThink();
	static void default_Think(GameMode *ptr);

	virtual void Tick() override;
	void LTick();
	static void default_Tick(GameMode *ptr);

	virtual void OnEntityTakeDamage(EntityHandle hEnt,DamageInfo &dmg) override;
	void LOnEntityTakeDamage(EntityHandle hEnt,DamageInfo &dmg);
	static void default_OnEntityTakeDamage(GameMode *ptr,EntityHandle hEnt,DamageInfo &dmg);

	virtual void OnEntityTakenDamage(EntityHandle hEnt,DamageInfo &dmg,unsigned short oldHealth,unsigned short newHealth) override;
	void LOnEntityTakenDamage(EntityHandle hEnt,DamageInfo &dmg,unsigned short oldHealth,unsigned short newHealth);
	static void default_OnEntityTakenDamage(GameMode *ptr,EntityHandle hEnt,DamageInfo &dmg,unsigned short oldHealth,unsigned short newHealth);

	virtual void OnEntityHealthChanged(EntityHandle hEnt,unsigned short oldHealth,unsigned short newHealth) override;
	void LOnEntityHealthChanged(EntityHandle hEnt,unsigned short oldHealth,unsigned short newHealth);
	static void default_OnEntityHealthChanged(GameMode *ptr,EntityHandle hEnt,unsigned short oldHealth,unsigned short newHealth);

	virtual void OnPlayerDeath(luabind::object hVictim,DamageInfo *dmgInfo) override;
	void LOnPlayerDeath(luabind::object hVictim,DamageInfo *dmgInfo);
	static void default_OnPlayerDeath(GameMode *ptr,luabind::object hVictim,DamageInfo *dmgInfo);

	virtual void OnPlayerSpawned(luabind::object hPlayer) override;
	void LOnPlayerSpawned(luabind::object hPlayer);
	static void default_OnPlayerSpawned(GameMode *ptr,luabind::object hPlayer);

	virtual void OnNPCDeath(luabind::object hVictim,DamageInfo *dmgInfo) override;
	void LOnNPCDeath(luabind::object hVictim,DamageInfo *dmgInfo);
	static void default_OnNPCDeath(GameMode *ptr,luabind::object hVictim,DamageInfo *dmgInfo);

	virtual bool OnActionInput(luabind::object hPlayer,uint32_t action,bool bPressed) override;
	bool LOnActionInput(luabind::object hPlayer,uint32_t action,bool bPressed);
	static bool default_OnActionInput(GameMode *ptr,luabind::object hPlayer,uint32_t action,bool bPressed);

	virtual void OnPlayerDropped(luabind::object hPlayer,int32_t reason) override;
	void LOnPlayerDropped(luabind::object hPlayer,int32_t reason);
	static void default_OnPlayerDropped(GameMode *ptr,luabind::object hPlayer,int32_t reason);

	virtual void OnPlayerReady(luabind::object hPlayer) override;
	void LOnPlayerReady(luabind::object hPlayer);
	static void default_OnPlayerReady(GameMode *ptr,luabind::object hPlayer);

	virtual void OnPlayerJoined(luabind::object hPlayer) override;
	void LOnPlayerJoined(luabind::object hPlayer);
	static void default_OnPlayerJoined(GameMode *ptr,luabind::object hPlayer);

	virtual void OnGameInitialized() override;
	void LOnGameInitialized();
	static void default_OnGameInitialized(GameMode *ptr);

	virtual void OnMapInitialized() override;
	void LOnMapInitialized();
	static void default_OnMapInitialized(GameMode *ptr);

	virtual void OnGameReady() override;
	void LOnGameReady();
	static void default_OnGameReady(GameMode *ptr);

	virtual void Initialize(Game *game,luabind::object &o) override;
};

#endif