--- @meta
--- 
--- @class geometry
geometry = {}

--- 
--- @param v0 math.Vector2
--- @param v1 math.Vector2
--- @param v2 math.Vector2
--- @return enum umath::geometry::WindingOrder ret0
--- @overload fun(v0: math.Vector, v1: math.Vector, v2: math.Vector, n: math.Vector): enum umath::geometry::WindingOrder
--- @overload fun(v0: vector.Vector, v1: vector.Vector, v2: vector.Vector, n: vector.Vector): enum umath::geometry::WindingOrder
function geometry.get_triangle_winding_order(v0, v1, v2) end

--- 
--- @param v0 math.Vector
--- @param v1 math.Vector
--- @param v2 math.Vector
--- @return number ret0
--- @overload fun(v0: vector.Vector, v1: vector.Vector, v2: vector.Vector): number
function geometry.calc_volume_of_triangle(v0, v1, v2) end

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
--- @overload fun(origin: vector.Vector, startRadius: number, dir: vector.Vector, dist: number, endRadius: number, outVerts: any, outTris: any, outNormals: any): 
--- @overload fun(origin: vector.Vector, startRadius: number, dir: vector.Vector, dist: number, endRadius: number, outVerts: any, outTris: any, outNormals: any, segmentCount: int): 
--- @overload fun(origin: vector.Vector, startRadius: number, dir: vector.Vector, dist: number, endRadius: number, outVerts: any, outTris: any, outNormals: any, segmentCount: int, caps: bool): 
--- @overload fun(origin: vector.Vector, startRadius: number, dir: vector.Vector, dist: number, endRadius: number, outVerts: any, outTris: any, outNormals: any, segmentCount: int, caps: bool, generateTriangles: bool): 
--- @overload fun(origin: vector.Vector, startRadius: number, dir: vector.Vector, dist: number, endRadius: number, outVerts: any, outTris: any, outNormals: any, segmentCount: int, caps: bool, generateTriangles: bool, generateNormals: bool): 
function geometry.generate_truncated_cone_mesh(origin, startRadius, dir, dist, endRadius, outVerts, outTris, outNormals) end

--- 
--- @param rectPos math.Vector2
--- @param rectSize math.Vector2
--- @param circlePos math.Vector2
--- @param circleRadius number
--- @return math.Vector2 ret0
function geometry.calc_rect_circle_touching_position(rectPos, rectSize, circlePos, circleRadius) end

--- 
--- @param v0 math.Vector
--- @param v1 math.Vector
--- @param v2 math.Vector
--- @return math.Vector ret0
--- @overload fun(v0: vector.Vector, v1: vector.Vector, v2: vector.Vector): math.Vector
function geometry.calc_face_normal(v0, v1, v2) end

--- 
--- @param v0 math.Vector
--- @param v1 math.Vector
--- @param v2 math.Vector
--- @param a number
--- @param b number
--- @return math.Vector ret0
--- @overload fun(v0: vector.Vector, v1: vector.Vector, v2: vector.Vector, a: number, b: number): math.Vector
function geometry.calc_point_on_triangle(v0, v1, v2, a, b) end

--- 
--- @param tVerts 
--- @param tTriangles 
--- @return number ret0
function geometry.calc_volume_of_polyhedron(tVerts, tTriangles) end

--- 
--- @param tVerts 
--- @param tTriangles 
--- @param outCom math.Vector
--- @param outVolume number
--- @overload fun(tVerts: , tTriangles: , outCom: vector.Vector, outVolume: number): 
function geometry.calc_center_of_mass(tVerts, tTriangles, outCom, outVolume) end

--- 
--- @param a math.Vector
--- @param b math.Vector
--- @param c math.Vector
--- @return number ret0
--- @overload fun(p0: math.Vector2, p1: math.Vector2, p2: math.Vector2): number
--- @overload fun(p0: math.Vector2, p1: math.Vector2, p2: math.Vector2, keepSign: bool): number
--- @overload fun(a: vector.Vector, b: vector.Vector, c: vector.Vector): number
function geometry.calc_triangle_area(a, b, c) end

--- 
--- @param p0 math.Vector
--- @param p1 math.Vector
--- @param p2 math.Vector
--- @param hitPoint math.Vector
--- @return math.Vector2 ret0
--- @overload fun(p0: math.Vector, uv0: math.Vector2, p1: math.Vector, uv1: math.Vector2, p2: math.Vector, uv2: math.Vector2, hitPoint: math.Vector): math.Vector2
--- @overload fun(p0: vector.Vector, p1: vector.Vector, p2: vector.Vector, hitPoint: vector.Vector): math.Vector2
--- @overload fun(p0: vector.Vector, uv0: math.Vector2, p1: vector.Vector, uv1: math.Vector2, p2: vector.Vector, uv2: math.Vector2, hitPoint: vector.Vector): math.Vector2
function geometry.calc_barycentric_coordinates(p0, p1, p2, hitPoint) end

--- 
--- @param a math.Vector2
--- @param b math.Vector2
--- @param c math.Vector2
--- @return enum umath::geometry::LineSide ret0
function geometry.get_side_of_point_to_line(a, b, c) end

--- 
--- @param n0 math.Vector
--- @param n1 math.Vector
--- @return math.Quaternion ret0
--- @overload fun(n0: vector.Vector, n1: vector.Vector): math.Quaternion
function geometry.calc_rotation_between_planes(n0, n1) end

--- 
--- @param origin math.Vector
--- @param radius number
--- @param start math.Vector
--- @param end math.Vector
--- @return math.Vector ret0
--- @overload fun(origin: math.Vector, radius: number, start: math.Vector, end_: math.Vector, bClampResultToSegment: bool): math.Vector
--- @overload fun(origin: vector.Vector, radius: number, start: vector.Vector, end_: vector.Vector): math.Vector
--- @overload fun(origin: vector.Vector, radius: number, start: vector.Vector, end_: vector.Vector, bClampResultToSegment: bool): math.Vector
function geometry.closest_point_on_sphere_to_line(origin, radius, start, end_) end

--- 
--- @param n math.Vector
--- @param d number
--- @param p math.Vector
--- @return enum umath::geometry::PlaneSide ret0
--- @overload fun(n: vector.Vector, d: number, p: vector.Vector): enum umath::geometry::PlaneSide
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
--- @overload fun(pA: vector.Vector, qA: vector.Vector, pB: vector.Vector, qB: vector.Vector, outCA: vector.Vector, outCB: vector.Vector, outD: number): 
function geometry.closest_points_between_lines(pA, qA, pB, qB, outCA, outCB, outD) end

--- 
--- @return int ret0
function geometry.get_outline_vertices() end

--- 
--- @param min math.Vector
--- @param max math.Vector
--- @param p math.Vector
--- @return math.Vector ret0
--- @overload fun(min: vector.Vector, max: vector.Vector, p: vector.Vector): math.Vector
function geometry.closest_point_on_aabb_to_point(min, max, p) end

--- 
--- @param n math.Vector
--- @param d number
--- @param p math.Vector
--- @return math.Vector ret0
--- @overload fun(n: vector.Vector, d: number, p: vector.Vector): math.Vector
function geometry.closest_point_on_plane_to_point(n, d, p) end

--- 
--- @param a math.Vector
--- @param b math.Vector
--- @param c math.Vector
--- @param p math.Vector
--- @return math.Vector ret0
--- @overload fun(a: vector.Vector, b: vector.Vector, c: vector.Vector, p: vector.Vector): math.Vector
function geometry.closest_point_on_triangle_to_point(a, b, c, p) end

--- 
--- @param tVerts 
--- @param outCenter math.Vector
--- @param outRadius number
--- @overload fun(tVerts: , outCenter: vector.Vector, outRadius: number): 
function geometry.smallest_enclosing_sphere(tVerts, outCenter, outRadius) end

--- 
--- @param start math.Vector
--- @param end math.Vector
--- @param p math.Vector
--- @return math.Vector ret0
--- @overload fun(start: math.Vector, end_: math.Vector, p: math.Vector, bClampResultToSegment: bool): math.Vector
--- @overload fun(start: vector.Vector, end_: vector.Vector, p: vector.Vector): math.Vector
--- @overload fun(start: vector.Vector, end_: vector.Vector, p: vector.Vector, bClampResultToSegment: bool): math.Vector
function geometry.closest_point_on_line_to_point(start, end_, p) end


