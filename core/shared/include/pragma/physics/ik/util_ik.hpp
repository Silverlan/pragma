// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __UTIL_IK_HPP__
#define __UTIL_IK_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/physics/ik/ik_method.hpp"

class Node;
namespace pragma::physics {
	class Transform;
};
namespace util {
	namespace ik {
		DLLNETWORK void get_local_transform(const Node &node, umath::Transform &t);
		DLLNETWORK Quat get_rotation(const Node &node);
	};
};

#endif
