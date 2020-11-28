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
	std::vector<EntityHandle> entities;
	std::vector<MaterialHandle> materials;
	std::vector<util::WeakHandle<prosper::Shader>> shaders;
	std::vector<std::shared_ptr<const CModelSubMesh>> meshes;
	uint32_t numShaderStateChanges = 0;
	uint32_t numMaterialStateChanges = 0;
	uint32_t numEntityStateChanges = 0;
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
	std::chrono::nanoseconds totalExecutionTime {0};
	uint32_t numJobs = 0;
};

struct DLLCLIENT RenderQueueBuilderStats
{
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
	std::chrono::nanoseconds lightCullingTime {0};
	RenderQueueBuilderStats renderQueueBuilderStats;
	RenderPassStats lightingPass;
	RenderPassStats lightingPassTranslucent;
	RenderPassStats prepass;
	RenderPassStats shadowPass;
};

#endif
