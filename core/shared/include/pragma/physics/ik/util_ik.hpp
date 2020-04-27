/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __UTIL_IK_HPP__
#define __UTIL_IK_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/physics/ik/ik_method.hpp"

class Node;
namespace pragma::physics {class Transform;};
namespace util
{
	namespace ik
	{
		DLLNETWORK void get_local_transform(const Node &node,pragma::physics::Transform &t);
		DLLNETWORK Quat get_rotation(const Node &node);
	};
};

#endif
