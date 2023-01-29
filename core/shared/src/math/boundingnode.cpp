/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/math/boundingnode.h"

BoundingNode::BoundingNode(const Vector3 &min, const Vector3 &max) : m_min(min), m_max(max) {}

void BoundingNode::GetBounds(Vector3 *min, Vector3 *max)
{
	*min = m_min;
	*max = m_max;
}

Vector3 &BoundingNode::GetMin() { return m_min; }
Vector3 &BoundingNode::GetMax() { return m_max; }
