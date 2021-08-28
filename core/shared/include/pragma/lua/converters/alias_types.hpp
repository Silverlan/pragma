/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LUA_ALIAS_TYPES_HPP__
#define __LUA_ALIAS_TYPES_HPP__

#include "pragma/lua/converters/alias_converter.hpp"
#include <sharedutils/util_path.hpp>
#include <mathutil/uvec.h>

namespace luabind
{
	// Vector
	namespace detail
	{
		// Vector3
		template<>
			struct AliasTypeConverter<Vector3,Vector2>
		{
			static void convert(const Vector2 &srcValue,Vector3 &outValue);
		};
		template<>
			struct AliasTypeConverter<Vector3i,Vector2>
		{
			static void convert(const Vector2 &srcValue,Vector3i &outValue);
		};
		template<>
			struct AliasTypeConverter<Vector3,Vector2i>
		{
			static void convert(const Vector2i &srcValue,Vector3 &outValue);
		};
		template<>
			struct AliasTypeConverter<Vector3i,Vector2i>
		{
			static void convert(const Vector2i &srcValue,Vector3i &outValue);
		};

		// Vector4
		template<>
			struct AliasTypeConverter<Vector4,Vector2>
		{
			static void convert(const Vector2 &srcValue,Vector4 &outValue);
		};
		template<>
			struct AliasTypeConverter<Vector4i,Vector2>
		{
			static void convert(const Vector2 &srcValue,Vector4i &outValue);
		};
		template<>
			struct AliasTypeConverter<Vector4,Vector2i>
		{
			static void convert(const Vector2i &srcValue,Vector4 &outValue);
		};
		template<>
			struct AliasTypeConverter<Vector4i,Vector2i>
		{
			static void convert(const Vector2i &srcValue,Vector4i &outValue);
		};
		template<>
			struct AliasTypeConverter<Vector4,Vector3>
		{
			static void convert(const Vector3 &srcValue,Vector4 &outValue);
		};
		template<>
			struct AliasTypeConverter<Vector4i,Vector3>
		{
			static void convert(const Vector3 &srcValue,Vector4i &outValue);
		};
		template<>
			struct AliasTypeConverter<Vector4,Vector3i>
		{
			static void convert(const Vector3i &srcValue,Vector4 &outValue);
		};
		template<>
			struct AliasTypeConverter<Vector4i,Vector3i>
		{
			static void convert(const Vector3i &srcValue,Vector4i &outValue);
		};
	};
	template<typename T>
		concept is_vector_alias_type = is_one_of_alias_candidates<T,Vector2,Vector2i,Vector3,Vector3i,Vector4,Vector4i>; // Is type a value or const reference of any of the types in the parameter list?
	template<typename TVector>
		using vector_alias_converter = alias_converter<TVector,Vector2,Vector2i,Vector3,Vector3i,Vector4,Vector4i>;

	template <typename T> requires(is_vector_alias_type<T>)
	struct default_converter<T>
		: vector_alias_converter<T>
	{};

	// EulerAngles / Quat
	namespace detail
	{
		template<>
			struct AliasTypeConverter<EulerAngles,Quat>
		{
			static void convert(const Quat &srcValue,EulerAngles &outValue);
		};
		template<>
			struct AliasTypeConverter<Quat,EulerAngles>
		{
			static void convert(const EulerAngles &srcValue,Quat &outValue);
		};
	};
	template <typename T> requires(is_one_of_alias_candidates<T,EulerAngles,Quat>)
	struct default_converter<T>
		: alias_converter<T,EulerAngles,Quat>
	{};

	// Path / String
	namespace detail
	{
		template<>
			struct AliasTypeConverter<std::string,util::Path>
		{
			static void convert(const util::Path &srcValue,std::string &outValue);
		};
		template<>
			struct AliasTypeConverter<util::Path,std::string>
		{
			static void convert(const std::string &srcValue,util::Path &outValue);
		};
	};
	template <typename T> requires(is_one_of_alias_candidates<T,std::string,util::Path>)
	struct default_converter<T>
		: alias_converter<T,std::string,util::Path>
	{};
};

#endif
