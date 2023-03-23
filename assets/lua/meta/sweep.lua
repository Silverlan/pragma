--- @meta
--- 
--- @class sweep
sweep = {}

--- 
--- @param originA math.Vector
--- @param endA math.Vector
--- @param extentsA math.Vector
--- @param originB math.Vector
--- @param endB math.Vector
--- @param extentsB math.Vector
--- @param outResult bool
--- @param outTEntry number
--- @param outTExit number
--- @param outNormal math.Vector
--- @overload fun(originA: vector.Vector, endA: vector.Vector, extentsA: vector.Vector, originB: vector.Vector, endB: vector.Vector, extentsB: vector.Vector, outResult: bool, outTEntry: number, outTExit: number, outNormal: vector.Vector): 
function sweep.aabb_with_aabb(originA, endA, extentsA, originB, endB, extentsB, outResult, outTEntry, outTExit, outNormal) end

--- 
--- @param ext math.Vector
--- @param origin math.Vector
--- @param dir math.Vector
--- @param n math.Vector
--- @param d number
--- @param outResult bool
--- @param outT number
--- @overload fun(ext: vector.Vector, origin: vector.Vector, dir: vector.Vector, n: vector.Vector, d: number, outResult: bool, outT: number): 
function sweep.aabb_with_plane(ext, origin, dir, n, d, outResult, outT) end


