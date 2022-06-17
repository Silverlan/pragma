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
	: m_luaState{l},ThreadPool{threadCount,name,"lua"}
{}

uint32_t pragma::lua::LuaThreadPool::AddTask(const std::function<ResultHandler()> &task)
{
	return ThreadPool::AddTask([this,task]() -> ThreadPool::ResultHandler {
		auto resHandler = task();
		return [this,resHandler]() {
			return resHandler(m_luaState);
		};
	});
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
		.def("WaitForCompletion",&LuaThreadPool::WaitForCompletion)
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
