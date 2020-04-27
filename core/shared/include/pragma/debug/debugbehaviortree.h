/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __DEBUGBEHAVIORTREE_H__
#define __DEBUGBEHAVIORTREE_H__

#include "pragma/networkdefinitions.h"
#include <string>
#include <vector>
#include <memory>

struct DLLNETWORK DebugBehaviorTreeNode
{
	enum class DLLNETWORK State : uint32_t
	{
		Initial = std::numeric_limits<std::underlying_type_t<State>>::max(),
		Invalid = Initial -1,
		Pending = 0,
		Failed,
		Succeeded
	};
	enum class DLLNETWORK BehaviorNodeType : uint32_t
	{
		Selector = 0,
		Sequence
	};
	enum class DLLNETWORK SelectorType : uint32_t
	{
		Sequential = 0,
		RandomShuffle
	};
	std::string name;
	BehaviorNodeType nodeType = BehaviorNodeType::Selector;
	SelectorType selectorType = SelectorType::Sequential;
	float lastStartTime = 0.f;
	float lastEndTime = 0.f;
	float lastUpdate = 0.f;
	uint64_t executionIndex = 0ull;
	bool active = false;
	State state = State::Pending;
	std::vector<std::shared_ptr<DebugBehaviorTreeNode>> children;
};

DLLNETWORK bool operator==(const DebugBehaviorTreeNode &a,const DebugBehaviorTreeNode &b);
DLLNETWORK bool operator!=(const DebugBehaviorTreeNode &a,const DebugBehaviorTreeNode &b);

#endif
