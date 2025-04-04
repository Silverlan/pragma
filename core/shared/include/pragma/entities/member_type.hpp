/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __PRAGMA_ENTITY_MEMBER_TYPE_HPP__
#define __PRAGMA_ENTITY_MEMBER_TYPE_HPP__

#include "pragma/networkdefinitions.h"
#include <mathutil/umath.h>
#include <udm.hpp>
#include "pragma/entities/entity_uuid_ref.hpp"

namespace pragma::ents {
	enum class EntityMemberType : uint8_t {
		// Nil = umath::to_integral(udm::Type::Nil),
		String = umath::to_integral(udm::Type::String),
		// Utf8String = umath::to_integral(udm::Type::Utf8String),

		Int8 = umath::to_integral(udm::Type::Int8),
		UInt8 = umath::to_integral(udm::Type::UInt8),
		Int16 = umath::to_integral(udm::Type::Int16),
		UInt16 = umath::to_integral(udm::Type::UInt16),
		Int32 = umath::to_integral(udm::Type::Int32),
		UInt32 = umath::to_integral(udm::Type::UInt32),
		Int64 = umath::to_integral(udm::Type::Int64),
		UInt64 = umath::to_integral(udm::Type::UInt64),

		Float = umath::to_integral(udm::Type::Float),
		Double = umath::to_integral(udm::Type::Double),
		Boolean = umath::to_integral(udm::Type::Boolean),

		Vector2 = umath::to_integral(udm::Type::Vector2),
		Vector3 = umath::to_integral(udm::Type::Vector3),
		Vector4 = umath::to_integral(udm::Type::Vector4),
		Quaternion = umath::to_integral(udm::Type::Quaternion),
		EulerAngles = umath::to_integral(udm::Type::EulerAngles),
		Srgba = umath::to_integral(udm::Type::Srgba),
		HdrColor = umath::to_integral(udm::Type::HdrColor),
		Transform = umath::to_integral(udm::Type::Transform),
		ScaledTransform = umath::to_integral(udm::Type::ScaledTransform),
		Mat4 = umath::to_integral(udm::Type::Mat4),
		Mat3x4 = umath::to_integral(udm::Type::Mat3x4),

		// Blob = umath::to_integral(udm::Type::Blob),
		// BlobLz4 = umath::to_integral(udm::Type::BlobLz4),

		// Array = umath::to_integral(udm::Type::Array),
		// ArrayLz4 = umath::to_integral(udm::Type::ArrayLz4),
		// Reference = umath::to_integral(udm::Type::Reference),
		// Struct = umath::to_integral(udm::Type::Struct),
		Half = umath::to_integral(udm::Type::Half),
		Vector2i = umath::to_integral(udm::Type::Vector2i),
		Vector3i = umath::to_integral(udm::Type::Vector3i),
		Vector4i = umath::to_integral(udm::Type::Vector4i),

		// Managed types
		Entity = umath::to_integral(udm::Type::Count) + 1,
		MultiEntity,
		ComponentProperty,
		Element,

		Count,
		Last = Count - 1,
		Invalid = umath::to_integral(udm::Type::Invalid),
		VersionIndex = 0 // Increment this when adding or removing enums
	};

	using Element = udm::PProperty;

	template<class T>
	struct tag_t {
		using type = T;
	};
	template<class T>
	constexpr tag_t<T> tag = {};
#pragma warning(push)
#pragma warning(disable : 4715)
#ifdef __linux__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
#endif
	constexpr std::variant<tag_t<EntityURef>, tag_t<MultiEntityURef>, tag_t<EntityUComponentMemberRef>, tag_t<Element>> get_managed_member_type_tag(EntityMemberType e)
	{
		switch(e) {
		case EntityMemberType::Entity:
			return tag<EntityURef>;
		case EntityMemberType::MultiEntity:
			return tag<MultiEntityURef>;
		case EntityMemberType::ComponentProperty:
			return tag<EntityUComponentMemberRef>;
		case EntityMemberType::Element:
			return tag<Element>;
		}
	}

#pragma warning(pop)
#ifdef __linux__
#pragma GCC diagnostic pop
#endif
	constexpr bool is_udm_member_type(EntityMemberType type) { return umath::to_integral(type) < umath::to_integral(udm::Type::Count) && type != EntityMemberType::Element; }
	constexpr bool is_managed_member_type_f(EntityMemberType type)
	{
		switch(type) {
		case EntityMemberType::Entity:
		case EntityMemberType::MultiEntity:
		case EntityMemberType::ComponentProperty:
		case EntityMemberType::Element:
			return true;
		}
		return false;
	}
	constexpr udm::Type member_type_to_udm_type(EntityMemberType type) { return (umath::to_integral(type) < umath::to_integral(udm::Type::Count)) ? static_cast<udm::Type>(type) : udm::Type::Invalid; }
	constexpr EntityMemberType udm_type_to_member_type(udm::Type type) { return static_cast<EntityMemberType>(type); }

	template<bool ENABLE_DEFAULT_RETURN = true, typename T>
	constexpr decltype(auto) visit_member(EntityMemberType type, T vs)
	{
		if(is_managed_member_type_f(type))
			return std::visit(vs, get_managed_member_type_tag(type));
		else
			return udm::visit<true, true, true, ENABLE_DEFAULT_RETURN>(member_type_to_udm_type(type), vs);
	}
	template<typename T>
	concept is_managed_member_type = std::is_same_v<T, EntityURef> || std::is_same_v<T, MultiEntityURef> || std::is_same_v<T, EntityUComponentMemberRef> || std::is_same_v<T, Element>;

	template<typename T>
	constexpr EntityMemberType member_type_to_enum()
	{
		if constexpr(is_managed_member_type<T>) {
			if constexpr(std::is_same_v<T, EntityURef>)
				return EntityMemberType::Entity;
			if constexpr(std::is_same_v<T, MultiEntityURef>)
				return EntityMemberType::MultiEntity;
			if constexpr(std::is_same_v<T, EntityUComponentMemberRef>)
				return EntityMemberType::ComponentProperty;
			if constexpr(std::is_same_v<T, Element>)
				return EntityMemberType::Element;
		}
		else
			return static_cast<EntityMemberType>(udm::type_to_enum<T>());
	}
};

#endif
