// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:physics.contact;

export import :physics.collision_object;
export import :physics.material;
export import :physics.shape;

export {
	namespace pragma::physics {
		struct DLLNETWORK ContactPoint {
			Vector3 impulse;
			Vector3 normal;
			Vector3 position;
			float distance;
			pragma::util::WeakHandle<IMaterial> material0;
			pragma::util::WeakHandle<IMaterial> material1;
		};
		struct DLLNETWORK ContactInfo {
			enum class Flags : uint32_t { None = 0u, StartTouch = 1u, EndTouch = StartTouch << 1u };
			pragma::util::WeakHandle<IShape> shape0;
			pragma::util::WeakHandle<IShape> shape1;
			pragma::util::TWeakSharedHandle<ICollisionObject> collisionObj0;
			pragma::util::TWeakSharedHandle<ICollisionObject> collisionObj1;
			std::vector<ContactPoint> contactPoints;
			Flags flags;
		};
		using namespace pragma::math::scoped_enum::bitwise;
	};
	REGISTER_ENUM_FLAGS(pragma::physics::ContactInfo::Flags)
};
