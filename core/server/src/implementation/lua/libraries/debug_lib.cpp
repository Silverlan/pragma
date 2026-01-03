// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :scripting.lua.libraries.debug;

import :server_state;

#undef PlaySound

std::string Lua::debug::Server::behavior_selector_type_to_string(pragma::ai::SelectorType selectorType)
{
	switch(selectorType) {
	case pragma::ai::SelectorType::Sequential:
		return "ai.BEHAVIOR_SELECTOR_TYPE_SEQUENTIAL";
	case pragma::ai::SelectorType::RandomShuffle:
		return "ai.BEHAVIOR_SELECTOR_TYPE_RANDOM_SHUFFLE";
	default:
		return "";
	}
}

std::string Lua::debug::Server::behavior_task_decorator_type_to_string(pragma::ai::TaskDecorator::DecoratorType decoratorType)
{
	switch(decoratorType) {
	case pragma::ai::TaskDecorator::DecoratorType::Inherit:
		return "ai.BEHAVIOR_TASK_DECORATOR_TYPE_INHERIT";
	case pragma::ai::TaskDecorator::DecoratorType::AlwaysFail:
		return "ai.BEHAVIOR_TASK_DECORATOR_TYPE_ALWAYS_FAIL";
	case pragma::ai::TaskDecorator::DecoratorType::AlwaysSucceed:
		return "ai.BEHAVIOR_TASK_DECORATOR_TYPE_ALWAYS_SUCCEED";
	case pragma::ai::TaskDecorator::DecoratorType::Invert:
		return "ai.BEHAVIOR_TASK_DECORATOR_TYPE_INVERT";
	case pragma::ai::TaskDecorator::DecoratorType::Limit:
		return "ai.BEHAVIOR_TASK_DECORATOR_TYPE_LIMIT";
	case pragma::ai::TaskDecorator::DecoratorType::Repeat:
		return "ai.BEHAVIOR_TASK_DECORATOR_TYPE_REPEAT";
	case pragma::ai::TaskDecorator::DecoratorType::UntilFail:
		return "ai.BEHAVIOR_TASK_DECORATOR_TYPE_UNTIL_FAIL";
	case pragma::ai::TaskDecorator::DecoratorType::UntilSuccess:
		return "ai.BEHAVIOR_TASK_DECORATOR_TYPE_UNTIL_SUCCESS";
	default:
		return "";
	}
}

std::string Lua::debug::Server::behavior_task_result_to_string(pragma::ai::BehaviorNode::Result result)
{
	switch(result) {
	case pragma::ai::BehaviorNode::Result::Pending:
		return "ai.BEHAVIOR_TASK_RESULT_PENDING";
	case pragma::ai::BehaviorNode::Result::Failed:
		return "ai.BEHAVIOR_TASK_RESULT_FAILED";
	case pragma::ai::BehaviorNode::Result::Succeeded:
		return "ai.BEHAVIOR_TASK_RESULT_SUCCEEDED";
	default:
		return "";
	}
}

std::string Lua::debug::Server::behavior_task_type_to_string(pragma::ai::BehaviorNode::Type type)
{
	switch(type) {
	case pragma::ai::BehaviorNode::Type::Selector:
		return "ai.BEHAVIOR_TASK_TYPE_SELECTOR";
	case pragma::ai::BehaviorNode::Type::Sequence:
		return "ai.BEHAVIOR_TASK_TYPE_SEQUENCE";
	default:
		return "";
	}
}

std::string Lua::debug::Server::disposition_to_string(DISPOSITION disposition)
{
	switch(disposition) {
	case DISPOSITION::HATE:
		return "ai.DISPOSITION_HATE";
	case DISPOSITION::FEAR:
		return "ai.DISPOSITION_FEAR";
	case DISPOSITION::NEUTRAL:
		return "ai.DISPOSITION_NEUTRAL";
	case DISPOSITION::LIKE:
		return "ai.DISPOSITION_LIKE";
	default:
		return "";
	}
}

std::string Lua::debug::Server::memory_type_to_string(pragma::ai::Memory::MemoryType memoryType)
{
	switch(memoryType) {
	case pragma::ai::Memory::MemoryType::Visual:
		return "ai.MEMORY_TYPE_VISUAL";
	case pragma::ai::Memory::MemoryType::Sound:
		return "ai.MEMORY_TYPE_SOUND";
	case pragma::ai::Memory::MemoryType::Smell:
		return "ai.MEMORY_TYPE_SMELL";
	default:
		return "";
	}
}

std::string Lua::debug::Server::npc_state_to_string(NPCSTATE npcState)
{
	switch(npcState) {
	case NPCSTATE::NONE:
		return "ai.NPC_STATE_NONE";
	case NPCSTATE::IDLE:
		return "ai.NPC_STATE_IDLE";
	case NPCSTATE::ALERT:
		return "ai.NPC_STATE_ALERT";
	case NPCSTATE::COMBAT:
		return "ai.NPC_STATE_COMBAT";
	case NPCSTATE::SCRIPT:
		return "ai.NPC_STATE_SCRIPT";
	default:
		return "";
	}
}

std::string Lua::debug::Server::task_to_string(pragma::ai::Task task)
{
	switch(task) {
	case pragma::ai::Task::MoveToTarget:
		return "ai.TASK_MOVE_TO_TARGET";
	case pragma::ai::Task::PlayAnimation:
		return "ai.TASK_PLAY_ANIMATION";
	case pragma::ai::Task::PlayActivity:
		return "ai.TASK_PLAY_ACTIVITY";
	case pragma::ai::Task::PlayLayeredAnimation:
		return "ai.TASK_PLAY_LAYERED_ANIMATION";
	case pragma::ai::Task::PlayLayeredActivity:
		return "ai.TASK_PLAY_LAYERED_ACTIVITY";
	case pragma::ai::Task::MoveRandom:
		return "ai.TASK_MOVE_RANDOM";
	case pragma::ai::Task::PlaySound:
		return "ai.TASK_PLAY_SOUND";
	case pragma::ai::Task::DebugPrint:
		return "ai.TASK_DEBUG_PRINT";
	case pragma::ai::Task::DebugDrawText:
		return "ai.TASK_DEBUG_DRAW_TEXT";
	case pragma::ai::Task::Decorator:
		return "ai.TASK_DECORATOR";
	case pragma::ai::Task::Wait:
		return "ai.TASK_WAIT";
	case pragma::ai::Task::TurnToTarget:
		return "ai.TASK_TURN_TO_TARGET";
	default:
		return "";
	}
}
