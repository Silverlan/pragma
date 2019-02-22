#ifndef __UTIL_IK_HPP__
#define __UTIL_IK_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/physics/ik/ik_method.hpp"

class Node;
class PhysTransform;
namespace util
{
	namespace ik
	{
		DLLNETWORK void get_local_transform(const Node &node,PhysTransform &t);
		DLLNETWORK Quat get_rotation(const Node &node);
	};
};

#endif
