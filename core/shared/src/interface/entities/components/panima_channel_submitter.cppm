// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cassert>

export module pragma.shared:entities.components.panima_channel_submitter;

import :console.output;
export import :entities.components.base;
export import :game.animation_channel_cache_data;
export import panima;

export {
	constexpr auto g_debugPrintPanima = false;

	constexpr uint8_t get_component_count(udm::Type type)
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

	constexpr bool is_type_compatible(udm::Type channelType, udm::Type memberType) { return get_component_count(channelType) <= get_component_count(memberType); }

	template<typename TChannel, typename TMember, auto TMapArray>
	    requires(pragma::is_animatable_type_v<TChannel> && pragma::is_animatable_type_v<TMember> && is_type_compatible(udm::type_to_enum<TChannel>(), udm::type_to_enum<TMember>()))
	panima::ChannelValueSubmitter get_member_channel_submitter(pragma::BaseEntityComponent &component, const pragma::AnimationChannelCacheData &cacheData, uint32_t memberIdx, void (*setter)(const pragma::ComponentMemberInfo &, pragma::BaseEntityComponent &, const void *, void *),
	  void *userData = nullptr);

	template<typename TChannel, typename TMember, typename T, uint32_t I, uint32_t ARRAY_INDEX_COUNT, T MAX_ARRAY_VALUE, template<typename, typename, auto TTFunc> class TFunc, T... values>
	panima::ChannelValueSubmitter runtime_array_to_compile_time(pragma::BaseEntityComponent &component, const pragma::AnimationChannelCacheData &cacheData, uint32_t memberIdx, void (*setter)(const pragma::ComponentMemberInfo &, pragma::BaseEntityComponent &, const void *, void *),
	  void *userData, const std::array<T, ARRAY_INDEX_COUNT> &rtValues);

	template<typename TChannel, typename TMember, auto TMapArray>
	struct get_member_channel_submitter_wrapper {
		panima::ChannelValueSubmitter operator()(pragma::BaseEntityComponent &component, const pragma::AnimationChannelCacheData &cacheData, uint32_t memberIdx, void (*setter)(const pragma::ComponentMemberInfo &, pragma::BaseEntityComponent &, const void *, void *), void *userData) const
		{
			return get_member_channel_submitter<TChannel, TMember, TMapArray>(component, cacheData, memberIdx, setter, userData);
		}
	};

	template<typename TChannel, typename TMember, auto TMapArray>
	    requires(pragma::is_animatable_type_v<TChannel> && pragma::is_animatable_type_v<TMember> && is_type_compatible(udm::type_to_enum<TChannel>(), udm::type_to_enum<TMember>()))
	panima::ChannelValueSubmitter get_member_channel_submitter(pragma::BaseEntityComponent &component, const pragma::AnimationChannelCacheData &cacheData, uint32_t memberIdx, void (*setter)(const pragma::ComponentMemberInfo &, pragma::BaseEntityComponent &, const void *, void *),
	  void *userData)
	{
		return panima::ChannelValueSubmitter {[&component, &cacheData, memberIdx, setter, userData](panima::Channel &channel, uint32_t &inOutPivotTimeIndex, double t) mutable {
			auto *memberInfo = cacheData.memberInfo; //component.GetMemberInfo(memberIdx);
			assert(memberInfo);
			if constexpr(std::is_same_v<TChannel, TMember>) {
				auto value = channel.GetInterpolatedValue<TChannel>(t, inOutPivotTimeIndex, memberInfo->interpolationFunction);

				channel.ApplyValueExpression<TChannel>(t, inOutPivotTimeIndex, value);
				if constexpr(g_debugPrintPanima) {
					TMember curVal;
					memberInfo->getterFunction(*memberInfo, component, &curVal);
					Con::COUT << "Changing channel value '" << channel.targetPath.ToUri() << " from " << to_string(curVal) << " to " << to_string(value) << " (t: " << t << ")..." << Con::endl;
				}
				auto &changed = const_cast<pragma::AnimationChannelCacheData &>(cacheData).changed;
				if(!pragma::math::is_flag_set(changed, pragma::AnimationChannelCacheData::State::Dirty | pragma::AnimationChannelCacheData::State::AlwaysDirty)) {
					if(memcmp(const_cast<void *>(static_cast<const void *>(cacheData.data.data())), &value, sizeof(value)) != 0)
						changed |= pragma::AnimationChannelCacheData::State::Dirty;
				}
				memcpy(const_cast<void *>(static_cast<const void *>(cacheData.data.data())), &value, sizeof(value));
				// setter(*memberInfo, component, &value, userData);
			}
			else {
				// Interpolation function cannot be used unless the type is an exact match
				auto value = channel.GetInterpolatedValue<TChannel>(t, inOutPivotTimeIndex);
				channel.ApplyValueExpression<TChannel>(t, inOutPivotTimeIndex, value);
				constexpr auto numChannelComponents = get_component_count(udm::type_to_enum<TChannel>());
				constexpr auto numMemberComponents = get_component_count(udm::type_to_enum<TMember>());
				static_assert(numChannelComponents == TMapArray.size());
				TMember curVal;
				memberInfo->getterFunction(*memberInfo, component, &curVal);
				if constexpr(numChannelComponents == 1) {
					if constexpr(numMemberComponents > 1)
						curVal[TMapArray[0]] = value;
					else
						curVal = value;
				}
				else {
					// Would be nicer to use a loop, but since there are no constexpr
					// loops as of C++20, we'll just do it manually...
					curVal[TMapArray[0]] = value[0];
					curVal[TMapArray[1]] = value[1];
					if constexpr(numChannelComponents > 2) {
						curVal[TMapArray[2]] = value[2];
						if constexpr(numChannelComponents > 3)
							curVal[TMapArray[3]] = value[3];
					}
				}
				if constexpr(g_debugPrintPanima) {
					TMember curVal;
					memberInfo->getterFunction(*memberInfo, component, &curVal);
					Con::COUT << "Changing " << TMapArray.size() << " components of channel value '" << channel.targetPath.ToUri() << " from " << to_string(curVal) << " to " << to_string(value) << " (t: " << t << ")..." << Con::endl;
				}
				auto &changed = const_cast<pragma::AnimationChannelCacheData &>(cacheData).changed;
				if(!pragma::math::is_flag_set(changed, pragma::AnimationChannelCacheData::State::Dirty | pragma::AnimationChannelCacheData::State::AlwaysDirty)) {
					if(memcmp(const_cast<void *>(static_cast<const void *>(cacheData.data.data())), &curVal, sizeof(curVal)) != 0)
						changed |= pragma::AnimationChannelCacheData::State::Dirty;
				}
				memcpy(const_cast<void *>(static_cast<const void *>(cacheData.data.data())), &curVal, sizeof(curVal));
				// setter(*memberInfo, component, &curVal, userData);
			}
		}};
	}

	template<typename TChannel, typename TMember, typename T, uint32_t I, uint32_t ARRAY_INDEX_COUNT, T MAX_ARRAY_VALUE, template<typename, typename, auto TTFunc> class TFunc, T... values>
	panima::ChannelValueSubmitter runtime_array_to_compile_time(pragma::BaseEntityComponent &component, const pragma::AnimationChannelCacheData &cacheData, uint32_t memberIdx, void (*setter)(const pragma::ComponentMemberInfo &, pragma::BaseEntityComponent &, const void *, void *),
	  void *userData, const std::array<T, ARRAY_INDEX_COUNT> &rtValues);

	template<typename TChannel, typename TMember, typename T, uint32_t I, uint32_t VAL, uint32_t ARRAY_INDEX_COUNT, T MAX_ARRAY_VALUE, template<typename, typename, auto TTFunc> class TFunc, T... values>
	panima::ChannelValueSubmitter runtime_array_to_compile_time_it(pragma::BaseEntityComponent &component, const pragma::AnimationChannelCacheData &cacheData, uint32_t memberIdx, void (*setter)(const pragma::ComponentMemberInfo &, pragma::BaseEntityComponent &, const void *, void *),
	  void *userData, const std::array<T, ARRAY_INDEX_COUNT> &rtValues)
	{
		if(rtValues[I] == VAL)
			return runtime_array_to_compile_time<TChannel, TMember, T, I + 1, ARRAY_INDEX_COUNT, MAX_ARRAY_VALUE, TFunc, values..., VAL>(component, cacheData, memberIdx, setter, userData, rtValues);
		else {
			if constexpr(VAL <= MAX_ARRAY_VALUE)
				return runtime_array_to_compile_time_it<TChannel, TMember, T, I, VAL + 1, ARRAY_INDEX_COUNT, MAX_ARRAY_VALUE, TFunc, values...>(component, cacheData, memberIdx, setter, userData, rtValues);
		}
		return panima::ChannelValueSubmitter {nullptr};
	}

	template<typename TChannel, typename TMember, typename T, uint32_t I, uint32_t ARRAY_INDEX_COUNT, T MAX_ARRAY_VALUE, template<typename, typename, auto TTFunc> class TFunc, T... values>
	panima::ChannelValueSubmitter runtime_array_to_compile_time(pragma::BaseEntityComponent &component, const pragma::AnimationChannelCacheData &cacheData, uint32_t memberIdx, void (*setter)(const pragma::ComponentMemberInfo &, pragma::BaseEntityComponent &, const void *, void *),
	  void *userData, const std::array<T, ARRAY_INDEX_COUNT> &rtValues)
	{
		if constexpr(I < ARRAY_INDEX_COUNT)
			return runtime_array_to_compile_time_it<TChannel, TMember, T, I, 0, ARRAY_INDEX_COUNT, MAX_ARRAY_VALUE, TFunc, values...>(component, cacheData, memberIdx, setter, userData, rtValues);
		else
			return TFunc<TChannel, TMember, std::array<T, ARRAY_INDEX_COUNT> {values...}> {}(component, cacheData, memberIdx, setter, userData);
	}

#pragma warning(push)
#pragma warning(disable : 4700)
	template<typename TChannel>
	void instantiate_get_member_channel_submitter()
	{
		if constexpr(pragma::is_animatable_type_v<TChannel>) {
			udm::visit_ng({}, [](auto tag) {
				using TMember = typename decltype(tag)::type;
				if constexpr(pragma::is_animatable_type_v<TMember> && is_type_compatible(udm::type_to_enum<TChannel>(), udm::type_to_enum<TMember>())) {
					// These are just dummy values and do not matter.
					pragma::BaseEntityComponent *component;
					uint32_t memberIdx;
					void (*setter)(const pragma::ComponentMemberInfo &, pragma::BaseEntityComponent &, const void *, void *);
					void *userData;
					pragma::AnimationChannelCacheData *cacheData;
					if constexpr(std::is_same_v<TChannel, TMember>)
						get_member_channel_submitter<TChannel, TMember, 0>(*component, *cacheData, memberIdx, setter, userData);

					constexpr auto numComponentsChannel = get_component_count(udm::type_to_enum<TChannel>());
					constexpr auto numComponentsMember = get_component_count(udm::type_to_enum<TMember>());
					if constexpr(numComponentsChannel > 0 && numComponentsMember > 0) {
						runtime_array_to_compile_time<TChannel, TMember, uint32_t, 0, numComponentsChannel, numComponentsMember, get_member_channel_submitter_wrapper>(*component, *cacheData, memberIdx, setter, nullptr, {});
					}
				}
			});
		}
	}
#pragma warning(pop)
}
