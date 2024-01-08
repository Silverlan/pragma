/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan */

#include "pragma/lua/classes/parallel_job.hpp"
#include "pragma/lua/lua_call.hpp"

using namespace pragma::lua;

LuaWorker::LuaWorker(Game &game, const std::string &name) : m_workerName {name}
{
	m_cbThink = game.AddCallback("Think", FunctionCallback<void>::Create([this]() { Update(); }));
	m_cbEndGame = game.AddCallback("EndGame", FunctionCallback<void>::Create([this]() { Clear(); }));
	AddThread([this]() {
		auto ul = std::unique_lock<std::mutex> {m_mutex};
		m_cond.wait(ul, [this]() -> bool { return GetStatus() != util::JobStatus::Pending; });
	});
}
LuaWorker::~LuaWorker() { Clear(); }
luabind::object LuaWorker::GetResult() { return m_result; }
void LuaWorker::Wait()
{
	// The Lua-function can only run on the main thread, so we cannot actually
	// wait for it. We'll cancel the task instead.
	auto status = GetStatus();
	if(status == util::JobStatus::Pending)
		SetStatus(util::JobStatus::Failed, "Premature cancellation.");
	Clear();
	m_mutex.lock();
	m_cond.notify_one();
	m_mutex.unlock();

	util::ParallelWorker<luabind::object>::Wait();
}
void LuaWorker::SetResult(const luabind::object &result) { m_result = result; }
void LuaWorker::UpdateTaskProgress(const Task &task, float taskProgress)
{
	auto progress = m_curProgress + (taskProgress * task.progressAmount);
	UpdateProgress(progress);
}
void LuaWorker::AddTask(const luabind::object &subJob, const Lua::func<bool> &onCompleteTask, float taskProgress)
{
	auto *job = luabind::object_cast<util::BaseParallelJob *>(subJob);
	auto cpySubJob = subJob; // We need to keep a reference to this job

	Task task {};
	task.progressAmount = taskProgress;
	task.update = [this, cpySubJob, job, onCompleteTask](const Task &task) mutable -> bool {
		UpdateTaskProgress(task, job->GetProgress());
		if(job->IsComplete() == false)
			return false;
		if(!job->IsSuccessful()) {
			SetStatus(job->GetStatus(), job->GetResultMessage());
			cpySubJob = {};
			return true;
		}
		auto *l = onCompleteTask.interpreter();
		auto r = Lua::CallFunction(
		  l,
		  [this, &onCompleteTask](lua_State *l) -> Lua::StatusCode {
			  onCompleteTask.push(l);
			  luabind::object o {l, this};
			  o.push(l);
			  return Lua::StatusCode::Ok;
		  },
		  1);
		if(r != Lua::StatusCode::Ok) {
			SetStatus(util::JobStatus::Failed, "Lua Error");
			cpySubJob = {};
			return true;
		}
		auto res = Lua::IsSet(l, -1) && Lua::CheckBool(l, -1);
		Lua::Pop(l);
		if(res)
			cpySubJob = {};
		return res;
	};
	task.cancel = [job](const Task &task) {
		job->Cancel();
		return true;
	};
	m_updateFuncs.push(std::move(task));
}
void LuaWorker::AddLuaTask(const std::shared_ptr<util::ParallelJob<luabind::object>> &subJob, float taskProgress)
{
	Task task {};
	task.progressAmount = taskProgress;
	task.update = [this, subJob](const Task &task) -> bool {
		UpdateTaskProgress(task, subJob->GetProgress());
		if(subJob->IsComplete() == false)
			return false;
		SetStatus(subJob->GetStatus(), subJob->GetResultMessage());
		return true;
	};
	task.cancel = [subJob](const Task &task) {
		subJob->Cancel();
		return true;
	};
	m_updateFuncs.push(std::move(task));
}
void LuaWorker::AddLuaTask(const std::shared_ptr<util::ParallelJob<luabind::object>> &subJob, const Lua::func<bool> &onCompleteTask, float taskProgress)
{
	Task task {};
	task.progressAmount = taskProgress;
	task.update = [this, subJob, onCompleteTask](const Task &task) -> bool {
		UpdateTaskProgress(task, subJob->GetProgress());
		if(subJob->IsComplete() == false)
			return false;
		if(!subJob->IsSuccessful()) {
			SetStatus(subJob->GetStatus(), subJob->GetResultMessage());
			return true;
		}
		auto *l = onCompleteTask.interpreter();
		auto r = Lua::CallFunction(
		  l,
		  [this, &onCompleteTask](lua_State *l) -> Lua::StatusCode {
			  onCompleteTask.push(l);
			  luabind::object o {l, this};
			  o.push(l);
			  return Lua::StatusCode::Ok;
		  },
		  1);
		if(r != Lua::StatusCode::Ok) {
			SetStatus(util::JobStatus::Failed, "Lua Error");
			return true;
		}
		auto res = Lua::IsSet(l, -1) && Lua::CheckBool(l, -1);
		Lua::Pop(l);
		return res;
	};
	task.cancel = [subJob](const Task &task) {
		subJob->Cancel();
		return true;
	};
	m_updateFuncs.push(std::move(task));
}
void LuaWorker::AddLuaTask(const Lua::func<bool> &luaFunc, const Lua::func<bool> &cancelFunc, float taskProgress)
{
	Task task {};
	task.progressAmount = taskProgress;
	task.update = [this, luaFunc, taskProgress](const Task &task) {
		auto *l = luaFunc.interpreter();
		auto r = Lua::CallFunction(
		  l,
		  [this, &luaFunc](lua_State *l) -> Lua::StatusCode {
			  luaFunc.push(l);
			  luabind::object o {l, this};
			  o.push(l);
			  return Lua::StatusCode::Ok;
		  },
		  1);
		if(r != Lua::StatusCode::Ok) {
			SetStatus(util::JobStatus::Failed, "Lua Error");
			UpdateTaskProgress(task, 1.f);
			return true;
		}
		auto res = Lua::IsSet(l, -1) && Lua::CheckBool(l, -1);
		Lua::Pop(l);
		if(res)
			UpdateTaskProgress(task, 1.f);
		return res;
	};
	task.cancel = [this, cancelFunc](const Task &task) {
		auto *l = cancelFunc.interpreter();
		auto r = Lua::CallFunction(
		  l,
		  [this, &cancelFunc](lua_State *l) -> Lua::StatusCode {
			  cancelFunc.push(l);
			  luabind::object o {l, this};
			  o.push(l);
			  return Lua::StatusCode::Ok;
		  },
		  0);
		return r == Lua::StatusCode::Ok;
	};
	m_updateFuncs.push(std::move(task));
}
void LuaWorker::CallOnComplete(const Lua::func<void> &func)
{
	m_onComplete = [this, func](util::ParallelWorker<luabind::object> &worker) {
		auto *l = func.interpreter();
		auto r = Lua::CallFunction(
		  l,
		  [this, &func](lua_State *l) -> Lua::StatusCode {
			  func.push(l);
			  luabind::object o {l, this};
			  o.push(l);
			  return Lua::StatusCode::Ok;
		  },
		  0);
	};
}
void LuaWorker::UpdateProgress(float progress) { util::ParallelWorker<luabind::object>::UpdateProgress(progress); }
void LuaWorker::DoCancel(const std::string &resultMsg, std::optional<int32_t> resultCode)
{
	if(std::this_thread::get_id() != pragma::get_engine()->GetMainThreadId())
		throw std::logic_error {"Cannot cancel Lua-worker from non-main thread!"};
	while(!m_updateFuncs.empty()) {
		auto task = std::move(m_updateFuncs.front());
		m_updateFuncs.pop();
		auto res = task.cancel(task);
		if(!res)
			SetStatus(util::JobStatus::Failed, "Failed to cancel: Lua Error");
	}
	Finalize();
}
void LuaWorker::Clear()
{
	m_result = {};
	m_updateFuncs = {};

	if(m_cbThink.IsValid())
		m_cbThink.Remove();
	if(m_cbEndGame.IsValid())
		m_cbEndGame.Remove();
}
void LuaWorker::Finalize()
{
	if(m_cbThink.IsValid())
		m_cbThink.Remove();
	m_mutex.lock();
	m_cond.notify_one();
	m_mutex.unlock();

	if(m_onComplete)
		m_onComplete(*this);
}
void LuaWorker::Update()
{
	if(GetStatus() == util::JobStatus::Initial)
		return; // Not yet started
	if(m_updateFuncs.empty()) {
		Finalize();
		return;
	}
	auto &task = m_updateFuncs.front();
	if(task.update(task)) {
		m_curProgress += task.progressAmount;
		UpdateTaskProgress(task, 0.f);
		m_updateFuncs.pop();
	}
	if(GetStatus() != util::JobStatus::Pending)
		Finalize();
}
