/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LDAMAGEINFO_H__
#define __LDAMAGEINFO_H__

#include <pragma/definitions.h>
#include "pragma/networkdefinitions.h"
#include "pragma/lua/ldefinitions.h"
#include <mathutil/glmutil.h>

class DamageInfo;
class EntityHandle;
namespace Lua
{
	namespace DamageInfo
	{
		DLLNETWORK void SetDamage(lua_State *l,::DamageInfo &info,unsigned int dmg);
		DLLNETWORK void AddDamage(lua_State *l,::DamageInfo &info,unsigned int dmg);
		DLLNETWORK void ScaleDamage(lua_State *l,::DamageInfo &info,float scale);
		DLLNETWORK void GetDamage(lua_State *l,::DamageInfo &info);
		DLLNETWORK void GetAttacker(lua_State *l,::DamageInfo &info);
		DLLNETWORK void SetAttacker(lua_State *l,::DamageInfo &info,EntityHandle &hAttacker);
		DLLNETWORK void GetInflictor(lua_State *l,::DamageInfo &info);
		DLLNETWORK void SetInflictor(lua_State *l,::DamageInfo &info,EntityHandle &hInflictor);
		DLLNETWORK void GetDamageTypes(lua_State *l,::DamageInfo &info);
		DLLNETWORK void SetDamageType(lua_State *l,::DamageInfo &info,int type);
		DLLNETWORK void AddDamageType(lua_State *l,::DamageInfo &info,int type);
		DLLNETWORK void RemoveDamageType(lua_State *l,::DamageInfo &info,int type);
		DLLNETWORK void IsDamageType(lua_State *l,::DamageInfo &info,int type);
		DLLNETWORK void SetSource(lua_State *l,::DamageInfo &info,Vector3 &source);
		DLLNETWORK void GetSource(lua_State *l,::DamageInfo &info);
		DLLNETWORK void SetHitPosition(lua_State *l,::DamageInfo &info,Vector3 &hitPos);
		DLLNETWORK void GetHitPosition(lua_State *l,::DamageInfo &info);
		DLLNETWORK void SetForce(lua_State *l,::DamageInfo &info,Vector3 &force);
		DLLNETWORK void GetForce(lua_State *l,::DamageInfo &info);
		DLLNETWORK void GetHitGroup(lua_State *l,::DamageInfo &info);
		DLLNETWORK void SetHitGroup(lua_State *l,::DamageInfo &info,uint32_t hitGroup);
	};
};

#endif
