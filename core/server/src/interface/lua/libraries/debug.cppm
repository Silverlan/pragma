// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:scripting.lua.libraries.debug;

import :ai;

export namespace Lua {
	namespace debug {
		namespace Server {
			DLLSERVER std::string behavior_selector_type_to_string(pragma::ai::SelectorType selectorType);
			DLLSERVER std::string behavior_task_decorator_type_to_string(pragma::ai::TaskDecorator::DecoratorType decoratorType);
			DLLSERVER std::string behavior_task_result_to_string(pragma::ai::BehaviorNode::Result result);
			DLLSERVER std::string behavior_task_type_to_string(pragma::ai::BehaviorNode::Type type);
			DLLSERVER std::string disposition_to_string(DISPOSITION disposition);
			DLLSERVER std::string memory_type_to_string(pragma::ai::Memory::MemoryType memoryType);
			DLLSERVER std::string npc_state_to_string(NPCSTATE npcState);
			DLLSERVER std::string task_to_string(pragma::ai::Task task);
		};
	};
};
