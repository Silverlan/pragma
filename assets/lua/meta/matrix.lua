--- @meta
--- 
--- @class matrix
matrix = {}

--- 
--- @param forward math.Vector
--- @param right math.Vector
--- @param up math.Vector
--- @return math.Mat4 ret0
function matrix.create_from_axes(forward, right, up) end

--- 
--- @param v math.Vector
--- @param ang number
--- @return math.Mat4 ret0
function matrix.create_from_axis_angle(v, ang) end

--- 
--- @param n math.Vector
--- @param d number
--- @return math.Mat4 ret0
function matrix.create_reflection(n, d) end

--- 
--- @param left number
--- @param right number
--- @param bottom number
--- @param top number
--- @param zNear number
--- @param zFar number
--- @return math.Mat4 ret0
function matrix.create_orthogonal_matrix(left, right, bottom, top, zNear, zFar) end

--- 
--- @param fov number
--- @param aspectRatio number
--- @param zNear number
--- @param zFar number
--- @return math.Mat4 ret0
function matrix.create_perspective_matrix(fov, aspectRatio, zNear, zFar) end

--- 
--- @param points 
--- @return math.Mat3 ret0
--- @overload fun(points: ): math.Mat3
--- @overload fun(points: , avg: math.Vector): math.Mat3
function matrix.calc_covariance_matrix(points) end

--- 
--- @param arg0 math.Vector
--- @param arg1 math.Vector
--- @param arg2 math.Vector
--- @return math.Mat4 ret0
function matrix.create_look_at_matrix(arg0, arg1, arg2) end

--- 
--- @param p number
--- @param nearZ number
--- @param farZ number
--- @param d number
--- @param delta number
--- @return number ret0
function matrix.calc_projection_depth_bias_offset(p, nearZ, farZ, d, delta) end


