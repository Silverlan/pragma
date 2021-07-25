/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LENTITY_H__
#define __LENTITY_H__
#include "pragma/networkdefinitions.h"
#include <pragma/definitions.h>
#include <mathutil/glmutil.h>
#include "pragma/lua/ldefinitions.h"

class EulerAngles;
class DamageInfo;
class ModelHandle;
namespace Lua
{
	namespace Entity
	{
		DLLNETWORK void register_class(luabind::class_<BaseEntity> &classDef);
		DLLNETWORK bool IsValid(BaseEntity *ent);

		DLLNETWORK void RemoveEntityOnRemoval(BaseEntity &ent,BaseEntity &entOther);
		DLLNETWORK void RemoveEntityOnRemoval(BaseEntity &ent,BaseEntity &entOther,Bool bRemove);

		DLLNETWORK float GetAirDensity(BaseEntity &ent);

		DLLNETWORK void SetEnabled(BaseEntity &ent,bool enabled);
		DLLNETWORK bool IsEnabled(BaseEntity &ent);
		DLLNETWORK bool IsDisabled(BaseEntity &ent);

		DLLNETWORK Color GetColor(BaseEntity &ent);
		DLLNETWORK void SetColor(BaseEntity &ent,const Color &color);
	};
};

#endif