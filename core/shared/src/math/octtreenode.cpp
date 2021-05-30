/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/math/octtreenode.h"

BaseOctTreeNode::BaseOctTreeNode(BaseOctTreeNode *parent)
	: m_activeNodes(0),m_parent(parent)
{}

BaseOctTreeNode *BaseOctTreeNode::GetParent() {return m_parent;}

bool BaseOctTreeNode::HasChildren() {return (m_activeNodes != 0) ? true : false;}