/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_ANIMATED_BVH_COMPONENT_HPP__
#define __C_ANIMATED_BVH_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/entities/components/base_bvh_component.hpp>

namespace pragma
{
	struct DLLCLIENT AnimatedBvhData
	{
		struct DLLCLIENT AnimationBvhData
		{
			std::vector<Mat4> boneMatrices;
		};
		struct DLLCLIENT MeshData
		{
			std::vector<Vector3> transformedVerts;
		};
		AnimationBvhData animationBvhData;
		std::vector<std::shared_ptr<ModelSubMesh>> renderMeshes;
		std::vector<MeshData> meshData;
		std::vector<BvhTriangle> transformedTris;
		std::condition_variable completeCondition;
		mutable std::mutex completeMutex;
		uint32_t completeCount = 0;
	};
	class DLLCLIENT CAnimatedBvhComponent final
		: public BaseEntityComponent
	{
	public:
		CAnimatedBvhComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void OnRemove() override;
		void SetUpdateLazily(bool updateLazily);
		bool ShouldUpdateLazily() const;
		void RebuildAnimatedBvh(bool force=false);
	private:
		void RebuildTemporaryBvhData();
		void Clear();
		void Cancel();
		void WaitForCompletion();
		bool IsBusy() const;

		AnimatedBvhData m_animatedBvhData;
		CallbackHandle m_cbOnMatricesUpdated;
		CallbackHandle m_cbOnBvhCleared;
		CallbackHandle m_cbRebuildScheduled;
		CallbackHandle m_cbOnBvhUpdateRequested;
		CallbackHandle m_cbOnBvhRebuilt;
		std::shared_ptr<BvhData> m_tmpBvhData = nullptr;
		bool m_rebuildScheduled = false;
		std::atomic<bool> m_cancelled = false;
		std::atomic<bool> m_busy = false;
		bool m_updateLazily = false;
		uint32_t m_numJobs = 0;
	};
};

#endif
