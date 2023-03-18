--- @meta
--- 
--- @class vector
vector = {}

--- 
--- @param points 
--- @return math.Vector ret0
function vector.calc_average(points) end

--- 
--- @return math.Vector2 ret0
function vector.random2D() end

--- 
--- @param points 
--- @param ang number
--- @param outNormal math.Vector
--- @param outDistance number
function vector.calc_best_fitting_plane(points, ang, outNormal, outDistance) end

--- 
--- @param refPos math.Vector
--- @param angVel math.Vector
--- @param tgtPos math.Vector
--- @return math.Vector ret0
function vector.calc_linear_velocity_from_angular(refPos, angVel, tgtPos) end

--- 
--- @param p math.Vector
--- @param d math.Vector
--- @return math.Vector ret0_1
--- @return math.Vector ret0_2
--- @overload fun(p: math.Vector, d: math.Vector, interocularDistance: number): math.Vector, math.Vector
--- @overload fun(p: math.Vector, d: math.Vector, interocularDistance: number, convergenceDistance: number): math.Vector, math.Vector
function vector.calc_spherical_stereo_transform(p, d) end

--- 
--- @return math.Vector ret0
function vector.random() end

--- 
--- @param inOutA math.Vector4
--- @param inOutB math.Vector4
--- @param c math.Vector4
--- @overload fun(inOutA: math.Vector, inOutB: math.Vector, c: math.Vector): 
--- @overload fun(inOutA: math.Vector2, inOutB: math.Vector2, c: math.Vector2): 
--- @overload fun(inOutA: math.Vector4, inOutB: math.Vector4): 
--- @overload fun(inOutA: math.Vector, inOutB: math.Vector): 
--- @overload fun(inOutA: math.Vector2, inOutB: math.Vector2): 
function vector.to_min_max(inOutA, inOutB, c) end

--- 
--- @param t 
--- @param outMin math.Vector4
--- @param outMax math.Vector4
--- @overload fun(t: , outMin: math.Vector, outMax: math.Vector): 
--- @overload fun(t: , outMin: math.Vector2, outMax: math.Vector2): 
function vector.get_min_max(t, outMin, outMax) end

--- 
--- @param str string
--- @return math.Vector ret0
function vector.create_from_string(str) end


--- 
--- @class vector.Vector
--- @field x number 
--- @field y number 
--- @field z number 
--- @field r number 
--- @field b number 
--- @field g number 
--- @overload fun(args: string):vector.Vector
--- @overload fun(arg1: math.Vector2, arg2: number):vector.Vector
--- @overload fun(arg1: number, arg2: number, arg3: number):vector.Vector
--- @overload fun():vector.Vector
vector.Vector = {}

--- 
--- @param 1 vector.Vector
--- @overload fun(arg1: number): 
function vector.Vector:__div(arg1) end

--- 
--- @return math.Mat4 ret0
function vector.Vector:ToMatrix() end

--- 
--- @param orientation math.Quaternion
--- @overload fun(normal: vector.Vector, angle: number): 
--- @overload fun(ang: math.EulerAngles): 
function vector.Vector:Rotate(orientation) end

--- 
--- @param n vector.Vector
--- @param d number
function vector.Vector:Reflect(n, d) end

--- 
function vector.Vector:__unm() end

--- 
--- @param vp math.Mat4
--- @return math.Vector2 ret0
function vector.Vector:ToScreenUv(vp) end

--- 
--- @param v1 vector.Vector
--- @return math.Mat3 ret0
function vector.Vector:OuterProduct(v1) end

--- 
--- @return vector.Vector ret0
function vector.Vector:GetPerpendicular() end

--- 
--- @param b math.ScaledTransform
--- @overload fun(b: math.Transform): 
function vector.Vector:Mul(b) end

--- 
--- @return vector.Vector ret0
function vector.Vector:GetNormal() end

--- 
--- @param 1 vector.Vector
function vector.Vector:__eq(arg1) end

--- 
--- @return math.EulerAngles ret0
function vector.Vector:ToEulerAngles() end

--- 
--- @param n vector.Vector
--- @return vector.Vector ret0
function vector.Vector:Project(n) end

--- 
--- @param p1 vector.Vector
--- @param n vector.Vector
--- @return number ret0
function vector.Vector:PlanarDistance(p1, n) end

--- 
--- @param b vector.Vector
--- @return number ret0
function vector.Vector:GetAngle(b) end

--- 
function vector.Vector:__tostring() end

--- 
--- @return number ret0
function vector.Vector:GetPitch() end

--- 
--- @param 1 vector.Vector
--- @overload fun(arg1: vector.Vector): 
--- @overload fun(arg1: math.Quaternion): 
--- @overload fun(arg1: vector.Vector): 
--- @overload fun(arg1: number): 
function vector.Vector:__mul(arg1) end

--- 
--- @param b vector.Vector
--- @return math.Quaternion ret0
function vector.Vector:GetRotation(b) end

--- 
--- @return number ret0
function vector.Vector:GetYaw() end

--- 
--- @param idx int
--- @param val number
--- @overload fun(x: number, y: number, z: number): 
--- @overload fun(vecB: vector.Vector): 
function vector.Vector:Set(idx, val) end

--- 
--- @param idx int
function vector.Vector:Get(idx) end

--- 
--- @param b vector.Vector
--- @param factor number
function vector.Vector:Slerp(b, factor) end

--- 
function vector.Vector:Normalize() end

--- 
--- @return number ret0
function vector.Vector:Length() end

--- 
--- @return number ret0
function vector.Vector:LengthSqr() end

--- 
--- @param b vector.Vector
--- @return number ret0
function vector.Vector:Distance(b) end

--- 
--- @param p1 vector.Vector
--- @return number ret0
function vector.Vector:DistanceSqr(p1) end

--- 
--- @param p1 vector.Vector
--- @param n vector.Vector
--- @return number ret0
function vector.Vector:PlanarDistanceSqr(p1, n) end

--- 
--- @param vecB vector.Vector
--- @return vector.Vector ret0
function vector.Vector:Cross(vecB) end

--- 
--- @param b vector.Vector
--- @return number ret0
function vector.Vector:DotProduct(b) end

--- 
--- @param origin vector.Vector
--- @param ang math.EulerAngles
function vector.Vector:RotateAround(origin, ang) end

--- 
--- @param vecB vector.Vector
--- @param factor number
--- @return vector.Vector ret0
function vector.Vector:Lerp(vecB, factor) end

--- 
--- @param b vector.Vector
--- @overload fun(b: vector.Vector, epsilon: number): 
function vector.Vector:Equals(b) end

--- 
--- @param gridSize int
--- @overload fun(): 
function vector.Vector:SnapToGrid(gridSize) end

--- 
--- @return vector.Vector ret0
function vector.Vector:Copy() end

--- 
--- @param n vector.Vector
--- @param d number
--- @return vector.Vector ret0
function vector.Vector:ProjectToPlane(n, d) end

--- 
--- @param 1 vector.Vector
function vector.Vector:__add(arg1) end

--- 
--- @param 1 vector.Vector
function vector.Vector:__sub(arg1) end


