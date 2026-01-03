// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.prop_base;

export import :entities.base_entity;
export import :entities.components.entity;

export {
	namespace pragma {
		class DLLCLIENT CPropComponent final : public BasePropComponent, public CBaseNetComponent {
		  public:
			CPropComponent(ecs::BaseEntity &ent) : BasePropComponent(ent) {}
			virtual void Initialize() override;
			virtual void ReceiveData(NetPacket &packet) override;
			virtual void OnEntitySpawn() override;
			virtual void InitializeLuaObject(lua::State *l) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
		  protected:
			physics::PhysicsType m_propPhysType = physics::PhysicsType::None;
			physics::MoveType m_propMoveType = physics::MoveType::None;
		};
	};

	class DLLCLIENT CBaseProp : public pragma::ecs::CBaseEntity {
	  protected:
	};
};
