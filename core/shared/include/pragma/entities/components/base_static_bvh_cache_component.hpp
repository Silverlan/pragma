/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASE_STATIC_BVH_CACHE_COMPONENT_HPP__
#define __BASE_STATIC_BVH_CACHE_COMPONENT_HPP__

#include "pragma/entities/components/base_bvh_component.hpp"
#include "pragma/util/util_thread_pool.hpp"
#include <unordered_set>

class FunctionalParallelWorker;
namespace pragma {
	class BaseStaticBvhUserComponent;
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

		virtual bool IntersectionTest(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, BvhHitInfo &outHitInfo) const override;
		using BaseBvhComponent::IntersectionTest;

		virtual bool IsStaticBvh() const override { return true; }
	  protected:
		struct BvhPendingWorkerResult {
			std::queue<std::function<void()>> callOnComplete;
			std::shared_ptr<pragma::BvhData> bvhData;
			std::atomic<bool> complete = false;
		};

		BaseStaticBvhCacheComponent(BaseEntity &ent);
		void RemoveEntityFromBvh(const BaseEntity &ent);
		void UpdateBuild();

		void Build(std::vector<std::shared_ptr<ModelSubMesh>> &&meshes, std::vector<BaseEntity *> &&meshToEntity, std::vector<umath::ScaledTransform> &&meshPoses);

		virtual void TestRebuildBvh() = 0;
		bool m_staticBvhDirty = false;
		bool m_bvhInitialized = false; // Was the bvh initialized at least once?
		std::shared_ptr<FunctionalParallelWorker> m_buildWorker = nullptr;
		std::unordered_set<BaseStaticBvhUserComponent *> m_entities;
		std::unique_ptr<BvhPendingWorkerResult> m_bvhPendingWorkerResult;
		CallbackHandle m_onEndGame;
		size_t m_currentBvhCacheVersion = 0;
	};
};

#endif
