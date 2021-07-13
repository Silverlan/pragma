/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/model/animation/animation_channel.hpp"

static constexpr auto VALUE_EPSILON = 0.001f;
#pragma optimize("",off)
pragma::animation::AnimationChannel::AnimationChannel()
	: m_times{::udm::Property::Create(udm::Type::ArrayLz4)},m_values{::udm::Property::Create(udm::Type::ArrayLz4)}
{
	GetTimesArray().SetValueType(udm::Type::Float);
}
bool pragma::animation::AnimationChannel::Save(udm::LinkedPropertyWrapper &prop) const
{
	prop["interpolation"] = interpolation;
	prop["targetPath"] = targetPath.GetString();

	prop["times"] = m_times;
	prop["values"] = m_values;
	return true;
}
bool pragma::animation::AnimationChannel::Load(udm::LinkedPropertyWrapper &prop)
{
	prop["interpolation"](interpolation);
	std::string targetPath;
	prop["targetPath"](targetPath);
	this->targetPath = std::move(targetPath);

	prop["times"](m_times);
	prop["values"](m_values);
	return true;
}
uint32_t pragma::animation::AnimationChannel::GetSize() const {return GetTimesArray().GetSize();}
void pragma::animation::AnimationChannel::Resize(uint32_t numValues)
{
	m_times->GetValue<udm::Array>().Resize(numValues);
	m_values->GetValue<udm::Array>().Resize(numValues);
}
uint32_t pragma::animation::AnimationChannel::AddValue(float t,const void *value)
{
	float interpFactor;
	auto indices = FindInterpolationIndices(t,interpFactor);
	if(indices.first == std::numeric_limits<decltype(indices.first)>::max())
	{
		auto size = GetSize() +1;
		Resize(size);
		GetTimesArray()[size -1] = t;
		GetValueArray()[size -1] = value;
		return 0;
	}
	if(umath::abs(t -*GetTime(indices.first)) < VALUE_EPSILON)
	{
		// Replace value at first index with new value
		GetTimesArray()[indices.first] = t;
		GetValueArray()[indices.first] = value;
		return 0;
	}
	if(umath::abs(t -*GetTime(indices.second)) < VALUE_EPSILON)
	{
		// Replace value at second index with new value
		GetTimesArray()[indices.second] = t;
		GetValueArray()[indices.second] = value;
		return 0;
	}
	// Insert new value between the two indices
	
	//	udm::PProperty m_times = nullptr;
	//	udm::PProperty m_values = nullptr;


	// This is a stub
	throw std::runtime_error{"Not yet implemented"};
	return 0;
	//float interpFactor;
	//auto indices = FindInterpolationIndices(t,interpFactor);
	//values.insert(values.begin() +indices.first);
	//values.
}
udm::Array &pragma::animation::AnimationChannel::GetTimesArray() {return m_times->GetValue<udm::Array>();}
udm::Array &pragma::animation::AnimationChannel::GetValueArray() {return m_values->GetValue<udm::Array>();}
udm::Type pragma::animation::AnimationChannel::GetValueType() const {return GetValueArray().GetValueType();}
void pragma::animation::AnimationChannel::SetValueType(udm::Type type) {GetValueArray().SetValueType(type);}
std::pair<uint32_t,uint32_t> pragma::animation::AnimationChannel::FindInterpolationIndices(float t,float &interpFactor,uint32_t pivotIndex,uint32_t recursionDepth) const
{
	constexpr uint32_t MAX_RECURSION_DEPTH = 2;
	auto &times = GetTimesArray();
	if(pivotIndex >= times.GetSize() || times.GetSize() < 2 || recursionDepth == MAX_RECURSION_DEPTH)
		return FindInterpolationIndices(t,interpFactor);
	// We'll use the pivot index as the starting point of our search and check out the times immediately surrounding it.
	// If we have a match, we can return immediately. If not, we'll slightly broaden the search until we've reached the max recursion depth or found a match.
	// If we hit the max recusion depth, we'll just do a regular binary search instead.
	auto tPivot = times.GetValue<float>(pivotIndex);
	if(t >= tPivot)
	{
		if(pivotIndex == times.GetSize() -1)
		{
			interpFactor = 0.f;
			return {static_cast<uint32_t>(GetValueArray().GetSize() -1),static_cast<uint32_t>(GetValueArray().GetSize() -1)};
		}
		auto tPivotNext = times.GetValue<float>(pivotIndex +1);
		if(t < tPivotNext)
		{
			// Most common case
			interpFactor = (t -tPivot) /(tPivotNext -tPivot);
			return {pivotIndex,pivotIndex +1};
		}
		return FindInterpolationIndices(t,interpFactor,pivotIndex +1,recursionDepth +1);
	}
	if(pivotIndex == 0)
	{
		interpFactor = 0.f;
		return {0u,0u};
	}
	return FindInterpolationIndices(t,interpFactor,pivotIndex -1,recursionDepth +1);
}
std::pair<uint32_t,uint32_t> pragma::animation::AnimationChannel::FindInterpolationIndices(float t,float &interpFactor,uint32_t pivotIndex) const
{
	return FindInterpolationIndices(t,interpFactor,pivotIndex,0u);
}

// TODO: Move this to UDM library
class ArrayFloatIterator
{
public:
	using iterator_category = std::forward_iterator_tag;
	using value_type = float;
	using difference_type = std::ptrdiff_t;
	using pointer = float*;
	using reference = float&;
	
	ArrayFloatIterator(float *data)
		: m_data{data}
	{}
	ArrayFloatIterator &operator++()
	{
		m_data++;
		return *this;
	}
	ArrayFloatIterator operator++(int)
	{
		++m_data;
		return *this;
	}
	ArrayFloatIterator operator+(uint32_t n)
	{
		m_data += n;
		return *this;
	}
	int32_t operator-(const ArrayFloatIterator &other) const {return m_data -other.m_data;}
	ArrayFloatIterator operator-(int32_t idx) const {return ArrayFloatIterator{m_data -idx};}
	reference operator*() {return *m_data;}
	const reference operator*() const {return const_cast<ArrayFloatIterator*>(this)->operator*();}
	pointer operator->() {return m_data;}
	const pointer operator->() const {return const_cast<ArrayFloatIterator*>(this)->operator->();}
	bool operator==(const ArrayFloatIterator &other) const {return m_data == other.m_data;}
	bool operator!=(const ArrayFloatIterator &other) const {return !operator==(other);}
private:
	float *m_data = nullptr;
};
static ArrayFloatIterator begin(const udm::Array &a)
{
	return ArrayFloatIterator{static_cast<float*>(&const_cast<udm::Array&>(a).GetValue<float>(0))};
}
static ArrayFloatIterator end(const udm::Array &a)
{
	return ArrayFloatIterator{static_cast<float*>(&const_cast<udm::Array&>(a).GetValue<float>(0)) +a.GetSize()};
}
std::pair<uint32_t,uint32_t> pragma::animation::AnimationChannel::FindInterpolationIndices(float t,float &interpFactor) const
{
	auto &times = GetTimesArray();
	if(times.GetSize() == 0)
	{
		interpFactor = 0.f;
		return {std::numeric_limits<uint32_t>::max(),std::numeric_limits<uint32_t>::max()};
	}
	// Binary search
	auto it = std::upper_bound(begin(times),end(times),t);
	if(it == end(times))
	{
		interpFactor = 0.f;
		return {static_cast<uint32_t>(times.GetSize() -1),static_cast<uint32_t>(times.GetSize() -1)};
	}
	if(it == begin(times))
	{
		interpFactor = 0.f;
		return {0u,0u};
	}
	auto itPrev = it -1;
	interpFactor = (t -*itPrev) /(*it -*itPrev);
	return {static_cast<uint32_t>(itPrev -begin(times)),static_cast<uint32_t>(it -begin(times))};
}

uint32_t pragma::animation::AnimationChannel::GetTimeCount() const {return GetTimesArray().GetSize();}
uint32_t pragma::animation::AnimationChannel::GetValueCount() const {return GetValueArray().GetSize();}
std::optional<float> pragma::animation::AnimationChannel::GetTime(uint32_t idx) const
{
	auto &times = GetTimesArray();
	auto n = times.GetSize();
	return (idx < n) ? times.GetValue<float>(idx) : std::optional<float>{};
}

std::ostream &operator<<(std::ostream &out,const pragma::animation::AnimationChannel &o)
{
	out<<"AnimationChannel";
	out<<"[Path:"<<o.targetPath<<"]";
	out<<"[Interp:"<<magic_enum::enum_name(o.interpolation)<<"]";
	out<<"[Values:"<<o.GetValueArray().GetSize()<<"]";
	
	std::pair<float,float> timeRange {0.f,0.f};
	auto n = o.GetTimeCount();
	if(n > 0)
		out<<"[TimeRange:"<<*o.GetTime(0)<<","<<*o.GetTime(n -1)<<"]";
	return out;
}
#pragma optimize("",on)
