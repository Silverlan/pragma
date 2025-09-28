// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

// module;


// export module pragma.scripting.lua:include;

// export
namespace pragma::scripting::lua {
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

	DLLNETWORK IncludeResult include(lua_State *l, const std::string &path, IncludeFlags flags = IncludeFlags::None);
	DLLNETWORK void execute_files_in_directory(lua_State *l, const std::string &path);
	DLLNETWORK Lua::StatusCode execute_file(lua_State *l, const std::string &path, std::string *optOutErrMsg = nullptr);
};

// export {
	REGISTER_BASIC_BITWISE_OPERATORS(pragma::scripting::lua::IncludeFlags)
// }
