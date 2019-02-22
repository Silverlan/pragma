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

		// Obsolete
		/*
		DLLNETWORK void CallCallbacks(lua_State *l,EntityHandle &hEnt,const std::string &name);
		DLLNETWORK void CallCallbacks(lua_State *l,EntityHandle &hEnt,const std::string &name,luabind::object o1);
		DLLNETWORK void CallCallbacks(lua_State *l,EntityHandle &hEnt,const std::string &name,luabind::object o1,luabind::object o2);
		DLLNETWORK void CallCallbacks(lua_State *l,EntityHandle &hEnt,const std::string &name,luabind::object o1,luabind::object o2,luabind::object o3);
		DLLNETWORK void CallCallbacks(lua_State *l,EntityHandle &hEnt,const std::string &name,luabind::object o1,luabind::object o2,luabind::object o3,luabind::object o4);
		DLLNETWORK void CallCallbacks(lua_State *l,EntityHandle &hEnt,const std::string &name,luabind::object o1,luabind::object o2,luabind::object o3,luabind::object o4,luabind::object o5);
		DLLNETWORK void CallCallbacks(lua_State *l,EntityHandle &hEnt,const std::string &name,luabind::object o1,luabind::object o2,luabind::object o3,luabind::object o4,luabind::object o5,luabind::object o6);
		DLLNETWORK void CallCallbacks(lua_State *l,EntityHandle &hEnt,const std::string &name,luabind::object o1,luabind::object o2,luabind::object o3,luabind::object o4,luabind::object o5,luabind::object o6,luabind::object o7);
		DLLNETWORK void CallCallbacks(lua_State *l,EntityHandle &hEnt,const std::string &name,luabind::object o1,luabind::object o2,luabind::object o3,luabind::object o4,luabind::object o5,luabind::object o6,luabind::object o7,luabind::object o8);
		DLLNETWORK void CallCallbacks(lua_State *l,EntityHandle &hEnt,const std::string &name,luabind::object o1,luabind::object o2,luabind::object o3,luabind::object o4,luabind::object o5,luabind::object o6,luabind::object o7,luabind::object o8,luabind::object o9);
		DLLNETWORK void CallCallbacks(lua_State *l,EntityHandle &hEnt,const std::string &name,luabind::object o1,luabind::object o2,luabind::object o3,luabind::object o4,luabind::object o5,luabind::object o6,luabind::object o7,luabind::object o8,luabind::object o9,luabind::object o10);
		DLLNETWORK void CallCallbacks(lua_State *l,EntityHandle &hEnt,const std::string &name,luabind::object o1,luabind::object o2,luabind::object o3,luabind::object o4,luabind::object o5,luabind::object o6,luabind::object o7,luabind::object o8,luabind::object o9,luabind::object o10,luabind::object o11);
		DLLNETWORK void CallCallbacks(lua_State *l,EntityHandle &hEnt,const std::string &name,luabind::object o1,luabind::object o2,luabind::object o3,luabind::object o4,luabind::object o5,luabind::object o6,luabind::object o7,luabind::object o8,luabind::object o9,luabind::object o10,luabind::object o11,luabind::object o12);
		DLLNETWORK void CallCallbacks(lua_State *l,EntityHandle &hEnt,const std::string &name,luabind::object o1,luabind::object o2,luabind::object o3,luabind::object o4,luabind::object o5,luabind::object o6,luabind::object o7,luabind::object o8,luabind::object o9,luabind::object o10,luabind::object o11,luabind::object o12,luabind::object o13);
		DLLNETWORK void CallCallbacks(lua_State *l,EntityHandle &hEnt,const std::string &name,luabind::object o1,luabind::object o2,luabind::object o3,luabind::object o4,luabind::object o5,luabind::object o6,luabind::object o7,luabind::object o8,luabind::object o9,luabind::object o10,luabind::object o11,luabind::object o12,luabind::object o13,luabind::object o14);
		DLLNETWORK void CallCallbacks(lua_State *l,EntityHandle &hEnt,const std::string &name,luabind::object o1,luabind::object o2,luabind::object o3,luabind::object o4,luabind::object o5,luabind::object o6,luabind::object o7,luabind::object o8,luabind::object o9,luabind::object o10,luabind::object o11,luabind::object o12,luabind::object o13,luabind::object o14,luabind::object o15);
		*/
		DLLNETWORK void Save(lua_State *l,EntityHandle &hEnt,::DataStream &ds);
		DLLNETWORK void Load(lua_State *l,EntityHandle &hEnt,::DataStream &ds);
		DLLNETWORK void Copy(lua_State *l,EntityHandle &hEnt);

		DLLNETWORK void GetAirDensity(lua_State *l,EntityHandle &hEnt);

		DLLNETWORK void IsStatic(lua_State *l,EntityHandle &hEnt);
		DLLNETWORK void IsDynamic(lua_State *l,EntityHandle &hEnt);
	};
};

#endif