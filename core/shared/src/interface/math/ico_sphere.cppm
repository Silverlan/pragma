// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:math.ico_sphere;

export import pragma.math;

export namespace pragma::math {
	class DLLNETWORK IcoSphere {
	private:
		IcoSphere() = delete;
	public:
		static void Create(const Vector3 &origin, float radius, std::vector<Vector3> &verts, std::vector<uint16_t> &indices, uint32_t recursionLevel = 1);
		static void Create(const Vector3 &origin, float radius, std::vector<Vector3> &verts, uint32_t recursionLevel = 1);
	};
}
