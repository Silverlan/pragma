// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;




module pragma.shared;

import :scripting.lua.libraries.ray;

void Lua::TraceData::SetSource(lua::State *l, ::TraceData &data, const pragma::physics::IConvexShape &shape) { data.SetShape(shape); }
void Lua::TraceData::SetFlags(lua::State *, ::TraceData &data, unsigned int flags) { data.SetFlags(static_cast<pragma::physics::RayCastFlags>(flags)); }
void Lua::TraceData::SetCollisionFilterMask(lua::State *, ::TraceData &data, unsigned int mask) { data.SetCollisionFilterMask(static_cast<pragma::physics::CollisionMask>(mask)); }
void Lua::TraceData::SetCollisionFilterGroup(lua::State *, ::TraceData &data, unsigned int group) { data.SetCollisionFilterGroup(static_cast<pragma::physics::CollisionMask>(group)); }
void Lua::TraceData::GetSourceTransform(lua::State *l, ::TraceData &data) { Lua::Push<umath::Transform>(l, data.GetSource()); }
void Lua::TraceData::GetTargetTransform(lua::State *l, ::TraceData &data) { Lua::Push<umath::Transform>(l, data.GetTarget()); }
void Lua::TraceData::GetSourceOrigin(lua::State *l, ::TraceData &data) { Lua::Push<Vector3>(l, data.GetSourceOrigin()); }
void Lua::TraceData::GetTargetOrigin(lua::State *l, ::TraceData &data) { Lua::Push<Vector3>(l, data.GetTargetOrigin()); }
void Lua::TraceData::GetSourceRotation(lua::State *l, ::TraceData &data) { Lua::Push<Quat>(l, data.GetSourceRotation()); }
void Lua::TraceData::GetTargetRotation(lua::State *l, ::TraceData &data) { Lua::Push<Quat>(l, data.GetTargetRotation()); }
void Lua::TraceData::GetDistance(lua::State *l, ::TraceData &data) { Lua::PushNumber(l, data.GetDistance()); }
void Lua::TraceData::GetDirection(lua::State *l, ::TraceData &data) { Lua::Push<Vector3>(l, data.GetDirection()); }
void Lua::TraceData::SetFilter(lua::State *l, ::TraceData &data, luabind::object)
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
	else if(Lua::IsTable(l, 2)) {
		Lua::CheckTable(l, 2);
		Lua::PushValue(l, 2); /* 1 */
		auto table = Lua::GetStackTop(l);

		Lua::PushNil(l); /* 2 */
		std::vector<EntityHandle> ents;
		while(Lua::GetNextPair(l, table) != 0) /* 3 */
		{
			pragma::ecs::BaseEntity &v = Lua::Check<pragma::ecs::BaseEntity>(l, -1); /* 3 */
			ents.push_back(v.GetHandle());

			Lua::Pop(l, 1); /* 2 */
		} /* 1 */
		Lua::Pop(l, 1); /* 0 */
		data.SetFilter(std::move(ents));
		return;
	}
	Lua::CheckFunction(l, 2);
	auto oFc = luabind::object(luabind::from_stack(l, 2));
	data.SetFilter([l, oFc](pragma::physics::IShape &shape, pragma::physics::IRigidBody &body) -> pragma::physics::RayCastHitType {
		auto c = Lua::CallFunction(
		  l,
		  [oFc, &shape, &body](lua::State *l) -> Lua::StatusCode {
			  oFc.push(l);
			  shape.Push(l);
			  body.Push(l);
			  return Lua::StatusCode::Ok;
		  },
		  1);
		if(c != Lua::StatusCode::Ok || Lua::IsSet(l, -1) == false || Lua::IsNumber(l, -1) == false)
			return pragma::physics::RayCastHitType::Block;
		return static_cast<pragma::physics::RayCastHitType>(Lua::CheckInt(l, -1));
	});
}
