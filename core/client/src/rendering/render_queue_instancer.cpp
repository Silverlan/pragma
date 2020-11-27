/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/render_queue_instancer.hpp"
#include "pragma/rendering/render_queue.hpp"
#include "pragma/entities/entity_instance_index_buffer.hpp"
#include "pragma/console/c_cvar.h"

using namespace pragma;

extern DLLCLIENT CGame *c_game;
#pragma optimize("",off)
static auto cvInstancingThreshold = GetClientConVar("render_instancing_threshold");
rendering::RenderQueueInstancer::RenderQueueInstancer(pragma::rendering::RenderQueue &renderQueue)
	: m_renderQueue{renderQueue},m_instanceThreshold{static_cast<uint32_t>(umath::max(cvInstancingThreshold->GetInt(),2))}
{}

void rendering::RenderQueueInstancer::Process()
{
	uint32_t prevNumMeshes = 0;
	EntityIndex entIndex;
	auto prevHash = CalcNextEntityHash(prevNumMeshes,entIndex);
	m_instantiableEntityList.push_back(entIndex);
	uint32_t numMeshes = 0;
	auto &sortedItemIndices = m_renderQueue.sortedItemIndices;
	while(m_curIndex < sortedItemIndices.size())
	{
		auto hash = CalcNextEntityHash(numMeshes,entIndex);
		if(hash != prevHash) // New entity is different; no instantiation possible
		{
			// Process the instantiation list for everything before the current entity
			ProcessInstantiableList(m_curIndex -numMeshes,prevNumMeshes,prevHash);
			m_instantiableEntityList.push_back(entIndex);
			prevHash = hash;
			prevNumMeshes = numMeshes;
			continue;
		}
		m_instantiableEntityList.push_back(entIndex);
	}
	ProcessInstantiableList(m_curIndex,prevNumMeshes,prevHash);
}

void rendering::RenderQueueInstancer::ProcessInstantiableList(uint32_t endIndex,uint32_t numMeshes,util::Hash hash)
{
	auto numInstantiableEntities = m_instantiableEntityList.size();
	if(numInstantiableEntities < m_instanceThreshold)
	{
		m_instantiableEntityList.clear();
		return;
	}

	std::vector<pragma::RenderBufferIndex> renderBufferIndices {};
	renderBufferIndices.reserve(numInstantiableEntities);
	for(auto entIdx : m_instantiableEntityList)
	{
		auto renderBufferIndex = static_cast<CBaseEntity*>(c_game->GetEntityByLocalIndex(entIdx))->GetRenderComponent()->GetRenderBufferIndex();
		renderBufferIndices.push_back(*renderBufferIndex);
	}
		
	auto &instanceIndexBuffer = pragma::CSceneComponent::GetEntityInstanceIndexBuffer();
	auto instanceBuf = instanceIndexBuffer->AddInstanceList(m_renderQueue,std::move(renderBufferIndices),util::hash_combine<uint64_t>(hash,numInstantiableEntities));
	if(instanceBuf == nullptr)
		return;

	m_renderQueue.instanceSets.push_back({});

	auto setIdx = m_renderQueue.instanceSets.size() -1;
	auto startIndex = endIndex -(numInstantiableEntities *numMeshes);

	auto &instanceSet = m_renderQueue.instanceSets.back();
	instanceSet.instanceCount = numInstantiableEntities;
	instanceSet.instanceBuffer = instanceBuf;
	instanceSet.meshCount = numMeshes;
	instanceSet.startSkipIndex = startIndex;

	for(auto i=startIndex;i<(startIndex +numMeshes);++i)
	{
		auto &item = m_renderQueue.queue[m_renderQueue.sortedItemIndices[i].first];
		item.instanceSetIndex = setIdx;
	}
	// TODO: Instanced items are skipped anyway, so technically we don't need this second loop
	for(auto i=(startIndex +numMeshes);i<endIndex;++i)
	{
		auto &item = m_renderQueue.queue[m_renderQueue.sortedItemIndices[i].first];
		item.instanceSetIndex = pragma::rendering::RenderQueueItem::INSTANCED;
	}

	m_instantiableEntityList.clear();
}

util::Hash rendering::RenderQueueInstancer::CalcNextEntityHash(uint32_t &outNumMeshes,EntityIndex &entIndex)
{
	auto &sortedItemIndices = m_renderQueue.sortedItemIndices;
	if(m_curIndex >= sortedItemIndices.size())
		return 0;
	util::Hash hash = 0;
	auto entity = m_renderQueue.queue[sortedItemIndices[m_curIndex].first].entity;
	entIndex = entity;
	outNumMeshes = 0;
	while(m_curIndex < sortedItemIndices.size())
	{
		auto &sortKey = sortedItemIndices[m_curIndex];
		auto &item = m_renderQueue.queue[sortKey.first];
		if(item.entity != entity)
			break;
		++outNumMeshes;
		++m_curIndex;
		hash = util::hash_combine<uint64_t>(hash,*reinterpret_cast<uint64_t*>(&sortKey.second));
	}
	return hash;
}
#pragma optimize("",on)
