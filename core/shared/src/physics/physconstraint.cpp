#include "stdafx_shared.h"
#include "pragma/physics/physenvironment.h"
#include "pragma/physics/physconstraint.h"

DEFINE_BASE_HANDLE(DLLNETWORK,PhysConstraint,Constraint);

PhysConstraint::PhysConstraint(PhysEnv *env)
	: PhysBase(env),LuaObj<ConstraintHandle>(),m_bCollisionsEnabled(true)
{
	m_handle = new ConstraintHandle(this);
}

void PhysConstraint::Initialize()
{
	auto *world = m_physEnv->GetWorld();
	world->removeConstraint(m_constraint);
	world->addConstraint(m_constraint);
	InitializeHandle();
	auto *nwState = m_physEnv->GetNetworkState();
	InitializeLuaObject(nwState->GetLuaState());
}

void PhysConstraint::Remove()
{
	delete this;
}

void PhysConstraint::InitializeHandle()
{
	PhysConstraint::InitializeHandle<ConstraintHandle>();
}

PhysTransform &PhysConstraint::GetSourceTransform() {return m_srcTransform;}
PhysTransform &PhysConstraint::GetTargetTransform() {return m_tgtTransform;}
Vector3 PhysConstraint::GetSourcePosition() {return m_srcTransform.GetOrigin();}
Quat PhysConstraint::GetSourceRotation() {return m_srcTransform.GetRotation();}
Vector3 PhysConstraint::GetTargetPosition() {return m_tgtTransform.GetOrigin();}
Quat PhysConstraint::GetTargetRotation() {return m_tgtTransform.GetRotation();}

PhysRigidBody *PhysConstraint::GetSourceObject()
{
	auto &bodyA = m_constraint->getRigidBodyA();
	return static_cast<PhysRigidBody*>(static_cast<PhysCollisionObject*>(bodyA.getUserPointer()));
}
PhysRigidBody *PhysConstraint::GetTargetObject()
{
	auto &bodyB = m_constraint->getRigidBodyB();
	return static_cast<PhysRigidBody*>(static_cast<PhysCollisionObject*>(bodyB.getUserPointer()));
}

void PhysConstraint::SetCollisionsEnabled(Bool b)
{
	if(b == m_bCollisionsEnabled)
		return;
	m_bCollisionsEnabled = b;
	auto &bodyA = m_constraint->getRigidBodyA();
	auto &bodyB = m_constraint->getRigidBodyB();
	if(b == false)
	{
		bodyA.addConstraintRef(m_constraint);
		bodyB.addConstraintRef(m_constraint);
		return;
	}
	bodyA.removeConstraintRef(m_constraint);
	bodyB.removeConstraintRef(m_constraint);
}

void PhysConstraint::EnableCollisions()
{
	SetCollisionsEnabled(true);
}

void PhysConstraint::DisableCollisions()
{
	SetCollisionsEnabled(false);
}

Bool PhysConstraint::GetCollisionsEnabled() const {return m_bCollisionsEnabled;}

PhysConstraint::~PhysConstraint()
{
	m_physEnv->RemoveConstraint(this);
	delete m_constraint;
}

void PhysConstraint::SetEnabled(bool b) {m_constraint->setEnabled(b);}
bool PhysConstraint::IsEnabled() const {return m_constraint->isEnabled();}

ConstraintHandle *PhysConstraint::CreateHandle() {return m_handle->Copy();}
ConstraintHandle PhysConstraint::GetHandle() {return *m_handle;}

btTypedConstraint *PhysConstraint::GetConstraint() {return m_constraint;}

void PhysConstraint::SetOverrideSolverIterationCount(int32_t count) {m_constraint->setOverrideNumSolverIterations(count);}
int32_t PhysConstraint::GetOverrideSolverIterationCount() const {return m_constraint->getOverrideNumSolverIterations();}
float PhysConstraint::GetBreakingImpulseThreshold() const {return m_constraint->getBreakingImpulseThreshold() /PhysEnv::WORLD_SCALE;}
void PhysConstraint::SetBreakingImpulseThreshold(float threshold) {m_constraint->setBreakingImpulseThreshold(threshold *PhysEnv::WORLD_SCALE);}

////////////////////////////

DEFINE_DERIVED_CHILD_HANDLE(DLLNETWORK,Constraint,PhysConstraint,Constraint,PhysFixedConstraint,FixedConstraint);

PhysFixedConstraint::PhysFixedConstraint(PhysEnv *env,btFixedConstraint *constraint)
	: PhysConstraint(env)
{
	m_constraint = constraint;
	m_srcTransform = PhysTransform(constraint->getFrameOffsetA());
	m_tgtTransform = PhysTransform(constraint->getFrameOffsetB());
	Initialize();
}
void PhysFixedConstraint::InitializeLuaObject(lua_State *lua) {PhysConstraint::InitializeLuaObject<FixedConstraintHandle>(lua);}
void PhysFixedConstraint::InitializeHandle() {PhysConstraint::InitializeHandle<FixedConstraintHandle>();}

////////////////////////////

DEFINE_DERIVED_CHILD_HANDLE(DLLNETWORK,Constraint,PhysConstraint,Constraint,PhysBallSocket,BallSocketConstraint);

PhysBallSocket::PhysBallSocket(PhysEnv *env,btPoint2PointConstraint *constraint)
	: PhysConstraint(env)
{
	m_constraint = constraint;
	Initialize();
}
void PhysBallSocket::InitializeLuaObject(lua_State *lua) {PhysConstraint::InitializeLuaObject<BallSocketConstraintHandle>(lua);}
void PhysBallSocket::InitializeHandle() {PhysConstraint::InitializeHandle<BallSocketConstraintHandle>();}

////////////////////////////

DEFINE_DERIVED_CHILD_HANDLE(DLLNETWORK,Constraint,PhysConstraint,Constraint,PhysHinge,HingeConstraint);

PhysHinge::PhysHinge(PhysEnv *env,btHingeConstraint *constraint)
	: PhysConstraint(env)
{
	m_constraint = constraint;
	Initialize();
}
void PhysHinge::InitializeLuaObject(lua_State *lua) {PhysConstraint::InitializeLuaObject<HingeConstraintHandle>(lua);}
void PhysHinge::InitializeHandle() {PhysConstraint::InitializeHandle<HingeConstraintHandle>();}

void PhysHinge::SetLimit(float low,float high,float softness,float biasFactor,float relaxationFactor)
{
	static_cast<btHingeConstraint*>(m_constraint)->setLimit(low,high,softness,biasFactor,relaxationFactor);
}

////////////////////////////

DEFINE_DERIVED_CHILD_HANDLE(DLLNETWORK,Constraint,PhysConstraint,Constraint,PhysSlider,SliderConstraint);

PhysSlider::PhysSlider(PhysEnv *env,btSliderConstraint *constraint)
	: PhysConstraint(env)
{
	m_constraint = constraint;
	Initialize();
}
void PhysSlider::InitializeLuaObject(lua_State *lua) {PhysConstraint::InitializeLuaObject<SliderConstraintHandle>(lua);}
void PhysSlider::InitializeHandle() {PhysConstraint::InitializeHandle<SliderConstraintHandle>();}

////////////////////////////

DEFINE_DERIVED_CHILD_HANDLE(DLLNETWORK,Constraint,PhysConstraint,Constraint,PhysConeTwist,ConeTwistConstraint);

PhysConeTwist::PhysConeTwist(PhysEnv *env,btConeTwistConstraint *constraint)
	: PhysConstraint(env)
{
	m_constraint = constraint;
	m_srcTransform = PhysTransform(constraint->getAFrame());
	m_tgtTransform = PhysTransform(constraint->getBFrame());
	Initialize();
}
void PhysConeTwist::InitializeLuaObject(lua_State *lua) {PhysConstraint::InitializeLuaObject<ConeTwistConstraintHandle>(lua);}
void PhysConeTwist::InitializeHandle() {PhysConstraint::InitializeHandle<ConeTwistConstraintHandle>();}
void PhysConeTwist::SetLimit(float swingSpan1,float swingSpan2,float twistSpan,float softness,float biasFactor,float relaxationFactor)
{
	static_cast<btConeTwistConstraint*>(m_constraint)->setLimit(swingSpan1,swingSpan2,twistSpan,softness,biasFactor,relaxationFactor);
}

////////////////////////////

DEFINE_DERIVED_CHILD_HANDLE(DLLNETWORK,Constraint,PhysConstraint,Constraint,PhysDoF,DoFConstraint);

PhysDoF::PhysDoF(PhysEnv *env,btGeneric6DofConstraint *constraint)
	: PhysConstraint(env)
{
	m_constraint = constraint;
	Initialize();
}
void PhysDoF::InitializeLuaObject(lua_State *lua) {PhysConstraint::InitializeLuaObject<DoFConstraintHandle>(lua);}
void PhysDoF::InitializeHandle() {PhysConstraint::InitializeHandle<DoFConstraintHandle>();}

void PhysDoF::SetLinearLimit(const Vector3 &lower,const Vector3 &upper)
{
	SetLinearLowerLimit(lower);
	SetLinearUpperLimit(upper);
}
void PhysDoF::SetLinearLimit(const Vector3 &lim) {SetLinearLimit(-lim,lim);}
void PhysDoF::SetLinearLowerLimit(const Vector3 &lim)
{
	static_cast<btGeneric6DofConstraint*>(m_constraint)->setLinearLowerLimit(uvec::create_bt(lim) *PhysEnv::WORLD_SCALE);
}
void PhysDoF::SetLinearUpperLimit(const Vector3 &lim)
{
	static_cast<btGeneric6DofConstraint*>(m_constraint)->setLinearUpperLimit(uvec::create_bt(lim) *PhysEnv::WORLD_SCALE);
}
void PhysDoF::SetAngularLimit(const EulerAngles &lower,const EulerAngles &upper)
{
	SetAngularLowerLimit(lower);
	SetAngularUpperLimit(upper);
}
void PhysDoF::SetAngularLimit(const EulerAngles &lim) {SetAngularLimit(-lim,lim);}
void PhysDoF::SetAngularLowerLimit(const EulerAngles &lim)
{
	static_cast<btGeneric6DofConstraint*>(m_constraint)->setAngularLowerLimit(btVector3(umath::deg_to_rad(lim.p),umath::deg_to_rad(lim.y),umath::deg_to_rad(lim.r)));
}
void PhysDoF::SetAngularUpperLimit(const EulerAngles &lim)
{
	static_cast<btGeneric6DofConstraint*>(m_constraint)->setAngularUpperLimit(btVector3(umath::deg_to_rad(lim.p),umath::deg_to_rad(lim.y),umath::deg_to_rad(lim.r)));
}

Vector3 PhysDoF::GetLinearLowerLimit() const
{
	btVector3 lowerLimit;
	static_cast<btGeneric6DofConstraint*>(m_constraint)->getLinearLowerLimit(lowerLimit);
	return uvec::create(lowerLimit /PhysEnv::WORLD_SCALE);
}
Vector3 PhysDoF::GetlinearUpperLimit() const
{
	btVector3 upperLimit;
	static_cast<btGeneric6DofConstraint*>(m_constraint)->getLinearUpperLimit(upperLimit);
	return uvec::create(upperLimit /PhysEnv::WORLD_SCALE);
}
EulerAngles PhysDoF::GetAngularLowerLimit() const
{
	btVector3 lowerLimit;
	static_cast<btGeneric6DofConstraint*>(m_constraint)->getAngularLowerLimit(lowerLimit);
	return EulerAngles(umath::rad_to_deg(lowerLimit.x()),umath::rad_to_deg(lowerLimit.y()),umath::rad_to_deg(lowerLimit.z()));
}
EulerAngles PhysDoF::GetAngularUpperLimit() const
{
	btVector3 upperLimit;
	static_cast<btGeneric6DofConstraint*>(m_constraint)->getAngularUpperLimit(upperLimit);
	return EulerAngles(umath::rad_to_deg(upperLimit.x()),umath::rad_to_deg(upperLimit.y()),umath::rad_to_deg(upperLimit.z()));
}

Vector3 PhysDoF::GetAngularTargetVelocity() const
{
	Vector3 r {};
	for(uint8_t axis=0;axis<3;++axis)
	{
		auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getRotationalLimitMotor(axis);
		if(motor == nullptr)
			continue;
		r[axis] = motor->m_targetVelocity;
	}
	return r;
}
Vector3 PhysDoF::GetAngularMaxMotorForce() const
{
	Vector3 r {};
	for(uint8_t axis=0;axis<3;++axis)
	{
		auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getRotationalLimitMotor(axis);
		if(motor == nullptr)
			continue;
		r[axis] = motor->m_maxMotorForce;
	}
	return r;
}
void PhysDoF::SetAngularMaxMotorForce(const Vector3 &force)
{
	for(uint8_t axis=0;axis<3;++axis)
	{
		auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getRotationalLimitMotor(axis);
		if(motor == nullptr)
			continue;
		motor->m_maxMotorForce = force[axis];
	}
}
Vector3 PhysDoF::GetAngularMaxLimitForce() const
{
	Vector3 r {};
	for(uint8_t axis=0;axis<3;++axis)
	{
		auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getRotationalLimitMotor(axis);
		if(motor == nullptr)
			continue;
		r[axis] = motor->m_maxLimitForce;
	}
	return r;
}
void PhysDoF::SetAngularMaxLimitForce(const Vector3 &force)
{
	for(uint8_t axis=0;axis<3;++axis)
	{
		auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getRotationalLimitMotor(axis);
		if(motor == nullptr)
			continue;
		motor->m_maxLimitForce = force[axis];
	}
}
Vector3 PhysDoF::GetAngularDamping() const
{
	Vector3 r {};
	for(uint8_t axis=0;axis<3;++axis)
	{
		auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getRotationalLimitMotor(axis);
		if(motor == nullptr)
			continue;
		r[axis] = motor->m_damping;
	}
	return r;
}
void PhysDoF::SetAngularDamping(const Vector3 &damping)
{
	for(uint8_t axis=0;axis<3;++axis)
	{
		auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getRotationalLimitMotor(axis);
		if(motor == nullptr)
			continue;
		motor->m_damping = damping[axis];
	}
}
Vector3 PhysDoF::GetAngularLimitSoftness() const
{
	Vector3 r {};
	for(uint8_t axis=0;axis<3;++axis)
	{
		auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getRotationalLimitMotor(axis);
		if(motor == nullptr)
			continue;
		r[axis] = motor->m_limitSoftness;
	}
	return r;
}
void PhysDoF::SetAngularLimitSoftness(const Vector3 &softness)
{
	for(uint8_t axis=0;axis<3;++axis)
	{
		auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getRotationalLimitMotor(axis);
		if(motor == nullptr)
			continue;
		motor->m_limitSoftness = softness[axis];
	}
}
Vector3 PhysDoF::GetAngularForceMixingFactor() const
{
	Vector3 r {};
	for(uint8_t axis=0;axis<3;++axis)
	{
		auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getRotationalLimitMotor(axis);
		if(motor == nullptr)
			continue;
		r[axis] = motor->m_normalCFM;
	}
	return r;
}
void PhysDoF::SetAngularForceMixingFactor(const Vector3 &factor)
{
	for(uint8_t axis=0;axis<3;++axis)
	{
		auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getRotationalLimitMotor(axis);
		if(motor == nullptr)
			continue;
		motor->m_normalCFM = factor[axis];
	}
}
Vector3 PhysDoF::GetAngularLimitErrorTolerance() const
{
	Vector3 r {};
	for(uint8_t axis=0;axis<3;++axis)
	{
		auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getRotationalLimitMotor(axis);
		if(motor == nullptr)
			continue;
		r[axis] = motor->m_stopERP;
	}
	return r;
}
void PhysDoF::SetAngularLimitErrorTolerance(const Vector3 &tolerance)
{
	for(uint8_t axis=0;axis<3;++axis)
	{
		auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getRotationalLimitMotor(axis);
		if(motor == nullptr)
			continue;
		motor->m_stopERP = tolerance[axis];
	}
}
Vector3 PhysDoF::GetAngularLimitForceMixingFactor() const
{
	Vector3 r {};
	for(uint8_t axis=0;axis<3;++axis)
	{
		auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getRotationalLimitMotor(axis);
		if(motor == nullptr)
			continue;
		r[axis] = motor->m_stopCFM;
	}
	return r;
}
void PhysDoF::SetAngularLimitForceMixingFactor(const Vector3 &factor)
{
	for(uint8_t axis=0;axis<3;++axis)
	{
		auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getRotationalLimitMotor(axis);
		if(motor == nullptr)
			continue;
		motor->m_stopCFM = factor[axis];
	}
}
Vector3 PhysDoF::GetAngularRestitutionFactor() const
{
	Vector3 r {};
	for(uint8_t axis=0;axis<3;++axis)
	{
		auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getRotationalLimitMotor(axis);
		if(motor == nullptr)
			continue;
		r[axis] = motor->m_bounce;
	}
	return r;
}
void PhysDoF::SetAngularRestitutionFactor(const Vector3 &factor)
{
	for(uint8_t axis=0;axis<3;++axis)
	{
		auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getRotationalLimitMotor(axis);
		if(motor == nullptr)
			continue;
		motor->m_bounce = factor[axis];
	}
}
bool PhysDoF::IsAngularMotorEnabled(uint8_t axis) const
{
	auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getRotationalLimitMotor(axis);
	if(motor == nullptr)
		return 0.f;
	return motor->m_enableMotor;
}
void PhysDoF::SetAngularMotorEnabled(uint8_t axis,bool bEnabled)
{
	auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getRotationalLimitMotor(axis);
	if(motor == nullptr)
		return;
	motor->m_enableMotor = bEnabled;
}
Vector3 PhysDoF::GetCurrentAngularLimitError() const
{
	Vector3 r {};
	for(uint8_t axis=0;axis<3;++axis)
	{
		auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getRotationalLimitMotor(axis);
		if(motor == nullptr)
			continue;
		r[axis] = motor->m_currentLimitError;
	}
	return r;
}
Vector3 PhysDoF::GetCurrentAngularPosition() const
{
	Vector3 r {};
	for(uint8_t axis=0;axis<3;++axis)
	{
		auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getRotationalLimitMotor(axis);
		if(motor == nullptr)
			continue;
		r[axis] = motor->m_currentPosition;
	}
	return r;
}
Vector3i PhysDoF::GetCurrentAngularLimit() const
{
	Vector3 r {};
	for(uint8_t axis=0;axis<3;++axis)
	{
		auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getRotationalLimitMotor(axis);
		if(motor == nullptr)
			continue;
		r[axis] = motor->m_currentLimit;
	}
	return r;
}
Vector3 PhysDoF::GetCurrentAngularAccumulatedImpulse() const
{
	Vector3 r {};
	for(uint8_t axis=0;axis<3;++axis)
	{
		auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getRotationalLimitMotor(axis);
		if(motor == nullptr)
			continue;
		r[axis] = motor->m_accumulatedImpulse /PhysEnv::WORLD_SCALE;
	}
	return r;
}

Vector3 PhysDoF::GetLinearTargetVelocity() const
{
	auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getTranslationalLimitMotor();
	if(motor == nullptr)
		return {};
	return uvec::create(motor->m_targetVelocity /PhysEnv::WORLD_SCALE);
}
Vector3 PhysDoF::GetLinearMaxMotorForce() const
{
	auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getTranslationalLimitMotor();
	if(motor == nullptr)
		return {};
	return uvec::create(motor->m_maxMotorForce /PhysEnv::WORLD_SCALE);
}
void PhysDoF::SetLinearMaxMotorForce(const Vector3 &force)
{
	auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getTranslationalLimitMotor();
	if(motor == nullptr)
		return;
	motor->m_maxMotorForce = uvec::create_bt(force) *PhysEnv::WORLD_SCALE;
}
float PhysDoF::GetLinearDamping() const
{
	auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getTranslationalLimitMotor();
	if(motor == nullptr)
		return 0.f;
	return motor->m_damping;
}
void PhysDoF::SetLinearDamping(float damping)
{
	auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getTranslationalLimitMotor();
	if(motor == nullptr)
		return;
	motor->m_damping = damping;
}
float PhysDoF::GetLinearLimitSoftness() const
{
	auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getTranslationalLimitMotor();
	if(motor == nullptr)
		return 0.f;
	return motor->m_limitSoftness;
}
void PhysDoF::SetLinearLimitSoftness(float softness)
{
	auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getTranslationalLimitMotor();
	if(motor == nullptr)
		return;
	motor->m_limitSoftness = softness;
}
Vector3 PhysDoF::GetLinearForceMixingFactor() const
{
	auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getTranslationalLimitMotor();
	if(motor == nullptr)
		return {};
	return uvec::create(motor->m_normalCFM /PhysEnv::WORLD_SCALE);
}
void PhysDoF::SetLinearForceMixingFactor(const Vector3 &factor)
{
	auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getTranslationalLimitMotor();
	if(motor == nullptr)
		return;
	motor->m_normalCFM = uvec::create_bt(factor) *PhysEnv::WORLD_SCALE;
}
Vector3 PhysDoF::GetLinearLimitErrorTolerance() const
{
	auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getTranslationalLimitMotor();
	if(motor == nullptr)
		return {};
	return uvec::create(motor->m_stopERP /PhysEnv::WORLD_SCALE);
}
void PhysDoF::SetLinearLimitErrorTolerance(const Vector3 &tolerance)
{
	auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getTranslationalLimitMotor();
	if(motor == nullptr)
		return;
	motor->m_stopERP = uvec::create_bt(tolerance) *PhysEnv::WORLD_SCALE;
}
Vector3 PhysDoF::GetLinearLimitForceMixingFactor() const
{
	auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getTranslationalLimitMotor();
	if(motor == nullptr)
		return {};
	return uvec::create(motor->m_stopCFM /PhysEnv::WORLD_SCALE);
}
void PhysDoF::SetLinearLimitForceMixingFactor(const Vector3 &factor)
{
	auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getTranslationalLimitMotor();
	if(motor == nullptr)
		return;
	motor->m_stopCFM = uvec::create_bt(factor) *PhysEnv::WORLD_SCALE;
}
float PhysDoF::GetLinearRestitutionFactor() const
{
	auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getTranslationalLimitMotor();
	if(motor == nullptr)
		return 0.f;
	return motor->m_restitution;
}
void PhysDoF::SetLinearRestitutionFactor(float factor)
{
	auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getTranslationalLimitMotor();
	if(motor == nullptr)
		return;
	motor->m_restitution = factor;
}
bool PhysDoF::IsLinearMotorEnabled(uint8_t axis) const
{
	auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getTranslationalLimitMotor();
	if(motor == nullptr || axis > 2)
		return false;
	return motor->m_enableMotor[axis];
}
void PhysDoF::SetLinearMotorEnabled(uint8_t axis,bool bEnabled)
{
	auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getTranslationalLimitMotor();
	if(motor == nullptr || axis > 2)
		return;
	motor->m_enableMotor[axis] = bEnabled;
}
Vector3 PhysDoF::GetCurrentLinearDifference() const
{
	auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getTranslationalLimitMotor();
	if(motor == nullptr)
		return {};
	return uvec::create(motor->m_currentLinearDiff /PhysEnv::WORLD_SCALE);
}
Vector3 PhysDoF::GetCurrentLinearLimitError() const
{
	auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getTranslationalLimitMotor();
	if(motor == nullptr)
		return {};
	return uvec::create(motor->m_currentLimitError);
}
Vector3i PhysDoF::GetCurrentLinearLimit() const
{
	auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getTranslationalLimitMotor();
	if(motor == nullptr)
		return {};
	return {motor->m_currentLimit[0],motor->m_currentLimit[1],motor->m_currentLimit[2]};
}
Vector3 PhysDoF::GetCurrentLinearAccumulatedImpulse() const
{
	auto *motor = static_cast<btGeneric6DofConstraint*>(m_constraint)->getTranslationalLimitMotor();
	if(motor == nullptr)
		return {};
	return uvec::create(motor->m_currentLinearDiff /PhysEnv::WORLD_SCALE);
}
