// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:physics.controller_hit_data;

export import :physics.object_handle;

export namespace pragma::physics {
	struct DLLNETWORK ControllerHitData {
		ControllerHitData();
		~ControllerHitData();
		PhysObjHandle *physObj = nullptr;
		Vector3 hitNormal = {};
		void Clear();
	};
}
