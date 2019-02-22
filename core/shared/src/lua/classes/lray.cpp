#include "stdafx_shared.h"
#include "pragma/lua/libraries/lray.h"
#include "pragma/physics/raytraces.h"
#include <pragma/engine.h>
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/lua/classes/ldef_physobj.h"
#include "pragma/lua/classes/ldef_physx.h"
#include "pragma/lua/classes/ldef_tracedata.h"
#include "pragma/lua/classes/lphysics.h"
#include <pragma/game/game.h>
#include "luasystem.h"
#include "pragma/physics/physshape.h"

extern DLLENGINE Engine *engine;

void Lua_TraceData_SetSource(lua_State*,TraceData &data,const LPhysShape &shape)
{
	auto *btShape = shape.GetSharedPointer()->GetShape();
	if(btShape == nullptr || !btShape->isConvex())
		return;
	data.SetSource(static_cast<btConvexShape*>(btShape));
}
void Lua_TraceData_SetSource(lua_State*,TraceData &data,const PhysObjHandle &hPhys)
{
	if(!hPhys.IsValid())
		return;
	data.SetSource(hPhys.get());
}
void Lua_TraceData_SetSource(lua_State*,TraceData &data,const EntityHandle &hSource)
{
	if(!hSource.IsValid())
		return;
	data.SetSource(hSource.get());
}
void Lua_TraceData_SetFlags(lua_State*,TraceData &data,unsigned int flags)
{
	data.SetFlags(static_cast<FTRACE>(flags));
}
void Lua_TraceData_SetCollisionFilterMask(lua_State*,TraceData &data,unsigned int mask)
{
	data.SetCollisionFilterMask(static_cast<CollisionMask>(mask));
}
void Lua_TraceData_SetCollisionFilterGroup(lua_State*,TraceData &data,unsigned int group)
{
	data.SetCollisionFilterGroup(static_cast<CollisionMask>(group));
}
void Lua_TraceData_GetSourceTransform(lua_State *l,TraceData &data) {Lua::Push<PhysTransform>(l,data.GetSource());}
void Lua_TraceData_GetTargetTransform(lua_State *l,TraceData &data) {Lua::Push<PhysTransform>(l,data.GetTarget());}
void Lua_TraceData_GetSourceOrigin(lua_State *l,TraceData &data) {Lua::Push<Vector3>(l,data.GetSourceOrigin());}
void Lua_TraceData_GetTargetOrigin(lua_State *l,TraceData &data) {Lua::Push<Vector3>(l,data.GetTargetOrigin());}
void Lua_TraceData_GetSourceRotation(lua_State *l,TraceData &data) {Lua::Push<Quat>(l,data.GetSourceRotation());}
void Lua_TraceData_GetTargetRotation(lua_State *l,TraceData &data) {Lua::Push<Quat>(l,data.GetTargetRotation());}
void Lua_TraceData_GetDistance(lua_State *l,TraceData &data) {Lua::PushNumber(l,data.GetDistance());}
void Lua_TraceData_GetDirection(lua_State *l,TraceData &data) {Lua::Push<Vector3>(l,data.GetDirection());}
void Lua_TraceData_SetFilter(lua_State *l,TraceData &data,luabind::object)
{
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
}

#ifdef PHYS_ENGINE_BULLET

#elif PHYS_ENGINE_PHYSX
static struct PxFilteredQueryFilterLuaCallback
	: physx::PxQueryFilterCallback
{
protected:
	int m_function;
	lua_State *m_lua;
public:
	PxFilteredQueryFilterLuaCallback(lua_State *l,int function)
		: m_lua(l),m_function(function)
	{}
	~PxFilteredQueryFilterLuaCallback()
	{
		Lua::ReleaseReference(m_lua,m_function);
	}
	physx::PxQueryHitType::Enum preFilter(const physx::PxFilterData &filterData,const physx::PxShape* shape,const physx::PxRigidActor *actor,physx::PxHitFlags &queryFlags)
	{
		void *data = actor->userData;
		if(data == NULL)
			return physx::PxQueryHitType::Enum::eNONE;
		PhysObj *phys = static_cast<PhysObj*>(data);
		BaseEntity *owner = phys->GetOwner();
		if(owner == NULL)
			return physx::PxQueryHitType::Enum::eNONE;
		lua_rawgeti(m_lua,LUA_REGISTRYINDEX,m_function);
		lua_pushentity(m_lua,owner);
		int s = Lua::ProtectedCall(m_lua,1,1);
		if(s == 0)
		{
			int r = Lua::ToInt(m_lua,-1);
			return physx::PxQueryHitType::Enum(r);
		}
		NetworkState *state = engine->GetNetworkState(m_lua);
		_luaerror(state->GetGameState(),s,m_lua);
		return physx::PxQueryHitType::Enum::eBLOCK;
	}
	physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData &filterData,const physx::PxQueryHit &hit)
	{
		void *data = hit.actor->userData;
		if(data == NULL)
			return physx::PxQueryHitType::Enum::eNONE;
		PhysObj *phys = static_cast<PhysObj*>(data);
		BaseEntity *owner = phys->GetOwner();
		if(owner == NULL)
			return physx::PxQueryHitType::Enum::eNONE;
		lua_pushentity(m_lua,owner);
		int s = Lua::ProtectedCall(m_lua,1,1);
		if(s == 0)
		{
			int r = Lua::ToInt(m_lua,-1);
			return physx::PxQueryHitType::Enum(r);
		}
		NetworkState *state = engine->GetNetworkState(m_lua);
		_luaerror(state->GetGameState(),s,m_lua);
		return physx::PxQueryHitType::Enum::eBLOCK;
	}
};
static void FillTraceResultTable(TraceResult &res,lua_State *l,int table)
{
	Lua::PushString(l,"hit");
	Lua::PushBool(l,res.actor);
	Lua::SetTableValue(l,table);
	if(res.hit == true)
	{
		if(res.entity != NULL)
		{
			Lua::PushString(l,"entity");
			lua_pushentity(l,res.entity);
			Lua::SetTableValue(l,table);
		}
		if(res.physObj != NULL)
		{
			Lua::PushString(l,"physObj");
			luabind::object(l,res.physObj->GetHandle()).push(l);
			Lua::SetTableValue(l,table);
		}
		if(res.actor != NULL)
		{
			Lua::PushString(l,"actor");
			Lua::Push<physx::PxRigidActor*>(l,res.actor);
			Lua::SetTableValue(l,table);
		}
		Lua::PushString(l,"distance");
		Lua::PushNumber(l,res.distance);
		Lua::SetTableValue(l,table);

		Lua::PushString(l,"flags");
		Lua::PushInt(l,physx::PxU16(res.flags));
		Lua::SetTableValue(l,table);

		Lua::PushString(l,"normal");
		Lua::Push<Vector3>(l,res.normal);
		Lua::SetTableValue(l,table);

		Lua::PushString(l,"position");
		Lua::Push<Vector3>(l,res.position);
		Lua::SetTableValue(l,table);

		Lua::PushString(l,"shape");
		Lua::Push<PhysXShape>(l,PhysXShape(res.shape));
		Lua::SetTableValue(l,table);

		Lua::PushString(l,"hadInitialOverlap");
		Lua::PushBool(l,res.hadInitialOverlap);
		Lua::SetTableValue(l,table);

		Lua::PushString(l,"u");
		Lua::PushNumber(l,res.u);
		Lua::SetTableValue(l,table);

		Lua::PushString(l,"v");
		Lua::PushNumber(l,res.v);
		Lua::SetTableValue(l,table);

		Lua::PushString(l,"faceIndex");
		Lua::PushInt(l,res.faceIndex);
		Lua::SetTableValue(l,table);
	}
}

extern DLLENGINE Engine *engine;
DLLNETWORK int Lua_game_RayCast(lua_State *l)
{
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	Vector3 *origin = Lua::CheckVector(l,1);
	Vector3 *dir = Lua::CheckVector(l,2);
	float dist = Lua::CheckNumber(l,3);
	TraceResult res;
	bool b;
	unsigned int flags = 0;
	if(Lua::IsSet(l,4))
	{
		if(Lua::IsEntity(l,4))
		{
			BaseEntity *ent = Lua::CheckEntity(l,4);
			b = game->RayCast(*origin,*dir,dist,ent,&res,flags);
		}
		else if(Lua::IsTable(l,4))
		{
			Lua::PushValue(l,4);
			int table = Lua::GetStackTop(l);
			Lua::PushNil(l);
			char type = -1;
			std::vector<BaseEntity*> ents;
			std::vector<physx::PxRigidActor*> actors;
			while(Lua::GetNextPair(l,table) != 0)
			{
				if(type == -1)
				{
					if(Lua::IsEntity(l,-1))
						type = 0;
					else
						type = 1;
				}
				if(type == 0)
				{
					BaseEntity *ent = Lua::CheckEntity(l,-1);
					ents.push_back(ent);
				}
				else
				{
					PhysXRigidActor *actor = Lua::CheckPhysXRigidActor(l,-1);
					actors.push_back(actor->get());
				}
			}
			Lua::Pop(l,1);
			if(type == -1)
				b = game->RayCast(*origin,*dir,dist,&res,flags);
			else if(type == 0)
				b = game->RayCast(*origin,*dir,dist,ents,&res,flags);
			else
				b = game->RayCast(*origin,*dir,dist,actors,&res,flags);
		}
		else if(Lua::IsFunction(l,4))
		{
			Lua::CheckFunction(l,4);
			int fc = Lua::CreateReference(l,4);
			PxFilteredQueryFilterLuaCallback cb(l,fc);
			b = game->RayCast(*origin,*dir,dist,cb,&res,flags);
		}
		else if(Lua::IsPhysXRigidActor(l,4))
		{
			PhysXRigidActor *actor = Lua::CheckPhysXRigidActor(l,4);
			b = game->RayCast(*origin,*dir,dist,actor->get(),&res,flags);
		}
		else
		{
			flags = Lua::CheckInt(l,4);
			b = game->RayCast(*origin,*dir,dist,&res,flags);
		}
	}
	else
		b = game->RayCast(*origin,*dir,dist,&res);
	if((flags &FTRACE_ANY_HIT) == FTRACE_ANY_HIT)
		return b;
	int table = Lua::CreateTable(l);
	FillTraceResultTable(res,l,table);
	for(unsigned int i=0;i<res.touches.size();i++)
	{
		int tableTouch = Lua::CreateTable(l);
		TraceResult &touch = res.touches[i];
		FillTraceResultTable(touch,l,tableTouch);

		lua_rawseti(l,tableTouch,i +1);
	}
	return 1;
}

DLLNETWORK int Lua_game_Sweep(lua_State *l)
{
	return 0;
}

DLLNETWORK int Lua_game_Overlap(lua_State *l)
{
	return 0;
}
#endif