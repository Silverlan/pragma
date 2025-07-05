// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
	using BoneId = uint16_t;
	using FlexControllerId = uint32_t;
	using LayeredAnimationSlot = uint32_t;
	class Pose;
	struct Bone;
	class Skeleton;

	static constexpr auto INVALID_BONE_INDEX = std::numeric_limits<BoneId>::max();
	static constexpr auto INVALID_FLEX_CONTROLLER_INDEX = std::numeric_limits<FlexControllerId>::max();
};

#endif
