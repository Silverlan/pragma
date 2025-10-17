// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"


export module pragma.shared:math.bounding_node;

export class DLLNETWORK BoundingNode {
  protected:
	Vector3 m_min = {};
	Vector3 m_max = {};
  public:
	BoundingNode(const Vector3 &min, const Vector3 &max);
	void GetBounds(Vector3 *min, Vector3 *max);
	Vector3 &GetMin();
	Vector3 &GetMax();
};
