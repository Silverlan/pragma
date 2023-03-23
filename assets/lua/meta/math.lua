--- @meta
--- 
--- @class math
math = {}

--- 
--- @param val number
--- @param tgt number
--- @param inc number
--- @return number ret0
function math.approach(val, tgt, inc) end

--- 
--- @param angA number
--- @param angB number
--- @param amount number
--- @return number ret0
function math.approach_angle(angA, angB, amount) end

--- 
--- @param v number
--- @param min number
--- @param max number
--- @return bool ret0
function math.is_in_range(v, min, max) end

--- 
--- @param angA number
--- @param angB number
--- @return number ret0
function math.get_angle_difference(angA, angB) end

--- 
--- @param ang number
--- @param min number
--- @param max number
--- @return bool ret0
function math.is_angle_in_range(ang, min, max) end

--- 
--- @param v int
--- @return int ret0
function math.get_next_power_of_2(v) end

--- 
--- @param t number
--- @param type enum umath::EaseType
--- @return number ret0
function math.ease_out(t, type) end

--- 
--- @param arg0 number
--- @param arg1 number
--- @return number ret0
--- @overload fun(angle: number): number
function math.normalize_angle(arg0, arg1) end

--- 
--- @param val number
--- @param tgt number
--- @param inc number
--- @return number ret0
function math.calc_ballistic_angle_of_reach(val, tgt, inc) end

--- 
--- @param val number
--- @param min number
--- @param max number
--- @return number ret0
function math.clamp(val, min, max) end

--- 
--- @param ang number
--- @param min number
--- @param max number
--- @return number ret0
function math.clamp_angle(ang, min, max) end

--- 
--- @param v int
--- @return int ret0
function math.get_previous_power_of_2(v) end

--- 
--- @param v int
--- @return table ret0
function math.get_power_of_2_values(v) end

--- 
--- @param edge0 number
--- @param edge1 number
--- @param x number
--- @return number ret0
function math.smooth_step(edge0, edge1, x) end

--- 
--- @param v int
--- @return number ret0
function math.half_float_to_float(v) end

--- 
--- @param edge0 number
--- @param edge1 number
--- @param x number
--- @return number ret0
function math.smoother_step(edge0, edge1, x) end

--- 
--- @param start math.Vector
--- @param end math.Vector
--- @param vel math.Vector
--- @param gravity number
--- @return number ret0
--- @overload fun(start: math.Vector, end_: math.Vector, launchAngle: number, velocity: number, gravity: number): number
--- @overload fun(start: math.Vector, vel: math.Vector, gravity: number): number
--- @overload fun(start: math.Vector, launchAngle: number, velocity: number, gravity: number): number
--- @overload fun(start: vector.Vector, end_: vector.Vector, vel: vector.Vector, gravity: number): number
--- @overload fun(start: vector.Vector, end_: vector.Vector, launchAngle: number, velocity: number, gravity: number): number
--- @overload fun(start: vector.Vector, vel: vector.Vector, gravity: number): number
--- @overload fun(start: vector.Vector, launchAngle: number, velocity: number, gravity: number): number
function math.calc_ballistic_time_of_flight(start, end_, vel, gravity) end

--- 
--- @param speed number
--- @param gravity number
--- @param initialHeight number
--- @return number ret0
function math.calc_ballistic_range(speed, gravity, initialHeight) end

--- 
--- @param start math.Vector
--- @param vel math.Vector
--- @param gravity number
--- @param t number
--- @return math.Vector ret0
--- @overload fun(start: vector.Vector, vel: vector.Vector, gravity: number, t: number): math.Vector
function math.calc_ballistic_position(start, vel, gravity, t) end

--- 
--- @param start math.Vector
--- @param end math.Vector
--- @param angle number
--- @param gravity number
--- @return math.Vector ret0
--- @overload fun(start: vector.Vector, end_: vector.Vector, angle: number, gravity: number): math.Vector
function math.calc_ballistic_velocity(start, end_, angle, gravity) end

--- 
--- @param pos math.Vector
--- @param forward math.Vector
--- @param z number
--- @return math.Vector ret0
--- @overload fun(pos: vector.Vector, forward: vector.Vector, z: number): math.Vector
function math.get_frustum_plane_center(pos, forward, z) end

--- 
--- @param tRotations 
--- @param tWeights 
--- @return math.Quaternion ret0
--- @overload fun(t: ): math.Quaternion
function math.calc_average_rotation(tRotations, tWeights) end

--- 
--- @param f number
--- @return int ret0
--- @overload fun(f: number, multiple: number): int
function math.round(f) end

--- 
--- @param v number
--- @param c number
--- @param i number
--- @return number ret0
function math.map_value_to_fraction(v, c, i) end

--- 
--- @param min number
--- @param max number
--- @return number ret0
--- @overload fun(): number
function math.randomf(min, max) end

--- 
--- @param focalLength number
--- @param sensorSize number
--- @return number ret0
function math.calc_fov_from_focal_length(focalLength, sensorSize) end

--- 
--- @param f number
--- @return int ret0
function math.float_to_half_float(f) end

--- 
--- @param n number
--- @return int ret0
function math.sign(n) end

--- 
--- @param t number
--- @param type enum umath::EaseType
--- @return number ret0
function math.ease_in_out(t, type) end

--- 
--- @param v math.Vector
--- @return number ret0
--- @overload fun(v: math.Vector, seed: int): number
--- @overload fun(v: vector.Vector): number
--- @overload fun(v: vector.Vector, seed: int): number
function math.perlin_noise(v) end

--- 
--- @param fovDeg number
--- @param widthOrAspectRatio number
--- @return number ret0
--- @overload fun(fovDeg: number, widthOrAspectRatio: number, height: number): number
function math.horizontal_fov_to_vertical_fov(fovDeg, widthOrAspectRatio) end

--- 
--- @param a number
--- @param b number
--- @param f number
--- @return number ret0
function math.lerp(a, b, f) end

--- 
--- @param f number
--- @param gridSize int
--- @return int ret0
--- @overload fun(f: number): int
function math.snap_to_grid(f, gridSize) end

--- 
--- @param f number
--- @param gridSize number
--- @return number ret0
--- @overload fun(f: number): number
function math.snap_to_gridf(f, gridSize) end

--- 
--- @param flags int
--- @param flag int
--- @param set bool
--- @return int ret0
function math.set_flag(flags, flag, set) end

--- 
--- @param p0 math.Vector
--- @param p1 math.Vector
--- @param p2 math.Vector
--- @param p3 math.Vector
--- @param segmentCount int
--- @param curvature number
--- @return table ret0
--- @overload fun(p0: math.Vector, p1: math.Vector, p2: math.Vector, p3: math.Vector, segmentCount: int): table
--- @overload fun(p0: vector.Vector, p1: vector.Vector, p2: vector.Vector, p3: vector.Vector, segmentCount: int, curvature: number): table
--- @overload fun(p0: vector.Vector, p1: vector.Vector, p2: vector.Vector, p3: vector.Vector, segmentCount: int): table
function math.calc_hermite_spline(p0, p1, p2, p3, segmentCount, curvature) end

--- 
--- @param p0 math.Vector
--- @param p1 math.Vector
--- @param p2 math.Vector
--- @param p3 math.Vector
--- @param s number
--- @param curvature number
--- @return math.Vector ret0
--- @overload fun(p0: math.Vector, p1: math.Vector, p2: math.Vector, p3: math.Vector, s: number): math.Vector
--- @overload fun(p0: vector.Vector, p1: vector.Vector, p2: vector.Vector, p3: vector.Vector, s: number, curvature: number): math.Vector
--- @overload fun(p0: vector.Vector, p1: vector.Vector, p2: vector.Vector, p3: vector.Vector, s: number): math.Vector
function math.calc_hermite_spline_position(p0, p1, p2, p3, s, curvature) end

--- 
--- @param focalLengthInMM number
--- @param width number
--- @param height number
--- @return number ret0
function math.calc_vertical_fov(focalLengthInMM, width, height) end

--- 
--- @param projPos math.Vector
--- @param projSpeed number
--- @param tgt math.Vector
--- @param gravity number
--- @return table ret0
--- @overload fun(projPos: math.Vector, projSpeed: number, tgt: math.Vector, targetVel: math.Vector, gravity: number): table
--- @overload fun(projPos: vector.Vector, projSpeed: number, tgt: vector.Vector, gravity: number): table
--- @overload fun(projPos: vector.Vector, projSpeed: number, tgt: vector.Vector, targetVel: vector.Vector, gravity: number): table
function math.solve_ballistic_arc(projPos, projSpeed, tgt, gravity) end

--- 
--- @param projPos math.Vector
--- @param lateralSpeed number
--- @param tgt math.Vector
--- @param maxHeight number
--- @return math.Vector ret0_1
--- @return number ret0_2
--- @overload fun(projPos: math.Vector, lateralSpeed: number, tgt: math.Vector, targetVel: math.Vector, maxHeightOffset: number): math.Vector, number
--- @overload fun(projPos: vector.Vector, lateralSpeed: number, tgt: vector.Vector, maxHeight: number): math.Vector, number
--- @overload fun(projPos: vector.Vector, lateralSpeed: number, tgt: vector.Vector, targetVel: vector.Vector, maxHeightOffset: number): math.Vector, number
function math.solve_ballistic_arc_lateral(projPos, lateralSpeed, tgt, maxHeight) end

--- 
--- @param uv math.Vector2
function math.normalize_uv_coordinates(uv) end

--- 
--- @param val number
--- @return number ret0
function math.is_nan(val) end

--- 
--- @param val number
--- @return number ret0
function math.is_inf(val) end

--- 
--- @param val number
--- @return number ret0
function math.is_finite(val) end

--- 
--- @param x number
--- @return number ret0
function math.cot(x) end

--- 
--- @param diagonalFov number
--- @param aspectRatio number
--- @return number ret0
function math.diagonal_fov_to_vertical_fov(diagonalFov, aspectRatio) end

--- 
--- @param sensorSize number
--- @param focalLength number
--- @param aspectRatio number
--- @return number ret0
function math.calc_fov_from_lens(sensorSize, focalLength, aspectRatio) end

--- 
--- @param pos math.Vector
--- @param forward math.Vector
--- @param right math.Vector
--- @param up math.Vector
--- @param fovRad number
--- @param aspectRatio number
--- @param uv math.Vector2
--- @param z number
--- @return math.Vector ret0
--- @overload fun(pos: vector.Vector, forward: vector.Vector, right: vector.Vector, up: vector.Vector, fovRad: number, aspectRatio: number, uv: math.Vector2, z: number): math.Vector
function math.get_frustum_plane_point(pos, forward, right, up, fovRad, aspectRatio, uv, z) end

--- 
--- @param hfov number
--- @param sensorSize number
--- @return number ret0
function math.calc_focal_length_from_fov(hfov, sensorSize) end

--- 
--- @param fstop number
--- @param focalLength number
--- @return number ret0
--- @overload fun(fstop: number, focalLength: number, orthographicCamera: bool): number
function math.calc_aperture_size_from_fstop(fstop, focalLength) end

--- 
--- @param focalLengthInMM number
--- @param width number
--- @param height number
--- @return number ret0
function math.calc_horizontal_fov(focalLengthInMM, width, height) end

--- 
--- @param focalLengthInMM number
--- @param width number
--- @param height number
--- @return number ret0
function math.calc_diagonal_fov(focalLengthInMM, width, height) end

--- 
--- @param ior number
--- @return number ret0
function math.calc_dielectric_specular_reflection(ior) end

--- 
--- @param t number
--- @param type enum umath::EaseType
--- @return number ret0
function math.ease_in(t, type) end

--- 
--- @param fovRad number
--- @param aspectRatio number
--- @param z number
--- @return number ret0_1
--- @return number ret0_2
function math.get_frustum_plane_size(fovRad, aspectRatio, z) end

--- 
--- @param pos math.Vector
--- @param forward math.Vector
--- @param up math.Vector
--- @param fovRad number
--- @param aspectRatio number
--- @param z number
--- @return table ret0_1
--- @return number ret0_2
--- @return number ret0_3
--- @overload fun(pos: vector.Vector, forward: vector.Vector, up: vector.Vector, fovRad: number, aspectRatio: number, z: number): table, number, number
function math.get_frustum_plane_boundaries(pos, forward, up, fovRad, aspectRatio, z) end

--- 
--- @param fovDeg number
--- @param widthOrAspectRatio number
--- @return number ret0
--- @overload fun(fovDeg: number, widthOrAspectRatio: number, height: number): number
function math.vertical_fov_to_horizontal_fov(fovDeg, widthOrAspectRatio) end

--- 
--- @param time number
--- @param cp0Time number
--- @param cp0Val number
--- @param cp0OutTime number
--- @param cp0OutVal number
--- @param cp1InTime number
--- @param cp1InVal number
--- @param cp1Time number
--- @param cp1Val number
--- @return number ret0
function math.calc_bezier_point(time, cp0Time, cp0Val, cp0OutTime, cp0OutVal, cp1InTime, cp1InVal, cp1Time, cp1Val) end

--- 
--- @param value number
--- @param lower number
--- @param upper number
--- @return number ret0
function math.map_value_to_range(value, lower, upper) end


--- 
--- @class math.Transform
--- @field z number 
--- @field yaw number 
--- @field roll number 
--- @field x number 
--- @field y number 
--- @field pitch number 
--- @overload fun():math.Transform
--- @overload fun(arg1: math.Quaternion):math.Transform
--- @overload fun(arg1: math.Vector):math.Transform
--- @overload fun(arg1: math.Vector, arg2: math.Quaternion):math.Transform
--- @overload fun(arg1: math.Mat4):math.Transform
--- @overload fun(arg1: vector.Vector):math.Transform
--- @overload fun(arg1: vector.Vector, arg2: math.Quaternion):math.Transform
math.Transform = {}

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function math.Transform:GetForward() end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function math.Transform:GetUp() end

--- 
--- @param 1 math.Plane
--- @overload fun(arg1: math.Quaternion): 
--- @overload fun(arg1: math.Vector): 
--- @overload fun(arg1: math.ScaledTransform): 
--- @overload fun(arg1: math.Transform): 
--- @overload fun(arg1: vector.Vector): 
function math.Transform:__mul(arg1) end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function math.Transform:GetRight() end

--- 
--- @return bool ret0
function math.Transform:IsIdentity() end

--- 
--- @return math.Plane ret0
function math.Transform:ToPlane() end

--- 
--- @param arg1 math.Vector
--- @overload fun(arg1: vector.Vector): 
function math.Transform:TranslateGlobal(arg1) end

--- 
function math.Transform:SetIdentity() end

--- 
--- @param arg1 math.Vector
--- @overload fun(arg1: vector.Vector): 
function math.Transform:SetOrigin(arg1) end

--- 
--- @param arg1 math.Vector
--- @overload fun(arg1: vector.Vector): 
function math.Transform:TranslateLocal(arg1) end

--- 
--- @param arg1 math.Quaternion
function math.Transform:RotateGlobal(arg1) end

--- 
--- @param arg1 math.Quaternion
function math.Transform:RotateLocal(arg1) end

--- 
--- @param t2 math.Transform
function math.Transform:TransformGlobal(t2) end

--- 
--- @param t2 math.Transform
function math.Transform:TransformLocal(t2) end

--- 
--- @param arg1 math.Vector
--- @param arg2 number
--- @overload fun(arg1: vector.Vector, arg2: number): 
function math.Transform:Reflect(arg1, arg2) end

--- 
--- @return math.Transform ret0
function math.Transform:GetInverse() end

--- 
--- @return math.Mat4 ret0
function math.Transform:ToMatrix() end

--- 
--- @param arg1 math.Quaternion
function math.Transform:SetRotation(arg1) end

--- 
--- @param m math.Mat4
function math.Transform:SetMatrix(m) end

--- 
--- @param arg1 math.Transform
--- @param arg2 number
function math.Transform:Interpolate(arg1, arg2) end

--- 
--- @return math.Transform ret0
function math.Transform:Copy() end

--- 
--- @param 1 math.Transform
function math.Transform:__eq(arg1) end

--- 
--- @param tOther math.Transform
function math.Transform:Set(tOther) end

--- 
--- @param arg1 math.EulerAngles
function math.Transform:SetAngles(arg1) end

--- 
function math.Transform:__tostring() end

--- 
--- @param arg1 number
function math.Transform:InterpolateToIdentity(arg1) end

--- 
--- @return math.EulerAngles ret0
function math.Transform:GetAngles() end

--- 
--- @return math.Quaternion ret0
function math.Transform:GetRotation() end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function math.Transform:GetOrigin() end


--- 
--- @class math.Vector2i
--- @field x int 
--- @field y int 
--- @overload fun(args: string):math.Vector2i
--- @overload fun(arg1: int, arg2: int):math.Vector2i
--- @overload fun():math.Vector2i
math.Vector2i = {}

--- 
function math.Vector2i:__unm() end

--- 
--- @param 1 math.Vector2i
--- @overload fun(arg1: int): 
function math.Vector2i:__mul(arg1) end

--- 
function math.Vector2i:__tostring() end

--- 
--- @param idx int
function math.Vector2i:Get(idx) end

--- 
--- @param 1 math.Vector2i
function math.Vector2i:__sub(arg1) end

--- 
--- @return math.Vector2i ret0
function math.Vector2i:Copy() end

--- 
--- @param 1 math.Vector2i
function math.Vector2i:__eq(arg1) end

--- 
--- @param 1 math.Vector2i
function math.Vector2i:__add(arg1) end

--- 
--- @param 1 math.Vector2i
--- @overload fun(arg1: int): 
function math.Vector2i:__div(arg1) end


--- 
--- @class math.Quaternion
--- @field x number 
--- @field y number 
--- @field z number 
--- @field w number 
--- @overload fun(arg1: math.Vector, arg2: math.Vector):math.Quaternion
--- @overload fun(arg1: math.Vector, arg2: math.Vector, arg3: math.Vector):math.Quaternion
--- @overload fun(arg1: math.Vector, arg2: number):math.Quaternion
--- @overload fun():math.Quaternion
--- @overload fun(arg1: math.Quaternion):math.Quaternion
--- @overload fun(arg1: number, arg2: number, arg3: number, arg4: number):math.Quaternion
--- @overload fun(arg1: vector.Vector, arg2: vector.Vector):math.Quaternion
--- @overload fun(arg1: vector.Vector, arg2: vector.Vector, arg3: vector.Vector):math.Quaternion
--- @overload fun(arg1: vector.Vector, arg2: number):math.Quaternion
math.Quaternion = {}

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function math.Quaternion:GetForward() end

--- 
function math.Quaternion:Normalize() end

--- 
--- @return math.Mat4 ret0
function math.Quaternion:ToMatrix() end

--- 
--- @param b math.ScaledTransform
--- @return math.Quaternion ret0
--- @overload fun(b: math.Transform): math.Quaternion
function math.Quaternion:Mul(b) end

--- 
--- @param 1 math.Quaternion
function math.Quaternion:__eq(arg1) end

--- 
--- @return math.EulerAngles ret0
--- @overload fun(rotationOrder: int): math.EulerAngles
function math.Quaternion:ToEulerAngles() end

--- 
--- @return math.Quaternion ret0
function math.Quaternion:GetNormal() end

--- 
--- @param q1 math.Quaternion
--- @return number ret0
function math.Quaternion:Distance(q1) end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function math.Quaternion:GetUp() end

--- 
--- @return math.Quaternion ret0
function math.Quaternion:Copy() end

--- 
--- @param q2 math.Quaternion
--- @return number ret0
function math.Quaternion:DotProduct(q2) end

--- 
--- @return math.Quaternion ret0
function math.Quaternion:GetConjugate() end

--- 
--- @param ang math.EulerAngles
--- @overload fun(axis: math.Vector, ang: number): 
--- @overload fun(axis: vector.Vector, ang: number): 
function math.Quaternion:Rotate(ang) end

--- 
--- @param q2 math.Quaternion
--- @param factor number
--- @return math.Quaternion ret0
function math.Quaternion:Lerp(q2, factor) end

--- 
--- @param q2 math.Quaternion
--- @param factor number
--- @return math.Quaternion ret0
function math.Quaternion:Slerp(q2, factor) end

--- 
--- @param idx int
function math.Quaternion:Get(idx) end

--- 
--- @param idx int
--- @param value number
--- @overload fun(rotNew: math.Quaternion): 
--- @overload fun(w: number, x: number, y: number, z: number): 
function math.Quaternion:Set(idx, value) end

--- 
--- @param 1 math.Quaternion
--- @overload fun(arg1: math.Quaternion): 
--- @overload fun(arg1: math.Vector): 
--- @overload fun(arg1: number): 
--- @overload fun(arg1: vector.Vector): 
function math.Quaternion:__mul(arg1) end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function math.Quaternion:GetRight() end

--- 
--- @return math.Vector ret0_1
--- @return math.Vector ret0_2
--- @return math.Vector ret0_3
--- @overload fun(): math.Vector, math.Vector, math.Vector
function math.Quaternion:GetOrientation() end

--- 
function math.Quaternion:__unm() end

--- 
function math.Quaternion:Inverse() end

--- 
--- @return math.Vector ret0_1
--- @return number ret0_2
--- @overload fun(): math.Vector, number
function math.Quaternion:ToAxisAngle() end

--- 
--- @param ang number
function math.Quaternion:RotateX(ang) end

--- 
--- @param ang number
function math.Quaternion:RotateY(ang) end

--- 
--- @param ang number
function math.Quaternion:RotateZ(ang) end

--- 
--- @param up math.Vector
--- @param targetDir math.Vector
--- @param rotAmount math.Vector2
--- @return math.Quaternion ret0_1
--- @return math.Vector2 ret0_2
--- @overload fun(up: math.Vector, targetDir: math.Vector, rotAmount: math.Vector2, pitchLimit: math.Vector2): math.Quaternion, math.Vector2
--- @overload fun(up: math.Vector, targetDir: math.Vector, rotAmount: math.Vector2, pitchLimit: math.Vector2, yawLimit: math.Vector2): math.Quaternion, math.Vector2
--- @overload fun(up: math.Vector, targetDir: math.Vector, rotAmount: math.Vector2, pitchLimit: math.Vector2, yawLimit: math.Vector2, rotPivot: math.Quaternion): math.Quaternion, math.Vector2
--- @overload fun(up: math.Vector, targetDir: math.Vector, rotAmount: math.Vector2, pitchLimit: math.Vector2, yawLimit: math.Vector2, rotPivot: math.Quaternion, angOffset: math.EulerAngles): math.Quaternion, math.Vector2
--- @overload fun(up: vector.Vector, targetDir: vector.Vector, rotAmount: math.Vector2): math.Quaternion, math.Vector2
--- @overload fun(up: vector.Vector, targetDir: vector.Vector, rotAmount: math.Vector2, pitchLimit: math.Vector2): math.Quaternion, math.Vector2
--- @overload fun(up: vector.Vector, targetDir: vector.Vector, rotAmount: math.Vector2, pitchLimit: math.Vector2, yawLimit: math.Vector2): math.Quaternion, math.Vector2
--- @overload fun(up: vector.Vector, targetDir: vector.Vector, rotAmount: math.Vector2, pitchLimit: math.Vector2, yawLimit: math.Vector2, rotPivot: math.Quaternion): math.Quaternion, math.Vector2
--- @overload fun(up: vector.Vector, targetDir: vector.Vector, rotAmount: math.Vector2, pitchLimit: math.Vector2, yawLimit: math.Vector2, rotPivot: math.Quaternion, angOffset: math.EulerAngles): math.Quaternion, math.Vector2
function math.Quaternion:ApproachDirection(up, targetDir, rotAmount) end

--- 
--- @param bounds math.EulerAngles
--- @return math.Quaternion ret0
--- @overload fun(minBounds: math.EulerAngles, maxBounds: math.EulerAngles): math.Quaternion
function math.Quaternion:ClampRotation(bounds) end

--- 
--- @param axis int
function math.Quaternion:MirrorAxis(axis) end

--- 
--- @param 1 number
function math.Quaternion:__div(arg1) end

--- 
--- @return number ret0
function math.Quaternion:Length() end

--- 
--- @return math.Quaternion ret0
function math.Quaternion:GetInverse() end

--- 
function math.Quaternion:__tostring() end


--- 
--- @class math.NoiseModule
math.NoiseModule = {}

--- 
--- @param float number
function math.NoiseModule:SetScale(float) end

--- 
--- @param v math.Vector
--- @overload fun(v: vector.Vector): 
function math.NoiseModule:GetValue(v) end


--- 
--- @class math.NoiseModule.Displace: math.NoiseModule
math.NoiseModule.Displace = {}


--- 
--- @class math.NoiseModule.Exponent: math.NoiseModule
math.NoiseModule.Exponent = {}


--- 
--- @class math.NoiseModule.Power: math.NoiseModule
math.NoiseModule.Power = {}


--- 
--- @class math.NoiseModule.Blend: math.NoiseModule
math.NoiseModule.Blend = {}


--- 
--- @class math.NoiseModule.Checkerboard: math.NoiseModule
math.NoiseModule.Checkerboard = {}


--- 
--- @class math.NoiseModule.Cylinders: math.NoiseModule
math.NoiseModule.Cylinders = {}


--- 
--- @class math.NoiseModule.Const: math.NoiseModule
math.NoiseModule.Const = {}


--- 
--- @class math.NoiseModule.Max: math.NoiseModule
math.NoiseModule.Max = {}


--- 
--- @class math.NoiseModule.Abs: math.NoiseModule
math.NoiseModule.Abs = {}


--- 
--- @class math.NoiseModule.RotatePoint: math.NoiseModule
math.NoiseModule.RotatePoint = {}


--- 
--- @class math.NoiseModule.Min: math.NoiseModule
math.NoiseModule.Min = {}


--- 
--- @class math.NoiseModule.Perlin: math.NoiseModule
math.NoiseModule.Perlin = {}

--- 
--- @param octaveCount int
function math.NoiseModule.Perlin:SetOctaveCount(octaveCount) end

--- 
--- @param persistence number
function math.NoiseModule.Perlin:SetPersistence(persistence) end

--- 
--- @param seed int
function math.NoiseModule.Perlin:SetSeed(seed) end

--- 
function math.NoiseModule.Perlin:GetFrequency() end

--- 
function math.NoiseModule.Perlin:GetLacunarity() end

--- 
function math.NoiseModule.Perlin:GetNoiseQuality() end

--- 
function math.NoiseModule.Perlin:GetOctaveCount() end

--- 
function math.NoiseModule.Perlin:GetPersistence() end

--- 
function math.NoiseModule.Perlin:GetSeed() end

--- 
--- @param frequency number
function math.NoiseModule.Perlin:SetFrequency(frequency) end

--- 
--- @param lacunarity number
function math.NoiseModule.Perlin:SetLacunarity(lacunarity) end

--- 
--- @param quality int
function math.NoiseModule.Perlin:SetNoiseQuality(quality) end


--- 
--- @class math.NoiseModule.Terrace: math.NoiseModule
math.NoiseModule.Terrace = {}


--- 
--- @class math.NoiseModule.Cache: math.NoiseModule
math.NoiseModule.Cache = {}


--- 
--- @class math.NoiseModule.Spheres: math.NoiseModule
math.NoiseModule.Spheres = {}


--- 
--- @class math.NoiseModule.TranslatePoint: math.NoiseModule
math.NoiseModule.TranslatePoint = {}


--- 
--- @class math.NoiseModule.Invert: math.NoiseModule
math.NoiseModule.Invert = {}


--- 
--- @class math.NoiseModule.ScalePoint: math.NoiseModule
math.NoiseModule.ScalePoint = {}


--- 
--- @class math.NoiseModule.Select: math.NoiseModule
math.NoiseModule.Select = {}


--- 
--- @class math.NoiseModule.Billow: math.NoiseModule
math.NoiseModule.Billow = {}

--- 
--- @param octaveCount int
function math.NoiseModule.Billow:SetOctaveCount(octaveCount) end

--- 
--- @param persistence number
function math.NoiseModule.Billow:SetPersistence(persistence) end

--- 
--- @param seed int
function math.NoiseModule.Billow:SetSeed(seed) end

--- 
function math.NoiseModule.Billow:GetFrequency() end

--- 
function math.NoiseModule.Billow:GetLacunarity() end

--- 
function math.NoiseModule.Billow:GetNoiseQuality() end

--- 
function math.NoiseModule.Billow:GetOctaveCount() end

--- 
function math.NoiseModule.Billow:GetPersistence() end

--- 
function math.NoiseModule.Billow:GetSeed() end

--- 
--- @param frequency number
function math.NoiseModule.Billow:SetFrequency(frequency) end

--- 
--- @param lacunarity number
function math.NoiseModule.Billow:SetLacunarity(lacunarity) end

--- 
--- @param quality int
function math.NoiseModule.Billow:SetNoiseQuality(quality) end


--- 
--- @class math.NoiseModule.Clamp: math.NoiseModule
math.NoiseModule.Clamp = {}


--- 
--- @class math.NoiseModule.Curve: math.NoiseModule
math.NoiseModule.Curve = {}


--- 
--- @class math.NoiseModule.Multiply: math.NoiseModule
math.NoiseModule.Multiply = {}


--- 
--- @class math.NoiseModule.RidgedMulti: math.NoiseModule
math.NoiseModule.RidgedMulti = {}

--- 
--- @param octaveCount int
function math.NoiseModule.RidgedMulti:SetOctaveCount(octaveCount) end

--- 
--- @param seed int
function math.NoiseModule.RidgedMulti:SetSeed(seed) end

--- 
function math.NoiseModule.RidgedMulti:GetFrequency() end

--- 
function math.NoiseModule.RidgedMulti:GetLacunarity() end

--- 
--- @param frequency number
function math.NoiseModule.RidgedMulti:SetFrequency(frequency) end

--- 
function math.NoiseModule.RidgedMulti:GetNoiseQuality() end

--- 
function math.NoiseModule.RidgedMulti:GetOctaveCount() end

--- 
function math.NoiseModule.RidgedMulti:GetSeed() end

--- 
--- @param lacunarity number
function math.NoiseModule.RidgedMulti:SetLacunarity(lacunarity) end

--- 
--- @param quality int
function math.NoiseModule.RidgedMulti:SetNoiseQuality(quality) end


--- 
--- @class math.NoiseModule.ScaleBias: math.NoiseModule
math.NoiseModule.ScaleBias = {}


--- 
--- @class math.NoiseModule.Turbulance: math.NoiseModule
math.NoiseModule.Turbulance = {}


--- 
--- @class math.NoiseModule.Voronoi: math.NoiseModule
math.NoiseModule.Voronoi = {}

--- 
--- @param seed int
function math.NoiseModule.Voronoi:SetSeed(seed) end

--- 
function math.NoiseModule.Voronoi:GetFrequency() end

--- 
function math.NoiseModule.Voronoi:GetSeed() end

--- 
--- @param frequency number
function math.NoiseModule.Voronoi:SetFrequency(frequency) end

--- 
function math.NoiseModule.Voronoi:GetDisplacement() end

--- 
--- @param displacement number
function math.NoiseModule.Voronoi:SetDisplacement(displacement) end


--- 
--- @class math.Vectori
--- @field x int 
--- @field y int 
--- @field z int 
--- @overload fun(args: string):math.Vectori
--- @overload fun(arg1: int, arg2: int, arg3: int):math.Vectori
--- @overload fun():math.Vectori
math.Vectori = {}

--- 
function math.Vectori:__unm() end

--- 
--- @param 1 math.Vectori
--- @overload fun(arg1: int): 
function math.Vectori:__mul(arg1) end

--- 
function math.Vectori:__tostring() end

--- 
--- @param idx int
function math.Vectori:Get(idx) end

--- 
--- @param 1 math.Vectori
function math.Vectori:__sub(arg1) end

--- 
--- @return math.Vectori ret0
function math.Vectori:Copy() end

--- 
--- @param 1 math.Vectori
function math.Vectori:__eq(arg1) end

--- 
--- @param 1 math.Vectori
function math.Vectori:__add(arg1) end

--- 
--- @param 1 math.Vectori
--- @overload fun(arg1: int): 
function math.Vectori:__div(arg1) end


--- 
--- @class math.Vector4i
--- @field x int 
--- @field y int 
--- @field z int 
--- @field w int 
--- @overload fun(args: string):math.Vector4i
--- @overload fun(arg1: int, arg2: int, arg3: int, arg4: int):math.Vector4i
--- @overload fun():math.Vector4i
math.Vector4i = {}

--- 
function math.Vector4i:__unm() end

--- 
--- @param 1 math.Vector4i
--- @overload fun(arg1: math.Vector4i): 
--- @overload fun(arg1: int): 
function math.Vector4i:__mul(arg1) end

--- 
function math.Vector4i:__tostring() end

--- 
--- @param idx int
function math.Vector4i:Get(idx) end

--- 
--- @param 1 math.Vector4i
function math.Vector4i:__sub(arg1) end

--- 
--- @return math.Vector4i ret0
function math.Vector4i:Copy() end

--- 
--- @param 1 math.Vector4i
function math.Vector4i:__eq(arg1) end

--- 
--- @param 1 math.Vector4i
function math.Vector4i:__add(arg1) end

--- 
--- @param 1 math.Vector4i
--- @overload fun(arg1: int): 
function math.Vector4i:__div(arg1) end


--- 
--- @class math.Mat2x4
--- @overload fun(args: string):math.Mat2x4
--- @overload fun(arg1: math.Mat4x3):math.Mat2x4
--- @overload fun(arg1: math.Mat4x2):math.Mat2x4
--- @overload fun(arg1: math.Mat4):math.Mat2x4
--- @overload fun(arg1: math.Mat3x4):math.Mat2x4
--- @overload fun(arg1: math.Mat3x2):math.Mat2x4
--- @overload fun(arg1: math.Mat3):math.Mat2x4
--- @overload fun(arg1: math.Mat2x4):math.Mat2x4
--- @overload fun(arg1: math.Mat2x3):math.Mat2x4
--- @overload fun(arg1: math.Mat2):math.Mat2x4
--- @overload fun(arg1: number):math.Mat2x4
--- @overload fun():math.Mat2x4
--- @overload fun(arg1: number, arg2: number, arg3: number, arg4: number, arg5: number, arg6: number, arg7: number, arg8: number):math.Mat2x4
math.Mat2x4 = {}

--- 
function math.Mat2x4:__tostring() end

--- 
--- @param 1 math.Mat2x4
function math.Mat2x4:__sub(arg1) end

--- 
--- @param arg2 int
--- @param arg3 int
function math.Mat2x4:Get(arg2, arg3) end

--- 
--- @param 1 math.Mat2x4
function math.Mat2x4:__eq(arg1) end

--- 
--- @param 1 math.Mat2x4
function math.Mat2x4:__add(arg1) end

--- 
function math.Mat2x4:Copy() end

--- 
function math.Mat2x4:GetTranspose() end

--- 
function math.Mat2x4:Transpose() end

--- 
--- @param mat2 math.Mat2x4
--- @overload fun(x1: number, y1: number, z1: number, w1: number, x2: number, y2: number, z2: number, w2: number): 
--- @overload fun(arg2: int, arg3: int, arg4: number): 
function math.Mat2x4:Set(mat2) end

--- 
--- @param 1 math.Vector2
--- @overload fun(arg1: math.Mat2x4): 
--- @overload fun(arg1: number): 
function math.Mat2x4:__mul(arg1) end

--- 
--- @param 1 math.Mat2x4
--- @overload fun(arg1: number): 
function math.Mat2x4:__div(arg1) end


--- 
--- @class math.PIDController
--- @overload fun(arg1: number, arg2: number, arg3: number, arg4: number, arg5: number):math.PIDController
--- @overload fun(arg1: number, arg2: number, arg3: number):math.PIDController
--- @overload fun():math.PIDController
math.PIDController = {}

--- 
--- @param arg1 number
function math.PIDController:SetDerivativeTerm(arg1) end

--- 
function math.PIDController:Reset() end

--- 
--- @param arg1 number
--- @param arg2 number
function math.PIDController:SetRange(arg1, arg2) end

--- 
function math.PIDController:ClearRange() end

--- 
--- @param arg1 number
--- @param arg2 number
--- @param arg3 number
--- @return number ret0
function math.PIDController:Calculate(arg1, arg2, arg3) end

--- 
--- @param arg1 number
function math.PIDController:SetMin(arg1) end

--- 
--- @param arg1 number
function math.PIDController:SetMax(arg1) end

--- 
function math.PIDController:GetRange() end

--- 
--- @param arg1 number
function math.PIDController:SetIntegralTerm(arg1) end

--- 
--- @param arg1 number
--- @param arg2 number
--- @param arg3 number
function math.PIDController:SetTerms(arg1, arg2, arg3) end

--- 
--- @return number ret0
function math.PIDController:GetProportionalTerm() end

--- 
--- @return number ret0
function math.PIDController:GetIntegralTerm() end

--- 
--- @return number ret0
function math.PIDController:GetDerivativeTerm() end

--- 
function math.PIDController:GetTerms() end

--- 
--- @param arg1 number
function math.PIDController:SetProportionalTerm(arg1) end


--- 
--- @class math.Vector
--- @field x number 
--- @field y number 
--- @field z number 
--- @field r number 
--- @field b number 
--- @field g number 
--- @overload fun(args: string):math.Vector
--- @overload fun(arg1: math.Vector2, arg2: number):math.Vector
--- @overload fun(arg1: number, arg2: number, arg3: number):math.Vector
--- @overload fun():math.Vector
math.Vector = {}

--- 
--- @param 1 math.Vector
--- @overload fun(arg1: math.Vector): 
--- @overload fun(arg1: math.Quaternion): 
--- @overload fun(arg1: math.Vector): 
--- @overload fun(arg1: number): 
function math.Vector:__mul(arg1) end

--- 
--- @return number ret0
function math.Vector:GetPitch() end

--- 
--- @return number ret0
function math.Vector:GetYaw() end

--- 
--- @param b math.ScaledTransform
--- @overload fun(b: math.Transform): 
function math.Vector:Mul(b) end

--- 
--- @param 1 math.Vector
function math.Vector:__eq(arg1) end

--- 
--- @return math.EulerAngles ret0
function math.Vector:ToEulerAngles() end

--- 
--- @return number ret0
function math.Vector:LengthSqr() end

--- 
--- @return math.Vector ret0
function math.Vector:GetNormal() end

--- 
function math.Vector:Normalize() end

--- 
--- @return number ret0
function math.Vector:Length() end

--- 
--- @param b math.Vector
--- @return number ret0
function math.Vector:Distance(b) end

--- 
--- @param p1 math.Vector
--- @return number ret0
function math.Vector:DistanceSqr(p1) end

--- 
--- @param n math.Vector
--- @param d number
function math.Vector:Reflect(n, d) end

--- 
--- @param orientation math.Quaternion
--- @overload fun(normal: math.Vector, angle: number): 
--- @overload fun(ang: math.EulerAngles): 
function math.Vector:Rotate(orientation) end

--- 
--- @param p1 math.Vector
--- @param n math.Vector
--- @return number ret0
function math.Vector:PlanarDistanceSqr(p1, n) end

--- 
--- @param vecB math.Vector
--- @return math.Vector ret0
function math.Vector:Cross(vecB) end

--- 
--- @param b math.Vector
--- @return number ret0
function math.Vector:DotProduct(b) end

--- 
--- @param origin math.Vector
--- @param ang math.EulerAngles
function math.Vector:RotateAround(origin, ang) end

--- 
--- @param vecB math.Vector
--- @param factor number
--- @return math.Vector ret0
function math.Vector:Lerp(vecB, factor) end

--- 
--- @param b math.Vector
--- @overload fun(b: math.Vector, epsilon: number): 
function math.Vector:Equals(b) end

--- 
--- @param b math.Vector
--- @return number ret0
function math.Vector:GetAngle(b) end

--- 
--- @param n math.Vector
--- @return math.Vector ret0
function math.Vector:Project(n) end

--- 
--- @param p1 math.Vector
--- @param n math.Vector
--- @return number ret0
function math.Vector:PlanarDistance(p1, n) end

--- 
--- @param b math.Vector
--- @param factor number
function math.Vector:Slerp(b, factor) end

--- 
--- @param idx int
function math.Vector:Get(idx) end

--- 
--- @param gridSize int
--- @overload fun(): 
function math.Vector:SnapToGrid(gridSize) end

--- 
--- @return math.Vector ret0
function math.Vector:GetPerpendicular() end

--- 
--- @param v1 math.Vector
--- @return math.Mat3 ret0
function math.Vector:OuterProduct(v1) end

--- 
--- @param vp math.Mat4
--- @return math.Vector2 ret0
function math.Vector:ToScreenUv(vp) end

--- 
function math.Vector:__unm() end

--- 
--- @param 1 math.Vector
function math.Vector:__add(arg1) end

--- 
--- @param 1 math.Vector
function math.Vector:__sub(arg1) end

--- 
--- @param 1 math.Vector
--- @overload fun(arg1: number): 
function math.Vector:__div(arg1) end

--- 
function math.Vector:__tostring() end

--- 
--- @param n math.Vector
--- @param d number
--- @return math.Vector ret0
function math.Vector:ProjectToPlane(n, d) end

--- 
--- @return math.Mat4 ret0
function math.Vector:ToMatrix() end

--- 
--- @param b math.Vector
--- @return math.Quaternion ret0
function math.Vector:GetRotation(b) end

--- 
--- @param idx int
--- @param val number
--- @overload fun(x: number, y: number, z: number): 
--- @overload fun(vecB: math.Vector): 
function math.Vector:Set(idx, val) end

--- 
--- @return math.Vector ret0
function math.Vector:Copy() end


--- 
--- @class math.Vector4
--- @field y number 
--- @field a number 
--- @field z number 
--- @field b number 
--- @field r number 
--- @field g number 
--- @field w number 
--- @field x number 
--- @overload fun(args: string):math.Vector4
--- @overload fun(arg1: math.Vector, arg2: number):math.Vector4
--- @overload fun(arg1: number, arg2: number, arg3: number, arg4: number):math.Vector4
--- @overload fun():math.Vector4
--- @overload fun(arg1: vector.Vector, arg2: number):math.Vector4
math.Vector4 = {}

--- 
--- @param n math.Vector4
--- @return math.Vector4 ret0
function math.Vector4:Project(n) end

--- 
--- @param 1 math.Vector4
function math.Vector4:__add(arg1) end

--- 
--- @param 1 math.Vector4
function math.Vector4:__eq(arg1) end

--- 
--- @param idx int
function math.Vector4:Get(idx) end

--- 
--- @param 1 math.Vector4
function math.Vector4:__sub(arg1) end

--- 
--- @param x number
--- @param y number
--- @param z number
--- @param w number
--- @overload fun(vecB: math.Vector4): 
function math.Vector4:Set(x, y, z, w) end

--- 
--- @param 1 math.Vector4
--- @overload fun(arg1: math.Mat4): 
--- @overload fun(arg1: math.Vector4): 
--- @overload fun(arg1: number): 
function math.Vector4:__mul(arg1) end

--- 
--- @return math.Vector4 ret0
function math.Vector4:Copy() end

--- 
--- @param 1 math.Vector4
--- @overload fun(arg1: number): 
function math.Vector4:__div(arg1) end

--- 
function math.Vector4:__unm() end

--- 
--- @return math.Vector4 ret0
function math.Vector4:GetNormal() end

--- 
function math.Vector4:__tostring() end

--- 
--- @return number ret0
function math.Vector4:Length() end

--- 
--- @return number ret0
function math.Vector4:LengthSqr() end

--- 
--- @param b math.Vector4
--- @return number ret0
function math.Vector4:Distance(b) end

--- 
--- @param b math.Vector4
--- @return number ret0
function math.Vector4:DistanceSqr(b) end

--- 
function math.Vector4:Normalize() end

--- 
--- @param b math.Vector4
--- @return number ret0
function math.Vector4:DotProduct(b) end

--- 
--- @param vecB math.Vector4
--- @param factor number
--- @return math.Vector4 ret0
function math.Vector4:Lerp(vecB, factor) end


--- 
--- @class math.Vector2
--- @field x number 
--- @field y number 
--- @overload fun(args: string):math.Vector2
--- @overload fun(arg1: number, arg2: number):math.Vector2
--- @overload fun():math.Vector2
math.Vector2 = {}

--- 
--- @param b math.Vector2
--- @return number ret0
function math.Vector2:GetAngle(b) end

--- 
--- @param n math.Vector2
--- @return math.Vector2 ret0
function math.Vector2:Project(n) end

--- 
--- @param 1 math.Vector2
function math.Vector2:__add(arg1) end

--- 
--- @param 1 math.Vector2
--- @overload fun(arg1: number): 
function math.Vector2:__div(arg1) end

--- 
--- @param idx int
function math.Vector2:Get(idx) end

--- 
--- @param 1 math.Vector2
function math.Vector2:__sub(arg1) end

--- 
--- @param 1 math.Vector2
--- @overload fun(arg1: math.Vector2): 
--- @overload fun(arg1: number): 
function math.Vector2:__mul(arg1) end

--- 
--- @param x number
--- @param y number
--- @overload fun(vecB: math.Vector2): 
function math.Vector2:Set(x, y) end

--- 
function math.Vector2:__unm() end

--- 
--- @return math.Vector2 ret0
function math.Vector2:GetNormal() end

--- 
function math.Vector2:__tostring() end

--- 
--- @return math.Vector2 ret0
function math.Vector2:Copy() end

--- 
--- @return number ret0
function math.Vector2:Length() end

--- 
--- @return number ret0
function math.Vector2:LengthSqr() end

--- 
--- @param b math.Vector2
--- @return number ret0
function math.Vector2:Distance(b) end

--- 
--- @param b math.Vector2
--- @return number ret0
function math.Vector2:DistanceSqr(b) end

--- 
function math.Vector2:Normalize() end

--- 
--- @param 1 math.Vector2
function math.Vector2:__eq(arg1) end

--- 
--- @param b math.Vector2
--- @return math.Vector2 ret0
function math.Vector2:Cross(b) end

--- 
--- @param b math.Vector2
--- @return number ret0
function math.Vector2:DotProduct(b) end

--- 
--- @param ang math.EulerAngles
function math.Vector2:Rotate(ang) end

--- 
--- @param origin math.Vector2
--- @param ang math.EulerAngles
function math.Vector2:RotateAround(origin, ang) end

--- 
--- @param vecB math.Vector2
--- @param factor number
--- @return math.Vector2 ret0
function math.Vector2:Lerp(vecB, factor) end


--- 
--- @class math.Mat2x3
--- @overload fun(args: string):math.Mat2x3
--- @overload fun(arg1: math.Mat4x3):math.Mat2x3
--- @overload fun(arg1: math.Mat4x2):math.Mat2x3
--- @overload fun(arg1: math.Mat4):math.Mat2x3
--- @overload fun(arg1: math.Mat3x4):math.Mat2x3
--- @overload fun(arg1: math.Mat3x2):math.Mat2x3
--- @overload fun(arg1: math.Mat3):math.Mat2x3
--- @overload fun(arg1: math.Mat2x4):math.Mat2x3
--- @overload fun(arg1: math.Mat2x3):math.Mat2x3
--- @overload fun(arg1: math.Mat2):math.Mat2x3
--- @overload fun(arg1: number):math.Mat2x3
--- @overload fun():math.Mat2x3
--- @overload fun(arg1: number, arg2: number, arg3: number, arg4: number, arg5: number, arg6: number):math.Mat2x3
math.Mat2x3 = {}

--- 
function math.Mat2x3:__tostring() end

--- 
--- @param 1 math.Mat2x3
function math.Mat2x3:__sub(arg1) end

--- 
--- @param arg2 int
--- @param arg3 int
function math.Mat2x3:Get(arg2, arg3) end

--- 
--- @param 1 math.Mat2x3
function math.Mat2x3:__eq(arg1) end

--- 
--- @param 1 math.Mat2x3
function math.Mat2x3:__add(arg1) end

--- 
function math.Mat2x3:Copy() end

--- 
function math.Mat2x3:GetTranspose() end

--- 
function math.Mat2x3:Transpose() end

--- 
--- @param mat2 math.Mat2x3
--- @overload fun(x1: number, y1: number, z1: number, x2: number, y2: number, z2: number): 
--- @overload fun(arg2: int, arg3: int, arg4: number): 
function math.Mat2x3:Set(mat2) end

--- 
--- @param 1 math.Vector2
--- @overload fun(arg1: math.Mat2x3): 
--- @overload fun(arg1: number): 
function math.Mat2x3:__mul(arg1) end

--- 
--- @param 1 math.Mat2x3
--- @overload fun(arg1: number): 
function math.Mat2x3:__div(arg1) end


--- 
--- @class math.Mat3x4
--- @overload fun(args: string):math.Mat3x4
--- @overload fun(arg1: math.Mat4x3):math.Mat3x4
--- @overload fun(arg1: math.Mat4x2):math.Mat3x4
--- @overload fun(arg1: math.Mat4):math.Mat3x4
--- @overload fun(arg1: math.Mat3x4):math.Mat3x4
--- @overload fun(arg1: math.Mat3x2):math.Mat3x4
--- @overload fun(arg1: math.Mat3):math.Mat3x4
--- @overload fun(arg1: math.Mat2x4):math.Mat3x4
--- @overload fun(arg1: math.Mat2x3):math.Mat3x4
--- @overload fun(arg1: math.Mat2):math.Mat3x4
--- @overload fun(arg1: number):math.Mat3x4
--- @overload fun():math.Mat3x4
--- @overload fun(arg1: number, arg2: number, arg3: number, arg4: number, arg5: number, arg6: number, arg7: number, arg8: number, arg9: number, arg10: number, arg11: number, arg12: number):math.Mat3x4
math.Mat3x4 = {}

--- 
function math.Mat3x4:__tostring() end

--- 
--- @param 1 math.Mat3x4
function math.Mat3x4:__sub(arg1) end

--- 
--- @param arg2 int
--- @param arg3 int
function math.Mat3x4:Get(arg2, arg3) end

--- 
--- @param 1 math.Mat3x4
function math.Mat3x4:__eq(arg1) end

--- 
--- @param 1 math.Mat3x4
function math.Mat3x4:__add(arg1) end

--- 
function math.Mat3x4:Copy() end

--- 
function math.Mat3x4:GetTranspose() end

--- 
function math.Mat3x4:Transpose() end

--- 
--- @param mat2 math.Mat3x4
--- @overload fun(x1: number, y1: number, z1: number, w1: number, x2: number, y2: number, z2: number, w2: number, x3: number, y3: number, z3: number, w3: number): 
--- @overload fun(arg2: int, arg3: int, arg4: number): 
function math.Mat3x4:Set(mat2) end

--- 
--- @param 1 math.Vector
--- @overload fun(arg1: math.Mat3x4): 
--- @overload fun(arg1: number): 
--- @overload fun(arg1: vector.Vector): 
function math.Mat3x4:__mul(arg1) end

--- 
--- @param 1 math.Mat3x4
--- @overload fun(arg1: number): 
function math.Mat3x4:__div(arg1) end


--- 
--- @class math.Mat3
--- @overload fun(args: string):math.Mat3
--- @overload fun(arg1: math.Mat4x3):math.Mat3
--- @overload fun(arg1: math.Mat4x2):math.Mat3
--- @overload fun(arg1: math.Mat4):math.Mat3
--- @overload fun(arg1: math.Mat3x4):math.Mat3
--- @overload fun(arg1: math.Mat3x2):math.Mat3
--- @overload fun(arg1: math.Mat3):math.Mat3
--- @overload fun(arg1: math.Mat2x4):math.Mat3
--- @overload fun(arg1: math.Mat2x3):math.Mat3
--- @overload fun(arg1: math.Mat2):math.Mat3
--- @overload fun(arg1: number):math.Mat3
--- @overload fun():math.Mat3
--- @overload fun(arg1: math.Quaternion):math.Mat3
--- @overload fun(arg1: number, arg2: number, arg3: number, arg4: number, arg5: number, arg6: number, arg7: number, arg8: number, arg9: number):math.Mat3
math.Mat3 = {}

--- 
function math.Mat3:CalcEigenValues() end

--- 
function math.Mat3:Inverse() end

--- 
function math.Mat3:__tostring() end

--- 
--- @param 1 math.Mat3
function math.Mat3:__sub(arg1) end

--- 
function math.Mat3:GetInverse() end

--- 
--- @param 1 math.Mat3
function math.Mat3:__eq(arg1) end

--- 
--- @param 1 math.Mat3
function math.Mat3:__add(arg1) end

--- 
function math.Mat3:Copy() end

--- 
function math.Mat3:GetTranspose() end

--- 
--- @param arg2 int
--- @param arg3 int
function math.Mat3:Get(arg2, arg3) end

--- 
function math.Mat3:Transpose() end

--- 
--- @param mat2 math.Mat3
--- @overload fun(x1: number, y1: number, z1: number, x2: number, y2: number, z2: number, x3: number, y3: number, z3: number): 
--- @overload fun(arg2: int, arg3: int, arg4: number): 
function math.Mat3:Set(mat2) end

--- 
--- @param 1 math.Vector
--- @overload fun(arg1: math.Mat3): 
--- @overload fun(arg1: math.Mat3): 
--- @overload fun(arg1: number): 
--- @overload fun(arg1: vector.Vector): 
function math.Mat3:__mul(arg1) end

--- 
--- @param 1 math.Mat3
--- @overload fun(arg1: number): 
function math.Mat3:__div(arg1) end


--- 
--- @class math.Mat3x2
--- @overload fun(args: string):math.Mat3x2
--- @overload fun(arg1: math.Mat4x3):math.Mat3x2
--- @overload fun(arg1: math.Mat4x2):math.Mat3x2
--- @overload fun(arg1: math.Mat4):math.Mat3x2
--- @overload fun(arg1: math.Mat3x4):math.Mat3x2
--- @overload fun(arg1: math.Mat3x2):math.Mat3x2
--- @overload fun(arg1: math.Mat3):math.Mat3x2
--- @overload fun(arg1: math.Mat2x4):math.Mat3x2
--- @overload fun(arg1: math.Mat2x3):math.Mat3x2
--- @overload fun(arg1: math.Mat2):math.Mat3x2
--- @overload fun(arg1: number):math.Mat3x2
--- @overload fun():math.Mat3x2
--- @overload fun(arg1: number, arg2: number, arg3: number, arg4: number, arg5: number, arg6: number):math.Mat3x2
math.Mat3x2 = {}

--- 
function math.Mat3x2:__tostring() end

--- 
--- @param 1 math.Mat3x2
function math.Mat3x2:__sub(arg1) end

--- 
--- @param arg2 int
--- @param arg3 int
function math.Mat3x2:Get(arg2, arg3) end

--- 
--- @param 1 math.Mat3x2
function math.Mat3x2:__eq(arg1) end

--- 
--- @param 1 math.Mat3x2
function math.Mat3x2:__add(arg1) end

--- 
function math.Mat3x2:Copy() end

--- 
function math.Mat3x2:GetTranspose() end

--- 
function math.Mat3x2:Transpose() end

--- 
--- @param mat2 math.Mat3x2
--- @overload fun(x1: number, y1: number, x2: number, y2: number, x3: number, y3: number): 
--- @overload fun(arg2: int, arg3: int, arg4: number): 
function math.Mat3x2:Set(mat2) end

--- 
--- @param 1 math.Vector
--- @overload fun(arg1: math.Mat3x2): 
--- @overload fun(arg1: number): 
--- @overload fun(arg1: vector.Vector): 
function math.Mat3x2:__mul(arg1) end

--- 
--- @param 1 math.Mat3x2
--- @overload fun(arg1: number): 
function math.Mat3x2:__div(arg1) end


--- 
--- @class math.Mat4x3
--- @overload fun(args: string):math.Mat4x3
--- @overload fun(arg1: math.Mat4x3):math.Mat4x3
--- @overload fun(arg1: math.Mat4x2):math.Mat4x3
--- @overload fun(arg1: math.Mat4):math.Mat4x3
--- @overload fun(arg1: math.Mat3x4):math.Mat4x3
--- @overload fun(arg1: math.Mat3x2):math.Mat4x3
--- @overload fun(arg1: math.Mat3):math.Mat4x3
--- @overload fun(arg1: math.Mat2x4):math.Mat4x3
--- @overload fun(arg1: math.Mat2x3):math.Mat4x3
--- @overload fun(arg1: math.Mat2):math.Mat4x3
--- @overload fun(arg1: number):math.Mat4x3
--- @overload fun():math.Mat4x3
--- @overload fun(arg1: number, arg2: number, arg3: number, arg4: number, arg5: number, arg6: number, arg7: number, arg8: number, arg9: number, arg10: number, arg11: number, arg12: number):math.Mat4x3
math.Mat4x3 = {}

--- 
function math.Mat4x3:__tostring() end

--- 
--- @param 1 math.Mat4x3
function math.Mat4x3:__sub(arg1) end

--- 
--- @param arg2 int
--- @param arg3 int
function math.Mat4x3:Get(arg2, arg3) end

--- 
--- @param 1 math.Mat4x3
function math.Mat4x3:__eq(arg1) end

--- 
--- @param 1 math.Mat4x3
function math.Mat4x3:__add(arg1) end

--- 
function math.Mat4x3:Copy() end

--- 
function math.Mat4x3:GetTranspose() end

--- 
function math.Mat4x3:Transpose() end

--- 
--- @param mat2 math.Mat4x3
--- @overload fun(x1: number, y1: number, z1: number, x2: number, y2: number, z2: number, x3: number, y3: number, z3: number, x4: number, y4: number, z4: number): 
--- @overload fun(arg2: int, arg3: int, arg4: number): 
function math.Mat4x3:Set(mat2) end

--- 
--- @param 1 math.Vector4
--- @overload fun(arg1: math.Mat4x3): 
--- @overload fun(arg1: number): 
function math.Mat4x3:__mul(arg1) end

--- 
--- @param 1 math.Mat4x3
--- @overload fun(arg1: number): 
function math.Mat4x3:__div(arg1) end


--- 
--- @class math.Mat4x2
--- @overload fun(args: string):math.Mat4x2
--- @overload fun(arg1: math.Mat4x3):math.Mat4x2
--- @overload fun(arg1: math.Mat4x2):math.Mat4x2
--- @overload fun(arg1: math.Mat4):math.Mat4x2
--- @overload fun(arg1: math.Mat3x4):math.Mat4x2
--- @overload fun(arg1: math.Mat3x2):math.Mat4x2
--- @overload fun(arg1: math.Mat3):math.Mat4x2
--- @overload fun(arg1: math.Mat2x4):math.Mat4x2
--- @overload fun(arg1: math.Mat2x3):math.Mat4x2
--- @overload fun(arg1: math.Mat2):math.Mat4x2
--- @overload fun(arg1: number):math.Mat4x2
--- @overload fun():math.Mat4x2
--- @overload fun(arg1: number, arg2: number, arg3: number, arg4: number, arg5: number, arg6: number, arg7: number, arg8: number):math.Mat4x2
math.Mat4x2 = {}

--- 
function math.Mat4x2:__tostring() end

--- 
--- @param 1 math.Mat4x2
function math.Mat4x2:__sub(arg1) end

--- 
--- @param arg2 int
--- @param arg3 int
function math.Mat4x2:Get(arg2, arg3) end

--- 
--- @param 1 math.Mat4x2
function math.Mat4x2:__eq(arg1) end

--- 
--- @param 1 math.Mat4x2
function math.Mat4x2:__add(arg1) end

--- 
function math.Mat4x2:Copy() end

--- 
function math.Mat4x2:GetTranspose() end

--- 
function math.Mat4x2:Transpose() end

--- 
--- @param mat2 math.Mat4x2
--- @overload fun(x1: number, y1: number, x2: number, y2: number, x3: number, y3: number, x4: number, y4: number): 
--- @overload fun(arg2: int, arg3: int, arg4: number): 
function math.Mat4x2:Set(mat2) end

--- 
--- @param 1 math.Vector4
--- @overload fun(arg1: math.Mat4x2): 
--- @overload fun(arg1: number): 
function math.Mat4x2:__mul(arg1) end

--- 
--- @param 1 math.Mat4x2
--- @overload fun(arg1: number): 
function math.Mat4x2:__div(arg1) end


--- 
--- @class math.NoiseMap
math.NoiseMap = {}

--- 
--- @param x int
--- @param y int
function math.NoiseMap:GetValue(x, y) end

--- 
function math.NoiseMap:GetWidth() end

--- 
function math.NoiseMap:GetHeight() end


--- 
--- @class math.Mat4
--- @overload fun(args: string):math.Mat4
--- @overload fun(arg1: math.Mat4x3):math.Mat4
--- @overload fun(arg1: math.Mat4x2):math.Mat4
--- @overload fun(arg1: math.Mat4):math.Mat4
--- @overload fun(arg1: math.Mat3x4):math.Mat4
--- @overload fun(arg1: math.Mat3x2):math.Mat4
--- @overload fun(arg1: math.Mat3):math.Mat4
--- @overload fun(arg1: math.Mat2x4):math.Mat4
--- @overload fun(arg1: math.Mat2x3):math.Mat4
--- @overload fun(arg1: math.Mat2):math.Mat4
--- @overload fun(arg1: number):math.Mat4
--- @overload fun():math.Mat4
--- @overload fun(arg1: number, arg2: number, arg3: number, arg4: number, arg5: number, arg6: number, arg7: number, arg8: number, arg9: number, arg10: number, arg11: number, arg12: number, arg13: number, arg14: number, arg15: number, arg16: number):math.Mat4
math.Mat4 = {}

--- 
--- @param pos math.Vector
--- @overload fun(pos: vector.Vector): 
function math.Mat4:Translate(pos) end

--- 
--- @param colIndex int
--- @param factor number
function math.Mat4:MulCol(colIndex, factor) end

--- 
--- @param colSrc int
--- @param colDst int
function math.Mat4:SwapColumns(colSrc, colDst) end

--- 
--- @param axis math.Vector
--- @param ang number
--- @overload fun(ang: math.EulerAngles): 
--- @overload fun(axis: vector.Vector, ang: number): 
function math.Mat4:Rotate(axis, ang) end

--- 
function math.Mat4:Decompose() end

--- 
--- @param rowIndex int
--- @param factor number
function math.Mat4:MulRow(rowIndex, factor) end

--- 
--- @param arg2 int
--- @param arg3 int
function math.Mat4:Get(arg2, arg3) end

--- 
--- @param rowSrc int
--- @param rowDst int
function math.Mat4:SwapRows(rowSrc, rowDst) end

--- 
function math.Mat4:Inverse() end

--- 
--- @param nearZ number
--- @param farZ number
--- @param d number
--- @param delta number
function math.Mat4:ApplyProjectionDepthBiasOffset(nearZ, farZ, d, delta) end

--- 
--- @param mat2 math.Mat4
--- @overload fun(x1: number, y1: number, z1: number, w1: number, x2: number, y2: number, z2: number, w2: number, x3: number, y3: number, z3: number, w3: number, x4: number, y4: number, z4: number, w4: number): 
--- @overload fun(arg2: int, arg3: int, arg4: number): 
function math.Mat4:Set(mat2) end

--- 
--- @param 1 math.Vector4
--- @overload fun(arg1: math.Mat4): 
--- @overload fun(arg1: math.Mat4): 
--- @overload fun(arg1: number): 
function math.Mat4:__mul(arg1) end

--- 
--- @param 1 math.Mat4
--- @overload fun(arg1: number): 
function math.Mat4:__div(arg1) end

--- 
function math.Mat4:ToQuaternion() end

--- 
function math.Mat4:__tostring() end

--- 
function math.Mat4:ToEulerAngles() end

--- 
--- @param 1 math.Mat4
function math.Mat4:__eq(arg1) end

--- 
function math.Mat4:GetInverse() end

--- 
--- @param 1 math.Mat4
function math.Mat4:__sub(arg1) end

--- 
function math.Mat4:Copy() end

--- 
--- @param 1 math.Mat4
function math.Mat4:__add(arg1) end

--- 
function math.Mat4:Transpose() end

--- 
--- @param scale math.Vector
--- @overload fun(scale: vector.Vector): 
function math.Mat4:Scale(scale) end

--- 
function math.Mat4:GetTranspose() end


--- 
--- @class math.Mat2
--- @overload fun(args: string):math.Mat2
--- @overload fun(arg1: math.Mat4x3):math.Mat2
--- @overload fun(arg1: math.Mat4x2):math.Mat2
--- @overload fun(arg1: math.Mat4):math.Mat2
--- @overload fun(arg1: math.Mat3x4):math.Mat2
--- @overload fun(arg1: math.Mat3x2):math.Mat2
--- @overload fun(arg1: math.Mat3):math.Mat2
--- @overload fun(arg1: math.Mat2x4):math.Mat2
--- @overload fun(arg1: math.Mat2x3):math.Mat2
--- @overload fun(arg1: math.Mat2):math.Mat2
--- @overload fun(arg1: number):math.Mat2
--- @overload fun():math.Mat2
--- @overload fun(arg1: number, arg2: number, arg3: number, arg4: number):math.Mat2
math.Mat2 = {}

--- 
function math.Mat2:Inverse() end

--- 
function math.Mat2:__tostring() end

--- 
--- @param 1 math.Mat2
function math.Mat2:__sub(arg1) end

--- 
function math.Mat2:GetInverse() end

--- 
--- @param 1 math.Mat2
function math.Mat2:__eq(arg1) end

--- 
--- @param 1 math.Mat2
function math.Mat2:__add(arg1) end

--- 
function math.Mat2:Copy() end

--- 
function math.Mat2:GetTranspose() end

--- 
--- @param arg2 int
--- @param arg3 int
function math.Mat2:Get(arg2, arg3) end

--- 
function math.Mat2:Transpose() end

--- 
--- @param mat2 math.Mat2
--- @overload fun(x1: number, y1: number, x2: number, y2: number): 
--- @overload fun(arg2: int, arg3: int, arg4: number): 
function math.Mat2:Set(mat2) end

--- 
--- @param 1 math.Vector2
--- @overload fun(arg1: math.Mat2): 
--- @overload fun(arg1: math.Mat2): 
--- @overload fun(arg1: number): 
function math.Mat2:__mul(arg1) end

--- 
--- @param 1 math.Mat2
--- @overload fun(arg1: number): 
function math.Mat2:__div(arg1) end


--- 
--- @class math.Plane
--- @overload fun(arg1: math.Vector, arg2: number):math.Plane
--- @overload fun(arg1: math.Vector, arg2: math.Vector):math.Plane
--- @overload fun(arg1: math.Vector, arg2: math.Vector, arg3: math.Vector):math.Plane
--- @overload fun(arg1: vector.Vector, arg2: number):math.Plane
--- @overload fun(arg1: vector.Vector, arg2: vector.Vector):math.Plane
--- @overload fun(arg1: vector.Vector, arg2: vector.Vector, arg3: vector.Vector):math.Plane
math.Plane = {}

--- 
--- @param arg1 number
function math.Plane:SetDistance(arg1) end

--- 
--- @param arg1 math.Vector
--- @overload fun(arg1: vector.Vector): 
function math.Plane:SetNormal(arg1) end

--- 
function math.Plane:Copy() end

--- 
--- @return string ret0
function math.Plane:__tostring() end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function math.Plane:GetPos() end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function math.Plane:GetCenterPos() end

--- 
--- @param arg1 math.Vector
--- @return number ret0
--- @overload fun(): number
--- @overload fun(arg1: vector.Vector): number
function math.Plane:GetDistance(arg1) end

--- 
--- @param transform math.Mat4
function math.Plane:Transform(transform) end

--- 
--- @param pos math.Vector
--- @overload fun(pos: vector.Vector): 
function math.Plane:MoveToPos(pos) end

--- 
--- @param arg1 math.EulerAngles
function math.Plane:Rotate(arg1) end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function math.Plane:GetNormal() end


--- 
--- @class math.ScaledTransform: math.Transform
math.ScaledTransform = {}

--- 
--- @param arg1 math.Vector
--- @overload fun(arg1: vector.Vector): 
function math.ScaledTransform:Scale(arg1) end

--- 
--- @param arg1 math.Vector
--- @overload fun(arg1: vector.Vector): 
function math.ScaledTransform:SetScale(arg1) end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function math.ScaledTransform:GetScale() end


--- 
--- @class math.EulerAngles
--- @field y number 
--- @field p number 
--- @field r number 
--- @overload fun(arg1: string):math.EulerAngles
--- @overload fun(arg1: math.Quaternion):math.EulerAngles
--- @overload fun(arg1: math.Vector, arg2: math.Vector):math.EulerAngles
--- @overload fun(arg1: math.Vector):math.EulerAngles
--- @overload fun(arg1: math.Mat4):math.EulerAngles
--- @overload fun(arg1: math.EulerAngles):math.EulerAngles
--- @overload fun(arg1: number, arg2: number, arg3: number):math.EulerAngles
--- @overload fun():math.EulerAngles
--- @overload fun(arg1: vector.Vector, arg2: vector.Vector):math.EulerAngles
--- @overload fun(arg1: vector.Vector):math.EulerAngles
math.EulerAngles = {}

--- 
--- @param b math.EulerAngles
--- @return bool ret0
--- @overload fun(b: math.EulerAngles, epsilon: number): bool
function math.EulerAngles:Equals(b) end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function math.EulerAngles:GetForward() end

--- 
--- @param 1 math.EulerAngles
function math.EulerAngles:__eq(arg1) end

--- 
--- @param 1 math.EulerAngles
function math.EulerAngles:__add(arg1) end

--- 
--- @param 1 math.EulerAngles
function math.EulerAngles:__sub(arg1) end

--- 
--- @return math.Vector ret0_1
--- @return math.Vector ret0_2
--- @return math.Vector ret0_3
--- @overload fun(): math.Vector, math.Vector, math.Vector
function math.EulerAngles:GetOrientation() end

--- 
function math.EulerAngles:__unm() end

--- 
--- @return math.Mat4 ret0
function math.EulerAngles:ToMatrix() end

--- 
--- @param arg1 number
--- @overload fun(): 
function math.EulerAngles:Normalize(arg1) end

--- 
--- @param idx int
--- @return number ret0
function math.EulerAngles:Get(idx) end

--- 
--- @param idx int
--- @param value number
--- @overload fun(p: number, y: number, r: number): 
--- @overload fun(arg1: math.EulerAngles): 
function math.EulerAngles:Set(idx, value) end

--- 
--- @return math.EulerAngles ret0
function math.EulerAngles:Copy() end

--- 
--- @overload fun(rotationOrder: int): 
function math.EulerAngles:ToQuaternion() end

--- 
--- @param 1 number
function math.EulerAngles:__div(arg1) end

--- 
--- @param 1 math.EulerAngles
--- @overload fun(arg1: number): 
function math.EulerAngles:__mul(arg1) end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function math.EulerAngles:GetRight() end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function math.EulerAngles:GetUp() end

--- 
function math.EulerAngles:__tostring() end


--- 
--- @class pfm.math
pfm.math = {}



Vector2i = math.Vector2i
Vector = math.Vector
Vector2 = math.Vector2
Vector4 = math.Vector4
EulerAngles = math.EulerAngles
Quaternion = math.Quaternion
