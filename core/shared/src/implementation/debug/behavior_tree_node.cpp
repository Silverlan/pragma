// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.shared;

import :debug.behavior_tree_node;

bool operator==(const pragma::debug::DebugBehaviorTreeNode &a, const pragma::debug::DebugBehaviorTreeNode &b)
{
	if(a.name != b.name || /*a.startTime != b.startTime || a.endTime != b.endTime ||*/ a.state != b.state || a.children.size() != b.children.size())
		return false;
	auto numChildren = a.children.size();
	for(auto i = decltype(numChildren) {0}; i < numChildren; ++i) {
		auto r = ((*a.children[i]) == (*b.children[i])) ? true : false;
		if(r == false)
			return false;
	}
	return true;
}
bool operator!=(const pragma::debug::DebugBehaviorTreeNode &a, const pragma::debug::DebugBehaviorTreeNode &b) { return (a == b) ? false : true; }
