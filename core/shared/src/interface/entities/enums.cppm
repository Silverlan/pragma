// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cinttypes>
#include <mathutil/umath.h>
#include "udm.hpp"

export module pragma.shared:entities.enums;

import panima;

export {
	constexpr uint32_t SF_STARTON = 1024;

	constexpr uint32_t SF_TRIGGER_PLAYERS = 8;
	constexpr uint32_t SF_TRIGGER_NPCS = 16;
	constexpr uint32_t SF_TRIGGER_PHYSICS = 32;
	constexpr uint32_t SF_TRIGGER_EVERYTHING = 64;

    namespace pragma {
        enum class FRenderFlags : uint32_t { None = 0u, CastShadows = 1u, Unlit = CastShadows << 1u };
    };
    REGISTER_BASIC_BITWISE_OPERATORS(pragma::FRenderFlags);

	enum class ObserverMode : int {
		None,
		FirstPerson,
		ThirdPerson,
		Shoulder,
		Roaming,
		Count,
	};

	enum class FAttachmentMode : uint8_t {
		None = 0u,
		PositionOnly = 1u,
		BoneMerge = PositionOnly << 1u,
		UpdateEachFrame = BoneMerge << 1u,
		PlayerView = UpdateEachFrame << 1u,
		PlayerViewYaw = PlayerView << 1u,
		SnapToOrigin = PlayerViewYaw << 1u,

		ForceTranslationInPlace = SnapToOrigin << 1u,
		ForceRotationInPlace = ForceTranslationInPlace << 1u,
		ForceInPlace = ForceTranslationInPlace | ForceRotationInPlace
	};

	namespace pragma {
		enum class ComponentMemberFlags : uint32_t {
			None = 0,
			HideInInterface = 1,
			Controller = HideInInterface << 1u,
			WorldSpace = Controller << 1u,
			ObjectSpace = WorldSpace << 1u,
			// This is merely a hint flag for interfaces not to allow writing this property
			ReadOnly = ObjectSpace << 1u
		};

		enum class AttributeSpecializationType : uint8_t {
			None = 0,
			Color = 1,
			Distance,
			LightIntensity,
			Angle,
			File,
			Directory,
			Plane,
			Normal,
			Fraction,

			Custom,

			Count
		};

		using NetEventId = uint32_t;
		constexpr NetEventId INVALID_NET_EVENT = std::numeric_limits<decltype(pragma::INVALID_NET_EVENT)>::max();
	};
	REGISTER_BASIC_BITWISE_OPERATORS(pragma::ComponentMemberFlags)
	REGISTER_BASIC_BITWISE_OPERATORS(FAttachmentMode);

	namespace pragma {
		namespace ents {
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

			template<typename T>
			concept is_managed_member_type = std::is_same_v<T, EntityURef> || std::is_same_v<T, MultiEntityURef> || std::is_same_v<T, EntityUComponentMemberRef> || std::is_same_v<T, Element>;
		}

		using ComponentId = uint32_t;
		using ComponentEventId = uint32_t;
		using ComponentMemberIndex = uint32_t;
		constexpr auto INVALID_COMPONENT_ID = std::numeric_limits<ComponentId>::max();
		constexpr auto INVALID_COMPONENT_MEMBER_INDEX = std::numeric_limits<ComponentMemberIndex>::max();

		constexpr bool is_animatable_type(udm::Type type) { return panima::is_animatable_type(type); }
		constexpr bool is_animatable_type(pragma::ents::EntityMemberType type) { return (umath::to_integral(type) < umath::to_integral(udm::Type::Count)) ? is_animatable_type(static_cast<udm::Type>(type)) : false; }
		constexpr bool is_valid_component_property_type(udm::Type type) { return is_animatable_type(type) || type == udm::Type::String || type == udm::Type::Transform || type == udm::Type::ScaledTransform; }
		constexpr bool is_valid_component_property_type(pragma::ents::EntityMemberType type)
		{
			static_assert(umath::to_integral(pragma::ents::EntityMemberType::VersionIndex) == 0);
			return is_valid_component_property_type(static_cast<udm::Type>(type)) || type == pragma::ents::EntityMemberType::Entity || type == pragma::ents::EntityMemberType::MultiEntity || type == pragma::ents::EntityMemberType::ComponentProperty
			  || type == pragma::ents::EntityMemberType::Element;
		}
		template<typename T>
		concept is_animatable_type_v = is_animatable_type(udm::type_to_enum<T>());
	};
};
