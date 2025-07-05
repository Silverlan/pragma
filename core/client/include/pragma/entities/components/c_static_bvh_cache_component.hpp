// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_STATIC_BVH_CACHE_COMPONENT_HPP__
#define __C_STATIC_BVH_CACHE_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/entities/components/base_static_bvh_cache_component.hpp>
#include <unordered_set>

namespace pragma {
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

#endif
