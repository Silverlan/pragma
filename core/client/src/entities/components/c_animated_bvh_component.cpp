/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_animated_bvh_component.hpp"
#include "pragma/entities/components/c_bvh_component.hpp"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/model/c_modelmesh.h"
#include <pragma/entities/entity_component_system_t.hpp>

extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

static std::unique_ptr<pragma::ThreadPool> g_threadPool = nullptr;
static uint32_t g_instanceCount = 0;

static pragma::ThreadPool &get_thread_pool() {return *g_threadPool;}
static void init_thread_pool() {g_threadPool = std::make_unique<pragma::ThreadPool>(8,"bvh_animated");}
static void free_thread_pool() {g_threadPool = nullptr;}

#pragma optimize("",off)
CAnimatedBvhComponent::CAnimatedBvhComponent(BaseEntity &ent)
	: BaseEntityComponent(ent)
{
	if(g_instanceCount++ == 0)
		init_thread_pool();
}
void CAnimatedBvhComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l);}

void CAnimatedBvhComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	
	auto animC = GetEntity().GetComponent<CAnimatedComponent>();
	if(animC.valid())
	{
		m_cbOnMatricesUpdated = animC->AddEventCallback(CAnimatedComponent::EVENT_ON_BONE_MATRICES_UPDATED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
			RebuildAnimatedBvh();
			return util::EventReply::Unhandled;
		});
		animC->SetSkeletonUpdateCallbacksEnabled(true);
	}

	auto bvhC = GetEntity().GetComponent<CBvhComponent>();
	if(bvhC.valid())
	{
		m_cbOnBvhCleared = bvhC->AddEventCallback(CBvhComponent::EVENT_ON_CLEAR_BVH,
			[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
			Clear();
			return util::EventReply::Unhandled;
		});
	}
}

void CAnimatedBvhComponent::Clear()
{
	Cancel();
	WaitForCompletion();
}

void CAnimatedBvhComponent::OnRemove()
{
	Clear();
	
	if(m_cbOnMatricesUpdated.IsValid())
		m_cbOnMatricesUpdated.Remove();
	if(m_cbOnBvhCleared.IsValid())
		m_cbOnBvhCleared.Remove();


	auto animC = GetEntity().GetComponent<CAnimatedComponent>();
	if(animC.valid())
		animC->SetSkeletonUpdateCallbacksEnabled(false);

	BaseEntityComponent::OnRemove();
	if(--g_instanceCount == 0)
		free_thread_pool();
}

void CAnimatedBvhComponent::Cancel()
{
	m_cancelled = true;
}
void CAnimatedBvhComponent::WaitForCompletion()
{
	std::unique_lock<std::mutex> lock {m_animatedBvhData.completeMutex};
	m_animatedBvhData.completeCondition.wait(lock,[this]() {
		return m_animatedBvhData.completeCount == m_numJobs;
	});
}

void CAnimatedBvhComponent::RebuildAnimatedBvh()
{
	// TODO: If already updating, mark for update, but wait for current work to be completed
	Clear();
	m_numJobs = 0;
	m_animatedBvhData.completeCount = 0;
	m_cancelled = false;

	auto *animC = static_cast<CAnimatedComponent*>(GetEntity().GetAnimatedComponent().get());
	auto *mdlC = static_cast<CModelComponent*>(GetEntity().GetModelComponent());
	if(!animC || !mdlC)
		return;
	
	// Need to copy the current bone matrices
	auto &animBvhData = m_animatedBvhData.animationBvhData;
	animBvhData.boneMatrices = animC->GetBoneMatrices();
	
	auto &renderMeshes = mdlC->GetRenderMeshes();
	auto &pool = get_thread_pool();

	// Prepare mesh data
	auto &meshDatas = m_animatedBvhData.meshData;
	meshDatas.resize(renderMeshes.size());
	constexpr uint32_t numVerticesPerBatch = 500;
	uint32_t &numJobs = m_numJobs;
	numJobs = 0;
	size_t numIndices = 0;
	for(auto &renderMesh : renderMeshes)
	{
		auto numVerts = renderMesh->GetVertexCount();
		numJobs += numVerts /numVerticesPerBatch;
		if((numVerts %numVerticesPerBatch) > 0)
			++numJobs;
		numIndices += renderMesh->GetIndexCount();
	}
	m_animatedBvhData.transformedTris.resize(numIndices /3);

	auto bvhC = GetEntity().GetComponent<CBvhComponent>();
	auto finalize = [this,bvhC,&animBvhData,&renderMeshes]() mutable -> pragma::ThreadPool::ResultHandler {
		size_t indexOffset = 0;
		uint32_t meshIdx = 0;
		for(auto &renderMesh : renderMeshes)
		{
			renderMesh->VisitIndices([this,meshIdx,&indexOffset](auto *indexDataSrc,uint32_t numIndicesSrc) {
				auto &verts = m_animatedBvhData.meshData[meshIdx].transformedVerts;
				for(auto i=decltype(numIndicesSrc){0};i<numIndicesSrc;i+=3)
					m_animatedBvhData.transformedTris[(indexOffset +i) /3] = {verts[indexDataSrc[i]],verts[indexDataSrc[i +1]],verts[indexDataSrc[i +2]]};
				indexOffset += numIndicesSrc;
			});
			++meshIdx;
		}

		// TODO: Add thread safety
		bvhC->SetVertexData(m_animatedBvhData.transformedTris);
		return {};
	};
	for(auto i=decltype(renderMeshes.size()){0u};i<renderMeshes.size();++i)
	{
		auto &mesh = *renderMeshes[i];
		auto numVerts = mesh.GetVertexCount();
		auto &meshData = meshDatas[i];
		meshData.vertexMatrices.resize(numVerts);
		meshData.transformedVerts.resize(numVerts);

		pool.BatchProcess(numVerts,numVerticesPerBatch,
			[this,numJobs,&mesh,&meshData,&animBvhData,finalize](uint32_t start,uint32_t end) mutable -> pragma::ThreadPool::ResultHandler {
			auto &verts = mesh.GetVertices();
			auto &vertexWeights = mesh.GetVertexWeights();
			auto &vertexMatrices = meshData.vertexMatrices;
			auto &transformedVerts = meshData.transformedVerts;
			for(auto i=start;i<end;++i)
			{
				if(m_cancelled)
					break;
				auto &v = verts[i];
				auto &vw = vertexWeights[i];
				Mat4 mat {0.f};
				for(auto i=0u;i<4u;++i)
				{
					auto boneId = vw.boneIds[i];
					if(boneId == -1)
						continue;
					auto weight = vw.weights[i];
					mat += weight *animBvhData.boneMatrices[boneId];
				}

				Vector4 vpos {v.position.x,v.position.y,v.position.z,1.f};
				vpos = vertexMatrices[i] *vpos;
				transformedVerts[i] = Vector3{vpos.x,vpos.y,vpos.z} /vpos.w;
			}
			m_animatedBvhData.completeMutex.lock();
				if(++m_animatedBvhData.completeCount == numJobs)
				{
					finalize();
					m_animatedBvhData.completeCondition.notify_one();
				}
			m_animatedBvhData.completeMutex.unlock();
			return {};
		});
	}
}
#pragma optimize("",on)
