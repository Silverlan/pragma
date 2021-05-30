/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __PHYS_CONTACT_HPP__
#define __PHYS_CONTACT_HPP__

#include "pragma/networkdefinitions.h"
#include <mathutil/uvec.h>
#include <sharedutils/util_weak_handle.hpp>
#include <sharedutils/util_shared_handle.hpp>

namespace pragma::physics
{
	class IMaterial;
	class ICollisionObject;
	class IShape;
	struct DLLNETWORK ContactPoint
	{
		Vector3 impulse;
		Vector3 normal;
		Vector3 position;
		float distance;
		util::WeakHandle<IMaterial> material0;
		util::WeakHandle<IMaterial> material1;
	};
	struct DLLNETWORK ContactInfo
	{
		enum class Flags : uint32_t
		{
			None = 0u,
			StartTouch = 1u,
			EndTouch = StartTouch<<1u
		};
		util::WeakHandle<IShape> shape0;
		util::WeakHandle<IShape> shape1;
		util::TWeakSharedHandle<ICollisionObject> collisionObj0;
		util::TWeakSharedHandle<ICollisionObject> collisionObj1;
		std::vector<ContactPoint> contactPoints;
		Flags flags;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::physics::ContactInfo::Flags)

#endif
