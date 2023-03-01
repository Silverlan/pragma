/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_POLYMESH_H__
#define __C_POLYMESH_H__
#include "pragma/clientdefinitions.h"
#include "pragma/model/polymesh.h"
#include "pragma/model/brush/c_brushmesh.h"
#include "pragma/model/c_polymesh.h"
#include "pragma/model/c_poly.h"
#include "pragma/model/c_side.h"

class DLLCLIENT CPolyMesh : public PolyMesh {
	friend Con::c_cout &operator<<(Con::c_cout &, const CPolyMesh &);
  private:
	std::vector<CPoly *> m_polys;
  public:
	static void GenerateBrushMeshes(pragma::physics::IEnvironment &env, std::vector<std::shared_ptr<CBrushMesh>> &outBrushMeshes, const std::vector<std::shared_ptr<CPolyMesh>> &meshes);
};

inline Con::c_cout &operator<<(Con::c_cout &os, const CPolyMesh &mesh)
{
	os << static_cast<PolyMesh>(mesh);
	return os;
}

inline void CPolyMesh::GenerateBrushMeshes(pragma::physics::IEnvironment &env, std::vector<std::shared_ptr<CBrushMesh>> &outBrushMeshes, const std::vector<std::shared_ptr<CPolyMesh>> &meshes)
{
	return PolyMesh::GenerateBrushMeshes<CSide, CPolyMesh, CBrushMesh>(env, outBrushMeshes, meshes);
}

#endif
