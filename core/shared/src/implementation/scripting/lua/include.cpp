// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_shared.h"
#include <pragma/engine.h>
#include <sharedutils/util_path.hpp>
#include <luainterface.hpp>
#include <stack>

module pragma.scripting.lua;

import :error_handling;

static pragma::scripting::lua::IncludeResult include_file(lua_State *l, const util::Path &fileName, pragma::scripting::lua::IncludeFlags flags, Lua::IncludeCache *optIncludeCache = nullptr);
static pragma::scripting::lua::IncludeResult include_directory(lua_State *l, const util::Path &fullLuaPath, const util::Path &relLuaPath, pragma::scripting::lua::IncludeFlags flags, Lua::IncludeCache *optIncludeCache = nullptr)
{
    // Note: relLuaPath is the path relative to the current Lua include path (see Lua::GetIncludePath)

    // If a pre-compiled and a raw file exist of the same script, only the pre-compiled version will be loaded.
    std::unordered_set<uint32_t> traversed;
    for (auto &ext : {Lua::FILE_EXTENSION_PRECOMPILED, Lua::FILE_EXTENSION}) {
        std::vector<std::string> luaFiles;
        filemanager::find_files(fullLuaPath.GetString() +"*." + ext, &luaFiles, nullptr);
        std::sort(luaFiles.begin(), luaFiles.end());

        for (auto &fileName : luaFiles) {
            auto hash = ustring::string_switch_ci::hash(fileName.substr(0, fileName.length() -ext.length()));
            auto it = traversed.find(hash);
            if (it != traversed.end())
                continue; // Already loaded

            auto nStack = Lua::GetStackTop(l);
            auto relFilePath = util::FilePath(relLuaPath, fileName);
            auto result = include_file(l, relFilePath.GetString(), flags, optIncludeCache);
            auto nStackNew = Lua::GetStackTop(l);
            if (nStackNew > nStack)
                Lua::Pop(l, nStackNew -nStack);
            if (result.statusCode != Lua::StatusCode::Ok)
                return result;
        }
    }
    return pragma::scripting::lua::IncludeResult{Lua::StatusCode::Ok};
}

void pragma::scripting::lua::execute_files_in_directory(lua_State *l, const std::string &path)
{
    auto normPath = ::util::DirPath(path);
    auto fullNormPath = ::util::DirPath(Lua::SCRIPT_DIRECTORY, normPath);

    // If a pre-compiled and a raw file exist of the same script, only the pre-compiled version will be loaded.
    std::unordered_set<uint32_t> traversed;
    for (auto &ext : {Lua::FILE_EXTENSION_PRECOMPILED, Lua::FILE_EXTENSION}) {
        std::vector<std::string> luaFiles;
        filemanager::find_files(fullNormPath.GetString() +"*." + ext, &luaFiles, nullptr);
        std::sort(luaFiles.begin(), luaFiles.end());

        for (auto &fileName : luaFiles) {
            auto hash = ustring::string_switch_ci::hash(fileName.substr(0, fileName.length() -ext.length()));
            auto it = traversed.find(hash);
            if (it != traversed.end())
                continue; // Already loaded

            auto fullPath = ::util::FilePath(normPath, fileName);
            std::string errMsg;
            auto statusCode = execute_file(l, fullPath.GetString(), &errMsg);
            if (statusCode != Lua::StatusCode::Ok)
                submit_error(l, errMsg);
        }
    }
}

struct CallInfo {
    std::string fileName;
    std::string errorMessage;
};
// Lua files may only be executed from the main thread, so we can use a static variable here.
// We need to use a stack to account for recursion.
static std::stack<CallInfo> g_callInfoStack;
// format_error_message may only be called from include_file
static std::optional<std::string> format_error_message(lua_State *l, Lua::StatusCode statusCode, pragma::scripting::lua::ErrorType errType, bool useIncludeCallInfoStack) {
    if (!Lua::IsString(l, -1))
        return {};
    auto *errorMessage = Lua::CheckString(l, -1);
    std::string *fileName = nullptr;
    if (useIncludeCallInfoStack) {
        assert(!g_callInfoStack.empty());
        if (g_callInfoStack.empty())
            throw std::runtime_error{"Invalid call info stack!"};
        auto &item = g_callInfoStack.top();
        fileName = &item.fileName;
    }
    auto formattedMsg = pragma::scripting::lua::format_error_message(l, errorMessage, statusCode, fileName, errType);
    if (useIncludeCallInfoStack)
        g_callInfoStack.top().errorMessage = formattedMsg;
    return formattedMsg;
}

template<bool useIncludeCallInfoStack>
static Lua::StatusCode execute_file(lua_State *l, const std::string &path, std::string *optOutErrMsg)
{
    auto pathNorm = path;
    std::string errMsg;
    auto statusCode = Lua::ExecuteFile(l, pathNorm, errMsg, [](lua_State *l) -> int32_t {
        auto newErrMsg = format_error_message(l, Lua::StatusCode::ErrorRun, pragma::scripting::lua::ErrorType::RuntimeError, useIncludeCallInfoStack);
        if (!newErrMsg)
            return 0;
        Lua::PushString(l, *newErrMsg);
        return 1; // We don't have to pop the original error message
    }, LUA_MULTRET, [](lua_State *l, Lua::StatusCode statusCode) {
        // File failed to load, most likely a syntax error.
        // Possible error codes are: ErrorFile (file not found), ErrorSyntax, ErrorMemory
        auto newErrMsg = format_error_message(l, statusCode, pragma::scripting::lua::ErrorType::LoadError, useIncludeCallInfoStack);
        if (!newErrMsg)
            return;
        Lua::Pop(l, 1); // Pop the original error message
        Lua::PushString(l, *newErrMsg);
    });
    if (statusCode == Lua::StatusCode::Ok)
        return statusCode;
    if (optOutErrMsg)
        *optOutErrMsg = std::move(errMsg);
    else
        pragma::scripting::lua::submit_error(l, errMsg);
    return statusCode;
}

Lua::StatusCode pragma::scripting::lua::execute_file(lua_State *l, const std::string &path, std::string *optOutErrMsg)
{
    return ::execute_file<false>(l, path, optOutErrMsg);
}

pragma::scripting::lua::IncludeResult include_file(lua_State *l, const util::Path &fileName, pragma::scripting::lua::IncludeFlags flags, Lua::IncludeCache *optIncludeCache) {
    auto includeFileName = Lua::GetIncludePath(fileName.GetString());

    if (optIncludeCache) {
        if (umath::is_flag_set(flags, pragma::scripting::lua::IncludeFlags::SkipIfCached) && optIncludeCache->Contains(includeFileName))
            return pragma::scripting::lua::IncludeResult {Lua::StatusCode::Ok};
    }

    g_callInfoStack.push({includeFileName});
    std::string errMsg;
    auto statusCode = execute_file<true>(l, includeFileName, &errMsg);

    auto callInfo = std::move(g_callInfoStack.top());
    g_callInfoStack.pop();
    if (statusCode == Lua::StatusCode::Ok) {
        if (optIncludeCache && umath::is_flag_set(flags, pragma::scripting::lua::IncludeFlags::AddToCache))
            optIncludeCache->Add(includeFileName);
        return {Lua::StatusCode::Ok};
    }

    pragma::scripting::lua::IncludeResult result {};
    result.statusCode = statusCode;
    result.errorFilePath = std::move(includeFileName);
    result.errorMessage = std::move(errMsg);
    return result;
}

pragma::scripting::lua::IncludeResult pragma::scripting::lua::include(lua_State *l, const std::string &path, IncludeFlags flags) {
    static Lua::IncludeCache tmpCache;
    static uint32_t recursionDepth = 0;
    Lua::IncludeCache *includeCache = nullptr;
    if (umath::is_flag_set(flags, IncludeFlags::IgnoreGlobalCache)) {
        flags |= IncludeFlags::AddToCache;
        includeCache = &tmpCache;
        ++recursionDepth;
    }

    if (!includeCache) {
        auto *lInterface = pragma::get_engine()->GetLuaInterface(l);
        if (lInterface)
            includeCache = &lInterface->GetIncludeCache();
    }

    auto nStack = Lua::GetStackTop(l);
    IncludeResult result;
    auto fullIncludeDirPath = ::util::DirPath(Lua::SCRIPT_DIRECTORY, Lua::GetIncludePath(path)).GetString();
    if (filemanager::is_dir(fullIncludeDirPath))
        result = include_directory(l, fullIncludeDirPath, path, flags, includeCache);
    else
        result = include_file(l, path, flags, includeCache);

    auto numResults = Lua::GetStackTop(l) -nStack;
    assert(numResults >= 0);
    if (numResults < 0)
        numResults = 0;

    if (umath::is_flag_set(flags, IncludeFlags::IgnoreGlobalCache) && --recursionDepth == 0)
        tmpCache.Clear();

    result.numResults = numResults;
    return result;
}
