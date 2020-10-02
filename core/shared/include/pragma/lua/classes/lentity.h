/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __LENTITY_H__
#define __LENTITY_H__
#include "pragma/networkdefinitions.h"
#include <pragma/definitions.h>
#include <mathutil/glmutil.h>
#include "pragma/lua/ldefinitions.h"
class EntityHandle;
class EulerAngles;
class DamageInfo;
class ModelHandle;
namespace Lua
{
	namespace Entity
	{
		DLLNETWORK void register_class(luabind::class_<EntityHandle> &classDef);
		DLLNETWORK void IsValid(lua_State *l,EntityHandle &hEnt);
		DLLNETWORK void Remove(lua_State *l,EntityHandle &hEnt);
		DLLNETWORK void GetIndex(lua_State *l,EntityHandle &hEnt);
		DLLNETWORK void IsCharacter(lua_State *l,EntityHandle &hEnt);
		DLLNETWORK void IsPlayer(lua_State *l,EntityHandle &hEnt);
		DLLNETWORK void IsNPC(lua_State *l,EntityHandle &hEnt);
		DLLNETWORK void IsWorld(lua_State *l,EntityHandle &hEnt);
		DLLNETWORK void IsInert(lua_State *l,EntityHandle &hEnt);
		DLLNETWORK void Spawn(lua_State *l,EntityHandle &hEnt);
		DLLNETWORK void GetClass(lua_State *l,EntityHandle &hEnt);

		DLLNETWORK void IsScripted(lua_State *l,EntityHandle &hEnt);
		DLLNETWORK void IsSpawned(lua_State *l,EntityHandle &hEnt);
		DLLNETWORK void SetKeyValue(lua_State *l,EntityHandle &hEnt,std::string key,std::string val);
		DLLNETWORK void IsWeapon(lua_State *l,EntityHandle &hEnt);
		DLLNETWORK void IsVehicle(lua_State *l,EntityHandle &hEnt);
		DLLNETWORK void RemoveSafely(lua_State *l,EntityHandle &hEnt);
		DLLNETWORK void RemoveEntityOnRemoval(lua_State *l,EntityHandle &hEnt,EntityHandle &hEntOther);
		DLLNETWORK void RemoveEntityOnRemoval(lua_State *l,EntityHandle &hEnt,EntityHandle &hEntOther,Bool bRemove);
		DLLNETWORK void GetSpawnFlags(lua_State *l,EntityHandle &hEnt);

		DLLNETWORK void Save(lua_State *l,EntityHandle &hEnt,::DataStream &ds);
		DLLNETWORK void Load(lua_State *l,EntityHandle &hEnt,::DataStream &ds);
		DLLNETWORK void Copy(lua_State *l,EntityHandle &hEnt);

		DLLNETWORK void GetAirDensity(lua_State *l,EntityHandle &hEnt);

		DLLNETWORK void IsStatic(lua_State *l,EntityHandle &hEnt);
		DLLNETWORK void IsDynamic(lua_State *l,EntityHandle &hEnt);

		DLLNETWORK void SetEnabled(lua_State *l,EntityHandle &hEnt,bool enabled);
		DLLNETWORK void IsEnabled(lua_State *l,EntityHandle &hEnt);

		DLLNETWORK Color GetColor(lua_State *l,EntityHandle &hEnt);
		DLLNETWORK void SetColor(lua_State *l,EntityHandle &hEnt,const Color &color);
	};
};

#endif