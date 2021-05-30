/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __PHYSTOUCH_H__
#define __PHYSTOUCH_H__
#include "pragma/networkdefinitions.h"
#include <sharedutils/functioncallback.h>
#include "pragma/entities/baseentity_handle.h"
class BaseEntity;
struct DLLNETWORK PhysTouch
{
	PhysTouch(BaseEntity *ent,CallbackHandle onRemove);
	~PhysTouch();
	EntityHandle entity;
	CallbackHandle onRemoveCallback;
};

#endif
