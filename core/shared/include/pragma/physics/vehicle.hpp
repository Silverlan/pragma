/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __PHYS_VEHICLE_HPP__
#define __PHYS_VEHICLE_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/physics/base.hpp"
#include <mathutil/transform.hpp>
#include "pragma/util/util_typed_manager.hpp"
#include <optional>

namespace pragma::physics
{
	class IConvexHullShape;
	class IShape;
	class IRigidBody;
	struct DLLNETWORK ChassisCreateInfo
	{
		std::optional<Vector3> momentOfInertia = {};
		// Indices for all of the actor shapes that make up the chassis
		std::vector<uint32_t> shapeIndices = {};
		// Optional custom center of mass.
		// If not specified, center of mass will be calculated automatically
		std::optional<Vector3> centerOfMass = {};
		float GetMass(const pragma::physics::IRigidBody &body) const;
		void GetAABB(const pragma::physics::IRigidBody &body,Vector3 &min,Vector3 &max) const;
		Vector3 GetMomentOfInertia(const pragma::physics::IRigidBody &body) const;
		Vector3 GetCenterOfMass(const pragma::physics::IRigidBody &body) const;
		std::vector<const pragma::physics::IShape*> GetShapes(const pragma::physics::IRigidBody &body) const;
		bool operator==(const ChassisCreateInfo &other) const
		{
			return momentOfInertia == other.momentOfInertia &&
				shapeIndices == other.shapeIndices &&
				centerOfMass == other.centerOfMass;
		}
		bool operator!=(const ChassisCreateInfo &other) const {return !operator==(other);}
	};

	struct DLLNETWORK WheelCreateInfo
	{
		struct DLLNETWORK SuspensionInfo
		{
			float maxCompression = 0.3f;
			float maxDroop = 0.1f;
			float springStrength = 35'000.0f;
			float springDamperRate = 4'500.0f;

			float camberAngleAtRest = 0.f;
			float camberAngleAtMaxDroop = 0.01f;
			float camberAngleAtMaxCompression = -0.01f;

			bool operator==(const SuspensionInfo &other) const
			{
				return maxCompression == other.maxCompression &&
					maxDroop == other.maxDroop &&
					springStrength == other.springStrength &&
					springDamperRate == other.springDamperRate &&
					camberAngleAtRest == other.camberAngleAtRest &&
					camberAngleAtMaxDroop == other.camberAngleAtMaxDroop &&
					camberAngleAtMaxCompression == other.camberAngleAtMaxCompression;
			}
			bool operator!=(const SuspensionInfo &other) const {return !operator==(other);}
		};

		static WheelCreateInfo CreateStandardFrontWheel();
		static WheelCreateInfo CreateStandardRearWheel();
		bool operator==(const WheelCreateInfo &other) const
		{
			return flags == other.flags &&
				width == other.width &&
				radius == other.radius &&
				maxHandbrakeTorque == other.maxHandbrakeTorque &&
				shapeIndex == other.shapeIndex &&
				maxSteeringAngle == other.maxSteeringAngle &&
				chassisOffset == other.chassisOffset &&
				suspension == other.suspension &&
				tireType == other.tireType &&
				momentOfInertia == other.momentOfInertia;
		}
		bool operator!=(const WheelCreateInfo &other) const {return !operator==(other);}
		enum class Flags : uint32_t
		{
			None = 0u,
			Front = 1u,
			Rear = Front<<1u,
			Left = Rear<<1u,
			Right = Left<<1u
		};
		Flags flags = Flags::None;
		// If width or radius are not specified, they will be
		// determined by the physics shape AABB!
		std::optional<float> width = {};
		std::optional<float> radius = {};
		float maxHandbrakeTorque = 0.f;
		// The index of the shape of the vehicle's rigid body
		// for this wheel. -1 means no shape is associated with
		// the wheel.
		int32_t shapeIndex = -1;
		umath::Degree maxSteeringAngle = 0.f;
		Vector3 chassisOffset = {};
		SuspensionInfo suspension = {};
		// Has to match one of the tire types defined in
		// the tire type manager of the physics environment
		TypeId tireType = 0;

		std::optional<float> momentOfInertia = {};
		// Returns moi if specified, otherwise calculates
		// moi for a cylinder of the specified radius and mass.
		float GetMomentOfInertia(const pragma::physics::IRigidBody &body) const;
		const pragma::physics::IShape *GetShape(const pragma::physics::IRigidBody &body) const;
		void GetAABB(const pragma::physics::IRigidBody &body,Vector3 &min,Vector3 &max) const;
		float GetRadius(const pragma::physics::IRigidBody &body) const;
		float GetWidth(const pragma::physics::IRigidBody &body) const;
	};

	class IRigidBody;
	struct DLLNETWORK VehicleCreateInfo
	{
		static constexpr uint32_t WHEEL_COUNT_4W_DRIVE = 4u;
		enum class WheelDrive : uint8_t
		{
			Front = 0u,
			Rear,
			Four
		};
		enum class Wheel : uint8_t
		{
			// Note: Order is important!
			FrontLeft = 0,
			FrontRight,
			RearLeft,
			RearRight,

			Dummy
		};
		struct DLLNETWORK AntiRollBar
		{
			AntiRollBar(Wheel wheel0,Wheel wheel1,float stiffness=10'000.0f)
				: wheel0{wheel0},wheel1{wheel1},stiffness{stiffness}
			{}
			AntiRollBar()=default;
			bool operator==(const AntiRollBar &other) const
			{
				return wheel0 == other.wheel0 &&
					wheel1 == other.wheel1 &&
					stiffness == other.stiffness;
			}
			bool operator!=(const AntiRollBar &other) const {return !operator==(other);}
			Wheel wheel0 = Wheel::FrontLeft;
			Wheel wheel1 = Wheel::FrontRight;
			float stiffness = 10'000.0f;
		};
		static Wheel GetWheelType(const WheelCreateInfo &wheelDesc);
		static VehicleCreateInfo CreateStandardFourWheelDrive(
			const std::array<Vector3,WHEEL_COUNT_4W_DRIVE> &wheelCenterOffsets,
			float handBrakeTorque=6'400'000.0,
			float maxSteeringAngle=60.0
		);
		bool operator==(const VehicleCreateInfo &other) const
		{
			return chassis == other.chassis &&
				wheels == other.wheels &&
				wheelDrive == other.wheelDrive &&
				antiRollBars == other.antiRollBars &&
				maxEngineTorque == other.maxEngineTorque &&
				gearSwitchTime == other.gearSwitchTime &&
				clutchStrength == other.clutchStrength &&
				gravityFactor == other.gravityFactor &&
				actor.Get() == other.actor.Get();
		}
		bool operator!=(const VehicleCreateInfo &other) const {return !operator==(other);}

		ChassisCreateInfo chassis = {};
		std::vector<WheelCreateInfo> wheels = {};
		WheelDrive wheelDrive = WheelDrive::Four;
		std::vector<AntiRollBar> antiRollBars = {};
		float maxEngineTorque = 500.f;
		umath::Radian maxEngineRotationSpeed = 600.f;
		float gearSwitchTime = 0.5f;
		float clutchStrength = 10.f;
		float gravityFactor = 1.f; // TODO
		mutable util::TSharedHandle<IRigidBody> actor = nullptr;
	};

	class ICollisionObject;
	class DLLNETWORK IVehicle
		: public IBase,public IWorldObject
	{
	public:
		enum class Gear : uint8_t
		{
			Reverse = 0,
			Neutral,
			First,
			Second,
			Third,
			Fourth,
			Fifth,
			Sixth,
			Seventh,
			Eighth,
			Ninth,
			Tenth,
			Eleventh,
			Twelfth,
			Thirteenth,
			Fourteenth,
			Fifteenth,
			Sixteenth,
			Seventeenth,
			Eighteenth,
			Nineteenth,
			Twentieth,
			Twentyfirst,
			Twentysecond,
			Twentythird,
			Twentyfourth,
			Twentyfifth,
			Twentysixth,
			Twentyseventh,
			Twentyeighth,
			Twentyninth,
			Thirtieth,

			Count
		};

		using WheelIndex = uint32_t;

		virtual void OnRemove() override;

		ICollisionObject *GetCollisionObject();
		const ICollisionObject *GetCollisionObject() const;
		virtual void InitializeLuaObject(lua_State *lua) override;

		virtual void SetUseDigitalInputs(bool bUseDigitalInputs)=0;

		virtual void SetBrakeFactor(float f)=0;
		virtual void SetHandbrakeFactor(float f)=0;
		virtual void SetAccelerationFactor(float f)=0;
		virtual void SetSteerFactor(float f)=0;

		virtual void SetGear(Gear gear)=0;
		virtual void SetGearDown()=0;
		virtual void SetGearUp()=0;
		virtual void SetGearSwitchTime(float time)=0;
		virtual void ChangeToGear(Gear gear)=0;
		virtual void SetUseAutoGears(bool useAutoGears)=0;

		virtual bool ShouldUseAutoGears() const=0;
		virtual Gear GetCurrentGear() const=0;
		virtual umath::Radian GetEngineRotationSpeed() const=0;
		virtual void SetEngineRotationSpeed(umath::Radian speed) const=0;

		virtual void SetRestState()=0;

		virtual void ResetControls()=0;

		virtual void SetWheelRotationAngle(WheelIndex wheel,umath::Radian angle)=0;
		virtual void SetWheelRotationSpeed(WheelIndex wheel,umath::Radian speed)=0;

		virtual bool IsInAir() const=0;

		virtual std::optional<umath::Transform> GetLocalWheelPose(WheelIndex wheelIndex) const=0;
		virtual uint32_t GetWheelCount() const=0;
		virtual float GetSteerFactor() const=0;
		virtual umath::Radian GetWheelYawAngle(WheelIndex wheel) const=0;
		virtual umath::Radian GetWheelRollAngle(WheelIndex wheel) const=0;
		virtual float GetForwardSpeed() const=0;
		virtual float GetSidewaysSpeed() const=0;

		virtual float GetBrakeFactor() const=0;
		virtual float GetHandbrakeFactor() const=0;
		virtual float GetAccelerationFactor() const=0;
		virtual umath::Radian GetWheelRotationSpeed(WheelIndex wheel) const=0;
	protected:
		IVehicle(IEnvironment &env,const util::TSharedHandle<ICollisionObject> &collisionObject);
		virtual bool ShouldUseDigitalInputs() const=0;
		util::TSharedHandle<ICollisionObject> m_collisionObject = nullptr;
	};
	
	class DLLNETWORK IWheel
		: public IBase
	{
	public:
	protected:
		IWheel(IEnvironment &env);
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::physics::WheelCreateInfo::Flags)

#endif
