/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __PRAGMA__GLOBAL_STRING_TABLE_HPP__
#define __PRAGMA__GLOBAL_STRING_TABLE_HPP__

#include "pragma/definitions.h"
#include <iostream>
#include <string>
#include <string_view>

namespace pragma {
#pragma pack(push, 1)
	struct DLLNETWORK GString {
		GString();
		GString(const char *str);
		GString(const std::string &str);
		GString(const std::string_view &str);
		GString(const GString &other);
		GString &operator=(const char *str);
		GString &operator=(const std::string &str);
		GString &operator=(const std::string_view &str);
		GString &operator=(const GString &str);
		bool operator==(const char *str) const;
		bool operator!=(const char *str) const;
		bool operator==(const std::string &str) const;
		bool operator!=(const std::string &str) const;
		bool operator==(const std::string_view &str) const;
		bool operator!=(const std::string_view &str) const;
		bool operator==(const GString &str) const;
		bool operator!=(const GString &str) const;
		operator const char *() const;
		operator std::string() const;
		operator std::string_view() const;
		const char *operator*() const;
		const char *c_str() const;
		bool empty() const;
		size_t length() const;
		const char *str = nullptr;
	};
#pragma pack(pop)
	DLLNETWORK const char *register_global_string(const std::string &str);
	DLLNETWORK const char *register_global_string(const std::string_view &str);
	DLLNETWORK const char *register_global_string(const char *str);
};

DLLNETWORK std::ostream &operator<<(std::ostream &stream, const pragma::GString &str);

#endif
