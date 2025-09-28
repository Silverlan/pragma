// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


module pragma.shared;

import :physics.controller_hit_data;

ControllerHitData::ControllerHitData() : physObj(NULL) { Clear(); }

void ControllerHitData::Clear()
{
	if(physObj != NULL) {
		delete physObj;
		physObj = NULL;
	}
}

ControllerHitData::~ControllerHitData() { Clear(); }
