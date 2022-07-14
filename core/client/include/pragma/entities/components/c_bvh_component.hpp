/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_BVH_COMPONENT_HPP__
#define __C_BVH_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/entities/components/base_bvh_component.hpp>
#include <unordered_set>

namespace pragma
{
	class DLLCLIENT CBvhComponent final
		: public BaseBvhComponent
	{
	public:
		CBvhComponent(BaseEntity &ent) : BaseBvhComponent(ent) {}
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void OnRemove() override;
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void OnEntityComponentRemoved(BaseEntityComponent &component) override;
		virtual bool IntersectionTest(
			const Vector3 &origin,const Vector3 &dir,float minDist,float maxDist,
			BvhHitInfo &outHitInfo
		) const override;
		using BaseBvhComponent::IntersectionTest;
	private:
		void UpdateBvhStatus();
		virtual void DoRebuildBvh() override;
	};
};

#endif
