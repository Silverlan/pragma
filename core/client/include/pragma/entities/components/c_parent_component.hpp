/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_PARENT_COMPONENT_HPP__
#define __C_PARENT_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_parent_component.hpp>

namespace pragma
{
	class CParticleSystemComponent;
	class DLLCLIENT CParentComponent final
		: public BaseParentComponent
	{
	public:
		CParentComponent(BaseEntity &ent) : BaseParentComponent(ent) {}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;

		virtual void OnRemove() override;
	};
};

#endif
