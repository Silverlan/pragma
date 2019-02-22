#include "stdafx_shared.h"
#include "luasystem.h"
#include "pragma/lua/classes/lphysics.h"
#include "pragma/physics/physenvironment.h"
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/lua/classes/ldef_quaternion.h"

extern DLLENGINE Engine *engine;

namespace Lua
{
	namespace PhysConstraint
	{
		static void IsValid(lua_State *l,ConstraintHandle &hConstraint);
		static void Remove(lua_State *l,ConstraintHandle &hConstraint);
		static void GetSourceObject(lua_State *l,ConstraintHandle &hConstraint);
		static void GetTargetObject(lua_State *l,ConstraintHandle &hConstraint);
		static void GetSourceTransform(lua_State *l,ConstraintHandle &hConstraint);
		static void GetTargetTransform(lua_State *l,ConstraintHandle &hConstraint);
		static void GetSourcePosition(lua_State *l,ConstraintHandle &hConstraint);
		static void GetSourceRotation(lua_State *l,ConstraintHandle &hConstraint);
		static void GetTargetPosition(lua_State *l,ConstraintHandle &hConstraint);
		static void GetTargetRotation(lua_State *l,ConstraintHandle &hConstraint);

		static void SetOverrideSolverIterationCount(lua_State *l,ConstraintHandle &hConstraint,int32_t count);
		static void GetOverrideSolverIterationCount(lua_State *l,ConstraintHandle &hConstraint);
		static void GetBreakingImpulseThreshold(lua_State *l,ConstraintHandle &hConstraint);
		static void SetBreakingImpulseThreshold(lua_State *l,ConstraintHandle &hConstraint,float threshold);

		static void SetEnabled(lua_State *l,ConstraintHandle &hConstraint,bool b);
		static void IsEnabled(lua_State *l,ConstraintHandle &hConstraint);
		static void SetCollisionsEnabled(lua_State *l,ConstraintHandle &hConstraint,bool b);
		static void GetCollisionsEnabled(lua_State *l,ConstraintHandle &hConstraint);
		static void EnableCollisions(lua_State *l,ConstraintHandle &hConstraint);
		static void DisableCollisions(lua_State *l,ConstraintHandle &hConstraint);
	};
	namespace PhysConeTwistConstraint
	{
		static void SetLimit(lua_State *l,ConeTwistConstraintHandle &constraint,float swingSpan1,float swingSpan2,float twistSpan);
		static void SetLimit(lua_State *l,ConeTwistConstraintHandle &constraint,const EulerAngles &ang);
	};
	namespace PhysDoFConstraint
	{
		static void SetLinearLowerLimit(lua_State *l,DoFConstraintHandle &constraint,const Vector3 &limit);
		static void SetLinearUpperLimit(lua_State *l,DoFConstraintHandle &constraint,const Vector3 &limit);
		static void SetLinearLimit(lua_State *l,DoFConstraintHandle &constraint,const Vector3 &lower,const Vector3 &upper);
		static void SetLinearLimit(lua_State *l,DoFConstraintHandle &constraint,const Vector3 &limit);
		static void SetAngularLowerLimit(lua_State *l,DoFConstraintHandle &constraint,const EulerAngles &limit);
		static void SetAngularUpperLimit(lua_State *l,DoFConstraintHandle &constraint,const EulerAngles &limit);
		static void SetAngularLimit(lua_State *l,DoFConstraintHandle &constraint,const EulerAngles &lower,const EulerAngles &upper);
		static void SetAngularLimit(lua_State *l,DoFConstraintHandle &constraint,const EulerAngles &limit);

		static void GetLinearLowerLimit(lua_State *l,DoFConstraintHandle &constraint);
		static void GetlinearUpperLimit(lua_State *l,DoFConstraintHandle &constraint);
		static void GetAngularLowerLimit(lua_State *l,DoFConstraintHandle &constraint);
		static void GetAngularUpperLimit(lua_State *l,DoFConstraintHandle &constraint);

		static void GetAngularTargetVelocity(lua_State *l,DoFConstraintHandle &constraint);
		static void GetAngularMaxMotorForce(lua_State *l,DoFConstraintHandle &constraint);
		static void SetAngularMaxMotorForce(lua_State *l,DoFConstraintHandle &constraint,const Vector3 &force);
		static void GetAngularMaxLimitForce(lua_State *l,DoFConstraintHandle &constraint);
		static void SetAngularMaxLimitForce(lua_State *l,DoFConstraintHandle &constraint,const Vector3 &force);
		static void GetAngularDamping(lua_State *l,DoFConstraintHandle &constraint);
		static void SetAngularDamping(lua_State *l,DoFConstraintHandle &constraint,const Vector3 &damping);
		static void GetAngularLimitSoftness(lua_State *l,DoFConstraintHandle &constraint);
		static void SetAngularLimitSoftness(lua_State *l,DoFConstraintHandle &constraint,const Vector3 &softness);
		static void GetAngularForceMixingFactor(lua_State *l,DoFConstraintHandle &constraint);
		static void SetAngularForceMixingFactor(lua_State *l,DoFConstraintHandle &constraint,const Vector3 &factor);
		static void GetAngularLimitErrorTolerance(lua_State *l,DoFConstraintHandle &constraint);
		static void SetAngularLimitErrorTolerance(lua_State *l,DoFConstraintHandle &constraint,const Vector3 &tolerance);
		static void GetAngularLimitForceMixingFactor(lua_State *l,DoFConstraintHandle &constraint);
		static void SetAngularLimitForceMixingFactor(lua_State *l,DoFConstraintHandle &constraint,const Vector3 &factor);
		static void GetAngularRestitutionFactor(lua_State *l,DoFConstraintHandle &constraint);
		static void SetAngularRestitutionFactor(lua_State *l,DoFConstraintHandle &constraint,const Vector3 &factor);
		static void IsAngularMotorEnabled(lua_State *l,DoFConstraintHandle &constraint,uint8_t axis);
		static void SetAngularMotorEnabled(lua_State *l,DoFConstraintHandle &constraint,uint8_t axis,bool bEnabled);
		static void GetCurrentAngularLimitError(lua_State *l,DoFConstraintHandle &constraint);
		static void GetCurrentAngularPosition(lua_State *l,DoFConstraintHandle &constraint);
		static void GetCurrentAngularLimit(lua_State *l,DoFConstraintHandle &constraint);
		static void GetCurrentAngularAccumulatedImpulse(lua_State *l,DoFConstraintHandle &constraint);

		static void GetLinearTargetVelocity(lua_State *l,DoFConstraintHandle &constraint);
		static void GetLinearMaxMotorForce(lua_State *l,DoFConstraintHandle &constraint);
		static void SetLinearMaxMotorForce(lua_State *l,DoFConstraintHandle &constraint,const Vector3 &force);
		static void GetLinearDamping(lua_State *l,DoFConstraintHandle &constraint);
		static void SetLinearDamping(lua_State *l,DoFConstraintHandle &constraint,float damping);
		static void GetLinearLimitSoftness(lua_State *l,DoFConstraintHandle &constraint);
		static void SetLinearLimitSoftness(lua_State *l,DoFConstraintHandle &constraint,float softness);
		static void GetLinearForceMixingFactor(lua_State *l,DoFConstraintHandle &constraint);
		static void SetLinearForceMixingFactor(lua_State *l,DoFConstraintHandle &constraint,const Vector3 &factor);
		static void GetLinearLimitErrorTolerance(lua_State *l,DoFConstraintHandle &constraint);
		static void SetLinearLimitErrorTolerance(lua_State *l,DoFConstraintHandle &constraint,const Vector3 &tolerance);
		static void GetLinearLimitForceMixingFactor(lua_State *l,DoFConstraintHandle &constraint);
		static void SetLinearLimitForceMixingFactor(lua_State *l,DoFConstraintHandle &constraint,const Vector3 &factor);

		static void GetLinearRestitutionFactor(lua_State *l,DoFConstraintHandle &constraint);
		static void SetLinearRestitutionFactor(lua_State *l,DoFConstraintHandle &constraint,float factor);
		static void IsLinearMotorEnabled(lua_State *l,DoFConstraintHandle &constraint,uint8_t axis);
		static void SetLinearMotorEnabled(lua_State *l,DoFConstraintHandle &constraint,uint8_t axis,bool bEnabled);
		static void GetCurrentLinearDifference(lua_State *l,DoFConstraintHandle &constraint);
		static void GetCurrentLinearLimitError(lua_State *l,DoFConstraintHandle &constraint);
		static void GetCurrentLinearLimit(lua_State *l,DoFConstraintHandle &constraint);
		static void GetCurrentLinearAccumulatedImpulse(lua_State *l,DoFConstraintHandle &constraint);
	};
};

void Lua::PhysConstraint::register_class(lua_State *l,luabind::module_ &mod)
{
	auto classDef = luabind::class_<ConstraintHandle>("Constraint");
	classDef.def("IsValid",&IsValid);
	classDef.def("Remove",&Remove);
	classDef.def("GetSourceObject",&GetSourceObject);
	classDef.def("GetTargetObject",&GetTargetObject);
	classDef.def("GetSourceTransform",&GetSourceTransform);
	classDef.def("GetTargetTransform",&GetTargetTransform);
	classDef.def("GetSourcePosition",&GetSourcePosition);
	classDef.def("GetSourceRotation",&GetSourceRotation);
	classDef.def("GetTargetPosition",&GetTargetPosition);
	classDef.def("GetTargetRotation",&GetTargetRotation);

	classDef.def("SetOverrideSolverIterationCount",&SetOverrideSolverIterationCount);
	classDef.def("GetOverrideSolverIterationCount",&GetOverrideSolverIterationCount);
	classDef.def("GetBreakingImpulseThreshold",&GetBreakingImpulseThreshold);
	classDef.def("SetBreakingImpulseThreshold",&SetBreakingImpulseThreshold);

	classDef.def("SetEnabled",&SetEnabled);
	classDef.def("IsEnabled",&IsEnabled);
	classDef.def("SetCollisionsEnabled",&SetCollisionsEnabled);
	classDef.def("GetCollisionsEnabled",&GetCollisionsEnabled);
	classDef.def("EnableCollisions",&EnableCollisions);
	classDef.def("DisableCollisions",&DisableCollisions);

	// Note: Constraints derived from the base 'Constraint' mustn't be in its scope because the base class 'Constraint' must be
	// fully defined before a derived class is registered, and scoped classes cannot be added AFTER a class was defined.
	// For this reason all constraint classes are in the 'phys' library instead.

	auto fixedClassDef = luabind::class_<FixedConstraintHandle COMMA ConstraintHandle>("FixedConstraint");

	auto ballSocketClassDef = luabind::class_<BallSocketConstraintHandle COMMA ConstraintHandle>("BallSocketConstraint");

	auto hingeClassDef = luabind::class_<HingeConstraintHandle COMMA ConstraintHandle>("HingeConstraint");

	auto sliderClassDef = luabind::class_<SliderConstraintHandle COMMA ConstraintHandle>("SliderConstraint");

	auto coneTwistClassDef = luabind::class_<ConeTwistConstraintHandle COMMA ConstraintHandle>("ConeTwistConstraint");
	coneTwistClassDef.def("SetLimit",static_cast<void(*)(lua_State*,ConeTwistConstraintHandle&,float,float,float)>(&PhysConeTwistConstraint::SetLimit));
	coneTwistClassDef.def("SetLimit",static_cast<void(*)(lua_State*,ConeTwistConstraintHandle&,const EulerAngles&)>(&PhysConeTwistConstraint::SetLimit));

	auto DoFClassDef = luabind::class_<DoFConstraintHandle COMMA ConstraintHandle>("DoFConstraint");
	DoFClassDef.def("SetLinearLowerLimit",&PhysDoFConstraint::SetLinearLowerLimit);
	DoFClassDef.def("SetLinearUpperLimit",&PhysDoFConstraint::SetLinearUpperLimit);
	DoFClassDef.def("SetLinearLimit",static_cast<void(*)(lua_State*,DoFConstraintHandle&,const Vector3&)>(&PhysDoFConstraint::SetLinearLimit));
	DoFClassDef.def("SetLinearLimit",static_cast<void(*)(lua_State*,DoFConstraintHandle&,const Vector3&,const Vector3&)>(&PhysDoFConstraint::SetLinearLimit));

	DoFClassDef.def("SetAngularLowerLimit",&PhysDoFConstraint::SetAngularLowerLimit);
	DoFClassDef.def("SetAngularUpperLimit",&PhysDoFConstraint::SetAngularUpperLimit);
	DoFClassDef.def("SetAngularLimit",static_cast<void(*)(lua_State*,DoFConstraintHandle&,const EulerAngles&)>(&PhysDoFConstraint::SetAngularLimit));
	DoFClassDef.def("SetAngularLimit",static_cast<void(*)(lua_State*,DoFConstraintHandle&,const EulerAngles&,const EulerAngles&)>(&PhysDoFConstraint::SetAngularLimit));

	DoFClassDef.def("GetLinearLowerLimit",&PhysDoFConstraint::GetLinearLowerLimit);
	DoFClassDef.def("GetlinearUpperLimit",&PhysDoFConstraint::GetlinearUpperLimit);
	DoFClassDef.def("GetAngularLowerLimit",&PhysDoFConstraint::GetAngularLowerLimit);
	DoFClassDef.def("GetAngularUpperLimit",&PhysDoFConstraint::GetAngularUpperLimit);

	DoFClassDef.def("GetAngularTargetVelocity",&PhysDoFConstraint::GetAngularTargetVelocity);
	DoFClassDef.def("GetAngularMaxMotorForce",&PhysDoFConstraint::GetAngularMaxMotorForce);
	DoFClassDef.def("SetAngularMaxMotorForce",&PhysDoFConstraint::SetAngularMaxMotorForce);
	DoFClassDef.def("GetAngularMaxLimitForce",&PhysDoFConstraint::GetAngularMaxLimitForce);
	DoFClassDef.def("SetAngularMaxLimitForce",&PhysDoFConstraint::SetAngularMaxLimitForce);
	DoFClassDef.def("GetAngularDamping",&PhysDoFConstraint::GetAngularDamping);
	DoFClassDef.def("SetAngularDamping",&PhysDoFConstraint::SetAngularDamping);
	DoFClassDef.def("GetAngularLimitSoftness",&PhysDoFConstraint::GetAngularLimitSoftness);
	DoFClassDef.def("SetAngularLimitSoftness",&PhysDoFConstraint::SetAngularLimitSoftness);
	DoFClassDef.def("GetAngularForceMixingFactor",&PhysDoFConstraint::GetAngularForceMixingFactor);
	DoFClassDef.def("SetAngularForceMixingFactor",&PhysDoFConstraint::SetAngularForceMixingFactor);
	DoFClassDef.def("GetAngularLimitErrorTolerance",&PhysDoFConstraint::GetAngularLimitErrorTolerance);
	DoFClassDef.def("SetAngularLimitErrorTolerance",&PhysDoFConstraint::SetAngularLimitErrorTolerance);
	DoFClassDef.def("GetAngularLimitForceMixingFactor",&PhysDoFConstraint::GetAngularLimitForceMixingFactor);
	DoFClassDef.def("SetAngularLimitForceMixingFactor",&PhysDoFConstraint::SetAngularLimitForceMixingFactor);
	DoFClassDef.def("GetAngularRestitutionFactor",&PhysDoFConstraint::GetAngularRestitutionFactor);
	DoFClassDef.def("SetAngularRestitutionFactor",&PhysDoFConstraint::SetAngularRestitutionFactor);
	DoFClassDef.def("IsAngularMotorEnabled",&PhysDoFConstraint::IsAngularMotorEnabled);
	DoFClassDef.def("SetAngularMotorEnabled",&PhysDoFConstraint::SetAngularMotorEnabled);
	DoFClassDef.def("GetCurrentAngularLimitError",&PhysDoFConstraint::GetCurrentAngularLimitError);
	DoFClassDef.def("GetCurrentAngularPosition",&PhysDoFConstraint::GetCurrentAngularPosition);
	DoFClassDef.def("GetCurrentAngularLimit",&PhysDoFConstraint::GetCurrentAngularLimit);
	DoFClassDef.def("GetCurrentAngularAccumulatedImpulse",&PhysDoFConstraint::GetCurrentAngularAccumulatedImpulse);

	DoFClassDef.def("GetLinearTargetVelocity",&PhysDoFConstraint::GetLinearTargetVelocity);
	DoFClassDef.def("GetLinearMaxMotorForce",&PhysDoFConstraint::GetLinearMaxMotorForce);
	DoFClassDef.def("SetLinearMaxMotorForce",&PhysDoFConstraint::SetLinearMaxMotorForce);
	DoFClassDef.def("GetLinearDamping",&PhysDoFConstraint::GetLinearDamping);
	DoFClassDef.def("SetLinearDamping",&PhysDoFConstraint::SetLinearDamping);
	DoFClassDef.def("GetLinearLimitSoftness",&PhysDoFConstraint::GetLinearLimitSoftness);
	DoFClassDef.def("SetLinearLimitSoftness",&PhysDoFConstraint::SetLinearLimitSoftness);
	DoFClassDef.def("GetLinearForceMixingFactor",&PhysDoFConstraint::GetLinearForceMixingFactor);
	DoFClassDef.def("SetLinearForceMixingFactor",&PhysDoFConstraint::SetLinearForceMixingFactor);
	DoFClassDef.def("GetLinearLimitErrorTolerance",&PhysDoFConstraint::GetLinearLimitErrorTolerance);
	DoFClassDef.def("SetLinearLimitErrorTolerance",&PhysDoFConstraint::SetLinearLimitErrorTolerance);
	DoFClassDef.def("GetLinearLimitForceMixingFactor",&PhysDoFConstraint::GetLinearLimitForceMixingFactor);
	DoFClassDef.def("SetLinearLimitForceMixingFactor",&PhysDoFConstraint::SetLinearLimitForceMixingFactor);
	DoFClassDef.def("GetLinearRestitutionFactor",&PhysDoFConstraint::GetLinearRestitutionFactor);
	DoFClassDef.def("SetLinearRestitutionFactor",&PhysDoFConstraint::SetLinearRestitutionFactor);
	DoFClassDef.def("IsLinearMotorEnabled",&PhysDoFConstraint::IsLinearMotorEnabled);
	DoFClassDef.def("SetLinearMotorEnabled",&PhysDoFConstraint::SetLinearMotorEnabled);
	DoFClassDef.def("GetCurrentLinearDifference",&PhysDoFConstraint::GetCurrentLinearDifference);
	DoFClassDef.def("GetCurrentLinearLimitError",&PhysDoFConstraint::GetCurrentLinearLimitError);
	DoFClassDef.def("GetCurrentLinearLimit",&PhysDoFConstraint::GetCurrentLinearLimit);
	DoFClassDef.def("GetCurrentLinearAccumulatedImpulse",&PhysDoFConstraint::GetCurrentLinearAccumulatedImpulse);

	auto doFSprintClassDef = luabind::class_<DoFSpringConstraintHandle COMMA ConstraintHandle>("DoFSpringConstraint");
	doFSprintClassDef.def("CalculateTransforms",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&)>([](lua_State *l,DoFSpringConstraintHandle &constraint) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		static_cast<PhysDoFSpringConstraint*>(constraint.get())->CalculateTransforms();
	}));
	doFSprintClassDef.def("CalculateTransforms",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,const PhysTransform&,const PhysTransform&)>([](lua_State *l,DoFSpringConstraintHandle &constraint,const PhysTransform &transformA,const PhysTransform &transformB) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		static_cast<PhysDoFSpringConstraint*>(constraint.get())->CalculateTransforms(transformA,transformB);
	}));
	doFSprintClassDef.def("GetRotationalLimitMotor",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,uint32_t)>([](lua_State *l,DoFSpringConstraintHandle &constraint,uint32_t axis) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		auto *pRotLimitMotor = static_cast<PhysDoFSpringConstraint*>(constraint.get())->GetRotationalLimitMotor(static_cast<pragma::Axis>(axis));
		if(pRotLimitMotor == nullptr)
			return;
		Lua::Push<boost::reference_wrapper<btRotationalLimitMotor2>>(l,boost::ref(*pRotLimitMotor));
	}));
	doFSprintClassDef.def("GetTranslationalLimitMotor",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&)>([](lua_State *l,DoFSpringConstraintHandle &constraint) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		auto *pTransLimitMotor = static_cast<PhysDoFSpringConstraint*>(constraint.get())->GetTranslationalLimitMotor();
		if(pTransLimitMotor == nullptr)
			return;
		Lua::Push<boost::reference_wrapper<btTranslationalLimitMotor2>>(l,boost::ref(*pTransLimitMotor));
	}));
	doFSprintClassDef.def("GetCalculatedTransformA",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&)>([](lua_State *l,DoFSpringConstraintHandle &constraint) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		Lua::Push<PhysTransform>(l,static_cast<PhysDoFSpringConstraint*>(constraint.get())->GetCalculatedTransformA());
	}));
	doFSprintClassDef.def("GetCalculatedTransformB",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&)>([](lua_State *l,DoFSpringConstraintHandle &constraint) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		Lua::Push<PhysTransform>(l,static_cast<PhysDoFSpringConstraint*>(constraint.get())->GetCalculatedTransformB());
	}));
	doFSprintClassDef.def("GetFrameOffsetA",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&)>([](lua_State *l,DoFSpringConstraintHandle &constraint) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		Lua::Push<PhysTransform>(l,static_cast<PhysDoFSpringConstraint*>(constraint.get())->GetFrameOffsetA());
	}));
	doFSprintClassDef.def("GetFrameOffsetB",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&)>([](lua_State *l,DoFSpringConstraintHandle &constraint) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		Lua::Push<PhysTransform>(l,static_cast<PhysDoFSpringConstraint*>(constraint.get())->GetFrameOffsetB());
	}));
	doFSprintClassDef.def("GetAxis",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,uint32_t)>([](lua_State *l,DoFSpringConstraintHandle &constraint,uint32_t axis) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		Lua::Push<Vector3>(l,static_cast<PhysDoFSpringConstraint*>(constraint.get())->GetAxis(static_cast<pragma::Axis>(axis)));
	}));
	doFSprintClassDef.def("GetAngle",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,uint32_t)>([](lua_State *l,DoFSpringConstraintHandle &constraint,uint32_t axis) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		Lua::PushNumber(l,static_cast<PhysDoFSpringConstraint*>(constraint.get())->GetAngle(static_cast<pragma::Axis>(axis)));
	}));
	doFSprintClassDef.def("GetRelativePivotPosition",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,uint32_t)>([](lua_State *l,DoFSpringConstraintHandle &constraint,uint32_t axis) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		Lua::PushNumber(l,static_cast<PhysDoFSpringConstraint*>(constraint.get())->GetRelativePivotPosition(static_cast<pragma::Axis>(axis)));
	}));
	doFSprintClassDef.def("SetFrames",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,const PhysTransform&,const PhysTransform&)>([](lua_State *l,DoFSpringConstraintHandle &constraint,const PhysTransform &transformA,const PhysTransform &transformB) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		static_cast<PhysDoFSpringConstraint*>(constraint.get())->SetFrames(transformA,transformB);
	}));
	doFSprintClassDef.def("SetLinearLowerLimit",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,const Vector3&)>([](lua_State *l,DoFSpringConstraintHandle &constraint,const Vector3 &linearLower) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		static_cast<PhysDoFSpringConstraint*>(constraint.get())->SetLinearLowerLimit(linearLower);
	}));
	doFSprintClassDef.def("GetLinearLowerLimit",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&)>([](lua_State *l,DoFSpringConstraintHandle &constraint) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		Lua::Push<Vector3>(l,static_cast<PhysDoFSpringConstraint*>(constraint.get())->GetLinearLowerLimit());
	}));
	doFSprintClassDef.def("SetLinearUpperLimit",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,const Vector3&)>([](lua_State *l,DoFSpringConstraintHandle &constraint,const Vector3 &linearUpper) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		static_cast<PhysDoFSpringConstraint*>(constraint.get())->SetLinearUpperLimit(linearUpper);
	}));
	doFSprintClassDef.def("GetLinearUpperLimit",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&)>([](lua_State *l,DoFSpringConstraintHandle &constraint) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		Lua::Push<Vector3>(l,static_cast<PhysDoFSpringConstraint*>(constraint.get())->GetLinearUpperLimit());
	}));
	doFSprintClassDef.def("SetAngularLowerLimit",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,const Vector3&)>([](lua_State *l,DoFSpringConstraintHandle &constraint,const Vector3 &angularLower) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		static_cast<PhysDoFSpringConstraint*>(constraint.get())->SetAngularLowerLimit(angularLower);
	}));
	doFSprintClassDef.def("SetAngularLowerLimitReversed",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,const Vector3&)>([](lua_State *l,DoFSpringConstraintHandle &constraint,const Vector3 &angularLower) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		static_cast<PhysDoFSpringConstraint*>(constraint.get())->SetAngularLowerLimitReversed(angularLower);
	}));
	doFSprintClassDef.def("GetAngularLowerLimit",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&)>([](lua_State *l,DoFSpringConstraintHandle &constraint) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		Lua::Push<Vector3>(l,static_cast<PhysDoFSpringConstraint*>(constraint.get())->GetAngularLowerLimit());
	}));
	doFSprintClassDef.def("GetAngularLowerLimitReversed",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&)>([](lua_State *l,DoFSpringConstraintHandle &constraint) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		Lua::Push<Vector3>(l,static_cast<PhysDoFSpringConstraint*>(constraint.get())->GetAngularLowerLimitReversed());
	}));
	doFSprintClassDef.def("SetAngularUpperLimit",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,const Vector3&)>([](lua_State *l,DoFSpringConstraintHandle &constraint,const Vector3 &angularUpper) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		static_cast<PhysDoFSpringConstraint*>(constraint.get())->SetAngularUpperLimit(angularUpper);
	}));
	doFSprintClassDef.def("SetAngularUpperLimitReversed",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,const Vector3&)>([](lua_State *l,DoFSpringConstraintHandle &constraint,const Vector3 &angularUpper) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		static_cast<PhysDoFSpringConstraint*>(constraint.get())->SetAngularUpperLimitReversed(angularUpper);
	}));
	doFSprintClassDef.def("GetAngularUpperLimit",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&)>([](lua_State *l,DoFSpringConstraintHandle &constraint) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		Lua::Push<Vector3>(l,static_cast<PhysDoFSpringConstraint*>(constraint.get())->GetAngularUpperLimit());
	}));
	doFSprintClassDef.def("GetAngularUpperLimitReversed",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&)>([](lua_State *l,DoFSpringConstraintHandle &constraint) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		Lua::Push<Vector3>(l,static_cast<PhysDoFSpringConstraint*>(constraint.get())->GetAngularUpperLimitReversed());
	}));
	doFSprintClassDef.def("SetLimit",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,uint32_t,uint32_t,double,double)>([](lua_State *l,DoFSpringConstraintHandle &constraint,uint32_t type,uint32_t axis,double lo,double hi) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		static_cast<PhysDoFSpringConstraint*>(constraint.get())->SetLimit(static_cast<PhysDoFSpringConstraint::AxisType>(type),static_cast<pragma::Axis>(axis),lo,hi);
	}));
	doFSprintClassDef.def("SetLimitReversed",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,uint32_t,uint32_t,double,double)>([](lua_State *l,DoFSpringConstraintHandle &constraint,uint32_t type,uint32_t axis,double lo,double hi) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		static_cast<PhysDoFSpringConstraint*>(constraint.get())->SetLimitReversed(static_cast<PhysDoFSpringConstraint::AxisType>(type),static_cast<pragma::Axis>(axis),lo,hi);
	}));
	doFSprintClassDef.def("IsLimited",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,uint32_t,uint32_t)>([](lua_State *l,DoFSpringConstraintHandle &constraint,uint32_t axis,uint32_t type) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		Lua::PushBool(l,static_cast<PhysDoFSpringConstraint*>(constraint.get())->IsLimited(static_cast<PhysDoFSpringConstraint::AxisType>(type),static_cast<pragma::Axis>(axis)));
	}));
	doFSprintClassDef.def("SetRotationOrder",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,uint32_t)>([](lua_State *l,DoFSpringConstraintHandle &constraint,uint32_t rotationOrder) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		static_cast<PhysDoFSpringConstraint*>(constraint.get())->SetRotationOrder(static_cast<pragma::RotationOrder>(rotationOrder));
	}));
	doFSprintClassDef.def("GetRotationOrder",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&)>([](lua_State *l,DoFSpringConstraintHandle &constraint) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		Lua::PushInt(l,umath::to_integral(static_cast<PhysDoFSpringConstraint*>(constraint.get())->GetRotationOrder()));
	}));
	doFSprintClassDef.def("SetAxis",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,const Vector3&,const Vector3&)>([](lua_State *l,DoFSpringConstraintHandle &constraint,const Vector3 &axis1,const Vector3 &axis2) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		static_cast<PhysDoFSpringConstraint*>(constraint.get())->SetAxis(axis1,axis2);
	}));
	doFSprintClassDef.def("SetBounce",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,uint32_t,uint32_t,double)>([](lua_State *l,DoFSpringConstraintHandle &constraint,uint32_t type,uint32_t axis,double bounce) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		static_cast<PhysDoFSpringConstraint*>(constraint.get())->SetBounce(static_cast<PhysDoFSpringConstraint::AxisType>(type),static_cast<pragma::Axis>(axis),bounce);
	}));
	doFSprintClassDef.def("EnableMotor",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,uint32_t,uint32_t,bool)>([](lua_State *l,DoFSpringConstraintHandle &constraint,uint32_t type,uint32_t axis,bool enable) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		static_cast<PhysDoFSpringConstraint*>(constraint.get())->EnableMotor(static_cast<PhysDoFSpringConstraint::AxisType>(type),static_cast<pragma::Axis>(axis),enable);
	}));
	doFSprintClassDef.def("SetServo",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,uint32_t,uint32_t,bool)>([](lua_State *l,DoFSpringConstraintHandle &constraint,uint32_t type,uint32_t axis,bool enable) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		static_cast<PhysDoFSpringConstraint*>(constraint.get())->SetServo(static_cast<PhysDoFSpringConstraint::AxisType>(type),static_cast<pragma::Axis>(axis),enable);
	}));
	doFSprintClassDef.def("SetTargetVelocity",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,uint32_t,uint32_t,double)>([](lua_State *l,DoFSpringConstraintHandle &constraint,uint32_t type,uint32_t axis,double velocity) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		static_cast<PhysDoFSpringConstraint*>(constraint.get())->SetTargetVelocity(static_cast<PhysDoFSpringConstraint::AxisType>(type),static_cast<pragma::Axis>(axis),velocity);
	}));
	doFSprintClassDef.def("SetServoTarget",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,uint32_t,uint32_t,double)>([](lua_State *l,DoFSpringConstraintHandle &constraint,uint32_t type,uint32_t axis,double target) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		static_cast<PhysDoFSpringConstraint*>(constraint.get())->SetServoTarget(static_cast<PhysDoFSpringConstraint::AxisType>(type),static_cast<pragma::Axis>(axis),target);
	}));
	doFSprintClassDef.def("SetMaxMotorForce",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,uint32_t,uint32_t,double)>([](lua_State *l,DoFSpringConstraintHandle &constraint,uint32_t type,uint32_t axis,double force) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		static_cast<PhysDoFSpringConstraint*>(constraint.get())->SetMaxMotorForce(static_cast<PhysDoFSpringConstraint::AxisType>(type),static_cast<pragma::Axis>(axis),force);
	}));
	doFSprintClassDef.def("EnableSpring",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,uint32_t,uint32_t,bool)>([](lua_State *l,DoFSpringConstraintHandle &constraint,uint32_t type,uint32_t axis,bool enable) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		static_cast<PhysDoFSpringConstraint*>(constraint.get())->EnableSpring(static_cast<PhysDoFSpringConstraint::AxisType>(type),static_cast<pragma::Axis>(axis),enable);
	}));
	doFSprintClassDef.def("SetStiffness",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,uint32_t,uint32_t,double,bool)>([](lua_State *l,DoFSpringConstraintHandle &constraint,uint32_t type,uint32_t axis,double stiffness,bool limitIfNeeded) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		static_cast<PhysDoFSpringConstraint*>(constraint.get())->SetStiffness(static_cast<PhysDoFSpringConstraint::AxisType>(type),static_cast<pragma::Axis>(axis),stiffness,limitIfNeeded);
	}));
	doFSprintClassDef.def("SetStiffness",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,uint32_t,uint32_t,double)>([](lua_State *l,DoFSpringConstraintHandle &constraint,uint32_t type,uint32_t axis,double stiffness) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		static_cast<PhysDoFSpringConstraint*>(constraint.get())->SetStiffness(static_cast<PhysDoFSpringConstraint::AxisType>(type),static_cast<pragma::Axis>(axis),stiffness);
	}));
	doFSprintClassDef.def("SetDamping",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,uint32_t,uint32_t,double,bool)>([](lua_State *l,DoFSpringConstraintHandle &constraint,uint32_t type,uint32_t axis,double damping,bool limitIfNeeded) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		static_cast<PhysDoFSpringConstraint*>(constraint.get())->SetDamping(static_cast<PhysDoFSpringConstraint::AxisType>(type),static_cast<pragma::Axis>(axis),damping,limitIfNeeded);
	}));
	doFSprintClassDef.def("SetDamping",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,uint32_t,uint32_t,double)>([](lua_State *l,DoFSpringConstraintHandle &constraint,uint32_t type,uint32_t axis,double damping) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		static_cast<PhysDoFSpringConstraint*>(constraint.get())->SetDamping(static_cast<PhysDoFSpringConstraint::AxisType>(type),static_cast<pragma::Axis>(axis),damping);
	}));
	doFSprintClassDef.def("SetEquilibriumPoint",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&)>([](lua_State *l,DoFSpringConstraintHandle &constraint) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		static_cast<PhysDoFSpringConstraint*>(constraint.get())->SetEquilibriumPoint();
	}));
	doFSprintClassDef.def("SetEquilibriumPoint",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,uint32_t,uint32_t)>([](lua_State *l,DoFSpringConstraintHandle &constraint,uint32_t type,uint32_t axis) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		static_cast<PhysDoFSpringConstraint*>(constraint.get())->SetEquilibriumPoint(static_cast<PhysDoFSpringConstraint::AxisType>(type),static_cast<pragma::Axis>(axis));
	}));
	doFSprintClassDef.def("SetEquilibriumPoint",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,uint32_t,uint32_t,double)>([](lua_State *l,DoFSpringConstraintHandle &constraint,uint32_t type,uint32_t axis,double value) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		static_cast<PhysDoFSpringConstraint*>(constraint.get())->SetEquilibriumPoint(static_cast<PhysDoFSpringConstraint::AxisType>(type),static_cast<pragma::Axis>(axis),value);
	}));
	doFSprintClassDef.def("SetERP",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,uint32_t,uint32_t,double)>([](lua_State *l,DoFSpringConstraintHandle &constraint,uint32_t type,uint32_t axis,double value) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		static_cast<PhysDoFSpringConstraint*>(constraint.get())->SetERP(static_cast<PhysDoFSpringConstraint::AxisType>(type),static_cast<pragma::Axis>(axis),value);
	}));
	doFSprintClassDef.def("GetERP",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,uint32_t,uint32_t)>([](lua_State *l,DoFSpringConstraintHandle &constraint,uint32_t type,uint32_t axis) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		Lua::PushNumber(l,static_cast<PhysDoFSpringConstraint*>(constraint.get())->GetERP(static_cast<PhysDoFSpringConstraint::AxisType>(type),static_cast<pragma::Axis>(axis)));
	}));
	doFSprintClassDef.def("SetStopERP",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,uint32_t,uint32_t,double)>([](lua_State *l,DoFSpringConstraintHandle &constraint,uint32_t type,uint32_t axis,double value) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		static_cast<PhysDoFSpringConstraint*>(constraint.get())->SetStopERP(static_cast<PhysDoFSpringConstraint::AxisType>(type),static_cast<pragma::Axis>(axis),value);
	}));
	doFSprintClassDef.def("GetStopERP",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,uint32_t,uint32_t)>([](lua_State *l,DoFSpringConstraintHandle &constraint,uint32_t type,uint32_t axis) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		Lua::PushNumber(l,static_cast<PhysDoFSpringConstraint*>(constraint.get())->GetStopERP(static_cast<PhysDoFSpringConstraint::AxisType>(type),static_cast<pragma::Axis>(axis)));
	}));
	doFSprintClassDef.def("SetCFM",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,uint32_t,uint32_t,double)>([](lua_State *l,DoFSpringConstraintHandle &constraint,uint32_t type,uint32_t axis,double value) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		static_cast<PhysDoFSpringConstraint*>(constraint.get())->SetCFM(static_cast<PhysDoFSpringConstraint::AxisType>(type),static_cast<pragma::Axis>(axis),value);
	}));
	doFSprintClassDef.def("GetCFM",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,uint32_t,uint32_t)>([](lua_State *l,DoFSpringConstraintHandle &constraint,uint32_t type,uint32_t axis) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		Lua::PushNumber(l,static_cast<PhysDoFSpringConstraint*>(constraint.get())->GetCFM(static_cast<PhysDoFSpringConstraint::AxisType>(type),static_cast<pragma::Axis>(axis)));
	}));
	doFSprintClassDef.def("SetStopCFM",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,uint32_t,uint32_t,double)>([](lua_State *l,DoFSpringConstraintHandle &constraint,uint32_t type,uint32_t axis,double value) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		static_cast<PhysDoFSpringConstraint*>(constraint.get())->SetStopCFM(static_cast<PhysDoFSpringConstraint::AxisType>(type),static_cast<pragma::Axis>(axis),value);
	}));
	doFSprintClassDef.def("GetStopCFM",static_cast<void(*)(lua_State*,DoFSpringConstraintHandle&,uint32_t,uint32_t)>([](lua_State *l,DoFSpringConstraintHandle &constraint,uint32_t type,uint32_t axis) {
		LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
		Lua::PushNumber(l,static_cast<PhysDoFSpringConstraint*>(constraint.get())->GetStopCFM(static_cast<PhysDoFSpringConstraint::AxisType>(type),static_cast<pragma::Axis>(axis)));
	}));

	auto defRotLimitMotor = luabind::class_<btRotationalLimitMotor2>("RotationalLimitMotor");
	defRotLimitMotor.def_readwrite("loLimit",&btRotationalLimitMotor2::m_loLimit);
	defRotLimitMotor.def_readwrite("hiLimit",&btRotationalLimitMotor2::m_hiLimit);
	defRotLimitMotor.def_readwrite("bounce",&btRotationalLimitMotor2::m_bounce);
	defRotLimitMotor.def_readwrite("stopERP",&btRotationalLimitMotor2::m_stopERP);
	defRotLimitMotor.def_readwrite("stopCFM",&btRotationalLimitMotor2::m_stopCFM);
	defRotLimitMotor.def_readwrite("motorERP",&btRotationalLimitMotor2::m_motorERP);
	defRotLimitMotor.def_readwrite("motorCFM",&btRotationalLimitMotor2::m_motorCFM);
	defRotLimitMotor.def_readwrite("enableMotor",&btRotationalLimitMotor2::m_enableMotor);
	defRotLimitMotor.def_readwrite("targetVelocity",&btRotationalLimitMotor2::m_targetVelocity);
	defRotLimitMotor.def_readwrite("maxMotorForce",&btRotationalLimitMotor2::m_maxMotorForce);
	defRotLimitMotor.def_readwrite("servoMotor",&btRotationalLimitMotor2::m_servoMotor);
	defRotLimitMotor.def_readwrite("servoTarget",&btRotationalLimitMotor2::m_servoTarget);
	defRotLimitMotor.def_readwrite("enableSpring",&btRotationalLimitMotor2::m_enableSpring);
	defRotLimitMotor.def_readwrite("springStiffness",&btRotationalLimitMotor2::m_springStiffness);
	defRotLimitMotor.def_readwrite("springStiffnessLimited",&btRotationalLimitMotor2::m_springStiffnessLimited);
	defRotLimitMotor.def_readwrite("springDamping",&btRotationalLimitMotor2::m_springDamping);
	defRotLimitMotor.def_readwrite("springDampingLimited",&btRotationalLimitMotor2::m_springDampingLimited);
	defRotLimitMotor.def_readwrite("equilibriumPoint",&btRotationalLimitMotor2::m_equilibriumPoint);
	defRotLimitMotor.def_readwrite("currentLimitError",&btRotationalLimitMotor2::m_currentLimitError);
	defRotLimitMotor.def_readwrite("currentLimitErrorHi",&btRotationalLimitMotor2::m_currentLimitErrorHi);
	defRotLimitMotor.def_readwrite("currentPosition",&btRotationalLimitMotor2::m_currentPosition);
	defRotLimitMotor.def_readwrite("currentLimit",&btRotationalLimitMotor2::m_currentLimit);
	defRotLimitMotor.def("IsLimited",static_cast<void(*)(lua_State*,btRotationalLimitMotor2&)>([](lua_State *l,btRotationalLimitMotor2 &rotLimitMotor) {
		Lua::PushBool(l,rotLimitMotor.isLimited());
	}));
	doFSprintClassDef.scope[defRotLimitMotor];

	auto defTransLimitMotor = luabind::class_<btTranslationalLimitMotor2>("TranslationalLimitMotor");
	defTransLimitMotor.property("lowerLimit",static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor) {
		Lua::Push<Vector3>(l,uvec::create(transLimitMotor.m_lowerLimit /PhysEnv::WORLD_SCALE));
	}),static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&,const Vector3&)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor,const Vector3 &limit) {
		transLimitMotor.m_lowerLimit = uvec::create_bt(limit) *PhysEnv::WORLD_SCALE;
	}));
	defTransLimitMotor.property("upperLimit",static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor) {
		Lua::Push<Vector3>(l,uvec::create(transLimitMotor.m_upperLimit /PhysEnv::WORLD_SCALE));
	}),static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&,const Vector3&)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor,const Vector3 &limit) {
		transLimitMotor.m_upperLimit = uvec::create_bt(limit) *PhysEnv::WORLD_SCALE;
	}));
	defTransLimitMotor.property("bounce",static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor) {
		Lua::Push<Vector3>(l,uvec::create(transLimitMotor.m_bounce));
	}),static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&,const Vector3&)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor,const Vector3 &bounce) {
		transLimitMotor.m_bounce = uvec::create_bt(bounce);
	}));
	defTransLimitMotor.property("stopERP",static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor) {
		Lua::Push<Vector3>(l,uvec::create(transLimitMotor.m_stopERP));
	}),static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&,const Vector3&)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor,const Vector3 &erp) {
		transLimitMotor.m_stopERP = uvec::create_bt(erp);
	}));
	defTransLimitMotor.property("stopCFM",static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor) {
		Lua::Push<Vector3>(l,uvec::create(transLimitMotor.m_stopCFM));
	}),static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&,const Vector3&)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor,const Vector3 &cfm) {
		transLimitMotor.m_stopCFM = uvec::create_bt(cfm);
	}));
	defTransLimitMotor.property("motorERP",static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor) {
		Lua::Push<Vector3>(l,uvec::create(transLimitMotor.m_motorERP));
	}),static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&,const Vector3&)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor,const Vector3 &motorErp) {
		transLimitMotor.m_motorERP = uvec::create_bt(motorErp);
	}));
	defTransLimitMotor.property("motorCFM",static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor) {
		Lua::Push<Vector3>(l,uvec::create(transLimitMotor.m_motorCFM));
	}),static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&,const Vector3&)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor,const Vector3 &motorCfm) {
		transLimitMotor.m_motorCFM = uvec::create_bt(motorCfm);
	}));
	defTransLimitMotor.property("servoMotor",static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&,uint32_t)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor,uint32_t axis) {
		Lua::PushBool(l,transLimitMotor.m_servoMotor[axis]);
	}),static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&,uint32_t,bool)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor,uint32_t axis,bool servoMotor) {
		transLimitMotor.m_servoMotor[axis] = servoMotor;
	}));
	defTransLimitMotor.property("enableSpring",static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&,uint32_t)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor,uint32_t axis) {
		Lua::PushBool(l,transLimitMotor.m_enableSpring[axis]);
	}),static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&,uint32_t,bool)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor,uint32_t axis,bool spring) {
		transLimitMotor.m_enableSpring[axis] = spring;
	}));
	defTransLimitMotor.property("servoTarget",static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor) {
		Lua::Push<Vector3>(l,uvec::create(transLimitMotor.m_servoTarget));
	}),static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&,const Vector3&)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor,const Vector3 &servoTarget) {
		transLimitMotor.m_servoTarget = uvec::create_bt(servoTarget);
	}));
	defTransLimitMotor.property("springStiffness",static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor) {
		Lua::Push<Vector3>(l,uvec::create(transLimitMotor.m_springStiffness));
	}),static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&,const Vector3&)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor,const Vector3 &stiffness) {
		transLimitMotor.m_springStiffness = uvec::create_bt(stiffness);
	}));
	defTransLimitMotor.property("springStiffnessLimited",static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&,uint32_t)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor,uint32_t axis) {
		Lua::PushBool(l,transLimitMotor.m_springStiffnessLimited[axis]);
	}),static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&,uint32_t,bool)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor,uint32_t axis,bool stiffnessLimited) {
		transLimitMotor.m_springStiffnessLimited[axis] = stiffnessLimited;
	}));
	defTransLimitMotor.property("springDamping",static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor) {
		Lua::Push<Vector3>(l,uvec::create(transLimitMotor.m_springDamping));
	}),static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&,const Vector3&)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor,const Vector3 &damping) {
		transLimitMotor.m_springDamping = uvec::create_bt(damping);
	}));
	defTransLimitMotor.property("springDampingLimited",static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&,uint32_t)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor,uint32_t axis) {
		Lua::PushBool(l,transLimitMotor.m_springDampingLimited[axis]);
	}),static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&,uint32_t,bool)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor,uint32_t axis,bool dampingLimited) {
		transLimitMotor.m_springDampingLimited[axis] = dampingLimited;
	}));
	defTransLimitMotor.property("equilibriumPoint",static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor) {
		Lua::Push<Vector3>(l,uvec::create(transLimitMotor.m_equilibriumPoint /PhysEnv::WORLD_SCALE));
	}),static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&,const Vector3&)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor,const Vector3 &equiPoint) {
		transLimitMotor.m_equilibriumPoint = uvec::create_bt(equiPoint) *PhysEnv::WORLD_SCALE;
	}));
	defTransLimitMotor.property("targetVelocity",static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor) {
		Lua::Push<Vector3>(l,uvec::create(transLimitMotor.m_targetVelocity /PhysEnv::WORLD_SCALE));
	}),static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&,const Vector3&)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor,const Vector3 &targetVel) {
		transLimitMotor.m_targetVelocity = uvec::create_bt(targetVel) *PhysEnv::WORLD_SCALE;
	}));
	defTransLimitMotor.property("maxMotorForce",static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor) {
		Lua::Push<Vector3>(l,uvec::create(transLimitMotor.m_maxMotorForce /PhysEnv::WORLD_SCALE));
	}),static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&,const Vector3&)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor,const Vector3 &maxMotorForce) {
		transLimitMotor.m_maxMotorForce = uvec::create_bt(maxMotorForce) *PhysEnv::WORLD_SCALE;
	}));
	defTransLimitMotor.property("currentLimitError",static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor) {
		Lua::Push<Vector3>(l,uvec::create(transLimitMotor.m_currentLimitError /PhysEnv::WORLD_SCALE));
	}),static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&,const Vector3&)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor,const Vector3 &limitErr) {
		transLimitMotor.m_currentLimitError = uvec::create_bt(limitErr) *PhysEnv::WORLD_SCALE;
	}));
	defTransLimitMotor.property("currentLimitErrorHi",static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor) {
		Lua::Push<Vector3>(l,uvec::create(transLimitMotor.m_currentLimitErrorHi /PhysEnv::WORLD_SCALE));
	}),static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&,const Vector3&)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor,const Vector3 &limitErrHi) {
		transLimitMotor.m_currentLimitErrorHi = uvec::create_bt(limitErrHi) *PhysEnv::WORLD_SCALE;
	}));
	defTransLimitMotor.property("currentLinearDiff",static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor) {
		Lua::Push<Vector3>(l,uvec::create(transLimitMotor.m_currentLinearDiff /PhysEnv::WORLD_SCALE));
	}),static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&,const Vector3&)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor,const Vector3 &linearDiff) {
		transLimitMotor.m_currentLinearDiff = uvec::create_bt(linearDiff) *PhysEnv::WORLD_SCALE;
	}));
	defTransLimitMotor.property("currentLimit",static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&,uint32_t)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor,uint32_t axis) {
		Lua::PushInt(l,transLimitMotor.m_currentLimit[axis]);
	}),static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&,uint32_t,int32_t)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor,uint32_t axis,int32_t currentLimit) {
		transLimitMotor.m_currentLimit[axis] = currentLimit;
	}));
	defTransLimitMotor.def("IsLimited",static_cast<void(*)(lua_State*,btTranslationalLimitMotor2&,uint32_t)>([](lua_State *l,btTranslationalLimitMotor2 &transLimitMotor,uint32_t axis) {
		Lua::PushBool(l,transLimitMotor.isLimited(axis));
	}));
	doFSprintClassDef.scope[defTransLimitMotor];
	mod[classDef];
	mod[ballSocketClassDef];
	mod[doFSprintClassDef];
	mod[DoFClassDef];
	mod[coneTwistClassDef];
	mod[sliderClassDef];
	mod[fixedClassDef];
	mod[hingeClassDef];
}

void Lua::PhysConstraint::IsValid(lua_State *l,ConstraintHandle &hConstraint)
{
	Lua::PushBool(l,hConstraint.IsValid());
}
void Lua::PhysConstraint::GetSourceObject(lua_State *l,ConstraintHandle &hConstraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,hConstraint);
	auto *o = hConstraint->GetSourceObject();
	if(o == nullptr)
		return;
	o->GetLuaObject()->push(l);
}
void Lua::PhysConstraint::GetTargetObject(lua_State *l,ConstraintHandle &hConstraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,hConstraint);
	auto *o = hConstraint->GetTargetObject();
	if(o == nullptr)
		return;
	o->GetLuaObject()->push(l);
}
void Lua::PhysConstraint::Remove(lua_State *l,ConstraintHandle &hConstraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,hConstraint);
	hConstraint->Remove();
}
void Lua::PhysConstraint::GetSourceTransform(lua_State *l,ConstraintHandle &hConstraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,hConstraint);
	Lua::Push<PhysTransform>(l,hConstraint->GetSourceTransform());
}
void Lua::PhysConstraint::GetTargetTransform(lua_State *l,ConstraintHandle &hConstraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,hConstraint);
	Lua::Push<PhysTransform>(l,hConstraint->GetTargetTransform());
}
void Lua::PhysConstraint::GetSourcePosition(lua_State *l,ConstraintHandle &hConstraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,hConstraint);
	Lua::Push<Vector3>(l,hConstraint->GetSourcePosition());
}
void Lua::PhysConstraint::GetSourceRotation(lua_State *l,ConstraintHandle &hConstraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,hConstraint);
	Lua::Push<Quat>(l,hConstraint->GetSourceRotation());
}
void Lua::PhysConstraint::GetTargetPosition(lua_State *l,ConstraintHandle &hConstraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,hConstraint);
	Lua::Push<Vector3>(l,hConstraint->GetTargetPosition());
}
void Lua::PhysConstraint::GetTargetRotation(lua_State *l,ConstraintHandle &hConstraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,hConstraint);
	Lua::Push<Quat>(l,hConstraint->GetTargetRotation());
}
void Lua::PhysConstraint::SetOverrideSolverIterationCount(lua_State *l,ConstraintHandle &hConstraint,int32_t count)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,hConstraint);
	hConstraint->SetOverrideSolverIterationCount(count);
}
void Lua::PhysConstraint::GetOverrideSolverIterationCount(lua_State *l,ConstraintHandle &hConstraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,hConstraint);
	Lua::PushInt(l,hConstraint->GetOverrideSolverIterationCount());
}
void Lua::PhysConstraint::GetBreakingImpulseThreshold(lua_State *l,ConstraintHandle &hConstraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,hConstraint);
	Lua::PushNumber(l,hConstraint->GetBreakingImpulseThreshold());
}
void Lua::PhysConstraint::SetBreakingImpulseThreshold(lua_State *l,ConstraintHandle &hConstraint,float threshold)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,hConstraint);
	hConstraint->SetBreakingImpulseThreshold(threshold);
}
void Lua::PhysConstraint::SetEnabled(lua_State *l,ConstraintHandle &hConstraint,bool b)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,hConstraint);
	hConstraint->SetEnabled(b);
}
void Lua::PhysConstraint::IsEnabled(lua_State *l,ConstraintHandle &hConstraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,hConstraint);
	Lua::PushBool(l,hConstraint->IsEnabled());
}
void Lua::PhysConstraint::SetCollisionsEnabled(lua_State *l,ConstraintHandle &hConstraint,bool b)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,hConstraint);
	hConstraint->SetCollisionsEnabled(b);
}
void Lua::PhysConstraint::GetCollisionsEnabled(lua_State *l,ConstraintHandle &hConstraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,hConstraint);
	Lua::PushBool(l,hConstraint->GetCollisionsEnabled());
}
void Lua::PhysConstraint::EnableCollisions(lua_State *l,ConstraintHandle &hConstraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,hConstraint);
	hConstraint->EnableCollisions();
}
void Lua::PhysConstraint::DisableCollisions(lua_State *l,ConstraintHandle &hConstraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,hConstraint);
	hConstraint->DisableCollisions();
}

void Lua::PhysConeTwistConstraint::SetLimit(lua_State *l,ConeTwistConstraintHandle &constraint,float swingSpan1,float swingSpan2,float twistSpan)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	auto *c = constraint->GetConstraint();
	static_cast<btConeTwistConstraint*>(c)->setLimit(swingSpan1,swingSpan2,twistSpan);
}
void Lua::PhysConeTwistConstraint::SetLimit(lua_State *l,ConeTwistConstraintHandle &constraint,const EulerAngles &ang)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	auto *c = constraint->GetConstraint();
	static_cast<btConeTwistConstraint*>(c)->setLimit(umath::deg_to_rad(ang.p),umath::deg_to_rad(ang.y),umath::deg_to_rad(ang.r));
}

void Lua::PhysDoFConstraint::SetLinearLowerLimit(lua_State *l,DoFConstraintHandle &constraint,const Vector3 &limit)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	static_cast<PhysDoF*>(constraint.get())->SetLinearLowerLimit(limit);
}
void Lua::PhysDoFConstraint::SetLinearUpperLimit(lua_State *l,DoFConstraintHandle &constraint,const Vector3 &limit)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	static_cast<PhysDoF*>(constraint.get())->SetLinearUpperLimit(limit);
}
void Lua::PhysDoFConstraint::SetLinearLimit(lua_State *l,DoFConstraintHandle &constraint,const Vector3 &lower,const Vector3 &upper)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	static_cast<PhysDoF*>(constraint.get())->SetLinearLimit(lower,upper);
}
void Lua::PhysDoFConstraint::SetLinearLimit(lua_State *l,DoFConstraintHandle &constraint,const Vector3 &limit)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	static_cast<PhysDoF*>(constraint.get())->SetLinearLimit(limit);
}
void Lua::PhysDoFConstraint::SetAngularLowerLimit(lua_State *l,DoFConstraintHandle &constraint,const EulerAngles &limit)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	static_cast<PhysDoF*>(constraint.get())->SetAngularLowerLimit(limit);
}
void Lua::PhysDoFConstraint::SetAngularUpperLimit(lua_State *l,DoFConstraintHandle &constraint,const EulerAngles &limit)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	static_cast<PhysDoF*>(constraint.get())->SetAngularUpperLimit(limit);
}
void Lua::PhysDoFConstraint::SetAngularLimit(lua_State *l,DoFConstraintHandle &constraint,const EulerAngles &lower,const EulerAngles &upper)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	static_cast<PhysDoF*>(constraint.get())->SetAngularLimit(lower,upper);
}
void Lua::PhysDoFConstraint::SetAngularLimit(lua_State *l,DoFConstraintHandle &constraint,const EulerAngles &limit)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	static_cast<PhysDoF*>(constraint.get())->SetAngularLimit(limit);
}

void Lua::PhysDoFConstraint::GetLinearLowerLimit(lua_State *l,DoFConstraintHandle &constraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	Lua::Push<Vector3>(l,static_cast<PhysDoF*>(constraint.get())->GetLinearLowerLimit());
}
void Lua::PhysDoFConstraint::GetlinearUpperLimit(lua_State *l,DoFConstraintHandle &constraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	Lua::Push<Vector3>(l,static_cast<PhysDoF*>(constraint.get())->GetlinearUpperLimit());
}
void Lua::PhysDoFConstraint::GetAngularLowerLimit(lua_State *l,DoFConstraintHandle &constraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	Lua::Push<EulerAngles>(l,static_cast<PhysDoF*>(constraint.get())->GetAngularLowerLimit());
}
void Lua::PhysDoFConstraint::GetAngularUpperLimit(lua_State *l,DoFConstraintHandle &constraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	Lua::Push<EulerAngles>(l,static_cast<PhysDoF*>(constraint.get())->GetAngularUpperLimit());
}

void Lua::PhysDoFConstraint::GetAngularTargetVelocity(lua_State *l,DoFConstraintHandle &constraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	Lua::Push<Vector3>(l,static_cast<PhysDoF*>(constraint.get())->GetAngularTargetVelocity());
}
void Lua::PhysDoFConstraint::GetAngularMaxMotorForce(lua_State *l,DoFConstraintHandle &constraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	Lua::Push<Vector3>(l,static_cast<PhysDoF*>(constraint.get())->GetAngularMaxMotorForce());
}
void Lua::PhysDoFConstraint::SetAngularMaxMotorForce(lua_State *l,DoFConstraintHandle &constraint,const Vector3 &force)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	static_cast<PhysDoF*>(constraint.get())->SetAngularMaxMotorForce(force);
}
void Lua::PhysDoFConstraint::GetAngularMaxLimitForce(lua_State *l,DoFConstraintHandle &constraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	Lua::Push<Vector3>(l,static_cast<PhysDoF*>(constraint.get())->GetAngularMaxLimitForce());
}
void Lua::PhysDoFConstraint::SetAngularMaxLimitForce(lua_State *l,DoFConstraintHandle &constraint,const Vector3 &force)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	static_cast<PhysDoF*>(constraint.get())->SetAngularMaxLimitForce(force);
}
void Lua::PhysDoFConstraint::GetAngularDamping(lua_State *l,DoFConstraintHandle &constraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	Lua::Push<Vector3>(l,static_cast<PhysDoF*>(constraint.get())->GetAngularDamping());
}
void Lua::PhysDoFConstraint::SetAngularDamping(lua_State *l,DoFConstraintHandle &constraint,const Vector3 &damping)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	static_cast<PhysDoF*>(constraint.get())->SetAngularDamping(damping);
}
void Lua::PhysDoFConstraint::GetAngularLimitSoftness(lua_State *l,DoFConstraintHandle &constraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	Lua::Push<Vector3>(l,static_cast<PhysDoF*>(constraint.get())->GetAngularLimitSoftness());
}
void Lua::PhysDoFConstraint::SetAngularLimitSoftness(lua_State *l,DoFConstraintHandle &constraint,const Vector3 &softness)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	static_cast<PhysDoF*>(constraint.get())->SetAngularLimitSoftness(softness);
}
void Lua::PhysDoFConstraint::GetAngularForceMixingFactor(lua_State *l,DoFConstraintHandle &constraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	Lua::Push<Vector3>(l,static_cast<PhysDoF*>(constraint.get())->GetAngularForceMixingFactor());
}
void Lua::PhysDoFConstraint::SetAngularForceMixingFactor(lua_State *l,DoFConstraintHandle &constraint,const Vector3 &factor)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	static_cast<PhysDoF*>(constraint.get())->SetAngularForceMixingFactor(factor);
}
void Lua::PhysDoFConstraint::GetAngularLimitErrorTolerance(lua_State *l,DoFConstraintHandle &constraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	Lua::Push<Vector3>(l,static_cast<PhysDoF*>(constraint.get())->GetAngularLimitErrorTolerance());
}
void Lua::PhysDoFConstraint::SetAngularLimitErrorTolerance(lua_State *l,DoFConstraintHandle &constraint,const Vector3 &tolerance)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	static_cast<PhysDoF*>(constraint.get())->SetAngularLimitErrorTolerance(tolerance);
}
void Lua::PhysDoFConstraint::GetAngularLimitForceMixingFactor(lua_State *l,DoFConstraintHandle &constraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	Lua::Push<Vector3>(l,static_cast<PhysDoF*>(constraint.get())->GetAngularLimitForceMixingFactor());
}
void Lua::PhysDoFConstraint::SetAngularLimitForceMixingFactor(lua_State *l,DoFConstraintHandle &constraint,const Vector3 &factor)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	static_cast<PhysDoF*>(constraint.get())->SetAngularLimitForceMixingFactor(factor);
}
void Lua::PhysDoFConstraint::GetAngularRestitutionFactor(lua_State *l,DoFConstraintHandle &constraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	Lua::Push<Vector3>(l,static_cast<PhysDoF*>(constraint.get())->GetAngularRestitutionFactor());
}
void Lua::PhysDoFConstraint::SetAngularRestitutionFactor(lua_State *l,DoFConstraintHandle &constraint,const Vector3 &factor)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	static_cast<PhysDoF*>(constraint.get())->SetAngularRestitutionFactor(factor);
}
void Lua::PhysDoFConstraint::IsAngularMotorEnabled(lua_State *l,DoFConstraintHandle &constraint,uint8_t axis)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	Lua::PushBool(l,static_cast<PhysDoF*>(constraint.get())->IsAngularMotorEnabled(axis));
}
void Lua::PhysDoFConstraint::SetAngularMotorEnabled(lua_State *l,DoFConstraintHandle &constraint,uint8_t axis,bool bEnabled)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	static_cast<PhysDoF*>(constraint.get())->SetAngularMotorEnabled(axis,bEnabled);
}
void Lua::PhysDoFConstraint::GetCurrentAngularLimitError(lua_State *l,DoFConstraintHandle &constraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	Lua::Push<Vector3>(l,static_cast<PhysDoF*>(constraint.get())->GetCurrentAngularLimitError());
}
void Lua::PhysDoFConstraint::GetCurrentAngularPosition(lua_State *l,DoFConstraintHandle &constraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	Lua::Push<Vector3>(l,static_cast<PhysDoF*>(constraint.get())->GetCurrentAngularPosition());
}
void Lua::PhysDoFConstraint::GetCurrentAngularLimit(lua_State *l,DoFConstraintHandle &constraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	Lua::Push<Vector3>(l,static_cast<PhysDoF*>(constraint.get())->GetCurrentAngularLimit());
}
void Lua::PhysDoFConstraint::GetCurrentAngularAccumulatedImpulse(lua_State *l,DoFConstraintHandle &constraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	Lua::Push<Vector3>(l,static_cast<PhysDoF*>(constraint.get())->GetCurrentAngularAccumulatedImpulse());
}
void Lua::PhysDoFConstraint::GetLinearTargetVelocity(lua_State *l,DoFConstraintHandle &constraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	Lua::Push<Vector3>(l,static_cast<PhysDoF*>(constraint.get())->GetLinearTargetVelocity());
}
void Lua::PhysDoFConstraint::GetLinearMaxMotorForce(lua_State *l,DoFConstraintHandle &constraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	Lua::Push<Vector3>(l,static_cast<PhysDoF*>(constraint.get())->GetLinearMaxMotorForce());
}
void Lua::PhysDoFConstraint::SetLinearMaxMotorForce(lua_State *l,DoFConstraintHandle &constraint,const Vector3 &force)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	static_cast<PhysDoF*>(constraint.get())->SetLinearMaxMotorForce(force);
}
void Lua::PhysDoFConstraint::GetLinearDamping(lua_State *l,DoFConstraintHandle &constraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	Lua::PushNumber(l,static_cast<PhysDoF*>(constraint.get())->GetLinearDamping());
}
void Lua::PhysDoFConstraint::SetLinearDamping(lua_State *l,DoFConstraintHandle &constraint,float damping)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	static_cast<PhysDoF*>(constraint.get())->SetLinearDamping(damping);
}
void Lua::PhysDoFConstraint::GetLinearLimitSoftness(lua_State *l,DoFConstraintHandle &constraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	Lua::PushNumber(l,static_cast<PhysDoF*>(constraint.get())->GetLinearLimitSoftness());
}
void Lua::PhysDoFConstraint::SetLinearLimitSoftness(lua_State *l,DoFConstraintHandle &constraint,float softness)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	static_cast<PhysDoF*>(constraint.get())->SetLinearLimitSoftness(softness);
}
void Lua::PhysDoFConstraint::GetLinearForceMixingFactor(lua_State *l,DoFConstraintHandle &constraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	Lua::Push<Vector3>(l,static_cast<PhysDoF*>(constraint.get())->GetLinearForceMixingFactor());
}
void Lua::PhysDoFConstraint::SetLinearForceMixingFactor(lua_State *l,DoFConstraintHandle &constraint,const Vector3 &factor)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	static_cast<PhysDoF*>(constraint.get())->SetLinearForceMixingFactor(factor);
}
void Lua::PhysDoFConstraint::GetLinearLimitErrorTolerance(lua_State *l,DoFConstraintHandle &constraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	Lua::Push<Vector3>(l,static_cast<PhysDoF*>(constraint.get())->GetLinearLimitErrorTolerance());
}
void Lua::PhysDoFConstraint::SetLinearLimitErrorTolerance(lua_State *l,DoFConstraintHandle &constraint,const Vector3 &tolerance)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	static_cast<PhysDoF*>(constraint.get())->SetLinearLimitErrorTolerance(tolerance);
}
void Lua::PhysDoFConstraint::GetLinearLimitForceMixingFactor(lua_State *l,DoFConstraintHandle &constraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	Lua::Push<Vector3>(l,static_cast<PhysDoF*>(constraint.get())->GetLinearLimitForceMixingFactor());
}
void Lua::PhysDoFConstraint::SetLinearLimitForceMixingFactor(lua_State *l,DoFConstraintHandle &constraint,const Vector3 &factor)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	static_cast<PhysDoF*>(constraint.get())->SetLinearLimitForceMixingFactor(factor);
}

void Lua::PhysDoFConstraint::GetLinearRestitutionFactor(lua_State *l,DoFConstraintHandle &constraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	Lua::PushNumber(l,static_cast<PhysDoF*>(constraint.get())->GetLinearRestitutionFactor());
}
void Lua::PhysDoFConstraint::SetLinearRestitutionFactor(lua_State *l,DoFConstraintHandle &constraint,float factor)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	static_cast<PhysDoF*>(constraint.get())->SetLinearRestitutionFactor(factor);
}
void Lua::PhysDoFConstraint::IsLinearMotorEnabled(lua_State *l,DoFConstraintHandle &constraint,uint8_t axis)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	Lua::PushBool(l,static_cast<PhysDoF*>(constraint.get())->IsLinearMotorEnabled(axis));
}
void Lua::PhysDoFConstraint::SetLinearMotorEnabled(lua_State *l,DoFConstraintHandle &constraint,uint8_t axis,bool bEnabled)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	static_cast<PhysDoF*>(constraint.get())->SetLinearMotorEnabled(axis,bEnabled);
}
void Lua::PhysDoFConstraint::GetCurrentLinearDifference(lua_State *l,DoFConstraintHandle &constraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	Lua::Push<Vector3>(l,static_cast<PhysDoF*>(constraint.get())->GetCurrentLinearDifference());
}
void Lua::PhysDoFConstraint::GetCurrentLinearLimitError(lua_State *l,DoFConstraintHandle &constraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	Lua::Push<Vector3>(l,static_cast<PhysDoF*>(constraint.get())->GetCurrentLinearLimitError());
}
void Lua::PhysDoFConstraint::GetCurrentLinearLimit(lua_State *l,DoFConstraintHandle &constraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	Lua::Push<Vector3>(l,static_cast<PhysDoF*>(constraint.get())->GetCurrentLinearLimit());
}
void Lua::PhysDoFConstraint::GetCurrentLinearAccumulatedImpulse(lua_State *l,DoFConstraintHandle &constraint)
{
	LUA_CHECK_PHYS_CONSTRAINT(l,constraint);
	Lua::Push<Vector3>(l,static_cast<PhysDoF*>(constraint.get())->GetCurrentLinearAccumulatedImpulse());
}