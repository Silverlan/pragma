/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

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
