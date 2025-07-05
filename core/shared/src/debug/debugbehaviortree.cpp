// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/debug/debugbehaviortree.h"

bool operator==(const DebugBehaviorTreeNode &a, const DebugBehaviorTreeNode &b)
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
bool operator!=(const DebugBehaviorTreeNode &a, const DebugBehaviorTreeNode &b) { return (a == b) ? false : true; }
