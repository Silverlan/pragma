#include "stdafx_shared.h"
#include "pragma/physics/environment.hpp"
#include "pragma/physics/collision_object.hpp"
#include "pragma/physics/shape.hpp"
#include "pragma/physics/controller.hpp"

#pragma optimize("",off)
pragma::physics::IController::IController(IEnvironment &env,const util::TSharedHandle<ICollisionObject> &collisionObject)
	: IBase{env},m_collisionObject{collisionObject}
{}

void pragma::physics::IController::OnRemove()
{
	IBase::OnRemove();
	m_physEnv.RemoveController(*this);
}

pragma::physics::IController::CollisionFlags pragma::physics::IController::Move(Vector3 &disp)
{
	m_moveDisplacement = disp;
	return DoMove(disp);
}

pragma::physics::ICollisionObject *pragma::physics::IController::GetCollisionObject() {return m_collisionObject.Get();}
const pragma::physics::ICollisionObject *pragma::physics::IController::GetCollisionObject() const {return const_cast<IController*>(this)->GetCollisionObject();}

const pragma::physics::IConvexShape *pragma::physics::IController::GetShape() const {return const_cast<IController*>(this)->GetShape();}
pragma::physics::IConvexShape *pragma::physics::IController::GetShape()
{
	if(m_collisionObject == nullptr)
		return nullptr;
	auto *shape = m_collisionObject->GetCollisionShape();
	return (shape && shape->IsConvex()) ? shape->GetConvexShape() : nullptr;
}
const Vector3 &pragma::physics::IController::GetLastMoveDisplacement() const {return m_moveDisplacement;}
#pragma optimize("",on)
