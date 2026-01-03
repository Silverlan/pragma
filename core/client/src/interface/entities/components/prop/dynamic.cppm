// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.prop_dynamic;

export import :entities.base_entity;

export {
	namespace pragma {
		class DLLCLIENT CPropDynamicComponent final : public BasePropDynamicComponent {
		  public:
			CPropDynamicComponent(ecs::BaseEntity &ent) : BasePropDynamicComponent(ent) {}
			virtual void Initialize() override;
			using BasePropDynamicComponent::BasePropDynamicComponent;
			virtual void InitializeLuaObject(lua::State *l) override;
		  protected:
			virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		};
	};

	class DLLCLIENT CPropDynamic : public pragma::ecs::CBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
