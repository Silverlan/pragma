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
	class DLLNETWORK FunctionalParallelWorker : public util::ParallelWorker<void> {
	  public:
		using Task = std::function<void(FunctionalParallelWorker &)>;
		FunctionalParallelWorker(bool continuousUntilCancelled);
		virtual ~FunctionalParallelWorker() override;
		void WaitForTask();
		virtual void GetResult() override;

		void ResetTask(const Task &task);
		void CancelTask();
		bool IsTaskCancelled() const;
		void CallOnRemove(const std::function<void(void)> &callOnRemove);

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
};

#endif
