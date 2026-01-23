// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :physics.controller_hit_data;

pragma::physics::ControllerHitData::ControllerHitData() : physObj(nullptr) { Clear(); }

void pragma::physics::ControllerHitData::Clear()
{
	if(physObj != nullptr) {
		delete physObj;
		physObj = nullptr;
	}
}

pragma::physics::ControllerHitData::~ControllerHitData() { Clear(); }
