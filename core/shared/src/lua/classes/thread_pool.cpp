/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/custom_constructor.hpp"
#include "pragma/lua/classes/thread_pool.hpp"

pragma::lua::LuaThreadPool::LuaThreadPool(lua_State *l, uint32_t threadCount) : LuaThreadPool {l, threadCount, ""} {}
pragma::lua::LuaThreadPool::LuaThreadPool(lua_State *l, uint32_t threadCount, const std::string &name) : m_luaState {l}, ThreadPool {threadCount, name, "lua"} {}

uint32_t pragma::lua::LuaThreadPool::AddTask(const std::function<ResultHandler()> &task)
{
	return ThreadPool::AddTask([this, task]() -> ThreadPool::ResultHandler {
		auto resHandler = task();
		return [this, resHandler]() { return resHandler(m_luaState); };
	});
}

uint32_t pragma::lua::LuaThreadPool::AddTask(const std::shared_ptr<LuaThreadTask> &task)
{
	return AddTask([task]() -> ResultHandler {
		for(auto &st : task->subTasks)
			st();
		return {};
	});
}

namespace pragma::lua {
	void register_thread_pool(lua_State *l, luabind::module_ &modUtil)
	{
		auto classDefTask = luabind::class_<LuaThreadTask>("ThreadTask");

		auto classDefPool = luabind::class_<LuaThreadPool>("ThreadPool")
		                      .def("IsComplete", static_cast<bool (LuaThreadPool::*)() const>(&LuaThreadPool::IsComplete))
		                      .def("IsComplete", static_cast<bool (LuaThreadPool::*)(uint32_t) const>(&LuaThreadPool::IsComplete))
		                      .def("AddBarrier", &LuaThreadPool::AddBarrier)
		                      .def(
		                        "AddTask", +[](LuaThreadPool &pool, std::shared_ptr<LuaThreadTask> &threadTask) { return pool.AddTask(threadTask); })
		                      .def("Stop", &LuaThreadPool::Stop)
		                      .def("PushResults", &LuaThreadPool::PushResults)
		                      .def("WaitForPendingCount", &LuaThreadPool::WaitForPendingCount)
		                      .def("WaitForCompletion", &LuaThreadPool::WaitForCompletion)
		                      .def("GetTaskCount", &LuaThreadPool::GetTotalTaskCount)
		                      .def("GetPendingTaskCount", &LuaThreadPool::GetPendingTaskCount)
		                      .def("GetCompletedTaskCount", &LuaThreadPool::GetCompletedTaskCount);
		classDefPool.scope[classDefTask];
		modUtil[classDefPool];
		pragma::lua::define_custom_constructor<LuaThreadPool, +[](lua_State *l, uint32_t threadCount) -> std::shared_ptr<LuaThreadPool> { return std::make_shared<LuaThreadPool>(l, threadCount); }, lua_State *, uint32_t>(l);
		pragma::lua::define_custom_constructor<LuaThreadPool, +[](lua_State *l, uint32_t threadCount, const std::string &name) -> std::shared_ptr<LuaThreadPool> { return std::make_shared<LuaThreadPool>(l, threadCount, name); }, lua_State *, uint32_t, const std::string &>(l);
		pragma::lua::define_custom_constructor<LuaThreadTask, +[]() -> std::shared_ptr<LuaThreadTask> { return std::make_shared<LuaThreadTask>(); }>(l);
	}
};

pragma::lua::LuaThreadWrapper::LuaThreadWrapper(std::shared_ptr<LuaThreadTask> &luaThreadTask) : m_ptr {&luaThreadTask}, m_isPool {false} {}
pragma::lua::LuaThreadWrapper::LuaThreadWrapper(LuaThreadPool &luaThreadTask) : m_ptr {&luaThreadTask}, m_isPool {true} {}

std::shared_ptr<pragma::lua::LuaThreadTask> &pragma::lua::LuaThreadWrapper::GetTask() const
{
	assert(!m_isPool);
	return *static_cast<std::shared_ptr<pragma::lua::LuaThreadTask> *>(m_ptr);
}
pragma::lua::LuaThreadPool &pragma::lua::LuaThreadWrapper::GetPool() const
{
	assert(m_isPool);
	return *static_cast<pragma::lua::LuaThreadPool *>(m_ptr);
}
bool pragma::lua::LuaThreadWrapper::IsPool() const { return m_isPool; }
bool pragma::lua::LuaThreadWrapper::IsTask() const { return !m_isPool; }
