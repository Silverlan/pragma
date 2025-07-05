// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_FUNC_KINEMATIC_HPP__
#define __C_FUNC_KINEMATIC_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/func/basefunckinematic.hpp>
#include <pragma/entities/entity_component_manager.hpp>

namespace pragma {
	class DLLCLIENT CKinematicComponent final : public BaseFuncKinematicComponent, public CBaseNetComponent {
	  public:
		CKinematicComponent(BaseEntity &ent) : BaseFuncKinematicComponent(ent) {}
		virtual void Initialize() override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
		virtual void OnEntitySpawn() override;
	  protected:
		bool m_bInitiallyMoving = false;
	};
};

class DLLCLIENT CFuncKinematic : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
