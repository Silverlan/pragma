// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.libraries.ray;

export import :physics.raycast;

export namespace Lua::TraceData {
	DLLNETWORK void SetSource(lua::State *l, ::TraceData &data, const pragma::physics::IConvexShape &shape);
	DLLNETWORK void SetFlags(lua::State *l, ::TraceData &data, unsigned int flags);
	DLLNETWORK void SetFilter(lua::State *l, ::TraceData &data, luabind::object);
	DLLNETWORK void SetCollisionFilterMask(lua::State *, ::TraceData &data, unsigned int mask);
	DLLNETWORK void SetCollisionFilterGroup(lua::State *, ::TraceData &data, unsigned int group);
	DLLNETWORK void GetSourceTransform(lua::State *, ::TraceData &data);
	DLLNETWORK void GetTargetTransform(lua::State *, ::TraceData &data);
	DLLNETWORK void GetSourceOrigin(lua::State *, ::TraceData &data);
	DLLNETWORK void GetTargetOrigin(lua::State *, ::TraceData &data);
	DLLNETWORK void GetSourceRotation(lua::State *, ::TraceData &data);
	DLLNETWORK void GetTargetRotation(lua::State *, ::TraceData &data);
	DLLNETWORK void GetDistance(lua::State *, ::TraceData &data);
	DLLNETWORK void GetDirection(lua::State *, ::TraceData &data);
};
