#ifndef __LRAY_H__
#define __LRAY_H__
#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

class TraceData;
class PhysObjHandle;
class EntityHandle;
class LPhysShape;
DLLNETWORK void Lua_TraceData_SetSource(lua_State *l,TraceData &data,const LPhysShape &shape);
DLLNETWORK void Lua_TraceData_SetSource(lua_State *l,TraceData &data,const PhysObjHandle &hPhys);
DLLNETWORK void Lua_TraceData_SetSource(lua_State *l,TraceData &data,const EntityHandle &hAttacker);
DLLNETWORK void Lua_TraceData_SetFlags(lua_State *l,TraceData &data,unsigned int flags);
DLLNETWORK void Lua_TraceData_SetFilter(lua_State *l,TraceData &data,luabind::object);
DLLNETWORK void Lua_TraceData_SetCollisionFilterMask(lua_State*,TraceData &data,unsigned int mask);
DLLNETWORK void Lua_TraceData_SetCollisionFilterGroup(lua_State*,TraceData &data,unsigned int group);
DLLNETWORK void Lua_TraceData_GetSourceTransform(lua_State*,TraceData &data);
DLLNETWORK void Lua_TraceData_GetTargetTransform(lua_State*,TraceData &data);
DLLNETWORK void Lua_TraceData_GetSourceOrigin(lua_State*,TraceData &data);
DLLNETWORK void Lua_TraceData_GetTargetOrigin(lua_State*,TraceData &data);
DLLNETWORK void Lua_TraceData_GetSourceRotation(lua_State*,TraceData &data);
DLLNETWORK void Lua_TraceData_GetTargetRotation(lua_State*,TraceData &data);
DLLNETWORK void Lua_TraceData_GetDistance(lua_State*,TraceData &data);
DLLNETWORK void Lua_TraceData_GetDirection(lua_State*,TraceData &data);

DLLNETWORK void Lua_TraceData_FillTraceResultTable(lua_State *l,TraceResult &res);

#endif
