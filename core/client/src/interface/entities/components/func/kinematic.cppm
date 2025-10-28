// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"



export module pragma.client:entities.components.func_kinematic;

import :entities.base_entity;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLCLIENT CKinematicComponent final : public BaseFuncKinematicComponent, public CBaseNetComponent {
		public:
			CKinematicComponent(pragma::ecs::BaseEntity &ent) : BaseFuncKinematicComponent(ent) {}
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
};
