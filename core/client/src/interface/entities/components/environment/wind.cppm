// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"

export module pragma.client:entities.components.env_wind;

export import :entities.base_entity;
export import :entities.components.entity;

export {
	namespace pragma {
		class DLLCLIENT CWindComponent final : public BaseEnvWindComponent, public CBaseNetComponent {
		  public:
			CWindComponent(pragma::ecs::BaseEntity &ent) : BaseEnvWindComponent(ent) {}
			virtual void ReceiveData(NetPacket &packet) override;
			virtual void InitializeLuaObject(lua::State *l) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
		};
	};

	class DLLCLIENT CEnvWind : public CBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
