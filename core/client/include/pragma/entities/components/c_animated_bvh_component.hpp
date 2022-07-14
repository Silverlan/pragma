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
			std::vector<Mat4> vertexMatrices;
			std::vector<Vector3> transformedVerts;
		};
		AnimationBvhData animationBvhData;
		std::vector<MeshData> meshData;
		std::vector<BvhTriangle> transformedTris;
		std::condition_variable completeCondition;
		std::mutex completeMutex;
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
		void RebuildAnimatedBvh();
	private:
		void Clear();
		void Cancel();
		void WaitForCompletion();

		AnimatedBvhData m_animatedBvhData;
		CallbackHandle m_cbOnMatricesUpdated;
		CallbackHandle m_cbOnBvhCleared;
		std::atomic<bool> m_cancelled = false;
		uint32_t m_numJobs = 0;
	};
};

#endif
