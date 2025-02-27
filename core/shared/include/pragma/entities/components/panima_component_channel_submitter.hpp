/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __PANIMA_COMPONENT_CHANNEL_SUBMITTER_HPP__
#define __PANIMA_COMPONENT_CHANNEL_SUBMITTER_HPP__

#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/game/animation_channel_cache_data.hpp"

import panima;

static constexpr auto g_debugPrint = false;

static constexpr uint8_t get_component_count(udm::Type type)
{
	if(udm::is_numeric_type(type))
		return 1;
	switch(type) {
	case udm::Type::Vector2:
	case udm::Type::Vector2i:
		return 2;
	case udm::Type::Vector3:
	case udm::Type::Vector3i:
	case udm::Type::EulerAngles:
		return 3;
	case udm::Type::Vector4:
	case udm::Type::Vector4i:
		return 4;
	}
	return 0;
}

static constexpr bool is_type_compatible(udm::Type channelType, udm::Type memberType) { return get_component_count(channelType) <= get_component_count(memberType); }

template<typename TChannel, typename TMember, auto TMapArray>
    requires(pragma::is_animatable_type_v<TChannel> && pragma::is_animatable_type_v<TMember> && is_type_compatible(udm::type_to_enum<TChannel>(), udm::type_to_enum<TMember>()))
panima::ChannelValueSubmitter get_member_channel_submitter(pragma::BaseEntityComponent &component, const pragma::AnimationChannelCacheData &cacheData, uint32_t memberIdx, void (*setter)(const pragma::ComponentMemberInfo &, pragma::BaseEntityComponent &, const void *, void *),
  void *userData = nullptr);

template<typename TChannel, typename TMember, typename T, uint32_t I, uint32_t ARRAY_INDEX_COUNT, T MAX_ARRAY_VALUE, template<typename, typename, auto TTFunc> class TFunc, T... values>
panima::ChannelValueSubmitter runtime_array_to_compile_time(pragma::BaseEntityComponent &component, const pragma::AnimationChannelCacheData &cacheData, uint32_t memberIdx, void (*setter)(const pragma::ComponentMemberInfo &, pragma::BaseEntityComponent &, const void *, void *),
  void *userData,
  const std::array<T, ARRAY_INDEX_COUNT> &rtValues);

template<typename TChannel, typename TMember, auto TMapArray>
struct get_member_channel_submitter_wrapper {
	panima::ChannelValueSubmitter operator()(pragma::BaseEntityComponent &component, const pragma::AnimationChannelCacheData &cacheData, uint32_t memberIdx, void (*setter)(const pragma::ComponentMemberInfo &, pragma::BaseEntityComponent &, const void *, void *), void *userData) const
	{
		return get_member_channel_submitter<TChannel, TMember, TMapArray>(component, cacheData, memberIdx, setter, userData);
	}
};

#endif
