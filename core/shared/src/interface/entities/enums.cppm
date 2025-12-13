// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "util_enum_flags.hpp"

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
		using namespace pragma::math::scoped_enum::bitwise;
	}
	REGISTER_ENUM_FLAGS(pragma::FRenderFlags);

	using EntityIndex = uint32_t;

	enum class ObserverMode : int {
		None,
		FirstPerson,
		ThirdPerson,
		Shoulder,
		Roaming,
		Count,
	};

	namespace pragma {
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

		enum class ComponentFlags : uint8_t {
			None = 0u,
			Networked = 1u,

			// Component isn't networked, but wants to be.
			// (e.g. because a networked event has been registered).
			// In this case the component will be networked the next time
			// it is created. Note: This flag only works for
			// Lua-based components! It also has no effect if the
			// component has already been created at least one
			// in the past.
			MakeNetworked = Networked << 1u,

			LuaBased = MakeNetworked << 1u,
			HideInEditor = LuaBased << 1u,
		};

		using NetEventId = uint32_t;
		constexpr NetEventId INVALID_NET_EVENT = std::numeric_limits<decltype(INVALID_NET_EVENT)>::max();

		using namespace pragma::math::scoped_enum::bitwise;
	};
	REGISTER_ENUM_FLAGS(pragma::ComponentMemberFlags)
	REGISTER_ENUM_FLAGS(pragma::FAttachmentMode);

	namespace pragma {
		namespace ents {
			enum class EntityMemberType : uint8_t {
				// Nil = pragma::math::to_integral(udm::Type::Nil),
				String = math::to_integral(udm::Type::String),
				// Utf8String = pragma::math::to_integral(udm::Type::Utf8String),

				Int8 = math::to_integral(udm::Type::Int8),
				UInt8 = math::to_integral(udm::Type::UInt8),
				Int16 = math::to_integral(udm::Type::Int16),
				UInt16 = math::to_integral(udm::Type::UInt16),
				Int32 = math::to_integral(udm::Type::Int32),
				UInt32 = math::to_integral(udm::Type::UInt32),
				Int64 = math::to_integral(udm::Type::Int64),
				UInt64 = math::to_integral(udm::Type::UInt64),

				Float = math::to_integral(udm::Type::Float),
				Double = math::to_integral(udm::Type::Double),
				Boolean = math::to_integral(udm::Type::Boolean),

				Vector2 = math::to_integral(udm::Type::Vector2),
				Vector3 = math::to_integral(udm::Type::Vector3),
				Vector4 = math::to_integral(udm::Type::Vector4),
				Quaternion = math::to_integral(udm::Type::Quaternion),
				EulerAngles = math::to_integral(udm::Type::EulerAngles),
				Srgba = math::to_integral(udm::Type::Srgba),
				HdrColor = math::to_integral(udm::Type::HdrColor),
				Transform = math::to_integral(udm::Type::Transform),
				ScaledTransform = math::to_integral(udm::Type::ScaledTransform),
				Mat4 = math::to_integral(udm::Type::Mat4),
				Mat3x4 = math::to_integral(udm::Type::Mat3x4),

				// Blob = pragma::math::to_integral(udm::Type::Blob),
				// BlobLz4 = pragma::math::to_integral(udm::Type::BlobLz4),

				// Array = pragma::math::to_integral(udm::Type::Array),
				// ArrayLz4 = pragma::math::to_integral(udm::Type::ArrayLz4),
				// Reference = pragma::math::to_integral(udm::Type::Reference),
				// Struct = pragma::math::to_integral(udm::Type::Struct),
				Half = math::to_integral(udm::Type::Half),
				Vector2i = math::to_integral(udm::Type::Vector2i),
				Vector3i = math::to_integral(udm::Type::Vector3i),
				Vector4i = math::to_integral(udm::Type::Vector4i),

				// Managed types
				Entity = math::to_integral(udm::Type::Count) + 1,
				MultiEntity,
				ComponentProperty,
				Element,

				Count,
				Last = Count - 1,
				Invalid = math::to_integral(udm::Type::Invalid),
				VersionIndex = 0 // Increment this when adding or removing enums
			};

			constexpr bool is_udm_member_type(EntityMemberType type) { return math::to_integral(type) < math::to_integral(udm::Type::Count) && type != EntityMemberType::Element; }
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
			constexpr udm::Type member_type_to_udm_type(EntityMemberType type) { return (math::to_integral(type) < math::to_integral(udm::Type::Count)) ? static_cast<udm::Type>(type) : udm::Type::Invalid; }
			constexpr EntityMemberType udm_type_to_member_type(udm::Type type) { return static_cast<EntityMemberType>(type); }
		}

		using ComponentId = uint32_t;
		using ComponentEventId = uint32_t;
		using ComponentMemberIndex = uint32_t;
		constexpr auto INVALID_COMPONENT_ID = std::numeric_limits<ComponentId>::max();
		constexpr auto INVALID_COMPONENT_MEMBER_INDEX = std::numeric_limits<ComponentMemberIndex>::max();

		constexpr bool is_animatable_type(udm::Type type) { return panima::is_animatable_type(type); }
		constexpr bool is_animatable_type(ents::EntityMemberType type) { return (math::to_integral(type) < math::to_integral(udm::Type::Count)) ? is_animatable_type(static_cast<udm::Type>(type)) : false; }
		constexpr bool is_valid_component_property_type(udm::Type type) { return is_animatable_type(type) || type == udm::Type::String || type == udm::Type::Transform || type == udm::Type::ScaledTransform; }
		constexpr bool is_valid_component_property_type(ents::EntityMemberType type)
		{
			static_assert(math::to_integral(ents::EntityMemberType::VersionIndex) == 0);
			return is_valid_component_property_type(static_cast<udm::Type>(type)) || type == ents::EntityMemberType::Entity || type == ents::EntityMemberType::MultiEntity || type == ents::EntityMemberType::ComponentProperty
			  || type == ents::EntityMemberType::Element;
		}
		template<typename T>
		concept is_animatable_type_v = is_animatable_type(udm::type_to_enum<T>());
	};
};
