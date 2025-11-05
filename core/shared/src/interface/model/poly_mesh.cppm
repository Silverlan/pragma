// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:model.poly_mesh;

export import :model.poly;
export import :types;

export {
	class DLLNETWORK PolyMeshInfo {
	  public:
		std::vector<Vector3> vertexList;
	};

	class DLLNETWORK PolyMesh {
		friend Con::c_cout &operator<<(Con::c_cout &, const PolyMesh &);
	  public:
		PolyMesh();
	  private:
		bool m_bHasDisplacements = false;
		std::vector<std::shared_ptr<Poly>> m_polys;
		PolyMeshInfo m_compiledData = {};
		Vector3 m_min = {};
		Vector3 m_max = {};
		Vector3 m_center = {};
		std::optional<Vector3> m_centerLocalized = {};
		Vector3 m_centerOfMass = {};
		std::vector<Vector3> m_vertices;
		bool m_bValid = true;
		bool HasVertex(const PolyVertex &v, Vector3 *vThis = nullptr);
		void Localize();
		void Localize(const Vector3 &center);
	  public:
		PolyMeshInfo &GetCompiledData();
		bool HasDisplacements();
		void SetHasDisplacements(bool b);
		void AddPoly(std::shared_ptr<Poly> poly);
		int BuildPolyMesh();
		void Calculate();
		unsigned int GetPolyCount();
		void CenterPolys();
		const std::vector<std::shared_ptr<Poly>> &GetPolys() const;
		std::vector<std::shared_ptr<Poly>> &GetPolys();
		void debug_print();
		std::shared_ptr<pragma::Model> GenerateModel();
		template<class TSide, class TPolyMesh, class TBrushMesh>
		void GetBounds(Vector3 *min, Vector3 *max);
		Vector3 GetCenter();
		Vector3 *GetLocalizedCenter();
		Vector3 *GetWorldPosition();

		void Validate();
		bool IsValid();
	};

	inline Con::c_cout &operator<<(Con::c_cout &os, const PolyMesh &mesh)
	{
		os << "Mesh[" << &mesh << "] [" << mesh.m_polys.size() << "] [" << mesh.m_vertices.size() << "]";
		return os;
	}
};
