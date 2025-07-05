// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LASSET_HPP__
#define __LASSET_HPP__

#include "pragma/networkdefinitions.h"
#include <string>

namespace pragma::asset {
	enum class Type : uint8_t;
};
namespace Lua {
	class Interface;
	namespace asset {
		DLLNETWORK void register_library(Lua::Interface &lua, bool extended);

		DLLNETWORK bool exists(lua_State *l, const std::string &name, pragma::asset::Type type);
		DLLNETWORK opt<std::string> find_file(lua_State *l, const std::string &name, pragma::asset::Type type);
		DLLNETWORK bool is_loaded(lua_State *l, const std::string &name, pragma::asset::Type type);
		DLLNETWORK tb<std::string> get_supported_import_file_extensions(lua_State *l, pragma::asset::Type type);
		DLLNETWORK tb<std::string> get_supported_export_file_extensions(lua_State *l, pragma::asset::Type type);
	};
};

#endif
