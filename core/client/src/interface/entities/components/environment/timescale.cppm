// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <memory>

export module pragma.client:entities.components.env_timescale;

import :entities.base_entity;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLCLIENT CEnvTimescaleComponent final : public BaseEnvTimescaleComponent, public CBaseNetComponent {
		public:
			CEnvTimescaleComponent(BaseEntity &ent) : BaseEnvTimescaleComponent(ent) {}
			virtual void ReceiveData(NetPacket &packet) override;
			virtual void InitializeLuaObject(lua_State *l) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
		};
	};

	class DLLCLIENT CEnvTimescale : public CBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
