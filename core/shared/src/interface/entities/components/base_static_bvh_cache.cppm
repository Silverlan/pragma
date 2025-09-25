// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/entities/components/base_bvh_component.hpp"
#include "pragma/util/util_thread_pool.hpp"
#include "pragma/util/functional_parallel_worker.hpp"
#include <unordered_set>

export module pragma.shared:entities.components.base_static_bvh_cache;

export namespace pragma {
	class DLLNETWORK BaseStaticBvhCacheComponent : public BaseBvhComponent {
	  public:
		virtual void Initialize() override;
		virtual void OnRemove() override;
		virtual void OnTick(double tDelta) override;

		virtual ~BaseStaticBvhCacheComponent() override;
		void SetCacheDirty();

		void SetEntityDirty(BaseEntity &ent);
		void AddEntity(BaseEntity &ent);
		void RemoveEntity(BaseEntity &ent, bool removeFinal = true);

		virtual bool IntersectionTest(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, HitInfo &outHitInfo) const override;
		using BaseBvhComponent::IntersectionTest;

		virtual bool IsStaticBvh() const override { return true; }
	  protected:
		struct BvhPendingWorkerResult {
			std::queue<std::function<void()>> callOnComplete;
			std::shared_ptr<pragma::bvh::MeshBvhTree> bvhData;
			std::atomic<bool> complete = false;
		};

		BaseStaticBvhCacheComponent(BaseEntity &ent);
		void RemoveEntityFromBvh(const BaseEntity &ent);
		void UpdateBuild();

		void Build(std::vector<std::shared_ptr<ModelSubMesh>> &&meshes, std::vector<BaseEntity *> &&meshToEntity, std::vector<umath::ScaledTransform> &&meshPoses);

		virtual void TestRebuildBvh() = 0;
		bool m_staticBvhDirty = false;
		bool m_bvhInitialized = false; // Was the bvh initialized at least once?
		std::shared_ptr<util::FunctionalParallelWorker> m_buildWorker = nullptr;
		std::unordered_set<BaseStaticBvhUserComponent *> m_entities;
		std::unique_ptr<BvhPendingWorkerResult> m_bvhPendingWorkerResult;
		CallbackHandle m_onEndGame;
		size_t m_currentBvhCacheVersion = 0;
	};
};
