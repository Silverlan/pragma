// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"



export module pragma.client:entities.components.prop_base;

import :entities.base_entity;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLCLIENT CPropComponent final : public BasePropComponent, public CBaseNetComponent {
		public:
			CPropComponent(pragma::ecs::BaseEntity &ent) : BasePropComponent(ent) {}
			virtual void Initialize() override;
			virtual void ReceiveData(NetPacket &packet) override;
			virtual void OnEntitySpawn() override;
			virtual void InitializeLuaObject(lua_State *l) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
		protected:
			PHYSICSTYPE m_propPhysType = pragma::physics::PHYSICSTYPE::NONE;
			MOVETYPE m_propMoveType = pragma::physics::MOVETYPE::NONE;
		};
	};

	class DLLCLIENT CBaseProp : public CBaseEntity {
	protected:
	};
};
