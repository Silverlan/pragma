// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.libraries.bounding_volume;

export import :scripting.lua.api;
export import pragma.math;

export namespace Lua::boundingvolume {
	DLLNETWORK luabind::mult<Vector3, Vector3> GetRotatedAABB(lua::State *l, const Vector3 &min, const Vector3 &max, const Mat3 &rot);
	DLLNETWORK luabind::mult<Vector3, Vector3> GetRotatedAABB(lua::State *l, const Vector3 &min, const Vector3 &max, const Quat &rot);
};
