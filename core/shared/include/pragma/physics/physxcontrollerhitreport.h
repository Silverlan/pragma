#ifndef __PHYSXCONTROLLERHITREPORT_H__
#define __PHYSXCONTROLLERHITREPORT_H__

#ifdef PHYS_ENGINE_PHYSX
#include "pragma/networkdefinitions.h"
#include "pragma/physics/physxapi.h"

class Game;
class DLLNETWORK PhysXControllerHitReport
	: public physx::PxUserControllerHitReport
{
protected:
	Game *m_game;
public:
	PhysXControllerHitReport(Game *game);
	Game *GetGameState();
	virtual ~PhysXControllerHitReport() override;
	virtual void onShapeHit(const physx::PxControllerShapeHit& hit) override;
	virtual void onControllerHit(const physx::PxControllersHit& hit) override;
	virtual void onObstacleHit(const physx::PxControllerObstacleHit& hit) override;
};
#endif

#endif