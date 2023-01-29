/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

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
