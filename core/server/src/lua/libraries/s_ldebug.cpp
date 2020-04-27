/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#include "stdafx_server.h"
#include "pragma/lua/libraries/s_ldebug.h"
#include <pragma/serverstate/serverstate.h>
#include "luasystem.h"
#include "pragma/ai/ai_behavior.h"
#include "pragma/ai/ai_task_decorator.h"
#include "pragma/ai/ai_memory.h"
#include "pragma/ai/s_disposition.h"
#include "pragma/ai/s_npcstate.h"
#include "pragma/ai/ai_task.h"

int Lua::debug::Server::behavior_selector_type_to_string(lua_State *l)
{
	auto v = Lua::CheckInt(l,1);
	switch(static_cast<pragma::ai::SelectorType>(v))
	{
		case pragma::ai::SelectorType::Sequential:
			Lua::PushString(l,"ai.BEHAVIOR_SELECTOR_TYPE_SEQUENTIAL");
			break;
		case pragma::ai::SelectorType::RandomShuffle:
			Lua::PushString(l,"ai.BEHAVIOR_SELECTOR_TYPE_RANDOM_SHUFFLE");
			break;
		default:
			Lua::PushString(l,"");
			break;
	}
	return 1;
}

int Lua::debug::Server::behavior_task_decorator_type_to_string(lua_State *l)
{
	auto v = Lua::CheckInt(l,1);
	switch(static_cast<pragma::ai::TaskDecorator::DecoratorType>(v))
	{
		case pragma::ai::TaskDecorator::DecoratorType::Inherit:
			Lua::PushString(l,"ai.BEHAVIOR_TASK_DECORATOR_TYPE_INHERIT");
			break;
		case pragma::ai::TaskDecorator::DecoratorType::AlwaysFail:
			Lua::PushString(l,"ai.BEHAVIOR_TASK_DECORATOR_TYPE_ALWAYS_FAIL");
			break;
		case pragma::ai::TaskDecorator::DecoratorType::AlwaysSucceed:
			Lua::PushString(l,"ai.BEHAVIOR_TASK_DECORATOR_TYPE_ALWAYS_SUCCEED");
			break;
		case pragma::ai::TaskDecorator::DecoratorType::Invert:
			Lua::PushString(l,"ai.BEHAVIOR_TASK_DECORATOR_TYPE_INVERT");
			break;
		case pragma::ai::TaskDecorator::DecoratorType::Limit:
			Lua::PushString(l,"ai.BEHAVIOR_TASK_DECORATOR_TYPE_LIMIT");
			break;
		case pragma::ai::TaskDecorator::DecoratorType::Repeat:
			Lua::PushString(l,"ai.BEHAVIOR_TASK_DECORATOR_TYPE_REPEAT");
			break;
		case pragma::ai::TaskDecorator::DecoratorType::UntilFail:
			Lua::PushString(l,"ai.BEHAVIOR_TASK_DECORATOR_TYPE_UNTIL_FAIL");
			break;
		case pragma::ai::TaskDecorator::DecoratorType::UntilSuccess:
			Lua::PushString(l,"ai.BEHAVIOR_TASK_DECORATOR_TYPE_UNTIL_SUCCESS");
			break;
		default:
			Lua::PushString(l,"");
			break;
	}
	return 1;
}

int Lua::debug::Server::behavior_task_result_to_string(lua_State *l)
{
	auto v = Lua::CheckInt(l,1);
	switch(static_cast<pragma::ai::BehaviorNode::Result>(v))
	{
		case pragma::ai::BehaviorNode::Result::Pending:
			Lua::PushString(l,"ai.BEHAVIOR_TASK_RESULT_PENDING");
			break;
		case pragma::ai::BehaviorNode::Result::Failed:
			Lua::PushString(l,"ai.BEHAVIOR_TASK_RESULT_FAILED");
			break;
		case pragma::ai::BehaviorNode::Result::Succeeded:
			Lua::PushString(l,"ai.BEHAVIOR_TASK_RESULT_SUCCEEDED");
			break;
		default:
			Lua::PushString(l,"");
			break;
	}
	return 1;
}

int Lua::debug::Server::behavior_task_type_to_string(lua_State *l)
{
	auto v = Lua::CheckInt(l,1);
	switch(static_cast<pragma::ai::BehaviorNode::Type>(v))
	{
		case pragma::ai::BehaviorNode::Type::Selector:
			Lua::PushString(l,"ai.BEHAVIOR_TASK_TYPE_SELECTOR");
			break;
		case pragma::ai::BehaviorNode::Type::Sequence:
			Lua::PushString(l,"ai.BEHAVIOR_TASK_TYPE_SEQUENCE");
			break;
		default:
			Lua::PushString(l,"");
			break;
	}
	return 1;
}

int Lua::debug::Server::disposition_to_string(lua_State *l)
{
	auto v = Lua::CheckInt(l,1);
	switch(static_cast<DISPOSITION>(v))
	{
		case DISPOSITION::HATE:
			Lua::PushString(l,"ai.DISPOSITION_HATE");
			break;
		case DISPOSITION::FEAR:
			Lua::PushString(l,"ai.DISPOSITION_FEAR");
			break;
		case DISPOSITION::NEUTRAL:
			Lua::PushString(l,"ai.DISPOSITION_NEUTRAL");
			break;
		case DISPOSITION::LIKE:
			Lua::PushString(l,"ai.DISPOSITION_LIKE");
			break;
		default:
			Lua::PushString(l,"");
			break;
	}
	return 1;
}

int Lua::debug::Server::memory_type_to_string(lua_State *l)
{
	auto v = Lua::CheckInt(l,1);
	switch(static_cast<pragma::ai::Memory::MemoryType>(v))
	{
		case pragma::ai::Memory::MemoryType::Visual:
			Lua::PushString(l,"ai.MEMORY_TYPE_VISUAL");
			break;
		case pragma::ai::Memory::MemoryType::Sound:
			Lua::PushString(l,"ai.MEMORY_TYPE_SOUND");
			break;
		case pragma::ai::Memory::MemoryType::Smell:
			Lua::PushString(l,"ai.MEMORY_TYPE_SMELL");
			break;
		default:
			Lua::PushString(l,"");
			break;
	}
	return 1;
}

int Lua::debug::Server::npc_state_to_string(lua_State *l)
{
	auto v = Lua::CheckInt(l,1);
	switch(static_cast<NPCSTATE>(v))
	{
		case NPCSTATE::NONE:
			Lua::PushString(l,"ai.NPC_STATE_NONE");
			break;
		case NPCSTATE::IDLE:
			Lua::PushString(l,"ai.NPC_STATE_IDLE");
			break;
		case NPCSTATE::ALERT:
			Lua::PushString(l,"ai.NPC_STATE_ALERT");
			break;
		case NPCSTATE::COMBAT:
			Lua::PushString(l,"ai.NPC_STATE_COMBAT");
			break;
		case NPCSTATE::SCRIPT:
			Lua::PushString(l,"ai.NPC_STATE_SCRIPT");
			break;
		default:
			Lua::PushString(l,"");
			break;
	}
	return 1;
}

int Lua::debug::Server::task_to_string(lua_State *l)
{
	auto v = Lua::CheckInt(l,1);
	switch(static_cast<::pragma::ai::Task>(v))
	{
		case ::pragma::ai::Task::MoveToTarget:
			Lua::PushString(l,"ai.TASK_MOVE_TO_TARGET");
			break;
		case ::pragma::ai::Task::PlayAnimation:
			Lua::PushString(l,"ai.TASK_PLAY_ANIMATION");
			break;
		case ::pragma::ai::Task::PlayActivity:
			Lua::PushString(l,"ai.TASK_PLAY_ACTIVITY");
			break;
		case ::pragma::ai::Task::PlayLayeredAnimation:
			Lua::PushString(l,"ai.TASK_PLAY_LAYERED_ANIMATION");
			break;
		case ::pragma::ai::Task::PlayLayeredActivity:
			Lua::PushString(l,"ai.TASK_PLAY_LAYERED_ACTIVITY");
			break;
		case ::pragma::ai::Task::MoveRandom:
			Lua::PushString(l,"ai.TASK_MOVE_RANDOM");
			break;
		case ::pragma::ai::Task::PlaySound:
			Lua::PushString(l,"ai.TASK_PLAY_SOUND");
			break;
		case ::pragma::ai::Task::DebugPrint:
			Lua::PushString(l,"ai.TASK_DEBUG_PRINT");
			break;
		case ::pragma::ai::Task::DebugDrawText:
			Lua::PushString(l,"ai.TASK_DEBUG_DRAW_TEXT");
			break;
		case ::pragma::ai::Task::Decorator:
			Lua::PushString(l,"ai.TASK_DECORATOR");
			break;
		case ::pragma::ai::Task::Wait:
			Lua::PushString(l,"ai.TASK_WAIT");
			break;
		case ::pragma::ai::Task::TurnToTarget:
			Lua::PushString(l,"ai.TASK_TURN_TO_TARGET");
			break;
		default:
			Lua::PushString(l,"");
			break;
	}
	return 1;
}
