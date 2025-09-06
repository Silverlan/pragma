// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/func/basefuncsoftphysics.hpp>

export module pragma.server.entities.components.func.soft_physics;

import pragma.server.entities;

export {
	namespace pragma {
		class DLLSERVER SFuncSoftPhysicsComponent final : public BaseFuncSoftPhysicsComponent, public SBaseNetComponent {
		public:
			SFuncSoftPhysicsComponent(BaseEntity &ent) : BaseFuncSoftPhysicsComponent(ent) {}
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER FuncSoftPhysics : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
