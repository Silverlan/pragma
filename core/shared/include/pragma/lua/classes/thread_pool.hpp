/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __PRAGMA_LUA_THREAD_POOL_HPP__
#define __PRAGMA_LUA_THREAD_POOL_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/util/util_thread_pool.hpp"
#include <pragma/lua/luaapi.h>

namespace pragma::lua {
	struct DLLNETWORK LuaThreadTask {
		void AddSubTask(const std::function<void()> &subTask) { subTasks.push_back(subTask); }
		std::vector<std::function<void()>> subTasks;
	};
	class DLLNETWORK LuaThreadPool : public pragma::ThreadPool {
	  public:
		using ResultHandler = std::function<void(lua_State *)>;

		LuaThreadPool(lua_State *l, uint32_t threadCount);
		LuaThreadPool(lua_State *l, uint32_t threadCount, const std::string &name);
		uint32_t AddTask(const std::function<ResultHandler()> &task);
		uint32_t AddTask(const std::shared_ptr<LuaThreadTask> &task);
	  private:
		lua_State *m_luaState = nullptr;
	};
	struct DLLNETWORK LuaThreadWrapper {
		LuaThreadWrapper() = default;
		LuaThreadWrapper(std::shared_ptr<LuaThreadTask> &luaThreadTask);
		LuaThreadWrapper(LuaThreadPool &luaThreadTask);

		std::shared_ptr<LuaThreadTask> &GetTask() const;
		LuaThreadPool &GetPool() const;
		bool IsPool() const;
		bool IsTask() const;
	  private:
		bool m_isPool = false;
		mutable void *m_ptr = nullptr;
	};
};

#endif
