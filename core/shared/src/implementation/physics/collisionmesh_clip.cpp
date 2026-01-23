// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :physics.collision_mesh;

static Vector3 intersect_line_plane(const Vector3 &a, const Vector3 &b, float da, float db) { return a + da / (da - db) * (b - a); }

static uint8_t clip_triangle(const std::function<void(uint16_t)> &fAddTriangleIndex, const std::function<void(const Vector3 &)> &fAddNewVertex, const Vector3 &C, const std::vector<Vector3> &verts, const std::array<uint16_t, 3> &indices, const std::array<double, 3> &distances)
{
	const auto d0 = distances.at(0);
	const auto d1 = distances.at(1);
	const auto d2 = distances.at(2);

	const auto &v0 = verts.at(indices.at(0));
	const auto &v1 = verts.at(indices.at(1));
	const auto &v2 = verts.at(indices.at(2));

	auto ab = intersect_line_plane(v0, v1, d0, d1);

	if(d0 < 0.0) {
		if(d2 < 0.0) {
			auto bc = intersect_line_plane(v1, v2, d1, d2);

			fAddNewVertex(bc);
			fAddTriangleIndex(indices.at(2));
			fAddTriangleIndex(indices.at(0));
			fAddNewVertex(ab);
			fAddNewVertex(bc);
			fAddTriangleIndex(indices.at(0));
			return 2;
		}
		else {
			auto ac = intersect_line_plane(v0, v2, d0, d2);

			fAddTriangleIndex(indices.at(0));
			fAddNewVertex(ab);
			fAddNewVertex(ac);
			return 1;
		}
	}
	else {
		if(d2 < 0.0) {
			auto ac = intersect_line_plane(v0, v2, d0, d2);

			fAddNewVertex(ac);
			fAddNewVertex(ab);
			fAddTriangleIndex(indices.at(1));
			fAddTriangleIndex(indices.at(1));
			fAddTriangleIndex(indices.at(2));
			fAddNewVertex(ac);
			return 2;
		}
		else {
			auto bc = intersect_line_plane(v1, v2, d1, d2);

			fAddTriangleIndex(indices.at(1));
			fAddNewVertex(bc);
			fAddNewVertex(ab);
			return 1;
		}
	}
	return 0;
}

void pragma::physics::CollisionMesh::ClipAgainstPlane(const Vector3 &n, double d, CollisionMesh &clippedMesh)
{
	const auto &verts = GetVertices();
	const auto &triangles = GetTriangles();
	const auto &surfaceMaterials = GetSurfaceMaterials();

	auto C = n * static_cast<float>(d);
	auto &clipMeshVerts = clippedMesh.GetVertices();
	auto &clipMeshTriangles = clippedMesh.GetTriangles();
	auto &clipSurfaceMaterials = clippedMesh.GetSurfaceMaterials();
	clipMeshVerts.reserve(verts.size());
	clipMeshTriangles.reserve(triangles.size());
	clipSurfaceMaterials.reserve(triangles.size() / 3);
	std::vector<uint32_t> translationTable(verts.size(), std::numeric_limits<uint32_t>::max());
	auto fAddTriangleIndex = [&translationTable, &triangles, &verts, &clipMeshVerts, &clipMeshTriangles](uint16_t idx) {
		if(translationTable.at(idx) == std::numeric_limits<uint32_t>::max()) {
			translationTable.at(idx) = clipMeshVerts.size();
			clipMeshVerts.push_back(verts.at(idx));
		}
		clipMeshTriangles.push_back(translationTable.at(idx));
	};
	auto fAddNewVertex = [&clipMeshTriangles, &clipMeshVerts](const Vector3 &v) {
		clipMeshTriangles.push_back(clipMeshVerts.size());
		clipMeshVerts.push_back(v);
	};
	if(triangles.empty() == false) {
		for(auto i = decltype(triangles.size()) {0}; i < triangles.size(); i += 3) {
			auto idx0 = triangles.at(i);
			auto idx1 = triangles.at(i + 1);
			auto idx2 = triangles.at(i + 2);

			auto &v0 = verts.at(idx0);
			auto &v1 = verts.at(idx1);
			auto &v2 = verts.at(idx2);

			auto d0 = uvec::dot(v0, n) - d;
			auto d1 = uvec::dot(v1, n) - d;
			auto d2 = uvec::dot(v2, n) - d;

			uint8_t numTrianglesAdded = 0;
			// a to b crosses the clipping plane
			if(d0 * d1 < 0.0)
				numTrianglesAdded = clip_triangle(fAddTriangleIndex, fAddNewVertex, C, verts, {idx0, idx1, idx2}, {d0, d1, d2});
			// a to c crosses the clipping plane
			else if(d0 * d2 < 0.0)
				numTrianglesAdded = clip_triangle(fAddTriangleIndex, fAddNewVertex, C, verts, {idx2, idx0, idx1}, {d2, d0, d1});
			// b to c crosses the clipping plane
			else if(d1 * d2 < 0.0)
				numTrianglesAdded = clip_triangle(fAddTriangleIndex, fAddNewVertex, C, verts, {idx1, idx2, idx0}, {d1, d2, d0});
			// Full clipping plane intersection; keep the whole triangle
			else if(d0 < 0.0 || d1 < 0.0 || d2 < 0.0) {
				fAddTriangleIndex(idx0);
				fAddTriangleIndex(idx1);
				fAddTriangleIndex(idx2);
				numTrianglesAdded = 1;
			}
			if(surfaceMaterials.empty() == false) {
				auto surfMatId = surfaceMaterials.at(i / 3);
				for(auto j = decltype(numTrianglesAdded) {0}; j < numTrianglesAdded; ++j)
					clipSurfaceMaterials.push_back(surfMatId);
			}
		}
	}
	else {
		// No triangles available; Assume this is a point cloud mesh. All clipped
		// points will be projected onto the plane.
		clippedMesh.m_vertices = verts;
		auto bAllProjected = true;
		for(auto &v : clippedMesh.m_vertices) {
			if(math::geometry::get_side_of_point_to_plane(n, d, v) != math::geometry::PlaneSide::Back)
				v = uvec::project_to_plane(v, n, d);
			else
				bAllProjected = false;
		}
		if(bAllProjected == true) // Mesh has no volume
			clippedMesh.m_vertices.clear();
	}
	clippedMesh.m_game = m_game;
	clippedMesh.m_min = m_min;
	clippedMesh.m_max = m_max;
	clippedMesh.m_origin = m_origin;
	clippedMesh.m_bConvex = m_bConvex;
	clippedMesh.m_boneID = m_boneID;
	clippedMesh.m_surfaceMaterialId = m_surfaceMaterialId;
	clippedMesh.m_centerOfMass = m_centerOfMass;
	clippedMesh.m_volume = m_volume;
}
void pragma::physics::CollisionMesh::ClipAgainstPlane(const Vector3 &n, double d, CollisionMesh &clippedMeshA, CollisionMesh &clippedMeshB)
{
	ClipAgainstPlane(n, d, clippedMeshA);
	ClipAgainstPlane(-n, -d, clippedMeshB);
}
