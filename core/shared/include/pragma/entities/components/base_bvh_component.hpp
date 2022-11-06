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

	struct BvhMeshIntersectionInfo;
	struct DLLNETWORK BvhIntersectionInfo
	{
		void Clear();
		std::vector<size_t> primitives;

		BvhMeshIntersectionInfo *GetMeshIntersectionInfo();
	protected:
		bool m_isMeshIntersectionInfo = false;
	};

	struct BvhMeshRange;
	struct DLLNETWORK BvhMeshIntersectionInfo
		: public BvhIntersectionInfo
	{
		BvhMeshIntersectionInfo()
		{
			m_isMeshIntersectionInfo = true;
		}

		// For internal use only
		std::vector<const pragma::BvhMeshRange*> &GetTemporaryMeshRanges() {return m_tmpMeshRanges;}
		std::unordered_set<size_t> &GetTemporarMeshMap() {return m_tmpMeshes;}
	protected:
		std::vector<const pragma::BvhMeshRange*> m_tmpMeshRanges;
		std::unordered_set<size_t> m_tmpMeshes;
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

	struct DLLNETWORK BvhTriangle
	{
		Vector3 p0,e1,e2,n;
		BvhTriangle()=default;
		BvhTriangle(const Vector3 &p0, const Vector3 &p1, const Vector3 &p2)
			: p0(p0), e1(p0 - p1), e2(p2 - p0)
		{
			n = cross(e1,e2);
		}
	};

	struct BvhData;
	class BaseStaticBvhCacheComponent;
	DLLNETWORK std::vector<BvhMeshRange> &get_bvh_mesh_ranges(BvhData &bvhData);
	class DLLNETWORK BaseBvhComponent
		: public BaseEntityComponent
	{
	public:
		static ComponentEventId EVENT_ON_CLEAR_BVH;
		static ComponentEventId EVENT_ON_BVH_UPDATE_REQUESTED;
		static ComponentEventId EVENT_ON_BVH_REBUILT;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager,TRegisterComponentEvent registerEvent);

		virtual void Initialize() override;

		virtual ~BaseBvhComponent() override;
		std::optional<BvhHitInfo> IntersectionTest(
			const Vector3 &origin,const Vector3 &dir,float minDist,float maxDist
		) const;
		virtual bool IntersectionTest(
			const Vector3 &origin,const Vector3 &dir,float minDist,float maxDist,
			BvhHitInfo &outHitInfo
		) const;
		bool IntersectionTestAabb(const Vector3 &min,const Vector3 &max) const;
		bool IntersectionTestAabb(const Vector3 &min,const Vector3 &max,BvhIntersectionInfo &outIntersectionInfo) const;
		bool IntersectionTestKDop(const std::vector<umath::Plane> &planes) const;
		bool IntersectionTestKDop(const std::vector<umath::Plane> &planes,BvhIntersectionInfo &outIntersectionInfo) const;
		void SetStaticCache(BaseStaticBvhCacheComponent *staticCache);
		virtual bool IsStaticBvh() const {return false;}
		const BvhMeshRange *FindPrimitiveMeshInfo(size_t primIdx) const;
		
		void SendBvhUpdateRequestOnInteraction();
		static bool SetVertexData(pragma::BvhData &bvhData,const std::vector<BvhTriangle> &data);
		bool SetVertexData(const std::vector<BvhTriangle> &data);
		void RebuildBvh();
		void ClearBvh();

		// For internal use only
		static std::shared_ptr<pragma::BvhData> RebuildBvh(
			const std::vector<std::shared_ptr<ModelSubMesh>> &meshes,const std::vector<umath::ScaledTransform> *optPoses=nullptr,
			const std::function<bool()> &fIsCancelled=nullptr,std::vector<size_t> *optOutMeshIndices=nullptr
		);
		std::shared_ptr<BvhData> SetBvhData(std::shared_ptr<BvhData> &bvhData);
		bool HasBvhData() const;
	protected:
		BaseBvhComponent(BaseEntity &ent);
		virtual void DoRebuildBvh()=0;
		const std::shared_ptr<BvhData> &GetUpdatedBvh() const;
		std::vector<BvhMeshRange> &GetMeshRanges();
		std::shared_ptr<BvhData> m_bvhData = nullptr;
		ComponentHandle<BaseStaticBvhCacheComponent> m_staticCache;
		mutable std::mutex m_bvhDataMutex;
		bool m_sendBvhUpdateRequestOnInteraction = false;
	};
};

#endif
