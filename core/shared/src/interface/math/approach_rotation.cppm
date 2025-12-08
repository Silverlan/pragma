// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:math.approach_rotation;

export import pragma.math;

export namespace pragma::math {
	DLLNETWORK Quat approach_direction(const Quat &rot, const Vector3 &up, const Vector3 &targetDir, const Vector2 &rotAmount, Vector2 *resRotAm = nullptr, const Vector2 *pitchLimit = nullptr, const Vector2 *yawLimit = nullptr, const Quat *rotPivot = nullptr,
	  const EulerAngles *angOffset = nullptr);
};
