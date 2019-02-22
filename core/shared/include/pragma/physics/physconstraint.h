#ifndef __PHYSCONSTRAINT_H__
#define __PHYSCONSTRAINT_H__

#include "pragma/networkdefinitions.h"
#include <memory>
#include <sharedutils/def_handle.h>
#include <pragma/physics/physapi.h>
#include "pragma/physics/physbase.h"
#include "pragma/physics/phystransform.h"
#include "pragma/lua/baseluaobj.h"
#include <vector>

class PhysEnv;
class PhysRigidBody;

class PhysConstraint;
DECLARE_BASE_HANDLE(DLLNETWORK,PhysConstraint,Constraint);

class DLLNETWORK PhysConstraint
	: public PhysBase,public LuaObj<ConstraintHandle>
{
public:
	friend PhysEnv;
protected:
	PhysConstraint(PhysEnv *env);
	virtual ~PhysConstraint() override;
	btTypedConstraint *m_constraint;
	Bool m_bCollisionsEnabled;
	PhysTransform m_srcTransform;
	PhysTransform m_tgtTransform;
	void Initialize();
	virtual void InitializeHandle() override;
	template<class THandle>
		void InitializeHandle();
public:
	btTypedConstraint *GetConstraint();
	ConstraintHandle *CreateHandle();
	ConstraintHandle GetHandle();
	void SetEnabled(bool b);
	bool IsEnabled() const;
	void SetCollisionsEnabled(Bool b);
	Bool GetCollisionsEnabled() const;
	void EnableCollisions();
	void DisableCollisions();
	PhysRigidBody *GetSourceObject();
	PhysRigidBody *GetTargetObject();
	PhysTransform &GetSourceTransform();
	PhysTransform &GetTargetTransform();
	Vector3 GetSourcePosition();
	Quat GetSourceRotation();
	Vector3 GetTargetPosition();
	Quat GetTargetRotation();
	void Remove();

	void SetOverrideSolverIterationCount(int32_t count);
	int32_t GetOverrideSolverIterationCount() const;
	float GetBreakingImpulseThreshold() const;
	void SetBreakingImpulseThreshold(float threshold);
};

template<class THandle>
	void PhysConstraint::InitializeHandle()
{
	m_handle = new THandle(new PtrConstraint(this));
}

class DLLNETWORK PhysDoFSpringConstraint;
DECLARE_DERIVED_CHILD_HANDLE(DLLNETWORK,Constraint,PhysConstraint,Constraint,PhysDoFSpringConstraint,DoFSpringConstraint);

namespace pragma
{
	enum class Axis : uint8_t;
	enum class RotationOrder : uint16_t;
};
class DLLNETWORK PhysDoFSpringConstraint
	: public PhysConstraint
{
public:
	friend PhysEnv;
protected:
	PhysDoFSpringConstraint(PhysEnv *env,btGeneric6DofSpring2Constraint *constraint);
	virtual void InitializeHandle() override;
public:
	virtual void InitializeLuaObject(lua_State *lua) override;

	enum class AxisType : uint8_t
	{
		Linear = 0u,
		Angular
	};

	void CalculateTransforms();
	void CalculateTransforms(const PhysTransform &frameA,const PhysTransform &frameB);
	btRotationalLimitMotor2 *GetRotationalLimitMotor(pragma::Axis index) const;
	btTranslationalLimitMotor2 *GetTranslationalLimitMotor() const;
	PhysTransform GetCalculatedTransformA() const;
	PhysTransform GetCalculatedTransformB() const;
	PhysTransform GetFrameOffsetA() const;
	PhysTransform GetFrameOffsetB() const;
	Vector3 GetAxis(pragma::Axis axisIndex) const;
	double GetAngle(pragma::Axis axisIndex) const;
	double GetRelativePivotPosition(pragma::Axis axisIndex) const;
	void SetFrames(const PhysTransform &frameA,const PhysTransform &frameB);
	void SetLinearLowerLimit(const Vector3 &linearLower);
	Vector3 GetLinearLowerLimit() const;
	void SetLinearUpperLimit(const Vector3 &linearUpper);
	Vector3 GetLinearUpperLimit() const;
	void SetAngularLowerLimit(const Vector3 &angularLower);
	void SetAngularLowerLimitReversed(const Vector3 &angularLower);
	Vector3 GetAngularLowerLimit() const;
	Vector3 GetAngularLowerLimitReversed() const;
	void SetAngularUpperLimit(const Vector3 &angularUpper);
	void SetAngularUpperLimitReversed(const Vector3 &angularUpper);
	Vector3 GetAngularUpperLimit() const;
	Vector3 GetAngularUpperLimitReversed() const;
	void SetLimit(AxisType type,pragma::Axis axis,double lo,double hi);
	void SetLimitReversed(AxisType type,pragma::Axis axis,double lo,double hi);
	bool IsLimited(AxisType type,pragma::Axis axis) const;
	void SetRotationOrder(pragma::RotationOrder order);
	pragma::RotationOrder GetRotationOrder() const;
	void SetAxis(const Vector3 &axis1,const Vector3 &axis2);
	void SetBounce(AxisType type,pragma::Axis axis,double bounce);
	void EnableMotor(AxisType type,pragma::Axis axis,bool onOff);
	void SetServo(AxisType type,pragma::Axis axis,bool onOff);
	void SetTargetVelocity(AxisType type,pragma::Axis axis,double velocity);
	void SetServoTarget(AxisType type,pragma::Axis axis,double target);
	void SetMaxMotorForce(AxisType type,pragma::Axis axis,double force);
	void EnableSpring(AxisType type,pragma::Axis axis,bool onOff);
	void SetStiffness(AxisType type,pragma::Axis axis,double stiffness,bool limitIfNeeded=true);
	void SetDamping(AxisType type,pragma::Axis axis,double damping,bool limitIfNeeded=true);
	void SetEquilibriumPoint();
	void SetEquilibriumPoint(AxisType type,pragma::Axis axis);
	void SetEquilibriumPoint(AxisType type,pragma::Axis axis,double val);

	void SetERP(AxisType type,pragma::Axis axis,double value);
	double GetERP(AxisType type,pragma::Axis axis) const;
	void SetStopERP(AxisType type,pragma::Axis axis,double value);
	double GetStopERP(AxisType type,pragma::Axis axis) const;
	void SetCFM(AxisType type,pragma::Axis axis,double value);
	double GetCFM(AxisType type,pragma::Axis axis) const;
	void SetStopCFM(AxisType type,pragma::Axis axis,double value);
	double GetStopCFM(AxisType type,pragma::Axis axis) const;

	bool MatrixToEulerXYZ(const btMatrix3x3& mat,btVector3& xyz) const;
	bool MatrixToEulerXZY(const btMatrix3x3& mat,btVector3& xyz) const;
	bool MatrixToEulerYXZ(const btMatrix3x3& mat,btVector3& xyz) const;
	bool MatrixToEulerYZX(const btMatrix3x3& mat,btVector3& xyz) const;
	bool MatrixToEulerZXY(const btMatrix3x3& mat,btVector3& xyz) const;
	bool MatrixToEulerZYX(const btMatrix3x3& mat,btVector3& xyz) const;
};

class DLLNETWORK PhysFixedConstraint;
DECLARE_DERIVED_CHILD_HANDLE(DLLNETWORK,Constraint,PhysConstraint,Constraint,PhysFixedConstraint,FixedConstraint);

class DLLNETWORK PhysFixedConstraint
	: public PhysConstraint
{
public:
	friend PhysEnv;
protected:
	PhysFixedConstraint(PhysEnv *env,btFixedConstraint *constraint);
	virtual void InitializeHandle() override;
public:
	virtual void InitializeLuaObject(lua_State *lua) override;
};

class DLLNETWORK PhysBallSocket;
DECLARE_DERIVED_CHILD_HANDLE(DLLNETWORK,Constraint,PhysConstraint,Constraint,PhysBallSocket,BallSocketConstraint);

class DLLNETWORK PhysBallSocket
	: public PhysConstraint
{
public:
	friend PhysEnv;
protected:
	PhysBallSocket(PhysEnv *env,btPoint2PointConstraint *constraint);
	virtual void InitializeHandle() override;
public:
	virtual void InitializeLuaObject(lua_State *lua) override;
};

class DLLNETWORK PhysHinge;
DECLARE_DERIVED_CHILD_HANDLE(DLLNETWORK,Constraint,PhysConstraint,Constraint,PhysHinge,HingeConstraint);

class DLLNETWORK PhysHinge
	: public PhysConstraint
{
public:
	friend PhysEnv;
protected:
	PhysHinge(PhysEnv *env,btHingeConstraint *constraint);
	virtual void InitializeHandle() override;
public:
	void SetLimit(float low,float high,float softness=0.9f,float biasFactor=0.3f,float relaxationFactor=1.f);
	virtual void InitializeLuaObject(lua_State *lua) override;
};

class DLLNETWORK PhysSlider;
DECLARE_DERIVED_CHILD_HANDLE(DLLNETWORK,Constraint,PhysConstraint,Constraint,PhysSlider,SliderConstraint);

class DLLNETWORK PhysSlider
	: public PhysConstraint
{
public:
	friend PhysEnv;
protected:
	PhysSlider(PhysEnv *env,btSliderConstraint *constraint);
	virtual void InitializeHandle() override;
public:
	virtual void InitializeLuaObject(lua_State *lua) override;
};

class DLLNETWORK PhysConeTwist;
DECLARE_DERIVED_CHILD_HANDLE(DLLNETWORK,Constraint,PhysConstraint,Constraint,PhysConeTwist,ConeTwistConstraint);

class DLLNETWORK PhysConeTwist
	: public PhysConstraint
{
public:
	friend PhysEnv;
protected:
	PhysConeTwist(PhysEnv *env,btConeTwistConstraint *constraint);
	virtual void InitializeHandle() override;
public:
	virtual void InitializeLuaObject(lua_State *lua) override;
	void SetLimit(float swingSpan1,float swingSpan2,float twistSpan,float softness=1.f,float biasFactor=0.3f,float relaxationFactor=1.f);
};

class DLLNETWORK PhysDoF;
DECLARE_DERIVED_CHILD_HANDLE(DLLNETWORK,Constraint,PhysConstraint,Constraint,PhysDoF,DoFConstraint);

class DLLNETWORK PhysDoF
	: public PhysConstraint
{
protected:
	virtual void InitializeHandle() override;
public:
	PhysDoF(PhysEnv *env,btGeneric6DofConstraint *constraint);
	virtual void InitializeLuaObject(lua_State *lua) override;
	void SetLinearLimit(const Vector3 &lower,const Vector3 &upper);
	void SetLinearLimit(const Vector3 &lim);
	void SetLinearLowerLimit(const Vector3 &lim);
	void SetLinearUpperLimit(const Vector3 &lim);
	void SetAngularLimit(const EulerAngles &lower,const EulerAngles &upper);
	void SetAngularLimit(const EulerAngles &lim);
	void SetAngularLowerLimit(const EulerAngles &lim);
	void SetAngularUpperLimit(const EulerAngles &lim);

	Vector3 GetLinearLowerLimit() const;
	Vector3 GetlinearUpperLimit() const;
	EulerAngles GetAngularLowerLimit() const;
	EulerAngles GetAngularUpperLimit() const;

	Vector3 GetAngularTargetVelocity() const;
	Vector3 GetAngularMaxMotorForce() const;
	void SetAngularMaxMotorForce(const Vector3 &force);
	Vector3 GetAngularMaxLimitForce() const;
	void SetAngularMaxLimitForce(const Vector3 &force);
	Vector3 GetAngularDamping() const;
	void SetAngularDamping(const Vector3 &damping);
	Vector3 GetAngularLimitSoftness() const;
	void SetAngularLimitSoftness(const Vector3 &softness);
	Vector3 GetAngularForceMixingFactor() const;
	void SetAngularForceMixingFactor(const Vector3 &factor);
	Vector3 GetAngularLimitErrorTolerance() const;
	void SetAngularLimitErrorTolerance(const Vector3 &tolerance);
	Vector3 GetAngularLimitForceMixingFactor() const;
	void SetAngularLimitForceMixingFactor(const Vector3 &factor);
	Vector3 GetAngularRestitutionFactor() const;
	void SetAngularRestitutionFactor(const Vector3 &factor);
	bool IsAngularMotorEnabled(uint8_t axis) const;
	void SetAngularMotorEnabled(uint8_t axis,bool bEnabled);
	Vector3 GetCurrentAngularLimitError() const;
	Vector3 GetCurrentAngularPosition() const;
	Vector3i GetCurrentAngularLimit() const;
	Vector3 GetCurrentAngularAccumulatedImpulse() const;

	Vector3 GetLinearTargetVelocity() const;
	Vector3 GetLinearMaxMotorForce() const;
	void SetLinearMaxMotorForce(const Vector3 &force);
	float GetLinearDamping() const;
	void SetLinearDamping(float damping);
	float GetLinearLimitSoftness() const;
	void SetLinearLimitSoftness(float softness);
	Vector3 GetLinearForceMixingFactor() const;
	void SetLinearForceMixingFactor(const Vector3 &factor);
	Vector3 GetLinearLimitErrorTolerance() const;
	void SetLinearLimitErrorTolerance(const Vector3 &tolerance);
	Vector3 GetLinearLimitForceMixingFactor() const;
	void SetLinearLimitForceMixingFactor(const Vector3 &factor);
	float GetLinearRestitutionFactor() const;
	void SetLinearRestitutionFactor(float factor);
	bool IsLinearMotorEnabled(uint8_t axis) const;
	void SetLinearMotorEnabled(uint8_t axis,bool bEnabled);
	Vector3 GetCurrentLinearDifference() const;
	Vector3 GetCurrentLinearLimitError() const;
	Vector3i GetCurrentLinearLimit() const;
	Vector3 GetCurrentLinearAccumulatedImpulse() const;
};

#endif