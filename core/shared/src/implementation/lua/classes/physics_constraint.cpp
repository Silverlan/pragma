// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.classes.physics;

// #define ENABLE_DEPRECATED_PHYSICS

namespace Lua {
	namespace PhysConstraint {
		static void IsValid(lua::State *l, pragma::physics::IConstraint *hConstraint);
		static void Remove(lua::State *l, pragma::physics::IConstraint *hConstraint);
		static void GetSourceObject(lua::State *l, pragma::physics::IConstraint *hConstraint);
		static void GetTargetObject(lua::State *l, pragma::physics::IConstraint *hConstraint);
		static void GetSourceTransform(lua::State *l, pragma::physics::IConstraint *hConstraint);
		static void GetTargetTransform(lua::State *l, pragma::physics::IConstraint *hConstraint);
		static void GetSourcePosition(lua::State *l, pragma::physics::IConstraint *hConstraint);
		static void GetSourceRotation(lua::State *l, pragma::physics::IConstraint *hConstraint);
		static void GetTargetPosition(lua::State *l, pragma::physics::IConstraint *hConstraint);
		static void GetTargetRotation(lua::State *l, pragma::physics::IConstraint *hConstraint);

		static void GetBreakForce(lua::State *l, pragma::physics::IConstraint *hConstraint);
		static void SetBreakForce(lua::State *l, pragma::physics::IConstraint *hConstraint, float threshold);
		static void GetBreakTorque(lua::State *l, pragma::physics::IConstraint *hConstraint);
		static void SetBreakTorque(lua::State *l, pragma::physics::IConstraint *hConstraint, float threshold);

		static void SetEnabled(lua::State *l, pragma::physics::IConstraint *hConstraint, bool b);
		static void IsEnabled(lua::State *l, pragma::physics::IConstraint *hConstraint);
		static void SetCollisionsEnabled(lua::State *l, pragma::physics::IConstraint *hConstraint, bool b);
		static void GetCollisionsEnabled(lua::State *l, pragma::physics::IConstraint *hConstraint);
		static void EnableCollisions(lua::State *l, pragma::physics::IConstraint *hConstraint);
		static void DisableCollisions(lua::State *l, pragma::physics::IConstraint *hConstraint);
	};
	namespace PhysConeTwistConstraint {
		static void IsValid(lua::State *l, pragma::physics::IConeTwistConstraint *hConstraint);
		static void SetLimit(lua::State *l, pragma::physics::IConeTwistConstraint *constraint, float swingSpan1, float swingSpan2, float twistSpan);
		static void SetLimit(lua::State *l, pragma::physics::IConeTwistConstraint *constraint, const EulerAngles &ang);
	};
	namespace PhysDoFConstraint {
		static void IsValid(lua::State *l, pragma::physics::IDoFConstraint *hConstraint);
		static void SetLinearLowerLimit(lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &limit);
		static void SetLinearUpperLimit(lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &limit);
		static void SetLinearLimit(lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &lower, const Vector3 &upper);
		static void SetLinearLimit(lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &limit);
		static void SetAngularLowerLimit(lua::State *l, pragma::physics::IDoFConstraint *constraint, const EulerAngles &limit);
		static void SetAngularUpperLimit(lua::State *l, pragma::physics::IDoFConstraint *constraint, const EulerAngles &limit);
		static void SetAngularLimit(lua::State *l, pragma::physics::IDoFConstraint *constraint, const EulerAngles &lower, const EulerAngles &upper);
		static void SetAngularLimit(lua::State *l, pragma::physics::IDoFConstraint *constraint, const EulerAngles &limit);

		static void GetLinearLowerLimit(lua::State *l, pragma::physics::IDoFConstraint *constraint);
		static void GetlinearUpperLimit(lua::State *l, pragma::physics::IDoFConstraint *constraint);
		static void GetAngularLowerLimit(lua::State *l, pragma::physics::IDoFConstraint *constraint);
		static void GetAngularUpperLimit(lua::State *l, pragma::physics::IDoFConstraint *constraint);

		static void GetAngularTargetVelocity(lua::State *l, pragma::physics::IDoFConstraint *constraint);
		static void GetAngularMaxMotorForce(lua::State *l, pragma::physics::IDoFConstraint *constraint);
		static void SetAngularMaxMotorForce(lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &force);
		static void GetAngularMaxLimitForce(lua::State *l, pragma::physics::IDoFConstraint *constraint);
		static void SetAngularMaxLimitForce(lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &force);
		static void GetAngularDamping(lua::State *l, pragma::physics::IDoFConstraint *constraint);
		static void SetAngularDamping(lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &damping);
		static void GetAngularLimitSoftness(lua::State *l, pragma::physics::IDoFConstraint *constraint);
		static void SetAngularLimitSoftness(lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &softness);
		static void GetAngularForceMixingFactor(lua::State *l, pragma::physics::IDoFConstraint *constraint);
		static void SetAngularForceMixingFactor(lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &factor);
		static void GetAngularLimitErrorTolerance(lua::State *l, pragma::physics::IDoFConstraint *constraint);
		static void SetAngularLimitErrorTolerance(lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &tolerance);
		static void GetAngularLimitForceMixingFactor(lua::State *l, pragma::physics::IDoFConstraint *constraint);
		static void SetAngularLimitForceMixingFactor(lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &factor);
		static void GetAngularRestitutionFactor(lua::State *l, pragma::physics::IDoFConstraint *constraint);
		static void SetAngularRestitutionFactor(lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &factor);
		static void IsAngularMotorEnabled(lua::State *l, pragma::physics::IDoFConstraint *constraint, uint8_t axis);
		static void SetAngularMotorEnabled(lua::State *l, pragma::physics::IDoFConstraint *constraint, uint8_t axis, bool bEnabled);
		static void SetAngularMotorEnabled(lua::State *l, pragma::physics::IDoFConstraint *constraint, bool bEnabled);
		static void GetCurrentAngularLimitError(lua::State *l, pragma::physics::IDoFConstraint *constraint);
		static void GetCurrentAngularPosition(lua::State *l, pragma::physics::IDoFConstraint *constraint);
		static void GetCurrentAngularLimit(lua::State *l, pragma::physics::IDoFConstraint *constraint);
		static void GetCurrentAngularAccumulatedImpulse(lua::State *l, pragma::physics::IDoFConstraint *constraint);

		static void GetLinearTargetVelocity(lua::State *l, pragma::physics::IDoFConstraint *constraint);
		static void GetLinearMaxMotorForce(lua::State *l, pragma::physics::IDoFConstraint *constraint);
		static void SetLinearMaxMotorForce(lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &force);
		static void GetLinearDamping(lua::State *l, pragma::physics::IDoFConstraint *constraint);
		static void SetLinearDamping(lua::State *l, pragma::physics::IDoFConstraint *constraint, float damping);
		static void GetLinearLimitSoftness(lua::State *l, pragma::physics::IDoFConstraint *constraint);
		static void SetLinearLimitSoftness(lua::State *l, pragma::physics::IDoFConstraint *constraint, float softness);
		static void GetLinearForceMixingFactor(lua::State *l, pragma::physics::IDoFConstraint *constraint);
		static void SetLinearForceMixingFactor(lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &factor);
		static void GetLinearLimitErrorTolerance(lua::State *l, pragma::physics::IDoFConstraint *constraint);
		static void SetLinearLimitErrorTolerance(lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &tolerance);
		static void GetLinearLimitForceMixingFactor(lua::State *l, pragma::physics::IDoFConstraint *constraint);
		static void SetLinearLimitForceMixingFactor(lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &factor);

		static void GetLinearRestitutionFactor(lua::State *l, pragma::physics::IDoFConstraint *constraint);
		static void SetLinearRestitutionFactor(lua::State *l, pragma::physics::IDoFConstraint *constraint, float factor);
		static void IsLinearMotorEnabled(lua::State *l, pragma::physics::IDoFConstraint *constraint, uint8_t axis);
		static void SetLinearMotorEnabled(lua::State *l, pragma::physics::IDoFConstraint *constraint, uint8_t axis, bool bEnabled);
		static void SetLinearMotorEnabled(lua::State *l, pragma::physics::IDoFConstraint *constraint, bool bEnabled);
		static void GetCurrentLinearDifference(lua::State *l, pragma::physics::IDoFConstraint *constraint);
		static void GetCurrentLinearLimitError(lua::State *l, pragma::physics::IDoFConstraint *constraint);
		static void GetCurrentLinearLimit(lua::State *l, pragma::physics::IDoFConstraint *constraint);
		static void GetCurrentLinearAccumulatedImpulse(lua::State *l, pragma::physics::IDoFConstraint *constraint);
	};
};

void Lua::PhysConstraint::register_class(lua::State *l, luabind::module_ &mod)
{
	auto classDef = luabind::class_<pragma::physics::IConstraint>("Constraint");
	classDef.def("IsValid", &IsValid);
	classDef.def("Remove", &Remove);
	classDef.def("GetSourceActor", &GetSourceObject);
	classDef.def("GetTargetActor", &GetTargetObject);
	classDef.def("GetSourceTransform", &GetSourceTransform);
	classDef.def("GetTargetTransform", &GetTargetTransform);
	classDef.def("GetSourcePosition", &GetSourcePosition);
	classDef.def("GetSourceRotation", &GetSourceRotation);
	classDef.def("GetTargetPosition", &GetTargetPosition);
	classDef.def("GetTargetRotation", &GetTargetRotation);

	classDef.def("GetBreakForce", &GetBreakForce);
	classDef.def("SetBreakForce", &SetBreakForce);
	classDef.def("GetBreakTorque", &GetBreakTorque);
	classDef.def("SetBreakTorque", &SetBreakTorque);

	classDef.def("SetEnabled", &SetEnabled);
	classDef.def("IsEnabled", &IsEnabled);
	classDef.def("SetCollisionsEnabled", &SetCollisionsEnabled);
	classDef.def("GetCollisionsEnabled", &GetCollisionsEnabled);
	classDef.def("EnableCollisions", &EnableCollisions);
	classDef.def("DisableCollisions", &DisableCollisions);

	// Note: Constraints derived from the base 'Constraint' mustn't be in its scope because the base class 'Constraint' must be
	// fully defined before a derived class is registered, and scoped classes cannot be added AFTER a class was defined.
	// For this reason all constraint classes are in the 'phys' library instead.

	auto fixedClassDef = luabind::class_<pragma::physics::IFixedConstraint, pragma::physics::IConstraint>("FixedConstraint");
	fixedClassDef.def("IsValid", static_cast<void (*)(lua::State *, pragma::physics::IFixedConstraint *)>([](lua::State *l, pragma::physics::IFixedConstraint *constraint) { PushBool(l, constraint != nullptr); }));

	auto ballSocketClassDef = luabind::class_<pragma::physics::IBallSocketConstraint, pragma::physics::IConstraint>("BallSocketConstraint");
	ballSocketClassDef.def("IsValid", static_cast<void (*)(lua::State *, pragma::physics::IBallSocketConstraint *)>([](lua::State *l, pragma::physics::IBallSocketConstraint *constraint) { PushBool(l, constraint != nullptr); }));

	auto hingeClassDef = luabind::class_<pragma::physics::IHingeConstraint, pragma::physics::IConstraint>("HingeConstraint");
	hingeClassDef.def("IsValid", static_cast<void (*)(lua::State *, pragma::physics::IHingeConstraint *)>([](lua::State *l, pragma::physics::IHingeConstraint *constraint) { PushBool(l, constraint != nullptr); }));

	auto sliderClassDef = luabind::class_<pragma::physics::ISliderConstraint, pragma::physics::IConstraint>("SliderConstraint");
	sliderClassDef.def("IsValid", static_cast<void (*)(lua::State *, pragma::physics::ISliderConstraint *)>([](lua::State *l, pragma::physics::ISliderConstraint *constraint) { PushBool(l, constraint != nullptr); }));
	sliderClassDef.def("SetLimits", static_cast<void (*)(lua::State *, pragma::physics::ISliderConstraint *, float, float)>([](lua::State *l, pragma::physics::ISliderConstraint *constraint, float lower, float upper) {
		if(Lua::CheckHandle<pragma::physics::ISliderConstraint>(l, constraint) == false)
			return;
		constraint->SetLimit(lower, upper);
	}));
	sliderClassDef.def("SetSoftness", static_cast<void (*)(lua::State *, pragma::physics::ISliderConstraint *, float)>([](lua::State *l, pragma::physics::ISliderConstraint *constraint, float softness) {
		if(Lua::CheckHandle<pragma::physics::ISliderConstraint>(l, constraint) == false)
			return;
		constraint->SetSoftness(softness);
	}));
	sliderClassDef.def("SetDamping", static_cast<void (*)(lua::State *, pragma::physics::ISliderConstraint *, float)>([](lua::State *l, pragma::physics::ISliderConstraint *constraint, float damping) {
		if(Lua::CheckHandle<pragma::physics::ISliderConstraint>(l, constraint) == false)
			return;
		constraint->SetDamping(damping);
	}));
	sliderClassDef.def("SetRestitution", static_cast<void (*)(lua::State *, pragma::physics::ISliderConstraint *, float)>([](lua::State *l, pragma::physics::ISliderConstraint *constraint, float restitution) {
		if(Lua::CheckHandle<pragma::physics::ISliderConstraint>(l, constraint) == false)
			return;
		constraint->SetRestitution(restitution);
	}));
	sliderClassDef.def("DisableLimit", static_cast<void (*)(lua::State *, pragma::physics::ISliderConstraint *)>([](lua::State *l, pragma::physics::ISliderConstraint *constraint) {
		if(Lua::CheckHandle<pragma::physics::ISliderConstraint>(l, constraint) == false)
			return;
		constraint->DisableLimit();
	}));
	sliderClassDef.def("GetLimits", static_cast<void (*)(lua::State *, pragma::physics::ISliderConstraint *)>([](lua::State *l, pragma::physics::ISliderConstraint *constraint) {
		if(Lua::CheckHandle<pragma::physics::ISliderConstraint>(l, constraint) == false)
			return;
		auto limit = constraint->GetLimit();
		PushNumber(l, limit.first);
		PushNumber(l, limit.second);
	}));
	sliderClassDef.def("GetSoftness", static_cast<void (*)(lua::State *, pragma::physics::ISliderConstraint *)>([](lua::State *l, pragma::physics::ISliderConstraint *constraint) {
		if(Lua::CheckHandle<pragma::physics::ISliderConstraint>(l, constraint) == false)
			return;
		PushNumber(l, constraint->GetSoftness());
	}));
	sliderClassDef.def("GetDamping", static_cast<void (*)(lua::State *, pragma::physics::ISliderConstraint *)>([](lua::State *l, pragma::physics::ISliderConstraint *constraint) {
		if(Lua::CheckHandle<pragma::physics::ISliderConstraint>(l, constraint) == false)
			return;
		PushNumber(l, constraint->GetDamping());
	}));
	sliderClassDef.def("GetRestitution", static_cast<void (*)(lua::State *, pragma::physics::ISliderConstraint *)>([](lua::State *l, pragma::physics::ISliderConstraint *constraint) {
		if(Lua::CheckHandle<pragma::physics::ISliderConstraint>(l, constraint) == false)
			return;
		PushNumber(l, constraint->GetRestitution());
	}));

	auto coneTwistClassDef = luabind::class_<pragma::physics::IConeTwistConstraint, pragma::physics::IConstraint>("ConeTwistConstraint");
	coneTwistClassDef.def("IsValid", static_cast<void (*)(lua::State *, pragma::physics::IConeTwistConstraint *)>([](lua::State *l, pragma::physics::IConeTwistConstraint *constraint) { PushBool(l, constraint != nullptr); }));
	coneTwistClassDef.def("SetLimit", static_cast<void (*)(lua::State *, pragma::physics::IConeTwistConstraint *, float, float, float)>(&PhysConeTwistConstraint::SetLimit));
	coneTwistClassDef.def("SetLimit", static_cast<void (*)(lua::State *, pragma::physics::IConeTwistConstraint *, const EulerAngles &)>(&PhysConeTwistConstraint::SetLimit));

	auto DoFClassDef = luabind::class_<pragma::physics::IDoFConstraint, pragma::physics::IConstraint>("DoFConstraint");
	DoFClassDef.def("IsValid", static_cast<void (*)(lua::State *, pragma::physics::IDoFConstraint *)>([](lua::State *l, pragma::physics::IDoFConstraint *constraint) { PushBool(l, constraint != nullptr); }));
	DoFClassDef.def("SetLinearLowerLimit", &PhysDoFConstraint::SetLinearLowerLimit);
	DoFClassDef.def("SetLinearUpperLimit", &PhysDoFConstraint::SetLinearUpperLimit);
	DoFClassDef.def("SetLinearLimit", static_cast<void (*)(lua::State *, pragma::physics::IDoFConstraint *, const Vector3 &)>(&PhysDoFConstraint::SetLinearLimit));
	DoFClassDef.def("SetLinearLimit", static_cast<void (*)(lua::State *, pragma::physics::IDoFConstraint *, const Vector3 &, const Vector3 &)>(&PhysDoFConstraint::SetLinearLimit));

	DoFClassDef.def("SetAngularLowerLimit", &PhysDoFConstraint::SetAngularLowerLimit);
	DoFClassDef.def("SetAngularUpperLimit", &PhysDoFConstraint::SetAngularUpperLimit);
	DoFClassDef.def("SetAngularLimit", static_cast<void (*)(lua::State *, pragma::physics::IDoFConstraint *, const EulerAngles &)>(&PhysDoFConstraint::SetAngularLimit));
	DoFClassDef.def("SetAngularLimit", static_cast<void (*)(lua::State *, pragma::physics::IDoFConstraint *, const EulerAngles &, const EulerAngles &)>(&PhysDoFConstraint::SetAngularLimit));

	DoFClassDef.def("GetLinearLowerLimit", &PhysDoFConstraint::GetLinearLowerLimit);
	DoFClassDef.def("GetlinearUpperLimit", &PhysDoFConstraint::GetlinearUpperLimit);
	DoFClassDef.def("GetAngularLowerLimit", &PhysDoFConstraint::GetAngularLowerLimit);
	DoFClassDef.def("GetAngularUpperLimit", &PhysDoFConstraint::GetAngularUpperLimit);

	DoFClassDef.def("GetAngularTargetVelocity", &PhysDoFConstraint::GetAngularTargetVelocity);
	DoFClassDef.def("GetAngularMaxMotorForce", &PhysDoFConstraint::GetAngularMaxMotorForce);
	DoFClassDef.def("SetAngularMaxMotorForce", &PhysDoFConstraint::SetAngularMaxMotorForce);
	DoFClassDef.def("GetAngularMaxLimitForce", &PhysDoFConstraint::GetAngularMaxLimitForce);
	DoFClassDef.def("SetAngularMaxLimitForce", &PhysDoFConstraint::SetAngularMaxLimitForce);
	DoFClassDef.def("GetAngularDamping", &PhysDoFConstraint::GetAngularDamping);
	DoFClassDef.def("SetAngularDamping", &PhysDoFConstraint::SetAngularDamping);
	DoFClassDef.def("GetAngularLimitSoftness", &PhysDoFConstraint::GetAngularLimitSoftness);
	DoFClassDef.def("SetAngularLimitSoftness", &PhysDoFConstraint::SetAngularLimitSoftness);
	DoFClassDef.def("GetAngularForceMixingFactor", &PhysDoFConstraint::GetAngularForceMixingFactor);
	DoFClassDef.def("SetAngularForceMixingFactor", &PhysDoFConstraint::SetAngularForceMixingFactor);
	DoFClassDef.def("GetAngularLimitErrorTolerance", &PhysDoFConstraint::GetAngularLimitErrorTolerance);
	DoFClassDef.def("SetAngularLimitErrorTolerance", &PhysDoFConstraint::SetAngularLimitErrorTolerance);
	DoFClassDef.def("GetAngularLimitForceMixingFactor", &PhysDoFConstraint::GetAngularLimitForceMixingFactor);
	DoFClassDef.def("SetAngularLimitForceMixingFactor", &PhysDoFConstraint::SetAngularLimitForceMixingFactor);
	DoFClassDef.def("GetAngularRestitutionFactor", &PhysDoFConstraint::GetAngularRestitutionFactor);
	DoFClassDef.def("SetAngularRestitutionFactor", &PhysDoFConstraint::SetAngularRestitutionFactor);
	DoFClassDef.def("IsAngularMotorEnabled", &PhysDoFConstraint::IsAngularMotorEnabled);
	DoFClassDef.def("SetAngularMotorEnabled", static_cast<void (*)(lua::State *, pragma::physics::IDoFConstraint *, uint8_t, bool)>(&PhysDoFConstraint::SetAngularMotorEnabled));
	DoFClassDef.def("SetAngularMotorEnabled", static_cast<void (*)(lua::State *, pragma::physics::IDoFConstraint *, bool)>(&PhysDoFConstraint::SetAngularMotorEnabled));
	DoFClassDef.def("GetCurrentAngularLimitError", &PhysDoFConstraint::GetCurrentAngularLimitError);
	DoFClassDef.def("GetCurrentAngularPosition", &PhysDoFConstraint::GetCurrentAngularPosition);
	DoFClassDef.def("GetCurrentAngularLimit", &PhysDoFConstraint::GetCurrentAngularLimit);
	DoFClassDef.def("GetCurrentAngularAccumulatedImpulse", &PhysDoFConstraint::GetCurrentAngularAccumulatedImpulse);
	DoFClassDef.def("SetAngularTargetVelocity", static_cast<void (*)(lua::State *, pragma::physics::IDoFConstraint *, const Vector3 &)>([](lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &velocity) {
		if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
			return;
		constraint->SetAngularTargetVelocity(velocity);
	}));

	DoFClassDef.def("GetLinearTargetVelocity", &PhysDoFConstraint::GetLinearTargetVelocity);
	DoFClassDef.def("GetLinearMaxMotorForce", &PhysDoFConstraint::GetLinearMaxMotorForce);
	DoFClassDef.def("SetLinearMaxMotorForce", &PhysDoFConstraint::SetLinearMaxMotorForce);
	DoFClassDef.def("GetLinearDamping", &PhysDoFConstraint::GetLinearDamping);
	DoFClassDef.def("SetLinearDamping", &PhysDoFConstraint::SetLinearDamping);
	DoFClassDef.def("GetLinearLimitSoftness", &PhysDoFConstraint::GetLinearLimitSoftness);
	DoFClassDef.def("SetLinearLimitSoftness", &PhysDoFConstraint::SetLinearLimitSoftness);
	DoFClassDef.def("GetLinearForceMixingFactor", &PhysDoFConstraint::GetLinearForceMixingFactor);
	DoFClassDef.def("SetLinearForceMixingFactor", &PhysDoFConstraint::SetLinearForceMixingFactor);
	DoFClassDef.def("GetLinearLimitErrorTolerance", &PhysDoFConstraint::GetLinearLimitErrorTolerance);
	DoFClassDef.def("SetLinearLimitErrorTolerance", &PhysDoFConstraint::SetLinearLimitErrorTolerance);
	DoFClassDef.def("GetLinearLimitForceMixingFactor", &PhysDoFConstraint::GetLinearLimitForceMixingFactor);
	DoFClassDef.def("SetLinearLimitForceMixingFactor", &PhysDoFConstraint::SetLinearLimitForceMixingFactor);
	DoFClassDef.def("GetLinearRestitutionFactor", &PhysDoFConstraint::GetLinearRestitutionFactor);
	DoFClassDef.def("SetLinearRestitutionFactor", &PhysDoFConstraint::SetLinearRestitutionFactor);
	DoFClassDef.def("IsLinearMotorEnabled", &PhysDoFConstraint::IsLinearMotorEnabled);
	DoFClassDef.def("SetLinearMotorEnabled", static_cast<void (*)(lua::State *, pragma::physics::IDoFConstraint *, uint8_t, bool)>(&PhysDoFConstraint::SetLinearMotorEnabled));
	DoFClassDef.def("SetLinearMotorEnabled", static_cast<void (*)(lua::State *, pragma::physics::IDoFConstraint *, bool)>(&PhysDoFConstraint::SetLinearMotorEnabled));
	DoFClassDef.def("GetCurrentLinearDifference", &PhysDoFConstraint::GetCurrentLinearDifference);
	DoFClassDef.def("GetCurrentLinearLimitError", &PhysDoFConstraint::GetCurrentLinearLimitError);
	DoFClassDef.def("GetCurrentLinearLimit", &PhysDoFConstraint::GetCurrentLinearLimit);
	DoFClassDef.def("GetCurrentLinearAccumulatedImpulse", &PhysDoFConstraint::GetCurrentLinearAccumulatedImpulse);
	DoFClassDef.def("SetLinearTargetVelocity", static_cast<void (*)(lua::State *, pragma::physics::IDoFConstraint *, const Vector3 &)>([](lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &velocity) {
		if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
			return;
		constraint->SetLinearTargetVelocity(velocity);
	}));

	auto doFSprintClassDef = luabind::class_<pragma::physics::IDoFSpringConstraint, pragma::physics::IConstraint>("DoFSpringConstraint");
	doFSprintClassDef.def("IsValid", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint) { PushBool(l, constraint != nullptr); }));
	doFSprintClassDef.def("CalculateTransforms", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		constraint->CalculateTransforms();
	}));
	doFSprintClassDef.def("CalculateTransforms",
	  static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, const pragma::math::Transform &, const pragma::math::Transform &)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, const pragma::math::Transform &transformA, const pragma::math::Transform &transformB) {
		  if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			  return;
		  constraint->CalculateTransforms(transformA, transformB);
	  }));
#ifdef ENABLE_DEPRECATED_PHYSICS
	doFSprintClassDef.def("GetRotationalLimitMotor", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, uint32_t)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, uint32_t axis) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		auto *pRotLimitMotor = constraint->GetRotationalLimitMotor(static_cast<pragma::Axis>(axis));
		if(pRotLimitMotor == nullptr)
			return;
		Lua::Push<btRotationalLimitMotor2 *>(l, pRotLimitMotor);
	}));
	doFSprintClassDef.def("GetTranslationalLimitMotor", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		auto *pTransLimitMotor = constraint->GetTranslationalLimitMotor();
		if(pTransLimitMotor == nullptr)
			return;
		Lua::Push<btTranslationalLimitMotor2 *>(l, pTransLimitMotor);
	}));
#endif
	doFSprintClassDef.def("GetCalculatedTransformA", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		Lua::Push<pragma::math::Transform>(l, constraint->GetCalculatedTransformA());
	}));
	doFSprintClassDef.def("GetCalculatedTransformB", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		Lua::Push<pragma::math::Transform>(l, constraint->GetCalculatedTransformB());
	}));
	doFSprintClassDef.def("GetFrameOffsetA", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		Lua::Push<pragma::math::Transform>(l, constraint->GetFrameOffsetA());
	}));
	doFSprintClassDef.def("GetFrameOffsetB", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		Lua::Push<pragma::math::Transform>(l, constraint->GetFrameOffsetB());
	}));
	doFSprintClassDef.def("GetAxis", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, uint32_t)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, uint32_t axis) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		Lua::Push<Vector3>(l, constraint->GetAxis(static_cast<pragma::Axis>(axis)));
	}));
	doFSprintClassDef.def("GetAngle", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, uint32_t)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, uint32_t axis) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		PushNumber(l, constraint->GetAngle(static_cast<pragma::Axis>(axis)));
	}));
	doFSprintClassDef.def("GetRelativePivotPosition", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, uint32_t)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, uint32_t axis) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		PushNumber(l, constraint->GetRelativePivotPosition(static_cast<pragma::Axis>(axis)));
	}));
	doFSprintClassDef.def("SetFrames",
	  static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, const pragma::math::Transform &, const pragma::math::Transform &)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, const pragma::math::Transform &transformA, const pragma::math::Transform &transformB) {
		  if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			  return;
		  constraint->SetFrames(transformA, transformB);
	  }));
	doFSprintClassDef.def("SetLinearLowerLimit", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, const Vector3 &)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, const Vector3 &linearLower) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		constraint->SetLinearLowerLimit(linearLower);
	}));
	doFSprintClassDef.def("GetLinearLowerLimit", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		Lua::Push<Vector3>(l, constraint->GetLinearLowerLimit());
	}));
	doFSprintClassDef.def("SetLinearUpperLimit", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, const Vector3 &)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, const Vector3 &linearUpper) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		constraint->SetLinearUpperLimit(linearUpper);
	}));
	doFSprintClassDef.def("GetLinearUpperLimit", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		Lua::Push<Vector3>(l, constraint->GetLinearUpperLimit());
	}));
	doFSprintClassDef.def("SetAngularLowerLimit", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, const Vector3 &)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, const Vector3 &angularLower) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		constraint->SetAngularLowerLimit(angularLower);
	}));
	doFSprintClassDef.def("SetAngularLowerLimitReversed", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, const Vector3 &)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, const Vector3 &angularLower) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		constraint->SetAngularLowerLimitReversed(angularLower);
	}));
	doFSprintClassDef.def("GetAngularLowerLimit", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		Lua::Push<Vector3>(l, constraint->GetAngularLowerLimit());
	}));
	doFSprintClassDef.def("GetAngularLowerLimitReversed", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		Lua::Push<Vector3>(l, constraint->GetAngularLowerLimitReversed());
	}));
	doFSprintClassDef.def("SetAngularUpperLimit", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, const Vector3 &)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, const Vector3 &angularUpper) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		constraint->SetAngularUpperLimit(angularUpper);
	}));
	doFSprintClassDef.def("SetAngularUpperLimitReversed", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, const Vector3 &)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, const Vector3 &angularUpper) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		constraint->SetAngularUpperLimitReversed(angularUpper);
	}));
	doFSprintClassDef.def("GetAngularUpperLimit", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		Lua::Push<Vector3>(l, constraint->GetAngularUpperLimit());
	}));
	doFSprintClassDef.def("GetAngularUpperLimitReversed", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		Lua::Push<Vector3>(l, constraint->GetAngularUpperLimitReversed());
	}));
	doFSprintClassDef.def("SetLimit", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, uint32_t, uint32_t, double, double)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, uint32_t type, uint32_t axis, double lo, double hi) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		constraint->SetLimit(static_cast<pragma::physics::IDoFSpringConstraint::AxisType>(type), static_cast<pragma::Axis>(axis), lo, hi);
	}));
	doFSprintClassDef.def("SetLimitReversed", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, uint32_t, uint32_t, double, double)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, uint32_t type, uint32_t axis, double lo, double hi) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		constraint->SetLimitReversed(static_cast<pragma::physics::IDoFSpringConstraint::AxisType>(type), static_cast<pragma::Axis>(axis), lo, hi);
	}));
	doFSprintClassDef.def("IsLimited", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, uint32_t, uint32_t)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, uint32_t axis, uint32_t type) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		PushBool(l, constraint->IsLimited(static_cast<pragma::physics::IDoFSpringConstraint::AxisType>(type), static_cast<pragma::Axis>(axis)));
	}));
	doFSprintClassDef.def("SetRotationOrder", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, uint32_t)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, uint32_t rotationOrder) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		constraint->SetRotationOrder(static_cast<pragma::RotationOrder>(rotationOrder));
	}));
	doFSprintClassDef.def("GetRotationOrder", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		PushInt(l, pragma::math::to_integral(constraint->GetRotationOrder()));
	}));
	doFSprintClassDef.def("SetAxis", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, const Vector3 &, const Vector3 &)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, const Vector3 &axis1, const Vector3 &axis2) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		constraint->SetAxis(axis1, axis2);
	}));
	doFSprintClassDef.def("SetBounce", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, uint32_t, uint32_t, double)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, uint32_t type, uint32_t axis, double bounce) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		constraint->SetBounce(static_cast<pragma::physics::IDoFSpringConstraint::AxisType>(type), static_cast<pragma::Axis>(axis), bounce);
	}));
	doFSprintClassDef.def("EnableMotor", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, uint32_t, uint32_t, bool)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, uint32_t type, uint32_t axis, bool enable) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		constraint->EnableMotor(static_cast<pragma::physics::IDoFSpringConstraint::AxisType>(type), static_cast<pragma::Axis>(axis), enable);
	}));
	doFSprintClassDef.def("SetServo", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, uint32_t, uint32_t, bool)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, uint32_t type, uint32_t axis, bool enable) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		constraint->SetServo(static_cast<pragma::physics::IDoFSpringConstraint::AxisType>(type), static_cast<pragma::Axis>(axis), enable);
	}));
	doFSprintClassDef.def("SetTargetVelocity", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, uint32_t, uint32_t, double)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, uint32_t type, uint32_t axis, double velocity) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		constraint->SetTargetVelocity(static_cast<pragma::physics::IDoFSpringConstraint::AxisType>(type), static_cast<pragma::Axis>(axis), velocity);
	}));
	doFSprintClassDef.def("SetServoTarget", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, uint32_t, uint32_t, double)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, uint32_t type, uint32_t axis, double target) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		constraint->SetServoTarget(static_cast<pragma::physics::IDoFSpringConstraint::AxisType>(type), static_cast<pragma::Axis>(axis), target);
	}));
	doFSprintClassDef.def("SetMaxMotorForce", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, uint32_t, uint32_t, double)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, uint32_t type, uint32_t axis, double force) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		constraint->SetMaxMotorForce(static_cast<pragma::physics::IDoFSpringConstraint::AxisType>(type), static_cast<pragma::Axis>(axis), force);
	}));
	doFSprintClassDef.def("EnableSpring", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, uint32_t, uint32_t, bool)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, uint32_t type, uint32_t axis, bool enable) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		constraint->EnableSpring(static_cast<pragma::physics::IDoFSpringConstraint::AxisType>(type), static_cast<pragma::Axis>(axis), enable);
	}));
	doFSprintClassDef.def("SetStiffness",
	  static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, uint32_t, uint32_t, double, bool)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, uint32_t type, uint32_t axis, double stiffness, bool limitIfNeeded) {
		  if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			  return;
		  constraint->SetStiffness(static_cast<pragma::physics::IDoFSpringConstraint::AxisType>(type), static_cast<pragma::Axis>(axis), stiffness, limitIfNeeded);
	  }));
	doFSprintClassDef.def("SetStiffness", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, uint32_t, uint32_t, double)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, uint32_t type, uint32_t axis, double stiffness) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		constraint->SetStiffness(static_cast<pragma::physics::IDoFSpringConstraint::AxisType>(type), static_cast<pragma::Axis>(axis), stiffness);
	}));
	doFSprintClassDef.def("SetDamping", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, uint32_t, uint32_t, double, bool)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, uint32_t type, uint32_t axis, double damping, bool limitIfNeeded) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		constraint->SetDamping(static_cast<pragma::physics::IDoFSpringConstraint::AxisType>(type), static_cast<pragma::Axis>(axis), damping, limitIfNeeded);
	}));
	doFSprintClassDef.def("SetDamping", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, uint32_t, uint32_t, double)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, uint32_t type, uint32_t axis, double damping) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		constraint->SetDamping(static_cast<pragma::physics::IDoFSpringConstraint::AxisType>(type), static_cast<pragma::Axis>(axis), damping);
	}));
	doFSprintClassDef.def("SetEquilibriumPoint", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		constraint->SetEquilibriumPoint();
	}));
	doFSprintClassDef.def("SetEquilibriumPoint", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, uint32_t, uint32_t)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, uint32_t type, uint32_t axis) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		constraint->SetEquilibriumPoint(static_cast<pragma::physics::IDoFSpringConstraint::AxisType>(type), static_cast<pragma::Axis>(axis));
	}));
	doFSprintClassDef.def("SetEquilibriumPoint", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, uint32_t, uint32_t, double)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, uint32_t type, uint32_t axis, double value) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		constraint->SetEquilibriumPoint(static_cast<pragma::physics::IDoFSpringConstraint::AxisType>(type), static_cast<pragma::Axis>(axis), value);
	}));
	doFSprintClassDef.def("SetERP", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, uint32_t, uint32_t, double)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, uint32_t type, uint32_t axis, double value) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		constraint->SetERP(static_cast<pragma::physics::IDoFSpringConstraint::AxisType>(type), static_cast<pragma::Axis>(axis), value);
	}));
	doFSprintClassDef.def("GetERP", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, uint32_t, uint32_t)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, uint32_t type, uint32_t axis) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		PushNumber(l, constraint->GetERP(static_cast<pragma::physics::IDoFSpringConstraint::AxisType>(type), static_cast<pragma::Axis>(axis)));
	}));
	doFSprintClassDef.def("SetStopERP", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, uint32_t, uint32_t, double)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, uint32_t type, uint32_t axis, double value) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		constraint->SetStopERP(static_cast<pragma::physics::IDoFSpringConstraint::AxisType>(type), static_cast<pragma::Axis>(axis), value);
	}));
	doFSprintClassDef.def("GetStopERP", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, uint32_t, uint32_t)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, uint32_t type, uint32_t axis) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		PushNumber(l, constraint->GetStopERP(static_cast<pragma::physics::IDoFSpringConstraint::AxisType>(type), static_cast<pragma::Axis>(axis)));
	}));
	doFSprintClassDef.def("SetCFM", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, uint32_t, uint32_t, double)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, uint32_t type, uint32_t axis, double value) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		constraint->SetCFM(static_cast<pragma::physics::IDoFSpringConstraint::AxisType>(type), static_cast<pragma::Axis>(axis), value);
	}));
	doFSprintClassDef.def("GetCFM", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, uint32_t, uint32_t)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, uint32_t type, uint32_t axis) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		PushNumber(l, constraint->GetCFM(static_cast<pragma::physics::IDoFSpringConstraint::AxisType>(type), static_cast<pragma::Axis>(axis)));
	}));
	doFSprintClassDef.def("SetStopCFM", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, uint32_t, uint32_t, double)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, uint32_t type, uint32_t axis, double value) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		constraint->SetStopCFM(static_cast<pragma::physics::IDoFSpringConstraint::AxisType>(type), static_cast<pragma::Axis>(axis), value);
	}));
	doFSprintClassDef.def("GetStopCFM", static_cast<void (*)(lua::State *, pragma::physics::IDoFSpringConstraint *, uint32_t, uint32_t)>([](lua::State *l, pragma::physics::IDoFSpringConstraint *constraint, uint32_t type, uint32_t axis) {
		if(Lua::CheckHandle<pragma::physics::IDoFSpringConstraint>(l, constraint) == false)
			return;
		PushNumber(l, constraint->GetStopCFM(static_cast<pragma::physics::IDoFSpringConstraint::AxisType>(type), static_cast<pragma::Axis>(axis)));
	}));

#ifdef ENABLE_DEPRECATED_PHYSICS
	auto defRotLimitMotor = luabind::class_<btRotationalLimitMotor2>("RotationalLimitMotor");
	defRotLimitMotor.def_readwrite("loLimit", &btRotationalLimitMotor2::m_loLimit);
	defRotLimitMotor.def_readwrite("hiLimit", &btRotationalLimitMotor2::m_hiLimit);
	defRotLimitMotor.def_readwrite("bounce", &btRotationalLimitMotor2::m_bounce);
	defRotLimitMotor.def_readwrite("stopERP", &btRotationalLimitMotor2::m_stopERP);
	defRotLimitMotor.def_readwrite("stopCFM", &btRotationalLimitMotor2::m_stopCFM);
	defRotLimitMotor.def_readwrite("motorERP", &btRotationalLimitMotor2::m_motorERP);
	defRotLimitMotor.def_readwrite("motorCFM", &btRotationalLimitMotor2::m_motorCFM);
	defRotLimitMotor.def_readwrite("enableMotor", &btRotationalLimitMotor2::m_enableMotor);
	defRotLimitMotor.def_readwrite("targetVelocity", &btRotationalLimitMotor2::m_targetVelocity);
	defRotLimitMotor.def_readwrite("maxMotorForce", &btRotationalLimitMotor2::m_maxMotorForce);
	defRotLimitMotor.def_readwrite("servoMotor", &btRotationalLimitMotor2::m_servoMotor);
	defRotLimitMotor.def_readwrite("servoTarget", &btRotationalLimitMotor2::m_servoTarget);
	defRotLimitMotor.def_readwrite("enableSpring", &btRotationalLimitMotor2::m_enableSpring);
	defRotLimitMotor.def_readwrite("springStiffness", &btRotationalLimitMotor2::m_springStiffness);
	defRotLimitMotor.def_readwrite("springStiffnessLimited", &btRotationalLimitMotor2::m_springStiffnessLimited);
	defRotLimitMotor.def_readwrite("springDamping", &btRotationalLimitMotor2::m_springDamping);
	defRotLimitMotor.def_readwrite("springDampingLimited", &btRotationalLimitMotor2::m_springDampingLimited);
	defRotLimitMotor.def_readwrite("equilibriumPoint", &btRotationalLimitMotor2::m_equilibriumPoint);
	defRotLimitMotor.def_readwrite("currentLimitError", &btRotationalLimitMotor2::m_currentLimitError);
	defRotLimitMotor.def_readwrite("currentLimitErrorHi", &btRotationalLimitMotor2::m_currentLimitErrorHi);
	defRotLimitMotor.def_readwrite("currentPosition", &btRotationalLimitMotor2::m_currentPosition);
	defRotLimitMotor.def_readwrite("currentLimit", &btRotationalLimitMotor2::m_currentLimit);
	defRotLimitMotor.def("IsLimited", static_cast<void (*)(lua::State *, btRotationalLimitMotor2 &)>([](lua::State *l, btRotationalLimitMotor2 &rotLimitMotor) { Lua::PushBool(l, rotLimitMotor.isLimited()); }));
	doFSprintClassDef.scope[defRotLimitMotor];

	auto defTransLimitMotor = luabind::class_<btTranslationalLimitMotor2>("TranslationalLimitMotor");
	defTransLimitMotor.property("lowerLimit", static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor) { Lua::Push<Vector3>(l, uvec::create(transLimitMotor.m_lowerLimit / PhysEnv::WORLD_SCALE)); }),
	  static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &, const Vector3 &)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor, const Vector3 &limit) { transLimitMotor.m_lowerLimit = uvec::create_bt(limit) * PhysEnv::WORLD_SCALE; }));
	defTransLimitMotor.property("upperLimit", static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor) { Lua::Push<Vector3>(l, uvec::create(transLimitMotor.m_upperLimit / PhysEnv::WORLD_SCALE)); }),
	  static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &, const Vector3 &)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor, const Vector3 &limit) { transLimitMotor.m_upperLimit = uvec::create_bt(limit) * PhysEnv::WORLD_SCALE; }));
	defTransLimitMotor.property("bounce", static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor) { Lua::Push<Vector3>(l, uvec::create(transLimitMotor.m_bounce)); }),
	  static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &, const Vector3 &)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor, const Vector3 &bounce) { transLimitMotor.m_bounce = uvec::create_bt(bounce); }));
	defTransLimitMotor.property("stopERP", static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor) { Lua::Push<Vector3>(l, uvec::create(transLimitMotor.m_stopERP)); }),
	  static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &, const Vector3 &)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor, const Vector3 &erp) { transLimitMotor.m_stopERP = uvec::create_bt(erp); }));
	defTransLimitMotor.property("stopCFM", static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor) { Lua::Push<Vector3>(l, uvec::create(transLimitMotor.m_stopCFM)); }),
	  static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &, const Vector3 &)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor, const Vector3 &cfm) { transLimitMotor.m_stopCFM = uvec::create_bt(cfm); }));
	defTransLimitMotor.property("motorERP", static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor) { Lua::Push<Vector3>(l, uvec::create(transLimitMotor.m_motorERP)); }),
	  static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &, const Vector3 &)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor, const Vector3 &motorErp) { transLimitMotor.m_motorERP = uvec::create_bt(motorErp); }));
	defTransLimitMotor.property("motorCFM", static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor) { Lua::Push<Vector3>(l, uvec::create(transLimitMotor.m_motorCFM)); }),
	  static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &, const Vector3 &)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor, const Vector3 &motorCfm) { transLimitMotor.m_motorCFM = uvec::create_bt(motorCfm); }));
	defTransLimitMotor.property("servoMotor", static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &, uint32_t)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor, uint32_t axis) { Lua::PushBool(l, transLimitMotor.m_servoMotor[axis]); }),
	  static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &, uint32_t, bool)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor, uint32_t axis, bool servoMotor) { transLimitMotor.m_servoMotor[axis] = servoMotor; }));
	defTransLimitMotor.property("enableSpring", static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &, uint32_t)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor, uint32_t axis) { Lua::PushBool(l, transLimitMotor.m_enableSpring[axis]); }),
	  static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &, uint32_t, bool)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor, uint32_t axis, bool spring) { transLimitMotor.m_enableSpring[axis] = spring; }));
	defTransLimitMotor.property("servoTarget", static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor) { Lua::Push<Vector3>(l, uvec::create(transLimitMotor.m_servoTarget)); }),
	  static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &, const Vector3 &)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor, const Vector3 &servoTarget) { transLimitMotor.m_servoTarget = uvec::create_bt(servoTarget); }));
	defTransLimitMotor.property("springStiffness", static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor) { Lua::Push<Vector3>(l, uvec::create(transLimitMotor.m_springStiffness)); }),
	  static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &, const Vector3 &)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor, const Vector3 &stiffness) { transLimitMotor.m_springStiffness = uvec::create_bt(stiffness); }));
	defTransLimitMotor.property("springStiffnessLimited", static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &, uint32_t)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor, uint32_t axis) { Lua::PushBool(l, transLimitMotor.m_springStiffnessLimited[axis]); }),
	  static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &, uint32_t, bool)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor, uint32_t axis, bool stiffnessLimited) { transLimitMotor.m_springStiffnessLimited[axis] = stiffnessLimited; }));
	defTransLimitMotor.property("springDamping", static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor) { Lua::Push<Vector3>(l, uvec::create(transLimitMotor.m_springDamping)); }),
	  static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &, const Vector3 &)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor, const Vector3 &damping) { transLimitMotor.m_springDamping = uvec::create_bt(damping); }));
	defTransLimitMotor.property("springDampingLimited", static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &, uint32_t)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor, uint32_t axis) { Lua::PushBool(l, transLimitMotor.m_springDampingLimited[axis]); }),
	  static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &, uint32_t, bool)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor, uint32_t axis, bool dampingLimited) { transLimitMotor.m_springDampingLimited[axis] = dampingLimited; }));
	defTransLimitMotor.property("equilibriumPoint", static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor) { Lua::Push<Vector3>(l, uvec::create(transLimitMotor.m_equilibriumPoint / PhysEnv::WORLD_SCALE)); }),
	  static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &, const Vector3 &)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor, const Vector3 &equiPoint) { transLimitMotor.m_equilibriumPoint = uvec::create_bt(equiPoint) * PhysEnv::WORLD_SCALE; }));
	defTransLimitMotor.property("targetVelocity", static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor) { Lua::Push<Vector3>(l, uvec::create(transLimitMotor.m_targetVelocity / PhysEnv::WORLD_SCALE)); }),
	  static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &, const Vector3 &)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor, const Vector3 &targetVel) { transLimitMotor.m_targetVelocity = uvec::create_bt(targetVel) * PhysEnv::WORLD_SCALE; }));
	defTransLimitMotor.property("maxMotorForce", static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor) { Lua::Push<Vector3>(l, uvec::create(transLimitMotor.m_maxMotorForce / PhysEnv::WORLD_SCALE)); }),
	  static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &, const Vector3 &)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor, const Vector3 &maxMotorForce) { transLimitMotor.m_maxMotorForce = uvec::create_bt(maxMotorForce) * PhysEnv::WORLD_SCALE; }));
	defTransLimitMotor.property("currentLimitError", static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor) { Lua::Push<Vector3>(l, uvec::create(transLimitMotor.m_currentLimitError / PhysEnv::WORLD_SCALE)); }),
	  static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &, const Vector3 &)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor, const Vector3 &limitErr) { transLimitMotor.m_currentLimitError = uvec::create_bt(limitErr) * PhysEnv::WORLD_SCALE; }));
	defTransLimitMotor.property("currentLimitErrorHi", static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor) { Lua::Push<Vector3>(l, uvec::create(transLimitMotor.m_currentLimitErrorHi / PhysEnv::WORLD_SCALE)); }),
	  static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &, const Vector3 &)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor, const Vector3 &limitErrHi) { transLimitMotor.m_currentLimitErrorHi = uvec::create_bt(limitErrHi) * PhysEnv::WORLD_SCALE; }));
	defTransLimitMotor.property("currentLinearDiff", static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor) { Lua::Push<Vector3>(l, uvec::create(transLimitMotor.m_currentLinearDiff / PhysEnv::WORLD_SCALE)); }),
	  static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &, const Vector3 &)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor, const Vector3 &linearDiff) { transLimitMotor.m_currentLinearDiff = uvec::create_bt(linearDiff) * PhysEnv::WORLD_SCALE; }));
	defTransLimitMotor.property("currentLimit", static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &, uint32_t)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor, uint32_t axis) { Lua::PushInt(l, transLimitMotor.m_currentLimit[axis]); }),
	  static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &, uint32_t, int32_t)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor, uint32_t axis, int32_t currentLimit) { transLimitMotor.m_currentLimit[axis] = currentLimit; }));
	defTransLimitMotor.def("IsLimited", static_cast<void (*)(lua::State *, btTranslationalLimitMotor2 &, uint32_t)>([](lua::State *l, btTranslationalLimitMotor2 &transLimitMotor, uint32_t axis) { Lua::PushBool(l, transLimitMotor.isLimited(axis)); }));
	doFSprintClassDef.scope[defTransLimitMotor];
#endif
	mod[classDef];
	mod[ballSocketClassDef];
	mod[doFSprintClassDef];
	mod[DoFClassDef];
	mod[coneTwistClassDef];
	mod[sliderClassDef];
	mod[fixedClassDef];
	mod[hingeClassDef];
}

void Lua::PhysConstraint::IsValid(lua::State *l, pragma::physics::IConstraint *hConstraint) { PushBool(l, hConstraint != nullptr); }
void Lua::PhysConstraint::GetSourceObject(lua::State *l, pragma::physics::IConstraint *hConstraint)
{
	if(Lua::CheckHandle<pragma::physics::IConstraint>(l, hConstraint) == false)
		return;
	auto *o = hConstraint->GetSourceActor();
	if(o == nullptr)
		return;
	o->Push(l);
}
void Lua::PhysConstraint::GetTargetObject(lua::State *l, pragma::physics::IConstraint *hConstraint)
{
	if(Lua::CheckHandle<pragma::physics::IConstraint>(l, hConstraint) == false)
		return;
	auto *o = hConstraint->GetTargetActor();
	if(o == nullptr)
		return;
	o->Push(l);
}
void Lua::PhysConstraint::Remove(lua::State *l, pragma::physics::IConstraint *hConstraint)
{
	if(Lua::CheckHandle<pragma::physics::IConstraint>(l, hConstraint) == false)
		return;
	hConstraint->ClaimOwnership().Remove();
}
void Lua::PhysConstraint::GetSourceTransform(lua::State *l, pragma::physics::IConstraint *hConstraint)
{
	if(Lua::CheckHandle<pragma::physics::IConstraint>(l, hConstraint) == false)
		return;
	Lua::Push<pragma::math::Transform>(l, hConstraint->GetSourceTransform());
}
void Lua::PhysConstraint::GetTargetTransform(lua::State *l, pragma::physics::IConstraint *hConstraint)
{
	if(Lua::CheckHandle<pragma::physics::IConstraint>(l, hConstraint) == false)
		return;
	Lua::Push<pragma::math::Transform>(l, hConstraint->GetTargetTransform());
}
void Lua::PhysConstraint::GetSourcePosition(lua::State *l, pragma::physics::IConstraint *hConstraint)
{
	if(Lua::CheckHandle<pragma::physics::IConstraint>(l, hConstraint) == false)
		return;
	Lua::Push<Vector3>(l, hConstraint->GetSourcePosition());
}
void Lua::PhysConstraint::GetSourceRotation(lua::State *l, pragma::physics::IConstraint *hConstraint)
{
	if(Lua::CheckHandle<pragma::physics::IConstraint>(l, hConstraint) == false)
		return;
	Lua::Push<Quat>(l, hConstraint->GetSourceRotation());
}
void Lua::PhysConstraint::GetTargetPosition(lua::State *l, pragma::physics::IConstraint *hConstraint)
{
	if(Lua::CheckHandle<pragma::physics::IConstraint>(l, hConstraint) == false)
		return;
	Lua::Push<Vector3>(l, hConstraint->GetTargetPosition());
}
void Lua::PhysConstraint::GetTargetRotation(lua::State *l, pragma::physics::IConstraint *hConstraint)
{
	if(Lua::CheckHandle<pragma::physics::IConstraint>(l, hConstraint) == false)
		return;
	Lua::Push<Quat>(l, hConstraint->GetTargetRotation());
}
void Lua::PhysConstraint::GetBreakForce(lua::State *l, pragma::physics::IConstraint *hConstraint)
{
	if(Lua::CheckHandle<pragma::physics::IConstraint>(l, hConstraint) == false)
		return;
	PushNumber(l, hConstraint->GetBreakForce());
}
void Lua::PhysConstraint::SetBreakForce(lua::State *l, pragma::physics::IConstraint *hConstraint, float threshold)
{
	if(Lua::CheckHandle<pragma::physics::IConstraint>(l, hConstraint) == false)
		return;
	hConstraint->SetBreakForce(threshold);
}
void Lua::PhysConstraint::GetBreakTorque(lua::State *l, pragma::physics::IConstraint *hConstraint)
{
	if(Lua::CheckHandle<pragma::physics::IConstraint>(l, hConstraint) == false)
		return;
	PushNumber(l, hConstraint->GetBreakTorque());
}
void Lua::PhysConstraint::SetBreakTorque(lua::State *l, pragma::physics::IConstraint *hConstraint, float threshold)
{
	if(Lua::CheckHandle<pragma::physics::IConstraint>(l, hConstraint) == false)
		return;
	hConstraint->SetBreakTorque(threshold);
}
void Lua::PhysConstraint::SetEnabled(lua::State *l, pragma::physics::IConstraint *hConstraint, bool b)
{
	if(Lua::CheckHandle<pragma::physics::IConstraint>(l, hConstraint) == false)
		return;
	hConstraint->SetEnabled(b);
}
void Lua::PhysConstraint::IsEnabled(lua::State *l, pragma::physics::IConstraint *hConstraint)
{
	if(Lua::CheckHandle<pragma::physics::IConstraint>(l, hConstraint) == false)
		return;
	PushBool(l, hConstraint->IsEnabled());
}
void Lua::PhysConstraint::SetCollisionsEnabled(lua::State *l, pragma::physics::IConstraint *hConstraint, bool b)
{
	if(Lua::CheckHandle<pragma::physics::IConstraint>(l, hConstraint) == false)
		return;
	hConstraint->SetCollisionsEnabled(b);
}
void Lua::PhysConstraint::GetCollisionsEnabled(lua::State *l, pragma::physics::IConstraint *hConstraint)
{
	if(Lua::CheckHandle<pragma::physics::IConstraint>(l, hConstraint) == false)
		return;
	PushBool(l, hConstraint->GetCollisionsEnabled());
}
void Lua::PhysConstraint::EnableCollisions(lua::State *l, pragma::physics::IConstraint *hConstraint)
{
	if(Lua::CheckHandle<pragma::physics::IConstraint>(l, hConstraint) == false)
		return;
	hConstraint->EnableCollisions();
}
void Lua::PhysConstraint::DisableCollisions(lua::State *l, pragma::physics::IConstraint *hConstraint)
{
	if(Lua::CheckHandle<pragma::physics::IConstraint>(l, hConstraint) == false)
		return;
	hConstraint->DisableCollisions();
}

void Lua::PhysConeTwistConstraint::SetLimit(lua::State *l, pragma::physics::IConeTwistConstraint *constraint, float swingSpan1, float swingSpan2, float twistSpan)
{
	if(Lua::CheckHandle<pragma::physics::IConeTwistConstraint>(l, constraint) == false)
		return;
	constraint->SetLimit(swingSpan1, swingSpan2, twistSpan);
}
void Lua::PhysConeTwistConstraint::SetLimit(lua::State *l, pragma::physics::IConeTwistConstraint *constraint, const EulerAngles &ang)
{
	if(Lua::CheckHandle<pragma::physics::IConeTwistConstraint>(l, constraint) == false)
		return;
	constraint->SetLimit(pragma::math::deg_to_rad(ang.p), pragma::math::deg_to_rad(ang.y), pragma::math::deg_to_rad(ang.r));
}

void Lua::PhysDoFConstraint::SetLinearLowerLimit(lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &limit)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	constraint->SetLinearLowerLimit(limit);
}
void Lua::PhysDoFConstraint::SetLinearUpperLimit(lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &limit)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	constraint->SetLinearUpperLimit(limit);
}
void Lua::PhysDoFConstraint::SetLinearLimit(lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &lower, const Vector3 &upper)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	constraint->SetLinearLimit(lower, upper);
}
void Lua::PhysDoFConstraint::SetLinearLimit(lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &limit)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	constraint->SetLinearLimit(limit);
}
void Lua::PhysDoFConstraint::SetAngularLowerLimit(lua::State *l, pragma::physics::IDoFConstraint *constraint, const EulerAngles &limit)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	constraint->SetAngularLowerLimit(limit);
}
void Lua::PhysDoFConstraint::SetAngularUpperLimit(lua::State *l, pragma::physics::IDoFConstraint *constraint, const EulerAngles &limit)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	constraint->SetAngularUpperLimit(limit);
}
void Lua::PhysDoFConstraint::SetAngularLimit(lua::State *l, pragma::physics::IDoFConstraint *constraint, const EulerAngles &lower, const EulerAngles &upper)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	constraint->SetAngularLimit(lower, upper);
}
void Lua::PhysDoFConstraint::SetAngularLimit(lua::State *l, pragma::physics::IDoFConstraint *constraint, const EulerAngles &limit)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	constraint->SetAngularLimit(limit);
}

void Lua::PhysDoFConstraint::GetLinearLowerLimit(lua::State *l, pragma::physics::IDoFConstraint *constraint)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	Lua::Push<Vector3>(l, constraint->GetLinearLowerLimit());
}
void Lua::PhysDoFConstraint::GetlinearUpperLimit(lua::State *l, pragma::physics::IDoFConstraint *constraint)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	Lua::Push<Vector3>(l, constraint->GetlinearUpperLimit());
}
void Lua::PhysDoFConstraint::GetAngularLowerLimit(lua::State *l, pragma::physics::IDoFConstraint *constraint)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	Lua::Push<EulerAngles>(l, constraint->GetAngularLowerLimit());
}
void Lua::PhysDoFConstraint::GetAngularUpperLimit(lua::State *l, pragma::physics::IDoFConstraint *constraint)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	Lua::Push<EulerAngles>(l, constraint->GetAngularUpperLimit());
}

void Lua::PhysDoFConstraint::GetAngularTargetVelocity(lua::State *l, pragma::physics::IDoFConstraint *constraint)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	Lua::Push<Vector3>(l, constraint->GetAngularTargetVelocity());
}
void Lua::PhysDoFConstraint::GetAngularMaxMotorForce(lua::State *l, pragma::physics::IDoFConstraint *constraint)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	Lua::Push<Vector3>(l, constraint->GetAngularMaxMotorForce());
}
void Lua::PhysDoFConstraint::SetAngularMaxMotorForce(lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &force)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	constraint->SetAngularMaxMotorForce(force);
}
void Lua::PhysDoFConstraint::GetAngularMaxLimitForce(lua::State *l, pragma::physics::IDoFConstraint *constraint)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	Lua::Push<Vector3>(l, constraint->GetAngularMaxLimitForce());
}
void Lua::PhysDoFConstraint::SetAngularMaxLimitForce(lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &force)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	constraint->SetAngularMaxLimitForce(force);
}
void Lua::PhysDoFConstraint::GetAngularDamping(lua::State *l, pragma::physics::IDoFConstraint *constraint)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	Lua::Push<Vector3>(l, constraint->GetAngularDamping());
}
void Lua::PhysDoFConstraint::SetAngularDamping(lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &damping)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	constraint->SetAngularDamping(damping);
}
void Lua::PhysDoFConstraint::GetAngularLimitSoftness(lua::State *l, pragma::physics::IDoFConstraint *constraint)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	Lua::Push<Vector3>(l, constraint->GetAngularLimitSoftness());
}
void Lua::PhysDoFConstraint::SetAngularLimitSoftness(lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &softness)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	constraint->SetAngularLimitSoftness(softness);
}
void Lua::PhysDoFConstraint::GetAngularForceMixingFactor(lua::State *l, pragma::physics::IDoFConstraint *constraint)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	Lua::Push<Vector3>(l, constraint->GetAngularForceMixingFactor());
}
void Lua::PhysDoFConstraint::SetAngularForceMixingFactor(lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &factor)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	constraint->SetAngularForceMixingFactor(factor);
}
void Lua::PhysDoFConstraint::GetAngularLimitErrorTolerance(lua::State *l, pragma::physics::IDoFConstraint *constraint)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	Lua::Push<Vector3>(l, constraint->GetAngularLimitErrorTolerance());
}
void Lua::PhysDoFConstraint::SetAngularLimitErrorTolerance(lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &tolerance)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	constraint->SetAngularLimitErrorTolerance(tolerance);
}
void Lua::PhysDoFConstraint::GetAngularLimitForceMixingFactor(lua::State *l, pragma::physics::IDoFConstraint *constraint)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	Lua::Push<Vector3>(l, constraint->GetAngularLimitForceMixingFactor());
}
void Lua::PhysDoFConstraint::SetAngularLimitForceMixingFactor(lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &factor)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	constraint->SetAngularLimitForceMixingFactor(factor);
}
void Lua::PhysDoFConstraint::GetAngularRestitutionFactor(lua::State *l, pragma::physics::IDoFConstraint *constraint)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	Lua::Push<Vector3>(l, constraint->GetAngularRestitutionFactor());
}
void Lua::PhysDoFConstraint::SetAngularRestitutionFactor(lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &factor)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	constraint->SetAngularRestitutionFactor(factor);
}
void Lua::PhysDoFConstraint::IsAngularMotorEnabled(lua::State *l, pragma::physics::IDoFConstraint *constraint, uint8_t axis)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	PushBool(l, constraint->IsAngularMotorEnabled(axis));
}
void Lua::PhysDoFConstraint::SetAngularMotorEnabled(lua::State *l, pragma::physics::IDoFConstraint *constraint, uint8_t axis, bool bEnabled)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	constraint->SetAngularMotorEnabled(axis, bEnabled);
}
void Lua::PhysDoFConstraint::SetAngularMotorEnabled(lua::State *l, pragma::physics::IDoFConstraint *constraint, bool bEnabled)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	for(uint32_t i = 0; i < 3; ++i)
		SetAngularMotorEnabled(l, constraint, i, bEnabled);
}
void Lua::PhysDoFConstraint::GetCurrentAngularLimitError(lua::State *l, pragma::physics::IDoFConstraint *constraint)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	Lua::Push<Vector3>(l, constraint->GetCurrentAngularLimitError());
}
void Lua::PhysDoFConstraint::GetCurrentAngularPosition(lua::State *l, pragma::physics::IDoFConstraint *constraint)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	Lua::Push<Vector3>(l, constraint->GetCurrentAngularPosition());
}
void Lua::PhysDoFConstraint::GetCurrentAngularLimit(lua::State *l, pragma::physics::IDoFConstraint *constraint)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	Lua::Push<Vector3>(l, constraint->GetCurrentAngularLimit());
}
void Lua::PhysDoFConstraint::GetCurrentAngularAccumulatedImpulse(lua::State *l, pragma::physics::IDoFConstraint *constraint)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	Lua::Push<Vector3>(l, constraint->GetCurrentAngularAccumulatedImpulse());
}
void Lua::PhysDoFConstraint::GetLinearTargetVelocity(lua::State *l, pragma::physics::IDoFConstraint *constraint)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	Lua::Push<Vector3>(l, constraint->GetLinearTargetVelocity());
}
void Lua::PhysDoFConstraint::GetLinearMaxMotorForce(lua::State *l, pragma::physics::IDoFConstraint *constraint)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	Lua::Push<Vector3>(l, constraint->GetLinearMaxMotorForce());
}
void Lua::PhysDoFConstraint::SetLinearMaxMotorForce(lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &force)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	constraint->SetLinearMaxMotorForce(force);
}
void Lua::PhysDoFConstraint::GetLinearDamping(lua::State *l, pragma::physics::IDoFConstraint *constraint)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	PushNumber(l, constraint->GetLinearDamping());
}
void Lua::PhysDoFConstraint::SetLinearDamping(lua::State *l, pragma::physics::IDoFConstraint *constraint, float damping)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	constraint->SetLinearDamping(damping);
}
void Lua::PhysDoFConstraint::GetLinearLimitSoftness(lua::State *l, pragma::physics::IDoFConstraint *constraint)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	PushNumber(l, constraint->GetLinearLimitSoftness());
}
void Lua::PhysDoFConstraint::SetLinearLimitSoftness(lua::State *l, pragma::physics::IDoFConstraint *constraint, float softness)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	constraint->SetLinearLimitSoftness(softness);
}
void Lua::PhysDoFConstraint::GetLinearForceMixingFactor(lua::State *l, pragma::physics::IDoFConstraint *constraint)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	Lua::Push<Vector3>(l, constraint->GetLinearForceMixingFactor());
}
void Lua::PhysDoFConstraint::SetLinearForceMixingFactor(lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &factor)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	constraint->SetLinearForceMixingFactor(factor);
}
void Lua::PhysDoFConstraint::GetLinearLimitErrorTolerance(lua::State *l, pragma::physics::IDoFConstraint *constraint)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	Lua::Push<Vector3>(l, constraint->GetLinearLimitErrorTolerance());
}
void Lua::PhysDoFConstraint::SetLinearLimitErrorTolerance(lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &tolerance)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	constraint->SetLinearLimitErrorTolerance(tolerance);
}
void Lua::PhysDoFConstraint::GetLinearLimitForceMixingFactor(lua::State *l, pragma::physics::IDoFConstraint *constraint)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	Lua::Push<Vector3>(l, constraint->GetLinearLimitForceMixingFactor());
}
void Lua::PhysDoFConstraint::SetLinearLimitForceMixingFactor(lua::State *l, pragma::physics::IDoFConstraint *constraint, const Vector3 &factor)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	constraint->SetLinearLimitForceMixingFactor(factor);
}

void Lua::PhysDoFConstraint::GetLinearRestitutionFactor(lua::State *l, pragma::physics::IDoFConstraint *constraint)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	PushNumber(l, constraint->GetLinearRestitutionFactor());
}
void Lua::PhysDoFConstraint::SetLinearRestitutionFactor(lua::State *l, pragma::physics::IDoFConstraint *constraint, float factor)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	constraint->SetLinearRestitutionFactor(factor);
}
void Lua::PhysDoFConstraint::IsLinearMotorEnabled(lua::State *l, pragma::physics::IDoFConstraint *constraint, uint8_t axis)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	PushBool(l, constraint->IsLinearMotorEnabled(axis));
}
void Lua::PhysDoFConstraint::SetLinearMotorEnabled(lua::State *l, pragma::physics::IDoFConstraint *constraint, uint8_t axis, bool bEnabled)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	constraint->SetLinearMotorEnabled(axis, bEnabled);
}
void Lua::PhysDoFConstraint::SetLinearMotorEnabled(lua::State *l, pragma::physics::IDoFConstraint *constraint, bool bEnabled)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	for(uint32_t i = 0; i < 3; ++i)
		SetLinearMotorEnabled(l, constraint, i, bEnabled);
}
void Lua::PhysDoFConstraint::GetCurrentLinearDifference(lua::State *l, pragma::physics::IDoFConstraint *constraint)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	Lua::Push<Vector3>(l, constraint->GetCurrentLinearDifference());
}
void Lua::PhysDoFConstraint::GetCurrentLinearLimitError(lua::State *l, pragma::physics::IDoFConstraint *constraint)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	Lua::Push<Vector3>(l, constraint->GetCurrentLinearLimitError());
}
void Lua::PhysDoFConstraint::GetCurrentLinearLimit(lua::State *l, pragma::physics::IDoFConstraint *constraint)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	Lua::Push<Vector3>(l, constraint->GetCurrentLinearLimit());
}
void Lua::PhysDoFConstraint::GetCurrentLinearAccumulatedImpulse(lua::State *l, pragma::physics::IDoFConstraint *constraint)
{
	if(Lua::CheckHandle<pragma::physics::IDoFConstraint>(l, constraint) == false)
		return;
	Lua::Push<Vector3>(l, constraint->GetCurrentLinearAccumulatedImpulse());
}
