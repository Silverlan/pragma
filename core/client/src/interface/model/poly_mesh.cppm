// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/model/polymesh.h"
#include "pragma/model/brush/c_brushmesh.h"
#include "pragma/model/c_poly.h"
#include "pragma/model/c_side.h"

export module pragma.client.model:poly_mesh;

export {
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
};
