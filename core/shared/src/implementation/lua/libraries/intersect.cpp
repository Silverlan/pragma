// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.libraries.intersect;

luabind::object Lua::intersect::line_obb(lua::State *l, const Vector3 &rayStart, const Vector3 &rayDir, const Vector3 &min, const Vector3 &max, bool precise, const pragma::math::Transform &pose)
{
	auto dist = 0.f;
	float *pDist = nullptr;
	if(precise == true)
		pDist = &dist;
	auto r = pragma::math::intersection::line_obb(rayStart, rayDir, min, max, pDist, pose.GetOrigin(), pose.GetRotation());
	if(r == false || precise == false)
		return luabind::object {l, r};
	return luabind::object {l, dist};
}

static bool get_line_mesh_result(lua::State *l, const pragma::math::intersection::LineMeshResult &res, bool precise, luabind::object &r0, luabind::object &r1)
{
	if(precise == false) {
		r0 = luabind::object {l, res.result == pragma::math::intersection::Result::Intersect};
		return false;
	}
	r0 = luabind::object {l, res.result};
	r1 = luabind::newtable(l);

	r1["hitPos"] = res.hitPos;
	r1["hitValue"] = res.hitValue;
	if(res.precise) {
		r1["triIdx"] = res.precise->triIdx;
		r1["barycentricCoords"] = Vector3 {static_cast<float>(res.precise->t), static_cast<float>(res.precise->u), static_cast<float>(res.precise->v)};
	}
	return true;
}

void Lua::intersect::line_mesh(lua::State *l, const Vector3 &rayStart, const Vector3 &rayDir, pragma::geometry::ModelSubMesh &mesh, luabind::object &r0, luabind::object &r1, bool precise, const pragma::math::Transform &meshPose)
{
	auto &origin = meshPose.GetOrigin();
	auto &rot = meshPose.GetRotation();
	pragma::math::intersection::LineMeshResult res {};
	pragma::math::intersection::line_with_mesh(rayStart, rayDir, mesh, res, precise, &origin, &rot);
	get_line_mesh_result(l, res, precise, r0, r1);
}

void Lua::intersect::line_mesh(lua::State *l, const Vector3 &rayStart, const Vector3 &rayDir, pragma::geometry::ModelMesh &mesh, luabind::object &r0, luabind::object &r1, bool precise, const pragma::math::Transform &meshPose)
{
	auto &origin = meshPose.GetOrigin();
	auto &rot = meshPose.GetRotation();
	pragma::math::intersection::LineMeshResult res {};
	pragma::math::intersection::line_with_mesh(rayStart, rayDir, mesh, res, precise, &origin, &rot);
	if(get_line_mesh_result(l, res, precise, r0, r1) == false)
		return;
	if(res.precise)
		r1["subMeshIdx"] = res.precise->subMeshIdx;
}

void Lua::intersect::line_mesh(lua::State *l, const Vector3 &rayStart, const Vector3 &rayDir, pragma::asset::Model &mdl, uint32_t lod, luabind::object &r0, luabind::object &r1, bool precise, const pragma::math::Transform &meshPose)
{
	pragma::math::intersection::LineMeshResult res {};
	pragma::math::intersection::line_with_mesh(rayStart, rayDir, mdl, res, precise, nullptr, lod, meshPose.GetOrigin(), meshPose.GetRotation());
	if(get_line_mesh_result(l, res, precise, r0, r1) == false)
		return;
	if(res.precise) {
		r1["subMeshIdx"] = res.precise->subMeshIdx;
		r1["meshGroupIdx"] = res.precise->meshGroupIndex;
		r1["meshIdx"] = res.precise->meshIdx;
	}
}

void Lua::intersect::line_mesh(lua::State *l, const Vector3 &rayStart, const Vector3 &rayDir, pragma::asset::Model &mdl, luabind::table<> tBodyGroups, luabind::object &r0, luabind::object &r1, bool precise, const pragma::math::Transform &meshPose)
{
	auto bodyGroups = Lua::table_to_vector<uint32_t>(l, tBodyGroups, 4);
	pragma::math::intersection::LineMeshResult res {};
	pragma::math::intersection::line_with_mesh(rayStart, rayDir, mdl, res, precise, &bodyGroups, 0, meshPose.GetOrigin(), meshPose.GetRotation());
	if(get_line_mesh_result(l, res, precise, r0, r1) == false)
		return;
	if(res.precise) {
		r1["subMeshIdx"] = res.precise->subMeshIdx;
		r1["meshGroupIdx"] = res.precise->meshGroupIndex;
		r1["meshIdx"] = res.precise->meshIdx;
	}
}

void Lua::intersect::line_aabb(lua::State *l, const Vector3 &start, const Vector3 &dir, const Vector3 &min, const Vector3 &max, luabind::object &outMin, luabind::object &outMax)
{
	float tMin, tMax;
	auto res = pragma::math::intersection::line_aabb(start, dir, min, max, &tMin, &tMax) == pragma::math::intersection::Result::Intersect;
	if(res == false) {
		outMin = luabind::object {l, false};
		return;
	}
	outMin = luabind::object {l, tMin};
	outMax = luabind::object {l, tMax};
}

luabind::object Lua::intersect::line_plane(lua::State *l, const Vector3 &origin, const Vector3 &dir, const Vector3 &n, float d)
{
	float t;
	bool b = pragma::math::intersection::line_plane(origin, dir, n, d, &t) == pragma::math::intersection::Result::Intersect;
	if(b == false)
		return luabind::object {l, false};
	return luabind::object {l, t};
}

bool Lua::intersect::point_in_plane_mesh(lua::State *l, const Vector3 &vec, luabind::table<> planeTable)
{
	auto planes = Lua::table_to_vector<pragma::math::Plane>(l, planeTable, 2);
	return pragma::math::intersection::point_in_plane_mesh(vec, planes.begin(), planes.end());
}

int Lua::intersect::sphere_in_plane_mesh(lua::State *l, const Vector3 &vec, float r, luabind::table<> planeTable)
{
	auto planes = Lua::table_to_vector<pragma::math::Plane>(l, planeTable, 3);
	return pragma::math::to_integral(pragma::math::intersection::sphere_in_plane_mesh(vec, r, planes.begin(), planes.end()));
}

int Lua::intersect::aabb_in_plane_mesh(lua::State *l, const Vector3 &min, const Vector3 &max, luabind::table<> planeTable)
{
	auto planes = Lua::table_to_vector<pragma::math::Plane>(l, planeTable, 3);
	return pragma::math::to_integral(pragma::math::intersection::aabb_in_plane_mesh(min, max, planes.begin(), planes.end()));
}

void Lua::intersect::line_triangle(lua::State *l, const Vector3 &lineOrigin, const Vector3 &lineDir, const Vector3 &v0, const Vector3 &v1, const Vector3 &v2, luabind::object &outT, luabind::object &outUv, bool cull)
{
	double t, u, v;
	auto bIntersect = pragma::math::intersection::line_triangle(lineOrigin, lineDir, v0, v1, v2, t, u, v, cull);
	if(bIntersect == false) {
		outT = luabind::object {l, false};
		return;
	}
	outT = luabind::object {l, t};
	outUv = luabind::object {l, ::Vector2 {u, v}};
}

luabind::object Lua::intersect::line_line(lua::State *l, const ::Vector2 &start0, const ::Vector2 &end0, const ::Vector2 &start1, const ::Vector2 &end1)
{
	auto result = pragma::math::intersection::line_line(start0, end0, start1, end1);
	if(result.has_value() == false)
		return luabind::object {l, false};
	return luabind::object {l, *result};
}
