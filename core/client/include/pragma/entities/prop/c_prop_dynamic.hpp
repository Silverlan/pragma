/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_PROP_DYNAMIC_HPP__
#define __C_PROP_DYNAMIC_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include <pragma/entities/prop/prop_dynamic.hpp>

namespace pragma
{
	class DLLCLIENT CPropDynamicComponent final
		: public BasePropDynamicComponent
	{
	public:
		CPropDynamicComponent(BaseEntity &ent) : BasePropDynamicComponent(ent) {}
		virtual void Initialize() override;
		using BasePropDynamicComponent::BasePropDynamicComponent;
		virtual void InitializeLuaObject(lua_State *l) override;
	protected:
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
	};
};

class DLLCLIENT CPropDynamic
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif