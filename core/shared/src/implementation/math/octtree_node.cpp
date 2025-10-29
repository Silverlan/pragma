// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <ostream>

#include <string>

module pragma.shared;

import :math.octtree_node;

BaseOctTreeNode::BaseOctTreeNode(BaseOctTreeNode *parent) : m_activeNodes(0), m_parent(parent) {}

BaseOctTreeNode *BaseOctTreeNode::GetParent() { return m_parent; }

bool BaseOctTreeNode::HasChildren() { return (m_activeNodes != 0) ? true : false; }
