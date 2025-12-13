// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.libraries.geometry;

export import pragma.lua;

export namespace Lua {
	namespace geometry {
		DLLNETWORK Vector3 closest_point_on_aabb_to_point(const Vector3 &min, const Vector3 &max, const Vector3 &p);
		DLLNETWORK void closest_points_between_lines(const Vector3 &pA, const Vector3 &qA, const Vector3 &pB, const Vector3 &qB, Vector3 &outCA, Vector3 &outCB, float &outD);
		DLLNETWORK Vector3 closest_point_on_plane_to_point(const Vector3 &n, float d, const Vector3 &p);
		DLLNETWORK Vector3 closest_point_on_triangle_to_point(const Vector3 &a, const Vector3 &b, const Vector3 &c, const Vector3 &p);
		DLLNETWORK void smallest_enclosing_sphere(lua::State *l, luabind::table<> tVerts, Vector3 &outCenter, float &outRadius);
		DLLNETWORK void generate_truncated_cone_mesh(lua::State *l, const Vector3 &origin, float startRadius, const Vector3 &dir, float dist, float endRadius, luabind::object &outVerts, luabind::object &outTris, luabind::object &outNormals, uint32_t segmentCount = 12, bool caps = true,
		  bool generateTriangles = true, bool generateNormals = false);

		DLLNETWORK double calc_volume_of_polyhedron(lua::State *l, luabind::table<> tVerts, luabind::table<> tTriangles);
		DLLNETWORK void calc_center_of_mass(lua::State *l, luabind::table<> tVerts, luabind::table<> tTriangles, Vector3 &outCom, double &outVolume);

		DLLNETWORK Vector2 calc_barycentric_coordinates(const Vector3 &p0, const Vector2 &uv0, const Vector3 &p1, const Vector2 &uv1, const Vector3 &p2, const Vector2 &uv2, const Vector3 &hitPoint);
		DLLNETWORK Vector2 calc_barycentric_coordinates(const Vector3 &p0, const Vector3 &p1, const Vector3 &p2, const Vector3 &hitPoint);
		DLLNETWORK int get_outline_vertices(lua::State *l);
		DLLNETWORK int triangulate_point_cloud(lua::State *l);
		DLLNETWORK luabind::object triangulate(lua::State *l, luabind::table<> contour);
	};
};
