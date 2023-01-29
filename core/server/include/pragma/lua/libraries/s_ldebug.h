/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_LDEBUG_H__
#define __S_LDEBUG_H__

#include "pragma/serverdefinitions.h"
#include <pragma/lua/ldefinitions.h>
#include "pragma/ai/ai_behavior.h"
#include "pragma/ai/ai_task_decorator.h"
#include "pragma/ai/ai_memory.h"
#include "pragma/ai/s_disposition.h"
#include "pragma/ai/s_npcstate.h"
#include "pragma/ai/ai_task.h"

namespace Lua {
	namespace debug {
		namespace Server {
			DLLSERVER std::string behavior_selector_type_to_string(pragma::ai::SelectorType selectorType);
			DLLSERVER std::string behavior_task_decorator_type_to_string(pragma::ai::TaskDecorator::DecoratorType decoratorType);
			DLLSERVER std::string behavior_task_result_to_string(pragma::ai::BehaviorNode::Result result);
			DLLSERVER std::string behavior_task_type_to_string(pragma::ai::BehaviorNode::Type type);
			DLLSERVER std::string disposition_to_string(DISPOSITION disposition);
			DLLSERVER std::string memory_type_to_string(pragma::ai::Memory::MemoryType memoryType);
			DLLSERVER std::string npc_state_to_string(NPCSTATE npcState);
			DLLSERVER std::string task_to_string(::pragma::ai::Task task);
		};
	};
};

#endif
