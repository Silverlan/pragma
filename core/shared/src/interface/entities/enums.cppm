// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cinttypes>
#include <mathutil/umath.h>

export module pragma.shared:entities.enums;

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
		using ComponentId = uint32_t;
		using ComponentEventId = uint32_t;
		using ComponentMemberIndex = uint32_t;
		const auto INVALID_COMPONENT_ID = std::numeric_limits<ComponentId>::max();
	};
};
