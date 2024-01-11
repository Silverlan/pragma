/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan */

#ifndef __FUNCTIONAL_PARALLEL_WORKER_HPP__
#define __FUNCTIONAL_PARALLEL_WORKER_HPP__

#include "pragma/networkdefinitions.h"
#include <sharedutils/util_parallel_job.hpp>
#include <functional>

namespace util {
	template<typename T>
	class TFunctionalParallelWorker : public util::ParallelWorker<T> {
	  public:
		using Task = std::function<void(TFunctionalParallelWorker<T> &)>;
		TFunctionalParallelWorker(bool continuousUntilCancelled);
		virtual ~TFunctionalParallelWorker() override;
		void WaitForTask();

		void ResetTask(const Task &task);
		void CancelTask();
		bool IsTaskCancelled() const;
		void CallOnRemove(const std::function<void(void)> &callOnRemove);
		void UpdateProgress(float progress) { util::ParallelWorker<T>::UpdateProgress(progress); }

		virtual T GetResult() override { return T {}; }

		using util::ParallelWorker<void>::SetResultMessage;
		using util::ParallelWorker<void>::UpdateProgress;
	  private:
		using util::ParallelWorker<void>::AddThread;
		virtual void DoCancel(const std::string &resultMsg, std::optional<int32_t> resultCode) override;
		bool IsFinished() const;
		std::function<void(void)> m_callOnRemove = nullptr;

		std::mutex m_taskMutex;

		Task m_nextTask;
		std::atomic<bool> m_taskCancelled = false;

		std::condition_variable m_taskAvailableCond;

		std::condition_variable m_taskCompleteCond;
		std::mutex m_taskCompleteMutex;

		std::atomic<bool> m_taskAvailable = false;
		std::atomic<bool> m_taskComplete = false;
		bool m_continuousUntilCancelled = false;
		std::mutex m_taskAvailableMutex;
		template<typename TJob, typename... TARGS>
		friend util::ParallelJob<typename TJob::RESULT_TYPE> util::create_parallel_job(TARGS &&...args);
	};
	using FunctionalParallelWorker = TFunctionalParallelWorker<void>;

	template<typename T>
	class TFunctionalParallelWorkerWithResult : public TFunctionalParallelWorker<T> {
	  public:
		TFunctionalParallelWorkerWithResult(bool continuousUntilCancelled) : TFunctionalParallelWorker<T> {continuousUntilCancelled} {}
		virtual T GetResult() override;
		const T &GetResultRef() const
		    requires(!std::is_same_v<T, void>)
		{
			return m_result;
		}
		void SetResult(T &&v);
	  private:
		T m_result;
	};
};

template<typename T>
util::TFunctionalParallelWorker<T>::~TFunctionalParallelWorker()
{
	if(m_callOnRemove)
		m_callOnRemove();
}

template<typename T>
void util::TFunctionalParallelWorker<T>::CallOnRemove(const std::function<void(void)> &callOnRemove)
{
	m_callOnRemove = callOnRemove;
}

template<typename T>
bool util::TFunctionalParallelWorker<T>::IsTaskCancelled() const
{
	return m_taskCancelled;
}

template<typename T>
void util::TFunctionalParallelWorker<T>::CancelTask()
{
	if(!this->IsPending())
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

template<typename T>
void util::TFunctionalParallelWorker<T>::ResetTask(const Task &task)
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

template<typename T>
util::TFunctionalParallelWorker<T>::TFunctionalParallelWorker(bool continuousUntilCancelled) : util::ParallelWorker<T> {}, m_continuousUntilCancelled {continuousUntilCancelled}
{
	this->AddThread([this]() {
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
template<typename T>
bool util::TFunctionalParallelWorker<T>::IsFinished() const
{
	if(this->IsCancelled())
		return true;
	if(m_continuousUntilCancelled)
		return false;
	return this->GetStatus() != util::JobStatus::Pending;
}
template<typename T>
void util::TFunctionalParallelWorker<T>::WaitForTask()
{
	auto ul = std::unique_lock<std::mutex> {m_taskCompleteMutex};
	m_taskCompleteCond.wait(ul, [this]() -> bool { return m_taskComplete; });
}
template<typename T>
T util::TFunctionalParallelWorkerWithResult<T>::GetResult()
{
	return m_result;
}
template<typename T>
void util::TFunctionalParallelWorkerWithResult<T>::SetResult(T &&val)
{
	m_result = val;
}
template<typename T>
void util::TFunctionalParallelWorker<T>::DoCancel(const std::string &resultMsg, std::optional<int32_t> resultCode)
{
	util::ParallelWorker<T>::DoCancel(resultMsg, resultCode);

	m_taskAvailableMutex.lock();
	m_taskAvailable = true;
	m_taskAvailableCond.notify_one();
	m_taskAvailableMutex.unlock();
}

#endif
