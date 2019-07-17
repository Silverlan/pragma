#include "stdafx_shared.h"
#include "pragma/physics/vehicle.hpp"
#include "pragma/physics/environment.hpp"
#include "pragma/physics/shape.hpp"

#pragma optimize("",off)
pragma::physics::WheelCreateInfo pragma::physics::WheelCreateInfo::CreateStandardFrontWheel()
{
	WheelCreateInfo createInfo {};
	createInfo.maxSteeringAngle = 60.f;
	return createInfo;
}
pragma::physics::WheelCreateInfo pragma::physics::WheelCreateInfo::CreateStandardRearWheel()
{
	WheelCreateInfo createInfo {};
	createInfo.maxHandbrakeTorque = 6'400'000.f;
	return createInfo;
}

///////////////

static pragma::physics::IShape *get_shape_from_shape_index(const pragma::physics::IRigidBody &body,int32_t shapeIndex)
{
	auto *pShape = body.GetCollisionShape();
	if(pShape == nullptr || pShape->IsCompoundShape() == false)
		return nullptr;
	auto &pCompoundShape = *pShape->GetCompoundShape();
	auto &subShapes = pCompoundShape.GetShapes();
	if(shapeIndex < 0 || shapeIndex >= subShapes.size())
		return nullptr;
	auto &subShape = subShapes.at(shapeIndex);
	return subShape.shape.get();
}
const pragma::physics::IShape *pragma::physics::ChassisCreateInfo::GetShape(const pragma::physics::IRigidBody &body) const
{
	return get_shape_from_shape_index(body,shapeIndex);
}
Vector3 pragma::physics::ChassisCreateInfo::GetMomentOfInertia(const pragma::physics::IRigidBody &body) const
{
	if(momentOfInertia.has_value())
		return *momentOfInertia;
	auto *shape = GetShape(body);
	if(shape == nullptr)
		return Vector3{};
	auto mass = shape->GetMass();
	Vector3 min,max;
	shape->GetAABB(min,max);
	auto dims = (max -min) *0.5f;
	return Vector3 {
		(umath::pow2(dims.y) +umath::pow2(dims.z)) *mass /12.0,
		(umath::pow2(dims.x) +umath::pow2(dims.z)) *0.8 *mass /12.0,
		(umath::pow2(dims.x) +umath::pow2(dims.y)) *mass /12.0
	};
}

///////////////

pragma::physics::VehicleCreateInfo::Wheel pragma::physics::VehicleCreateInfo::GetWheelType(const WheelCreateInfo &wheelDesc)
{
	if(umath::is_flag_set(wheelDesc.flags,WheelCreateInfo::Flags::Front))
	{
		if(umath::is_flag_set(wheelDesc.flags,WheelCreateInfo::Flags::Left))
			return Wheel::FrontLeft;
		if(umath::is_flag_set(wheelDesc.flags,WheelCreateInfo::Flags::Right))
			return Wheel::FrontRight;
	}
	if(umath::is_flag_set(wheelDesc.flags,WheelCreateInfo::Flags::Rear))
	{
		if(umath::is_flag_set(wheelDesc.flags,WheelCreateInfo::Flags::Left))
			return Wheel::RearLeft;
		if(umath::is_flag_set(wheelDesc.flags,WheelCreateInfo::Flags::Right))
			return Wheel::RearRight;
	}
	return Wheel::Dummy;
}
pragma::physics::VehicleCreateInfo pragma::physics::VehicleCreateInfo::CreateStandardFourWheelDrive(
	const std::array<Vector3,WHEEL_COUNT_4W_DRIVE> &wheelCenterOffsets,float wheelWidth,float wheelRadius
)
{
	VehicleCreateInfo vhcCreateInfo {};
	vhcCreateInfo.antiRollBars = {
		{Wheel::FrontLeft,Wheel::FrontRight},
		{Wheel::RearLeft,Wheel::RearRight}
	};
	vhcCreateInfo.wheelDrive = VehicleCreateInfo::WheelDrive::Four;

	constexpr auto handBrakeTorque = 6'400'000.0;
	constexpr auto maxSteeringAngle = 60.0;
	auto numWheels = WHEEL_COUNT_4W_DRIVE;
	vhcCreateInfo.wheels.reserve(numWheels);
	for(auto i=decltype(numWheels){0u};i<numWheels;++i)
	{
		vhcCreateInfo.wheels.push_back({});
		auto &wheelInfo = vhcCreateInfo.wheels.at(i);
		wheelInfo.width = wheelWidth;
		wheelInfo.radius = wheelRadius;
		wheelInfo.chassisOffset = wheelCenterOffsets.at(i);
		switch(static_cast<Wheel>(i))
		{
		case Wheel::FrontLeft:
			wheelInfo.flags = WheelCreateInfo::Flags::Front | WheelCreateInfo::Flags::Left;
			wheelInfo.maxSteeringAngle = maxSteeringAngle;
			break;
		case Wheel::FrontRight:
			wheelInfo.flags = WheelCreateInfo::Flags::Front | WheelCreateInfo::Flags::Right;
			wheelInfo.maxSteeringAngle = maxSteeringAngle;
			break;
		case Wheel::RearLeft:
			wheelInfo.flags = WheelCreateInfo::Flags::Rear | WheelCreateInfo::Flags::Left;
			wheelInfo.maxHandbrakeTorque = handBrakeTorque;
			break;
		case Wheel::RearRight:
			wheelInfo.flags = WheelCreateInfo::Flags::Rear | WheelCreateInfo::Flags::Right;
			wheelInfo.maxHandbrakeTorque = handBrakeTorque;
			break;
		}
	}
	return vhcCreateInfo;
}

///////////////

const pragma::physics::IShape *pragma::physics::WheelCreateInfo::GetShape(const pragma::physics::IRigidBody &body) const
{
	return get_shape_from_shape_index(body,shapeIndex);
}
float pragma::physics::WheelCreateInfo::GetMomentOfInertia(const pragma::physics::IRigidBody &body) const
{
	if(momentOfInertia.has_value())
		return *momentOfInertia;
	auto *pShape = GetShape(body);
	if(pShape == nullptr)
		return 0.f;
	// MOI of a cylinder
	return 0.5f *pShape->GetMass() *umath::pow2(radius);
}

pragma::physics::IVehicle::IVehicle(IEnvironment &env,const util::TSharedHandle<ICollisionObject> &collisionObject)
	: IBase{env},m_collisionObject{collisionObject}
{}

pragma::physics::ICollisionObject *pragma::physics::IVehicle::GetCollisionObject() {return m_collisionObject.Get();}
const pragma::physics::ICollisionObject *pragma::physics::IVehicle::GetCollisionObject() const {return const_cast<IVehicle*>(this)->GetCollisionObject();}

void pragma::physics::IVehicle::InitializeLuaObject(lua_State *lua)
{
	IBase::InitializeLuaObject<IVehicle>(lua);
}

void pragma::physics::IVehicle::OnRemove()
{
	RemoveWorldObject();
	m_physEnv.RemoveVehicle(*this);
	IBase::OnRemove();
}

pragma::physics::IWheel::IWheel(IEnvironment &env)
	: IBase{env}
{}
#pragma optimize("",on)
