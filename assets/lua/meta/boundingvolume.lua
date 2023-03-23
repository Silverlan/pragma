--- @meta
--- 
--- @class boundingvolume
boundingvolume = {}

--- 
--- @param min math.Vector
--- @param max math.Vector
--- @param rot math.Quaternion
--- @return math.Vector ret0_1
--- @return math.Vector ret0_2
--- @overload fun(min: math.Vector, max: math.Vector, rot: math.Mat3): math.Vector, math.Vector
--- @overload fun(min: vector.Vector, max: vector.Vector, rot: math.Quaternion): math.Vector, math.Vector
--- @overload fun(min: vector.Vector, max: vector.Vector, rot: math.Mat3): math.Vector, math.Vector
function boundingvolume.get_rotated_aabb(min, max, rot) end


