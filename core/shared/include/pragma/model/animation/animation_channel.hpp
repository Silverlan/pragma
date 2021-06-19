/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __ANIMATION_CHANNEL_HPP__
#define __ANIMATION_CHANNEL_HPP__

#include "pragma/networkdefinitions.h"
#include <sharedutils/util_path.hpp>
#include <udm.hpp>

namespace pragma::animation
{
	constexpr std::string_view ANIMATION_CHANNEL_PATH_POSITION = "position";
	constexpr std::string_view ANIMATION_CHANNEL_PATH_ROTATION = "rotation";
	constexpr std::string_view ANIMATION_CHANNEL_PATH_SCALE = "scale";

	constexpr auto ANIMATION_CHANNEL_TYPE_POSITION = udm::Type::Vector3;
	constexpr auto ANIMATION_CHANNEL_TYPE_ROTATION = udm::Type::Quaternion;
	constexpr auto ANIMATION_CHANNEL_TYPE_SCALE = udm::Type::Vector3;
	enum class AnimationChannelInterpolation : uint8_t
	{
		Linear = 0,
		Step,
		CubicSpline
	};
	struct DLLNETWORK AnimationChannel
		: public std::enable_shared_from_this<AnimationChannel>
	{
		template<typename T>
			class Iterator
		{
		public:
			using iterator_category = std::forward_iterator_tag;
			using value_type = T&;
			using difference_type = std::ptrdiff_t;
			using pointer = T*;
			using reference = T&;
	
			Iterator(std::vector<uint8_t> &values,bool end);
			Iterator &operator++();
			Iterator operator++(int);
			reference operator*();
			pointer operator->();
			bool operator==(const Iterator<T> &other) const;
			bool operator!=(const Iterator<T> &other) const;
		private:
			std::vector<uint8_t>::iterator m_it;
		};

		template<typename T>
			struct IteratorWrapper
		{
			IteratorWrapper(std::vector<uint8_t> &values);
			IteratorWrapper()=default;
			Iterator<T> begin();
			Iterator<T> end();
		private:
			std::vector<uint8_t> &m_values;
		};

		AnimationChannel();
		AnimationChannel(const AnimationChannel &other)=default;
		AnimationChannel(AnimationChannel &&other)=default;
		AnimationChannel &operator=(const AnimationChannel&)=default;
		AnimationChannel &operator=(AnimationChannel&&)=default;
		AnimationChannelInterpolation interpolation = AnimationChannelInterpolation::Linear;
		util::Path targetPath;

		template<typename T>
			uint32_t AddValue(float t,const T &value);

		udm::Array &GetTimesArray();
		const udm::Array &GetTimesArray() const {return const_cast<AnimationChannel*>(this)->GetTimesArray();}
		udm::Array &GetValueArray();
		const udm::Array &GetValueArray() const {return const_cast<AnimationChannel*>(this)->GetValueArray();}
		udm::Type GetValueType() const;
		void SetValueType(udm::Type type);

		uint32_t GetTimeCount() const;
		uint32_t GetValueCount() const;
		std::optional<float> GetTime(uint32_t idx) const;

		bool Save(udm::LinkedPropertyWrapper &prop) const;
		bool Load(udm::LinkedPropertyWrapper &prop);
		
		std::pair<uint32_t,uint32_t> FindInterpolationIndices(float t,float &outInterpFactor,uint32_t pivotIndex) const;
		std::pair<uint32_t,uint32_t> FindInterpolationIndices(float t,float &outInterpFactor) const;
		template<typename T>
			bool IsValueType() const {return udm::type_to_enum<T>() == GetValueType();}
		template<typename T>
			IteratorWrapper<T> It();
		template<typename T>
			IteratorWrapper<T> It() const {return const_cast<AnimationChannel*>(this)->It();}
		template<typename T>
			T &GetValue(uint32_t idx);
		template<typename T>
			const T &GetValue(uint32_t idx) const {return const_cast<AnimationChannel*>(this)->GetValue<T>(idx);}
		template<typename T>
			auto GetInterpolationFunction() const;
		template<typename T>
			T GetInterpolatedValue(float t,uint32_t &inOutPivotTimeIndex) const;
		template<typename T>
			T GetInterpolatedValue(float t) const;
	private:
		uint32_t AddValue(float t,const void *value);
		std::pair<uint32_t,uint32_t> FindInterpolationIndices(float t,float &outInterpFactor,uint32_t pivotIndex,uint32_t recursionDepth) const;
		udm::PProperty m_times = nullptr;
		udm::PProperty m_values = nullptr;
	};
};

DLLNETWORK std::ostream &operator<<(std::ostream &out,const pragma::animation::AnimationChannel &o);

template<typename T>
	uint32_t pragma::animation::AnimationChannel::AddValue(float t,const T &value)
{
	if(udm::type_to_enum<T>() != GetValueType())
		throw std::invalid_argument{"Value type mismatch!"};
	return AddValue(t,&value);
}

/////////////////////

template<typename T>
	pragma::animation::AnimationChannel::Iterator<T>::Iterator(std::vector<uint8_t> &values,bool end)
		: m_it{end ? values.end() : values.begin()}
{}
template<typename T>
	pragma::animation::AnimationChannel::Iterator<T> &pragma::animation::AnimationChannel::Iterator<T>::operator++()
{
	m_it += udm::size_of_base_type(udm::type_to_enum<T>());
	return *this;
}
template<typename T>
	pragma::animation::AnimationChannel::Iterator<T> pragma::animation::AnimationChannel::Iterator<T>::operator++(int)
{
	auto it = *this;
	it.operator++();
	return it;
}
template<typename T>
	typename pragma::animation::AnimationChannel::Iterator<T>::reference pragma::animation::AnimationChannel::Iterator<T>::operator*()
{
	return *operator->();
}
template<typename T>
	typename pragma::animation::AnimationChannel::Iterator<T>::pointer pragma::animation::AnimationChannel::Iterator<T>::operator->()
{
	return reinterpret_cast<T*>(&*m_it);
}
template<typename T>
	bool pragma::animation::AnimationChannel::Iterator<T>::operator==(const Iterator<T> &other) const {return m_it == other.m_it;}
template<typename T>
	bool pragma::animation::AnimationChannel::Iterator<T>::operator!=(const Iterator<T> &other) const {return !operator==(other);}

/////////////////////

template<typename T>
	pragma::animation::AnimationChannel::IteratorWrapper<T>::IteratorWrapper(std::vector<uint8_t> &values)
		: m_values{values}
{}
template<typename T>
	pragma::animation::AnimationChannel::Iterator<T> begin()
{
	// TODO: This is a stub
	throw std::runtime_error{"Not yet implemented!"};
	return {};
	//return Iterator<T>{m_values,false};
}
template<typename T>
	pragma::animation::AnimationChannel::Iterator<T> end()
{
	// TODO: This is a stub
	throw std::runtime_error{"Not yet implemented!"};
	return {};
	//return Iterator<T>{m_values,true};
}

template<typename T>
	pragma::animation::AnimationChannel::IteratorWrapper<T> pragma::animation::AnimationChannel::It()
{
	// TODO: This is a stub
	throw std::runtime_error{"Not yet implemented!"};
	//static std::vector<uint8_t> empty;
	//return IsValueType<T>() ? IteratorWrapper<T>{values} : IteratorWrapper<T>{empty};
	return {};
}

/////////////////////

template<typename T>
	T &pragma::animation::AnimationChannel::GetValue(uint32_t idx)
	{
		// TODO: This is a stub
		throw std::runtime_error{"Not yet implemented!"};
		return *static_cast<T*>(nullptr);
		//return *(reinterpret_cast<T*>(values.data()) +idx);
	}

template<typename T>
	auto pragma::animation::AnimationChannel::GetInterpolationFunction() const
{
	// TODO: This is a stub
	throw std::runtime_error{"Not yet implemented!"};
	return T{};
	/*constexpr auto type = udm::type_to_enum<T>();
	if constexpr(std::is_same_v<T,Vector3>)
		return &uvec::lerp;
	else if constexpr(std::is_same_v<T,Quat>)
		return &uquat::lerp; // TODO: Maybe use slerp? Test performance!
	else
		return [](const T &v0,const T &v1,float f) -> T {return (v0 +f *(v1 -v0));};*/
}

template<typename T>
	T pragma::animation::AnimationChannel::GetInterpolatedValue(float t,uint32_t &inOutPivotTimeIndex) const
{
	// TODO: This is a stub
	throw std::runtime_error{"Not yet implemented!"};
	return T{};
	/*if(udm::type_to_enum<T>() != GetValueType() || times.empty())
		return {};
	float factor;
	auto indices = FindInterpolationIndices(t,factor,inOutPivotTimeIndex);
	inOutPivotTimeIndex = indices.first;
	return GetInterpolationFunction<T>()(GetValue<T>(indices.first),GetValue<T>(indices.second),factor);*/
}

template<typename T>
	T pragma::animation::AnimationChannel::GetInterpolatedValue(float t) const
{
	// TODO: This is a stub
	throw std::runtime_error{"Not yet implemented!"};
	return T{};
	/*if(udm::type_to_enum<T>() != GetValueType() || times.empty())
		return {};
	float factor;
	auto indices = FindInterpolationIndices(t,factor);
	return GetInterpolationFunction<T>()(GetValue<T>(indices.first),GetValue<T>(indices.second),factor);*/
}

#endif
