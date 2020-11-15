/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/render_queue.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"

using namespace pragma::rendering;

std::shared_ptr<RenderQueue> RenderQueue::Create() {return std::shared_ptr<RenderQueue>{new RenderQueue{}};}

RenderQueue::RenderQueue()
{}

void RenderQueue::Reserve()
{
	if(queue.size() < queue.capacity())
		return;
	auto newCapacity = queue.size() *1.1 +100;
	queue.reserve(newCapacity);
	sortedItemIndices.reserve(newCapacity);
}
void RenderQueue::Clear()
{
	queue.clear();
	sortedItemIndices.clear();
}
void RenderQueue::Add(CBaseEntity &ent,RenderMeshIndex meshIdx,CMaterial &mat,pragma::ShaderTextured3DBase &shader)
{
	Reserve();
	queue.push_back({});
	auto &item = queue.back();
	item.material = mat.GetIndex();
	item.shader = shader.GetIndex();
	item.entity = ent.GetLocalIndex();
	item.mesh = meshIdx;
	item.sortingKey.material = item.material;
	item.sortingKey.shader = item.shader;
	item.sortingKey.translucent = 0;
	sortedItemIndices.push_back({queue.size() -1,item.sortingKey});
}
void RenderQueue::Add(const RenderQueueItem &item)
{
	Reserve();
	queue.push_back(item);
	sortedItemIndices.push_back({queue.size() -1,item.sortingKey});
}
void RenderQueue::Sort()
{
	std::sort(sortedItemIndices.begin(),sortedItemIndices.end(),[](const RenderQueueItemSortPair &a,const RenderQueueItemSortPair &b) {
		static_assert(sizeof(decltype(a.second)) == sizeof(uint64_t));
		return *reinterpret_cast<const uint64_t*>(&a.second) < *reinterpret_cast<const uint64_t*>(&b.second);
	});
}
