// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:util.lookup_identifier;

export import std.compat;

export namespace pragma {
	struct DLLNETWORK LookupIdentifier {
		LookupIdentifier(const std::string &name) : name {name} {}
		LookupIdentifier(const char *name) : name {name} {}
		LookupIdentifier(uint32_t id) : id {id} {}

		std::optional<std::string> name {};
		mutable std::optional<uint32_t> id {};
		bool resolved() const { return id.has_value(); }
	};
};
