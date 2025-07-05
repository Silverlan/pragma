// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PHYSTOUCH_H__
#define __PHYSTOUCH_H__
#include "pragma/networkdefinitions.h"
#include <sharedutils/functioncallback.h>
#include "pragma/entities/baseentity_handle.h"
class BaseEntity;
struct DLLNETWORK PhysTouch {
	PhysTouch(BaseEntity *ent, CallbackHandle onRemove);
	~PhysTouch();
	EntityHandle entity;
	CallbackHandle onRemoveCallback;
};

#endif
