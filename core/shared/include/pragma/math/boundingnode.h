// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __BOUNDINGNODE_H__
#define __BOUNDINGNODE_H__

#include "pragma/networkdefinitions.h"
#include <mathutil/glmutil.h>

class DLLNETWORK BoundingNode {
  protected:
	Vector3 m_min = {};
	Vector3 m_max = {};
  public:
	BoundingNode(const Vector3 &min, const Vector3 &max);
	void GetBounds(Vector3 *min, Vector3 *max);
	Vector3 &GetMin();
	Vector3 &GetMax();
};

#endif
