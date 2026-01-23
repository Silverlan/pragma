// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.render_stats;

export import :model.mesh;

export namespace pragma::rendering {
	template<typename TCounter>
	struct BaseSceneStatsCounterList {
		void Increment(TCounter counter, uint32_t n = 1) { counters[math::to_integral(counter)] += n; }

		BaseSceneStatsCounterList<TCounter> &operator+(const BaseSceneStatsCounterList<TCounter> &other)
		{
			auto n = math::to_integral(TCounter::Count);
			for(auto i = decltype(n) {0u}; i < n; ++i)
				counters[i] += other.counters[i];
			return *this;
		}
		BaseSceneStatsCounterList<TCounter> &operator+=(const BaseSceneStatsCounterList<TCounter> &other)
		{
			*this = *this + other;
			return *this;
		}

		uint32_t GetCount(TCounter counter) const { return counters[math::to_integral(counter)]; }
		std::array<uint32_t, math::to_integral(TCounter::Count)> counters {};
	};

	template<typename TTimer>
	struct BaseSceneStatsTimerList {
		~BaseSceneStatsTimerList()
		{
			for(auto &timer : gpuTimers) {
				//if(timer)
				//	pragma::get_cengine()->GetRenderContext().KeepResourceAliveUntilPresentationComplete(timer);
			}
		}
		void SetTime(TTimer timer, std::chrono::nanoseconds s) { timers[math::to_integral(timer)] = s; }
		void AddTime(TTimer timer, std::chrono::nanoseconds s) { timers[math::to_integral(timer)] += s; }
		std::chrono::nanoseconds GetTime(TTimer timer) const
		{
			auto &t = timers[math::to_integral(timer)];
			if(math::to_integral(timer) >= math::to_integral(m_gpuTimerStart) && math::to_integral(timer) < math::to_integral(m_gpuTimerStart) + gpuTimers.size()) {
				auto &gpuTimer = gpuTimers[math::to_integral(timer) - math::to_integral(m_gpuTimerStart)];
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
			auto &t = gpuTimers[math::to_integral(timer)];
			if(t == nullptr)
				return;
			t->Begin(cmdBuffer);
		}
		void EndGpuTimer(TTimer timer, prosper::ICommandBuffer &cmdBuffer)
		{
			auto &t = gpuTimers[math::to_integral(timer)];
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
			auto n = math::to_integral(TTimer::Count);
			for(auto i = decltype(n) {0u}; i < n; ++i)
				timers[i] = GetTime(static_cast<TTimer>(i)) + other.GetTime(static_cast<TTimer>(i));
			return *this;
		}
		BaseSceneStatsTimerList<TTimer> &operator+=(const BaseSceneStatsTimerList<TTimer> &other)
		{
			*this = *this + other;
			return *this;
		}

		mutable std::array<std::chrono::nanoseconds, math::to_integral(TTimer::Count)> timers {};
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
		std::vector<material::MaterialHandle> materials;
		std::vector<util::WeakHandle<prosper::Shader>> shaders;
		std::vector<std::shared_ptr<const geometry::CModelSubMesh>> meshes;

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
			auto numWorkers = math::min(workerStats.size(), other.workerStats.size()); // Should be the same if they were executed during the same frame
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
			GlowPass,

			Count
		};

		enum class RenderStage : uint32_t {
			LightCullingGpu = 0,
			PostProcessingGpu,
			PostProcessingGpuFog,
			PostProcessingGpuDoF,
			PostProcessingGpuBloom,
			PostProcessingGpuGlow,
			PostProcessingGpuToneMapping,
			PostProcessingGpuFxaa,
			PostProcessingGpuSsao,
			RenderSceneGpu,
			RendererGpu,
			UpdateRenderBuffersGpu,
			UpdatePrepassRenderBuffersGpu,
			RenderShadowsGpu,
			RenderParticlesGpu,

			LightCullingCpu,
			PrepassExecutionCpu,
			LightingPassExecutionCpu,
			PostProcessingExecutionCpu,
			UpdateRenderBuffersCpu,
			RenderSceneCpu,

			Count,
			GpuStart = LightCullingGpu,
			GpuEnd = RenderParticlesGpu,
			CpuStart = LightCullingCpu,
			CpuEnd = RenderSceneCpu,
			GpuCount = (GpuEnd - GpuStart) + 1,
			CpuCount = (CpuEnd - CpuStart) + 1
		};

		RenderStats();
		~RenderStats();
		RenderStats &operator+(const RenderStats &other)
		{
			auto n = math::to_integral(RenderPass::Count);
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
		RenderPassStats &GetPassStats(RenderPass rp) { return passes[math::to_integral(rp)]; }
		const RenderPassStats &GetPassStats(RenderPass rp) const { return const_cast<RenderStats *>(this)->GetPassStats(rp); }

		std::shared_ptr<prosper::IQueryPool> queryPool = nullptr;
		uint32_t swapchainImageIndex = 0;
		RenderQueueBuilderStats renderQueueBuilderStats {};
		BaseSceneStatsTimerList<RenderStage> stageTimes;
		std::array<RenderPassStats, math::to_integral(RenderPass::Count)> passes {};
		BaseSceneStatsTimerList<RenderStage> *operator->() { return &stageTimes; }
		const BaseSceneStatsTimerList<RenderStage> *operator->() const { return const_cast<RenderStats *>(this)->operator->(); }
	};
};
