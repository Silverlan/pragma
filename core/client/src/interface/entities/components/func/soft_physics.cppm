// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.func_soft_physics;

export import :entities.base_entity;

export {
	namespace pragma {
		class DLLCLIENT CFuncSoftPhysicsComponent final : public BaseFuncSoftPhysicsComponent {
		  public:
			CFuncSoftPhysicsComponent(ecs::BaseEntity &ent) : BaseFuncSoftPhysicsComponent(ent) {}
			virtual void Initialize() override;
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLCLIENT CFuncSoftPhysics : public pragma::ecs::CBaseEntity {
	  public:
		virtual void Initialize() override;
	};
}
