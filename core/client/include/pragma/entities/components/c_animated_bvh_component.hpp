// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_ANIMATED_BVH_COMPONENT_HPP__
#define __C_ANIMATED_BVH_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/entities/components/base_bvh_component.hpp>

namespace pragma {
	struct DLLCLIENT AnimatedBvhData {
		struct DLLCLIENT AnimationBvhData {
			std::vector<Mat4> boneMatrices;
		};
		struct DLLCLIENT MeshData {
			std::vector<Vector3> transformedVerts;
		};
		AnimationBvhData animationBvhData;
		std::vector<std::shared_ptr<ModelSubMesh>> renderMeshes;
		std::vector<uint16_t> renderMeshIndices;
		std::vector<MeshData> meshData;
		std::vector<bvh::Primitive> transformedTris;
		std::condition_variable completeCondition;
		mutable std::mutex completeMutex;
		uint32_t completeCount = 0;
	};
	class DLLCLIENT CAnimatedBvhComponent final : public BaseEntityComponent {
	  public:
		CAnimatedBvhComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void OnRemove() override;
		void SetUpdateLazily(bool updateLazily);
		bool ShouldUpdateLazily() const;
		void RebuildAnimatedBvh(bool force = false);
	  private:
		void RebuildTemporaryBvhData();
		void Clear();
		void Cancel();
		void WaitForCompletion();
		bool IsBusy() const;
		void UpdateDirtyBones();
		void RebuildAnimatedBvh(bool force, const std::vector<bool> *optDirtyBones = nullptr);

		AnimatedBvhData m_animatedBvhData;
		CallbackHandle m_cbOnMatricesUpdated;
		CallbackHandle m_cbOnBvhCleared;
		CallbackHandle m_cbRebuildScheduled;
		CallbackHandle m_cbOnBvhUpdateRequested;
		CallbackHandle m_cbOnBvhRebuilt;
		std::shared_ptr<bvh::MeshBvhTree> m_tmpBvhData = nullptr;
		bool m_rebuildScheduled = false;
		std::atomic<bool> m_cancelled = false;
		std::atomic<bool> m_busy = false;
		std::chrono::steady_clock::time_point m_tStart;
		bool m_updateLazily = false;
		uint32_t m_numJobs = 0;

		std::vector<umath::ScaledTransform> m_prevBonePoses;
		std::vector<bool> m_dirtyBones;
	};
};

#endif
