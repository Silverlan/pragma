// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <mathutil/uvec.h>
#include <sharedutils/util_weak_handle.hpp>
#include <sharedutils/util_shared_handle.hpp>

export module pragma.shared:physics.contact;

export {
	namespace pragma::physics {
		struct DLLNETWORK ContactPoint {
			Vector3 impulse;
			Vector3 normal;
			Vector3 position;
			float distance;
			util::WeakHandle<IMaterial> material0;
			util::WeakHandle<IMaterial> material1;
		};
		struct DLLNETWORK ContactInfo {
			enum class Flags : uint32_t { None = 0u, StartTouch = 1u, EndTouch = StartTouch << 1u };
			util::WeakHandle<IShape> shape0;
			util::WeakHandle<IShape> shape1;
			util::TWeakSharedHandle<ICollisionObject> collisionObj0;
			util::TWeakSharedHandle<ICollisionObject> collisionObj1;
			std::vector<ContactPoint> contactPoints;
			Flags flags;
		};
	};
	REGISTER_BASIC_BITWISE_OPERATORS(pragma::physics::ContactInfo::Flags)
};
