// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"



export module pragma.client:entities.components.point_constraint_fixed;

export import :entities.base_entity;
export import :entities.components.entity;

export {
	namespace pragma {
		class DLLCLIENT CPointConstraintFixedComponent final : public BasePointConstraintFixedComponent, public CBaseNetComponent {
		public:
			CPointConstraintFixedComponent(pragma::ecs::BaseEntity &ent) : BasePointConstraintFixedComponent(ent) {}
			virtual void ReceiveData(NetPacket &packet) override;
			virtual void InitializeLuaObject(lua_State *l) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
		};
	};

	class DLLCLIENT CPointConstraintFixed : public CBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
