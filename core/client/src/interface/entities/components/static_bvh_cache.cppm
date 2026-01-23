// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.static_bvh_cache;

export import pragma.shared;

export namespace pragma {
	class DLLCLIENT CStaticBvhCacheComponent final : public BaseStaticBvhCacheComponent {
	  public:
		CStaticBvhCacheComponent(ecs::BaseEntity &ent) : BaseStaticBvhCacheComponent(ent) {}
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual void TestRebuildBvh() override;
	  private:
		virtual void DoRebuildBvh() override;
	};
};
