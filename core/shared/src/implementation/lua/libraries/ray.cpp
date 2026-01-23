// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.libraries.ray;

void Lua::TraceData::SetSource(lua::State *l, pragma::physics::TraceData &data, const pragma::physics::IConvexShape &shape) { data.SetShape(shape); }
void Lua::TraceData::SetFlags(lua::State *, pragma::physics::TraceData &data, unsigned int flags) { data.SetFlags(static_cast<pragma::physics::RayCastFlags>(flags)); }
void Lua::TraceData::SetCollisionFilterMask(lua::State *, pragma::physics::TraceData &data, unsigned int mask) { data.SetCollisionFilterMask(static_cast<pragma::physics::CollisionMask>(mask)); }
void Lua::TraceData::SetCollisionFilterGroup(lua::State *, pragma::physics::TraceData &data, unsigned int group) { data.SetCollisionFilterGroup(static_cast<pragma::physics::CollisionMask>(group)); }
void Lua::TraceData::GetSourceTransform(lua::State *l, pragma::physics::TraceData &data) { Lua::Push<pragma::math::Transform>(l, data.GetSource()); }
void Lua::TraceData::GetTargetTransform(lua::State *l, pragma::physics::TraceData &data) { Lua::Push<pragma::math::Transform>(l, data.GetTarget()); }
void Lua::TraceData::GetSourceOrigin(lua::State *l, pragma::physics::TraceData &data) { Lua::Push<Vector3>(l, data.GetSourceOrigin()); }
void Lua::TraceData::GetTargetOrigin(lua::State *l, pragma::physics::TraceData &data) { Lua::Push<Vector3>(l, data.GetTargetOrigin()); }
void Lua::TraceData::GetSourceRotation(lua::State *l, pragma::physics::TraceData &data) { Lua::Push<Quat>(l, data.GetSourceRotation()); }
void Lua::TraceData::GetTargetRotation(lua::State *l, pragma::physics::TraceData &data) { Lua::Push<Quat>(l, data.GetTargetRotation()); }
void Lua::TraceData::GetDistance(lua::State *l, pragma::physics::TraceData &data) { PushNumber(l, data.GetDistance()); }
void Lua::TraceData::GetDirection(lua::State *l, pragma::physics::TraceData &data) { Lua::Push<Vector3>(l, data.GetDirection()); }
void Lua::TraceData::SetFilter(lua::State *l, pragma::physics::TraceData &data, luabind::object)
{
	if(Lua::IsType<pragma::ecs::BaseEntity>(l, 2)) {
		auto &ent = Lua::Check<pragma::ecs::BaseEntity>(l, 2);
		data.SetFilter(ent);
		return;
	}
	else if(Lua::IsType<pragma::physics::PhysObj>(l, 2)) {
		auto &phys = Lua::Check<pragma::physics::PhysObj>(l, 2);
		data.SetFilter(phys);
		return;
	}
	else if(IsTable(l, 2)) {
		CheckTable(l, 2);
		PushValue(l, 2); /* 1 */
		auto table = GetStackTop(l);

		PushNil(l); /* 2 */
		std::vector<EntityHandle> ents;
		while(GetNextPair(l, table) != 0) /* 3 */
		{
			pragma::ecs::BaseEntity &v = Lua::Check<pragma::ecs::BaseEntity>(l, -1); /* 3 */
			ents.push_back(v.GetHandle());

			Pop(l, 1); /* 2 */
		} /* 1 */
		Pop(l, 1); /* 0 */
		data.SetFilter(std::move(ents));
		return;
	}
	CheckFunction(l, 2);
	auto oFc = luabind::object(luabind::from_stack(l, 2));
	data.SetFilter([l, oFc](pragma::physics::IShape &shape, pragma::physics::IRigidBody &body) -> pragma::physics::RayCastHitType {
		auto c = CallFunction(
		  l,
		  [oFc, &shape, &body](lua::State *l) -> StatusCode {
			  oFc.push(l);
			  shape.Push(l);
			  body.Push(l);
			  return StatusCode::Ok;
		  },
		  1);
		if(c != StatusCode::Ok || IsSet(l, -1) == false || IsNumber(l, -1) == false)
			return pragma::physics::RayCastHitType::Block;
		return static_cast<pragma::physics::RayCastHitType>(CheckInt(l, -1));
	});
}
