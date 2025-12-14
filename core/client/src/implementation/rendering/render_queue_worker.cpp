// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

#undef AddJob

module pragma.client;

import :rendering.render_queue_worker;
using namespace pragma::rendering;

RenderQueueWorker::RenderQueueWorker(RenderQueueWorkerManager &manager) : m_manager {manager} { StartThread(); }

void RenderQueueWorker::Stop() { m_running = false; }
void RenderQueueWorker::SetStats(RenderQueueWorkerStats *stats) { m_stats = stats; }

void RenderQueueWorker::StartThread()
{
	m_thread = std::thread {[this]() {
		std::queue<RenderQueueWorkerManager::Job> jobs;
		while(m_running) {
			std::unique_lock<std::mutex> mlock(m_manager.m_readyJobMutex);
			m_manager.m_readyJobCondition.wait(mlock, [this]() -> bool { return m_manager.m_workAvailable || !m_running; });

			std::chrono::steady_clock::time_point t;
			if(m_stats)
				t = std::chrono::steady_clock::now();

			++m_manager.m_numThreadsWorking;
			while(m_manager.m_readyJobs.empty() == false && jobs.size() < m_manager.m_numJobsPerBatch) {
				jobs.push(m_manager.m_readyJobs.front());
				m_manager.m_readyJobs.pop();
			}
			if(m_manager.m_readyJobs.empty() && jobs.empty()) {
				m_manager.m_workCompleteMutex.lock();
				m_manager.m_workAvailable = false;
				m_manager.m_workCompleteCondition.notify_one();
				m_manager.m_workCompleteMutex.unlock();
			}
			mlock.unlock();

			if(m_stats)
				m_stats->numJobs += jobs.size();
			while(jobs.empty() == false) {
				jobs.front()();
				jobs.pop();
			}
			if(m_stats)
				m_stats->totalExecutionTime += std::chrono::steady_clock::now() - t;
			if(--m_manager.m_numThreadsWorking == 0) {
				m_manager.m_workCompleteMutex.lock();
				m_manager.m_workCompleteCondition.notify_one();
				m_manager.m_workCompleteMutex.unlock();
			}
		}
	}};
	util::set_thread_name(m_thread, "render_queue_worker");
}

RenderQueueWorker::~RenderQueueWorker()
{
	assert(!m_running);
	m_manager.m_readyJobMutex.lock();
	m_manager.m_readyJobCondition.notify_all();
	m_manager.m_readyJobMutex.unlock();
	if(m_thread.joinable())
		m_thread.join();
}

///////////////////////

RenderQueueWorkerManager::RenderQueueWorkerManager(uint32_t numWorkers) { SetWorkerCount(numWorkers); }

RenderQueueWorker &RenderQueueWorkerManager::GetWorker(uint32_t i) { return *m_workers[i]; }
const RenderQueueWorker &RenderQueueWorkerManager::GetWorker(uint32_t i) const { return const_cast<RenderQueueWorkerManager *>(this)->GetWorker(i); }
uint32_t RenderQueueWorkerManager::GetWorkerCount() const { return m_workers.size(); }
void RenderQueueWorkerManager::SetWorkerCount(uint32_t numWorkers)
{
	if(numWorkers < m_workers.size()) {
		while(m_workers.size() > numWorkers) {
			auto &worker = m_workers.back();
			worker->Stop();
			m_workers.pop_back();
		}
		return;
	}
	m_workers.reserve(numWorkers);
	for(auto i = m_workers.size(); i < numWorkers; ++i)
		m_workers.push_back(pragma::util::make_shared<RenderQueueWorker>(*this));
}

RenderQueueWorkerManager::~RenderQueueWorkerManager()
{
	WaitForCompletion();
	for(auto &worker : m_workers)
		worker->Stop();
	m_workers.clear();
}

void RenderQueueWorkerManager::WaitForCompletion()
{
	FlushPendingJobs();
	std::unique_lock<std::mutex> mlock(m_workCompleteMutex);
	m_workCompleteCondition.wait(mlock, [this]() -> bool { return !m_workAvailable && m_numThreadsWorking == 0; });
}

void RenderQueueWorkerManager::FlushPendingJobs()
{
	m_readyJobMutex.lock();
	while(m_pendingJobs.empty() == false) {
		m_readyJobs.push(m_pendingJobs.front());
		m_pendingJobs.pop();
	}
	m_workAvailable = !m_readyJobs.empty();
	auto num = m_readyJobs.size();
	if(num > m_numJobsPerBatch)
		m_readyJobCondition.notify_all();
	else
		m_readyJobCondition.notify_one();
	m_readyJobMutex.unlock();
}

void RenderQueueWorkerManager::AddJob(const Job &job)
{
	m_pendingJobs.push(job);
	if(m_pendingJobs.size() < m_numJobsPerBatch)
		return; // We'll submit the jobs as batches, to reduce the overhead of the mutex
	FlushPendingJobs();
}
