// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <string>
#include <mathutil/uvec.h>
#include <vector>
#include <iostream>
#include <mathutil/glmutil.h>
#include "material.h"

export module pragma.shared:model.poly;

import :console.output;

export {
	class NetworkState;
	struct DLLNETWORK PolyVertex {
		Vector3 pos = {};
		Vector3 normal = {};
		double u = 0.0;
		double v = 0.0;
		PolyVertex() = default;
		PolyVertex(Vector3 _pos, Vector3 _normal)
		{
			PolyVertex();
			pos = _pos;
			normal = _normal;
		}
	};

	struct DLLNETWORK TextureData {
		std::string texture;
		Vector3 nu = {};
		Vector3 nv = {};
		float ou = 0.f;
		float ov = 0.f;
		float su = 0.f;
		float sv = 0.f;
		float rot = 0.f;
	};

	struct DLLNETWORK DispInfo {
		uint32_t power;
		uint32_t flags;
		uint32_t elevation;
		uint32_t subdiv;
		uint32_t startpositionId;
		Vector3 startposition;
		std::vector<std::vector<Vector3>> normals;
		std::vector<std::vector<float>> distances;
		std::vector<std::vector<Vector3>> offsets;
		std::vector<std::vector<Vector2>> alphas;
	};

	class DLLNETWORK PolyDispInfo {
	public:
		PolyDispInfo();
		std::vector<Vector3> vertices;
		std::vector<Vector3> normals;
		std::vector<Vector2> uvs;
		std::vector<uint32_t> triangles;
		std::vector<Vector2> alphas;
		std::vector<Vector3> faceNormals;
		Vector3 center;
		uint8_t numAlpha;
		uint32_t power;
		std::vector<uint32_t> powersMerged;
	};

	class DLLNETWORK PolyInfo {
	public:
		PolyInfo();
		Vector3 nu;
		Vector3 nv;
		float width;
		float height;
		std::unique_ptr<PolyDispInfo> displacement;
	};

	const short CLASSIFY_FRONT = 0;
	const short CLASSIFY_BACK = 1;
	const short CLASSIFY_ONPLANE = 2;
	const short CLASSIFY_SPANNING = 3;

	class DLLNETWORK Poly {
		friend Con::c_cout &operator<<(Con::c_cout &, const Poly &);
	public:
		Poly(NetworkState *nw);
	protected:
		NetworkState *m_nwState = nullptr;
		std::vector<PolyVertex> m_vertices;
		Vector3 m_normal = {};
		Vector3 m__normal = {};
		Vector3 m_center = {};
		std::optional<Vector3> m_centerLocalized = {};
		Vector3 m_min = {};
		Vector3 m_max = {};
		Vector3 m_centerOfMass = {};
		PolyInfo m_compiledData;
		std::shared_ptr<DispInfo> m_displacement;
		std::unique_ptr<TextureData> m_texData = nullptr;
		Material *m_material = nullptr;
		double m_distance = 0.0;
		bool CalculatePlane();
		void CalculateBounds();
		void ReverseVertexOrder();
		void Calculate();
		void CalculateNormal();
	public:
		PolyInfo &GetCompiledData();
		void SetDistance(double d);
		void SetNormal(Vector3 n);
		double GetDistance();
		Vector3 GetNormal();
		bool AddUniqueVertex(Vector3 vert, Vector3 n);
		void AddVertex(Vector3 vert, Vector3 n);
		bool HasVertex(Vector3 *vert);
		unsigned int GetVertexCount();
		bool IsValid();
		virtual void SortVertices();
		Vector3 GetCenter();
		std::vector<PolyVertex> &GetVertices();
		void GetBounds(Vector3 *min, Vector3 *max);
		void Localize(const Vector3 &center);
		void debug_print();
		bool GenerateTriangleMesh(std::vector<uint16_t> *triangles, std::vector<glm::vec2> *uvs, std::vector<Vector3> *normals);
		virtual void SetTextureData(std::string texture, Vector3 nu, Vector3 nv, float ou, float ov, float su, float sv, float rot = 0);
		TextureData *GetTextureData();
		virtual Material *GetMaterial();
		virtual void SetMaterial(Material *material);
		Vector3 *GetWorldPosition();
		const short ClassifyPoint(Vector3 *point);
		Vector3 GetCalculatedNormal();
		void ToTriangles(std::vector<Vector3> *vertices);

		void SetDisplacement(std::shared_ptr<DispInfo> disp);
		std::shared_ptr<DispInfo> GetDisplacement();
		bool IsDisplacement();
		void RemoveDisplacement();
		void BuildDisplacement(std::vector<Vector3> &verts, Vector3 &nu, Vector3 &nv, unsigned int w, unsigned int h, std::vector<Vector3> &outVertices, std::vector<glm::vec2> &outUvs, std::vector<unsigned int> &outTriangles, std::vector<Vector3> &faceNormals, unsigned char &numAlpha,
		std::vector<glm::vec2> *outAlphas);
	};
};
