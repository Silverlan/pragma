/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_NORMALMESH_H__
#define __C_NORMALMESH_H__

#include "pragma/clientdefinitions.h"
#include <mathutil/glmutil.h>
#include <vector>
#include <pragma/model/vertex.h>

class DLLCLIENT NormalMesh
{
protected:
	void ComputeTangentBasis(std::vector<Vertex> &verts,std::vector<uint16_t> &triangles);
public:
	NormalMesh();
};

#endif