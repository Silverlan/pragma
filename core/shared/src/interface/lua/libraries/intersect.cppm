// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.libraries.intersect;

export import :model;

export namespace Lua {
	namespace intersect {
		DLLNETWORK void line_aabb(lua::State *l, const Vector3 &start, const Vector3 &dir, const Vector3 &min, const Vector3 &max, luabind::object &outMin, luabind::object &outMax);
		DLLNETWORK luabind::object line_plane(lua::State *l, const Vector3 &origin, const Vector3 &dir, const Vector3 &n, float d);
		DLLNETWORK bool point_in_plane_mesh(lua::State *l, const Vector3 &vec, luabind::table<> planeTable);
		DLLNETWORK luabind::object line_obb(lua::State *l, const Vector3 &rayStart, const Vector3 &rayDir, const Vector3 &min, const Vector3 &max, bool precise = false, const pragma::math::Transform &pose = {});
		DLLNETWORK void line_mesh(lua::State *l, const Vector3 &rayStart, const Vector3 &rayDir, pragma::geometry::ModelSubMesh &mesh, luabind::object &r0, luabind::object &r1, bool precise = false, const pragma::math::Transform &meshPose = {});
		DLLNETWORK void line_mesh(lua::State *l, const Vector3 &rayStart, const Vector3 &rayDir, pragma::geometry::ModelMesh &mesh, luabind::object &r0, luabind::object &r1, bool precise = false, const pragma::math::Transform &meshPose = {});
		DLLNETWORK void line_mesh(lua::State *l, const Vector3 &rayStart, const Vector3 &rayDir, pragma::asset::Model &mdl, uint32_t lod, luabind::object &r0, luabind::object &r1, bool precise = false, const pragma::math::Transform &meshPose = {});
		DLLNETWORK void line_mesh(lua::State *l, const Vector3 &rayStart, const Vector3 &rayDir, pragma::asset::Model &mdl, luabind::table<> tBodyGroups, luabind::object &r0, luabind::object &r1, bool precise = false, const pragma::math::Transform &meshPose = {});
		DLLNETWORK int sphere_in_plane_mesh(lua::State *l, const Vector3 &vec, float r, luabind::table<> planeTable);
		DLLNETWORK int aabb_in_plane_mesh(lua::State *l, const Vector3 &min, const Vector3 &max, luabind::table<> planeTable);
		DLLNETWORK void line_triangle(lua::State *l, const Vector3 &lineOrigin, const Vector3 &lineDir, const Vector3 &v0, const Vector3 &v1, const Vector3 &v2, luabind::object &outT, luabind::object &outUv, bool cull = false);
		DLLNETWORK luabind::object line_line(lua::State *l, const Vector2 &start0, const Vector2 &end0, const Vector2 &start1, const Vector2 &end1);
	};
};
