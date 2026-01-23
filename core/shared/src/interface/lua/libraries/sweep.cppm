// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.libraries.sweep;

export import pragma.math;

export namespace Lua::sweep {
	DLLNETWORK void AABBWithAABB(const Vector3 &originA, const Vector3 &endA, const Vector3 &extentsA, const Vector3 &originB, const Vector3 &endB, const Vector3 &extentsB, bool &outResult, float &outTEntry, float &outTExit, Vector3 &outNormal);
	DLLNETWORK void AABBWithPlane(const Vector3 &ext, const Vector3 &origin, const Vector3 &dir, const Vector3 &n, float d, bool &outResult, float &outT);
};
