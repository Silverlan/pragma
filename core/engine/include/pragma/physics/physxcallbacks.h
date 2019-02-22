#ifndef __PHYSXCALLBACKS_H__
#define __PHYSXCALLBACKS_H__

#ifdef PHYS_ENGINE_PHYSX
#include <PxPhysicsAPI.h>
#include "pragma/definitions.h"
#include "pragma/networkdefinitions.h"

class DLLENGINE WVPxErrorCallback : public physx::PxErrorCallback
{
public:
	void reportError(physx::PxErrorCode::Enum code,const char* message,const char* file,int line);
};

class DLLNETWORK Game;
class DLLENGINE WVPxEventCallback : public physx::PxSimulationEventCallback
{
private:
	Game *m_game;
public:
	WVPxEventCallback(Game *game);
	virtual void onContact(const physx::PxContactPairHeader &pairHeader,const physx::PxContactPair *pairs,physx::PxU32 nbPairs) override;
	virtual void onTrigger(physx::PxTriggerPair *pairs,physx::PxU32 count) override;
	virtual void onConstraintBreak(physx::PxConstraintInfo *constraints,physx::PxU32 count) override;
	virtual void onWake(physx::PxActor **actors,physx::PxU32 count) override;
	virtual void onSleep(physx::PxActor **actors,physx::PxU32 count) override;
};
#endif


#endif