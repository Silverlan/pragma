/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_LAI_BEHAVIOR_H__
#define __S_LAI_BEHAVIOR_H__

#include "pragma/serverdefinitions.h"
#include <pragma/lua/ldefinitions.h>
#include "pragma/ai/ai_behavior.h"
#include "pragma/lua/luaobjectbase.h"
#include "pragma/lua/s_lentity_handles.hpp"

/*
	NOTE:
	AILuaBehaviorNode has a Lua-object to itself, which means it would never be garbage-collected.
	To resolve this, the function Clear() will clear the Lua-object and thus allow it to be garbage collected.
	Clear is called when the parent-task was destroyed, which means:
	- The root task in the tree must never be a AILuaBehaviorNode task (Otherwise destruction would never occur)
	- There mustn't be a way to create Lua-tasks without a parent task

	Both of these are guaranteed because the root task of a schedule is always an engine task, and the only way to create a new Lua-task is
	by calling "CreateTask" (In Lua) on an existing task.
*/
class DLLSERVER AILuaBehaviorNode : public pragma::ai::BehaviorNode, public LuaObjectBase {
  protected:
	luabind::object m_luaClass;
	virtual void Clear() override;
	virtual void OnTaskComplete(const pragma::ai::Schedule *sched, uint32_t taskId, Result result) override;
  public:
	AILuaBehaviorNode(Type type = Type::Sequence, pragma::ai::SelectorType selectorType = pragma::ai::SelectorType::Sequential);
	virtual std::shared_ptr<BehaviorNode> Copy() const override;
	virtual ~AILuaBehaviorNode() override;
	void SetLuaClass(const luabind::object &o);
	virtual void Stop() override;
	virtual Result Think(const pragma::ai::Schedule *sched, pragma::SAIComponent &ent) override;
	virtual Result Start(const pragma::ai::Schedule *sched, pragma::SAIComponent &ent) override;
	virtual void SetScheduleParameter(uint8_t taskParamId, uint8_t scheduleParamId) override;
	virtual void Print(const pragma::ai::Schedule *sched, std::ostream &o) const override;
};

namespace pragma {
	namespace ai {
		class TaskMoveToTarget;
		class TaskMoveRandom;
		class TaskLookAtTarget;
		class TaskPlayAnimation;
		class TaskPlayActivity;
		class TaskPlayLayeredAnimation;
		class TaskPlayLayeredActivity;
		class TaskPlaySound;
		class TaskRandom;
		class TaskDebugPrint;
		class TaskDebugDrawText;
		class TaskDecorator;
		class TaskEvent;
		class TaskWait;
	};
};
namespace Lua {
	namespace AIBehaviorNode {
		DLLSERVER void register_class(lua_State *l, luabind::module_ &mod);
	};

	namespace ai {
		class TaskWrapper {
		  public:
			TaskWrapper(pragma::ai::BehaviorNode &task);
			TaskWrapper(const std::shared_ptr<pragma::ai::BehaviorNode> &task);
			pragma::ai::BehaviorNode &GetTask();
			const pragma::ai::BehaviorNode &GetTask() const;

			const pragma::ai::BehaviorNode &operator*() const;
			pragma::ai::BehaviorNode &operator*();

			const pragma::ai::BehaviorNode *operator->() const;
			pragma::ai::BehaviorNode *operator->();
		  private:
			std::shared_ptr<pragma::ai::BehaviorNode> m_task = nullptr;
		};

		template<class TTask>
		class TTaskWrapper : public TaskWrapper {
		  public:
			TTask &GetTask() { return static_cast<TTask &>(TaskWrapper::GetTask()); }
			const TTask &GetTask() const { return static_cast<TTask &>(TaskWrapper::GetTask()); }

			const TTask &operator*() const { return static_cast<TTask &>(TaskWrapper::operator*()); }
			TTask &operator*() { return static_cast<TTask &>(TaskWrapper::operator*()); }

			const TTask *operator->() const { return static_cast<TTask *>(TaskWrapper::operator->()); }
			TTask *operator->() { return static_cast<TTask *>(TaskWrapper::operator->()); }
		};
		using TaskWrapperMoveToTarget = TTaskWrapper<pragma::ai::TaskMoveToTarget>;
		using TaskWrapperMoveRandom = TTaskWrapper<pragma::ai::TaskMoveRandom>;
		using TaskWrapperLookAtTarget = TTaskWrapper<pragma::ai::TaskLookAtTarget>;
		using TaskWrapperPlayAnimation = TTaskWrapper<pragma::ai::TaskPlayAnimation>;
		using TaskWrapperPlayActivity = TTaskWrapper<pragma::ai::TaskPlayActivity>;
		using TaskWrapperPlayLayeredAnimation = TTaskWrapper<pragma::ai::TaskPlayLayeredAnimation>;
		using TaskWrapperPlayLayeredActivity = TTaskWrapper<pragma::ai::TaskPlayLayeredActivity>;
		using TaskWrapperPlaySound = TTaskWrapper<pragma::ai::TaskPlaySound>;
		using TaskWrapperRandom = TTaskWrapper<pragma::ai::TaskRandom>;
		using TaskWrapperDebugPrint = TTaskWrapper<pragma::ai::TaskDebugPrint>;
		using TaskWrapperDebugDrawText = TTaskWrapper<pragma::ai::TaskDebugDrawText>;
		using TaskWrapperDecorator = TTaskWrapper<pragma::ai::TaskDecorator>;
		using TaskWrapperEvent = TTaskWrapper<pragma::ai::TaskEvent>;
		using TaskWrapperWait = TTaskWrapper<pragma::ai::TaskWait>;

		void push_task(lua_State *l, pragma::ai::BehaviorNode &task);
	};
};

namespace Lua {
	namespace ai {
		class BaseBehaviorTask : public ai::TaskWrapper {
		  public:
			using ai::TaskWrapper::TaskWrapper;
		};
	};
};

struct DLLSERVER AILuaBehaviorNodeWrapper : public Lua::ai::BaseBehaviorTask, luabind::wrap_base {
  public:
	AILuaBehaviorNodeWrapper(uint32_t nodeType, uint32_t selectorType);
	AILuaBehaviorNodeWrapper(uint32_t nodeType);
	AILuaBehaviorNodeWrapper();
	//AILuaBehaviorNodeWrapper(const AILuaBehaviorNodeWrapper&)=delete;
	//AILuaBehaviorNodeWrapper &operator=(const AILuaBehaviorNodeWrapper&)=delete;

	uint32_t Start(std::shared_ptr<pragma::ai::Schedule> &schedule, pragma::SAIComponent &hEnt);
	static uint32_t default_Start(lua_State *l, AILuaBehaviorNodeWrapper &wrapper, std::shared_ptr<pragma::ai::Schedule> &schedule, pragma::SAIComponent &hEnt);

	void Stop();
	static void default_Stop(lua_State *l, AILuaBehaviorNodeWrapper &wrapper);

	uint32_t Think(std::shared_ptr<pragma::ai::Schedule> &schedule, pragma::SAIComponent &hEnt, std::underlying_type_t<pragma::ai::BehaviorNode::Result> result);
	static uint32_t default_Think(lua_State *l, AILuaBehaviorNodeWrapper &wrapper, std::shared_ptr<pragma::ai::Schedule> &schedule, pragma::SAIComponent &hEnt, std::underlying_type_t<pragma::ai::BehaviorNode::Result> result);

	void OnTaskComplete(std::shared_ptr<pragma::ai::Schedule> &schedule, uint32_t taskId, std::underlying_type_t<pragma::ai::BehaviorNode::Result> result);
	static void default_OnTaskComplete(lua_State *l, AILuaBehaviorNodeWrapper &wrapper, std::shared_ptr<pragma::ai::Schedule> &schedule, uint32_t taskId, std::underlying_type_t<pragma::ai::BehaviorNode::Result> result);

	void OnSetScheduleParameter(uint8_t taskParamId, uint8_t scheduleParamId);
	static void default_OnSetScheduleParameter(lua_State *l, AILuaBehaviorNodeWrapper &wrapper, uint8_t taskParamId, uint8_t scheduleParamId);
};

#endif
