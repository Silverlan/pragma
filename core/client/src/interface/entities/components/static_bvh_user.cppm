// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <unordered_set>

export module pragma.client:entities.components.static_bvh_user;
export namespace pragma {
	class DLLCLIENT CStaticBvhUserComponent final : public BaseStaticBvhUserComponent {
	  public:
		CStaticBvhUserComponent(BaseEntity &ent) : BaseStaticBvhUserComponent(ent) {}
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void OnEntityComponentRemoved(BaseEntityComponent &component) override;
	};
};
