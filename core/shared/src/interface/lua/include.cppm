// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:scripting.lua.include;

export import pragma.lua;

export namespace pragma::scripting::lua_core {
	struct DLLNETWORK IncludeResult {
		Lua::StatusCode statusCode;
		uint32_t numResults;

		std::string errorFilePath;
		std::string errorMessage;
	};

	enum class IncludeFlags : uint8_t {
		None = 0u,
		AddToCache = 1u,
		SkipIfCached = AddToCache << 1u,
		IgnoreGlobalCache = SkipIfCached << 1u,

		Default = AddToCache | SkipIfCached,
	};
	using namespace pragma::math::scoped_enum::bitwise;

	DLLNETWORK IncludeResult include(lua::State *l, const std::string &path, IncludeFlags flags = IncludeFlags::None);
	DLLNETWORK void execute_files_in_directory(lua::State *l, const std::string &path);
	DLLNETWORK Lua::StatusCode execute_file(lua::State *l, const std::string &path, std::string *optOutErrMsg = nullptr, bool addToIncludeCache = false);
};
export {REGISTER_ENUM_FLAGS(pragma::scripting::lua_core::IncludeFlags)}
