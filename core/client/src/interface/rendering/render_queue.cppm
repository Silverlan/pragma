// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.render_queue;

export import :entities.components.camera;
export import :model.mesh;
export import :model.render_mesh_group;
export import :rendering.render_stats;

export namespace pragma::rendering {
	struct SortingKey {
		// Note: Order is important!
		// Distance should *not* be set unless necessary (e.g. translucent geometry),
		// otherwise instancing effectiveness will be reduced
		SortingKey() = default;
		SortingKey(material::MaterialIndex material, prosper::ShaderIndex shader, bool instantiable, bool translucentKey);
		union {
			struct {
				// Note: Distance is currently unused and could be used for other purposes in the future.
				// Technically sorting by distance could provide a very minor performance boost, but due to the
				// depth prepass overdraw isn't much of an issue and it would also introduce the additional cost for the
				// distance calculations.
				uint64_t instantiable : 1, distance : 32, material : 16, shader : 15; // Least significant to most significant
			} opaque;
			struct {
				uint64_t instantiable : 1, material : 16, shader : 15, distance : 32; // Least significant to most significant
			} translucent;
		};

		void SetDistance(const Vector3 &origin, const CCameraComponent &cam);
		void SetDistance(double distanceSqr, const CCameraComponent &cam);
	};
	struct RenderQueueItem {
		struct TranslucencyPassInfo {
			TranslucencyPassInfo(const CCameraComponent &camera) : camera {camera} {}
			const CCameraComponent &camera;
			std::optional<double> distanceOverrideSqr {};
		};
		static auto constexpr INSTANCED = std::numeric_limits<uint16_t>::max();
		static auto constexpr UNIQUE = std::numeric_limits<uint16_t>::max() - 1;
		RenderQueueItem() = default;
		RenderQueueItem(ecs::CBaseEntity &ent, RenderMeshIndex meshIdx, material::CMaterial &mat, prosper::PipelineID pipelineId, const TranslucencyPassInfo *optTranslucencyPassInfo = nullptr);

		material::CMaterial *GetMaterial() const;
		ecs::CBaseEntity *GetEntity() const;
		geometry::CModelSubMesh *GetMesh() const;
		prosper::ShaderGraphics *GetShader(uint32_t &outPipelineIndex) const;

		material::MaterialIndex material;
		prosper::PipelineID pipelineId = std::numeric_limits<prosper::PipelineID>::max();
		EntityIndex entity;
		RenderMeshIndex mesh;
		SortingKey sortingKey;
		bool translucentKey;

		uint16_t instanceSetIndex;
	};

	// using SortingKey = uint32_t;
	using RenderQueueItemIndex = uint32_t;
	using RenderQueueItemSortPair = std::pair<RenderQueueItemIndex, SortingKey>;
	using RenderQueueSortList = std::vector<RenderQueueItemSortPair>;
	class DLLCLIENT RenderQueue : public std::enable_shared_from_this<RenderQueue> {
	  public:
		struct InstanceSet {
			uint32_t instanceCount;
			std::shared_ptr<prosper::IBuffer> instanceBuffer;

			uint32_t meshCount;
			uint32_t startSkipIndex;
			uint32_t GetSkipCount() const { return instanceCount * meshCount; }
		};
		static std::shared_ptr<RenderQueue> Create(std::string name);
		~RenderQueue();
		void Clear();
		void Reserve();
		void Add(const std::vector<RenderQueueItem> &items);
		void Add(const RenderQueueItem &item);
		void Add(ecs::CBaseEntity &ent, RenderMeshIndex meshIdx, material::CMaterial &mat, prosper::PipelineID pipelineId, const CCameraComponent *optCam = nullptr);
		void Sort();
		void Merge(const RenderQueue &other);
		const std::string &GetName() const { return m_name; }
		std::vector<RenderQueueItem> queue;
		RenderQueueSortList sortedItemIndices;
		std::vector<InstanceSet> instanceSets;

		void Lock();
		void Unlock();
		void WaitForCompletion(RenderPassStats *optStats = nullptr) const;
		bool IsComplete() const;
	  private:
		RenderQueue(std::string name);

		std::atomic<bool> m_locked = false;
		mutable std::condition_variable m_threadWaitCondition {};
		mutable std::mutex m_threadWaitMutex {};
		std::mutex m_queueMutex {};
		std::string m_name;
	};

	class RenderQueueJob {
		std::shared_ptr<RenderQueue> m_renderQueue;
		bool IsLocked() const;
		void QueryForRendering();
		void Reset();
		void Start(); // ??
	};

	class DLLCLIENT RenderQueueBuilder {
	  public:
		RenderQueueBuilder();
		~RenderQueueBuilder();
		void Append(const std::function<void()> &workerBuildQueue, const std::function<void()> &workerCompleteQueue);
		void Flush();
		bool HasWork() const;
		uint32_t GetWorkQueueCount() const;
		void SetReadyForCompletion();
	  private:
		void Exec();
		void BuildRenderQueues();
		std::thread m_thread;

		std::mutex m_workMutex;
		std::queue<std::function<void()>> m_renderQueueBuildQueue;
		std::queue<std::function<void()>> m_renderQueueCompleteQueue;
		std::condition_variable m_threadWaitCondition {};
		std::atomic<bool> m_threadRunning = false;
		std::atomic<bool> m_hasWork = false;
		std::atomic<bool> m_readyForCompletion = false;
	};
};
