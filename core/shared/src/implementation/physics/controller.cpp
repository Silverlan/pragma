// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :physics.controller;

pragma::physics::IController::IController(IEnvironment &env, const util::TSharedHandle<ICollisionObject> &collisionObject, const Vector3 &halfExtents, ShapeType shapeType) : IBase {env}, m_collisionObject {collisionObject}, m_halfExtents {halfExtents}, m_shapeType {shapeType} {}

void pragma::physics::IController::OnRemove()
{
	RemoveWorldObject();
	m_physEnv.RemoveController(*this);
	IBase::OnRemove();
}

void pragma::physics::IController::Move(Vector3 &disp)
{
	AddMoveVelocity(disp);
	//m_moveDisplacement = disp;
	//DoMove(disp);
}
void pragma::physics::IController::SetMoveVelocity(const Vector3 &vel)
{
	//std::cout<<"Move Velocity: "<<vel.x<<","<<vel.y<<","<<vel.z<<std::endl;
	m_moveVelocity = vel;
}
void pragma::physics::IController::AddMoveVelocity(const Vector3 &vel) { SetMoveVelocity(GetMoveVelocity() + vel); }
const Vector3 &pragma::physics::IController::GetMoveVelocity() const { return m_moveVelocity; }

pragma::physics::ICollisionObject *pragma::physics::IController::GetCollisionObject() { return m_collisionObject.Get(); }
const pragma::physics::ICollisionObject *pragma::physics::IController::GetCollisionObject() const { return const_cast<IController *>(this)->GetCollisionObject(); }

const pragma::physics::IConvexShape *pragma::physics::IController::GetShape() const { return const_cast<IController *>(this)->GetShape(); }
pragma::physics::IConvexShape *pragma::physics::IController::GetShape()
{
	if(m_collisionObject == nullptr)
		return nullptr;
	auto *shape = m_collisionObject->GetCollisionShape();
	return (shape && shape->IsConvex()) ? shape->GetConvexShape() : nullptr;
}
const Vector3 &pragma::physics::IController::GetLastMoveDisplacement() const { return uvec::PRM_ORIGIN; } //m_moveDisplacement;}
void pragma::physics::IController::InitializeLuaObject(lua::State *lua) { IBase::InitializeLuaObject<IController>(lua); }
