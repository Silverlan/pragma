// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.libraries.asset;

export import :assets.common;
export import :scripting.lua.api;

export namespace Lua {
	namespace asset {
		DLLNETWORK void register_library(Interface &lua, bool extended);

		DLLNETWORK bool exists(lua::State *l, const std::string &name, pragma::asset::Type type);
		DLLNETWORK opt<std::string> find_file(lua::State *l, const std::string &name, pragma::asset::Type type);
		DLLNETWORK bool is_loaded(lua::State *l, const std::string &name, pragma::asset::Type type);
		DLLNETWORK tb<std::string> get_supported_import_file_extensions(lua::State *l, pragma::asset::Type type);
		DLLNETWORK tb<std::string> get_supported_export_file_extensions(lua::State *l, pragma::asset::Type type);
	};
};
