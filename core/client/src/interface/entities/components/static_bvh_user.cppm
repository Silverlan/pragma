// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.static_bvh_user;

export import pragma.shared;

export namespace pragma {
	class DLLCLIENT CStaticBvhUserComponent final : public BaseStaticBvhUserComponent {
	  public:
		CStaticBvhUserComponent(ecs::BaseEntity &ent) : BaseStaticBvhUserComponent(ent) {}
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void OnEntityComponentRemoved(BaseEntityComponent &component) override;
	};
};
