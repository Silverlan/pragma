--- @meta
--- 
--- @class vector
vector = {}

--- 
--- @param inOutA math.Vector4
--- @param inOutB math.Vector4
--- @param c math.Vector4
--- @overload fun(inOutA: math.Vector, inOutB: math.Vector, c: math.Vector): 
--- @overload fun(inOutA: math.Vector2, inOutB: math.Vector2, c: math.Vector2): 
--- @overload fun(inOutA: math.Vector4, inOutB: math.Vector4): 
--- @overload fun(inOutA: math.Vector, inOutB: math.Vector): 
--- @overload fun(inOutA: math.Vector2, inOutB: math.Vector2): 
--- @overload fun(inOutA: vector.Vector, inOutB: vector.Vector, c: vector.Vector): 
--- @overload fun(inOutA: vector.Vector, inOutB: vector.Vector): 
function vector.to_min_max(inOutA, inOutB, c) end

--- 
--- @param str string
--- @return math.Vector ret0
--- @overload fun(str: string): math.Vector
function vector.create_from_string(str) end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function vector.random() end

--- 
--- @param p math.Vector
--- @param d math.Vector
--- @return math.Vector ret0_1
--- @return math.Vector ret0_2
--- @overload fun(p: math.Vector, d: math.Vector, interocularDistance: number): math.Vector, math.Vector
--- @overload fun(p: math.Vector, d: math.Vector, interocularDistance: number, convergenceDistance: number): math.Vector, math.Vector
--- @overload fun(p: vector.Vector, d: vector.Vector): math.Vector, math.Vector
--- @overload fun(p: vector.Vector, d: vector.Vector, interocularDistance: number): math.Vector, math.Vector
--- @overload fun(p: vector.Vector, d: vector.Vector, interocularDistance: number, convergenceDistance: number): math.Vector, math.Vector
function vector.calc_spherical_stereo_transform(p, d) end

--- 
--- @param t 
--- @param outMin math.Vector4
--- @param outMax math.Vector4
--- @overload fun(t: , outMin: math.Vector, outMax: math.Vector): 
--- @overload fun(t: , outMin: math.Vector2, outMax: math.Vector2): 
--- @overload fun(t: , outMin: vector.Vector, outMax: vector.Vector): 
function vector.get_min_max(t, outMin, outMax) end

--- 
--- @return math.Vector2 ret0
function vector.random2D() end

--- 
--- @param points 
--- @return math.Vector ret0
--- @overload fun(points: ): math.Vector
function vector.calc_average(points) end

--- 
--- @param refPos math.Vector
--- @param angVel math.Vector
--- @param tgtPos math.Vector
--- @return math.Vector ret0
--- @overload fun(refPos: vector.Vector, angVel: vector.Vector, tgtPos: vector.Vector): math.Vector
function vector.calc_linear_velocity_from_angular(refPos, angVel, tgtPos) end

--- 
--- @param points 
--- @param ang number
--- @param outNormal math.Vector
--- @param outDistance number
--- @overload fun(points: , ang: number, outNormal: vector.Vector, outDistance: number): 
function vector.calc_best_fitting_plane(points, ang, outNormal, outDistance) end


--- 
--- @class vector.Vector
--- @field g number 
--- @field b number 
--- @field r number 
--- @field z number 
--- @field x number 
--- @field y number 
--- @overload fun(args: string):vector.Vector
--- @overload fun(arg1: math.Vector2, arg2: number):vector.Vector
--- @overload fun(arg1: number, arg2: number, arg3: number):vector.Vector
--- @overload fun():vector.Vector
vector.Vector = {}

--- 
--- @param b vector.Vector
--- @return math.Quaternion ret0
function vector.Vector:GetRotation(b) end

--- 
--- @param b math.ScaledTransform
--- @overload fun(b: math.Transform): 
function vector.Vector:Mul(b) end

--- 
--- @param 1 vector.Vector
function vector.Vector:__eq(arg1) end

--- 
--- @return math.EulerAngles ret0
function vector.Vector:ToEulerAngles() end

--- 
--- @return math.Mat4 ret0
function vector.Vector:ToMatrix() end

--- 
--- @return number ret0
function vector.Vector:Length() end

--- 
--- @param n vector.Vector
--- @param d number
function vector.Vector:Reflect(n, d) end

--- 
--- @param orientation math.Quaternion
--- @overload fun(normal: vector.Vector, angle: number): 
--- @overload fun(ang: math.EulerAngles): 
function vector.Vector:Rotate(orientation) end

--- 
--- @return vector.Vector ret0
function vector.Vector:GetNormal() end

--- 
function vector.Vector:Normalize() end

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
function vector.Vector:PlanarDistance(p1, n) end

--- 
--- @param b vector.Vector
--- @return number ret0
function vector.Vector:GetAngle(b) end

--- 
--- @param n vector.Vector
--- @return vector.Vector ret0
function vector.Vector:Project(n) end

--- 
--- @param p1 vector.Vector
--- @param n vector.Vector
--- @return number ret0
function vector.Vector:PlanarDistanceSqr(p1, n) end

--- 
--- @param 1 vector.Vector
function vector.Vector:__sub(arg1) end

--- 
--- @param 1 vector.Vector
--- @overload fun(arg1: vector.Vector): 
--- @overload fun(arg1: math.Quaternion): 
--- @overload fun(arg1: vector.Vector): 
--- @overload fun(arg1: number): 
function vector.Vector:__mul(arg1) end

--- 
--- @return number ret0
function vector.Vector:GetPitch() end

--- 
--- @param 1 vector.Vector
--- @overload fun(arg1: number): 
function vector.Vector:__div(arg1) end

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
function vector.Vector:__unm() end

--- 
--- @param vp math.Mat4
--- @return math.Vector2 ret0
function vector.Vector:ToScreenUv(vp) end

--- 
function vector.Vector:__tostring() end

--- 
--- @return number ret0
function vector.Vector:GetYaw() end

--- 
--- @param n vector.Vector
--- @param d number
--- @return vector.Vector ret0
function vector.Vector:ProjectToPlane(n, d) end

--- 
--- @return vector.Vector ret0
function vector.Vector:GetPerpendicular() end

--- 
--- @param v1 vector.Vector
--- @return math.Mat3 ret0
function vector.Vector:OuterProduct(v1) end

--- 
--- @param gridSize int
--- @overload fun(): 
function vector.Vector:SnapToGrid(gridSize) end

--- 
--- @param idx int
function vector.Vector:Get(idx) end

--- 
--- @param b vector.Vector
--- @param factor number
function vector.Vector:Slerp(b, factor) end

--- 
--- @param idx int
--- @param val number
--- @overload fun(x: number, y: number, z: number): 
--- @overload fun(vecB: vector.Vector): 
function vector.Vector:Set(idx, val) end

--- 
--- @return vector.Vector ret0
function vector.Vector:Copy() end

--- 
--- @param 1 vector.Vector
function vector.Vector:__add(arg1) end

--- 
--- @param b vector.Vector
--- @return number ret0
function vector.Vector:DotProduct(b) end

--- 
--- @param vecB vector.Vector
--- @return vector.Vector ret0
function vector.Vector:Cross(vecB) end

--- 
--- @param b vector.Vector
--- @overload fun(b: vector.Vector, epsilon: number): 
function vector.Vector:Equals(b) end


