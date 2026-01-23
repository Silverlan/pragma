// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:physics.constraint;

export import :entities.base_entity_handle;
export import :game.coordinate_system;
export import :game.enums;
export import :physics.shape;

export namespace pragma::physics {
	class IRigidBody;
	class DLLNETWORK IConstraint : public IBase, public IWorldObject {
	  public:
		virtual void OnRemove() override;
		virtual void SetEnabled(bool b) = 0;
		virtual bool IsEnabled() const = 0;
		virtual bool IsBroken() const = 0;
		virtual void Break() = 0;
		void SetCollisionsEnabled(bool b);
		Bool GetCollisionsEnabled() const;
		void EnableCollisions();
		void DisableCollisions();
		virtual IRigidBody *GetSourceActor() = 0;
		virtual IRigidBody *GetTargetActor() = 0;
		math::Transform &GetSourceTransform();
		math::Transform &GetTargetTransform();
		Vector3 GetSourcePosition();
		Quat GetSourceRotation();
		Vector3 GetTargetPosition();
		Quat GetTargetRotation();

		// Returns the entity this constraint belongs to (if any)
		ecs::BaseEntity *GetEntity() const;
		void SetEntity(ecs::BaseEntity &ent);

		virtual float GetBreakForce() const = 0;
		virtual void SetBreakForce(float threshold) = 0;
		virtual float GetBreakTorque() const = 0;
		virtual void SetBreakTorque(float torque) = 0;
		virtual void InitializeLuaObject(lua::State *lua) override;

		virtual void SetSoftness(float softness) = 0;
		virtual void SetDamping(float damping) = 0;
		virtual void SetRestitution(float restitution) = 0;

		virtual float GetSoftness() const = 0;
		virtual float GetDamping() const = 0;
		virtual float GetRestitution() const = 0;

		void OnBroken();
	  protected:
		IConstraint(IEnvironment &env);
		virtual void DoSetCollisionsEnabled(Bool b) = 0;
		math::Transform m_srcTransform;
		math::Transform m_tgtTransform;
		EntityHandle m_hEntity = {};
	  private:
		bool m_bCollisionsEnabled = true;
	};

	class DLLNETWORK IFixedConstraint : virtual public IConstraint {
	  public:
		virtual void InitializeLuaObject(lua::State *lua) override;
	  protected:
		using IConstraint::IConstraint;
	};

	class DLLNETWORK IBallSocketConstraint : virtual public IConstraint {
	  public:
		virtual void InitializeLuaObject(lua::State *lua) override;
	  protected:
		using IConstraint::IConstraint;
	};

	class DLLNETWORK IHingeConstraint : virtual public IConstraint {
	  public:
		virtual void InitializeLuaObject(lua::State *lua) override;
		virtual void SetLimit(math::Radian lowerLimit, math::Radian upperLimit) = 0;
		virtual std::pair<math::Radian, math::Radian> GetLimit() const = 0;
		virtual void DisableLimit() = 0;
	  protected:
		using IConstraint::IConstraint;
	};

	class DLLNETWORK ISliderConstraint : virtual public IConstraint {
	  public:
		virtual void InitializeLuaObject(lua::State *lua) override;
		virtual void SetLimit(float lowerLimit, float upperLimit) = 0;
		virtual void DisableLimit() = 0;
		virtual std::pair<float, float> GetLimit() const = 0;
	  protected:
		using IConstraint::IConstraint;
	};

	class DLLNETWORK IConeTwistConstraint : virtual public IConstraint {
	  protected:
		using IConstraint::IConstraint;
	  public:
		virtual void SetLimit(const Vector3 &lowerLimits, const Vector3 &upperLimits) = 0;
		virtual void SetLimit(float swingSpan1, float swingSpan2, float twistSpan) = 0;
		virtual void GetLimit(float &outSwingSpan1, float &outSwingSpan2, float &outTwistSpan) = 0;
		virtual void InitializeLuaObject(lua::State *lua) override;
	};

	class DLLNETWORK IDoFConstraint : virtual public IConstraint {
	  protected:
		using IConstraint::IConstraint;
	  public:
		virtual void InitializeLuaObject(lua::State *lua) override;
		virtual void SetLinearLimit(const Vector3 &lower, const Vector3 &upper) = 0;
		virtual void SetLinearLimit(const Vector3 &lim) = 0;
		virtual void SetLinearLowerLimit(const Vector3 &lim) = 0;
		virtual void SetLinearUpperLimit(const Vector3 &lim) = 0;
		virtual void SetAngularLimit(const EulerAngles &lower, const EulerAngles &upper) = 0;
		virtual void SetAngularLimit(const EulerAngles &lim) = 0;
		virtual void SetAngularLowerLimit(const EulerAngles &lim) = 0;
		virtual void SetAngularUpperLimit(const EulerAngles &lim) = 0;

		virtual Vector3 GetLinearLowerLimit() const = 0;
		virtual Vector3 GetlinearUpperLimit() const = 0;
		virtual EulerAngles GetAngularLowerLimit() const = 0;
		virtual EulerAngles GetAngularUpperLimit() const = 0;

		virtual Vector3 GetAngularTargetVelocity() const = 0;
		virtual void SetAngularTargetVelocity(const Vector3 &vel) const = 0;
		virtual Vector3 GetAngularMaxMotorForce() const = 0;
		virtual void SetAngularMaxMotorForce(const Vector3 &force) = 0;
		virtual Vector3 GetAngularMaxLimitForce() const = 0;
		virtual void SetAngularMaxLimitForce(const Vector3 &force) = 0;
		virtual Vector3 GetAngularDamping() const = 0;
		virtual void SetAngularDamping(const Vector3 &damping) = 0;
		virtual Vector3 GetAngularLimitSoftness() const = 0;
		virtual void SetAngularLimitSoftness(const Vector3 &softness) = 0;
		virtual Vector3 GetAngularForceMixingFactor() const = 0;
		virtual void SetAngularForceMixingFactor(const Vector3 &factor) = 0;
		virtual Vector3 GetAngularLimitErrorTolerance() const = 0;
		virtual void SetAngularLimitErrorTolerance(const Vector3 &tolerance) = 0;
		virtual Vector3 GetAngularLimitForceMixingFactor() const = 0;
		virtual void SetAngularLimitForceMixingFactor(const Vector3 &factor) = 0;
		virtual Vector3 GetAngularRestitutionFactor() const = 0;
		virtual void SetAngularRestitutionFactor(const Vector3 &factor) = 0;
		virtual bool IsAngularMotorEnabled(uint8_t axis) const = 0;
		virtual void SetAngularMotorEnabled(uint8_t axis, bool bEnabled) = 0;
		virtual Vector3 GetCurrentAngularLimitError() const = 0;
		virtual Vector3 GetCurrentAngularPosition() const = 0;
		virtual Vector3i GetCurrentAngularLimit() const = 0;
		virtual Vector3 GetCurrentAngularAccumulatedImpulse() const = 0;

		virtual Vector3 GetLinearTargetVelocity() const = 0;
		virtual void SetLinearTargetVelocity(const Vector3 &vel) const = 0;
		virtual Vector3 GetLinearMaxMotorForce() const = 0;
		virtual void SetLinearMaxMotorForce(const Vector3 &force) = 0;
		virtual float GetLinearDamping() const = 0;
		virtual void SetLinearDamping(float damping) = 0;
		virtual float GetLinearLimitSoftness() const = 0;
		virtual void SetLinearLimitSoftness(float softness) = 0;
		virtual Vector3 GetLinearForceMixingFactor() const = 0;
		virtual void SetLinearForceMixingFactor(const Vector3 &factor) = 0;
		virtual Vector3 GetLinearLimitErrorTolerance() const = 0;
		virtual void SetLinearLimitErrorTolerance(const Vector3 &tolerance) = 0;
		virtual Vector3 GetLinearLimitForceMixingFactor() const = 0;
		virtual void SetLinearLimitForceMixingFactor(const Vector3 &factor) = 0;
		virtual float GetLinearRestitutionFactor() const = 0;
		virtual void SetLinearRestitutionFactor(float factor) = 0;
		virtual bool IsLinearMotorEnabled(uint8_t axis) const = 0;
		virtual void SetLinearMotorEnabled(uint8_t axis, bool bEnabled) = 0;
		virtual Vector3 GetCurrentLinearDifference() const = 0;
		virtual Vector3 GetCurrentLinearLimitError() const = 0;
		virtual Vector3i GetCurrentLinearLimit() const = 0;
		virtual Vector3 GetCurrentLinearAccumulatedImpulse() const = 0;
	};

	class DLLNETWORK IDoFSpringConstraint : virtual public IConstraint {
	  protected:
		using IConstraint::IConstraint;
	  public:
		enum class AxisType : uint8_t { Linear = 0u, Angular };

		virtual void InitializeLuaObject(lua::State *lua) override;
		virtual void CalculateTransforms() = 0;
		virtual void CalculateTransforms(const math::Transform &frameA, const math::Transform &frameB) = 0;
		virtual math::Transform GetCalculatedTransformA() const = 0;
		virtual math::Transform GetCalculatedTransformB() const = 0;
		virtual math::Transform GetFrameOffsetA() const = 0;
		virtual math::Transform GetFrameOffsetB() const = 0;
		virtual Vector3 GetAxis(Axis axisIndex) const = 0;
		virtual double GetAngle(Axis axisIndex) const = 0;
		virtual double GetRelativePivotPosition(Axis axisIndex) const = 0;
		virtual void SetFrames(const math::Transform &frameA, const math::Transform &frameB) = 0;
		virtual void SetLinearLowerLimit(const Vector3 &linearLower) = 0;
		virtual Vector3 GetLinearLowerLimit() const = 0;
		virtual void SetLinearUpperLimit(const Vector3 &linearUpper) = 0;
		virtual Vector3 GetLinearUpperLimit() const = 0;
		virtual void SetAngularLowerLimit(const Vector3 &angularLower) = 0;
		virtual void SetAngularLowerLimitReversed(const Vector3 &angularLower) = 0;
		virtual Vector3 GetAngularLowerLimit() const = 0;
		virtual Vector3 GetAngularLowerLimitReversed() const = 0;
		virtual void SetAngularUpperLimit(const Vector3 &angularUpper) = 0;
		virtual void SetAngularUpperLimitReversed(const Vector3 &angularUpper) = 0;
		virtual Vector3 GetAngularUpperLimit() const = 0;
		virtual Vector3 GetAngularUpperLimitReversed() const = 0;
		virtual void SetLimit(AxisType type, Axis axis, double lo, double hi) = 0;
		virtual void SetLimitReversed(AxisType type, Axis axis, double lo, double hi) = 0;
		virtual bool IsLimited(AxisType type, Axis axis) const = 0;
		virtual void SetRotationOrder(RotationOrder order) = 0;
		virtual RotationOrder GetRotationOrder() const = 0;
		virtual void SetAxis(const Vector3 &axis1, const Vector3 &axis2) = 0;
		virtual void SetBounce(AxisType type, Axis axis, double bounce) = 0;
		virtual void EnableMotor(AxisType type, Axis axis, bool onOff) = 0;
		virtual void SetServo(AxisType type, Axis axis, bool onOff) = 0;
		virtual void SetTargetVelocity(AxisType type, Axis axis, double velocity) = 0;
		virtual void SetServoTarget(AxisType type, Axis axis, double target) = 0;
		virtual void SetMaxMotorForce(AxisType type, Axis axis, double force) = 0;
		virtual void EnableSpring(AxisType type, Axis axis, bool onOff) = 0;
		virtual void SetStiffness(AxisType type, Axis axis, double stiffness, bool limitIfNeeded = true) = 0;
		virtual void SetDamping(AxisType type, Axis axis, double damping, bool limitIfNeeded = true) = 0;
		virtual void SetEquilibriumPoint() = 0;
		virtual void SetEquilibriumPoint(AxisType type, Axis axis) = 0;
		virtual void SetEquilibriumPoint(AxisType type, Axis axis, double val) = 0;

		virtual void SetERP(AxisType type, Axis axis, double value) = 0;
		virtual double GetERP(AxisType type, Axis axis) const = 0;
		virtual void SetStopERP(AxisType type, Axis axis, double value) = 0;
		virtual double GetStopERP(AxisType type, Axis axis) const = 0;
		virtual void SetCFM(AxisType type, Axis axis, double value) = 0;
		virtual double GetCFM(AxisType type, Axis axis) const = 0;
		virtual void SetStopCFM(AxisType type, Axis axis, double value) = 0;
		virtual double GetStopCFM(AxisType type, Axis axis) const = 0;
	};
};
