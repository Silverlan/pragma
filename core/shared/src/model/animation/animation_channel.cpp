/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/model/animation/animation_channel.hpp"
#pragma optimize("",off)
bool pragma::animation::AnimationChannel::Save(udm::LinkedPropertyWrapper &prop) const
{
	prop["interpolation"] = interpolation;
	prop["targetPath"] = targetPath.GetString();

	prop["times"] = times;
	auto udmValues = prop.AddArray("values",values.size(),valueType);
	memcpy(udmValues.GetValue<udm::Array>().GetValuePtr(0),values.data(),util::size_of_container(values));
	return true;
}
bool pragma::animation::AnimationChannel::Load(udm::LinkedPropertyWrapper &prop)
{
	prop["interpolation"](interpolation);
	std::string targetPath;
	prop["targetPath"](targetPath);
	this->targetPath = std::move(targetPath);

	prop["times"](times);
	auto udmValues = prop["values"];
	auto *aValues = udmValues.GetValuePtr<udm::Array>();
	if(aValues)
	{
		valueType = aValues->GetValueType();
		uint64_t reqSize = 0;
		udmValues.GetBlobData(nullptr,0,&reqSize);
		values.resize(reqSize);
		udmValues.GetBlobData(values.data(),values.size());
	}
	else
	{
		valueType = udm::Type::Float;
		values.resize(sizeof(float) *times.size());
		for(auto i=decltype(times.size()){0u};i<times.size();++i)
			reinterpret_cast<float*>(values.data())[i] = 0.f;
	}
	return true;
}
std::pair<uint32_t,uint32_t> pragma::animation::AnimationChannel::FindInterpolationIndices(float t,float &interpFactor,uint32_t pivotIndex,uint32_t recursionDepth) const
{
	constexpr uint32_t MAX_RECURSION_DEPTH = 2;
	if(pivotIndex >= times.size() || times.size() < 2 || recursionDepth == MAX_RECURSION_DEPTH)
		return FindInterpolationIndices(t,interpFactor);
	// We'll use the pivot index as the starting point of our search and check out the times immediately surrounding it.
	// If we have a match, we can return immediately. If not, we'll slightly broaden the search until we've reached the max recursion depth or found a match.
	// If we hit the max recusion depth, we'll just do a regular binary search instead.
	auto tPivot = times[pivotIndex];
	if(t >= tPivot)
	{
		if(pivotIndex == times.size() -1)
		{
			interpFactor = 0.f;
			return {static_cast<uint32_t>(values.size() -1),static_cast<uint32_t>(values.size() -1)};
		}
		if(t < times[pivotIndex +1])
		{
			// Most common case
			interpFactor = (t -times[pivotIndex]) /(times[pivotIndex +1] -times[pivotIndex]);
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
std::pair<uint32_t,uint32_t> pragma::animation::AnimationChannel::FindInterpolationIndices(float t,float &interpFactor) const
{
	if(times.empty())
	{
		interpFactor = 0.f;
		return {std::numeric_limits<uint32_t>::max(),std::numeric_limits<uint32_t>::max()};
	}
	// Binary search
	auto it = std::upper_bound(times.begin(),times.end(),t);
	if(it == times.end())
	{
		interpFactor = 0.f;
		return {static_cast<uint32_t>(times.size() -1),static_cast<uint32_t>(times.size() -1)};
	}
	if(it == times.begin())
	{
		interpFactor = 0.f;
		return {0u,0u};
	}
	auto itPrev = it -1;
	interpFactor = (t -*itPrev) /(*it -*itPrev);
	return {static_cast<uint32_t>(itPrev -times.begin()),static_cast<uint32_t>(it -times.begin())};
}
#pragma optimize("",on)
