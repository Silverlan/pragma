/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __PRAGMA_TYPES_HPP__
#define __PRAGMA_TYPES_HPP__

#include "pragma/networkdefinitions.h"
#include <cinttypes>

namespace util {class EnumRegister;};

namespace umath
{
	struct Vertex;
	struct VertexWeight;
};

enum class Activity : uint16_t;
using BoneId = uint16_t;
class Skeleton;
namespace pragma::asset {enum class Type : uint8_t;};
namespace pragma::animation
{
	using AnimationId = uint32_t;
	constexpr auto INVALID_ANIMATION = std::numeric_limits<AnimationId>::max();

	using AnimationChannelId = uint16_t;
	class Animation;
	class AnimationPlayer;
	using PAnimationPlayer = std::shared_ptr<AnimationPlayer>;
	class AnimatedPose;

	using LayeredAnimationSlot = uint32_t;
};

#endif
