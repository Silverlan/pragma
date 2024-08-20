/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include <pragma/game/game.h>
#include <pragma/console/convars.h>
#include "pragma/physics/raytraces.h"
#include "pragma/lua/libraries/lmath.h"
#include "pragma/lua/libraries/ldebug.h"
#include <pragma/model/model.h>
#include <fsys/filesystem.h>
#include "luasystem_file.h"
#include "pragma/lua/class_manager.hpp"
#include <pragma/console/conout.h>
#include <pragma/console/cvar.h>
#include <pragma/lua/lua_error_handling.hpp>
#include <luainterface.hpp>
#include <sharedutils/util_string.h>
#include <sharedutils/util_file.h>

extern DLLNETWORK Engine *engine;

Lua::Interface &Game::GetLuaInterface() { return *m_lua; }
lua_State *Game::GetLuaState() { return (m_lua != nullptr) ? m_lua->GetState() : nullptr; }

void Game::InitializeLua()
{
	m_luaIncludeStack.clear();
	m_lua = std::make_shared<Lua::Interface>();
	m_lua->Open();

	m_luaClassManager = std::make_unique<pragma::lua::ClassManager>(*m_lua->GetState());

	Lua::initialize_lua_state(GetLuaInterface());
	RegisterLua();

	// Initialize component ids
	std::unordered_map<std::string, lua_Integer> componentIds;
	for(auto &componentInfo : m_componentManager->GetRegisteredComponentTypes()) {
		auto name = std::string {*componentInfo->name};
		ustring::to_upper(name);
		componentIds.insert(std::make_pair("COMPONENT_" + name, componentInfo->id));
	}
	Lua::RegisterLibraryEnums(GetLuaState(), "ents", componentIds);

	// Call math.randomseed to start Lua's random number generator
	auto tm = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch();
	std::stringstream lseed;
	lseed << "math.randomseed(" << tm.count() << ");";
	Lua::RunString(GetLuaState(), lseed.str(), "internal");

	// Add module paths
	UpdatePackagePaths();

	auto remDeb = engine->GetRemoteDebugging();
	if((remDeb == 1 && IsServer()) == true || (remDeb == 2 && IsClient() == true))
		Lua::debug::enable_remote_debugging(GetLuaState());
}

const pragma::lua::ClassManager &Game::GetLuaClassManager() const
{
	return const_cast<Game *>(this)->GetLuaClassManager();
	;
}
pragma::lua::ClassManager &Game::GetLuaClassManager() { return *m_luaClassManager; }

void Game::SetupLua() { GetNetworkState()->InitializeLuaModules(GetLuaState()); }

Lua::StatusCode Game::LoadLuaFile(std::string &fInOut, fsys::SearchFlags includeFlags, fsys::SearchFlags excludeFlags)
{
	auto *l = GetLuaState();
	auto r = Lua::LoadFile(l, fInOut, includeFlags, excludeFlags);
	Lua::HandleSyntaxError(l, r, fInOut);
	return r;
}

bool Game::ExecuteLuaFile(std::string &fInOut, lua_State *optCustomLuaState)
{
	auto *l = optCustomLuaState ? optCustomLuaState : GetLuaState();
	auto r = Lua::ExecuteFile(l, fInOut, Lua::HandleTracebackError);
	Lua::HandleSyntaxError(l, r, fInOut);
	return r == Lua::StatusCode::Ok;
}
/* Deprecated
bool Game::IncludeLuaFile(std::string &fInOut)
{
	return (Lua::Execute(GetLuaState(),[this,&fInOut](int(*traceback)(lua_State*)) {
		return Lua::IncludeFile(GetLuaState(),fInOut,traceback);
	},GetNetworkState()->GetLuaErrorColorMode()) == Lua::StatusCode::Ok) ? true : false;
}
*/
void Game::RunLuaFiles(const std::string &subPath)
{
	auto *l = GetLuaState();
	Lua::ExecuteFiles(l, subPath, Lua::HandleTracebackError, [this, l](Lua::StatusCode code, const std::string &luaFile) { Lua::HandleSyntaxError(l, code, luaFile); });
}

bool Game::RunLua(const std::string &lua, const std::string &chunkName)
{
	auto *l = GetLuaState();
	auto r = Lua::RunString(l, lua, chunkName, Lua::HandleTracebackError);
	Lua::HandleSyntaxError(l, r);
	return r == Lua::StatusCode::Ok;
}

Lua::StatusCode Game::ProtectedLuaCall(const std::function<Lua::StatusCode(lua_State *)> &pushFuncArgs, int32_t numResults)
{
	auto *l = GetLuaState();
	auto r = Lua::ProtectedCall(GetLuaState(), pushFuncArgs, numResults, Lua::HandleTracebackError);
	Lua::HandleSyntaxError(l, r);
	return r;
}

const std::array<std::string, 6> &Game::GetLuaEntityDirectories() const
{
	static std::array<std::string, 6> dirs = {"entities", "weapons", "vehicles", "npcs", "gamemodes", "players"};
	return dirs;
}

void Game::LoadLuaComponents(const std::string &typePath)
{
	std::string path = Lua::SCRIPT_DIRECTORY_SLASH;
	path += typePath;
	path += "\\components\\*";
	std::vector<std::string> dirs;
	FileManager::FindFiles(path.c_str(), nullptr, &dirs);
	for(auto &dir : dirs)
		LoadLuaComponent(typePath, dir);
}

void Game::LoadLuaEntities(std::string subPath)
{
	std::string path = Lua::SCRIPT_DIRECTORY_SLASH;
	path += subPath;
	path += "\\*";
	std::vector<std::string> dirs;
	FileManager::FindFiles(path.c_str(), NULL, &dirs);
	for(unsigned int i = 0; i < dirs.size(); i++)
		LoadLuaEntity(subPath, dirs[1]);
}

bool Game::LoadLuaComponent(const std::string &luaFilePath, const std::string &mainPath, const std::string &componentName)
{
	auto luaFilePathNoLuaDir = luaFilePath.substr(4);
	return ExecuteLuaFile(luaFilePathNoLuaDir);
}

bool Game::LoadLuaEntity(const std::string &mainPath, const std::string &className)
{
	auto fpath = mainPath + '\\' + className;
	if(FileManager::Exists(Lua::SCRIPT_DIRECTORY_SLASH + fpath + Lua::DOT_FILE_EXTENSION) || FileManager::Exists(Lua::SCRIPT_DIRECTORY_SLASH + fpath + Lua::DOT_FILE_EXTENSION_PRECOMPILED))
		return LoadLuaEntity(fpath);
	fpath = mainPath + '\\' + GetLuaNetworkDirectoryName() + '\\' + className;
	if(FileManager::Exists(Lua::SCRIPT_DIRECTORY_SLASH + fpath + Lua::DOT_FILE_EXTENSION) || FileManager::Exists(Lua::SCRIPT_DIRECTORY_SLASH + fpath + Lua::DOT_FILE_EXTENSION_PRECOMPILED))
		return LoadLuaEntity(fpath);
	return false;
}
bool Game::LoadLuaComponent(const std::string &mainPath, const std::string &componentName)
{
	auto nwStateDirName = GetLuaNetworkDirectoryName();
	auto luaFileName = GetLuaNetworkFileName();
	ufile::remove_extension_from_filename(luaFileName, std::array<std::string, 2> {Lua::FILE_EXTENSION, Lua::FILE_EXTENSION_PRECOMPILED});
	auto nComponentName = FileManager::GetCanonicalizedPath(componentName);
	auto filePath = Lua::SCRIPT_DIRECTORY_SLASH + mainPath + "\\components\\" + nComponentName + '\\';
	auto filePathLuaFile = filePath + luaFileName;
	if(FileManager::Exists(filePathLuaFile + Lua::DOT_FILE_EXTENSION))
		filePathLuaFile += Lua::DOT_FILE_EXTENSION;
	else if(FileManager::Exists(filePathLuaFile + Lua::DOT_FILE_EXTENSION_PRECOMPILED))
		filePathLuaFile += Lua::DOT_FILE_EXTENSION_PRECOMPILED;
	else {
		auto filePathLuaFileNw = filePath + nwStateDirName + '\\' + luaFileName;
		if(FileManager::Exists(filePathLuaFileNw + Lua::DOT_FILE_EXTENSION))
			filePathLuaFile = filePathLuaFileNw + Lua::DOT_FILE_EXTENSION;
		else if(FileManager::Exists(filePathLuaFileNw + Lua::DOT_FILE_EXTENSION_PRECOMPILED))
			filePathLuaFile = filePathLuaFileNw + Lua::DOT_FILE_EXTENSION_PRECOMPILED;
	}
	return LoadLuaComponent(filePathLuaFile, mainPath, componentName);
}

bool Game::LoadLuaEntity(std::string path)
{
	std::string ext;
	if(ufile::get_extension(path, &ext) == true && (ustring::compare<std::string>(ext, Lua::FILE_EXTENSION, false) == true || ustring::compare<std::string>(ext, Lua::FILE_EXTENSION_PRECOMPILED, false) == true))
		return ExecuteLuaFile(path);
	path = Lua::SCRIPT_DIRECTORY + "\\" + path;

	/*
	// Legacy system
	auto nwStateDirName = GetLuaNetworkDirectoryName();
	auto luaFileName = GetLuaNetworkFileName();

	auto luaFilePath = path +'\\' +nwStateDirName;
	if(FileManager::IsDir(luaFilePath))
	{
		luaFilePath += '\\' +luaFileName;
		return ExecuteLuaFile(luaFilePath.substr(4));
	}*/
	auto pathWithoutLuaDir = (path + Lua::DOT_FILE_EXTENSION).substr(4);
	return ExecuteLuaFile(pathWithoutLuaDir);
}

bool Game::LoadLuaComponent(std::string path)
{
	std::string ext;
	if(ufile::get_extension(path, &ext) == true && (ustring::compare<std::string>(ext, Lua::FILE_EXTENSION, false) == true || ustring::compare<std::string>(ext, Lua::FILE_EXTENSION_PRECOMPILED, false) == true))
		return ExecuteLuaFile(path);
	path = Lua::SCRIPT_DIRECTORY_SLASH + path;
	auto nwStateDirName = GetLuaNetworkDirectoryName();
	auto luaFileName = GetLuaNetworkFileName();

	auto luaFilePath = path + '\\' + nwStateDirName;
	if(FileManager::IsDir(luaFilePath)) {
		luaFilePath += '\\' + luaFileName;
		auto pathWithoutLuaDir = luaFilePath.substr(4);
		return ExecuteLuaFile(pathWithoutLuaDir);
	}

	luaFilePath = path + '\\' + luaFileName;
	if(FileManager::IsFile(luaFilePath)) {
		auto pathWithoutLuaDir = luaFilePath.substr(4);
		return ExecuteLuaFile(pathWithoutLuaDir);
	}

	auto pathWithoutLuaDir = (path + Lua::DOT_FILE_EXTENSION).substr(4);
	return ExecuteLuaFile(pathWithoutLuaDir);
}

bool Game::LoadLuaEntityByClass(const std::string &className)
{
	auto nwStateSubPath = GetLuaNetworkDirectoryName() + '\\';
	for(auto &typePath : GetLuaEntityDirectories()) {
		auto baseLuaFilePath = Lua::SCRIPT_DIRECTORY_SLASH + typePath + '\\';
		auto luaFilePath = baseLuaFilePath + className;
		auto nwLuaFilePath = baseLuaFilePath + nwStateSubPath + className;
		if(FileManager::Exists(luaFilePath + Lua::DOT_FILE_EXTENSION) || FileManager::Exists(luaFilePath + Lua::DOT_FILE_EXTENSION_PRECOMPILED) || FileManager::Exists(nwLuaFilePath + Lua::DOT_FILE_EXTENSION) || FileManager::Exists(nwLuaFilePath + Lua::DOT_FILE_EXTENSION_PRECOMPILED))
			return LoadLuaEntity(typePath, className);
	}

	// Obsolete
	/*auto nwStateSubPath = GetLuaNetworkDirectoryName() +'\\';
	for(auto &typePath : GetLuaEntityDirectories())
	{
		auto baseLuaFilePath = Lua::SCRIPT_DIRECTORY_SLASH +typePath +'\\';
		auto luaFilePath = baseLuaFilePath +className;
		auto nwLuaFilePath = baseLuaFilePath +nwStateSubPath +className;
		if(FileManager::IsDir(luaFilePath) || FileManager::IsDir(nwLuaFilePath))
			return LoadLuaEntity(typePath,className);

		luaFilePath += Lua::DOT_FILE_EXTENSION;
		nwLuaFilePath += Lua::DOT_FILE_EXTENSION;
		if(FileManager::Exists(luaFilePath) || FileManager::Exists(nwLuaFilePath))
			return LoadLuaEntity(typePath,className);
	}*/
	return false;
}
bool Game::LoadLuaComponentByName(const std::string &componentName)
{
	for(auto &typePath : GetLuaEntityDirectories()) {
		auto luaFilePath = Lua::SCRIPT_DIRECTORY_SLASH + typePath + "\\components\\" + componentName;
		if(FileManager::IsDir(luaFilePath))
			return LoadLuaComponent(typePath, componentName);
		if(FileManager::Exists(luaFilePath + Lua::DOT_FILE_EXTENSION) || FileManager::Exists(luaFilePath + Lua::DOT_FILE_EXTENSION_PRECOMPILED))
			return LoadLuaComponent(typePath, componentName);
	}
	return false;
}

CallbackHandle Game::AddConVarCallback(const std::string &cvar, LuaFunction function)
{
	auto lcvar = cvar;
	ustring::to_lower(lcvar);
	auto it = m_cvarCallbacks.find(lcvar);
	if(it == m_cvarCallbacks.end())
		it = m_cvarCallbacks.insert(std::make_pair(cvar, std::vector<CvarCallback> {})).first;
	CvarCallback cb {function};
	it->second.push_back(cb);
	return cb.GetFunction();
}
const std::unordered_map<std::string, std::vector<CvarCallback>> &Game::GetConVarCallbacks() const { return m_cvarCallbacks; }

void Game::RegisterLua()
{
	RegisterLuaClasses();
	RegisterLuaLibraries();
	RegisterLuaGlobals();

	auto modDebug = luabind::module_(GetLuaState(), "debug");
	modDebug[luabind::def("stackdump", Lua::debug::stackdump)];
}

DLLNETWORK void IncludeLuaEntityBaseClasses(lua_State *l, int refEntities, int obj, int data)
{
	Lua::PushString(l, "Base");
	Lua::GetTableValue(l, data);
	if(Lua::IsSet(l, -1)) {
		if(Lua::IsTable(l, -1)) {
			int tBase = Lua::GetStackTop(l);
			int i = 1;
			Lua::PushInt(l, i);
			Lua::GetTableValue(l, tBase);
			while(Lua::IsSet(l, -1)) {
				Lua::ToString(l, -1);
				Lua::GetTableValue(l, refEntities);
				if(Lua::IsTable(l, -1))
					IncludeLuaEntityBaseClasses(l, refEntities, obj, Lua::GetStackTop(l));
				Lua::Pop(l, 1);
				i++;
				Lua::PushInt(l, i);
				Lua::GetTableValue(l, tBase);
			}
			Lua::Pop(l, 1);
			Lua::PushNil(l);
			while(Lua::GetNextPair(l, -2)) {
				Lua::PushValue(l, -2);
				Lua::Insert(l, -2);
				Lua::SetTableValue(l, obj);
			}
		}
		else {
			Lua::ToString(l, -1);
			Lua::GetTableValue(l, refEntities);
			if(Lua::IsTable(l, -1)) {
				IncludeLuaEntityBaseClasses(l, refEntities, obj, Lua::GetStackTop(l));
				Lua::PushNil(l);
				while(Lua::GetNextPair(l, -2)) {
					Lua::PushValue(l, -2);
					Lua::Insert(l, -2);
					Lua::SetTableValue(l, obj);
				}
			}
		}
	}
	Lua::Pop(l, 1);
}
