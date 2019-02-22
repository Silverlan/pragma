#include "stdafx_server.h"
#ifdef PHYS_ENGINE_PHYSX
#include "pragma/entities/point/joints/s_point_joint_distance.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/physics/physobj.h>
LINK_ENTITY_TO_CLASS(point_joint_distance,PointJointDistance);

extern DLLENGINE Engine *engine;;
extern SGame *s_game;

PointJointDistance::PointJointDistance()
	: Entity(),BasePointJointDistance()
{}

void PointJointDistance::OnSpawn()
{
	Entity::OnSpawn();
	std::vector<BaseEntity*> entsSrc;
	std::vector<BaseEntity*> entsTgt;
	GetTargetEntities(entsSrc,entsTgt);

	Vector3 &pos = GetPosition();
	//Quat &rot = GetOrientation();
	//physx::PxTransform tSrc(physx::PxVec3(pos->x,pos->y,pos->z),physx::PxQuat(rot->x,rot->y,rot->z,rot->w));

	//Quat rotTgt = Quat(EulerAngles((*pos) -m_posTarget));
	//physx::PxTransform tTgt(physx::PxVec3(m_posTarget.x,m_posTarget.y,m_posTarget.z),physx::PxQuat(rotTgt.x,rotTgt.y,rotTgt.z,rotTgt.w));
	
	Vector3 posSrc = GetPosition();
	auto rotSrc = GetOrientation();

	Vector3 posTgt = m_posTarget;
	Quat rotTgt = rotSrc;//Quat(EulerAngles((*pos) -m_posTarget));
	Angle angTest = Angle(pos -m_posTarget);
	Angle angOther = Angle(Quat(EulerAngles(pos -m_posTarget)));

#ifdef PHYS_ENGINE_PHYSX
	physx::PxPhysics *physics = engine->GetPhysics();
	
	for(unsigned int i=0;i<entsSrc.size();i++)
	{
		BaseEntity *entSrc = entsSrc[i];
		PhysObj *physSrc = entSrc->GetPhysicsObject();
		if(physSrc != NULL)
		{
			std::vector<physx::PxRigidActor*> *actorsSrc = physSrc->GetActors();
			for(unsigned int j=0;j<entsTgt.size();j++)
			{
				BaseEntity *entTgt = entsTgt[j];
				PhysObj *physTgt = entTgt->GetPhysicsObject();
				if(physTgt != NULL)
				{
					std::vector<physx::PxRigidActor*> *actorsTgt = physTgt->GetActors();
					for(unsigned k=0;k<actorsSrc->size();k++)
					{
						physx::PxRigidActor *actorSrc = (*actorsSrc)[k];
						physx::PxTransform tSrc = actorSrc->getGlobalPose();
						tSrc.p = physx::PxVec3(posSrc.x,posSrc.y,posSrc.z);
						tSrc.q = physx::PxQuat(rotSrc.x,rotSrc.y,rotSrc.z,rotSrc.w);
						for(unsigned l=0;l<actorsTgt->size();l++)
						{
							physx::PxRigidActor *actorTgt = (*actorsTgt)[l];
							physx::PxTransform tTgt = actorTgt->getGlobalPose();
							tTgt.p = physx::PxVec3(posTgt.x,posTgt.y,posTgt.z);
							tTgt.q = physx::PxQuat(rotTgt.x,rotTgt.y,rotTgt.z,rotTgt.w);
							physx::PxDistanceJoint *joint = physx::PxDistanceJointCreate(
								*physics,
								actorSrc,
								tSrc,
								actorTgt,
								tTgt
							);
						}
					}
				}
			}
		}
	}
#endif
}

void PointJointDistance::SetKeyValue(std::string key,std::string val)
{
	Entity::SetKeyValue(key,val);
	BasePointJointDistance::SetKeyValue(key,val);
}
#endif