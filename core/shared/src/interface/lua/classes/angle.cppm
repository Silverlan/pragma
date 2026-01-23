// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.classes.angle;

export import :scripting.lua.api;
export import pragma.math;

export namespace Lua {
	namespace Angle {
		DLLNETWORK luabind::mult<Vector3, Vector3, Vector3> Orientation(lua::State *l, const EulerAngles &ang);
		DLLNETWORK EulerAngles Copy(lua::State *l, const EulerAngles &ang);
		DLLNETWORK void ToQuaternion(lua::State *l, const EulerAngles &ang, uint32_t rotationOrder);
		DLLNETWORK void Set(lua::State *l, EulerAngles &ang, float p, float y, float r);
	};
};
