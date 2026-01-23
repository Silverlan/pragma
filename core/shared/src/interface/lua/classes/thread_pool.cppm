// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.classes.thread_pool;

export import :util.thread_pool;
export import std.compat;

export namespace pragma::LuaCore {
	struct DLLNETWORK LuaThreadTask {
		void AddSubTask(const std::function<void()> &subTask) { subTasks.push_back(subTask); }
		std::vector<std::function<void()>> subTasks;
	};
	class DLLNETWORK LuaThreadPool : public ThreadPool {
	  public:
		using ResultHandler = std::function<void(lua::State *)>;

		LuaThreadPool(lua::State *l, uint32_t threadCount);
		LuaThreadPool(lua::State *l, uint32_t threadCount, const std::string &name);
		uint32_t AddTask(const std::function<ResultHandler()> &task);
		uint32_t AddTask(const std::shared_ptr<LuaThreadTask> &task);
	  private:
		lua::State *m_luaState = nullptr;
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
