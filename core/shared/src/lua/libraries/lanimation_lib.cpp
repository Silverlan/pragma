/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/model/animation/skeletal_animation.hpp"
#include "pragma/model/animation/play_animation_flags.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/lua/libraries/ludm.hpp"
#include "pragma/lua/types/udm.hpp"
#include "pragma/lua/custom_constructor.hpp"

#include "pragma/lua/ostream_operator_alias.hpp"
#include "pragma/model/model.h"
#include <pragma/lua/policies/default_parameter_policy.hpp>
#include <pragma/lua/policies/shared_from_this_policy.hpp>
#include <pragma/lua/converters/string_view_converter_t.hpp>
#include <pragma/lua/converters/optional_converter_t.hpp>
#include <pragma/lua/converters/vector_converter_t.hpp>
#include <pragma/lua/converters/pair_converter_t.hpp>
#include <luainterface.hpp>
#include "pragma/model/animation/pose.hpp"
#include "pragma/model/animation/skeleton.hpp"
#include <panima/channel.hpp>
#include <panima/channel_t.hpp>
#include <panima/player.hpp>
#include <panima/animation.hpp>
#include <panima/animation_set.hpp>
#include <panima/animation_manager.hpp>
#include <panima/slice.hpp>
#include <luabind/copy_policy.hpp>

namespace Lua::animation {
	void register_library(Lua::Interface &lua);
};

// See https://stackoverflow.com/a/1267878/2482983
template<typename T>
static void reorder(udm::Array &v, std::vector<size_t> const &order)
{
	for(int s = 1, d; s < order.size(); ++s) {
		for(d = order[s]; d < s; d = order[d])
			;
		if(d == s)
			while(d = order[d], d != s)
				std::swap(v.GetValue<T>(s), v.GetValue<T>(d));
	}
}

static std::optional<uint32_t> set_channel_time(lua_State *l, panima::Channel &channel, uint32_t valueIndex, float time, bool resort, bool removeExisting)
{
	if(removeExisting) {
		auto existingIndex = channel.FindValueIndex(time);
		if(existingIndex && *existingIndex != valueIndex) {
			// Make sure the new timestamp isn't already occupied
			channel.RemoveValueAtIndex(*existingIndex);
			if(valueIndex > *existingIndex)
				--valueIndex;
		}
	}
	if(!resort) {
		auto r = Lua::udm::set_array_value(l, channel.GetTimesArray(), valueIndex, luabind::object {l, time});
		channel.Update();
		return r ? valueIndex : std::optional<uint32_t> {};
	}
	auto &times = channel.GetTimesArray();
	auto &values = channel.GetValueArray();
	auto oldTime = channel.GetTime(valueIndex);
	times[valueIndex] = time;

	auto numTimes = times.GetSize();
	auto getTime = [numTimes, &times](uint32_t idx) -> std::optional<float> {
		if(idx >= numTimes)
			return {};
		return times.GetValue<float>(idx);
	};

	// Since we changed the time value, we may have to re-order
	auto swapValue = [&times, &values](uint32_t idx0, uint32_t idx1) {
		auto *time0 = times.GetValuePtr<float>(idx0);
		auto *time1 = times.GetValuePtr<float>(idx1);
		std::swap(*time0, *time1);

		::udm::visit_ng(values.GetValueType(), [&values, idx0, idx1](auto tag) {
			using T = typename decltype(tag)::type;
			auto *val0 = values.GetValuePtr<T>(idx0);
			auto *val1 = values.GetValuePtr<T>(idx1);
			std::swap(*val0, *val1);
		});
	};
	auto tNext = getTime(valueIndex + 1);
	const std::optional<float> nil {};
	while(tNext.has_value() && tNext < time) {
		// Value needs to be moved up
		swapValue(valueIndex, valueIndex + 1);
		valueIndex = valueIndex + 1;
		tNext = getTime(valueIndex + 1);
	}

	auto tPrev = (valueIndex > 0) ? getTime(valueIndex - 1) : nil;
	while(tPrev.has_value() && tPrev > time) {
		// Value needs to be moved down
		swapValue(valueIndex, valueIndex - 1);
		valueIndex = valueIndex - 1;
		tPrev = (valueIndex > 0) ? getTime(valueIndex - 1) : nil;
	}
	//

	channel.Update();
	return valueIndex;
}

static Lua::opt<Lua::udm_ng> get_interpolated_value(lua_State *l, panima::Channel &channel, float t, bool clampToBounds)
{
	if(channel.GetTimeCount() == 0)
		return Lua::nil;
	if(clampToBounds) {
		if(t < channel.GetTime(0) || t > channel.GetTime(channel.GetTimeCount() - 1))
			return Lua::nil;
	}
	float factor;
	auto indices = channel.FindInterpolationIndices(t, factor);
	if(indices.first == std::numeric_limits<decltype(indices.first)>::max())
		return Lua::nil;
	return ::udm::visit_ng(channel.GetValueType(), [l, &channel, indices, factor](auto tag) {
		using T = typename decltype(tag)::type;
		auto &value0 = channel.GetValue<T>(indices.first);
		auto &value1 = channel.GetValue<T>(indices.second);
		T result;
		::udm::lerp_value(value0, value1, factor, result, udm::type_to_enum<T>());
		return luabind::object {l, result};
	});
}

static std::optional<std::pair<uint32_t, uint32_t>> find_index_range_in_time_range(panima::Channel &channel, float startTime, float endTime, bool clampToOuterBoundaries)
{
	float startInterp, endInterp;
	auto start = channel.FindInterpolationIndices(startTime, startInterp);
	auto end = channel.FindInterpolationIndices(endTime, endInterp);
	if(start.first == std::numeric_limits<decltype(start.first)>::max() || start.second == std::numeric_limits<decltype(start.first)>::max())
		return {};
	uint32_t startIndex, endIndex;
	if(clampToOuterBoundaries) {
		if(start.first == 0)
			startIndex = 0;
		else if(startInterp == 0.f)
			startIndex = start.first;
		else if(startInterp == 1.f)
			startIndex = start.second; /* unreachable? */
		else
			startIndex = start.first - 1;

		if(end.second == 0)
			endIndex = 0;
		else if(endInterp == 0.f)
			endIndex = end.first;
		else if(endInterp == 1.f)
			endIndex = start.second; /* unreachable? */
		else
			endIndex = end.second;
	}
	else {
		if(startInterp == 0.f)
			startIndex = start.first;
		else
			startIndex = start.second;

		if(endInterp == 1.f)
			endIndex = end.second; /* unreachable? */
		else
			endIndex = end.first;
	}
	return std::pair<uint32_t, uint32_t> {startIndex, endIndex};
}

#ifdef __linux__
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(panima, TimeFrame);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(panima, ChannelPath);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(panima, Channel);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(panima, AnimationSet);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(panima, Player);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(panima, Slice);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(panima, AnimationManager);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(panima, Animation);
#endif

static uint32_t insert_channel_values(lua_State *l, panima::Channel &channel, const std::vector<float> &times, luabind::tableT<void> tValues, float offset, panima::Channel::InsertFlags flags)
{
	auto numTimes = times.size();
	auto numValues = Lua::GetObjectLength(l, tValues);
	if(numTimes != numValues)
		throw std::runtime_error {"Number of elements in times array (" + std::to_string(numTimes) + ") doesn't match number of values in values array (" + std::to_string(numValues) + ")! This is not allowed."};
	auto insertIndex = ::udm::visit(channel.GetValueType(), [l, &tValues, &channel, &times, numValues, offset, flags](auto tag) {
		using T = typename decltype(tag)::type;
		using TValue = std::conditional_t<std::is_same_v<T, bool>, uint8_t, T>;
		if constexpr(pragma::is_animatable_type_v<TValue>) {
			if constexpr(!std::is_same_v<T, bool>) {
				auto values = luabind::object_cast<std::vector<TValue>>(tValues);
				if(values.size() != times.size())
					throw std::runtime_error {"Number of data values does not match number of time values!"};
				return channel.InsertValues<TValue>(times.size(), times.data(), values.data(), offset, flags);
			}
			else {
				std::vector<TValue> values;
				auto n = Lua::GetObjectLength(l, tValues);
				values.reserve(n);
				if(n > 0) {
					auto type = Lua::GetType(tValues[1]);
					switch(type) {
					case Lua::Type::Bool:
						{
							for(auto i = decltype(n) {1}; i <= n; ++i) {
								auto val = luabind::object_cast<bool>(tValues[i]);
								values.push_back(static_cast<TValue>(val));
							}
							break;
						}
					default:
						{
							for(auto i = decltype(n) {1}; i <= n; ++i) {
								auto val = luabind::object_cast<TValue>(tValues[i]);
								values.push_back(val);
							}
							break;
						}
					}
				}
				if(values.size() != times.size())
					throw std::runtime_error {"Number of data values does not match number of time values!"};
				return channel.InsertValues<TValue>(times.size(), times.data(), values.data(), offset, flags);
			}
		}
		else
			return std::numeric_limits<uint32_t>::max();
	});

	channel.Update();
	return insertIndex;
}
static uint32_t insert_channel_values(lua_State *l, panima::Channel &channel, const std::vector<float> &times, luabind::tableT<void> tValues, float offset) { return insert_channel_values(l, channel, times, tValues, offset, panima::Channel::InsertFlags::ClearExistingDataInRange); }
static uint32_t insert_channel_values(lua_State *l, panima::Channel &channel, const std::vector<float> &times, luabind::tableT<void> tValues) { return insert_channel_values(l, channel, times, tValues, 0.f); }

void Lua::animation::register_library(Lua::Interface &lua)
{
	auto animMod = luabind::module(lua.GetState(), "panima");

	auto cdPose = luabind::class_<pragma::animation::Pose>("Pose");
	cdPose.def(luabind::tostring(luabind::self));
	cdPose.def(luabind::constructor<>());
	cdPose.def("SetTransformCount", &pragma::animation::Pose::SetTransformCount);
	cdPose.def("SetBoneIndex", &pragma::animation::Pose::SetBoneIndex);
	cdPose.def("GetTransform", static_cast<umath::ScaledTransform *(pragma::animation::Pose::*)(pragma::animation::BoneId)>(&pragma::animation::Pose::GetTransform));
	cdPose.def("SetTransform", &pragma::animation::Pose::SetTransform);
	cdPose.def("Clear", &pragma::animation::Pose::Clear);
	cdPose.def("Lerp", &pragma::animation::Pose::Lerp);
	cdPose.def("Localize", &pragma::animation::Pose::Localize);
	cdPose.def("Globalize", &pragma::animation::Pose::Globalize);
	cdPose.def("GetBoneTranslationTable", static_cast<std::vector<uint32_t> &(pragma::animation::Pose::*)()>(&pragma::animation::Pose::GetBoneTranslationTable));
	animMod[cdPose];

	auto cdTimeFrame = luabind::class_<panima::TimeFrame>("TimeFrame");
	cdTimeFrame.def(luabind::tostring(luabind::self));
	cdTimeFrame.def_readwrite("startOffset", &panima::TimeFrame::startOffset);
	cdTimeFrame.def_readwrite("scale", &panima::TimeFrame::scale);
	cdTimeFrame.def_readwrite("duration", &panima::TimeFrame::duration);
	animMod[cdTimeFrame];
	pragma::lua::define_custom_constructor<panima::TimeFrame, [](float startOffset, float scale, float duration) -> panima::TimeFrame { return panima::TimeFrame {startOffset, scale, duration}; }, float, float, float>(lua.GetState());
	pragma::lua::define_custom_constructor<panima::TimeFrame, []() -> panima::TimeFrame { return panima::TimeFrame {}; }>(lua.GetState());

	auto cdChannel = luabind::class_<panima::Channel>("Channel");
	cdChannel.scope[luabind::def(
	  "merge_data_arrays", +[](lua_State *l, const std::vector<float> &times0, const luabind::object &oValues0, const std::vector<float> &times1, const luabind::object &oValues1, ::udm::Type valueType) -> Lua::mult<Lua::tb<float>, Lua::tb<void>> {
		  return ::udm::visit_ng(valueType, [l, &times0, &times1, &oValues0, &oValues1](auto tag) {
			  using T = typename decltype(tag)::type;
			  using TValue = std::conditional_t<std::is_same_v<T, bool>, uint8_t, T>;
			  std::vector<float> times;
			  std::vector<TValue> values;
			  auto values0 = luabind::object_cast<std::vector<TValue>>(oValues0);
			  auto values1 = luabind::object_cast<std::vector<TValue>>(oValues1);
			  panima::Channel::MergeDataArrays(times0, values0, times1, values1, times, values);
			  return luabind::object {l, std::pair<std::vector<float>, std::vector<TValue>> {std::move(times), std::move(values)}};
		  });
	  })];
	cdChannel.add_static_constant("INSERT_FLAG_NONE", umath::to_integral(panima::Channel::InsertFlags::None));
	cdChannel.add_static_constant("INSERT_FLAG_BIT_CLEAR_EXISTING_DATA_IN_RANGE", umath::to_integral(panima::Channel::InsertFlags::ClearExistingDataInRange));
	cdChannel.add_static_constant("INSERT_FLAG_BIT_DECIMATE_INSERTED_DATA", umath::to_integral(panima::Channel::InsertFlags::DecimateInsertedData));

	auto cdPath = luabind::class_<panima::ChannelPath>("Path");
	cdPath.def(luabind::constructor<>());
	cdPath.def(luabind::constructor<const std::string &>());
	cdPath.def(luabind::tostring(luabind::self));
	cdPath.def_readwrite("path", &panima::ChannelPath::path);
	cdPath.property("components", static_cast<std::vector<std::string> *(panima::ChannelPath::*)()>(&panima::ChannelPath::GetComponents));
	cdPath.def("ToUri", &panima::ChannelPath::ToUri);
	cdPath.def("ToUri", &panima::ChannelPath::ToUri, luabind::default_parameter_policy<2, true> {});
	cdChannel.scope[cdPath];

	cdChannel.def(luabind::tostring(luabind::self));
	cdChannel.def(luabind::const_self == luabind::const_self);
	cdChannel.def("Update", &panima::Channel::Update);
	cdChannel.def("GetTimeFrame", static_cast<panima::TimeFrame &(panima::Channel::*)()>(&panima::Channel::GetTimeFrame));
	cdChannel.def("SetTimeFrame", &panima::Channel::SetTimeFrame);
	cdChannel.def("Optimize", &panima::Channel::Optimize);
	cdChannel.def("GetValueType", &panima::Channel::GetValueType);
	cdChannel.def("SetValueType", &panima::Channel::SetValueType);
	cdChannel.def("MergeValues", &panima::Channel::MergeValues);
	cdChannel.def(
	  "GetInterpolation", +[](lua_State *l, panima::Channel &channel) -> panima::ChannelInterpolation { return channel.interpolation; });
	cdChannel.def(
	  "SetInterpolation", +[](lua_State *l, panima::Channel &channel, panima::ChannelInterpolation interp) { channel.interpolation = interp; });
	cdChannel.def(
	  "GetTargetPath", +[](lua_State *l, panima::Channel &channel) -> panima::ChannelPath * { return &channel.targetPath; });
	cdChannel.def(
	  "SetTargetPath", +[](lua_State *l, panima::Channel &channel, const std::string &path) { channel.targetPath = path; });
	cdChannel.def(
	  "GetTimesArray", +[](lua_State *l, panima::Channel &channel) -> ::udm::LinkedPropertyWrapper { return ::udm::LinkedPropertyWrapper {channel.GetTimesProperty()}; });
	cdChannel.def(
	  "GetValueArray", +[](lua_State *l, panima::Channel &channel) -> ::udm::LinkedPropertyWrapper { return ::udm::LinkedPropertyWrapper {channel.GetValueProperty()}; });
	cdChannel.def("Resize", &panima::Channel::Resize);
	cdChannel.def("GetSize", &panima::Channel::GetSize);
	cdChannel.def(
	  "AddValue", +[](lua_State *l, panima::Channel &channel, float t, Lua::udm_ng value) -> uint32_t {
		  return ::udm::visit_ng(channel.GetValueType(), [&channel, t, &value](auto tag) {
			  using T = typename decltype(tag)::type;
			  auto v = Lua::udm::cast_object<T>(luabind::object {value});
			  return channel.AddValue(t, v);
		  });
	  });
	cdChannel.def("Save", &panima::Channel::Save);
	cdChannel.def("Load", &panima::Channel::Load);
	cdChannel.def("InsertSample", &panima::Channel::InsertSample);
	cdChannel.def("ScaleTimeInRange", &panima::Channel::ScaleTimeInRange);
	cdChannel.def("ScaleTimeInRange", &panima::Channel::ScaleTimeInRange, luabind::default_parameter_policy<6, true> {});
	cdChannel.def("ShiftTimeInRange", &panima::Channel::ShiftTimeInRange);
	cdChannel.def("ShiftTimeInRange", &panima::Channel::ShiftTimeInRange, luabind::default_parameter_policy<5, true> {});
	cdChannel.def("TransformGlobal", &panima::Channel::TransformGlobal);
	cdChannel.def("ClearAnimationData", &panima::Channel::ClearAnimationData);
	cdChannel.def("ClearRange", &panima::Channel::ClearRange);
	cdChannel.def("ClearRange", &panima::Channel::ClearRange, luabind::default_parameter_policy<4, true> {});
	cdChannel.def("ResolveDuplicates", &panima::Channel::ResolveDuplicates);
	cdChannel.def(
	  "RemoveValue", +[](lua_State *l, panima::Channel &channel, uint32_t idx) -> bool {
		  auto &times = channel.GetTimesArray();
		  auto &values = channel.GetValueArray();
		  if(idx >= times.GetSize() || idx >= values.GetSize())
			  return false;
		  times.RemoveValue(idx);
		  values.RemoveValue(idx);
		  channel.Update();
		  return true;
	  });
	cdChannel.def(
	  "SetTime", +[](lua_State *l, panima::Channel &channel, uint32_t idx, float time) -> bool {
		  auto r = set_channel_time(l, channel, idx, time, false, true);
		  return r.has_value();
	  });
	cdChannel.def(
	  "SetTime", +[](lua_State *l, panima::Channel &channel, uint32_t idx, float time, bool resort) -> bool {
		  auto r = set_channel_time(l, channel, idx, time, resort, true);
		  return r.has_value();
	  });
	cdChannel.def("SetTime", &set_channel_time);
	cdChannel.def(
	  "SetValue", +[](lua_State *l, panima::Channel &channel, uint32_t idx, const luabind::object &value) -> bool {
		  auto r = Lua::udm::set_array_value(l, channel.GetValueArray(), idx, value);
		  channel.Update();
		  return r;
	  });
	cdChannel.def("Validate", &panima::Channel::Validate);
	cdChannel.def(
	  "DebugPrint", +[](panima::Channel &channel) {
		  auto data = ::udm::Data::Create();
		  auto udmData = data->GetAssetData().GetData();
		  channel.Save(udmData);
		  std::stringstream ss;
		  data->ToAscii(ss, ::udm::AsciiSaveFlags::Default | :: udm::AsciiSaveFlags::DontCompressLz4Arrays);
		  Con::cout << ss.str() << Con::endl;
	  });
	cdChannel.def(
	  "PopulateRandom", +[](panima::Channel &channel, uint32_t numValues, float minTime, float maxTime, float minVal, float maxVal) {
		  return ::udm::visit_ng(channel.GetValueType(), [&channel, numValues, minTime, maxTime, minVal, maxVal](auto tag) {
			  using T = typename decltype(tag)::type;
			  using TValue = std::conditional_t<std::is_same_v<T, bool>, uint8_t, T>;
			  if constexpr(panima::is_animatable_type(::udm::type_to_enum<TValue>())) {
				  std::vector<float> times;
				  std::vector<TValue> values;
				  times.reserve(numValues);
				  values.reserve(numValues);
				  auto nc = ::udm::get_numeric_component_count(::udm::type_to_enum<TValue>());
				  for(auto i = decltype(numValues) {0u}; i < numValues; ++i) {
					  times.push_back(umath::random(minTime, maxTime));

					  using TBase = std::conditional_t<std::is_same_v<TValue, ::udm::Quaternion>, ::udm::EulerAngles, TValue>;
					  TBase value {};
					  for(auto i = decltype(nc) {0u}; i < nc; ++i)
						  ::udm::get_numeric_component(value, i) = umath::random(minVal, maxVal);
					  if constexpr(std::is_same_v<TValue, ::udm::Quaternion>)
						  values.push_back(uquat::create(value));
					  else
						  values.push_back(value);
				  }
				  std::sort(times.begin(), times.end());
				  channel.InsertValues<TValue>(numValues, times.data(), values.data());
			  }
		  });
	  });
	cdChannel.def("InsertValues", static_cast<uint32_t (*)(lua_State *, panima::Channel &, const std::vector<float> &, luabind::tableT<void>, float, panima::Channel::InsertFlags)>(&insert_channel_values));
	cdChannel.def("InsertValues", static_cast<uint32_t (*)(lua_State *, panima::Channel &, const std::vector<float> &, luabind::tableT<void>, float)>(&insert_channel_values));
	cdChannel.def("InsertValues", static_cast<uint32_t (*)(lua_State *, panima::Channel &, const std::vector<float> &, luabind::tableT<void>)>(&insert_channel_values));
	cdChannel.def(
	  "InsertValue", +[](lua_State *l, panima::Channel &channel, float t, const luabind::object &value) -> uint32_t {
		  auto tValues = luabind::newtable(l);
		  tValues[1] = value;
		  return insert_channel_values(l, channel, std::vector<float> {t}, tValues);
	  });
	cdChannel.def(
	  "SetValues", +[](lua_State *l, panima::Channel &channel, luabind::tableT<float> times, luabind::tableT<void> values) {
		  auto numTimes = Lua::GetObjectLength(l, times);
		  auto numValues = Lua::GetObjectLength(l, values);
		  if(numTimes != numValues)
			  throw std::runtime_error {"Number of elements in times array (" + std::to_string(numTimes) + ") doesn't match number of values in values array (" + std::to_string(numValues) + ")! This is not allowed."};
		  Lua::udm::set_array_values(l, channel.GetTimesArray(), times, 2);
		  Lua::udm::set_array_values(l, channel.GetValueArray(), values, 3);
		  channel.Update();
	  });
	cdChannel.def(
	  "SetValues", +[](lua_State *l, panima::Channel &channel, luabind::tableT<void> timeValueMap) {
		  auto type = channel.GetValueType();
		  if(type == ::udm::Type::Invalid)
			  return;
		  uint32_t count = 0;
		  for(luabind::iterator it {timeValueMap}, end; it != end; ++it)
			  ++count;

		  std::vector<std::pair<float, luabind::object>> timeValues;
		  std::vector<uint32_t> sortedIndices;
		  timeValues.resize(count);
		  sortedIndices.resize(count);

		  uint32_t idx = 0;
		  for(luabind::iterator it {timeValueMap}, end; it != end; ++it) {
			  auto &tv = timeValues[idx];
			  tv.first = luabind::object_cast<float>(it.key());
			  tv.second = luabind::object {*it};
			  sortedIndices[idx] = idx;
			  ++idx;
		  }

		  std::sort(sortedIndices.begin(), sortedIndices.end(), [&timeValues](uint32_t idx0, uint32_t idx1) -> bool { return timeValues[idx0].first < timeValues[idx1].first; });

		  auto &times = channel.GetTimesArray();
		  auto &values = channel.GetValueArray();
		  times.Resize(count);
		  values.Resize(count);

		  for(uint32_t i = 0; auto idx : sortedIndices) {
			  auto &tv = timeValues[idx];
			  times[i] = tv.first;
			  ::udm::visit_ng(type, [&tv, &values, i](auto tag) {
				  using T = typename decltype(tag)::type;
				  values[i] = Lua::udm::cast_object<T>(tv.second);
			  });
			  ++i;
		  }
		  channel.Update();
	  });
	cdChannel.def(
	  "SetValueExpression", +[](lua_State *l, panima::Channel &channel, std::string expression) -> Lua::var<bool, std::string> {
		  std::string err;
		  auto res = channel.SetValueExpression(std::move(expression), err);
		  if(res)
			  return luabind::object {l, true};
		  return luabind::object {l, err};
	  });
	cdChannel.def(
	  "TestValueExpression", +[](lua_State *l, panima::Channel &channel, std::string expression) -> Lua::var<bool, std::string> {
		  std::string err;
		  auto res = channel.TestValueExpression(std::move(expression), err);
		  if(res)
			  return luabind::object {l, true};
		  return luabind::object {l, err};
	  });
	cdChannel.def("ClearValueExpression", &panima::Channel::ClearValueExpression);
	cdChannel.def(
	  "GetValueExpression", +[](panima::Channel &channel) -> std::optional<std::string> {
		  auto *expr = channel.GetValueExpression();
		  return expr ? *expr : std::optional<std::string> {};
	  });
	cdChannel.def("GetValueCount", &panima::Channel::GetValueCount);
	cdChannel.def("GetTime", &panima::Channel::GetTime);
	cdChannel.def(
	  "GetInterpolatedValue", +[](lua_State *l, panima::Channel &channel, float t, bool clampToBounds) -> Lua::opt<Lua::udm_ng> { return get_interpolated_value(l, channel, t, clampToBounds); });
	cdChannel.def(
	  "GetInterpolatedValue", +[](lua_State *l, panima::Channel &channel, float t) -> Lua::opt<Lua::udm_ng> { return get_interpolated_value(l, channel, t, true); });
	// Default float argument not supported in clang (state: 23-09-12)
	// cdChannel.def("Decimate", static_cast<void (panima::Channel::*)(float, float, float)>(&panima::Channel::Decimate), luabind::default_parameter_policy<4, 0.03f> {});
	// cdChannel.def("Decimate", static_cast<void (panima::Channel::*)(float)>(&panima::Channel::Decimate), luabind::default_parameter_policy<2, 0.03f> {});
	cdChannel.def("Decimate", static_cast<void (panima::Channel::*)(float, float, float)>(&panima::Channel::Decimate));
	cdChannel.def(
	  "Decimate", +[](panima::Channel &channel, float tStart, float tEnd) { return channel.Decimate(tStart, tEnd); });
	cdChannel.def("Decimate", static_cast<void (panima::Channel::*)(float)>(&panima::Channel::Decimate));
	cdChannel.def(
	  "Decimate", +[](panima::Channel &channel) { return channel.Decimate(); });

	cdChannel.def(
	  "GetValue", +[](lua_State *l, panima::Channel &channel, uint32_t idx) -> Lua::udm_ng {
		  if(idx >= channel.GetValueCount())
			  return Lua::nil;
		  return ::udm::visit_ng(channel.GetValueType(), [l, &channel, idx](auto tag) {
			  using T = typename decltype(tag)::type;
			  auto value = channel.GetValue<T>(idx);
			  return luabind::object {l, value};
		  });
	  });
	cdChannel.def(
	  "GetTimes", +[](lua_State *l, panima::Channel &channel) -> Lua::tb<float> {
		  auto &a = channel.GetTimesArray();
		  auto t = luabind::newtable(l);
		  auto c = a.GetSize();
		  for(auto i = decltype(c) {0u}; i < c; ++i)
			  t[i + 1] = a.GetValue<float>(i);
		  return t;
	  });
	cdChannel.def(
	  "GetValues", +[](lua_State *l, panima::Channel &channel) -> Lua::tb<void> {
		  auto &a = channel.GetValueArray();
		  auto t = luabind::newtable(l);
		  auto c = a.GetSize();
		  ::udm::visit_ng(channel.GetValueType(), [l, &channel, c, &a, &t](auto tag) {
			  using T = typename decltype(tag)::type;
			  for(auto i = decltype(c) {0u}; i < c; ++i)
				  t[i + 1] = a.GetValue<T>(i);
		  });
		  return t;
	  });
	cdChannel.def(
	  "FindInterpolationIndices", +[](lua_State *l, panima::Channel &channel, float time) -> std::optional<std::tuple<uint32_t, uint32_t, float>> {
		  auto &t = channel.GetTimesArray();
		  float interpFactor;
		  auto indices = channel.FindInterpolationIndices(time, interpFactor);
		  if(indices.first == std::numeric_limits<decltype(indices.first)>::max())
			  return {};
		  return {std::tuple<uint32_t, uint32_t, float> {indices.first, indices.second, interpFactor}};
	  });
	cdChannel.def(
	  "FindIndex", +[](lua_State *l, panima::Channel &channel, float time) -> std::optional<uint32_t> { return channel.FindValueIndex(time, panima::Channel::VALUE_EPSILON); });
	cdChannel.def("FindIndex", &panima::Channel::FindValueIndex);
	cdChannel.def(
	  "RemoveValueRange", +[](lua_State *l, panima::Channel &channel, uint32_t startIndex, uint32_t count) {
		  auto &t = channel.GetTimesArray();
		  auto &v = channel.GetValueArray();

		  ::udm::Array::Range r0 {0 /* src */, 0 /* dst */, startIndex};
		  ::udm::Array::Range r1 {startIndex + count /* src */, startIndex /* dst */, t.GetSize() - (startIndex + count)};
		  t.Resize(t.GetSize() - count, r0, r1, false);
		  v.Resize(v.GetSize() - count, r0, r1, false);
	  });
	cdChannel.def(
	  "AddValueRange", +[](lua_State *l, panima::Channel &channel, uint32_t startIndex, uint32_t count) {
		  auto &t = channel.GetTimesArray();
		  auto &v = channel.GetValueArray();

		  ::udm::Array::Range r0 {0 /* src */, 0 /* dst */, startIndex};
		  ::udm::Array::Range r1 {startIndex /* src */, startIndex + count /* dst */, t.GetSize() - startIndex};
		  t.Resize(t.GetSize() + count, r0, r1, false);
		  v.Resize(v.GetSize() + count, r0, r1, false);
	  });
	cdChannel.def("FindIndexRangeInTimeRange", &find_index_range_in_time_range);
	cdChannel.def("FindIndexRangeInTimeRange", &find_index_range_in_time_range, luabind::default_parameter_policy<4, false> {});
	cdChannel.def(
	  "GetDataInRange", +[](lua_State *l, panima::Channel &channel, float tStart, float tEnd) -> Lua::mult<Lua::tb<float>, Lua::tb<void>> {
		  return ::udm::visit_ng(channel.GetValueType(), [l, &channel, tStart, tEnd](auto tag) {
			  using T = typename decltype(tag)::type;
			  using TValue = std::conditional_t<std::is_same_v<T, bool>, uint8_t, T>;
			  std::vector<float> times;
			  std::vector<TValue> values;
			  channel.GetDataInRange<TValue>(tStart, tEnd, times, values);
			  return luabind::object {l, std::pair<std::vector<float>, std::vector<TValue>> {std::move(times), std::move(values)}};
		  });
	  });
	cdChannel.def(
	  "GetTimesInRange", +[](lua_State *l, panima::Channel &channel, float tStart, float tEnd) -> std::vector<float> {
		  std::vector<float> times;
		  channel.GetTimesInRange(tStart, tEnd, times);
		  return times;
	  });
	cdChannel.def(
	  "SortValues", +[](lua_State *l, panima::Channel &channel) {
		  auto n = channel.GetTimeCount();
		  std::vector<size_t> order;
		  order.resize(n);
		  for(auto i = decltype(n) {0u}; i < n; ++i)
			  order[i] = i;
		  auto &timesArray = channel.GetTimesArray();
		  auto *times = timesArray.GetValuePtr<float>(0);
		  std::sort(order.begin(), order.end(), [times](size_t a, size_t b) { return times[a] < times[b]; });
		  reorder<float>(timesArray, order);
		  auto &valueArray = channel.GetValueArray();
		  ::udm::visit_ng(valueArray.GetValueType(), [&valueArray, &order](auto tag) {
			  using T = typename decltype(tag)::type;
			  reorder<T>(valueArray, order);
		  });
		  channel.Update();
	  });
	animMod[cdChannel];

	Lua::RegisterLibraryValue(lua.GetState(), "panima", "VALUE_EPSILON", panima::Channel::VALUE_EPSILON);
	Lua::RegisterLibraryValue(lua.GetState(), "panima", "TIME_EPSILON", panima::Channel::TIME_EPSILON);

	pragma::lua::define_custom_constructor<panima::Channel, []() -> std::shared_ptr<panima::Channel> { return std::make_shared<panima::Channel>(); }>(lua.GetState());
	pragma::lua::define_custom_constructor<panima::Channel, [](panima::Channel &channel) -> std::shared_ptr<panima::Channel> { return std::make_shared<panima::Channel>(channel); }, panima::Channel &>(lua.GetState());
	pragma::lua::define_custom_constructor<panima::Channel, [](::udm::LinkedPropertyWrapper &times, ::udm::LinkedPropertyWrapper &values) -> std::shared_ptr<panima::Channel> { return std::make_shared<panima::Channel>(times.ClaimOwnership(), values.ClaimOwnership()); },
	  ::udm::LinkedPropertyWrapper &, ::udm::LinkedPropertyWrapper &>(lua.GetState());

	auto cdSet = luabind::class_<panima::AnimationSet>("Set");
	cdSet.def(luabind::tostring(luabind::self));
	cdSet.def(luabind::const_self == luabind::const_self);
	cdSet.scope[luabind::def("create", &panima::AnimationSet::Create)];
	cdSet.def("Clear", &panima::AnimationSet::Clear);
	cdSet.def("AddAnimation", &panima::AnimationSet::AddAnimation);
	cdSet.def("RemoveAnimation", static_cast<void (panima::AnimationSet::*)(const std::string_view &)>(&panima::AnimationSet::RemoveAnimation));
	cdSet.def("RemoveAnimation", static_cast<void (panima::AnimationSet::*)(const panima::Animation &)>(&panima::AnimationSet::RemoveAnimation));
	cdSet.def("RemoveAnimation", static_cast<void (panima::AnimationSet::*)(panima::AnimationId)>(&panima::AnimationSet::RemoveAnimation));
	cdSet.def("LookupAnimation", &panima::AnimationSet::LookupAnimation);
	cdSet.def("GetAnimation", static_cast<panima::Animation *(panima::AnimationSet::*)(panima::AnimationId)>(&panima::AnimationSet::GetAnimation), luabind::shared_from_this_policy<0> {});
	cdSet.def("GetAnimations", static_cast<std::vector<std::shared_ptr<panima::Animation>> &(panima::AnimationSet::*)()>(&panima::AnimationSet::GetAnimations));
	cdSet.def("FindAnimation", static_cast<panima::Animation *(panima::AnimationSet::*)(const std::string_view &)>(&panima::AnimationSet::FindAnimation), luabind::shared_from_this_policy<0> {});
	cdSet.def("Reserve", &panima::AnimationSet::Reserve);
	cdSet.def("GetSize", &panima::AnimationSet::GetSize);
	animMod[cdSet];

	auto cdPlayer = luabind::class_<panima::Player>("Player");
	cdPlayer.def(luabind::tostring(luabind::self));
	cdPlayer.def(luabind::const_self == luabind::const_self);
	cdPlayer.scope[luabind::def("create", static_cast<std::shared_ptr<panima::Player> (*)()>(&panima::Player::Create))];
	cdPlayer.def("Advance", &panima::Player::Advance);
	cdPlayer.def("Advance", &panima::Player::Advance, luabind::default_parameter_policy<3, false> {});
	cdPlayer.def("GetDuration", &panima::Player::GetDuration);
	cdPlayer.def("GetRemainingDuration", &panima::Player::GetRemainingAnimationDuration);
	cdPlayer.def("GetCurrentTimeFraction", &panima::Player::GetCurrentTimeFraction);
	cdPlayer.def("GetCurrentTime", &panima::Player::GetCurrentTime);
	cdPlayer.def("GetPlaybackRate", &panima::Player::GetPlaybackRate);
	cdPlayer.def("SetPlaybackRate", &panima::Player::SetPlaybackRate);
	cdPlayer.def("SetAnimationDirty", &panima::Player::SetAnimationDirty);
	cdPlayer.def("SetCurrentTime", &panima::Player::SetCurrentTime);
	cdPlayer.def("SetCurrentTime", &panima::Player::SetCurrentTime, luabind::default_parameter_policy<3, false> {});
	cdPlayer.def("Reset", &panima::Player::Reset);
	cdPlayer.def("GetCurrentSlice", static_cast<panima::Slice *(*)(lua_State *, panima::Player &)>([](lua_State *l, panima::Player &player) { return &player.GetCurrentSlice(); }));
	cdPlayer.def(
	  "GetAnimation", +[](lua_State *l, panima::Player &player) -> std::shared_ptr<const panima::Animation> {
		  auto *anim = player.GetAnimation();
		  if(!anim)
			  return nullptr;
		  return anim->shared_from_this();
	  });
	cdPlayer.def("SetLooping", &panima::Player::SetLooping);
	cdPlayer.def("IsLooping", &panima::Player::IsLooping);
	cdPlayer.def("SetAnimation", &panima::Player::SetAnimation);
	animMod[cdPlayer];

	auto cdManager = luabind::class_<panima::AnimationManager>("Manager");
	cdManager.def(luabind::tostring(luabind::self));
	cdManager.def(luabind::const_self == luabind::const_self);
	cdManager.scope[luabind::def("create", static_cast<std::shared_ptr<panima::AnimationManager> (*)()>(&panima::AnimationManager::Create))];
	cdManager.def(
	  "GetPreviousSlice", +[](lua_State *l, panima::AnimationManager &manager) { return &manager.GetPreviousSlice(); });
	cdManager.def("GetCurrentAnimationId", &panima::AnimationManager::GetCurrentAnimationId);
	cdManager.def("AddAnimationSet", &panima::AnimationManager::AddAnimationSet);
	cdManager.def(
	  "GetCurrentAnimation", +[](lua_State *l, panima::AnimationManager &manager) -> luabind::optional<std::shared_ptr<panima::Animation>> {
		  auto *anim = manager.GetCurrentAnimation();
		  return anim ? opt<std::shared_ptr<panima::Animation>> {l, anim->shared_from_this()} : nil;
	  });
	cdManager.def("GetPlayer", static_cast<panima::Player &(panima::AnimationManager::*)()>(&panima::AnimationManager::GetPlayer), luabind::shared_from_this_policy<0> {});
	cdManager.def("StopAnimation", static_cast<void (panima::AnimationManager::*)()>(&panima::AnimationManager::StopAnimation));
	cdManager.def("PlayAnimation", static_cast<void (panima::AnimationManager::*)(const std::string &, panima::AnimationId, panima::PlaybackFlags)>(&panima::AnimationManager::PlayAnimation));
	cdManager.def("PlayAnimation", static_cast<void (panima::AnimationManager::*)(const std::string &, panima::AnimationId, panima::PlaybackFlags)>(&panima::AnimationManager::PlayAnimation), luabind::default_parameter_policy<4, panima::PlaybackFlags::Default> {});
	cdManager.def("PlayAnimation", static_cast<void (panima::AnimationManager::*)(const std::string &, const std::string &, panima::PlaybackFlags)>(&panima::AnimationManager::PlayAnimation));
	cdManager.def("PlayAnimation", static_cast<void (panima::AnimationManager::*)(const std::string &, const std::string &, panima::PlaybackFlags)>(&panima::AnimationManager::PlayAnimation), luabind::default_parameter_policy<4, panima::PlaybackFlags::Default> {});
	cdManager.def("PlayAnimation", static_cast<void (panima::AnimationManager::*)(const std::string &, panima::PlaybackFlags)>(&panima::AnimationManager::PlayAnimation));
	cdManager.def("PlayAnimation", static_cast<void (panima::AnimationManager::*)(const std::string &, panima::PlaybackFlags)>(&panima::AnimationManager::PlayAnimation), luabind::default_parameter_policy<4, panima::PlaybackFlags::Default> {});
	animMod[cdManager];

	auto cdSlice = luabind::class_<panima::Slice>("Slice");
	cdSlice.def(luabind::tostring(luabind::self));
	cdSlice.def(
	  "GetChannelValueCount", +[](lua_State *l, panima::Slice &slice) -> uint32_t { return slice.channelValues.size(); });
	cdSlice.def(
	  "GetChannelProperty", +[](lua_State *l, panima::Slice &slice, uint32_t idx) -> luabind::optional<::udm::PProperty> {
		  if(idx >= slice.channelValues.size())
			  return nil;
		  return {l, slice.channelValues[idx]};
	  });
	animMod[cdSlice];

	auto cdAnim2 = luabind::class_<panima::Animation>("Animation");
	cdAnim2.def(luabind::tostring(luabind::self));
	cdAnim2.def(luabind::const_self == luabind::const_self);
	cdAnim2.scope[luabind::def(
	  "create", +[](lua_State *l) { return std::make_shared<panima::Animation>(); })];
	cdAnim2.scope[luabind::def(
	  "load", +[](lua_State *l, ::udm::LinkedPropertyWrapper &prop) -> Lua::var<bool, std::shared_ptr<panima::Animation>> {
		  auto anim = std::make_shared<panima::Animation>();
		  if(anim->Load(prop) == false)
			  return luabind::object {l, false};
		  return luabind::object {l, anim};
	  })];
	cdAnim2.def("Merge", &panima::Animation::Merge);
	cdAnim2.def("GetChannelCount", &panima::Animation::GetChannelCount);
	cdAnim2.def("GetAnimationSpeedFactor", &panima::Animation::GetAnimationSpeedFactor);
	cdAnim2.def("SetAnimationSpeedFactor", &panima::Animation::SetAnimationSpeedFactor);
	cdAnim2.def("GetDuration", &panima::Animation::GetDuration);
	cdAnim2.def("SetDuration", &panima::Animation::SetDuration);
	cdAnim2.def(
	  "UpdateDuration", +[](lua_State *l, panima::Animation &anim) -> float {
		  auto duration = 0.f;
		  for(auto &channel : anim.GetChannels()) {
			  auto time = channel->GetTime(channel->GetTimeCount() - 1);
			  if(!time.has_value())
				  continue;
			  duration = umath::max(duration, *time);
		  }
		  anim.SetDuration(duration);
		  return duration;
	  });
	cdAnim2.def("RemoveChannel", static_cast<void (panima::Animation::*)(std::string)>(&panima::Animation::RemoveChannel));
	cdAnim2.def("RemoveChannel", static_cast<void (panima::Animation::*)(const panima::Channel &)>(&panima::Animation::RemoveChannel));
	cdAnim2.def("AddChannel", static_cast<void (panima::Animation::*)(panima::Channel &)>(&panima::Animation::AddChannel));
	cdAnim2.def(
	  "AddChannel", +[](lua_State *l, panima::Animation &anim, const std::string &path, ::udm::Type valueType) -> opt<std::shared_ptr<panima::Channel>> {
		  auto *channel = anim.AddChannel(path, valueType);
		  if(!channel)
			  return nil;
		  return {l, channel->shared_from_this()};
	  });
	cdAnim2.def(
	  "GetChannels", +[](lua_State *l, panima::Animation &anim) -> luabind::tableT<panima::Channel> { return Lua::vector_to_table<std::shared_ptr<panima::Channel>>(l, anim.GetChannels()); });
	cdAnim2.def(
	  "FindChannel", +[](lua_State *l, panima::Animation &anim, std::string path) -> opt<std::shared_ptr<panima::Channel>> {
		  auto *channel = anim.FindChannel(std::move(path));
		  if(!channel)
			  return nil;
		  return {l, channel->shared_from_this()};
	  });
	cdAnim2.def("Save", &panima::Animation::Save);
	cdAnim2.scope[luabind::def(
	  "Load", +[](lua_State *l, ::udm::LinkedPropertyWrapper &assetData) -> std::shared_ptr<panima::Animation> {
		  auto anim = std::make_shared<panima::Animation>();
		  if(anim->Load(assetData) == false)
			  return nullptr;
		  return anim;
	  })];
	animMod[cdAnim2];
}
