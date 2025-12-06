// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "definitions.hpp"

export module pragma.client:entities.components.prop_base;

export import :entities.base_entity;
export import :entities.components.entity;

export {
	namespace pragma {
		class DLLCLIENT CPropComponent final : public BasePropComponent, public CBaseNetComponent {
		  public:
			CPropComponent(pragma::ecs::BaseEntity &ent) : BasePropComponent(ent) {}
			virtual void Initialize() override;
			virtual void ReceiveData(NetPacket &packet) override;
			virtual void OnEntitySpawn() override;
			virtual void InitializeLuaObject(lua::State *l) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
		  protected:
			pragma::physics::PhysicsType m_propPhysType = pragma::physics::PhysicsType::None;
			pragma::physics::MoveType m_propMoveType = pragma::physics::MoveType::None;
		};
	};

	class DLLCLIENT CBaseProp : public CBaseEntity {
	  protected:
	};
};
