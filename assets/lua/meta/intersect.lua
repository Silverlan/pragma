--- @meta
--- 
--- @class intersect
intersect = {}

--- 
--- @param rayStart math.Vector
--- @param rayDir math.Vector
--- @param min math.Vector
--- @param max math.Vector
--- @return any ret0
--- @overload fun(rayStart: math.Vector, rayDir: math.Vector, min: math.Vector, max: math.Vector, precise: bool): any
--- @overload fun(rayStart: math.Vector, rayDir: math.Vector, min: math.Vector, max: math.Vector, precise: bool, pose: math.Transform): any
function intersect.line_with_obb(rayStart, rayDir, min, max) end

--- 
--- @param min math.Vector
--- @param max math.Vector
--- @param planeTable 
--- @return int ret0
function intersect.aabb_in_plane_mesh(min, max, planeTable) end

--- 
--- @param sphereOrigin math.Vector
--- @param sphereRadius number
--- @param n math.Vector
--- @param d number
--- @return bool ret0
function intersect.sphere_with_plane(sphereOrigin, sphereRadius, n, d) end

--- 
--- @param rayStart math.Vector
--- @param rayDir math.Vector
--- @param mdl game.Model
--- @param bodyGroups 
--- @param r0 any
--- @param r1 any
--- @overload fun(rayStart: math.Vector, rayDir: math.Vector, mdl: game.Model, bodyGroups: , r0: any, r1: any, precise: bool): 
--- @overload fun(rayStart: math.Vector, rayDir: math.Vector, mdl: game.Model, tBodyGroups: , r0: any, r1: any, precise: bool, meshPose: math.Transform): 
--- @overload fun(rayStart: math.Vector, rayDir: math.Vector, mdl: game.Model, lod: int, r0: any, r1: any): 
--- @overload fun(rayStart: math.Vector, rayDir: math.Vector, mdl: game.Model, lod: int, r0: any, r1: any, precise: bool): 
--- @overload fun(rayStart: math.Vector, rayDir: math.Vector, mdl: game.Model, lod: int, r0: any, r1: any, precise: bool, meshPose: math.Transform): 
--- @overload fun(rayStart: math.Vector, rayDir: math.Vector, mesh: game.Model.Mesh, r0: any, r1: any): 
--- @overload fun(rayStart: math.Vector, rayDir: math.Vector, mesh: game.Model.Mesh, r0: any, r1: any, precise: bool): 
--- @overload fun(rayStart: math.Vector, rayDir: math.Vector, mesh: game.Model.Mesh, r0: any, r1: any, precise: bool, meshPose: math.Transform): 
--- @overload fun(rayStart: math.Vector, rayDir: math.Vector, mesh: game.Model.Mesh.Sub, r0: any, r1: any): 
--- @overload fun(rayStart: math.Vector, rayDir: math.Vector, mesh: game.Model.Mesh.Sub, r0: any, r1: any, precise: bool): 
--- @overload fun(rayStart: math.Vector, rayDir: math.Vector, mesh: game.Model.Mesh.Sub, r0: any, r1: any, precise: bool, meshPose: math.Transform): 
function intersect.line_with_mesh(rayStart, rayDir, mdl, bodyGroups, r0, r1) end

--- 
--- @param vec math.Vector
--- @param planeTable 
--- @return bool ret0
function intersect.point_in_plane_mesh(vec, planeTable) end

--- 
--- @param pmin math.Vector
--- @param pmax math.Vector
--- @param pa math.Vector
--- @param pb math.Vector
--- @param pc math.Vector
--- @return bool ret0
function intersect.aabb_with_triangle(pmin, pmax, pa, pb, pc) end

--- 
--- @param origin math.Vector
--- @param dir math.Vector
--- @param n math.Vector
--- @param d number
--- @return any ret0
function intersect.line_with_plane(origin, dir, n, d) end

--- 
--- @param vec math.Vector
--- @param min math.Vector
--- @param max math.Vector
--- @return bool ret0
function intersect.point_in_aabb(vec, min, max) end

--- 
--- @param sphereOrigin math.Vector
--- @param radius number
--- @param coneOrigin math.Vector
--- @param coneDir math.Vector
--- @param coneAngle number
--- @return bool ret0
--- @overload fun(sphereOrigin: math.Vector, radius: number, coneOrigin: math.Vector, coneDir: math.Vector, coneAngle: number, coneSize: number): bool
function intersect.sphere_with_cone(sphereOrigin, radius, coneOrigin, coneDir, coneAngle) end

--- 
--- @param vec math.Vector
--- @param r number
--- @param planeTable 
--- @return int ret0
function intersect.sphere_in_plane_mesh(vec, r, planeTable) end

--- 
--- @param lineOrigin math.Vector
--- @param lineDir math.Vector
--- @param v0 math.Vector
--- @param v1 math.Vector
--- @param v2 math.Vector
--- @param outT any
--- @param outUv any
--- @overload fun(lineOrigin: math.Vector, lineDir: math.Vector, v0: math.Vector, v1: math.Vector, v2: math.Vector, outT: any, outUv: any, cull: bool): 
function intersect.line_with_triangle(lineOrigin, lineDir, v0, v1, v2, outT, outUv) end

--- 
--- @param min math.Vector
--- @param max math.Vector
--- @param n math.Vector
--- @param d number
--- @return bool ret0
function intersect.aabb_with_plane(min, max, n, d) end

--- 
--- @param min math.Vector
--- @param max math.Vector
--- @param origin math.Vector
--- @param rot math.Quaternion
--- @param n math.Vector
--- @param d number
--- @return bool ret0
function intersect.obb_with_plane(min, max, origin, rot, n, d) end

--- 
--- @param lineOrigin math.Vector
--- @param lineDir math.Vector
--- @param sphereOrigin math.Vector
--- @param sphereRadius number
function intersect.line_with_sphere(lineOrigin, lineDir, sphereOrigin, sphereRadius) end

--- 
--- @param minA math.Vector
--- @param maxA math.Vector
--- @param minB math.Vector
--- @param maxB math.Vector
--- @return enum umath::intersection::Intersect ret0
function intersect.aabb_with_aabb(minA, maxA, minB, maxB) end

--- 
--- @param originA math.Vector
--- @param rA number
--- @param originB math.Vector
--- @param rB number
--- @return bool ret0
function intersect.sphere_with_sphere(originA, rA, originB, rB) end

--- 
--- @param min math.Vector
--- @param max math.Vector
--- @param origin math.Vector
--- @param r number
--- @return bool ret0
function intersect.aabb_with_sphere(min, max, origin, r) end

--- 
--- @param start math.Vector
--- @param dir math.Vector
--- @param min math.Vector
--- @param max math.Vector
--- @param outMin any
--- @param outMax any
function intersect.line_with_aabb(start, dir, min, max, outMin, outMax) end


