/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

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
