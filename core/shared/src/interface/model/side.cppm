// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:model.side;

import :console.output;
export import pragma.materialsystem;

export {
	class DLLNETWORK Side {
	  public:
		Side();
	  public:
		void Update();
		void Merge(Side *other);
		msys::Material *GetMaterial();
		void SetMaterial(msys::Material *mat);
		void SetConvex(bool bConvex);
		bool IsConvex() const;
		unsigned int GetVertexCount() const;
		unsigned int GetTriangleVertexCount() const;
		void CalculateBounds();
		void GetBounds(Vector3 *min, Vector3 *max) const;
		const std::vector<Vector3> &GetVertices() const;
		std::vector<Vector3> &GetVertices();
		const std::vector<uint16_t> *GetTriangles(uint32_t lod = 0) const;
		std::vector<uint16_t> *GetTriangles(uint32_t lod = 0);
		const std::vector<Vector2> &GetUVMapping() const;
		std::vector<Vector2> &GetUVMapping();
		const std::vector<Vector3> &GetNormals() const;
		std::vector<Vector3> &GetNormals();
		const std::vector<Vector2> &GetAlphas() const;
		std::vector<Vector2> &GetAlphas();
		bool IsPointInside(Vector3 &p, double epsilon = 0) const;
		const Vector3 &GetNormal() const;
		float GetDistance() const;
		bool HasAlpha() const;
		unsigned char GetAlphaCount() const;
		virtual void SetDisplacementAlpha(unsigned char numAlpha);
	  protected:
		std::vector<std::vector<uint16_t>> m_triangles; // Vertices as triangles; Every index is an Lod
		std::vector<Vector2> m_alphas = {};
		std::vector<Vector3> m_vertices = {};
		std::vector<Vector2> m_uvs = {};
		std::vector<Vector3> m_normals = {};
		msys::Material *m_material = nullptr;
		Vector3 m_min = {};
		Vector3 m_max = {};
		Vector3 m_normal = {};
		float m_distance = 0.f;
		bool m_bConvex = false;
		unsigned char m_numAlphas = 0u;
	};

	DLLNETWORK Con::c_cout &operator<<(Con::c_cout &os, const Side side);
	DLLNETWORK std::ostream &operator<<(std::ostream &os, const Side side);
};
