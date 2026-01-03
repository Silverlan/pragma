// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.base_bvh;

export import :entities.components.base;
export import :entities.components.bvh_data;
export import :entities.components.intersection_handler;
export import :model.model_mesh;

export namespace pragma {
	class BaseStaticBvhCacheComponent;
	namespace baseBvhComponent {
		REGISTER_COMPONENT_EVENT(EVENT_ON_CLEAR_BVH)
		REGISTER_COMPONENT_EVENT(EVENT_ON_BVH_UPDATE_REQUESTED)
		REGISTER_COMPONENT_EVENT(EVENT_ON_BVH_REBUILT)
	}
	class DLLNETWORK BaseBvhComponent : public BaseEntityComponent {
	  public:
		static bool ShouldConsiderMesh(const geometry::ModelSubMesh &mesh);
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		virtual void Initialize() override;
		virtual void OnRemove() override;

		virtual ~BaseBvhComponent() override;
		virtual bool IntersectionTest(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, HitInfo &outHitInfo) const;
		bool IntersectionTestAabb(const Vector3 &min, const Vector3 &max) const;
		bool IntersectionTestAabb(const Vector3 &min, const Vector3 &max, IntersectionInfo &outIntersectionInfo) const;
		bool IntersectionTestKDop(const std::vector<math::Plane> &planes) const;
		bool IntersectionTestKDop(const std::vector<math::Plane> &planes, IntersectionInfo &outIntersectionInfo) const;
		void SetStaticCache(BaseStaticBvhCacheComponent *staticCache);
		virtual bool IsStaticBvh() const { return false; }
		const bvh::MeshRange *FindPrimitiveMeshInfo(size_t primIdx) const;

		void SendBvhUpdateRequestOnInteraction();
		static bool SetVertexData(bvh::MeshBvhTree &bvhData, const std::vector<bvh::Primitive> &data);
		static void DeleteRange(bvh::MeshBvhTree &bvhData, size_t start, size_t end);
		bool SetVertexData(const std::vector<bvh::Primitive> &data);
		void GetVertexData(std::vector<bvh::Primitive> &outData) const;
		void RebuildBvh();
		void ClearBvh();
		std::optional<Vector3> GetVertex(size_t idx) const;
		size_t GetTriangleCount() const;

		void DebugPrint();
		void DebugDraw();
		void DebugDrawBvhTree(const Vector3 &origin, const Vector3 &dir, float maxDist, float duration = 12.f) const;

		// For internal use only
		struct DLLNETWORK BvhBuildInfo {
			const std::vector<math::ScaledTransform> *poses = nullptr;
			std::function<bool()> isCancelled = nullptr;
			std::function<bool(const geometry::ModelSubMesh &, uint32_t)> shouldConsiderMesh = nullptr;
		};
		static std::shared_ptr<bvh::MeshBvhTree> RebuildBvh(const std::vector<std::shared_ptr<geometry::ModelSubMesh>> &meshes, const BvhBuildInfo *optBvhBuildInfo = nullptr, std::vector<size_t> *optOutMeshIndices = nullptr, ecs::BaseEntity *ent = nullptr);
		std::shared_ptr<bvh::MeshBvhTree> SetBvhData(std::shared_ptr<bvh::MeshBvhTree> &bvhData);
		bool HasBvhData() const;
	  protected:
		BaseBvhComponent(ecs::BaseEntity &ent);
		virtual void DoRebuildBvh() = 0;
		const std::shared_ptr<bvh::MeshBvhTree> &GetUpdatedBvh() const;
		std::vector<bvh::MeshRange> &GetMeshRanges();
		std::shared_ptr<bvh::MeshBvhTree> m_bvhData = nullptr;
		ComponentHandle<BaseStaticBvhCacheComponent> m_staticCache;
		mutable std::mutex m_bvhDataMutex;
		bool m_sendBvhUpdateRequestOnInteraction = false;
	};
};
