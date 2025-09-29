// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "mathutil/uvec.h"
#include <mathutil/glmutil.h>

export module pragma.shared:physics.controller_hit_data;

export import :physics.object_handle;

export struct DLLNETWORK ControllerHitData {
	ControllerHitData();
	~ControllerHitData();
	PhysObjHandle *physObj = nullptr;
	Vector3 hitNormal = {};
	void Clear();
};
