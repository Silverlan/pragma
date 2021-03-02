/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_LUA_COMPONENT_HPP__
#define __C_LUA_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/lua/sh_lua_component.hpp>

namespace pragma
{
	class DLLCLIENT CLuaBaseEntityComponent final
		: public BaseLuaBaseEntityComponent,
		public CBaseSnapshotComponent
	{
	public:
		CLuaBaseEntityComponent(BaseEntity &ent,luabind::object &o);

		virtual void ReceiveData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet) override;
		virtual void ReceiveSnapshotData(NetPacket &packet) override;
		virtual bool ShouldTransmitNetData() const override;
		virtual bool ShouldTransmitSnapshotData() const override;
	protected:
		virtual void InvokeNetEventHandle(const std::string &methodName,NetPacket &packet,pragma::BasePlayerComponent *pl) override;
	};
};

#endif
