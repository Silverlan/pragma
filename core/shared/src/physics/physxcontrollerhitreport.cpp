#include "stdafx_shared.h"
#ifdef PHYS_ENGINE_PHYSX
#include <pragma/game/game.h>
#include "pragma/physics/physxcontrollerhitreport.h"
#include "pragma/physics/physobj.h"
#include "vector3.h"

PhysXControllerHitReport::PhysXControllerHitReport(Game *game)
	: physx::PxUserControllerHitReport(),m_game(game)
{}
PhysXControllerHitReport::~PhysXControllerHitReport() {}

Game *PhysXControllerHitReport::GetGameState() {return m_game;}
void PhysXControllerHitReport::onShapeHit(const physx::PxControllerShapeHit& hit)
{
	physx::PxRigidDynamic *actor = hit.controller->getActor();
	if(actor == NULL)
		return;
	PhysObj *physOther = static_cast<PhysObj*>(actor->userData);
	if(physOther == NULL)
		return;
	ControllerPhysObj *phys = static_cast<ControllerPhysObj*>(actor->userData);
	if(phys == NULL)
		return;
	ControllerHitData &hitData = phys->GetControllerHitData();
	hitData.Clear();
	hitData.physObj = physOther->CreateHandle();
	hitData.hitNormal = Vector3(hit.worldNormal.x,hit.worldNormal.y,hit.worldNormal.z);
}
void PhysXControllerHitReport::onControllerHit(const physx::PxControllersHit& hit)
{

}
void PhysXControllerHitReport::onObstacleHit(const physx::PxControllerObstacleHit& hit)
{

}
#endif