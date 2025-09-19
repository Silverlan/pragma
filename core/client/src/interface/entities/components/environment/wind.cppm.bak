// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/environment/env_wind.hpp"
#include "pragma/entities/components/c_entity_component.hpp"

export module pragma.client.entities.components.env_wind;

export {
	namespace pragma {
		class DLLCLIENT CWindComponent final : public BaseEnvWindComponent, public CBaseNetComponent {
		public:
			CWindComponent(BaseEntity &ent) : BaseEnvWindComponent(ent) {}
			virtual void ReceiveData(NetPacket &packet) override;
			virtual void InitializeLuaObject(lua_State *l) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
		};
	};

	class DLLCLIENT CEnvWind : public CBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
