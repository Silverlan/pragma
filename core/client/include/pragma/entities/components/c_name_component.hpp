/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_NAME_COMPONENT_HPP__
#define __C_NAME_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_name_component.hpp>

namespace pragma
{
	class DLLCLIENT CNameComponent final
		: public BaseNameComponent,
		public CBaseNetComponent
	{
	public:
		CNameComponent(BaseEntity &ent) : BaseNameComponent(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
	};
};

#endif
