// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/math/octtreenode.h"

BaseOctTreeNode::BaseOctTreeNode(BaseOctTreeNode *parent) : m_activeNodes(0), m_parent(parent) {}

BaseOctTreeNode *BaseOctTreeNode::GetParent() { return m_parent; }

bool BaseOctTreeNode::HasChildren() { return (m_activeNodes != 0) ? true : false; }
