--- @meta
--- 
--- @class phys
phys = {}

--- 
--- @param game class Game
--- @param traceData phys.RayCastData
--- @return variant ret0
function phys.sweep(game, traceData) end

--- 
--- @param env class pragma::physics::IEnvironment
--- @param verts table
--- @param material phys.Material
--- @return phys.ConvexShape ret0
--- @overload fun(env: class pragma::physics::IEnvironment, verts: table, tris: table, material: phys.Material): phys.ConvexShape
--- @overload fun(env: class pragma::physics::IEnvironment, verts: table, material: phys.Material): phys.ConvexShape
--- @overload fun(env: class pragma::physics::IEnvironment, verts: table, tris: table, material: phys.Material): phys.ConvexShape
function phys.create_convex_shape(env, verts, material) end

--- 
--- @param env class pragma::physics::IEnvironment
--- @param material phys.Material
--- @return phys.ConvexHullShape ret0
function phys.create_convex_hull_shape(env, material) end

--- 
--- @param env class pragma::physics::IEnvironment
--- @param halfWidth number
--- @param halfHeight number
--- @param material phys.Material
--- @return phys.ConvexShape ret0
function phys.create_capsule_shape(env, halfWidth, halfHeight, material) end

--- 
--- @param env class pragma::physics::IEnvironment
--- @param verts table
--- @param tris table
--- @param material phys.Material
--- @return phys.TriangleShape ret0
--- @overload fun(env: class pragma::physics::IEnvironment, verts: table, tris: table, material: phys.Material): phys.TriangleShape
function phys.create_triangle_shape(env, verts, tris, material) end

--- 
--- @param env class pragma::physics::IEnvironment
--- @param halfExtents math.Vector
--- @param material phys.Material
--- @return phys.ConvexShape ret0
--- @overload fun(env: class pragma::physics::IEnvironment, halfExtents: vector.Vector, material: phys.Material): phys.ConvexShape
function phys.create_box_shape(env, halfExtents, material) end

--- 
--- @param env class pragma::physics::IEnvironment
--- @param radius number
--- @param material phys.Material
--- @return phys.ConvexShape ret0
function phys.create_sphere_shape(env, radius, material) end

--- 
--- @param env class pragma::physics::IEnvironment
--- @param radius number
--- @param height number
--- @param material phys.Material
--- @return phys.ConvexShape ret0
function phys.create_cylinder_shape(env, radius, height, material) end

--- 
--- @param env class pragma::physics::IEnvironment
--- @param shapes table
--- @return phys.CompoundShape ret0
function phys.create_compound_shape(env, shapes) end

--- 
--- @param env class pragma::physics::IEnvironment
--- @param width number
--- @param length number
--- @param maxHeight number
--- @param upAxis number
--- @param material phys.Material
--- @return phys.Shape ret0
function phys.create_heightfield_terrain_shape(env, width, length, maxHeight, upAxis, material) end

--- 
--- @param env class pragma::physics::IEnvironment
--- @param shape phys.Shape
--- @param dynamic bool
--- @return phys.RigidBody ret0
--- @overload fun(env: class pragma::physics::IEnvironment, shape: phys.Shape, dynamic: bool): phys.RigidBody
function phys.create_rigid_body(env, shape, dynamic) end

--- 
--- @param env class pragma::physics::IEnvironment
--- @param bodyA phys.RigidBody
--- @param pivotA math.Vector
--- @param rotA math.Quaternion
--- @param bodyB phys.RigidBody
--- @param pivotB math.Vector
--- @param rotB math.Quaternion
--- @return phys.FixedConstraint ret0
--- @overload fun(env: class pragma::physics::IEnvironment, bodyA: phys.RigidBody, pivotA: vector.Vector, rotA: math.Quaternion, bodyB: phys.RigidBody, pivotB: vector.Vector, rotB: math.Quaternion): phys.FixedConstraint
function phys.create_fixed_constraint(env, bodyA, pivotA, rotA, bodyB, pivotB, rotB) end

--- 
--- @param env class pragma::physics::IEnvironment
--- @param shape phys.Shape
--- @return phys.GhostObj ret0
function phys.create_ghost_object(env, shape) end

--- 
--- @param env class pragma::physics::IEnvironment
--- @param n math.Vector
--- @param d number
--- @param material phys.Material
--- @return phys.CollisionObj ret0
--- @overload fun(env: class pragma::physics::IEnvironment, n: vector.Vector, d: number, material: phys.Material): phys.CollisionObj
function phys.create_plane(env, n, d, material) end

--- 
--- @param env class pragma::physics::IEnvironment
--- @param bodyA phys.RigidBody
--- @param pivotA math.Vector
--- @param bodyB phys.RigidBody
--- @param pivotB math.Vector
--- @return phys.BallSocketConstraint ret0
--- @overload fun(env: class pragma::physics::IEnvironment, bodyA: phys.RigidBody, pivotA: vector.Vector, bodyB: phys.RigidBody, pivotB: vector.Vector): phys.BallSocketConstraint
function phys.create_ballsocket_constraint(env, bodyA, pivotA, bodyB, pivotB) end

--- 
--- @param game class Game
--- @param traceData phys.RayCastData
--- @return variant ret0
function phys.raycast(game, traceData) end

--- 
--- @param env class pragma::physics::IEnvironment
--- @param bodyA phys.RigidBody
--- @param pivotA math.Vector
--- @param bodyB phys.RigidBody
--- @param pivotB math.Vector
--- @param axis math.Vector
--- @return phys.HingeConstraint ret0
--- @overload fun(env: class pragma::physics::IEnvironment, bodyA: phys.RigidBody, pivotA: vector.Vector, bodyB: phys.RigidBody, pivotB: vector.Vector, axis: vector.Vector): phys.HingeConstraint
function phys.create_hinge_constraint(env, bodyA, pivotA, bodyB, pivotB, axis) end

--- 
--- @param angVel math.Vector
--- @param invInertiaTensor math.Mat3
--- @param dt number
--- @return math.Vector ret0
--- @overload fun(angVel: vector.Vector, invInertiaTensor: math.Mat3, dt: number): math.Vector
function phys.calc_torque_from_angular_velocity(angVel, invInertiaTensor, dt) end

--- 
--- @param env class pragma::physics::IEnvironment
--- @param bodyA phys.RigidBody
--- @param pivotA math.Vector
--- @param rotA math.Quaternion
--- @param bodyB phys.RigidBody
--- @param pivotB math.Vector
--- @param rotB math.Quaternion
--- @return phys.SliderConstraint ret0
--- @overload fun(env: class pragma::physics::IEnvironment, bodyA: phys.RigidBody, pivotA: vector.Vector, rotA: math.Quaternion, bodyB: phys.RigidBody, pivotB: vector.Vector, rotB: math.Quaternion): phys.SliderConstraint
function phys.create_slider_constraint(env, bodyA, pivotA, rotA, bodyB, pivotB, rotB) end

--- 
--- @param force math.Vector
--- @param mass number
--- @param dt number
--- @return math.Vector ret0
--- @overload fun(force: vector.Vector, mass: number, dt: number): math.Vector
function phys.calc_linear_velocity_from_force(force, mass, dt) end

--- 
--- @param env class pragma::physics::IEnvironment
--- @param bodyA phys.RigidBody
--- @param pivotA math.Vector
--- @param rotA math.Quaternion
--- @param bodyB phys.RigidBody
--- @param pivotB math.Vector
--- @param rotB math.Quaternion
--- @return phys.ConeTwistConstraint ret0
--- @overload fun(env: class pragma::physics::IEnvironment, bodyA: phys.RigidBody, pivotA: vector.Vector, rotA: math.Quaternion, bodyB: phys.RigidBody, pivotB: vector.Vector, rotB: math.Quaternion): phys.ConeTwistConstraint
function phys.create_conetwist_constraint(env, bodyA, pivotA, rotA, bodyB, pivotB, rotB) end

--- 
--- @param env class pragma::physics::IEnvironment
--- @param bodyA phys.RigidBody
--- @param pivotA math.Vector
--- @param rotA math.Quaternion
--- @param bodyB phys.RigidBody
--- @param pivotB math.Vector
--- @param rotB math.Quaternion
--- @return phys.DoFConstraint ret0
--- @overload fun(env: class pragma::physics::IEnvironment, bodyA: phys.RigidBody, pivotA: vector.Vector, rotA: math.Quaternion, bodyB: phys.RigidBody, pivotB: vector.Vector, rotB: math.Quaternion): phys.DoFConstraint
function phys.create_dof_constraint(env, bodyA, pivotA, rotA, bodyB, pivotB, rotB) end

--- 
--- @param env class pragma::physics::IEnvironment
--- @param bodyA phys.RigidBody
--- @param pivotA math.Vector
--- @param rotA math.Quaternion
--- @param bodyB phys.RigidBody
--- @param pivotB math.Vector
--- @param rotB math.Quaternion
--- @return phys.DoFSpringConstraint ret0
--- @overload fun(env: class pragma::physics::IEnvironment, bodyA: phys.RigidBody, pivotA: vector.Vector, rotA: math.Quaternion, bodyB: phys.RigidBody, pivotB: vector.Vector, rotB: math.Quaternion): phys.DoFSpringConstraint
function phys.create_dof_spring_constraint(env, bodyA, pivotA, rotA, bodyB, pivotB, rotB) end

--- 
--- @param game class Game
--- @param name string
--- @param friction number
--- @param restitution number
--- @return game.SurfaceMaterial ret0
function phys.create_surface_material(game, name, friction, restitution) end

--- 
--- @param env class pragma::physics::IEnvironment
--- @param staticFriction number
--- @param dynamicFriction number
--- @param restitution number
--- @return phys.Material ret0
function phys.create_material(env, staticFriction, dynamicFriction, restitution) end

--- 
--- @param env class pragma::physics::IEnvironment
--- @param halfExtents math.Vector
--- @param stepHeight number
--- @param slopeLimit number
--- @param startTransform math.Transform
--- @return phys.Controller ret0
--- @overload fun(env: class pragma::physics::IEnvironment, halfExtents: vector.Vector, stepHeight: number, slopeLimit: number, startTransform: math.Transform): phys.Controller
function phys.create_box_controller(env, halfExtents, stepHeight, slopeLimit, startTransform) end

--- 
--- @param env class pragma::physics::IEnvironment
--- @param halfWidth number
--- @param halfHeight number
--- @param stepHeight number
--- @param slopeLimit number
--- @param startTransform math.Transform
--- @return phys.Controller ret0
function phys.create_capsule_controller(env, halfWidth, halfHeight, stepHeight, slopeLimit, startTransform) end

--- 
--- @param env class pragma::physics::IEnvironment
--- @param vhcCreateInfo phys.VehicleCreateInfo
--- @return phys.Vehicle ret0
function phys.create_vehicle(env, vhcCreateInfo) end

--- 
--- @param arg0 class Game
--- @param arg1 int
--- @return game.SurfaceMaterial ret0
--- @overload fun(arg0: class Game, arg1: string): game.SurfaceMaterial
function phys.get_surface_material(arg0, arg1) end

--- 
--- @param game class Game
--- @return table ret0
function phys.get_surface_materials(game) end

--- 
--- @param env class pragma::physics::IEnvironment
--- @return phys.Material ret0
function phys.get_generic_material(env) end

--- 
--- @param linVel math.Vector
--- @param mass number
--- @param dt number
--- @return math.Vector ret0
--- @overload fun(linVel: vector.Vector, mass: number, dt: number): math.Vector
function phys.calc_force_from_linear_velocity(linVel, mass, dt) end

--- 
--- @param game class Game
--- @param traceData phys.RayCastData
--- @return variant ret0
function phys.overlap(game, traceData) end

--- 
--- @param torque math.Vector
--- @param invInertiaTensor math.Mat3
--- @param dt number
--- @return math.Vector ret0
--- @overload fun(torque: vector.Vector, invInertiaTensor: math.Mat3, dt: number): math.Vector
function phys.calc_angular_velocity_from_torque(torque, invInertiaTensor, dt) end


--- 
--- @class phys.ContactInfo
--- @field GetContactPointCount int 
--- @field collisionObj0  
--- @field flags int 
--- @field GetContactPoint  
--- @field shape1  
--- @field GetContactPoints table 
--- @field shape0  
--- @field collisionObj1  
phys.ContactInfo = {}


--- 
--- @class phys.BoxShape: phys.ConvexShape, phys.Shape, phys.Base
phys.BoxShape = {}

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function phys.BoxShape:GetHalfExtents() end


--- 
--- @class phys.Vehicle: phys.Base
phys.Vehicle = {}

--- 
--- @return int ret0
function phys.Vehicle:GetWheelCount() end

--- 
function phys.Vehicle:Remove() end

--- 
--- @return phys.CollisionObj ret0
function phys.Vehicle:GetCollisionObject() end

--- 
--- @param arg1 number
function phys.Vehicle:SetHandBrakeFactor(arg1) end

--- 
--- @param arg1 enum pragma::physics::IVehicle::Gear
function phys.Vehicle:ChangeToGear(arg1) end

--- 
--- @return number ret0
function phys.Vehicle:GetForwardSpeed() end

--- 
--- @param arg1 bool
function phys.Vehicle:SetUseDigitalInputs(arg1) end

--- 
--- @param arg1 number
function phys.Vehicle:SetBrakeFactor(arg1) end

--- 
--- @param arg1 number
function phys.Vehicle:SetAccelerationFactor(arg1) end

--- 
--- @param arg1 number
function phys.Vehicle:SetSteerFactor(arg1) end

--- 
--- @return enum pragma::physics::IVehicle::Gear ret0
function phys.Vehicle:GetCurrentGear() end

--- 
--- @param arg1 enum pragma::physics::IVehicle::Gear
function phys.Vehicle:SetGear(arg1) end

--- 
function phys.Vehicle:SetGearDown() end

--- 
function phys.Vehicle:SetGearUp() end

--- 
--- @param arg1 number
function phys.Vehicle:SetGearSwitchTime(arg1) end

--- 
--- @param arg1 bool
function phys.Vehicle:SetUseAutoGears(arg1) end

--- 
--- @return bool ret0
function phys.Vehicle:ShouldUseAutoGears() end

--- 
--- @return number ret0
function phys.Vehicle:GetEngineRotationSpeed() end

--- 
function phys.Vehicle:SetRestState() end

--- 
function phys.Vehicle:ResetControls() end

--- 
--- @param arg1 int
--- @param arg2 number
function phys.Vehicle:SetWheelRotationAngle(arg1, arg2) end

--- 
--- @param arg1 int
--- @param arg2 number
function phys.Vehicle:SetWheelRotationSpeed(arg1, arg2) end

--- 
--- @return bool ret0
function phys.Vehicle:IsInAir() end

--- 
--- @return bool ret0
function phys.Vehicle:IsValid() end

--- 
--- @return number ret0
function phys.Vehicle:GetSidewaysSpeed() end


--- @enum Gear
phys.Vehicle = {
	GEAR_TWENTYSIXTH = 27,
	GEAR_THIRTIETH = 31,
	GEAR_ELEVENTH = 12,
	GEAR_EIGHTH = 9,
	GEAR_TWENTYSECOND = 23,
	GEAR_EIGHTEENTH = 19,
	GEAR_NEUTRAL = 1,
	GEAR_COUNT = 32,
	GEAR_FIFTEENTH = 16,
	GEAR_FIFTH = 6,
	GEAR_FIRST = 2,
	GEAR_FOURTEENTH = 15,
	GEAR_FOURTH = 5,
	GEAR_NINETEENTH = 20,
	GEAR_NINTH = 10,
	GEAR_TWENTYSEVENTH = 28,
	GEAR_REVERSE = 0,
	GEAR_TWENTYNINTH = 30,
	GEAR_SECOND = 3,
	GEAR_SEVENTEENTH = 18,
	GEAR_SEVENTH = 8,
	GEAR_SIXTEENTH = 17,
	GEAR_SIXTH = 7,
	GEAR_TENTH = 11,
	GEAR_THIRD = 4,
	GEAR_THIRTEENTH = 14,
	GEAR_TWELFTH = 13,
	GEAR_TWENTIETH = 21,
	GEAR_TWENTYEIGHTH = 29,
	GEAR_TWENTYFIFTH = 26,
	GEAR_TWENTYFIRST = 22,
	GEAR_TWENTYFOURTH = 25,
	GEAR_TWENTYTHIRD = 24,
}

--- 
--- @class phys.IKTree
phys.IKTree = {}

--- 
--- @return int ret0
function phys.IKTree:GetEffectorCount() end

--- 
--- @return int ret0
function phys.IKTree:GetJointCount() end

--- 
--- @param arg1 phys.IKTree.Node
function phys.IKTree:InsertRoot(arg1) end

--- 
function phys.IKTree:Print() end

--- 
--- @param arg1 phys.IKTree.Node
--- @param arg2 phys.IKTree.Node
function phys.IKTree:InsertRightSibling(arg1, arg2) end

--- 
function phys.IKTree:Compute() end

--- 
function phys.IKTree:Init() end

--- 
--- @param arg1 phys.IKTree.Node
--- @param arg2 phys.IKTree.Node
function phys.IKTree:InsertLeftChild(arg1, arg2) end

--- 
--- @param arg1 int
--- @return phys.IKTree.Node ret0
function phys.IKTree:GetJoint(arg1) end

--- 
--- @param arg1 int
--- @return phys.IKTree.Node ret0
function phys.IKTree:GetEffector(arg1) end

--- 
--- @param nodeIdx int
--- @return math.Vector ret0
--- @overload fun(nodeIdx: int): math.Vector
function phys.IKTree:GetEffectorPosition(nodeIdx) end

--- 
--- @return phys.IKTree.Node ret0
function phys.IKTree:GetRoot() end

--- 
--- @param arg1 phys.IKTree.Node
--- @return phys.IKTree.Node ret0
function phys.IKTree:GetSuccessor(arg1) end

--- 
--- @param arg1 phys.IKTree.Node
--- @return phys.IKTree.Node ret0
function phys.IKTree:GetParent(arg1) end

--- 
function phys.IKTree:UnFreeze() end

--- 
--- @return int ret0
function phys.IKTree:GetNodeCount() end


--- 
--- @class phys.IKTree.Node
phys.IKTree.Node = {}

--- 
--- @return number ret0
function phys.IKTree.Node:GetMinTheta() end

--- 
function phys.IKTree.Node:ComputeW() end

--- 
--- @return bool ret0
function phys.IKTree.Node:IsJoint() end

--- 
--- @return number ret0
function phys.IKTree.Node:GetMaxTheta() end

--- 
--- @return bool ret0
function phys.IKTree.Node:IsEffector() end

--- 
--- @return number ret0
function phys.IKTree.Node:GetRestAngle() end

--- 
--- @param arg1 number
function phys.IKTree.Node:SetTheta(arg1) end

--- 
function phys.IKTree.Node:ComputeS() end

--- 
--- @return int ret0
function phys.IKTree.Node:GetEffectorIndex() end

--- 
function phys.IKTree.Node:Freeze() end

--- 
function phys.IKTree.Node:UnFreeze() end

--- 
--- @return int ret0
function phys.IKTree.Node:GetJointIndex() end

--- 
--- @return bool ret0
function phys.IKTree.Node:IsFrozen() end

--- 
--- @param delta number
--- @return number ret0
function phys.IKTree.Node:AddToTheta(delta) end

--- 
--- @return math.Transform ret0
function phys.IKTree.Node:GetLocalTransform() end

--- 
--- @param attach math.Vector
--- @overload fun(attach: vector.Vector): 
function phys.IKTree.Node:SetAttach(attach) end

--- 
function phys.IKTree.Node:PrintNode() end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function phys.IKTree.Node:GetRotationAxis() end

--- 
--- @param axis math.Vector
--- @overload fun(axis: vector.Vector): 
function phys.IKTree.Node:SetRotationAxis(axis) end

--- 
--- @return phys.IKTree.Node ret0
function phys.IKTree.Node:GetLeftChildNode() end

--- 
--- @param delta number
--- @return number ret0
function phys.IKTree.Node:UpdateTheta(delta) end

--- 
--- @return phys.IKTree.Node ret0
function phys.IKTree.Node:GetRightChildNode() end

--- 
function phys.IKTree.Node:InitNode() end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function phys.IKTree.Node:GetAttach() end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function phys.IKTree.Node:GetRelativePosition() end

--- 
--- @return number ret0
function phys.IKTree.Node:GetTheta() end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function phys.IKTree.Node:GetS() end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function phys.IKTree.Node:GetW() end


--- @enum Purpose
phys.IKTree.Node = {
	PURPOSE_JOINT = 0,
	PURPOSE_EFFECTOR = 1,
}

--- 
--- @class phys.DoFConstraint: phys.Constraint
phys.DoFConstraint = {}

--- 
function phys.DoFConstraint:GetCurrentLinearAccumulatedImpulse() end

--- 
--- @param damping number
function phys.DoFConstraint:SetLinearDamping(damping) end

--- 
function phys.DoFConstraint:GetAngularLimitForceMixingFactor() end

--- 
--- @param velocity math.Vector
--- @overload fun(velocity: vector.Vector): 
function phys.DoFConstraint:SetLinearTargetVelocity(velocity) end

--- 
--- @param force math.Vector
--- @overload fun(force: vector.Vector): 
function phys.DoFConstraint:SetLinearMaxMotorForce(force) end

--- 
--- @param limit math.EulerAngles
function phys.DoFConstraint:SetAngularUpperLimit(limit) end

--- 
--- @param limit math.Vector
--- @overload fun(limit: vector.Vector): 
function phys.DoFConstraint:SetLinearLowerLimit(limit) end

--- 
function phys.DoFConstraint:GetLinearLowerLimit() end

--- 
--- @param axis int
function phys.DoFConstraint:IsLinearMotorEnabled(axis) end

--- 
--- @param limit math.Vector
--- @overload fun(limit: vector.Vector): 
function phys.DoFConstraint:SetLinearUpperLimit(limit) end

--- 
function phys.DoFConstraint:GetCurrentAngularLimitError() end

--- 
function phys.DoFConstraint:GetCurrentAngularPosition() end

--- 
--- @param limit math.EulerAngles
function phys.DoFConstraint:SetAngularLowerLimit(limit) end

--- 
function phys.DoFConstraint:GetAngularLowerLimit() end

--- 
function phys.DoFConstraint:GetAngularUpperLimit() end

--- 
--- @param factor math.Vector
--- @overload fun(factor: vector.Vector): 
function phys.DoFConstraint:SetAngularLimitForceMixingFactor(factor) end

--- 
--- @param lower math.Vector
--- @param upper math.Vector
--- @overload fun(limit: math.Vector): 
--- @overload fun(lower: vector.Vector, upper: vector.Vector): 
--- @overload fun(limit: vector.Vector): 
function phys.DoFConstraint:SetLinearLimit(lower, upper) end

--- 
--- @param lower math.EulerAngles
--- @param upper math.EulerAngles
--- @overload fun(limit: math.EulerAngles): 
function phys.DoFConstraint:SetAngularLimit(lower, upper) end

--- 
function phys.DoFConstraint:GetlinearUpperLimit() end

--- 
--- @param softness math.Vector
--- @overload fun(softness: vector.Vector): 
function phys.DoFConstraint:SetAngularLimitSoftness(softness) end

--- 
function phys.DoFConstraint:GetAngularTargetVelocity() end

--- 
function phys.DoFConstraint:GetAngularMaxMotorForce() end

--- 
--- @param force math.Vector
--- @overload fun(force: vector.Vector): 
function phys.DoFConstraint:SetAngularMaxMotorForce(force) end

--- 
function phys.DoFConstraint:GetAngularMaxLimitForce() end

--- 
--- @param force math.Vector
--- @overload fun(force: vector.Vector): 
function phys.DoFConstraint:SetAngularMaxLimitForce(force) end

--- 
--- @param factor number
function phys.DoFConstraint:SetLinearRestitutionFactor(factor) end

--- 
function phys.DoFConstraint:GetAngularDamping() end

--- 
--- @param factor math.Vector
--- @overload fun(factor: vector.Vector): 
function phys.DoFConstraint:SetAngularForceMixingFactor(factor) end

--- 
--- @param damping math.Vector
--- @overload fun(damping: vector.Vector): 
function phys.DoFConstraint:SetAngularDamping(damping) end

--- 
function phys.DoFConstraint:GetAngularForceMixingFactor() end

--- 
--- @param axis int
function phys.DoFConstraint:IsAngularMotorEnabled(axis) end

--- 
function phys.DoFConstraint:GetAngularLimitSoftness() end

--- 
function phys.DoFConstraint:GetAngularLimitErrorTolerance() end

--- 
--- @param tolerance math.Vector
--- @overload fun(tolerance: vector.Vector): 
function phys.DoFConstraint:SetAngularLimitErrorTolerance(tolerance) end

--- 
function phys.DoFConstraint:GetAngularRestitutionFactor() end

--- 
--- @param factor math.Vector
--- @overload fun(factor: vector.Vector): 
function phys.DoFConstraint:SetAngularRestitutionFactor(factor) end

--- 
--- @param bEnabled bool
--- @overload fun(axis: int, bEnabled: bool): 
function phys.DoFConstraint:SetAngularMotorEnabled(bEnabled) end

--- 
function phys.DoFConstraint:GetLinearRestitutionFactor() end

--- 
function phys.DoFConstraint:GetCurrentAngularLimit() end

--- 
function phys.DoFConstraint:GetLinearLimitForceMixingFactor() end

--- 
function phys.DoFConstraint:GetCurrentAngularAccumulatedImpulse() end

--- 
--- @param velocity math.Vector
--- @overload fun(velocity: vector.Vector): 
function phys.DoFConstraint:SetAngularTargetVelocity(velocity) end

--- 
function phys.DoFConstraint:GetLinearTargetVelocity() end

--- 
function phys.DoFConstraint:GetLinearForceMixingFactor() end

--- 
function phys.DoFConstraint:GetLinearMaxMotorForce() end

--- 
function phys.DoFConstraint:GetLinearDamping() end

--- 
function phys.DoFConstraint:GetLinearLimitSoftness() end

--- 
--- @param softness number
function phys.DoFConstraint:SetLinearLimitSoftness(softness) end

--- 
--- @param factor math.Vector
--- @overload fun(factor: vector.Vector): 
function phys.DoFConstraint:SetLinearForceMixingFactor(factor) end

--- 
function phys.DoFConstraint:GetLinearLimitErrorTolerance() end

--- 
--- @param tolerance math.Vector
--- @overload fun(tolerance: vector.Vector): 
function phys.DoFConstraint:SetLinearLimitErrorTolerance(tolerance) end

--- 
--- @param factor math.Vector
--- @overload fun(factor: vector.Vector): 
function phys.DoFConstraint:SetLinearLimitForceMixingFactor(factor) end

--- 
--- @param bEnabled bool
--- @overload fun(axis: int, bEnabled: bool): 
function phys.DoFConstraint:SetLinearMotorEnabled(bEnabled) end

--- 
function phys.DoFConstraint:GetCurrentLinearDifference() end

--- 
function phys.DoFConstraint:GetCurrentLinearLimitError() end

--- 
function phys.DoFConstraint:GetCurrentLinearLimit() end


--- 
--- @class phys.GhostObj: phys.Base
phys.GhostObj = {}

--- 
function phys.GhostObj:IsValid() end


--- 
--- @class phys.VehicleCreateInfo
--- @field gravityFactor number 
--- @field wheelDrive int 
--- @field maxEngineRotationSpeed number 
--- @field actor class util::TSharedHandle<class pragma::physics::IRigidBody> 
--- @field chassis phys.ChassisCreateInfo 
--- @field gearSwitchTime number 
--- @field maxEngineTorque number 
--- @field clutchStrength number 
--- @overload fun():phys.VehicleCreateInfo
phys.VehicleCreateInfo = {}

--- 
--- @param wheelCreateInfo phys.WheelCreateInfo
function phys.VehicleCreateInfo:AddWheel(wheelCreateInfo) end

--- 
--- @return table ret0
function phys.VehicleCreateInfo:GetWheels() end

--- 
--- @param antiRollBar phys.VehicleCreateInfo.AntiRollBar
function phys.VehicleCreateInfo:AddAntiRollBar(antiRollBar) end

--- 
--- @return table ret0
function phys.VehicleCreateInfo:GetAntiRollBars() end


--- @enum WheelDrive
phys.VehicleCreateInfo = {
	WHEEL_DRIVE_FOUR = 2,
	WHEEL_DRIVE_FRONT = 0,
	WHEEL_DRIVE_REAR = 1,
}

--- 
--- @class phys.VehicleCreateInfo.AntiRollBar
--- @field wheel1 int 
--- @field stiffness number 
--- @field wheel0 int 
--- @overload fun():phys.VehicleCreateInfo.AntiRollBar
phys.VehicleCreateInfo.AntiRollBar = {}


--- 
--- @class phys.VehicleCreateInfo.SuspensionInfo
--- @field maxDroop number 
--- @field camberAngleAtRest number 
--- @field camberAngleAtMaxDroop number 
--- @field springStrength number 
--- @field springDamperRate number 
--- @field camberAngleAtMaxCompression number 
--- @field maxCompression number 
--- @overload fun():phys.VehicleCreateInfo.SuspensionInfo
phys.VehicleCreateInfo.SuspensionInfo = {}


--- 
--- @class phys.Shape: phys.Base
phys.Shape = {}

--- 
--- @return bool ret0
function phys.Shape:IsConvex() end

--- 
--- @return math.Vector ret0_1
--- @return math.Vector ret0_2
--- @overload fun(): math.Vector, math.Vector
function phys.Shape:GetBounds() end

--- 
--- @param arg1 math.Transform
function phys.Shape:SetLocalPose(arg1) end

--- 
--- @return math.Transform ret0
function phys.Shape:GetLocalPose() end

--- 
--- @return number ret0
function phys.Shape:GetMass() end

--- 
--- @return bool ret0
function phys.Shape:IsConvexHull() end

--- 
--- @return bool ret0
function phys.Shape:IsHeightfield() end

--- 
--- @return bool ret0
function phys.Shape:IsTriangleShape() end

--- 
--- @param arg1 number
--- @param arg2 math.Vector
--- @overload fun(arg1: number, arg2: vector.Vector): 
function phys.Shape:CalculateLocalInertia(arg1, arg2) end

--- 
--- @param arg1 number
function phys.Shape:SetMass(arg1) end


--- 
--- @class phys.TriangleShape: phys.Shape, phys.Base
phys.TriangleShape = {}


--- 
--- @class phys.Heightfield: phys.Shape, phys.Base
phys.Heightfield = {}

--- 
--- @return int ret0
function phys.Heightfield:GetWidth() end

--- 
--- @param arg1 int
--- @param arg2 int
--- @param arg3 number
function phys.Heightfield:SetHeight(arg1, arg2, arg3) end

--- 
--- @return number ret0
function phys.Heightfield:GetMaxHeight() end

--- 
--- @param arg1 int
--- @param arg2 int
--- @return number ret0
function phys.Heightfield:GetHeight(arg1, arg2) end

--- 
--- @return int ret0
function phys.Heightfield:GetLength() end

--- 
--- @return int ret0
function phys.Heightfield:GetUpAxis() end


--- 
--- @class phys.CompoundShape: phys.Shape, phys.Base
phys.CompoundShape = {}


--- 
--- @class phys.ScaledTransform: phys.Transform
phys.ScaledTransform = {}

--- 
--- @param arg1 math.Vector
function phys.ScaledTransform:Scale(arg1) end

--- 
--- @param arg1 math.Vector
function phys.ScaledTransform:SetScale(arg1) end

--- 
--- @return math.Vector ret0
function phys.ScaledTransform:GetScale() end


--- 
--- @class phys.ConvexHullShape: phys.ConvexShape, phys.Shape, phys.Base
phys.ConvexHullShape = {}

--- 
--- @param arg1 int
--- @param arg2 int
--- @param arg3 int
function phys.ConvexHullShape:AddTriangle(arg1, arg2, arg3) end

--- 
--- @param arg1 int
function phys.ConvexHullShape:ReservePoints(arg1) end

--- 
function phys.ConvexHullShape:Build() end

--- 
--- @param arg1 math.Vector
--- @overload fun(arg1: vector.Vector): 
function phys.ConvexHullShape:AddPoint(arg1) end

--- 
--- @param arg1 int
function phys.ConvexHullShape:ReserveTriangles(arg1) end


--- 
--- @class phys.Material: phys.Base
phys.Material = {}

--- 
--- @param arg1 game.SurfaceMaterial
function phys.Material:SetSurfaceMaterial(arg1) end

--- 
--- @return number ret0
function phys.Material:GetStaticFriction() end

--- 
--- @return game.SurfaceMaterial ret0
function phys.Material:GetSurfaceMaterial() end

--- 
--- @return string ret0
function phys.Material:__tostring() end

--- 
--- @param arg1 number
function phys.Material:SetStaticFriction(arg1) end

--- 
--- @param arg1 number
function phys.Material:SetFriction(arg1) end

--- 
--- @param arg1 number
function phys.Material:SetDynamicFriction(arg1) end

--- 
--- @return number ret0
function phys.Material:GetDynamicFriction() end

--- 
--- @return number ret0
function phys.Material:GetRestitution() end

--- 
--- @param arg1 number
function phys.Material:SetRestitution(arg1) end


--- 
--- @class phys.RayCastData
--- @overload fun():phys.RayCastData
phys.RayCastData = {}

--- 
--- @param mask int
function phys.RayCastData:SetCollisionFilterMask(mask) end

--- 
--- @param group int
function phys.RayCastData:SetCollisionFilterGroup(group) end

--- 
--- @param arg1 math.Transform
--- @overload fun(arg1: math.Vector): 
--- @overload fun(arg1: vector.Vector): 
function phys.RayCastData:SetSource(arg1) end

--- 
--- @param arg1 math.Transform
--- @overload fun(arg1: math.Vector): 
--- @overload fun(arg1: vector.Vector): 
function phys.RayCastData:SetTarget(arg1) end

--- 
--- @param arg1 math.Quaternion
function phys.RayCastData:SetSourceRotation(arg1) end

--- 
--- @param shape phys.ConvexShape
function phys.RayCastData:SetShape(shape) end

--- 
--- @param arg1 math.Quaternion
function phys.RayCastData:SetTargetRotation(arg1) end

--- 
--- @param object any
function phys.RayCastData:SetFilter(object) end

--- 
function phys.RayCastData:GetSourceTransform() end

--- 
function phys.RayCastData:GetTargetTransform() end

--- 
function phys.RayCastData:GetSourceOrigin() end

--- 
function phys.RayCastData:GetTargetOrigin() end

--- 
function phys.RayCastData:GetSourceRotation() end

--- 
function phys.RayCastData:GetTargetRotation() end

--- 
--- @param arg1 math.Quaternion
function phys.RayCastData:SetRotation(arg1) end

--- 
--- @return string ret0
function phys.RayCastData:__tostring() end

--- 
function phys.RayCastData:GetDirection() end

--- 
function phys.RayCastData:GetDistance() end

--- 
--- @param flags int
function phys.RayCastData:SetFlags(flags) end


--- 
--- @class phys.RigidBody: phys.CollisionObj, phys.Base
phys.RigidBody = {}

--- 
function phys.RigidBody:GetLinearDamping() end

--- 
function phys.RigidBody:GetAngularVelocity() end

--- 
--- @param vel math.Vector
--- @overload fun(vel: vector.Vector): 
function phys.RigidBody:SetLinearVelocity(vel) end

--- 
function phys.RigidBody:GetTotalTorque() end

--- 
function phys.RigidBody:GetLinearVelocity() end

--- 
function phys.RigidBody:GetInertia() end

--- 
--- @param linDamping number
--- @param angDamping number
function phys.RigidBody:SetDamping(linDamping, angDamping) end

--- 
function phys.RigidBody:GetInvInertiaTensorWorld() end

--- 
function phys.RigidBody:ClearForces() end

--- 
function phys.RigidBody:GetLinearSleepingThreshold() end

--- 
function phys.RigidBody:GetTotalForce() end

--- 
function phys.RigidBody:GetBoneID() end

--- 
--- @param linDamping number
function phys.RigidBody:SetLinearDamping(linDamping) end

--- 
--- @param boneId int
function phys.RigidBody:SetBoneID(boneId) end

--- 
--- @param bKinematic bool
function phys.RigidBody:SetKinematic(bKinematic) end

--- 
--- @param factor math.Vector
--- @overload fun(factor: vector.Vector): 
function phys.RigidBody:SetLinearFactor(factor) end

--- 
function phys.RigidBody:GetLinearFactor() end

--- 
function phys.RigidBody:GetAngularSleepingThreshold() end

--- 
--- @param factor math.Vector
--- @overload fun(factor: vector.Vector): 
function phys.RigidBody:SetAngularFactor(factor) end

--- 
function phys.RigidBody:GetAngularFactor() end

--- 
--- @param threshold number
function phys.RigidBody:SetLinearSleepingThreshold(threshold) end

--- 
--- @param impulse math.Vector
--- @param relPos math.Vector
--- @overload fun(impulse: math.Vector): 
--- @overload fun(impulse: vector.Vector, relPos: vector.Vector): 
--- @overload fun(impulse: vector.Vector): 
function phys.RigidBody:ApplyImpulse(impulse, relPos) end

--- 
--- @param threshold number
function phys.RigidBody:SetAngularSleepingThreshold(threshold) end

--- 
--- @param linear number
--- @param angular number
function phys.RigidBody:SetSleepingThresholds(linear, angular) end

--- 
function phys.RigidBody:GetSleepingThreshold() end

--- 
function phys.RigidBody:GetCenterOfMassOffset() end

--- 
--- @param offset math.Vector
--- @overload fun(offset: vector.Vector): 
function phys.RigidBody:SetCenterOfMassOffset(offset) end

--- 
--- @param vel math.Vector
--- @overload fun(vel: vector.Vector): 
function phys.RigidBody:SetAngularVelocity(vel) end

--- 
function phys.RigidBody:IsKinematic() end

--- 
--- @param force math.Vector
--- @param relPos math.Vector
--- @overload fun(force: math.Vector): 
--- @overload fun(force: vector.Vector, relPos: vector.Vector): 
--- @overload fun(force: vector.Vector): 
function phys.RigidBody:ApplyForce(force, relPos) end

--- 
--- @param torque math.Vector
--- @overload fun(torque: vector.Vector): 
function phys.RigidBody:ApplyTorque(torque) end

--- 
function phys.RigidBody:GetAngularDamping() end

--- 
--- @param mass number
function phys.RigidBody:SetMass(mass) end

--- 
--- @param torque math.Vector
--- @overload fun(torque: vector.Vector): 
function phys.RigidBody:ApplyTorqueImpulse(torque) end

--- 
function phys.RigidBody:GetMass() end

--- 
--- @param angDamping number
function phys.RigidBody:SetAngularDamping(angDamping) end


--- 
--- @class phys.RayCastResult
--- @field physObj class util::TWeakSharedHandle<class PhysObj> 
--- @field normal  
--- @field position  
--- @field distance number 
--- @field fraction number 
--- @field materialName string 
--- @field subMesh Model.Mesh.Sub 
--- @field material game.Material 
--- @field colObj phys.CollisionObj 
--- @field startPosition  
--- @field entity nil 
--- @field mesh Model.Mesh 
--- @field hitType int 
phys.RayCastResult = {}

--- 
--- @return string ret0
function phys.RayCastResult:__tostring() end


--- @enum HitType
phys.RayCastResult = {
	HIT_TYPE_BLOCK = 2,
	HIT_TYPE_NONE = 0,
	HIT_TYPE_TOUCH = 1,
}

--- 
--- @class phys.DoFSpringConstraint: phys.Constraint
phys.DoFSpringConstraint = {}

--- 
--- @param type int
--- @param axis int
--- @param stiffness number
--- @overload fun(type: int, axis: int, stiffness: number, limitIfNeeded: bool): 
function phys.DoFSpringConstraint:SetStiffness(type, axis, stiffness) end

--- 
--- @param transformA math.Transform
--- @param transformB math.Transform
--- @overload fun(): 
function phys.DoFSpringConstraint:CalculateTransforms(transformA, transformB) end

--- 
--- @param type int
--- @param axis int
function phys.DoFSpringConstraint:GetERP(type, axis) end

--- 
function phys.DoFSpringConstraint:GetCalculatedTransformA() end

--- 
--- @param type int
--- @param axis int
--- @param target number
function phys.DoFSpringConstraint:SetServoTarget(type, axis, target) end

--- 
function phys.DoFSpringConstraint:GetCalculatedTransformB() end

--- 
--- @param axis int
function phys.DoFSpringConstraint:GetAxis(axis) end

--- 
--- @param axis int
function phys.DoFSpringConstraint:GetRelativePivotPosition(axis) end

--- 
function phys.DoFSpringConstraint:GetFrameOffsetA() end

--- 
function phys.DoFSpringConstraint:GetLinearLowerLimit() end

--- 
function phys.DoFSpringConstraint:GetFrameOffsetB() end

--- 
--- @param transformA math.Transform
--- @param transformB math.Transform
function phys.DoFSpringConstraint:SetFrames(transformA, transformB) end

--- 
--- @param linearLower math.Vector
--- @overload fun(linearLower: vector.Vector): 
function phys.DoFSpringConstraint:SetLinearLowerLimit(linearLower) end

--- 
--- @param axis1 math.Vector
--- @param axis2 math.Vector
--- @overload fun(axis1: vector.Vector, axis2: vector.Vector): 
function phys.DoFSpringConstraint:SetAxis(axis1, axis2) end

--- 
--- @param linearUpper math.Vector
--- @overload fun(linearUpper: vector.Vector): 
function phys.DoFSpringConstraint:SetLinearUpperLimit(linearUpper) end

--- 
--- @param type int
--- @param axis int
function phys.DoFSpringConstraint:GetStopCFM(type, axis) end

--- 
function phys.DoFSpringConstraint:GetLinearUpperLimit() end

--- 
--- @param type int
--- @param axis int
--- @param velocity number
function phys.DoFSpringConstraint:SetTargetVelocity(type, axis, velocity) end

--- 
--- @param angularLower math.Vector
--- @overload fun(angularLower: vector.Vector): 
function phys.DoFSpringConstraint:SetAngularLowerLimit(angularLower) end

--- 
--- @param type int
--- @param axis int
--- @param damping number
--- @overload fun(type: int, axis: int, damping: number, limitIfNeeded: bool): 
function phys.DoFSpringConstraint:SetDamping(type, axis, damping) end

--- 
--- @param angularLower math.Vector
--- @overload fun(angularLower: vector.Vector): 
function phys.DoFSpringConstraint:SetAngularLowerLimitReversed(angularLower) end

--- 
function phys.DoFSpringConstraint:GetAngularLowerLimit() end

--- 
function phys.DoFSpringConstraint:GetAngularLowerLimitReversed() end

--- 
--- @param angularUpper math.Vector
--- @overload fun(angularUpper: vector.Vector): 
function phys.DoFSpringConstraint:SetAngularUpperLimit(angularUpper) end

--- 
--- @param angularUpper math.Vector
--- @overload fun(angularUpper: vector.Vector): 
function phys.DoFSpringConstraint:SetAngularUpperLimitReversed(angularUpper) end

--- 
--- @param axis int
function phys.DoFSpringConstraint:GetAngle(axis) end

--- 
function phys.DoFSpringConstraint:GetAngularUpperLimit() end

--- 
function phys.DoFSpringConstraint:GetAngularUpperLimitReversed() end

--- 
--- @param type int
--- @param axis int
--- @param lo number
--- @param hi number
function phys.DoFSpringConstraint:SetLimitReversed(type, axis, lo, hi) end

--- 
--- @param axis int
--- @param type int
function phys.DoFSpringConstraint:IsLimited(axis, type) end

--- 
--- @param type int
--- @param axis int
--- @param value number
function phys.DoFSpringConstraint:SetStopCFM(type, axis, value) end

--- 
--- @param rotationOrder int
function phys.DoFSpringConstraint:SetRotationOrder(rotationOrder) end

--- 
--- @param type int
--- @param axis int
--- @param bounce number
function phys.DoFSpringConstraint:SetBounce(type, axis, bounce) end

--- 
function phys.DoFSpringConstraint:GetRotationOrder() end

--- 
--- @param type int
--- @param axis int
--- @param enable bool
function phys.DoFSpringConstraint:EnableMotor(type, axis, enable) end

--- 
--- @param type int
--- @param axis int
--- @param enable bool
function phys.DoFSpringConstraint:SetServo(type, axis, enable) end

--- 
--- @param type int
--- @param axis int
--- @param force number
function phys.DoFSpringConstraint:SetMaxMotorForce(type, axis, force) end

--- 
--- @param type int
--- @param axis int
--- @param enable bool
function phys.DoFSpringConstraint:EnableSpring(type, axis, enable) end

--- 
--- @param type int
--- @param axis int
--- @param value number
--- @overload fun(type: int, axis: int): 
--- @overload fun(): 
function phys.DoFSpringConstraint:SetEquilibriumPoint(type, axis, value) end

--- 
--- @param type int
--- @param axis int
--- @param value number
function phys.DoFSpringConstraint:SetERP(type, axis, value) end

--- 
--- @param type int
--- @param axis int
--- @param value number
function phys.DoFSpringConstraint:SetStopERP(type, axis, value) end

--- 
--- @param type int
--- @param axis int
function phys.DoFSpringConstraint:GetStopERP(type, axis) end

--- 
--- @param type int
--- @param axis int
--- @param value number
function phys.DoFSpringConstraint:SetCFM(type, axis, value) end

--- 
--- @param type int
--- @param axis int
--- @param lo number
--- @param hi number
function phys.DoFSpringConstraint:SetLimit(type, axis, lo, hi) end

--- 
--- @param type int
--- @param axis int
function phys.DoFSpringConstraint:GetCFM(type, axis) end


--- 
--- @class phys.BallSocketConstraint: phys.Constraint
phys.BallSocketConstraint = {}


--- 
--- @class phys.SoftBody: phys.CollisionObj, phys.Base
phys.SoftBody = {}

--- 
--- @param nodeIndex int
function phys.SoftBody:NodeIndexToLocalVertexIndex(nodeIndex) end

--- 
--- @param density number
function phys.SoftBody:SetDensity(density) end

--- 
--- @param vel math.Vector
--- @overload fun(vel: vector.Vector): 
function phys.SoftBody:SetLinearVelocity(vel) end

--- 
--- @param scale number
function phys.SoftBody:SetRestLengthScale(scale) end

--- 
--- @param node int
--- @param force math.Vector
--- @overload fun(node: int, force: vector.Vector): 
function phys.SoftBody:AddAeroForceToNode(node, force) end

--- 
function phys.SoftBody:GetSoftVsRigidHardness() end

--- 
function phys.SoftBody:GetKineticContactsHardness() end

--- 
function phys.SoftBody:GetFriction() end

--- 
--- @param face int
--- @param force math.Vector
--- @overload fun(face: int, force: vector.Vector): 
function phys.SoftBody:AddAeroForceToFace(face, force) end

--- 
--- @param val number
function phys.SoftBody:SetSoftContactsHardness(val) end

--- 
function phys.SoftBody:GetRollingFriction() end

--- 
function phys.SoftBody:GetDragCoefficient() end

--- 
function phys.SoftBody:GetHitFraction() end

--- 
--- @param vel math.Vector
--- @overload fun(node: int, vel: math.Vector): 
--- @overload fun(vel: vector.Vector): 
--- @overload fun(node: int, vel: vector.Vector): 
function phys.SoftBody:AddLinearVelocity(vel) end

--- 
--- @param val number
function phys.SoftBody:SetSoftVsSoftHardness(val) end

--- 
function phys.SoftBody:GetAnisotropicFriction() end

--- 
--- @param matId int
function phys.SoftBody:GetMaterialVolumeStiffnessCoefficient(matId) end

--- 
--- @param fraction number
function phys.SoftBody:SetHitFraction(fraction) end

--- 
--- @param friction math.Vector
--- @overload fun(friction: vector.Vector): 
function phys.SoftBody:SetAnisotropicFriction(friction) end

--- 
--- @param val number
function phys.SoftBody:SetDragCoefficient(val) end

--- 
--- @param friction number
function phys.SoftBody:SetRollingFriction(friction) end

--- 
function phys.SoftBody:GetRestLengthScale() end

--- 
function phys.SoftBody:GetRigidContactsHardness() end

--- 
function phys.SoftBody:GetWindVelocity() end

--- 
--- @param vel math.Vector
--- @overload fun(vel: vector.Vector): 
function phys.SoftBody:SetWindVelocity(vel) end

--- 
--- @param val number
function phys.SoftBody:SetPoseMatchingCoefficient(val) end

--- 
--- @param density number
function phys.SoftBody:SetVolumeDensity(density) end

--- 
--- @param val number
function phys.SoftBody:SetSoftVsRigidImpulseSplitK(val) end

--- 
--- @param val number
function phys.SoftBody:SetVelocitiesCorrectionFactor(val) end

--- 
--- @param mass number
function phys.SoftBody:SetVolumeMass(mass) end

--- 
--- @param val number
function phys.SoftBody:SetPressureCoefficient(val) end

--- 
--- @param val number
function phys.SoftBody:SetAnchorsHardness(val) end

--- 
--- @param friction number
function phys.SoftBody:SetFriction(friction) end

--- 
function phys.SoftBody:GetVolume() end

--- 
--- @param val number
function phys.SoftBody:SetRigidContactsHardness(val) end

--- 
--- @param val number
function phys.SoftBody:SetDynamicFrictionCoefficient(val) end

--- 
--- @param val number
function phys.SoftBody:SetDampingCoefficient(val) end

--- 
--- @param val number
function phys.SoftBody:SetKineticContactsHardness(val) end

--- 
--- @param val number
function phys.SoftBody:SetSoftVsRigidImpulseSplitS(val) end

--- 
--- @param val number
function phys.SoftBody:SetLiftCoefficient(val) end

--- 
function phys.SoftBody:GetDynamicFrictionCoefficient() end

--- 
--- @param val number
function phys.SoftBody:SetSoftVsKineticHardness(val) end

--- 
--- @param rest number
function phys.SoftBody:SetRestitution(rest) end

--- 
--- @param val number
function phys.SoftBody:SetSoftVsRigidHardness(val) end

--- 
function phys.SoftBody:GetRestitution() end

--- 
--- @param localVertexIndex int
function phys.SoftBody:LocalVertexIndexToNodeIndex(localVertexIndex) end

--- 
--- @param val number
function phys.SoftBody:SetSoftVsRigidImpulseSplitR(val) end

--- 
--- @param val number
function phys.SoftBody:SetVolumeConversationCoefficient(val) end

--- 
--- @overload fun(node: int): 
function phys.SoftBody:GetMass() end

--- 
function phys.SoftBody:GetAnchorsHardness() end

--- 
function phys.SoftBody:GetLiftCoefficient() end

--- 
function phys.SoftBody:GetVelocitiesCorrectionFactor() end

--- 
function phys.SoftBody:GetPoseMatchingCoefficient() end

--- 
function phys.SoftBody:GetSoftContactsHardness() end

--- 
function phys.SoftBody:GetSoftVsKineticHardness() end

--- 
--- @param matId int
function phys.SoftBody:GetMaterialLinearStiffnessCoefficient(matId) end

--- 
function phys.SoftBody:GetSoftVsRigidImpulseSplitK() end

--- 
function phys.SoftBody:GetVolumeConversationCoefficient() end

--- 
function phys.SoftBody:GetSoftVsRigidImpulseSplitR() end

--- 
function phys.SoftBody:GetSoftVsRigidImpulseSplitS() end

--- 
--- @param matId int
--- @param val number
function phys.SoftBody:SetMaterialLinearStiffnessCoefficient(matId, val) end

--- 
--- @param matId int
--- @param val number
function phys.SoftBody:SetMaterialVolumeStiffnessCoefficient(matId, val) end

--- 
--- @param matId int
function phys.SoftBody:GetMaterialAngularStiffnessCoefficient(matId) end

--- 
--- @param nodeId int
--- @param hRigidBody phys.RigidBody
--- @overload fun(nodeId: int, hRigidBody: phys.RigidBody, bDisableCollision: bool): 
--- @overload fun(nodeId: int, hRigidBody: phys.RigidBody, bDisableCollision: bool, influence: number): 
--- @overload fun(nodeId: int, hRigidBody: phys.RigidBody, localPivot: math.Vector): 
--- @overload fun(nodeId: int, hRigidBody: phys.RigidBody, localPivot: math.Vector, bDisableCollision: bool): 
--- @overload fun(nodeId: int, hRigidBody: phys.RigidBody, localPivot: math.Vector, bDisableCollision: bool, influence: number): 
--- @overload fun(nodeId: int, hRigidBody: phys.RigidBody, localPivot: vector.Vector): 
--- @overload fun(nodeId: int, hRigidBody: phys.RigidBody, localPivot: vector.Vector, bDisableCollision: bool): 
--- @overload fun(nodeId: int, hRigidBody: phys.RigidBody, localPivot: vector.Vector, bDisableCollision: bool, influence: number): 
function phys.SoftBody:AppendAnchor(nodeId, hRigidBody) end

--- 
--- @param localIndex int
function phys.SoftBody:LocalVertexIndexToMeshVertexIndex(localIndex) end

--- 
--- @param meshVertexIndex int
function phys.SoftBody:MeshVertexIndexToLocalVertexIndex(meshVertexIndex) end

--- 
--- @param meshVertexIndex int
function phys.SoftBody:MeshVertexIndexToNodeIndex(meshVertexIndex) end

--- 
function phys.SoftBody:GetNodeCount() end

--- 
--- @param nodeIndex int
function phys.SoftBody:NodeIndexToMeshVertexIndex(nodeIndex) end

--- 
--- @param mass number
--- @overload fun(node: int, mass: number): 
function phys.SoftBody:SetMass(mass) end

--- 
--- @param force math.Vector
--- @overload fun(node: int, force: math.Vector): 
--- @overload fun(force: vector.Vector): 
--- @overload fun(node: int, force: vector.Vector): 
function phys.SoftBody:ApplyForce(force) end

--- 
function phys.SoftBody:GetDampingCoefficient() end

--- 
function phys.SoftBody:GetPressureCoefficient() end

--- 
function phys.SoftBody:GetSoftVsSoftHardness() end

--- 
--- @param matId int
--- @param val number
function phys.SoftBody:SetMaterialAngularStiffnessCoefficient(matId, val) end


--- 
--- @class phys.SoftBodyInfo
--- @field clusterCount int 
--- @field poseMatchingCoefficient number 
--- @field dragCoefficient number 
--- @field anchorsHardness number 
--- @field rigidContactsHardness number 
--- @field kineticContactsHardness number 
--- @field softContactsHardness number 
--- @field dynamicFrictionCoefficient number 
--- @field softVsRigidImpulseSplitK number 
--- @field liftCoefficient number 
--- @field dampingCoefficient number 
--- @field volumeConversationCoefficient number 
--- @field softVsRigidImpulseSplitR number 
--- @field softVsRigidImpulseSplitS number 
--- @field softVsKineticHardness number 
--- @field softVsRigidHardness number 
--- @field softVsSoftHardness number 
--- @field pressureCoefficient number 
--- @field velocitiesCorrectionFactor number 
--- @field bendingConstraintsDistance number 
--- @field maxClusterIterations int 
phys.SoftBodyInfo = {}

--- 
--- @param matId int
--- @param linear number
--- @param angular number
--- @param volume number
function phys.SoftBodyInfo:SetMaterialStiffnessCoefficients(matId, linear, angular, volume) end

--- 
--- @param matId int
--- @return number ret0_1
--- @return number ret0_2
--- @return number ret0_3
function phys.SoftBodyInfo:GetMaterialStiffnessCoefficients(matId) end


--- 
--- @class phys.IKJacobian
phys.IKJacobian = {}

--- 
function phys.IKJacobian:Reset() end

--- 
function phys.IKJacobian:SetJtargetActive() end

--- 
--- @param vTargets table
--- @overload fun(vTargets: table): 
function phys.IKJacobian:ComputeJacobian(vTargets) end

--- 
function phys.IKJacobian:CalcDeltaThetas() end

--- 
function phys.IKJacobian:CalcDeltaThetasPseudoinverse() end

--- 
function phys.IKJacobian:SetJendActive() end

--- 
function phys.IKJacobian:ZeroDeltaThetas() end

--- 
function phys.IKJacobian:CalcDeltaThetasTranspose() end

--- 
function phys.IKJacobian:CalcDeltaThetasDLS() end

--- 
function phys.IKJacobian:CalcDeltaThetasDLSwithSVD() end

--- 
function phys.IKJacobian:CalcDeltaThetasSDLS() end

--- 
function phys.IKJacobian:UpdateThetas() end

--- 
function phys.IKJacobian:UpdateThetaDot() end

--- 
--- @param vTargets table
--- @overload fun(vTargets: table): 
function phys.IKJacobian:UpdatedSClampValue(vTargets) end

--- 
--- @return int ret0
function phys.IKJacobian:GetRowCount() end

--- 
--- @return int ret0
function phys.IKJacobian:GetColumnCount() end


--- 
--- @class phys.FixedConstraint: phys.Constraint
phys.FixedConstraint = {}


--- 
--- @class phys.Base
phys.Base = {}


--- 
--- @class phys.CapsuleShape: phys.ConvexShape, phys.Shape, phys.Base
phys.CapsuleShape = {}

--- 
--- @return number ret0
function phys.CapsuleShape:GetRadius() end

--- 
--- @return number ret0
function phys.CapsuleShape:GetHalfHeight() end


--- 
--- @class phys.ConvexShape: phys.Shape, phys.Base
phys.ConvexShape = {}

--- 
--- @return Model.CollisionMesh ret0
function phys.ConvexShape:GetCollisionMesh() end


--- 
--- @class phys.ContactPoint
--- @field position  
--- @field normal  
--- @field distance number 
--- @field impulse  
--- @field material0 phys.Material 
--- @field material1 phys.Material 
phys.ContactPoint = {}


--- 
--- @class phys.HingeConstraint: phys.Constraint
phys.HingeConstraint = {}


--- 
--- @class phys.Object
phys.Object = {}

--- 
--- @param arg1 math.Vector
--- @overload fun(arg1: vector.Vector): 
function phys.Object:SetPos(arg1) end

--- 
--- @return bool ret0
function phys.Object:IsGroundWalkable() end

--- 
--- @return ents.BaseEntityBase ret0
function phys.Object:GetGroundEntity() end

--- 
--- @param arg1 enum CollisionMask
function phys.Object:SetCollisionFilterMask(arg1) end

--- 
--- @param group int
function phys.Object:SetCollisionFilterGroup(group) end

--- 
--- @param arg1 math.Vector
--- @param arg2 math.Vector
--- @overload fun(arg1: math.Vector): 
--- @overload fun(arg1: vector.Vector, arg2: vector.Vector): 
--- @overload fun(arg1: vector.Vector): 
function phys.Object:ApplyForce(arg1, arg2) end

--- 
--- @param arg1 math.Vector
--- @param arg2 math.Vector
--- @overload fun(arg1: math.Vector): 
--- @overload fun(arg1: vector.Vector, arg2: vector.Vector): 
--- @overload fun(arg1: vector.Vector): 
function phys.Object:ApplyImpulse(arg1, arg2) end

--- 
--- @return number ret0
function phys.Object:GetGroundFriction() end

--- 
--- @param arg1 number
function phys.Object:SetLinearSleepingThreshold(arg1) end

--- 
--- @return enum CollisionMask ret0
function phys.Object:GetCollisionFilterGroup() end

--- 
--- @return ents.EntityComponent ret0
function phys.Object:GetOwner() end

--- 
--- @param arg1 number
--- @param arg2 number
function phys.Object:SetDamping(arg1, arg2) end

--- 
--- @return phys.CollisionObj ret0
function phys.Object:GetGroundPhysCollisionObject() end

--- 
--- @return enum CollisionMask ret0
function phys.Object:GetCollisionFilterMask() end

--- 
--- @param arg1 number
function phys.Object:SetMass(arg1) end

--- 
--- @param arg1 math.Vector
--- @overload fun(arg1: vector.Vector): 
function phys.Object:ApplyTorque(arg1) end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function phys.Object:GetAngularVelocity() end

--- 
--- @return number ret0
function phys.Object:GetLinearDamping() end

--- 
function phys.Object:WakeUp() end

--- 
--- @param arg1 math.Vector
--- @overload fun(arg1: vector.Vector): 
function phys.Object:SetLinearVelocity(arg1) end

--- 
function phys.Object:PutToSleep() end

--- 
--- @param arg1 math.Vector
--- @overload fun(arg1: vector.Vector): 
function phys.Object:SetAngularVelocity(arg1) end

--- 
function phys.Object:ClearForces() end

--- 
--- @return number ret0
function phys.Object:GetLinearSleepingThreshold() end

--- 
--- @return number ret0
function phys.Object:GetAngularDamping() end

--- 
--- @return phys.Object ret0
function phys.Object:GetGroundPhysObject() end

--- 
--- @return table ret0
function phys.Object:GetCollisionObjects() end

--- 
--- @param arg1 number
function phys.Object:SetAngularDamping(arg1) end

--- 
--- @return number ret0
function phys.Object:GetMass() end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function phys.Object:GetGroundVelocity() end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function phys.Object:GetLinearVelocity() end

--- 
--- @return bool ret0
function phys.Object:IsValid() end

--- 
--- @return number ret0_1
--- @return number ret0_2
function phys.Object:GetSleepingThreshold() end

--- 
--- @param arg1 math.Vector
--- @overload fun(arg1: vector.Vector): 
function phys.Object:AddAngularVelocity(arg1) end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function phys.Object:GetTotalForce() end

--- 
--- @return int ret0
function phys.Object:GetGroundSurfaceMaterial() end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function phys.Object:GetTotalTorque() end

--- 
--- @param arg1 math.Quaternion
function phys.Object:SetRotation(arg1) end

--- 
--- @param arg1 math.Vector
--- @overload fun(arg1: vector.Vector): 
function phys.Object:ApplyTorqueImpulse(arg1) end

--- 
--- @param arg1 math.Vector
--- @overload fun(arg1: vector.Vector): 
function phys.Object:SetLinearFactor(arg1) end

--- 
--- @return math.Quaternion ret0
function phys.Object:GetRotation() end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function phys.Object:GetLinearFactor() end

--- 
--- @return number ret0
function phys.Object:GetAngularSleepingThreshold() end

--- 
--- @param arg1 math.Vector
--- @overload fun(arg1: vector.Vector): 
function phys.Object:SetAngularFactor(arg1) end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function phys.Object:GetAngularFactor() end

--- 
--- @param arg1 number
function phys.Object:SetAngularSleepingThreshold(arg1) end

--- 
--- @param arg1 number
--- @param arg2 number
function phys.Object:SetSleepingThresholds(arg1, arg2) end

--- 
--- @return bool ret0
function phys.Object:IsOnGround() end

--- 
--- @param arg1 number
function phys.Object:SetLinearDamping(arg1) end

--- 
--- @param arg1 math.Vector
--- @param arg2 math.Vector
--- @overload fun(arg1: vector.Vector, arg2: vector.Vector): 
function phys.Object:GetBounds(arg1, arg2) end

--- 
function phys.Object:__tostring() end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function phys.Object:GetPos() end

--- 
--- @param arg1 enum CollisionMask
--- @param arg2 enum CollisionMask
function phys.Object:SetCollisionFilter(arg1, arg2) end

--- 
--- @param arg1 math.Vector
--- @overload fun(arg1: vector.Vector): 
function phys.Object:AddLinearVelocity(arg1) end


--- 
--- @class phys.CollisionObj: phys.Base
phys.CollisionObj = {}

--- 
--- @param pos math.Vector
--- @overload fun(pos: vector.Vector): 
function phys.CollisionObj:SetPos(pos) end

--- 
--- @param mask int
function phys.CollisionObj:SetCollisionFilterMask(mask) end

--- 
function phys.CollisionObj:IsSimulationEnabled() end

--- 
--- @param group int
function phys.CollisionObj:SetCollisionFilterGroup(group) end

--- 
function phys.CollisionObj:GetBoundingSphere() end

--- 
function phys.CollisionObj:GetCollisionFilterGroup() end

--- 
--- @param b bool
function phys.CollisionObj:SetSimulationEnabled(b) end

--- 
function phys.CollisionObj:IsRigidBody() end

--- 
function phys.CollisionObj:Spawn() end

--- 
function phys.CollisionObj:IsGhostObject() end

--- 
function phys.CollisionObj:GetAngles() end

--- 
function phys.CollisionObj:DisableSimulation() end

--- 
function phys.CollisionObj:EnableSimulation() end

--- 
function phys.CollisionObj:IsValid() end

--- 
function phys.CollisionObj:GetActivationState() end

--- 
--- @overload fun(bForceActivation: bool): 
function phys.CollisionObj:WakeUp() end

--- 
--- @param state int
function phys.CollisionObj:SetActivationState(state) end

--- 
function phys.CollisionObj:GetWorldTransform() end

--- 
--- @param t math.Transform
function phys.CollisionObj:SetWorldTransform(t) end

--- 
function phys.CollisionObj:PutToSleep() end

--- 
function phys.CollisionObj:IsAsleep() end

--- 
function phys.CollisionObj:__tostring() end

--- 
--- @param alwaysAwake bool
function phys.CollisionObj:SetAlwaysAwake(alwaysAwake) end

--- 
--- @return bool ret0
function phys.CollisionObj:IsAlwaysAwake() end

--- 
function phys.CollisionObj:GetShape() end

--- 
function phys.CollisionObj:Remove() end

--- 
function phys.CollisionObj:GetPos() end

--- 
--- @param ang math.EulerAngles
function phys.CollisionObj:SetAngles(ang) end

--- 
--- @param origin math.Vector
--- @overload fun(origin: vector.Vector): 
function phys.CollisionObj:SetOrigin(origin) end

--- 
function phys.CollisionObj:GetBounds() end

--- 
function phys.CollisionObj:GetRotation() end

--- 
--- @param rot math.Quaternion
function phys.CollisionObj:SetRotation(rot) end

--- 
function phys.CollisionObj:GetOrigin() end

--- 
function phys.CollisionObj:IsSoftBody() end

--- 
function phys.CollisionObj:GetCollisionFilterMask() end


--- 
--- @class phys.SliderConstraint: phys.Constraint
phys.SliderConstraint = {}

--- 
--- @param softness number
function phys.SliderConstraint:SetSoftness(softness) end

--- 
function phys.SliderConstraint:DisableLimit() end

--- 
function phys.SliderConstraint:GetSoftness() end

--- 
function phys.SliderConstraint:GetDamping() end

--- 
function phys.SliderConstraint:GetLimits() end

--- 
--- @param lower number
--- @param upper number
function phys.SliderConstraint:SetLimits(lower, upper) end

--- 
--- @param damping number
function phys.SliderConstraint:SetDamping(damping) end

--- 
function phys.SliderConstraint:GetRestitution() end

--- 
--- @param restitution number
function phys.SliderConstraint:SetRestitution(restitution) end


--- 
--- @class phys.ConeTwistConstraint: phys.Constraint
phys.ConeTwistConstraint = {}

--- 
--- @param ang math.EulerAngles
--- @overload fun(swingSpan1: number, swingSpan2: number, twistSpan: number): 
function phys.ConeTwistConstraint:SetLimit(ang) end


--- 
--- @class phys.Constraint
phys.Constraint = {}

--- 
function phys.Constraint:GetSourcePosition() end

--- 
function phys.Constraint:Remove() end

--- 
--- @param threshold number
function phys.Constraint:SetBreakForce(threshold) end

--- 
function phys.Constraint:GetBreakTorque() end

--- 
function phys.Constraint:GetSourceRotation() end

--- 
function phys.Constraint:GetTargetRotation() end

--- 
--- @param threshold number
function phys.Constraint:SetBreakTorque(threshold) end

--- 
function phys.Constraint:GetTargetTransform() end

--- 
function phys.Constraint:GetSourceTransform() end

--- 
--- @param b bool
function phys.Constraint:SetEnabled(b) end

--- 
function phys.Constraint:IsEnabled() end

--- 
function phys.Constraint:DisableCollisions() end

--- 
function phys.Constraint:EnableCollisions() end

--- 
function phys.Constraint:GetCollisionsEnabled() end

--- 
--- @param b bool
function phys.Constraint:SetCollisionsEnabled(b) end

--- 
function phys.Constraint:GetBreakForce() end

--- 
function phys.Constraint:GetSourceActor() end

--- 
function phys.Constraint:GetTargetActor() end

--- 
function phys.Constraint:IsValid() end

--- 
function phys.Constraint:GetTargetPosition() end


--- 
--- @class phys.IKController
phys.IKController = {}

--- 
--- @param arg1 string
function phys.IKController:SetEffectorName(arg1) end

--- 
--- @param arg2 map
function phys.IKController:SetKeyValues(arg2) end

--- 
--- @param arg1 int
function phys.IKController:SetChainLength(arg1) end

--- 
--- @param arg1 enum util::ik::Method
function phys.IKController:SetMethod(arg1) end

--- 
--- @return enum util::ik::Method ret0
function phys.IKController:GetMethod() end

--- 
--- @param key string
--- @return string ret0
function phys.IKController:GetKeyValue(key) end

--- 
--- @return string ret0
function phys.IKController:GetType() end

--- 
--- @param key string
--- @param value string
function phys.IKController:SetKeyValue(key, value) end

--- 
--- @return map ret0
function phys.IKController:GetKeyValues() end

--- 
--- @return int ret0
function phys.IKController:GetChainLength() end

--- 
--- @return string ret0
function phys.IKController:GetEffectorName() end

--- 
--- @param arg1 string
function phys.IKController:SetType(arg1) end


--- @enum Method
phys.IKController = {
	METHOD_DEFAULT = 0,
	METHOD_JACOBIAN_TRANSPOSE = 4,
	METHOD_PSEUDOINVERSE = 3,
	METHOD_SELECTIVELY_DAMPED_LEAST_SQUARE = 0,
}

--- @enum MethodDampedLeast
phys.IKController = {
	METHOD_DAMPED_LEAST_SQUARES = 1,
	METHOD_DAMPED_LEAST_SQUARES_WITH_SINGULAR_VALUE_DECOMPOSITION = 2,
}

--- 
--- @class phys.WheelCreateInfo
--- @field flags int 
--- @field suspension  
--- @field maxSteeringAngle number 
--- @field width number 
--- @field momentOfInertia number 
--- @field radius number 
--- @field tireType int 
--- @field shapeIndex int 
--- @field chassisOffset  
--- @field maxHandBrakeTorque number 
--- @overload fun():phys.WheelCreateInfo
phys.WheelCreateInfo = {}


--- @enum FlagBit
phys.WheelCreateInfo = {
	FLAG_BIT_FRONT = 1,
	FLAG_BIT_LEFT = 4,
	FLAG_BIT_REAR = 2,
	FLAG_BIT_RIGHT = 8,
}

--- @enum FlagNone
phys.WheelCreateInfo = {
	FLAG_NONE = 0,
}

--- 
--- @class phys.ChassisCreateInfo
--- @field momentOfInertia  
--- @field centerOfMass  
--- @overload fun():phys.ChassisCreateInfo
phys.ChassisCreateInfo = {}

--- 
--- @param shapeIndex int
function phys.ChassisCreateInfo:AddShapeIndex(shapeIndex) end

--- 
--- @return table ret0
function phys.ChassisCreateInfo:GetShapeIndices() end


--- 
--- @class phys.Controller: phys.Base
phys.Controller = {}

--- 
--- @return phys.Shape ret0
function phys.Controller:GetGroundShape() end

--- 
function phys.Controller:Remove() end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function phys.Controller:GetGroundTouchPos() end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function phys.Controller:GetFootPos() end

--- 
--- @return number ret0
function phys.Controller:GetStepHeight() end

--- 
--- @param arg1 math.Vector
--- @overload fun(arg1: vector.Vector): 
function phys.Controller:SetDimensions(arg1) end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function phys.Controller:GetDimensions() end

--- 
--- @param arg1 math.Vector
--- @overload fun(arg1: vector.Vector): 
function phys.Controller:AddMoveVelocity(arg1) end

--- 
--- @return phys.CollisionObj ret0
function phys.Controller:GetCollisionObject() end

--- 
--- @param arg1 math.Vector
--- @overload fun(arg1: vector.Vector): 
function phys.Controller:SetFootPos(arg1) end

--- 
--- @param arg1 math.Vector
--- @overload fun(arg1: vector.Vector): 
function phys.Controller:SetUpDirection(arg1) end

--- 
--- @param arg1 math.Vector
--- @overload fun(arg1: vector.Vector): 
function phys.Controller:SetMoveVelocity(arg1) end

--- 
--- @param arg1 number
function phys.Controller:SetSlopeLimit(arg1) end

--- 
--- @param arg1 number
function phys.Controller:SetStepHeight(arg1) end

--- 
--- @param arg1 number
function phys.Controller:Resize(arg1) end

--- 
--- @return phys.ConvexShape ret0
function phys.Controller:GetShape() end

--- 
--- @return enum pragma::physics::IController::CollisionFlags ret0
function phys.Controller:GetCollisionFlags() end

--- 
--- @return number ret0
function phys.Controller:GetSlopeLimit() end

--- 
function phys.Controller:Spawn() end

--- 
--- @return bool ret0
function phys.Controller:IsValid() end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function phys.Controller:GetPos() end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function phys.Controller:GetUpDirection() end

--- 
--- @param arg1 math.Vector
--- @overload fun(arg1: vector.Vector): 
function phys.Controller:SetPos(arg1) end

--- 
--- @param disp math.Vector
--- @overload fun(disp: vector.Vector): 
function phys.Controller:Move(disp) end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function phys.Controller:GetMoveVelocity() end

--- 
--- @return phys.RigidBody ret0
function phys.Controller:GetGroundBody() end

--- 
--- @return phys.Material ret0
function phys.Controller:GetGroundMaterial() end

--- 
--- @return bool ret0
function phys.Controller:IsTouchingGround() end

--- 
--- @return math.Vector ret0
--- @overload fun(): math.Vector
function phys.Controller:GetGroundTouchNormal() end


--- 
--- @class phys.Transform
--- @field pitch  
--- @field yaw  
--- @field roll  
--- @field z  
--- @field x  
--- @field y  
--- @overload fun():phys.Transform
--- @overload fun(arg1: math.Quaternion):phys.Transform
--- @overload fun(arg1: math.Vector):phys.Transform
--- @overload fun(arg1: math.Vector, arg2: math.Quaternion):phys.Transform
--- @overload fun(arg1: math.Mat4):phys.Transform
phys.Transform = {}

--- 
--- @return math.Vector ret0
function phys.Transform:GetForward() end

--- 
--- @return math.Vector ret0
function phys.Transform:GetUp() end

--- 
--- @param 1 math.Quaternion
--- @overload fun(arg1: math.Vector): 
--- @overload fun(arg1: phys.ScaledTransform): 
--- @overload fun(arg1: phys.Transform): 
function phys.Transform:__mul(arg1) end

--- 
--- @return math.Vector ret0
function phys.Transform:GetRight() end

--- 
--- @return bool ret0
function phys.Transform:IsIdentity() end

--- 
--- @return math.Mat4 ret0
function phys.Transform:ToMatrix() end

--- 
--- @param arg1 math.Quaternion
function phys.Transform:SetRotation(arg1) end

--- 
function phys.Transform:SetIdentity() end

--- 
--- @param arg1 math.Vector
function phys.Transform:SetOrigin(arg1) end

--- 
--- @param arg1 math.Vector
function phys.Transform:TranslateGlobal(arg1) end

--- 
--- @param arg1 math.Vector
function phys.Transform:TranslateLocal(arg1) end

--- 
--- @param arg1 math.Quaternion
function phys.Transform:RotateGlobal(arg1) end

--- 
--- @param arg1 math.Quaternion
function phys.Transform:RotateLocal(arg1) end

--- 
--- @param t2 phys.Transform
function phys.Transform:TransformGlobal(t2) end

--- 
--- @param t2 phys.Transform
function phys.Transform:TransformLocal(t2) end

--- 
--- @param m math.Mat4
function phys.Transform:SetMatrix(m) end

--- 
--- @param arg1 phys.Transform
--- @param arg2 number
function phys.Transform:Interpolate(arg1, arg2) end

--- 
--- @param arg1 number
function phys.Transform:InterpolateToIdentity(arg1) end

--- 
--- @return phys.Transform ret0
function phys.Transform:GetInverse() end

--- 
--- @return math.Vector ret0
function phys.Transform:GetOrigin() end

--- 
--- @return phys.Transform ret0
function phys.Transform:Copy() end

--- 
--- @param tOther phys.Transform
function phys.Transform:Set(tOther) end

--- 
--- @param arg1 math.EulerAngles
function phys.Transform:SetAngles(arg1) end

--- 
function phys.Transform:__tostring() end

--- 
--- @return math.EulerAngles ret0
function phys.Transform:GetAngles() end

--- 
--- @return math.Quaternion ret0
function phys.Transform:GetRotation() end


