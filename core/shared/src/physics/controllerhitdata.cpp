// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/physics/controllerhitdata.h"
#include "pragma/physics/physobj.h"

ControllerHitData::ControllerHitData() : physObj(NULL) { Clear(); }

void ControllerHitData::Clear()
{
	if(physObj != NULL) {
		delete physObj;
		physObj = NULL;
	}
}

ControllerHitData::~ControllerHitData() { Clear(); }
