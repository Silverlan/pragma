// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <ostream>

#include <string>

module pragma.shared;

import :physics.controller_hit_data;

ControllerHitData::ControllerHitData() : physObj(nullptr) { Clear(); }

void ControllerHitData::Clear()
{
	if(physObj != nullptr) {
		delete physObj;
		physObj = nullptr;
	}
}

ControllerHitData::~ControllerHitData() { Clear(); }
