/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan */

#ifndef __LUA_PARALLEL_JOB_HPP__
#define __LUA_PARALLEL_JOB_HPP__

#include "pragma/networkdefinitions.h"
#include <sharedutils/util_parallel_job.hpp>
#include "pragma/lua/luaapi.h"
#include <condition_variable>
#include <mutex>

namespace pragma::lua {
	class DLLNETWORK LuaWorker : public util::ParallelWorker<luabind::object> {
	  public:
		enum class TaskStatus : uint8_t {
			Complete = 0,
			Pending,
		};

		LuaWorker(Game &game, const std::string &name);
		virtual ~LuaWorker() override;
		virtual luabind::object GetResult() override;
		virtual void Wait() override;
		void SetResult(const luabind::object &result);
		void AddTask(const luabind::object &subJob, const Lua::func<bool> &onCompleteTask, float taskProgress);
		void AddLuaTask(const std::shared_ptr<util::ParallelJob<luabind::object>> &subJob, float taskProgress);
		void AddLuaTask(const std::shared_ptr<util::ParallelJob<luabind::object>> &subJob, const Lua::func<bool> &onCompleteTask, float taskProgress);
		void AddLuaTask(const Lua::func<TaskStatus> &task, const Lua::func<bool> &cancel, float taskProgress);
		void CallOnComplete(const Lua::func<void> &func);
		void UpdateProgress(float progress);
		void SetProgressCallback(const Lua::func<float> &func);
	  private:
		virtual void DoCancel(const std::string &resultMsg, std::optional<int32_t> resultCode) override;
		void Clear();
		void Finalize();
		void Update();
		template<typename TJob, typename... TARGS>
		friend util::ParallelJob<typename TJob::RESULT_TYPE> util::create_parallel_job(TARGS &&...args);

		struct Task {
			std::function<TaskStatus(const Task &)> update;
			std::function<bool(const Task &)> cancel;
			float progressAmount = 1.f;
		};
		void UpdateTaskProgress(const Task &task, float taskProgress);

		std::optional<Lua::func<float>> m_progressCallback = {};
		float m_lastProgress = 0.f;

		std::function<void(LuaWorker &)> m_onComplete = nullptr;
		luabind::object m_result {};
		std::queue<Task> m_updateFuncs {};
		CallbackHandle m_cbThink;
		CallbackHandle m_cbEndGame;

		std::string m_workerName;
		std::atomic<float> m_curProgress = 0.f;
		std::condition_variable m_cond;
		std::mutex m_mutex;
	};
};

#endif
