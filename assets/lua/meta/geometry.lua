--- @meta
--- 
--- @class geometry
geometry = {}

--- 
--- @param n math.Vector
--- @param d number
--- @param p math.Vector
--- @return enum umath::geometry::PlaneSide ret0
function geometry.get_side_of_point_to_plane(n, d, p) end

--- 
--- @param tContour 
--- @return any ret0
function geometry.triangulate(tContour) end

--- 
--- @param pA math.Vector
--- @param qA math.Vector
--- @param pB math.Vector
--- @param qB math.Vector
--- @param outCA math.Vector
--- @param outCB math.Vector
--- @param outD number
function geometry.closest_points_between_lines(pA, qA, pB, qB, outCA, outCB, outD) end

--- 
--- @return int ret0
function geometry.get_outline_vertices() end

--- 
--- @param tVerts 
--- @param tTriangles 
--- @return number ret0
function geometry.calc_volume_of_polyhedron(tVerts, tTriangles) end

--- 
--- @param v0 math.Vector
--- @param v1 math.Vector
--- @param v2 math.Vector
--- @param a number
--- @param b number
--- @return math.Vector ret0
function geometry.calc_point_on_triangle(v0, v1, v2, a, b) end

--- 
--- @param v0 math.Vector
--- @param v1 math.Vector
--- @param v2 math.Vector
--- @return math.Vector ret0
function geometry.calc_face_normal(v0, v1, v2) end

--- 
--- @param rectPos math.Vector2
--- @param rectSize math.Vector2
--- @param circlePos math.Vector2
--- @param circleRadius number
--- @return math.Vector2 ret0
function geometry.calc_rect_circle_touching_position(rectPos, rectSize, circlePos, circleRadius) end

--- 
--- @param n math.Vector
--- @param d number
--- @param p math.Vector
--- @return math.Vector ret0
function geometry.closest_point_on_plane_to_point(n, d, p) end

--- 
--- @param min math.Vector
--- @param max math.Vector
--- @param p math.Vector
--- @return math.Vector ret0
function geometry.closest_point_on_aabb_to_point(min, max, p) end

--- 
--- @param a math.Vector
--- @param b math.Vector
--- @param c math.Vector
--- @param p math.Vector
--- @return math.Vector ret0
function geometry.closest_point_on_triangle_to_point(a, b, c, p) end

--- 
--- @param tVerts 
--- @param outCenter math.Vector
--- @param outRadius number
function geometry.smallest_enclosing_sphere(tVerts, outCenter, outRadius) end

--- 
--- @param start math.Vector
--- @param end math.Vector
--- @param p math.Vector
--- @return math.Vector ret0
--- @overload fun(start: math.Vector, end_: math.Vector, p: math.Vector, bClampResultToSegment: bool): math.Vector
function geometry.closest_point_on_line_to_point(start, end_, p) end

--- 
--- @param origin math.Vector
--- @param radius number
--- @param start math.Vector
--- @param end math.Vector
--- @return math.Vector ret0
--- @overload fun(origin: math.Vector, radius: number, start: math.Vector, end_: math.Vector, bClampResultToSegment: bool): math.Vector
function geometry.closest_point_on_sphere_to_line(origin, radius, start, end_) end

--- 
--- @param n0 math.Vector
--- @param n1 math.Vector
--- @return math.Quaternion ret0
function geometry.calc_rotation_between_planes(n0, n1) end

--- 
--- @param v0 math.Vector2
--- @param v1 math.Vector2
--- @param v2 math.Vector2
--- @return enum umath::geometry::WindingOrder ret0
--- @overload fun(v0: math.Vector, v1: math.Vector, v2: math.Vector, n: math.Vector): enum umath::geometry::WindingOrder
function geometry.get_triangle_winding_order(v0, v1, v2) end

--- 
--- @param origin math.Vector
--- @param startRadius number
--- @param dir math.Vector
--- @param dist number
--- @param endRadius number
--- @param outVerts any
--- @param outTris any
--- @param outNormals any
--- @overload fun(origin: math.Vector, startRadius: number, dir: math.Vector, dist: number, endRadius: number, outVerts: any, outTris: any, outNormals: any, segmentCount: int): 
--- @overload fun(origin: math.Vector, startRadius: number, dir: math.Vector, dist: number, endRadius: number, outVerts: any, outTris: any, outNormals: any, segmentCount: int, caps: bool): 
--- @overload fun(origin: math.Vector, startRadius: number, dir: math.Vector, dist: number, endRadius: number, outVerts: any, outTris: any, outNormals: any, segmentCount: int, caps: bool, generateTriangles: bool): 
--- @overload fun(origin: math.Vector, startRadius: number, dir: math.Vector, dist: number, endRadius: number, outVerts: any, outTris: any, outNormals: any, segmentCount: int, caps: bool, generateTriangles: bool, generateNormals: bool): 
function geometry.generate_truncated_cone_mesh(origin, startRadius, dir, dist, endRadius, outVerts, outTris, outNormals) end

--- 
--- @param v0 math.Vector
--- @param v1 math.Vector
--- @param v2 math.Vector
--- @return number ret0
function geometry.calc_volume_of_triangle(v0, v1, v2) end

--- 
--- @param tVerts 
--- @param tTriangles 
--- @param outCom math.Vector
--- @param outVolume number
function geometry.calc_center_of_mass(tVerts, tTriangles, outCom, outVolume) end

--- 
--- @param a math.Vector
--- @param b math.Vector
--- @param c math.Vector
--- @return number ret0
--- @overload fun(p0: math.Vector2, p1: math.Vector2, p2: math.Vector2): number
--- @overload fun(p0: math.Vector2, p1: math.Vector2, p2: math.Vector2, keepSign: bool): number
function geometry.calc_triangle_area(a, b, c) end

--- 
--- @param p0 math.Vector
--- @param p1 math.Vector
--- @param p2 math.Vector
--- @param hitPoint math.Vector
--- @return math.Vector2 ret0
--- @overload fun(p0: math.Vector, uv0: math.Vector2, p1: math.Vector, uv1: math.Vector2, p2: math.Vector, uv2: math.Vector2, hitPoint: math.Vector): math.Vector2
function geometry.calc_barycentric_coordinates(p0, p1, p2, hitPoint) end

--- 
--- @param a math.Vector2
--- @param b math.Vector2
--- @param c math.Vector2
--- @return enum umath::geometry::LineSide ret0
function geometry.get_side_of_point_to_line(a, b, c) end


