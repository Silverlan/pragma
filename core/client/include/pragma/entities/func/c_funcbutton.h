/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_FUNCBUTTON_H__
#define __C_FUNCBUTTON_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/func/basefuncbutton.h"

namespace pragma
{
	class DLLCLIENT CButtonComponent final
		: public BaseFuncButtonComponent
	{
	public:
		CButtonComponent(BaseEntity &ent) : BaseFuncButtonComponent(ent) {}
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class DLLCLIENT CFuncButton
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif
