// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/entities/entity_component_event.hpp"
#include <string>

export module pragma.shared:entities.components.events.key_value;

export namespace pragma {
	struct DLLNETWORK CEKeyValueData : public ComponentEvent {
		CEKeyValueData(const std::string &key, const std::string &value);
		virtual void PushArguments(lua_State *l) override;
		const std::string &key;
		const std::string &value;
	};
};
