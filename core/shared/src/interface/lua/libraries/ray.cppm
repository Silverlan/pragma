// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.libraries.ray;

export import :physics.raycast;

export namespace Lua::TraceData {
	DLLNETWORK void SetSource(lua::State *l, pragma::physics::TraceData &data, const pragma::physics::IConvexShape &shape);
	DLLNETWORK void SetFlags(lua::State *l, pragma::physics::TraceData &data, unsigned int flags);
	DLLNETWORK void SetFilter(lua::State *l, pragma::physics::TraceData &data, luabind::object);
	DLLNETWORK void SetCollisionFilterMask(lua::State *, pragma::physics::TraceData &data, unsigned int mask);
	DLLNETWORK void SetCollisionFilterGroup(lua::State *, pragma::physics::TraceData &data, unsigned int group);
	DLLNETWORK void GetSourceTransform(lua::State *, pragma::physics::TraceData &data);
	DLLNETWORK void GetTargetTransform(lua::State *, pragma::physics::TraceData &data);
	DLLNETWORK void GetSourceOrigin(lua::State *, pragma::physics::TraceData &data);
	DLLNETWORK void GetTargetOrigin(lua::State *, pragma::physics::TraceData &data);
	DLLNETWORK void GetSourceRotation(lua::State *, pragma::physics::TraceData &data);
	DLLNETWORK void GetTargetRotation(lua::State *, pragma::physics::TraceData &data);
	DLLNETWORK void GetDistance(lua::State *, pragma::physics::TraceData &data);
	DLLNETWORK void GetDirection(lua::State *, pragma::physics::TraceData &data);
};
