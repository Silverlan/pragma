// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.prop_physics;

export import :entities.base_entity;
export import :entities.components.entity;

export {
	namespace pragma {
		class DLLCLIENT CPropPhysicsComponent final : public BasePropPhysicsComponent {
		  public:
			CPropPhysicsComponent(ecs::BaseEntity &ent) : BasePropPhysicsComponent(ent) {}
			virtual void Initialize() override;
			virtual void InitializeLuaObject(lua::State *l) override;
		  protected:
			virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		};
	};

	class DLLCLIENT CPropPhysics : public pragma::ecs::CBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
