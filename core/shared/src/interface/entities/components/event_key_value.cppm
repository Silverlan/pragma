// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <string>

export module pragma.shared:entities.components.events.key_value;

export import :entities.components.events.event;

export namespace pragma {
	struct DLLNETWORK CEKeyValueData : public ComponentEvent {
		CEKeyValueData(const std::string &key, const std::string &value);
		virtual void PushArguments(lua_State *l) override;
		const std::string &key;
		const std::string &value;
	};
};
