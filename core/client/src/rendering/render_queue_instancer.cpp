/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/render_queue_instancer.hpp"
#include "pragma/rendering/render_queue.hpp"
#include "pragma/entities/entity_instance_index_buffer.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/console/c_cvar.h"

using namespace pragma;

extern DLLCLIENT CGame *c_game;

static auto cvInstancingThreshold = GetClientConVar("render_instancing_threshold");
rendering::RenderQueueInstancer::RenderQueueInstancer(pragma::rendering::RenderQueue &renderQueue) : m_renderQueue {renderQueue}, m_instanceThreshold {static_cast<uint32_t>(umath::max(cvInstancingThreshold->GetInt(), 2))} {}

void rendering::RenderQueueInstancer::Process()
{
	uint32_t prevNumMeshes = 0;
	EntityIndex entIndex;
	auto prevHash = CalcNextEntityHash(prevNumMeshes, entIndex);
	m_instantiableEntityList.push_back(entIndex);
	uint32_t numMeshes = 0;
	auto &sortedItemIndices = m_renderQueue.sortedItemIndices;
	while(m_curIndex < sortedItemIndices.size()) {
		auto hash = CalcNextEntityHash(numMeshes, entIndex);
		if(hash != prevHash) // New entity is different; no instantiation possible
		{
			// Process the instantiation list for everything before the current entity
			ProcessInstantiableList(m_curIndex - numMeshes, prevNumMeshes, prevHash);
			m_instantiableEntityList.push_back(entIndex);
			prevHash = hash;
			prevNumMeshes = numMeshes;
			continue;
		}
		m_instantiableEntityList.push_back(entIndex);
	}
	ProcessInstantiableList(m_curIndex, prevNumMeshes, prevHash);
}

void rendering::RenderQueueInstancer::ProcessInstantiableList(uint32_t endIndex, uint32_t numMeshes, util::Hash hash)
{
	auto numInstantiableEntities = m_instantiableEntityList.size();
	if(numInstantiableEntities < m_instanceThreshold) {
		m_instantiableEntityList.clear();
		return;
	}

	std::vector<size_t> sortedInstantiableEntityIndices;
	sortedInstantiableEntityIndices.resize(numInstantiableEntities);
	for(auto i = decltype(numInstantiableEntities) {0u}; i < numInstantiableEntities; ++i)
		sortedInstantiableEntityIndices[i] = i;

	std::sort(sortedInstantiableEntityIndices.begin(), sortedInstantiableEntityIndices.end(), [this](size_t idx0, size_t idx1) { return m_instantiableEntityList[idx0] < m_instantiableEntityList[idx1]; });

	std::vector<pragma::RenderBufferIndex> renderBufferIndices {};
	renderBufferIndices.reserve(numInstantiableEntities);
	for(auto i = decltype(sortedInstantiableEntityIndices.size()) {0u}; i < sortedInstantiableEntityIndices.size(); ++i) {
		auto renderBufferIndex = static_cast<CBaseEntity *>(c_game->GetEntityByLocalIndex(m_instantiableEntityList[i]))->GetRenderComponent()->GetRenderBufferIndex();
		renderBufferIndices.push_back(*renderBufferIndex);

		// Note: Hash has to be in the right order for frame coherence, which is not necessarily the same order as the queue (since the entity index
		// is not included in the sort key)
		hash = util::hash_combine<uint64_t>(hash, static_cast<uint64_t>(m_instantiableEntityList[sortedInstantiableEntityIndices[i]]));
	}

	auto &instanceIndexBuffer = pragma::CSceneComponent::GetEntityInstanceIndexBuffer();
	auto instanceBuf = instanceIndexBuffer->AddInstanceList(m_renderQueue, std::move(renderBufferIndices), hash);
	if(instanceBuf == nullptr)
		return;

	m_renderQueue.instanceSets.push_back({});

	auto setIdx = m_renderQueue.instanceSets.size() - 1;
	auto startIndex = endIndex - (numInstantiableEntities * numMeshes);

	auto &instanceSet = m_renderQueue.instanceSets.back();
	instanceSet.instanceCount = numInstantiableEntities;
	instanceSet.instanceBuffer = instanceBuf;
	instanceSet.meshCount = numMeshes;
	instanceSet.startSkipIndex = startIndex;

	for(auto i = startIndex; i < (startIndex + numMeshes); ++i) {
		auto &item = m_renderQueue.queue[m_renderQueue.sortedItemIndices[i].first];
		item.instanceSetIndex = setIdx;
	}
	if(startIndex + numMeshes < endIndex) {
		// We only need to set the first item after our base instance set to INSTANCED, since all others are skipped by the renderer anyway
		m_renderQueue.queue[m_renderQueue.sortedItemIndices[startIndex + numMeshes].first].instanceSetIndex = pragma::rendering::RenderQueueItem::INSTANCED;
	}
	/*for(auto i=(startIndex +numMeshes);i<endIndex;++i)
	{
		auto &item = m_renderQueue.queue[m_renderQueue.sortedItemIndices[i].first];
		item.instanceSetIndex = pragma::rendering::RenderQueueItem::INSTANCED;
	}*/

	m_instantiableEntityList.clear();
}

util::Hash rendering::RenderQueueInstancer::CalcNextEntityHash(uint32_t &outNumMeshes, EntityIndex &entIndex)
{
	auto &sortedItemIndices = m_renderQueue.sortedItemIndices;
	if(m_curIndex >= sortedItemIndices.size())
		return 0;
	util::Hash hash = 0;
	auto entity = m_renderQueue.queue[sortedItemIndices[m_curIndex].first].entity;
	entIndex = entity;
	auto &renderMeshes = static_cast<CBaseEntity &>(*c_game->GetEntityByLocalIndex(entIndex)).GetRenderComponent()->GetRenderMeshes();
	outNumMeshes = 0;
	while(m_curIndex < sortedItemIndices.size()) {
		auto &sortKey = sortedItemIndices[m_curIndex];
		auto &item = m_renderQueue.queue[sortKey.first];
		if(item.entity != entity)
			break;
		++outNumMeshes;
		++m_curIndex;
		static_assert(sizeof(SortingKey) == sizeof(uint64_t));
		hash = util::hash_combine<uint64_t>(hash, *reinterpret_cast<uint64_t *>(&sortKey.second));
		// TODO: The order of meshes is not guaranteed to be the same every frame!
		hash = util::hash_combine<uint64_t>(hash, reinterpret_cast<uint64_t>(renderMeshes[item.mesh].get()));
	}
	return hash;
}
