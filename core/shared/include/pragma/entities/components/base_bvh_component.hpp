/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASE_BVH_COMPONENT_HPP__
#define __BASE_BVH_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"

namespace pragma
{
	struct DLLNETWORK BvhHitInfo
	{
		std::shared_ptr<ModelSubMesh> mesh;
		size_t primitiveIndex;
		float distance;
		float t;
		float u;
		float v;
	};

	struct BvhData;
	class DLLNETWORK BaseBvhComponent
		: public BaseEntityComponent
	{
	public:
		virtual void Initialize() override;

		virtual ~BaseBvhComponent() override;
		std::optional<BvhHitInfo> IntersectionTest(
			const Vector3 &origin,const Vector3 &dir,float minDist,float maxDist
		) const;
		virtual bool IntersectionTest(
			const Vector3 &origin,const Vector3 &dir,float minDist,float maxDist,
			BvhHitInfo &outHitInfo
		) const;
	protected:
		BaseBvhComponent(BaseEntity &ent);
		void RebuildAnimatedBvh();
		bool RebuildBvh(const std::vector<std::shared_ptr<ModelSubMesh>> &meshes);
		void ClearBvh();
		std::unique_ptr<BvhData> m_bvhData = nullptr;
	};
};

#endif
