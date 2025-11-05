// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

module pragma.shared;

import :game.game;

Lua::Interface &pragma::Game::GetLuaInterface() { return *m_lua; }
lua::State *pragma::Game::GetLuaState() { return (m_lua != nullptr) ? m_lua->GetState() : nullptr; }

void pragma::Game::InitializeLua()
{
	m_luaIncludeStack.clear();
	m_lua = std::make_shared<Lua::Interface>();
	m_lua->Open();

	m_luaClassManager = std::make_unique<pragma::LuaCore::ClassManager>(*m_lua->GetState());

	Lua::initialize_lua_state(GetLuaInterface());
	RegisterLua();

	// Initialize component ids
	std::unordered_map<std::string, lua::Integer> componentIds;
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
	pragma::scripting::lua_core::run_string(GetLuaState(), lseed.str(), "initialize_random_seed");

	// Add module paths
	UpdatePackagePaths();

	auto remDeb = pragma::Engine::Get()->GetRemoteDebugging();
	if((remDeb == 1 && IsServer()) == true || (remDeb == 2 && IsClient() == true))
		Lua::debug::enable_remote_debugging(GetLuaState());
}

const pragma::LuaCore::ClassManager &pragma::Game::GetLuaClassManager() const { return const_cast<pragma::Game *>(this)->GetLuaClassManager(); }
pragma::LuaCore::ClassManager &pragma::Game::GetLuaClassManager() { return *m_luaClassManager; }

void pragma::Game::SetupLua() { GetNetworkState()->InitializeLuaModules(GetLuaState()); }

Lua::StatusCode pragma::Game::LoadLuaFile(std::string &fInOut, fsys::SearchFlags includeFlags, fsys::SearchFlags excludeFlags)
{
	auto *l = GetLuaState();
	auto r = Lua::LoadFile(l, fInOut, includeFlags, excludeFlags);
	Lua::HandleSyntaxError(l, r, fInOut);
	return r;
}

bool pragma::Game::ExecuteLuaFile(std::string &fInOut, lua::State *optCustomLuaState)
{
	auto *l = optCustomLuaState ? optCustomLuaState : GetLuaState();
	auto r = pragma::scripting::lua_core::execute_file(l, fInOut);
	return r == Lua::StatusCode::Ok;
}

void pragma::Game::RunLuaFiles(const std::string &subPath)
{
	auto *l = GetLuaState();
	pragma::scripting::lua_core::execute_files_in_directory(l, subPath);
}

bool pragma::Game::RunLua(const std::string &lua, const std::string &chunkName)
{
	auto *l = GetLuaState();
	auto r = pragma::scripting::lua_core::run_string(l, lua, chunkName);
	return r == Lua::StatusCode::Ok;
}

Lua::StatusCode pragma::Game::ProtectedLuaCall(const std::function<Lua::StatusCode(lua::State *)> &pushFuncArgs, int32_t numResults) { return pragma::scripting::lua_core::protected_call(GetLuaState(), pushFuncArgs, numResults); }

const std::array<std::string, 6> &pragma::Game::GetLuaEntityDirectories() const
{
	static std::array<std::string, 6> dirs = {"entities", "weapons", "vehicles", "npcs", "gamemodes", "players"};
	return dirs;
}

void pragma::Game::LoadLuaComponents(const std::string &typePath)
{
	std::string path = Lua::SCRIPT_DIRECTORY_SLASH;
	path += typePath;
	path += "\\components\\*";
	std::vector<std::string> dirs;
	FileManager::FindFiles(path.c_str(), nullptr, &dirs);
	for(auto &dir : dirs)
		LoadLuaComponent(typePath, dir);
}

void pragma::Game::LoadLuaEntities(std::string subPath)
{
	std::string path = Lua::SCRIPT_DIRECTORY_SLASH;
	path += subPath;
	path += "\\*";
	std::vector<std::string> dirs;
	FileManager::FindFiles(path.c_str(), nullptr, &dirs);
	for(unsigned int i = 0; i < dirs.size(); i++)
		LoadLuaEntity(subPath, dirs[1]);
}

bool pragma::Game::LoadLuaComponent(const std::string &luaFilePath, const std::string &mainPath, const std::string &componentName)
{
	auto luaFilePathNoLuaDir = luaFilePath.substr(4);
	return ExecuteLuaFile(luaFilePathNoLuaDir);
}

bool pragma::Game::LoadLuaEntity(const std::string &mainPath, const std::string &className)
{
	auto fpath = mainPath + '\\' + className;
	if(FileManager::Exists(Lua::SCRIPT_DIRECTORY_SLASH + fpath + Lua::DOT_FILE_EXTENSION) || FileManager::Exists(Lua::SCRIPT_DIRECTORY_SLASH + fpath + Lua::DOT_FILE_EXTENSION_PRECOMPILED))
		return LoadLuaEntity(fpath);
	fpath = mainPath + '\\' + GetLuaNetworkDirectoryName() + '\\' + className;
	if(FileManager::Exists(Lua::SCRIPT_DIRECTORY_SLASH + fpath + Lua::DOT_FILE_EXTENSION) || FileManager::Exists(Lua::SCRIPT_DIRECTORY_SLASH + fpath + Lua::DOT_FILE_EXTENSION_PRECOMPILED))
		return LoadLuaEntity(fpath);
	return false;
}
bool pragma::Game::LoadLuaComponent(const std::string &mainPath, const std::string &componentName)
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

bool pragma::Game::LoadLuaEntity(std::string path)
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

bool pragma::Game::LoadLuaComponent(std::string path)
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

bool pragma::Game::LoadLuaEntityByClass(const std::string &className)
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
bool pragma::Game::LoadLuaComponentByName(const std::string &componentName)
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

CallbackHandle pragma::Game::AddConVarCallback(const std::string &cvar, LuaFunction function)
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
const std::unordered_map<std::string, std::vector<CvarCallback>> &pragma::Game::GetConVarCallbacks() const { return m_cvarCallbacks; }

void pragma::Game::RegisterLua()
{
	RegisterLuaClasses();
	RegisterLuaLibraries();
	RegisterLuaGlobals();

	auto modDebug = luabind::module_(GetLuaState(), "debug");
	modDebug[luabind::def("stackdump", Lua::debug::stackdump)];
}

DLLNETWORK void IncludeLuaEntityBaseClasses(lua::State *l, int refEntities, int obj, int data)
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
