#ifndef __PHYS_VEHICLE_HPP__
#define __PHYS_VEHICLE_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/physics/base.hpp"
#include <optional>

namespace pragma::physics
{
	class IConvexHullShape;
	struct DLLNETWORK ChassisCreateInfo
	{
		float mass = 0.f;
		Vector3 momentOfInertia = {};
		Vector3 cmOffset = {};
		std::shared_ptr<pragma::physics::IConvexHullShape> shape = nullptr;
	};

	struct DLLNETWORK WheelCreateInfo
	{
		float mass = 0.f;
		float width = 0.f;
		float radius = 0.f;
		float maxHandbrakeTorque = 0.f;
		umath::Degree maxSteeringAngle = 0.f;
		Vector3 chassisOffset = {};
		std::shared_ptr<pragma::physics::IConvexHullShape> shape = nullptr;

		std::optional<float> momentOfInertia = {};
		// Returns moi if specified, otherwise calculates
		// moi for a cylinder of the specified radius and mass.
		float GetMomentOfInertia() const;
	};

	class ICollisionObject;
	class DLLNETWORK IVehicle
		: public IBase,public IWorldObject
	{
	public:
		enum class Gear : uint16_t
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
		virtual void SetTurnFactor(float f)=0;

		virtual void SetGear(Gear gear)=0;
		virtual void SetGearDown()=0;
		virtual void SetGearUp()=0;
		virtual void SetGearSwitchTime(float time)=0;
		virtual void ChangeToGear(Gear gear)=0;
		virtual void SetUseAutoGears(bool useAutoGears)=0;

		virtual bool ShouldUseAutoGears() const=0;
		virtual Gear GetCurrentGear() const=0;
		virtual float GetEngineRotationSpeed() const=0;

		virtual void SetRestState()=0;

		virtual void ResetControls()=0;

		virtual void SetWheelRotationAngle(WheelIndex wheel,umath::Radian angle)=0;
		virtual void SetWheelRotationSpeed(WheelIndex wheel,umath::Radian speed)=0;

		virtual bool IsInAir() const=0;

		virtual uint32_t GetWheelCount() const=0;
		virtual float GetForwardSpeed() const=0;
		virtual float GetSidewaysSpeed() const=0;
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

#endif
