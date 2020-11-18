/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __RENDER_QUEUE_HPP__
#define __RENDER_QUEUE_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/model/c_modelmesh.h"
#include <cmaterialmanager.h>
#include <shader/prosper_shader.hpp>

namespace pragma::rendering
{
	struct SortingKey
	{
		uint64_t distance : 32, shader : 16, material : 16;
		void SetDistance(const Vector3 &origin,const CCameraComponent &cam);
	};
	struct RenderQueueItem
	{
		MaterialIndex material;
		prosper::ShaderIndex shader;
		EntityIndex entity;
		pragma::RenderMeshIndex mesh;
		// float distance;
		SortingKey sortingKey;
	};

	// using SortingKey = uint32_t;
	using RenderQueueItemIndex = uint32_t;
	using RenderQueueItemSortPair = std::pair<RenderQueueItemIndex,SortingKey>;
	using RenderQueueSortList = std::vector<RenderQueueItemSortPair>;
	class DLLCLIENT RenderQueue
		: public std::enable_shared_from_this<RenderQueue>
	{
	public:
		static std::shared_ptr<RenderQueue> Create();
		void Clear();
		void Reserve();
		void Add(const RenderQueueItem &item);
		void Add(CBaseEntity &ent,RenderMeshIndex meshIdx,CMaterial &mat,pragma::ShaderTextured3DBase &shader,const CCameraComponent *optCam=nullptr);
		void Sort();
		void Merge(const RenderQueue &other);
		std::vector<RenderQueueItem> queue;
		RenderQueueSortList sortedItemIndices;
	private:
		RenderQueue();
	};
};

#endif
