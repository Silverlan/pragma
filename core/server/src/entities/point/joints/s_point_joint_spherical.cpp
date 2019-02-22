#include "stdafx_server.h"
#ifdef PHYS_ENGINE_PHYSX
#include "pragma/entities/point/joints/s_point_joint_spherical.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/physics/physobj.h>
LINK_ENTITY_TO_CLASS(point_joint_spherical,PointJointSpherical);

extern DLLENGINE Engine *engine;;
extern SGame *s_game;

PointJointSpherical::PointJointSpherical()
	: Entity(),BasePointJointSpherical()
{}

void PointJointSpherical::OnSpawn()
{
	Entity::OnSpawn();
	std::vector<BaseEntity*> entsSrc;
	std::vector<BaseEntity*> entsTgt;
	GetTargetEntities(entsSrc,entsTgt);

}

void PointJointSpherical::SetKeyValue(std::string key,std::string val)
{
	Entity::SetKeyValue(key,val);
	BasePointJoint::SetKeyValue(key,val);
}
#endif