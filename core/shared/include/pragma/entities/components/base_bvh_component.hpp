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
		EntityHandle entity;
		size_t primitiveIndex;
		float distance;
		float t;
		float u;
		float v;
	};

	struct DLLNETWORK BvhMeshRange
	{
		BaseEntity *entity = nullptr;
		std::shared_ptr<ModelSubMesh> mesh;
		size_t start;
		size_t end;
		bool operator<(const BvhMeshRange &other) const
		{
			return start < other.start;
		}
	};

	struct BvhData;
	class BaseStaticBvhCacheComponent;
	DLLNETWORK std::vector<BvhMeshRange> &get_bvh_mesh_ranges(BvhData &bvhData);
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
		void SetStaticCache(BaseStaticBvhCacheComponent *staticCache);
		virtual bool IsStaticBvh() const {return false;}
	protected:
		BaseBvhComponent(BaseEntity &ent);
		void RebuildAnimatedBvh();
		void RebuildBvh();
		std::shared_ptr<pragma::BvhData> RebuildBvh(
			const std::vector<std::shared_ptr<ModelSubMesh>> &meshes,const std::vector<umath::ScaledTransform> *optPoses=nullptr,
			util::BaseParallelWorker *optWorker=nullptr
		);
		virtual void DoRebuildBvh()=0;
		std::vector<BvhMeshRange> &GetMeshRanges();
		void ClearBvh();
		std::shared_ptr<BvhData> m_bvhData = nullptr;
		ComponentHandle<BaseStaticBvhCacheComponent> m_staticCache;
	};
};

#endif
