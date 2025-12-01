// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:math.seb;

export import pragma.math;

export namespace Seb {
	void DLLNETWORK Calculate(std::vector<Vector3> &verts, Vector3 &center, float &radius);
};
