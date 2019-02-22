#include "stdafx_server.h"
#ifdef PHYS_ENGINE_PHYSX
#include "pragma/entities/point/joints/s_point_joint_prismatic.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/physics/physobj.h>
LINK_ENTITY_TO_CLASS(point_joint_prismatic,PointJointPrismatic);

extern DLLENGINE Engine *engine;;
extern SGame *s_game;

PointJointPrismatic::PointJointPrismatic()
	: Entity(),BasePointJointPrismatic()
{}

void PointJointPrismatic::OnSpawn()
{
	Entity::OnSpawn();
	std::vector<BaseEntity*> entsSrc;
	std::vector<BaseEntity*> entsTgt;
	GetTargetEntities(entsSrc,entsTgt);

}

void PointJointPrismatic::SetKeyValue(std::string key,std::string val)
{
	Entity::SetKeyValue(key,val);
	BasePointJoint::SetKeyValue(key,val);
}
#endif