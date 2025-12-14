// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cassert>

module pragma.shared;

import :scripting.lua.classes.thread_pool;

pragma::LuaCore::LuaThreadPool::LuaThreadPool(lua::State *l, uint32_t threadCount) : LuaThreadPool {l, threadCount, ""} {}
pragma::LuaCore::LuaThreadPool::LuaThreadPool(lua::State *l, uint32_t threadCount, const std::string &name) : m_luaState {l}, ThreadPool {threadCount, name, "lua"} {}

uint32_t pragma::LuaCore::LuaThreadPool::AddTask(const std::function<ResultHandler()> &task)
{
	return ThreadPool::AddTask([this, task]() -> ThreadPool::ResultHandler {
		auto resHandler = task();
		return [this, resHandler]() { return resHandler(m_luaState); };
	});
}

uint32_t pragma::LuaCore::LuaThreadPool::AddTask(const std::shared_ptr<LuaThreadTask> &task)
{
	return AddTask([task]() -> ResultHandler {
		for(auto &st : task->subTasks)
			st();
		return {};
	});
}

namespace pragma::LuaCore {
	void register_thread_pool(lua::State *l, luabind::module_ &modUtil)
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
		pragma::LuaCore::define_custom_constructor<LuaThreadPool, +[](lua::State *l, uint32_t threadCount) -> std::shared_ptr<LuaThreadPool> { return pragma::util::make_shared<LuaThreadPool>(l, threadCount); }, lua::State *, uint32_t>(l);
		pragma::LuaCore::define_custom_constructor<LuaThreadPool, +[](lua::State *l, uint32_t threadCount, const std::string &name) -> std::shared_ptr<LuaThreadPool> { return pragma::util::make_shared<LuaThreadPool>(l, threadCount, name); }, lua::State *, uint32_t, const std::string &>(l);
		pragma::LuaCore::define_custom_constructor<LuaThreadTask, +[]() -> std::shared_ptr<LuaThreadTask> { return pragma::util::make_shared<LuaThreadTask>(); }>(l);
	}
};

pragma::LuaCore::LuaThreadWrapper::LuaThreadWrapper(std::shared_ptr<LuaThreadTask> &luaThreadTask) : m_ptr {&luaThreadTask}, m_isPool {false} {}
pragma::LuaCore::LuaThreadWrapper::LuaThreadWrapper(LuaThreadPool &luaThreadTask) : m_ptr {&luaThreadTask}, m_isPool {true} {}

std::shared_ptr<pragma::LuaCore::LuaThreadTask> &pragma::LuaCore::LuaThreadWrapper::GetTask() const
{
	assert(!m_isPool);
	return *static_cast<std::shared_ptr<LuaThreadTask> *>(m_ptr);
}
pragma::LuaCore::LuaThreadPool &pragma::LuaCore::LuaThreadWrapper::GetPool() const
{
	assert(m_isPool);
	return *static_cast<LuaThreadPool *>(m_ptr);
}
bool pragma::LuaCore::LuaThreadWrapper::IsPool() const { return m_isPool; }
bool pragma::LuaCore::LuaThreadWrapper::IsTask() const { return !m_isPool; }
