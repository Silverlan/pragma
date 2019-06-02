#include "stdafx_engine.h"
#ifdef PHYS_ENGINE_PHYSX
#include "pragma/physics/physxcallbacks.h"
#include "pragma/console/conout.h"
#include <pragma/entities/baseentity.h>
#include <pragma/physics/physobj.h>

DLLENGINE void WVPxErrorCallback::reportError(physx::PxErrorCode::Enum code,const char* message,const char* file,int line)
{
	std::stringstream ss;
	ss<<"[PHYSX] "<<message<<" ("<<code<<") in file "<<file<<" (Line "<<line<<")";
	util::set_console_color(util::ConsoleColorFlags::Green | util::ConsoleColorFlags::Intensity);
	Con::cout<<ss.str()<<Con::endl;
	if(Con::GetLogLevel() >= 2)
		Con::WriteToLog(ss);
}

/////////////////////////

DLLENGINE WVPxEventCallback::WVPxEventCallback(Game *game)
	: physx::PxSimulationEventCallback(),m_game(game)
{}

DLLENGINE void WVPxEventCallback::onContact(const physx::PxContactPairHeader &pairHeader,const physx::PxContactPair *pairs,physx::PxU32 nbPairs)
{
	physx::PxActor *a = pairHeader.actors[0];
	physx::PxActor *b = pairHeader.actors[1];
	if(a != NULL && b != NULL && a->userData != NULL && b->userData != NULL)
	{
		PhysObj *physA = static_cast<PhysObj*>(a->userData);
		BaseEntity *entA = physA->GetOwner();
		if(entA != NULL)
		{
			PhysObj *physB = static_cast<PhysObj*>(b->userData);
			BaseEntity *entB = physB->GetOwner();
			if(entB != NULL)
			{
				if(entA->GetCollisionCallbacksEnabled())
					entA->OnContact(entB,physB,b);
				if(entB->GetCollisionCallbacksEnabled())
					entB->OnContact(entA,physA,a);
				bool bContactA = entA->GetCollisionContactReportEnabled();
				bool bContactB = entB->GetCollisionContactReportEnabled();
				if(bContactA || bContactB)
				{
					const physx::PxU32 bufferSize = 64;
					physx::PxContactPairPoint contacts[bufferSize];
					for(physx::PxU32 i=0;i<nbPairs;i++)
					{
						physx::PxU32 nbContacts = pairs[i].extractContacts(contacts,bufferSize);
						if(bContactA)
							entA->OnContact(entB,physB,b,pairs[i],nbContacts,contacts);
						if(bContactB)
							entB->OnContact(entA,physA,a,pairs[i],nbContacts,contacts);
					}
				}
			}
		}
	}
}
DLLENGINE void WVPxEventCallback::onTrigger(physx::PxTriggerPair *pairs,physx::PxU32 count)
{
	if(pairs->flags &physx::PxTriggerPairFlag::eDELETED_SHAPE_OTHER || pairs->flags &physx::PxTriggerPairFlag::eDELETED_SHAPE_TRIGGER)
		return;
	if(pairs->triggerActor->userData != NULL && pairs->otherActor->userData != NULL)
	{
		PhysObj *phys = static_cast<PhysObj*>(pairs->triggerActor->userData);
		BaseEntity *ent = phys->GetOwner();
		if(ent != NULL && ent->IsTrigger())
		{
			PhysObj *physOther = static_cast<PhysObj*>(pairs->otherActor->userData);
			BaseEntity *entOther = physOther->GetOwner();
			if(entOther != NULL)
			{
				if(pairs->status &physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
					ent->StartTouch(entOther,physOther);
				else if(pairs->status &physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
					ent->EndTouch(entOther,physOther);
			}
		}
	}
}
DLLENGINE void WVPxEventCallback::onConstraintBreak(physx::PxConstraintInfo *constraints,physx::PxU32 count)
{
	std::cout<<"onConstraintBreak!"<<std::endl;
}
DLLENGINE void WVPxEventCallback::onWake(physx::PxActor **actors,physx::PxU32 count)
{
	//std::cout<<"onWake!"<<std::endl;
	for(unsigned int i=0;i<count;i++)
	{
		physx::PxActor *actor = actors[i];
		if(actor != NULL && actor->userData != NULL)
		{
			PhysObj *phys = static_cast<PhysObj*>(actor->userData);
			phys->OnWake();
		}
	}
}
DLLENGINE void WVPxEventCallback::onSleep(physx::PxActor **actors,physx::PxU32 count)
{
	//std::cout<<"onSleep!"<<std::endl;
	for(unsigned int i=0;i<count;i++)
	{
		physx::PxActor *actor = actors[i];
		if(actor != NULL && actor->userData != NULL)
		{
			PhysObj *phys = static_cast<PhysObj*>(actor->userData);
			phys->OnSleep();
		}
	}
}
#endif