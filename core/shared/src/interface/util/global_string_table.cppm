// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:util.global_string_table;

export import std.compat;

export {
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
			explicit operator const char *() const;
			operator std::string() const;
			operator std::string_view() const;
			explicit operator bool() const;
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
		DLLNETWORK std::ostream &operator<<(std::ostream &stream, const GString &str);
	};

	template<>
	struct std::hash<pragma::GString> {
		std::size_t operator()(const pragma::GString &k) const { return std::hash<std::string>()(k.c_str()); }
	};
};
