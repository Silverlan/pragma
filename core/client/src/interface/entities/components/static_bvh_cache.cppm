// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <unordered_set>

export module pragma.client:entities.components.static_bvh_cache;
export namespace pragma {
	class DLLCLIENT CStaticBvhCacheComponent final : public BaseStaticBvhCacheComponent {
	  public:
		CStaticBvhCacheComponent(BaseEntity &ent) : BaseStaticBvhCacheComponent(ent) {}
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void TestRebuildBvh() override;
	  private:
		virtual void DoRebuildBvh() override;
	};
};
