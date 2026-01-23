// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

module pragma.shared;

import :util.global_string_table;

namespace pragma::ents {
	struct StringTable {
		const char *RegisterString(const std::string &str);
		const char *RegisterString(const std::string_view &str);
		const char *RegisterString(const char *str);
		~StringTable();
	  private:
		std::unordered_map<std::string, const char *> m_strings;
		std::mutex m_mutex;
	};
};

pragma::ents::StringTable::~StringTable()
{
	std::unique_lock lock {m_mutex};
	for(auto &pair : m_strings)
		delete[] pair.second;
}

const char *pragma::ents::StringTable::RegisterString(const std::string_view &str)
{
	// TODO: Once https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0919r2.html is part of the C++ standard, we can use string_view directly for the map
	// lookup. Until then we'll convert it to a std::string.
	return RegisterString(std::string {str});
}

const char *pragma::ents::StringTable::RegisterString(const std::string &str)
{
	std::unique_lock lock {m_mutex};
	auto it = m_strings.find(str);
	if(it != m_strings.end())
		return it->second;
	else {
		char *registrationId = new char[str.size() + 1];
		std::copy(str.begin(), str.end(), registrationId);
		registrationId[str.size()] = '\0';
		m_strings.emplace(str, registrationId);
		return registrationId;
	}
}

const char *pragma::ents::StringTable::RegisterString(const char *str)
{
	std::unique_lock lock {m_mutex};
	auto it = m_strings.find(str);
	if(it != m_strings.end())
		return it->second;
	else {
		auto len = strlen(str);
		char *registrationId = new char[len + 1];
		strcpy(registrationId, str);
		registrationId[len] = '\0';
		m_strings.emplace(str, registrationId);
		return registrationId;
	}
}

DLLNETWORK pragma::ents::StringTable g_stringTable;
const char *pragma::register_global_string(const std::string &str) { return g_stringTable.RegisterString(str); }
const char *pragma::register_global_string(const std::string_view &str) { return g_stringTable.RegisterString(str); }
const char *pragma::register_global_string(const char *str) { return g_stringTable.RegisterString(str); }

pragma::GString::GString() {}
pragma::GString::GString(const char *str) : str {str} {}
pragma::GString::GString(const std::string &str) : str {register_global_string(str)} {}
pragma::GString::GString(const std::string_view &str) : str {register_global_string(str)} {}
pragma::GString::GString(const GString &other) : str {other.str} {}

pragma::GString &pragma::GString::operator=(const char *str)
{
	this->str = str;
	return *this;
}
pragma::GString &pragma::GString::operator=(const std::string &str)
{
	this->str = register_global_string(str);
	return *this;
}
pragma::GString &pragma::GString::operator=(const std::string_view &str)
{
	this->str = register_global_string(str);
	return *this;
}
pragma::GString &pragma::GString::operator=(const GString &str)
{
	this->str = str.str;
	return *this;
}
const char *pragma::GString::operator*() const { return str; }
const char *pragma::GString::c_str() const { return str; }
bool pragma::GString::empty() const { return str == nullptr || str[0] == '\0'; }
size_t pragma::GString::length() const { return str ? strlen(str) : 0; }

pragma::GString::operator const char *() const { return str ? str : ""; }
pragma::GString::operator std::string() const { return str ? str : ""; }
pragma::GString::operator std::string_view() const { return str ? str : ""; }
pragma::GString::operator bool() const { return str ? true : false; }

bool pragma::GString::operator==(const char *str) const
{
	if(this->str == nullptr && str == nullptr) {
		return true;
	}
	if(this->str == nullptr || str == nullptr) {
		return false;
	}
	return strcmp(this->str, str) == 0;
}
bool pragma::GString::operator!=(const char *str) const { return !(*this == str); }
bool pragma::GString::operator==(const std::string &str) const { return *this == str.c_str(); }
bool pragma::GString::operator!=(const std::string &str) const { return !(*this == str); }
bool pragma::GString::operator==(const std::string_view &str) const { return *this == str.data(); }
bool pragma::GString::operator!=(const std::string_view &str) const { return !(*this == str); }
bool pragma::GString::operator==(const GString &other) const { return *this == other.str; }
bool pragma::GString::operator!=(const GString &other) const { return !(*this == other); }

std::ostream &pragma::operator<<(std::ostream &stream, const GString &str)
{
	if(!str)
		stream << 0;
	else
		stream << str.c_str();
	return stream;
}
