/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/libraries/lray.h"
#include "pragma/physics/raytraces.h"
#include <pragma/engine.h>
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/lua/classes/ldef_physobj.h"
#include "pragma/lua/classes/ldef_tracedata.h"
#include "pragma/lua/classes/lphysics.h"
#include "pragma/lua/converters/game_type_converters_t.hpp"
#include "pragma/lua/lua_call.hpp"
#include <pragma/game/game.h>
#include "luasystem.h"
#include "pragma/physics/shape.hpp"

extern DLLNETWORK Engine *engine;

void Lua::TraceData::SetSource(lua_State *l, ::TraceData &data, const pragma::physics::IConvexShape &shape) { data.SetShape(shape); }
void Lua::TraceData::SetFlags(lua_State *, ::TraceData &data, unsigned int flags) { data.SetFlags(static_cast<RayCastFlags>(flags)); }
void Lua::TraceData::SetCollisionFilterMask(lua_State *, ::TraceData &data, unsigned int mask) { data.SetCollisionFilterMask(static_cast<CollisionMask>(mask)); }
void Lua::TraceData::SetCollisionFilterGroup(lua_State *, ::TraceData &data, unsigned int group) { data.SetCollisionFilterGroup(static_cast<CollisionMask>(group)); }
void Lua::TraceData::GetSourceTransform(lua_State *l, ::TraceData &data) { Lua::Push<umath::Transform>(l, data.GetSource()); }
void Lua::TraceData::GetTargetTransform(lua_State *l, ::TraceData &data) { Lua::Push<umath::Transform>(l, data.GetTarget()); }
void Lua::TraceData::GetSourceOrigin(lua_State *l, ::TraceData &data) { Lua::Push<Vector3>(l, data.GetSourceOrigin()); }
void Lua::TraceData::GetTargetOrigin(lua_State *l, ::TraceData &data) { Lua::Push<Vector3>(l, data.GetTargetOrigin()); }
void Lua::TraceData::GetSourceRotation(lua_State *l, ::TraceData &data) { Lua::Push<Quat>(l, data.GetSourceRotation()); }
void Lua::TraceData::GetTargetRotation(lua_State *l, ::TraceData &data) { Lua::Push<Quat>(l, data.GetTargetRotation()); }
void Lua::TraceData::GetDistance(lua_State *l, ::TraceData &data) { Lua::PushNumber(l, data.GetDistance()); }
void Lua::TraceData::GetDirection(lua_State *l, ::TraceData &data) { Lua::Push<Vector3>(l, data.GetDirection()); }
void Lua::TraceData::SetFilter(lua_State *l, ::TraceData &data, luabind::object)
{
	if(Lua::IsType<BaseEntity>(l, 2)) {
		auto &ent = Lua::Check<BaseEntity>(l, 2);
		data.SetFilter(ent);
		return;
	}
	else if(Lua::IsPhysObj(l, 2)) {
		auto &phys = Lua::Check<PhysObj>(l, 2);
		data.SetFilter(phys);
		return;
	}
	else if(Lua::IsTable(l, 2)) {
		Lua::CheckTable(l, 2);
		Lua::PushValue(l, 2); /* 1 */
		auto table = Lua::GetStackTop(l);

		Lua::PushNil(l); /* 2 */
		std::vector<EntityHandle> ents;
		while(Lua::GetNextPair(l, table) != 0) /* 3 */
		{
			BaseEntity &v = Lua::Check<BaseEntity>(l, -1); /* 3 */
			ents.push_back(v.GetHandle());

			Lua::Pop(l, 1); /* 2 */
		}                   /* 1 */
		Lua::Pop(l, 1);     /* 0 */
		data.SetFilter(std::move(ents));
		return;
	}
	Lua::CheckFunction(l, 2);
	auto oFc = luabind::object(luabind::from_stack(l, 2));
	data.SetFilter([l, oFc](pragma::physics::IShape &shape, pragma::physics::IRigidBody &body) -> RayCastHitType {
		auto c = Lua::CallFunction(
		  l,
		  [oFc, &shape, &body](lua_State *l) -> Lua::StatusCode {
			  oFc.push(l);
			  shape.Push(l);
			  body.Push(l);
			  return Lua::StatusCode::Ok;
		  },
		  1);
		if(c != Lua::StatusCode::Ok || Lua::IsSet(l, -1) == false || Lua::IsNumber(l, -1) == false)
			return RayCastHitType::Block;
		return static_cast<RayCastHitType>(Lua::CheckInt(l, -1));
	});
}
