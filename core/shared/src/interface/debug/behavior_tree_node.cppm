// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:debug.behavior_tree_node;

export import std.compat;

export namespace pragma::debug {
	struct DLLNETWORK DebugBehaviorTreeNode {
		enum class DLLNETWORK State : uint32_t { Initial = std::numeric_limits<std::underlying_type_t<State>>::max(), Invalid = Initial - 1, Pending = 0, Failed, Succeeded };
		enum class DLLNETWORK BehaviorNodeType : uint32_t { Selector = 0, Sequence };
		enum class DLLNETWORK SelectorType : uint32_t { Sequential = 0, RandomShuffle };
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
};

export {
	DLLNETWORK bool operator==(const pragma::debug::DebugBehaviorTreeNode &a, const pragma::debug::DebugBehaviorTreeNode &b);
	DLLNETWORK bool operator!=(const pragma::debug::DebugBehaviorTreeNode &a, const pragma::debug::DebugBehaviorTreeNode &b);
}
