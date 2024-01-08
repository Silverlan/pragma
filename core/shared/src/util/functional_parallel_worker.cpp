/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan */

#include "pragma/util/functional_parallel_worker.hpp"

using namespace pragma::lua;

util::FunctionalParallelWorker::~FunctionalParallelWorker()
{
	if(m_callOnRemove)
		m_callOnRemove();
}

void util::FunctionalParallelWorker::CallOnRemove(const std::function<void(void)> &callOnRemove) { m_callOnRemove = callOnRemove; }

bool util::FunctionalParallelWorker::IsTaskCancelled() const { return m_taskCancelled; }

void util::FunctionalParallelWorker::CancelTask()
{
	if(!IsPending())
		return;
	m_taskMutex.lock();
	m_taskCancelled = true;
	m_nextTask = nullptr;
	m_taskMutex.unlock();

	m_taskAvailableMutex.lock();
	m_taskAvailableCond.notify_one();
	m_taskAvailableMutex.unlock();

	WaitForTask();

	m_taskComplete = false;
	m_taskCancelled = false;
}

void util::FunctionalParallelWorker::ResetTask(const Task &task)
{
	CancelTask();

	m_taskMutex.lock();
	m_nextTask = task;
	m_taskMutex.unlock();

	m_taskAvailableMutex.lock();
	m_taskAvailable = true;
	m_taskAvailableCond.notify_one();
	m_taskAvailableMutex.unlock();
}

util::FunctionalParallelWorker::FunctionalParallelWorker(bool continuousUntilCancelled) : util::ParallelWorker<void> {}, m_continuousUntilCancelled {continuousUntilCancelled}
{
	AddThread([this]() {
		while(!IsFinished()) {
			auto ul = std::unique_lock<std::mutex> {m_taskAvailableMutex};
			m_taskAvailableCond.wait(ul, [this]() -> bool { return m_taskAvailable || IsFinished() || m_taskCancelled; });
			if(IsFinished())
				break;
			m_taskMutex.lock();
			auto task = std::move(m_nextTask);
			m_nextTask = nullptr;
			m_taskAvailable = false;
			auto taskCancelled = m_taskCancelled ? true : false;
			m_taskCancelled = false;
			m_taskMutex.unlock();

			if(!taskCancelled) {
				assert(task != nullptr);
				if(task)
					task(*this);
			}

			m_taskCompleteMutex.lock();
			m_taskComplete = true;
			m_taskCompleteCond.notify_one();
			m_taskCompleteMutex.unlock();
		}

		m_taskCompleteMutex.lock();
		m_taskComplete = true;
		m_taskCompleteCond.notify_one();
		m_taskCompleteMutex.unlock();
	});
}
bool util::FunctionalParallelWorker::IsFinished() const
{
	if(IsCancelled())
		return true;
	if(m_continuousUntilCancelled)
		return false;
	return GetStatus() != util::JobStatus::Pending;
}
void util::FunctionalParallelWorker::WaitForTask()
{
	auto ul = std::unique_lock<std::mutex> {m_taskCompleteMutex};
	m_taskCompleteCond.wait(ul, [this]() -> bool { return m_taskComplete; });
}
void util::FunctionalParallelWorker::GetResult() { return; } //std::move(m_result);}
void util::FunctionalParallelWorker::DoCancel(const std::string &resultMsg, std::optional<int32_t> resultCode)
{
	util::ParallelWorker<void>::DoCancel(resultMsg, resultCode);

	m_taskAvailableMutex.lock();
	m_taskAvailable = true;
	m_taskAvailableCond.notify_one();
	m_taskAvailableMutex.unlock();
}
