#include "stdafx_shared.h"
#include "pragma/physics/physconstraint.h"
#include "pragma/physics/physenvironment.h"
#include "pragma/game/game_coordinate_system.hpp"

DEFINE_DERIVED_CHILD_HANDLE(DLLNETWORK,Constraint,PhysConstraint,Constraint,PhysDoFSpringConstraint,DoFSpringConstraint);

PhysDoFSpringConstraint::PhysDoFSpringConstraint(PhysEnv *env,btGeneric6DofSpring2Constraint *constraint)
	: PhysConstraint(env)
{
	m_constraint = constraint;
	m_srcTransform = PhysTransform(constraint->getFrameOffsetA());
	m_tgtTransform = PhysTransform(constraint->getFrameOffsetB());
	Initialize();
}
void PhysDoFSpringConstraint::InitializeLuaObject(lua_State *lua) {PhysConstraint::InitializeLuaObject<DoFSpringConstraintHandle>(lua);}
void PhysDoFSpringConstraint::InitializeHandle() {PhysConstraint::InitializeHandle<DoFSpringConstraintHandle>();}

inline int32_t get_axis_index(PhysDoFSpringConstraint::AxisType type,pragma::Axis axis)
{
	auto r = umath::to_integral(axis);
	if(type == PhysDoFSpringConstraint::AxisType::Angular)
		r += 3u;
	return r;
}
inline RotateOrder get_spring_rotation_order(pragma::RotationOrder order)
{
	switch(order)
	{
		case pragma::RotationOrder::XYZ:
			return RO_XYZ;
		case pragma::RotationOrder::XZY:
			return RO_XZY;
		case pragma::RotationOrder::YXZ:
			return RO_YXZ;
		case pragma::RotationOrder::YZX:
			return RO_YZX;
		case pragma::RotationOrder::ZXY:
			return RO_ZXY;
		case pragma::RotationOrder::ZYX:
			return RO_ZYX;
		default: // Illegal case
			return RotateOrder::RO_XYZ;
	}
}
inline pragma::RotationOrder get_rotation_order(RotateOrder order)
{
	switch(order)
	{
		case RotateOrder::RO_XYZ:
			return pragma::RotationOrder::XYZ;
		case RotateOrder::RO_XZY:
			return pragma::RotationOrder::XZY;
		case RotateOrder::RO_YXZ:
			return pragma::RotationOrder::YXZ;
		case RotateOrder::RO_YZX:
			return pragma::RotationOrder::YZX;
		case RotateOrder::RO_ZXY:
			return pragma::RotationOrder::ZXY;
		case RotateOrder::RO_ZYX:
			return pragma::RotationOrder::ZYX;
		default:
			return pragma::RotationOrder::XYZ;
	}
}

void PhysDoFSpringConstraint::CalculateTransforms()
{
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->calculateTransforms();
}
void PhysDoFSpringConstraint::CalculateTransforms(const PhysTransform &frameA,const PhysTransform &frameB)
{
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->calculateTransforms(frameA.GetTransform(),frameB.GetTransform());
}
btRotationalLimitMotor2 *PhysDoFSpringConstraint::GetRotationalLimitMotor(pragma::Axis index) const
{
	return static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->getRotationalLimitMotor(umath::to_integral(index));
}
btTranslationalLimitMotor2 *PhysDoFSpringConstraint::GetTranslationalLimitMotor() const
{
	return static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->getTranslationalLimitMotor();
}
PhysTransform PhysDoFSpringConstraint::GetCalculatedTransformA() const
{
	return static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->getCalculatedTransformA();
}
PhysTransform PhysDoFSpringConstraint::GetCalculatedTransformB() const
{
	return static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->getCalculatedTransformB();
}
PhysTransform PhysDoFSpringConstraint::GetFrameOffsetA() const
{
	return static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->getFrameOffsetA();
}
PhysTransform PhysDoFSpringConstraint::GetFrameOffsetB() const
{
	return static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->getFrameOffsetB();
}
Vector3 PhysDoFSpringConstraint::GetAxis(pragma::Axis axisIndex) const
{
	return uvec::create(static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->getAxis(umath::to_integral(axisIndex)));
}
double PhysDoFSpringConstraint::GetAngle(pragma::Axis axisIndex) const
{
	return static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->getAngle(umath::to_integral(axisIndex));
}
double PhysDoFSpringConstraint::GetRelativePivotPosition(pragma::Axis axisIndex) const
{
	return static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->getRelativePivotPosition(umath::to_integral(axisIndex));
}
void PhysDoFSpringConstraint::SetFrames(const PhysTransform &frameA,const PhysTransform &frameB)
{
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->setFrames(frameA.GetTransform(),frameB.GetTransform());
}
void PhysDoFSpringConstraint::SetLinearLowerLimit(const Vector3 &linearLower)
{
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->setLinearLowerLimit(uvec::create_bt(linearLower) *PhysEnv::WORLD_SCALE);
}
Vector3 PhysDoFSpringConstraint::GetLinearLowerLimit() const
{
	btVector3 r;
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->getLinearLowerLimit(r);
	return uvec::create(r /PhysEnv::WORLD_SCALE);
}
void PhysDoFSpringConstraint::SetLinearUpperLimit(const Vector3 &linearUpper)
{
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->setLinearUpperLimit(uvec::create_bt(linearUpper) *PhysEnv::WORLD_SCALE);
}
Vector3 PhysDoFSpringConstraint::GetLinearUpperLimit() const
{
	btVector3 r;
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->getLinearUpperLimit(r);
	return uvec::create(r /PhysEnv::WORLD_SCALE);
}
void PhysDoFSpringConstraint::SetAngularLowerLimit(const Vector3 &angularLower)
{
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->setAngularLowerLimit(uvec::create_bt(angularLower));
}
void PhysDoFSpringConstraint::SetAngularLowerLimitReversed(const Vector3 &angularLower)
{
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->setAngularLowerLimitReversed(uvec::create_bt(angularLower));
}
Vector3 PhysDoFSpringConstraint::GetAngularLowerLimit() const
{
	btVector3 r;
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->getAngularLowerLimit(r);
	return uvec::create(r);
}
Vector3 PhysDoFSpringConstraint::GetAngularLowerLimitReversed() const
{
	btVector3 r;
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->getAngularLowerLimitReversed(r);
	return uvec::create(r);
}
void PhysDoFSpringConstraint::SetAngularUpperLimit(const Vector3 &angularUpper)
{
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->setAngularUpperLimit(uvec::create_bt(angularUpper));
}
void PhysDoFSpringConstraint::SetAngularUpperLimitReversed(const Vector3 &angularUpper)
{
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->setAngularUpperLimitReversed(uvec::create_bt(angularUpper));
}
Vector3 PhysDoFSpringConstraint::GetAngularUpperLimit() const
{
	btVector3 r;
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->getAngularUpperLimit(r);
	return uvec::create(r);
}
Vector3 PhysDoFSpringConstraint::GetAngularUpperLimitReversed() const
{
	btVector3 r;
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->getAngularUpperLimitReversed(r);
	return uvec::create(r);
}
void PhysDoFSpringConstraint::SetLimit(AxisType type,pragma::Axis axis,double lo,double hi)
{
	if(type == AxisType::Linear)
	{
		lo *= PhysEnv::WORLD_SCALE;
		hi *= PhysEnv::WORLD_SCALE;
	}
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->setLimit(get_axis_index(type,axis),lo,hi);
}
void PhysDoFSpringConstraint::SetLimitReversed(AxisType type,pragma::Axis axis,double lo,double hi)
{
	if(type == AxisType::Linear)
	{
		lo *= PhysEnv::WORLD_SCALE;
		hi *= PhysEnv::WORLD_SCALE;
	}
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->setLimitReversed(get_axis_index(type,axis),lo,hi);
}
bool PhysDoFSpringConstraint::IsLimited(AxisType type,pragma::Axis axis) const
{
	return static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->isLimited(get_axis_index(type,axis));
}
void PhysDoFSpringConstraint::SetRotationOrder(pragma::RotationOrder order)
{
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->setRotationOrder(get_spring_rotation_order(order));
}
pragma::RotationOrder PhysDoFSpringConstraint::GetRotationOrder() const
{
	return get_rotation_order(static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->getRotationOrder());
}
void PhysDoFSpringConstraint::SetAxis(const Vector3 &axis1,const Vector3 &axis2)
{
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->setAxis(uvec::create_bt(axis1),uvec::create_bt(axis2));
}
void PhysDoFSpringConstraint::SetBounce(AxisType type,pragma::Axis axis,double bounce)
{
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->setBounce(get_axis_index(type,axis),bounce);
}
void PhysDoFSpringConstraint::EnableMotor(AxisType type,pragma::Axis axis,bool onOff)
{
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->enableMotor(get_axis_index(type,axis),onOff);
}
void PhysDoFSpringConstraint::SetServo(AxisType type,pragma::Axis axis,bool onOff)
{
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->setServo(get_axis_index(type,axis),onOff);
}
void PhysDoFSpringConstraint::SetTargetVelocity(AxisType type,pragma::Axis axis,double velocity)
{
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->setTargetVelocity(get_axis_index(type,axis),velocity);
}
void PhysDoFSpringConstraint::SetServoTarget(AxisType type,pragma::Axis axis,double target)
{
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->setServoTarget(get_axis_index(type,axis),target);
}
void PhysDoFSpringConstraint::SetMaxMotorForce(AxisType type,pragma::Axis axis,double force)
{
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->setMaxMotorForce(get_axis_index(type,axis),force);
}
void PhysDoFSpringConstraint::EnableSpring(AxisType type,pragma::Axis axis,bool onOff)
{
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->enableSpring(get_axis_index(type,axis),onOff);
}
void PhysDoFSpringConstraint::SetStiffness(AxisType type,pragma::Axis axis,double stiffness,bool limitIfNeeded)
{
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->setStiffness(get_axis_index(type,axis),stiffness,limitIfNeeded);
}
void PhysDoFSpringConstraint::SetDamping(AxisType type,pragma::Axis axis,double damping,bool limitIfNeeded)
{
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->setDamping(get_axis_index(type,axis),damping,limitIfNeeded);
}
void PhysDoFSpringConstraint::SetEquilibriumPoint()
{
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->setEquilibriumPoint();
}
void PhysDoFSpringConstraint::SetEquilibriumPoint(AxisType type,pragma::Axis axis)
{
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->setEquilibriumPoint(get_axis_index(type,axis));
}
void PhysDoFSpringConstraint::SetEquilibriumPoint(AxisType type,pragma::Axis axis,double val)
{
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->setEquilibriumPoint(get_axis_index(type,axis),val);
}
void PhysDoFSpringConstraint::SetERP(AxisType type,pragma::Axis axis,double value)
{
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->setParam(BT_CONSTRAINT_ERP,value,get_axis_index(type,axis));
}
double PhysDoFSpringConstraint::GetERP(AxisType type,pragma::Axis axis) const
{
	return static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->getParam(BT_CONSTRAINT_ERP,get_axis_index(type,axis));
}
void PhysDoFSpringConstraint::SetStopERP(AxisType type,pragma::Axis axis,double value)
{
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->setParam(BT_CONSTRAINT_STOP_ERP,value,get_axis_index(type,axis));
}
double PhysDoFSpringConstraint::GetStopERP(AxisType type,pragma::Axis axis) const
{
	return static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->getParam(BT_CONSTRAINT_STOP_ERP,get_axis_index(type,axis));
}
void PhysDoFSpringConstraint::SetCFM(AxisType type,pragma::Axis axis,double value)
{
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->setParam(BT_CONSTRAINT_CFM,value,get_axis_index(type,axis));
}
double PhysDoFSpringConstraint::GetCFM(AxisType type,pragma::Axis axis) const
{
	return static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->getParam(BT_CONSTRAINT_CFM,get_axis_index(type,axis));
}
void PhysDoFSpringConstraint::SetStopCFM(AxisType type,pragma::Axis axis,double value)
{
	static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->setParam(BT_CONSTRAINT_STOP_CFM,value,get_axis_index(type,axis));
}
double PhysDoFSpringConstraint::GetStopCFM(AxisType type,pragma::Axis axis) const
{
	return static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->getParam(BT_CONSTRAINT_STOP_CFM,get_axis_index(type,axis));
}
bool PhysDoFSpringConstraint::MatrixToEulerXYZ(const btMatrix3x3& mat,btVector3& xyz) const
{
	return static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->matrixToEulerXYZ(mat,xyz);
}
bool PhysDoFSpringConstraint::MatrixToEulerXZY(const btMatrix3x3& mat,btVector3& xyz) const
{
	return static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->matrixToEulerXZY(mat,xyz);
}
bool PhysDoFSpringConstraint::MatrixToEulerYXZ(const btMatrix3x3& mat,btVector3& xyz) const
{
	return static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->matrixToEulerYXZ(mat,xyz);
}
bool PhysDoFSpringConstraint::MatrixToEulerYZX(const btMatrix3x3& mat,btVector3& xyz) const
{
	return static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->matrixToEulerYZX(mat,xyz);
}
bool PhysDoFSpringConstraint::MatrixToEulerZXY(const btMatrix3x3& mat,btVector3& xyz) const
{
	return static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->matrixToEulerZXY(mat,xyz);
}
bool PhysDoFSpringConstraint::MatrixToEulerZYX(const btMatrix3x3& mat,btVector3& xyz) const
{
	return static_cast<btGeneric6DofSpring2Constraint*>(m_constraint)->matrixToEulerZYX(mat,xyz);
}
