/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __PRAGMA_LUA_THREAD_POOL_HPP__
#define __PRAGMA_LUA_THREAD_POOL_HPP__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
#include <sharedutils/ctpl_stl.h>

namespace pragma::lua
{
	class DLLNETWORK LuaThreadPool
	{
	public:
		using ResultHandler = std::function<void(lua_State*)>;
		static constexpr void(*NO_RESULT)() {nullptr};

		LuaThreadPool(lua_State *l,uint32_t threadCount);
		uint32_t AddTask(const std::function<ResultHandler()> &task);
		bool IsComplete() const {return m_completedTaskCount == m_totalTaskCount;}
		bool IsComplete(uint32_t taskId) const;
		void Stop(bool execRemainingQueue=false);
		void PushResults(uint32_t taskId);

		void WaitForPendingCount(uint32_t count);
		uint32_t GetTotalTaskCount() const {return m_totalTaskCount;}
		uint32_t GetPendingTaskCount() const {return m_totalTaskCount -GetCompletedTaskCount();}
		uint32_t GetCompletedTaskCount() const {return m_completedTaskCount;}
	private:
		struct TaskState
		{
			bool isComplete = false;
			ResultHandler resultHandler = nullptr;
		};
		lua_State *m_luaState = nullptr;
		ctpl::thread_pool m_pool;
		std::mutex m_taskMutex;

		std::condition_variable m_taskCompleteCondition;
		std::vector<TaskState> m_taskCompleted;
		mutable std::mutex m_taskCompletedMutex;

		std::atomic<uint32_t> m_completedTaskCount = 0;
		uint32_t m_totalTaskCount = 0;
	};
};

#endif