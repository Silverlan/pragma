/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_FUNC_SOFTPHYSICS_H__
#define __C_FUNC_SOFTPHYSICS_H__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/func/basefuncsoftphysics.hpp>

namespace pragma
{
	class DLLCLIENT CFuncSoftPhysicsComponent final
		: public BaseFuncSoftPhysicsComponent
	{
	public:
		CFuncSoftPhysicsComponent(BaseEntity &ent) : BaseFuncSoftPhysicsComponent(ent) {}
		virtual void Initialize() override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class DLLCLIENT CFuncSoftPhysics
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif
