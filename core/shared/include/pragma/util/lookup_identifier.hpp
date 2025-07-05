// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LOOKUP_IDENTIFIER_HPP__
#define __LOOKUP_IDENTIFIER_HPP__

#include "pragma/networkdefinitions.h"
#include <string>
#include <optional>

namespace pragma {
	struct DLLNETWORK LookupIdentifier {
		LookupIdentifier(const std::string &name) : name {name} {}
		LookupIdentifier(const char *name) : name {name} {}
		LookupIdentifier(uint32_t id) : id {id} {}

		std::optional<std::string> name {};
		mutable std::optional<uint32_t> id {};
		bool resolved() const { return id.has_value(); }
	};
};

#endif
