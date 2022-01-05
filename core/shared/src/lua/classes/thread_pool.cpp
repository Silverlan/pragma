/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/custom_constructor.hpp"
#include "pragma/lua/classes/thread_pool.hpp"

pragma::lua::LuaThreadPool::LuaThreadPool(lua_State *l,uint32_t threadCount)
	: LuaThreadPool{l,threadCount,""}
{}
pragma::lua::LuaThreadPool::LuaThreadPool(lua_State *l,uint32_t threadCount,const std::string &name)
	: m_luaState{l},m_pool{threadCount}
{
	auto n = m_pool.size();
	std::string fullName = "lua";
	if(!name.empty())
		fullName += '_' +name;
	for(auto i=decltype(n){0u};i<n;++i)
		util::set_thread_name(m_pool.get_thread(i),fullName);
}

void pragma::lua::LuaThreadPool::Stop(bool execRemainingQueue)
{
	m_pool.stop(execRemainingQueue);
}

void pragma::lua::LuaThreadPool::PushResults(uint32_t taskId)
{
	std::scoped_lock slock {m_taskCompletedMutex};
	if(taskId >= m_taskCompleted.size() || m_taskCompleted[taskId].isComplete == false)
		return;
	m_taskCompleted[taskId].resultHandler(m_luaState);
}

void pragma::lua::LuaThreadPool::WaitForPendingCount(uint32_t count)
{
	if(GetPendingTaskCount() < count)
		return;
	auto ul = std::unique_lock<std::mutex>(m_taskCompletedMutex);
	m_taskCompleteCondition.wait(ul,[this,count]() {
		return GetPendingTaskCount() < count;
	});
}

bool pragma::lua::LuaThreadPool::IsComplete(uint32_t taskId) const
{
	std::scoped_lock slock {m_taskCompletedMutex};
	return (taskId < m_taskCompleted.size()) ? m_taskCompleted[taskId].isComplete : false;
}

uint32_t pragma::lua::LuaThreadPool::AddTask(const std::function<ResultHandler()> &task)
{
	auto taskId = m_totalTaskCount++;
	if(m_totalTaskCount >= m_taskCompleted.size())
	{
		std::scoped_lock slock {m_taskCompletedMutex};
		m_taskCompleted.resize(m_totalTaskCount);
	}
	m_pool.push([this,task,taskId](int id) {
		auto resultHandler = task();
		++m_completedTaskCount;

		std::scoped_lock slock {m_taskCompletedMutex};
		m_taskCompleted[taskId].resultHandler = std::move(resultHandler);
		m_taskCompleted[taskId].isComplete = true;
		m_taskCompleteCondition.notify_one();
	});
	return taskId;
}

namespace pragma::lua
{
	void register_thread_pool(lua_State *l,luabind::module_ &modUtil)
	{
		auto classDefPool = luabind::class_<LuaThreadPool>("ThreadPool")
		.def("IsComplete",static_cast<bool(LuaThreadPool::*)() const>(&LuaThreadPool::IsComplete))
		.def("IsComplete",static_cast<bool(LuaThreadPool::*)(uint32_t) const>(&LuaThreadPool::IsComplete))
		.def("Stop",&LuaThreadPool::Stop)
		.def("PushResults",&LuaThreadPool::PushResults)
		.def("WaitForPendingCount",&LuaThreadPool::WaitForPendingCount)
		.def("GetTaskCount",&LuaThreadPool::GetTotalTaskCount)
		.def("GetPendingTaskCount",&LuaThreadPool::GetPendingTaskCount)
		.def("GetCompletedTaskCount",&LuaThreadPool::GetCompletedTaskCount);
		modUtil[classDefPool];
		pragma::lua::define_custom_constructor<LuaThreadPool,[](lua_State *l,uint32_t threadCount) -> std::shared_ptr<LuaThreadPool> {
			return std::make_shared<LuaThreadPool>(l,threadCount);
		},lua_State*,uint32_t>(l);
		pragma::lua::define_custom_constructor<LuaThreadPool,[](lua_State *l,uint32_t threadCount,const std::string &name) -> std::shared_ptr<LuaThreadPool> {
			return std::make_shared<LuaThreadPool>(l,threadCount,name);
		},lua_State*,uint32_t,const std::string&>(l);
	}
};
