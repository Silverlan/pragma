// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.events.event_info;

export import :entities.enums;

export namespace pragma {
	struct DLLNETWORK ComponentEventInfo {
		enum class Type : uint8_t { Broadcast = 0, Explicit };
		ComponentEventInfo(const std::string &name, std::optional<ComponentId> componentId = {}, std::optional<std::type_index> typeIndex = {}, Type type = Type::Broadcast)
		    : name(name), type {type}, typeIndex {typeIndex}, componentId {componentId.has_value() ? *componentId : INVALID_COMPONENT_ID}
		{
		}
		ComponentEventId id = std::numeric_limits<ComponentEventId>::max();
		std::string name;
		ComponentId componentId = INVALID_COMPONENT_ID;
		Type type = Type::Broadcast;

		// Only set if this is a C++ component
		std::optional<std::type_index> typeIndex {};
	};
	using TRegisterComponentEvent = const std::function<ComponentEventId(const std::string &, ComponentEventInfo::Type)> &;
};
