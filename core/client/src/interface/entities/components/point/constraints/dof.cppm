// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "definitions.hpp"

export module pragma.client:entities.components.point_constraint_dof;

export import :entities.base_entity;
export import :entities.components.entity;

export {
	namespace pragma {
		class DLLCLIENT CPointConstraintDoFComponent final : public BasePointConstraintDoFComponent, public CBaseNetComponent {
		  public:
			CPointConstraintDoFComponent(pragma::ecs::BaseEntity &ent) : BasePointConstraintDoFComponent(ent) {}
			virtual void ReceiveData(NetPacket &packet) override;
			virtual void InitializeLuaObject(lua::State *l) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
		};
	};

	class DLLCLIENT CPointConstraintDoF : public CBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
