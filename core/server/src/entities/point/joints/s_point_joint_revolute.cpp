#include "stdafx_server.h"
#ifdef PHYS_ENGINE_PHYSX
#include "pragma/entities/point/joints/s_point_joint_revolute.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/physics/physobj.h>
LINK_ENTITY_TO_CLASS(point_joint_revolute,PointJointRevolute);

extern DLLENGINE Engine *engine;;
extern SGame *s_game;

PointJointRevolute::PointJointRevolute()
	: Entity(),BasePointJointRevolute()
{}

void PointJointRevolute::OnSpawn()
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
	auto rotTgt = rotSrc;//Quat(EulerAngles((*pos) -m_posTarget));
	//Angle angTest = Angle((*pos) -m_posTarget);
	//Angle angOther = Angle(Quat(EulerAngles((*pos) -m_posTarget)));

#ifdef PHYS_ENGINE_PHYSX
	physx::PxPhysics *physics = engine->GetPhysics();
	Angle angTest(90.f,0.f,0);
	auto rotTest = Quat(angTest);
	// physx::PxVec3(0,0,1), 90 degree -> Rotation around (0,1,0), but WHY??
	physx::PxQuat qTest(Math::DegToRad(90),physx::PxVec3(0,0,1));//rotTest.x,rotTest.y,rotTest.z,rotTest.w);
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
						//physx::PxTransform tSrc(physx::PxVec3(posSrc.x,posSrc.y,posSrc.z));
						tSrc.q = qTest;
						tSrc.p = physx::PxVec3(posSrc.x -tSrc.p.x,posSrc.y -tSrc.p.y,posSrc.z -tSrc.p.z);
						//tSrc.p = physx::PxVec3(posSrc.x,posSrc.y,posSrc.z);
						//tSrc.q = physx::PxQuat(rotSrc.x,rotSrc.y,rotSrc.z,rotSrc.w);
						for(unsigned l=0;l<actorsTgt->size();l++)
						{
							physx::PxRigidActor *actorTgt = (*actorsTgt)[l];
							//physx::PxTransform tTgt = actorTgt->getGlobalPose();
							physx::PxTransform tTgt(physx::PxVec3(posTgt.x,posTgt.y,posTgt.z));
							tTgt.q = qTest;
							tTgt.p = physx::PxVec3(posTgt.x -tTgt.p.x,posTgt.y -tTgt.p.y,posTgt.z -tTgt.p.z);
							//tTgt.q = physx::PxQuat(0.4f,0.2f,0.3f,1.f);//rotTgt.x,rotTgt.y,rotTgt.z,rotTgt.w);
							//tTgt.q.normalize();

							physx::PxVec3 dir = tTgt.p -tSrc.p;
							Vector3 v(dir.x,dir.y,dir.z);
							Quat rot = Quat(EulerAngles(v));
							//tTgt.q = physx::PxQuat(rot.x,rot.y,rot.z,rot.w);
							/*physx::PxRevoluteJoint *joint = physx::PxRevoluteJointCreate(
								*physics,
								actorSrc,
								tSrc,
								actorTgt,
								tTgt
							);
							joint->setProjectionLinearTolerance(200.f);
							joint->setProjectionAngularTolerance(1.f);*/
						}
					}
				}
			}
		}
	}
#endif
}

void PointJointRevolute::SetKeyValue(std::string key,std::string val)
{
	Entity::SetKeyValue(key,val);
	BasePointJoint::SetKeyValue(key,val);
}
#endif