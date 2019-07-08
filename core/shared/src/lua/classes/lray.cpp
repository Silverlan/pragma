#include "stdafx_shared.h"
#include "pragma/lua/libraries/lray.h"
#include "pragma/physics/raytraces.h"
#include <pragma/engine.h>
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/lua/classes/ldef_physobj.h"
#include "pragma/lua/classes/ldef_tracedata.h"
#include "pragma/lua/classes/lphysics.h"
#include <pragma/game/game.h>
#include "luasystem.h"
#include "pragma/physics/shape.hpp"

extern DLLENGINE Engine *engine;

void Lua_TraceData_SetSource(lua_State*,TraceData &data,const util::TSharedHandle<pragma::physics::IConvexShape> &shape)
{
	data.SetShape(*shape);
}
void Lua_TraceData_SetFlags(lua_State*,TraceData &data,unsigned int flags)
{
	data.SetFlags(static_cast<RayCastFlags>(flags));
}
void Lua_TraceData_SetCollisionFilterMask(lua_State*,TraceData &data,unsigned int mask)
{
	data.SetCollisionFilterMask(static_cast<CollisionMask>(mask));
}
void Lua_TraceData_SetCollisionFilterGroup(lua_State*,TraceData &data,unsigned int group)
{
	data.SetCollisionFilterGroup(static_cast<CollisionMask>(group));
}
void Lua_TraceData_GetSourceTransform(lua_State *l,TraceData &data) {Lua::Push<pragma::physics::Transform>(l,data.GetSource());}
void Lua_TraceData_GetTargetTransform(lua_State *l,TraceData &data) {Lua::Push<pragma::physics::Transform>(l,data.GetTarget());}
void Lua_TraceData_GetSourceOrigin(lua_State *l,TraceData &data) {Lua::Push<Vector3>(l,data.GetSourceOrigin());}
void Lua_TraceData_GetTargetOrigin(lua_State *l,TraceData &data) {Lua::Push<Vector3>(l,data.GetTargetOrigin());}
void Lua_TraceData_GetSourceRotation(lua_State *l,TraceData &data) {Lua::Push<Quat>(l,data.GetSourceRotation());}
void Lua_TraceData_GetTargetRotation(lua_State *l,TraceData &data) {Lua::Push<Quat>(l,data.GetTargetRotation());}
void Lua_TraceData_GetDistance(lua_State *l,TraceData &data) {Lua::PushNumber(l,data.GetDistance());}
void Lua_TraceData_GetDirection(lua_State *l,TraceData &data) {Lua::Push<Vector3>(l,data.GetDirection());}
void Lua_TraceData_SetFilter(lua_State *l,TraceData &data,luabind::object)
{
#ifdef ENABLE_DEPRECATED_PHYSICS
	if(Lua::IsEntity(l,2))
	{
		auto *ent = Lua::CheckEntity(l,2);
		data.SetFilter(ent->GetHandle());
		return;
	}
	else if(Lua::IsPhysObj(l,2))
	{
		auto *phys = Lua::CheckEntity(l,2);
		data.SetFilter(phys->GetHandle());
		return;
	}
	else if(Lua::IsTable(l,2))
	{
		Lua::CheckTable(l,2);
		Lua::PushValue(l,2); /* 1 */
		auto table = Lua::GetStackTop(l);

		Lua::PushNil(l); /* 2 */
		std::vector<EntityHandle> ents;
		std::vector<PhysObjHandle> phys;
		while(Lua::GetNextPair(l,table) != 0) /* 3 */
		{
			if(Lua::IsEntity(l,-1))
			{
				if(phys.empty())
				{
					BaseEntity *v = Lua::CheckEntity(l,-1); /* 3 */
					ents.push_back(v->GetHandle());
				}
			}
			else if(ents.empty())
			{
				PhysObj *v = Lua::CheckPhysObj(l,-1); /* 3 */
				phys.push_back(v->GetHandle());
			}
			Lua::Pop(l,1); /* 2 */
		} /* 1 */
		Lua::Pop(l,1); /* 0 */
		if(!ents.empty())
			data.SetFilter(ents);
		else if(!phys.empty())
			data.SetFilter(phys);
		return;
	}
	Lua::CheckFunction(l,2);
	data.SetFilter(LuaFunction(luabind::object(luabind::from_stack(l,2))));
#endif
}
