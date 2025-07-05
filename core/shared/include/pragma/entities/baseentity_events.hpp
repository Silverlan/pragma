// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __BASEENTITY_EVENTS_HPP__
#define __BASEENTITY_EVENTS_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/entities/entity_component_event.hpp"
#include <string>

namespace pragma {
	struct DLLNETWORK CEKeyValueData : public ComponentEvent {
		CEKeyValueData(const std::string &key, const std::string &value);
		virtual void PushArguments(lua_State *l) override;
		const std::string &key;
		const std::string &value;
	};
};

#endif
