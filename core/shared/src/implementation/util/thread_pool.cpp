// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :util.thread_pool;

pragma::ThreadPool::ThreadPool(uint32_t threadCount) : ThreadPool {threadCount, ""} {}
pragma::ThreadPool::ThreadPool(uint32_t threadCount, const std::string &name, const std::string &baseName) : m_pool {static_cast<int>(threadCount)}
{
	auto n = m_pool.size();
	std::string fullName = baseName;
	if(!name.empty())
		fullName += '_' + name;
	for(auto i = decltype(n) {0u}; i < n; ++i)
		util::set_thread_name(m_pool.get_thread(i), fullName);
}

void pragma::ThreadPool::Stop(bool execRemainingQueue) { m_pool.stop(execRemainingQueue); }

void pragma::ThreadPool::PushResults(uint32_t taskId)
{
	std::scoped_lock slock {m_taskCompletedMutex};
	if(taskId >= m_taskCompleted.size() || m_taskCompleted[taskId].isComplete == false || m_taskCompleted[taskId].resultHandler == nullptr)
		return;
	m_taskCompleted[taskId].resultHandler();
}

void pragma::ThreadPool::BatchProcess(uint32_t numJobs, uint32_t numItemsPerJob, const std::function<ResultHandler(uint32_t, uint32_t)> &f)
{
	auto numBatches = numJobs / numItemsPerJob;
	if((numJobs % numItemsPerJob) > 0)
		++numBatches;
	for(auto i = decltype(numBatches) {0u}; i < numBatches; ++i) {
		auto offset = i * numItemsPerJob;
		auto end = math::min(offset + numItemsPerJob, numJobs);
		AddTask([f, offset, end]() -> ResultHandler { return f(offset, end); });
	}
}

void pragma::ThreadPool::WaitForCompletion() { return WaitForPendingCount(0); }

void pragma::ThreadPool::WaitForPendingCount(uint32_t count)
{
	if(GetPendingTaskCount() <= count)
		return;
	auto ul = std::unique_lock<std::mutex>(m_taskCompletedMutex);
	m_taskCompleteCondition.wait(ul, [this, count]() { return GetPendingTaskCount() <= count; });
}

bool pragma::ThreadPool::IsComplete(uint32_t taskId) const
{
	std::scoped_lock slock {m_taskCompletedMutex};
	return (taskId < m_taskCompleted.size()) ? m_taskCompleted[taskId].isComplete : false;
}

void pragma::ThreadPool::AddBarrier() { m_pool.barrier(); }

uint32_t pragma::ThreadPool::AddTask(const std::function<ResultHandler()> &task)
{
	auto taskId = m_totalTaskCount++;
	if(m_totalTaskCount >= m_taskCompleted.size()) {
		std::scoped_lock slock {m_taskCompletedMutex};
		m_taskCompleted.resize(m_totalTaskCount);
	}
	m_pool.push([this, task, taskId](int id) {
		auto resultHandler = task ? task() : nullptr;
		++m_completedTaskCount;

		std::scoped_lock slock {m_taskCompletedMutex};
		m_taskCompleted[taskId].resultHandler = std::move(resultHandler);
		m_taskCompleted[taskId].isComplete = true;
		m_taskCompleteCondition.notify_one();
	});
	return taskId;
}
