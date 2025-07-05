// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_FUNC_PHYSICS_H__
#define __S_FUNC_PHYSICS_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/func/basefuncphysics.h"
#include "pragma/entities/components/s_entity_component.hpp"

namespace pragma {
	class DLLSERVER SFuncPhysicsComponent final : public BaseFuncPhysicsComponent, public SBaseNetComponent {
	  public:
		SFuncPhysicsComponent(BaseEntity &ent) : BaseFuncPhysicsComponent(ent) {}
		virtual void Initialize() override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua_State *l) override;
	  protected:
		virtual PhysObj *InitializePhysics() override;
	};
};

class DLLSERVER FuncPhysics : public SBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
