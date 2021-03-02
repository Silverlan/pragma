/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_FUNC_KINEMATIC_HPP__
#define __C_FUNC_KINEMATIC_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/func/basefunckinematic.hpp>
#include <pragma/entities/entity_component_manager.hpp>

namespace pragma
{
	class DLLCLIENT CKinematicComponent final
		: public BaseFuncKinematicComponent,
		public CBaseNetComponent
	{
	public:
		CKinematicComponent(BaseEntity &ent) : BaseFuncKinematicComponent(ent) {}
		virtual void Initialize() override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet) override;
		virtual void OnEntitySpawn() override;
	protected:
		bool m_bInitiallyMoving = false;
	};
};

class DLLCLIENT CFuncKinematic
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif