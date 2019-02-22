#ifndef __JOINTINFO_H__
#define __JOINTINFO_H__

#include "pragma/networkdefinitions.h"
#ifdef PHYS_ENGINE_BULLET
#define JOINT_TYPE_NONE 0
#define JOINT_TYPE_FIXED 1
#define JOINT_TYPE_BALLSOCKET 2
#define JOINT_TYPE_HINGE 3
#define JOINT_TYPE_SLIDER 4
#define JOINT_TYPE_CONETWIST 5
#define JOINT_TYPE_DOF 6
#elif PHYS_ENGINE_PHYSX
#define JOINT_TYPE_NONE 0
#define JOINT_TYPE_SPHERICAL 1
#define JOINT_TYPE_FIXED 2
#define JOINT_TYPE_REVOLUTE 3
#define JOINT_TYPE_DISTANCE 4
#define JOINT_TYPE_PRISMATIC 5
#endif

struct DLLNETWORK JointInfo
{
	JointInfo(unsigned char ptype,unsigned int _src,unsigned int _dest)
		: type(ptype),src(_src),dest(_dest),collide(false)
	{}
	JointInfo()
		: JointInfo(JOINT_TYPE_NONE,0,0)
	{}
	unsigned char type;
	unsigned int src;
	unsigned int dest;
	bool collide;
	std::unordered_map<std::string,std::string> args;
};

#endif