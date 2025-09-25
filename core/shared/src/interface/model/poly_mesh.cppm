// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/model/poly.h"
#include "pragma/model/brush/brushmesh.h"
#include "pragma/model/model.h"
#include "pragma/model/side.h"
#include <vector>

#include <mathutil/glmutil.h>
#include <iostream>

#include <pragma/math/intersection.h>

export module pragma.shared:model.poly_mesh;

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
		bool HasVertex(const PolyVertex &v, Vector3 *vThis = NULL);
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
		std::shared_ptr<Model> GenerateModel();
		template<class TPoly, class TPolyMesh>
		static Model *GenerateModel(const std::vector<std::shared_ptr<TPolyMesh>> &meshes);
		template<class TSide, class TPolyMesh, class TBrushMesh>
		static void GenerateBrushMeshes(pragma::physics::IEnvironment &env, std::vector<std::shared_ptr<TBrushMesh>> &outBrushMeshes, const std::vector<std::shared_ptr<TPolyMesh>> &meshes);
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

	template<class TPoly, class TPolyMesh>
	Model *PolyMesh::GenerateModel(const std::vector<std::shared_ptr<TPolyMesh>> &meshes) // Obsolete?
	{
		return nullptr;
		/*Model *mdl = new Model();
		for(int i=0;i<meshes.size();i++)
		{
			std::vector<TPoly*> *polys;
			meshes[i]->GetPolys(&polys);
			CollisionMesh *meshCol = new CollisionMesh;
			for(int j=0;j<polys->size();j++)
			{
				TPoly *poly = (*polys)[j];
				std::vector<Vertex*> *polyVerts = poly->GetVertices();
				std::vector<Vector3> *vertexList = new std::vector<Vector3>;
				for(int k=0;k<polyVerts->size();k++)
					vertexList->push_back((*polyVerts)[k]->pos);
				std::vector<Vector3> *verts = new std::vector<Vector3>;
				std::vector<Vector2> *uvs = new std::vector<Vector2>;
				std::vector<Vector3> *normals = new std::vector<Vector3>;
				(*polys)[j]->GenerateTriangleMesh(verts,uvs,normals);
				Material *mat = (*polys)[j]->GetMaterial();

				Side *side = new Side(vertexList,verts,uvs,normals,mat);
				mdl->AddSide(side);
				for(int k=0;k<verts->size();k++)
					meshCol->vertices.push_back((*verts)[k]);
			}
			meshCol->CalculateBounds();
			mdl->AddCollisionMesh(meshCol);
		}
		mdl->CalculateBounds();
		return mdl;*/
	}

	template<class TSide, class TPolyMesh, class TBrushMesh>
	void PolyMesh::GenerateBrushMeshes(pragma::physics::IEnvironment &env, std::vector<std::shared_ptr<TBrushMesh>> &outBrushMeshes, const std::vector<std::shared_ptr<TPolyMesh>> &meshes) // Obsolete?
	{
		for(int i = 0; i < meshes.size(); i++) {
			auto mesh = std::make_shared<TBrushMesh>();
			auto &polys = meshes[i]->GetPolys();
			for(int j = 0; j < polys.size(); j++) {
				auto &poly = polys[j];
				auto &polyVerts = poly->GetVertices();
				/*std::vector<Vector3> *vertexList = new std::vector<Vector3>;
				for(int i=0;i<polyVerts.size();i++)
					vertexList->push_back(polyVerts[i].pos);*/
				//poly->GenerateTriangleMesh(verts,uvs,normals);

				//Material *mat = poly->GetMaterial();

				//TSide *side = new TSide(vertexList,verts,uvs,normals,mat);
				//mesh->AddSide(side);
			}
			mesh->Calculate(env);
			outBrushMeshes.push_back(mesh);
		}
	}
};
