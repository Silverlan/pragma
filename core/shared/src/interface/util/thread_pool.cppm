// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:util.thread_pool;

export import pragma.lua;

export namespace pragma {
	class DLLNETWORK ThreadPool {
	  public:
		using ResultHandler = std::function<void()>;
		static constexpr void (*NO_RESULT)() {nullptr};

		ThreadPool(uint32_t threadCount);
		ThreadPool(uint32_t threadCount, const std::string &name, const std::string &baseName = "tp");
		uint32_t AddTask(const std::function<ResultHandler()> &task);
		void AddBarrier();
		bool IsComplete() const { return m_completedTaskCount == m_totalTaskCount; }
		bool IsComplete(uint32_t taskId) const;
		void Stop(bool execRemainingQueue = false);
		void PushResults(uint32_t taskId);
		void BatchProcess(uint32_t numJobs, uint32_t numItemsPerJob, const std::function<ResultHandler(uint32_t, uint32_t)> &f);

		ctpl::thread_pool *operator->() { return &m_pool; }
		ctpl::thread_pool &operator*() { return m_pool; }

		void WaitForPendingCount(uint32_t count);
		void WaitForCompletion();
		uint32_t GetTotalTaskCount() const { return m_totalTaskCount; }
		uint32_t GetPendingTaskCount() const { return m_totalTaskCount - GetCompletedTaskCount(); }
		uint32_t GetCompletedTaskCount() const { return m_completedTaskCount; }
	  private:
		struct TaskState {
			bool isComplete = false;
			ResultHandler resultHandler = nullptr;
		};
		ctpl::thread_pool m_pool;

		std::condition_variable m_taskCompleteCondition;
		std::vector<TaskState> m_taskCompleted;
		mutable std::mutex m_taskCompletedMutex;

		std::atomic<uint32_t> m_completedTaskCount = 0;
		uint32_t m_totalTaskCount = 0;
	};
};
