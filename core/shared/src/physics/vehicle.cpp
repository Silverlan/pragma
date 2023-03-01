/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/physics/vehicle.hpp"
#include "pragma/physics/environment.hpp"
#include "pragma/physics/shape.hpp"
#include "pragma/physics/base_t.hpp"

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

static pragma::physics::IShape *get_shape_from_shape_index(const pragma::physics::IRigidBody &body, int32_t shapeIndex)
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
std::vector<const pragma::physics::IShape *> pragma::physics::ChassisCreateInfo::GetShapes(const pragma::physics::IRigidBody &body) const
{
	std::vector<const pragma::physics::IShape *> shapes {};
	shapes.reserve(shapeIndices.size());
	for(auto shapeIndex : shapeIndices) {
		auto *shape = get_shape_from_shape_index(body, shapeIndex);
		if(shape == nullptr)
			continue;
		shapes.push_back(shape);
	}
	return shapes;
}
float pragma::physics::ChassisCreateInfo::GetMass(const pragma::physics::IRigidBody &body) const
{
	auto shapes = GetShapes(body);
	auto mass = 0.f;
	for(auto *pShape : shapes)
		mass += pShape->GetMass();
	return mass;
}
void pragma::physics::ChassisCreateInfo::GetAABB(const pragma::physics::IRigidBody &body, Vector3 &min, Vector3 &max) const
{
	auto shapes = GetShapes(body);
	min = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
	max = {std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()};
	for(auto *pShape : shapes) {
		Vector3 shapeMin, shapeMax;
		pShape->GetAABB(shapeMin, shapeMax);
		uvec::min(&min, shapeMin);
		uvec::max(&max, shapeMax);
	}
}
Vector3 pragma::physics::ChassisCreateInfo::GetCenterOfMass(const pragma::physics::IRigidBody &body) const
{
	if(centerOfMass.has_value())
		return *centerOfMass;
	Vector3 min, max;
	GetAABB(body, min, max);

	auto dim = (max - min) / 2.f;
	// Move center of mass to center of shape AABB, then move it slightly forward and down.
	// This approximately represents the center of mass of real vehicles and usually
	// results in good handling.
	return (min + max) / 2.f + Vector3 {0.f, dim.y * -0.35f, dim.z * 0.1f};
}
Vector3 pragma::physics::ChassisCreateInfo::GetMomentOfInertia(const pragma::physics::IRigidBody &body) const
{
	if(momentOfInertia.has_value())
		return *momentOfInertia;
	auto shapes = GetShapes(body);
	if(shapes.empty())
		return Vector3 {};
	auto mass = GetMass(body);
	Vector3 min, max;
	GetAABB(body, min, max);

	auto dims = (max - min) * 0.5f;
	return Vector3 {(umath::pow2(dims.y) + umath::pow2(dims.z)) * mass / 12.0, (umath::pow2(dims.x) + umath::pow2(dims.z)) * 0.8 * mass / 12.0, (umath::pow2(dims.x) + umath::pow2(dims.y)) * mass / 12.0};
}

///////////////

pragma::physics::VehicleCreateInfo::Wheel pragma::physics::VehicleCreateInfo::GetWheelType(const WheelCreateInfo &wheelDesc)
{
	if(umath::is_flag_set(wheelDesc.flags, WheelCreateInfo::Flags::Front)) {
		if(umath::is_flag_set(wheelDesc.flags, WheelCreateInfo::Flags::Left))
			return Wheel::FrontLeft;
		if(umath::is_flag_set(wheelDesc.flags, WheelCreateInfo::Flags::Right))
			return Wheel::FrontRight;
	}
	if(umath::is_flag_set(wheelDesc.flags, WheelCreateInfo::Flags::Rear)) {
		if(umath::is_flag_set(wheelDesc.flags, WheelCreateInfo::Flags::Left))
			return Wheel::RearLeft;
		if(umath::is_flag_set(wheelDesc.flags, WheelCreateInfo::Flags::Right))
			return Wheel::RearRight;
	}
	return Wheel::Dummy;
}
pragma::physics::VehicleCreateInfo pragma::physics::VehicleCreateInfo::CreateStandardFourWheelDrive(const std::array<Vector3, WHEEL_COUNT_4W_DRIVE> &wheelCenterOffsets, float handBrakeTorque, float maxSteeringAngle)
{
	VehicleCreateInfo vhcCreateInfo {};
	vhcCreateInfo.antiRollBars = {{Wheel::FrontLeft, Wheel::FrontRight}, {Wheel::RearLeft, Wheel::RearRight}};
	vhcCreateInfo.wheelDrive = VehicleCreateInfo::WheelDrive::Four;

	auto numWheels = WHEEL_COUNT_4W_DRIVE;
	vhcCreateInfo.wheels.reserve(numWheels);
	for(auto i = decltype(numWheels) {0u}; i < numWheels; ++i) {
		vhcCreateInfo.wheels.push_back({});
		auto &wheelInfo = vhcCreateInfo.wheels.at(i);
		wheelInfo.chassisOffset = wheelCenterOffsets.at(i);
		switch(static_cast<Wheel>(i)) {
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

const pragma::physics::IShape *pragma::physics::WheelCreateInfo::GetShape(const pragma::physics::IRigidBody &body) const { return get_shape_from_shape_index(body, shapeIndex); }
void pragma::physics::WheelCreateInfo::GetAABB(const pragma::physics::IRigidBody &body, Vector3 &min, Vector3 &max) const
{
	auto *shape = GetShape(body);
	if(shape == nullptr) {
		min = {};
		max = {};
		return;
	}
	shape->GetAABB(min, max);
}
float pragma::physics::WheelCreateInfo::GetRadius(const pragma::physics::IRigidBody &body) const
{
	if(radius.has_value())
		return *radius;
	Vector3 min, max;
	GetAABB(body, min, max);
	auto dims = (max - min) * 0.5f;
	return dims.z;
}
float pragma::physics::WheelCreateInfo::GetWidth(const pragma::physics::IRigidBody &body) const
{
	if(width.has_value())
		return *width;
	Vector3 min, max;
	GetAABB(body, min, max);
	auto dims = (max - min) * 0.5f;
	return dims.x;
}
float pragma::physics::WheelCreateInfo::GetMomentOfInertia(const pragma::physics::IRigidBody &body) const
{
	if(momentOfInertia.has_value())
		return *momentOfInertia;
	auto *pShape = GetShape(body);
	if(pShape == nullptr)
		return 0.f;
	// MOI of a cylinder
	return 0.5f * pShape->GetMass() * umath::pow2(GetRadius(body));
}

pragma::physics::IVehicle::IVehicle(IEnvironment &env, const util::TSharedHandle<ICollisionObject> &collisionObject) : IBase {env}, m_collisionObject {collisionObject} {}

pragma::physics::ICollisionObject *pragma::physics::IVehicle::GetCollisionObject() { return m_collisionObject.Get(); }
const pragma::physics::ICollisionObject *pragma::physics::IVehicle::GetCollisionObject() const { return const_cast<IVehicle *>(this)->GetCollisionObject(); }

void pragma::physics::IVehicle::InitializeLuaObject(lua_State *lua) { IBase::InitializeLuaObject<IVehicle>(lua); }

void pragma::physics::IVehicle::OnRemove()
{
	RemoveWorldObject();
	m_physEnv.RemoveVehicle(*this);
	IBase::OnRemove();
}

pragma::physics::IWheel::IWheel(IEnvironment &env) : IBase {env} {}
