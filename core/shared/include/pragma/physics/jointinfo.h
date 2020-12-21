/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __JOINTINFO_H__
#define __JOINTINFO_H__

#include "pragma/networkdefinitions.h"

enum class JointType : uint8_t
{
	None = 0,
	Fixed,
	BallSocket,
	Hinge,
	Slider,
	ConeTwist,
	DOF
};

using BoneId = uint32_t;
struct DLLNETWORK JointInfo
{
	JointInfo(JointType type,BoneId child,BoneId parent)
		: type(type),parent(parent),child(child),collide(false)
	{}
	JointInfo()
		: JointInfo(JointType::None,0,0)
	{}
	JointType type;
	BoneId parent;
	BoneId child;
	bool collide;
	std::unordered_map<std::string,std::string> args;
};

#endif