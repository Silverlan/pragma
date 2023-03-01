#ifndef __PRAGMA_RENDER_STATS_HPP__
#define __PRAGMA_RENDER_STATS_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/baseentity_handle.h"
#include "pragma/c_engine.h"
#include <pragma/entities/baseentity.h>
#include <queries/prosper_timer_query.hpp>
#include <material.h>
#include <sharedutils/util_weak_handle.hpp>
#include <vector>
#include <chrono>
#include <unordered_set>

namespace prosper {
	class Shader;
};

template<typename TCounter>
struct BaseSceneStatsCounterList {
	void Increment(TCounter counter, uint32_t n = 1) { counters[umath::to_integral(counter)] += n; }

	BaseSceneStatsCounterList<TCounter> &operator+(const BaseSceneStatsCounterList<TCounter> &other)
	{
		auto n = umath::to_integral(TCounter::Count);
		for(auto i = decltype(n) {0u}; i < n; ++i)
			counters[i] += other.counters[i];
		return *this;
	}
	BaseSceneStatsCounterList<TCounter> &operator+=(const BaseSceneStatsCounterList<TCounter> &other)
	{
		*this = *this + other;
		return *this;
	}

	uint32_t GetCount(TCounter counter) const { return counters[umath::to_integral(counter)]; }
	std::array<uint32_t, umath::to_integral(TCounter::Count)> counters {};
};

template<typename TTimer>
struct BaseSceneStatsTimerList {
	~BaseSceneStatsTimerList()
	{
		for(auto &timer : gpuTimers) {
			if(timer)
				pragma::get_cengine()->GetRenderContext().KeepResourceAliveUntilPresentationComplete(timer);
		}
	}
	void SetTime(TTimer timer, std::chrono::nanoseconds s) { timers[umath::to_integral(timer)] = s; }
	void AddTime(TTimer timer, std::chrono::nanoseconds s) { timers[umath::to_integral(timer)] += s; }
	std::chrono::nanoseconds GetTime(TTimer timer) const
	{
		auto &t = timers[umath::to_integral(timer)];
		if(umath::to_integral(timer) >= umath::to_integral(m_gpuTimerStart) && umath::to_integral(timer) < umath::to_integral(m_gpuTimerStart) + gpuTimers.size()) {
			auto &gpuTimer = gpuTimers[umath::to_integral(timer) - umath::to_integral(m_gpuTimerStart)];
			if(t.count() > 0 || gpuTimer == nullptr)
				return t;
			t = std::chrono::nanoseconds {0};
			gpuTimer->QueryResult(t);
			return t;
		}
		return t;
	}

	void BeginGpuTimer(TTimer timer, prosper::ICommandBuffer &cmdBuffer)
	{
		auto &t = gpuTimers[umath::to_integral(timer)];
		if(t == nullptr)
			return;
		t->Begin(cmdBuffer);
	}
	void EndGpuTimer(TTimer timer, prosper::ICommandBuffer &cmdBuffer)
	{
		auto &t = gpuTimers[umath::to_integral(timer)];
		if(t == nullptr)
			return;
		t->End(cmdBuffer);
	}
	bool IsGPUExecutionTimeAvailable() const
	{
		for(auto &timer : gpuTimers) {
			if(timer && timer->IsResultAvailable() == false)
				return false;
		}
		return true;
	}

	BaseSceneStatsTimerList<TTimer> &operator+(const BaseSceneStatsTimerList<TTimer> &other)
	{
		auto n = umath::to_integral(TTimer::Count);
		for(auto i = decltype(n) {0u}; i < n; ++i)
			timers[i] = GetTime(static_cast<TTimer>(i)) + other.GetTime(static_cast<TTimer>(i));
		return *this;
	}
	BaseSceneStatsTimerList<TTimer> &operator+=(const BaseSceneStatsTimerList<TTimer> &other)
	{
		*this = *this + other;
		return *this;
	}

	mutable std::array<std::chrono::nanoseconds, umath::to_integral(TTimer::Count)> timers {};
	std::vector<std::shared_ptr<prosper::TimerQuery>> gpuTimers;
	void SetGpuTimerStart(TTimer start) { m_gpuTimerStart = start; }
  private:
	TTimer m_gpuTimerStart;
};

template<typename TCounter, typename TTimer>
struct BaseSceneStatsCounterTimerList : public BaseSceneStatsCounterList<TCounter>, BaseSceneStatsTimerList<TTimer> {
	BaseSceneStatsCounterTimerList<TCounter, TTimer> &operator+(const BaseSceneStatsCounterTimerList<TCounter, TTimer> &other)
	{
		BaseSceneStatsCounterList<TCounter>::operator+(other);
		BaseSceneStatsTimerList<TTimer>::operator+(other);
		return *this;
	}
	BaseSceneStatsCounterTimerList<TCounter, TTimer> &operator+=(const BaseSceneStatsCounterTimerList<TCounter, TTimer> &other)
	{
		*this = *this + other;
		return *this;
	}
};

class CModelSubMesh;
struct DLLCLIENT RenderPassStats {
	enum class Counter : uint32_t {
		ShaderStateChanges = 0,
		MaterialStateChanges,
		EntityStateChanges,
		DrawCalls,
		DrawnMeshes,
		DrawnVertices,
		DrawnTriangles,
		EntityBufferUpdates,
		InstanceSets,
		InstanceSetMeshes,
		InstancedMeshes,
		InstancedSkippedRenderItems,
		EntitiesWithoutInstancing,

		Count
	};
	enum class Timer : uint32_t {
		GpuExecution = 0,

		RenderThreadWait,
		CpuExecution,
		MaterialBind,
		EntityBind,
		DrawCall,
		ShaderBind,

		Count,
		GpuStart = GpuExecution,
		GpuEnd = GpuExecution,
		CpuStart = RenderThreadWait,
		CpuEnd = ShaderBind,
		GpuCount = (GpuEnd - GpuStart) + 1,
		CpuCount = (CpuEnd - CpuStart) + 1
	};
	RenderPassStats &operator+(const RenderPassStats &other)
	{
		entities.insert(entities.end(), other.entities.begin(), other.entities.end());
		materials.insert(materials.end(), other.materials.begin(), other.materials.end());
		shaders.insert(shaders.end(), other.shaders.begin(), other.shaders.end());
		meshes.insert(meshes.end(), other.meshes.begin(), other.meshes.end());
		instancedEntities.insert(other.instancedEntities.begin(), other.instancedEntities.end());

		stats += other.stats;
		return *this;
	}
	RenderPassStats &operator+=(const RenderPassStats &other)
	{
		*this = *this + other;
		return *this;
	}
	BaseSceneStatsCounterTimerList<Counter, Timer> *operator->() { return &stats; }
	const BaseSceneStatsCounterTimerList<Counter, Timer> *operator->() const { return const_cast<RenderPassStats *>(this)->operator->(); }
	std::vector<EntityHandle> entities;
	std::vector<msys::MaterialHandle> materials;
	std::vector<util::WeakHandle<prosper::Shader>> shaders;
	std::vector<std::shared_ptr<const CModelSubMesh>> meshes;

	std::unordered_set<EntityIndex> instancedEntities;

	BaseSceneStatsCounterTimerList<Counter, Timer> stats;
};

struct DLLCLIENT RenderQueueWorkerStats {
	RenderQueueWorkerStats &operator+(const RenderQueueWorkerStats &other)
	{
		totalExecutionTime += other.totalExecutionTime;
		numJobs += other.numJobs;
		return *this;
	}
	RenderQueueWorkerStats &operator+=(const RenderQueueWorkerStats &other)
	{
		*this = *this + other;
		return *this;
	}
	std::chrono::nanoseconds totalExecutionTime {0};
	uint32_t numJobs = 0;
};

struct DLLCLIENT RenderQueueBuilderStats {
	RenderQueueBuilderStats &operator+(const RenderQueueBuilderStats &other)
	{
		timers += other.timers;
		auto numWorkers = umath::min(workerStats.size(), other.workerStats.size()); // Should be the same if they were executed during the same frame
		for(auto i = decltype(numWorkers) {0u}; i < numWorkers; ++i)
			workerStats[i] += other.workerStats[i];
		return *this;
	}
	RenderQueueBuilderStats &operator+=(const RenderQueueBuilderStats &other)
	{
		*this = *this + other;
		return *this;
	}
	enum class Timer : uint32_t {
		TotalExecution = 0,
		WorldQueueUpdate,
		OctreeProcessing,
		WorkerWait,
		QueueSort,
		QueueInstancing,

		Count
	};
	BaseSceneStatsTimerList<Timer> timers;
	BaseSceneStatsTimerList<Timer> *operator->() { return &timers; }
	const BaseSceneStatsTimerList<Timer> *operator->() const { return const_cast<RenderQueueBuilderStats *>(this)->operator->(); }

	std::vector<RenderQueueWorkerStats> workerStats {};
};

struct DLLCLIENT RenderStats {
	enum class RenderPass : uint32_t {
		LightingPass = 0,
		LightingPassTranslucent,
		Prepass,
		ShadowPass,

		Count
	};

	enum class RenderStage : uint32_t {
		LightCullingGpu = 0,
		PostProcessingGpu,
		PostProcessingGpuFog,
		PostProcessingGpuDoF,
		PostProcessingGpuBloom,
		PostProcessingGpuToneMapping,
		PostProcessingGpuFxaa,
		PostProcessingGpuSsao,

		LightCullingCpu,
		PrepassExecutionCpu,
		LightingPassExecutionCpu,
		PostProcessingExecutionCpu,
		UpdateRenderBuffersCpu,

		Count,
		GpuStart = LightCullingGpu,
		GpuEnd = PostProcessingGpuSsao,
		CpuStart = LightCullingCpu,
		CpuEnd = UpdateRenderBuffersCpu,
		GpuCount = (GpuEnd - GpuStart) + 1,
		CpuCount = (CpuEnd - CpuStart) + 1
	};

	RenderStats();
	~RenderStats();
	RenderStats &operator+(const RenderStats &other)
	{
		auto n = umath::to_integral(RenderPass::Count);
		for(auto i = decltype(n) {0u}; i < n; ++i)
			passes[i] += other.passes[i];

		stageTimes += other.stageTimes;
		renderQueueBuilderStats += other.renderQueueBuilderStats;
		return *this;
	}
	RenderStats &operator+=(const RenderStats &other)
	{
		*this = *this + other;
		return *this;
	}
	RenderPassStats &GetPassStats(RenderPass rp) { return passes[umath::to_integral(rp)]; }
	const RenderPassStats &GetPassStats(RenderPass rp) const { return const_cast<RenderStats *>(this)->GetPassStats(rp); }

	std::shared_ptr<prosper::IQueryPool> queryPool = nullptr;
	uint32_t swapchainImageIndex = 0;
	RenderQueueBuilderStats renderQueueBuilderStats {};
	BaseSceneStatsTimerList<RenderStage> stageTimes;
	std::array<RenderPassStats, umath::to_integral(RenderPass::Count)> passes {};
	BaseSceneStatsTimerList<RenderStage> *operator->() { return &stageTimes; }
	const BaseSceneStatsTimerList<RenderStage> *operator->() const { return const_cast<RenderStats *>(this)->operator->(); }
};

#endif
