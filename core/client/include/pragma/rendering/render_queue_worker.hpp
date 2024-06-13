/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __RENDER_QUEUE_WORKER_HPP__
#define __RENDER_QUEUE_WORKER_HPP__

#include "pragma/clientdefinitions.h"
#include <queue>
#include <vector>
#include <mutex>
#include <atomic>
#include <thread>

#undef AddJob

struct RenderQueueWorkerStats;
namespace pragma::rendering {
	class RenderQueueWorker;
	class RenderQueueWorkerManager {
	  public:
		using Job = std::function<void(void)>;
		RenderQueueWorkerManager(uint32_t numWorkers);
		~RenderQueueWorkerManager();
		void WaitForCompletion();
		void FlushPendingJobs();
		void AddJob(const Job &job);
		uint32_t GetWorkerCount() const;
		void SetWorkerCount(uint32_t numWorkers);

		void SetJobsPerBatchCount(uint32_t numJobsPerBatch) { m_numJobsPerBatch = numJobsPerBatch; }
		uint32_t GetJobsPerBatchCount() const { return m_numJobsPerBatch; }

		RenderQueueWorker &GetWorker(uint32_t i);
		const RenderQueueWorker &GetWorker(uint32_t i) const;
	  private:
		friend RenderQueueWorker;
		std::vector<std::shared_ptr<RenderQueueWorker>> m_workers;
		std::queue<Job> m_pendingJobs;

		std::queue<Job> m_readyJobs;
		std::condition_variable m_readyJobCondition;
		std::atomic<bool> m_workAvailable = false;
		std::atomic<uint32_t> m_numThreadsWorking = 0;
		std::mutex m_readyJobMutex;

		std::condition_variable m_workCompleteCondition;
		std::mutex m_workCompleteMutex;
		uint32_t m_numJobsPerBatch = 2;
	};

	class RenderQueueWorker {
	  public:
		RenderQueueWorker(RenderQueueWorkerManager &manager);
		~RenderQueueWorker();

		void SetStats(RenderQueueWorkerStats *stats);
		void Stop();
	  private:
		void StartThread();
		std::thread m_thread;
		RenderQueueWorkerManager &m_manager;
		std::atomic<bool> m_running = true;
		RenderQueueWorkerStats *m_stats = nullptr;
	};
};

#endif
