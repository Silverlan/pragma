/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __PRAGMA_TYPES_HPP__
#define __PRAGMA_TYPES_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/entities/baseentity_handle.h"
#include <sharedutils/util_shared_handle.hpp>
#include <cinttypes>

namespace util {
	class EnumRegister;
};

class PhysObj;
using PhysObjHandle = util::TWeakSharedHandle<PhysObj>;

namespace udm {
	struct Property;
	using PProperty = std::shared_ptr<Property>;
};

namespace umath {
	struct Vertex;
	struct VertexWeight;
};

enum class Activity : uint16_t;
using BoneId = uint16_t;
namespace pragma {
	using RenderBufferIndex = uint32_t;
	template<class T>
	using ComponentHandle = util::TWeakSharedHandle<T>;

	using ComponentId = uint32_t;
	using ComponentMemberIndex = uint32_t;
	static constexpr auto INVALID_COMPONENT_MEMBER_INDEX = std::numeric_limits<ComponentMemberIndex>::max();
	class BaseEntityComponent;
	class EntityComponentManager;
	struct ComponentMemberInfo;
};
namespace pragma::asset {
	enum class Type : uint8_t;
};
namespace pragma::animation {
	using LayeredAnimationSlot = uint32_t;
};

namespace panima {
	using AnimationId = uint32_t;
	using AnimationChannelId = uint16_t;
	class Animation;
	struct Channel;
	class Manager;
	class Player;
	class Pose;
	struct Slice;
	using BoneId = uint16_t;
	struct Bone;
	class Skeleton;

	using PPlayer = std::shared_ptr<Player>;
};

#endif
