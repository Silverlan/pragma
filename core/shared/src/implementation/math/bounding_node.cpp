// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :math.bounding_node;

BoundingNode::BoundingNode(const Vector3 &min, const Vector3 &max) : m_min(min), m_max(max) {}

void BoundingNode::GetBounds(Vector3 *min, Vector3 *max)
{
	*min = m_min;
	*max = m_max;
}

Vector3 &BoundingNode::GetMin() { return m_min; }
Vector3 &BoundingNode::GetMax() { return m_max; }
