--- @meta
--- 
--- @class ik
ik = {}


--- 
--- @class ik.CCDIkSolver: ik.IkSolver
--- @overload fun():ik.CCDIkSolver
ik.CCDIkSolver = {}


--- 
--- @class ik.DistanceJoint: ik.Joint
ik.DistanceJoint = {}


--- 
--- @class ik.Control
ik.Control = {}


--- 
--- @class ik.IkSolver
ik.IkSolver = {}

--- 
--- @param arg1 int
--- @return math.ScaledTransform ret0
function ik.IkSolver:GetGlobalTransform(arg1) end

--- 
--- @param arg1 math.ScaledTransform
--- @return bool ret0
function ik.IkSolver:Solve(arg1) end

--- 
--- @param idx int
--- @param axis math.Vector
--- @return ik.IkHingeConstraint ret0
--- @overload fun(idx: int, axis: vector.Vector): ik.IkHingeConstraint
function ik.IkSolver:AddHingeConstraint(idx, axis) end

--- 
--- @param arg1 int
--- @param arg2 math.ScaledTransform
function ik.IkSolver:SetLocalTransform(arg1, arg2) end

--- 
--- @param arg1 int
--- @return math.ScaledTransform ret0
function ik.IkSolver:GetLocalTransform(arg1) end

--- 
--- @return int ret0
function ik.IkSolver:Size() end

--- 
--- @param arg2 int
--- @param arg3 unknown
--- @return ik.IkConstraint ret0
function ik.IkSolver:AddCustomConstraint(arg2, arg3) end

--- 
--- @param arg1 int
function ik.IkSolver:Resize(arg1) end

--- 
--- @param idx int
--- @param limit number
--- @return ik.IkBallSocketConstraint ret0
function ik.IkSolver:AddBallSocketConstraint(idx, limit) end


--- 
--- @class ik.TwistLimit: ik.Joint
ik.TwistLimit = {}

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function ik.TwistLimit:GetMeasurementAxisA() end

--- 
--- @param arg1 math.Vector
--- @overload fun(arg1: vector.Vector): 
function ik.TwistLimit:SetMeasurementAxisB(arg1) end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function ik.TwistLimit:GetMeasurementAxisB() end

--- 
--- @param arg1 math.Vector
--- @overload fun(arg1: vector.Vector): 
function ik.TwistLimit:SetMeasurementAxisA(arg1) end


--- 
--- @class ik.Bone
ik.Bone = {}

--- 
--- @param arg1 math.Quaternion
function ik.Bone:SetRot(arg1) end

--- 
--- @param arg1 bool
function ik.Bone:SetPinned(arg1) end

--- 
--- @return bool ret0
function ik.Bone:IsPinned() end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function ik.Bone:GetPos() end

--- 
--- @param arg1 math.Vector
--- @overload fun(arg1: vector.Vector): 
function ik.Bone:SetPos(arg1) end

--- 
--- @return math.Quaternion ret0
function ik.Bone:GetRot() end


--- 
--- @class ik.Joint
ik.Joint = {}

--- 
--- @return number ret0
function ik.Joint:GetRigidity() end

--- 
--- @param arg1 number
function ik.Joint:SetRigidity(arg1) end


--- 
--- @class ik.SwivelHingeJoint: ik.Joint
ik.SwivelHingeJoint = {}


--- 
--- @class ik.LinearAxisLimit: ik.Joint
ik.LinearAxisLimit = {}


--- 
--- @class ik.EllipseSwingLimit: ik.Joint
ik.EllipseSwingLimit = {}


--- 
--- @class ik.DragControl: ik.Control, ik.LinearMotorControl
ik.DragControl = {}


--- 
--- @class ik.StateControl: ik.Control, ik.LinearMotorControl
ik.StateControl = {}

--- 
--- @return math.Quaternion ret0
function ik.StateControl:GetTargetOrientation() end

--- 
--- @param arg1 math.Quaternion
function ik.StateControl:SetTargetOrientation(arg1) end


--- 
--- @class ik.AngularJoint: ik.Joint
ik.AngularJoint = {}


--- 
--- @class ik.LinearMotorControl
ik.LinearMotorControl = {}

--- 
--- @param arg1 math.Vector
--- @overload fun(arg1: vector.Vector): 
function ik.LinearMotorControl:SetTargetPosition(arg1) end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function ik.LinearMotorControl:GetTargetPosition() end


--- 
--- @class ik.IkBallSocketConstraint: ik.IkConstraint
ik.IkBallSocketConstraint = {}

--- 
--- @param arg1 number
function ik.IkBallSocketConstraint:SetLimit(arg1) end

--- 
--- @return number ret0
function ik.IkBallSocketConstraint:GetLimit() end


--- 
--- @class ik.Solver
--- @overload fun():ik.Solver
--- @overload fun(args: int):ik.Solver
--- @overload fun(arg1: int, arg2: int):ik.Solver
ik.Solver = {}

--- 
--- @return int ret0
function ik.Solver:GetBoneCount() end

--- 
--- @param arg1 ik.Bone
--- @param arg2 ik.Bone
--- @param arg3 math.Vector
--- @return ik.BallSocketJoint ret0
--- @overload fun(arg1: ik.Bone, arg2: ik.Bone, arg3: vector.Vector): ik.BallSocketJoint
function ik.Solver:AddBallSocketJoint(arg1, arg2, arg3) end

--- 
--- @param arg1 ik.Control
function ik.Solver:RemoveControl(arg1) end

--- 
--- @param arg1 ik.Bone
--- @param arg2 ik.Bone
--- @return ik.AngularJoint ret0
function ik.Solver:AddAngularJoint(arg1, arg2) end

--- 
--- @param arg1 ik.Bone
--- @param arg2 ik.Bone
--- @param arg3 math.Vector
--- @return ik.RevoluteJoint ret0
--- @overload fun(arg1: ik.Bone, arg2: ik.Bone, arg3: vector.Vector): ik.RevoluteJoint
function ik.Solver:AddRevoluteJoint(arg1, arg2, arg3) end

--- 
--- @param arg1 ik.Bone
--- @return ik.AngularPlaneControl ret0
function ik.Solver:AddAngularPlaneControl(arg1) end

--- 
--- @param arg1 ik.Bone
--- @param arg2 ik.Bone
--- @return ik.DistanceJoint ret0
function ik.Solver:AddDistanceJoint(arg1, arg2) end

--- 
--- @param arg1 ik.Bone
--- @param arg2 ik.Bone
--- @param arg3 math.Vector
--- @param arg4 math.Vector
--- @param arg5 math.Vector
--- @return ik.PointOnLineJoint ret0
--- @overload fun(arg1: ik.Bone, arg2: ik.Bone, arg3: vector.Vector, arg4: vector.Vector, arg5: vector.Vector): ik.PointOnLineJoint
function ik.Solver:AddPointOnLineJoint(arg1, arg2, arg3, arg4, arg5) end

--- 
--- @param arg1 ik.Bone
--- @param arg2 ik.Bone
--- @param arg3 math.Vector
--- @param arg4 math.Vector
--- @param arg5 number
--- @return ik.SwingLimit ret0
--- @overload fun(arg1: ik.Bone, arg2: ik.Bone, arg3: vector.Vector, arg4: vector.Vector, arg5: number): ik.SwingLimit
function ik.Solver:AddSwingLimit(arg1, arg2, arg3, arg4, arg5) end

--- 
--- @param arg1 ik.Bone
--- @param arg2 ik.Bone
--- @param arg3 math.Vector
--- @param arg4 math.Vector
--- @param arg5 math.Vector
--- @param arg6 math.Vector
--- @param arg7 number
--- @param arg8 number
--- @return ik.EllipseSwingLimit ret0
--- @overload fun(arg1: ik.Bone, arg2: ik.Bone, arg3: vector.Vector, arg4: vector.Vector, arg5: vector.Vector, arg6: vector.Vector, arg7: number, arg8: number): ik.EllipseSwingLimit
function ik.Solver:AddEllipseSwingLimit(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) end

--- 
--- @return int ret0
function ik.Solver:GetControlCount() end

--- 
--- @param arg1 ik.Bone
--- @param arg2 ik.Bone
--- @param arg3 math.Vector
--- @param arg4 math.Vector
--- @param arg5 math.Vector
--- @param arg6 number
--- @param arg7 number
--- @return ik.LinearAxisLimit ret0
--- @overload fun(arg1: ik.Bone, arg2: ik.Bone, arg3: vector.Vector, arg4: vector.Vector, arg5: vector.Vector, arg6: number, arg7: number): ik.LinearAxisLimit
function ik.Solver:AddLinearAxisLimit(arg1, arg2, arg3, arg4, arg5, arg6, arg7) end

--- 
--- @param arg1 ik.Bone
--- @param arg2 ik.Bone
--- @param arg3 math.Vector
--- @param arg4 math.Vector
--- @return ik.TwistJoint ret0
--- @overload fun(arg1: ik.Bone, arg2: ik.Bone, arg3: vector.Vector, arg4: vector.Vector): ik.TwistJoint
function ik.Solver:AddTwistJoint(arg1, arg2, arg3, arg4) end

--- 
--- @param arg1 ik.Bone
--- @param arg2 ik.Bone
--- @param arg3 math.Vector
--- @param arg4 math.Vector
--- @param arg5 number
--- @return ik.TwistLimit ret0
--- @overload fun(arg1: ik.Bone, arg2: ik.Bone, arg3: vector.Vector, arg4: vector.Vector, arg5: number): ik.TwistLimit
function ik.Solver:AddTwistLimit(arg1, arg2, arg3, arg4, arg5) end

--- 
--- @param arg1 ik.Bone
--- @param arg2 ik.Bone
--- @param arg3 math.Vector
--- @param arg4 math.Vector
--- @return ik.SwivelHingeJoint ret0
--- @overload fun(arg1: ik.Bone, arg2: ik.Bone, arg3: vector.Vector, arg4: vector.Vector): ik.SwivelHingeJoint
function ik.Solver:AddSwivelHingeJoint(arg1, arg2, arg3, arg4) end

--- 
--- @param arg1 int
--- @return ik.Control ret0
function ik.Solver:GetControl(arg1) end

--- 
--- @param arg1 ik.Bone
--- @return ik.DragControl ret0
function ik.Solver:AddDragControl(arg1) end

--- 
--- @param arg1 ik.Bone
--- @return ik.StateControl ret0
function ik.Solver:AddStateControl(arg1) end

--- 
--- @param arg1 int
--- @return ik.Joint ret0
function ik.Solver:GetJoint(arg1) end

--- 
function ik.Solver:Solve() end

--- 
--- @param arg1 int
--- @return ik.Bone ret0
function ik.Solver:GetBone(arg1) end

--- 
--- @return int ret0
function ik.Solver:GetJointCount() end

--- 
--- @param arg1 math.Vector
--- @param arg2 math.Quaternion
--- @param arg3 number
--- @param arg4 number
--- @param arg5 int
--- @return ik.Bone ret0
--- @overload fun(arg1: vector.Vector, arg2: math.Quaternion, arg3: number, arg4: number, arg5: int): ik.Bone
function ik.Solver:AddBone(arg1, arg2, arg3, arg4, arg5) end


--- 
--- @class ik.PointOnLineJoint: ik.Joint
ik.PointOnLineJoint = {}


--- 
--- @class ik.BallSocketJoint: ik.Joint
ik.BallSocketJoint = {}


--- 
--- @class ik.IkHingeConstraint: ik.IkConstraint
ik.IkHingeConstraint = {}

--- 
--- @param arg1 math.Vector2
function ik.IkHingeConstraint:SetLimits(arg1) end

--- 
function ik.IkHingeConstraint:ClearLimits() end

--- 
--- @return math.Vector2 ret0
function ik.IkHingeConstraint:GetLimits() end


--- 
--- @class ik.RevoluteJoint: ik.Joint
ik.RevoluteJoint = {}


--- 
--- @class ik.IkConstraint
ik.IkConstraint = {}

--- 
--- @return int ret0
function ik.IkConstraint:GetJointIndex() end


--- 
--- @class ik.SwingLimit: ik.Joint
ik.SwingLimit = {}


--- 
--- @class ik.TwistJoint: ik.Joint
ik.TwistJoint = {}


--- 
--- @class ik.FABRIkSolver: ik.IkSolver
--- @overload fun():ik.FABRIkSolver
ik.FABRIkSolver = {}


--- 
--- @class ik.AngularPlaneControl: ik.Control
ik.AngularPlaneControl = {}

--- 
--- @param arg1 math.Vector
--- @overload fun(arg1: vector.Vector): 
function ik.AngularPlaneControl:SetPlaneNormal(arg1) end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function ik.AngularPlaneControl:GetBoneLocalAxis() end

--- 
--- @param arg1 math.Vector
--- @overload fun(arg1: vector.Vector): 
function ik.AngularPlaneControl:SetBoneLocalAxis(arg1) end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function ik.AngularPlaneControl:GetPlaneNormal() end


