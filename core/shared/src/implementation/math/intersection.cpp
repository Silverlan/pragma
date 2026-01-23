// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :math.intersection;

bool pragma::math::intersection::line_with_mesh(const Vector3 &_start, const Vector3 &_dir, asset::Model &mdl, LineMeshResult &r, bool precise, const std::vector<uint32_t> *bodyGroups, uint32_t lod, const Vector3 &origin, const Quat &rot)
{
	auto start = _start;
	auto dir = _dir;
	uvec::world_to_local(origin, rot, start);
	uvec::rotate(&dir, uquat::get_inverse(rot));

	r.precise = r.precise ? r.precise : pragma::util::make_shared<LineMeshResult::Precise>();
	std::vector<std::shared_ptr<pragma::geometry::ModelMesh>> meshes;
	auto hasFoundBetterCandidate = false;
	if(bodyGroups == nullptr) {
		meshes.clear();
		mdl.GetBodyGroupMeshes({}, lod, meshes);
		for(auto i = decltype(meshes.size()) {0u}; i < meshes.size(); ++i) {
			auto &mesh = meshes.at(i);
			if(line_with_mesh(start, dir, *mesh, r, precise, nullptr, nullptr) == false)
				continue;
			hasFoundBetterCandidate = true;
			r.precise->meshGroup = mdl.GetMeshGroup(0);
			r.precise->meshGroupIndex = 0;
			r.precise->mesh = mesh;
			r.precise->meshIdx = i;
			if(precise == false && r.result == Result::Intersect)
				return true;
		}
		return hasFoundBetterCandidate;
	}
	for(auto outMeshGroupIdx : *bodyGroups) {
		meshes.clear();
		mdl.GetBodyGroupMeshes({outMeshGroupIdx}, lod, meshes);
		for(auto i = decltype(meshes.size()) {0u}; i < meshes.size(); ++i) {
			auto &mesh = meshes.at(i);
			if(line_with_mesh(start, dir, *mesh, r, precise, nullptr, nullptr) == false)
				continue;
			hasFoundBetterCandidate = true;
			r.precise->meshGroupIndex = outMeshGroupIdx;
			r.precise->meshGroup = mdl.GetMeshGroup(outMeshGroupIdx);
			r.precise->meshIdx = i;
			r.precise->mesh = mesh;
			if(precise == false && r.result == Result::Intersect)
				return true;
		}
	}
	return hasFoundBetterCandidate;
}
bool pragma::math::intersection::line_with_mesh(const Vector3 &start, const Vector3 &dir, asset::Model &mdl, LineMeshResult &r, bool precise, uint32_t lod, const Vector3 &origin, const Quat &rot) { return line_with_mesh(start, dir, mdl, r, precise, nullptr, lod, origin, rot); }
bool pragma::math::intersection::line_with_mesh(const Vector3 &start, const Vector3 &dir, asset::Model &mdl, LineMeshResult &r, bool precise, const std::vector<uint32_t> &bodyGroups, const Vector3 &origin, const Quat &rot) { return line_with_mesh(start, dir, mdl, r, precise, &bodyGroups, 0, origin, rot); }
bool pragma::math::intersection::line_with_mesh(const Vector3 &start, const Vector3 &dir, asset::Model &mdl, LineMeshResult &r, bool precise, const Vector3 &origin, const Quat &rot) { return line_with_mesh(start, dir, mdl, r, precise, 0, origin, rot); }

bool pragma::math::intersection::line_with_mesh(const Vector3 &_start, const Vector3 &_dir, pragma::geometry::ModelMesh &mesh, LineMeshResult &r, bool precise, const Vector3 *origin, const Quat *rot)
{
	auto start = _start;
	auto dir = _dir;
	if(origin != nullptr && rot != nullptr) {
		uvec::world_to_local(*origin, *rot, start);
		uvec::rotate(&dir, uquat::get_inverse(*rot));
	}

	r.precise = r.precise ? r.precise : pragma::util::make_shared<LineMeshResult::Precise>();
	auto &subMeshes = mesh.GetSubMeshes();
	auto hasFoundBetterCandidate = false;
	for(auto i = decltype(subMeshes.size()) {0u}; i < subMeshes.size(); ++i) {
		auto &subMesh = subMeshes.at(i);
		if(line_with_mesh(start, dir, *subMesh, r, precise, nullptr, nullptr) == false)
			continue;
		hasFoundBetterCandidate = true;
		r.precise->subMeshIdx = i;
		r.precise->subMesh = subMesh;
		if(precise == false && r.result == Result::Intersect)
			return true;
	}
	return hasFoundBetterCandidate;
}

static bool is_better_candidate(pragma::math::intersection::Result oldResult, pragma::math::intersection::Result newResult, float *tOld = nullptr, float *tNew = nullptr) { return newResult > oldResult || (tOld != nullptr && newResult == oldResult && tNew > tOld); }
static bool is_better_candidate(pragma::math::intersection::Result oldResult, pragma::math::intersection::Result newResult, float tOld, float tNew) { return is_better_candidate(oldResult, newResult, &tOld, &tNew); }
bool pragma::math::intersection::line_with_mesh(const Vector3 &_start, const Vector3 &_dir, pragma::geometry::ModelSubMesh &subMesh, LineMeshResult &r, bool precise, const Vector3 *origin, const Quat *rot)
{
	if(subMesh.GetGeometryType() != pragma::geometry::ModelSubMesh::GeometryType::Triangles || subMesh.GetTriangleCount() == 0)
		return false;
	auto start = _start;
	auto dir = _dir;
	if(origin != nullptr && rot != nullptr) {
		uvec::world_to_local(*origin, *rot, start);
		uvec::rotate(&dir, uquat::get_inverse(*rot));
	}

	Vector3 min, max;
	subMesh.GetBounds(min, max);
	if(uvec::distance_sqr(min, max) == 0.f)
		return false;
	for(uint8_t i = 0; i < 3; ++i) {
		// If the mesh is flat on one plane, we'll inflate it slightly
		if(max[i] - min[i] < 0.001)
			max[i] = min[i] + 0.001;
	}
	auto tBounds = 0.f;
	if(!point_in_aabb(start, min, max) && line_aabb(start, dir, min, max, &tBounds) == Result::NoIntersection)
		return false;

	r.precise = r.precise ? r.precise : pragma::util::make_shared<LineMeshResult::Precise>();
	auto &verts = subMesh.GetVertices();
	auto bHit = false;
	auto hasFoundBetterCandidate = false;
	auto foundEarlyIntersection = false;
	subMesh.VisitIndices([&verts, &start, &dir, &r, &hasFoundBetterCandidate, &foundEarlyIntersection, &bHit, &precise](auto *indexDataSrc, uint32_t numIndicesSrc) {
		for(auto i = decltype(numIndicesSrc) {0}; i < numIndicesSrc; i += 3) {
			auto &va = verts[indexDataSrc[i]].position;
			auto &vb = verts[indexDataSrc[i + 1]].position;
			auto &vc = verts[indexDataSrc[i + 2]].position;

			Plane p {va, vb, vc};
			float tl;
			auto rCur = line_plane(start, dir, p.GetNormal(), p.GetDistance(), &tl);
			if(is_better_candidate(r.result, rCur, r.hitValue, tl) == false)
				continue;
			double t, u, v;
			if(intersection::line_triangle(start, dir, va, vb, vc, t, u, v, true) == false)
				continue;
			hasFoundBetterCandidate = true;
			r.result = rCur;
			bHit = true;
			r.precise->triIdx = i / 3;
			r.hitValue = tl;
			r.hitPos = start + dir * static_cast<float>(r.hitValue);

			if(precise == false && r.result == Result::Intersect) {
				foundEarlyIntersection = true;
				break;
			}
		}
	});
	if(foundEarlyIntersection)
		return true;
	if(bHit == true) {
		if(r.precise)
			r.precise->subMesh = subMesh.shared_from_this();
		if(r.hitValue >= 0.f && r.hitValue <= 1.f)
			r.result = Result::Intersect;
		else
			r.result = Result::OutOfRange;
	}
	return hasFoundBetterCandidate;
}
