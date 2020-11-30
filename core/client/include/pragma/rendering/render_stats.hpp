#ifndef __PRAGMA_RENDER_STATS_HPP__
#define __PRAGMA_RENDER_STATS_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/baseentity_handle.h"
#include <material.h>
#include <sharedutils/util_weak_handle.hpp>
#include <vector>
#include <chrono>
#include <unordered_set>

namespace prosper {class Shader;};

class CModelSubMesh;
struct DLLCLIENT RenderPassStats
{
	RenderPassStats &operator+(const RenderPassStats &other)
	{
		entities.insert(entities.end(),other.entities.begin(),other.entities.end());
		materials.insert(materials.end(),other.materials.begin(),other.materials.end());
		shaders.insert(shaders.end(),other.shaders.begin(),other.shaders.end());
		meshes.insert(meshes.end(),other.meshes.begin(),other.meshes.end());
		instancedEntities.insert(other.instancedEntities.begin(),other.instancedEntities.end());

		numShaderStateChanges += other.numShaderStateChanges;
		numMaterialStateChanges += other.numMaterialStateChanges;
		numEntityStateChanges += other.numEntityStateChanges;
		numDrawCalls += other.numDrawCalls;
		numDrawnMeshes += other.numDrawnMeshes;
		numDrawnVertices += other.numDrawnVertices;
		numDrawnTrianges += other.numDrawnTrianges;
		numEntityBufferUpdates += other.numEntityBufferUpdates;
		numInstanceSets += other.numInstanceSets;
		numInstanceSetMeshes += other.numInstanceSetMeshes;
		numInstancedMeshes += other.numInstancedMeshes;
		numInstancedSkippedRenderItems += other.numInstancedSkippedRenderItems;
		numEntitiesWithoutInstancing += other.numEntitiesWithoutInstancing;
		renderThreadWaitTime += other.renderThreadWaitTime;
		cpuExecutionTime += other.cpuExecutionTime;
		return *this;
	}
	RenderPassStats &operator+=(const RenderPassStats &other)
	{
		*this = *this +other;
		return *this;
	}
	std::vector<EntityHandle> entities;
	std::vector<MaterialHandle> materials;
	std::vector<util::WeakHandle<prosper::Shader>> shaders;
	std::vector<std::shared_ptr<const CModelSubMesh>> meshes;
	uint32_t numShaderStateChanges = 0;
	uint32_t numMaterialStateChanges = 0;
	uint32_t numEntityStateChanges = 0;
	uint32_t numDrawCalls = 0;
	uint32_t numDrawnMeshes = 0;
	uint64_t numDrawnVertices = 0;
	uint64_t numDrawnTrianges = 0;
	uint32_t numEntityBufferUpdates = 0;
	uint32_t numInstanceSets = 0;
	uint32_t numInstanceSetMeshes = 0;
	std::unordered_set<EntityIndex> instancedEntities;
	uint32_t numInstancedMeshes = 0;
	uint32_t numInstancedSkippedRenderItems = 0;
	uint32_t numEntitiesWithoutInstancing = 0;
	std::chrono::nanoseconds renderThreadWaitTime {0};
	std::chrono::nanoseconds cpuExecutionTime {0};
};

struct DLLCLIENT RenderQueueWorkerStats
{
	RenderQueueWorkerStats &operator+(const RenderQueueWorkerStats &other)
	{
		totalExecutionTime += other.totalExecutionTime;
		numJobs += other.numJobs;
		return *this;
	}
	RenderQueueWorkerStats &operator+=(const RenderQueueWorkerStats &other)
	{
		*this = *this +other;
		return *this;
	}
	std::chrono::nanoseconds totalExecutionTime {0};
	uint32_t numJobs = 0;
};

struct DLLCLIENT RenderQueueBuilderStats
{
	RenderQueueBuilderStats &operator+(const RenderQueueBuilderStats &other)
	{
		totalExecutionTime += other.totalExecutionTime;
		worldQueueUpdateTime += other.worldQueueUpdateTime;
		octreeProcessingTime += other.octreeProcessingTime;
		workerWaitTime += other.workerWaitTime;
		queueSortTime += other.queueSortTime;
		queueInstancingTime += other.queueInstancingTime;
		auto numWorkers = umath::min(workerStats.size(),other.workerStats.size()); // Should be the same if they were executed during the same frame
		for(auto i=decltype(numWorkers){0u};i<numWorkers;++i)
			workerStats[i] += other.workerStats[i];
		return *this;
	}
	RenderQueueBuilderStats &operator+=(const RenderQueueBuilderStats &other)
	{
		*this = *this +other;
		return *this;
	}
	std::chrono::nanoseconds totalExecutionTime {0};
	std::chrono::nanoseconds worldQueueUpdateTime {0};
	std::chrono::nanoseconds octreeProcessingTime {0};
	std::chrono::nanoseconds workerWaitTime {0};
	std::chrono::nanoseconds queueSortTime {0};
	std::chrono::nanoseconds queueInstancingTime {0};

	std::vector<RenderQueueWorkerStats> workerStats;
};

struct DLLCLIENT RenderStats
{
	RenderStats &operator+(const RenderStats &other)
	{
		lightCullingTime += other.lightCullingTime;
		renderQueueBuilderStats += other.renderQueueBuilderStats;
		lightingPass += other.lightingPass;
		lightingPassTranslucent += other.lightingPassTranslucent;
		prepass += other.prepass;
		shadowPass += other.shadowPass;
		return *this;
	}
	RenderStats &operator+=(const RenderStats &other)
	{
		*this = *this +other;
		return *this;
	}
	std::chrono::nanoseconds lightCullingTime {0};
	RenderQueueBuilderStats renderQueueBuilderStats;
	RenderPassStats lightingPass;
	RenderPassStats lightingPassTranslucent;
	RenderPassStats prepass;
	RenderPassStats shadowPass;
};

#endif
