/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/libraries/lfile.h"
#include "luasystem.h"
#include "pragma/game/game_resources.hpp"
#include <sharedutils/util_file.h>
#include <sharedutils/util_library.hpp>
#include <sharedutils/util_path.hpp>
#include <fsys/ifile.hpp>

extern DLLNETWORK Engine *engine;

static bool is_permitted_root_dir(const std::string_view &str) { return str == "cache" or str == "temp"; }

LFile::LFile() {}

void LFile::Construct(const VFilePtr &f) { m_file = std::make_shared<fsys::File>(f); }
void LFile::Construct(const std::shared_ptr<ufile::IFile> &f) { m_file = f; }

bool LFile::Construct(const char *path, const char *mode, fsys::SearchFlags fsearchmode)
{
	auto f = FileManager::OpenFile(path, mode, nullptr, fsearchmode);
	if(!f)
		return false;
	m_file = std::make_shared<fsys::File>(f);
	return true;
}

LFile::~LFile() { Close(); }

std::shared_ptr<ufile::IFile> LFile::GetHandle()
{
	if(!IsValid())
		return NULL;
	return m_file;
}

bool LFile::Eof()
{
	if(!IsValid())
		return true;
	return (m_file->Eof() != 0) ? true : false;
}

bool LFile::IsValid() const { return (m_file != NULL) ? true : false; }

void LFile::Close()
{
	if(!IsValid())
		return;
	m_file.reset();
	m_file = NULL;
}

unsigned long long LFile::Size()
{
	if(!IsValid())
		return 0;
	return m_file->GetSize();
}

long long LFile::Tell()
{
	if(!IsValid())
		return 0;
	return m_file->Tell();
}

void LFile::Seek(unsigned long long pos)
{
	if(!IsValid())
		return;
	return m_file->Seek(pos);
}

bool LFile::Read(void *buf, unsigned long long l)
{
	if(!IsValid())
		return false;
	m_file->Read(buf, l);
	return true;
}

void LFile::Write(void *c, unsigned long long l)
{
	if(!IsValid())
		return;
	m_file->Write(c, l);
}

void LFile::WriteString(std::string str)
{
	if(!IsValid())
		return;
	m_file->WriteString(str);
}

std::string LFile::ReadLine()
{
	if(!IsValid())
		return "";
	return m_file->ReadLine();
}

std::string LFile::ReadString()
{
	if(!IsValid())
		return "";
	return m_file->ReadString();
}

void LFile::IgnoreComments(std::string start, std::string end)
{
	if(!IsValid())
		return;
	//m_file->IgnoreComments(start,end);
}

////////////////////////////////////

DLLNETWORK void Lua_LFile_Close(lua_State *, LFile &f) { f.Close(); }

DLLNETWORK void Lua_LFile_Size(lua_State *l, LFile &f)
{
	long long size = f.Size();
	Lua::PushInt<long long>(l, size);
}

DLLNETWORK void Lua_LFile_ReadLine(lua_State *l, LFile &f) { Lua::PushString(l, f.ReadLine()); }

DLLNETWORK void Lua_LFile_ReadString(lua_State *l, LFile &f, uint32_t len)
{
	if(!f.IsValid())
		return;
	std::string out;
	out.resize(len);
	f.Read(&out[0], len);
	Lua::PushString(l, out);
}

DLLNETWORK void Lua_LFile_ReadString(lua_State *l, LFile &f)
{
	if(!f.IsValid())
		return;
	auto file = f.GetHandle();
	std::string str = file->ReadString();
	lua_pushstring(l, str.c_str());
}

DLLNETWORK void Lua_LFile_WriteString(lua_State *, LFile &f, std::string str, bool bNullTerminated)
{
	if(!f.IsValid())
		return;
	auto file = f.GetHandle();
	if(bNullTerminated == true)
		file->WriteString(str);
	else
		file->Write(str.data(), str.length());
}

DLLNETWORK void Lua_LFile_WriteString(lua_State *l, LFile &f, std::string str) { Lua_LFile_WriteString(l, f, str, true); }

void Lua_LFile_ReadVector(lua_State *l, LFile &f)
{
	float x = f.Read<float>();
	float y = f.Read<float>();
	float z = f.Read<float>();
	luabind::object(l, Vector3(x, y, z)).push(l);
}

void Lua_LFile_WriteVector2(lua_State *, LFile &f, const Vector2 &v)
{
	f.Write<float>(v.x);
	f.Write<float>(v.y);
}

void Lua_LFile_ReadVector2(lua_State *l, LFile &f)
{
	auto x = f.Read<float>();
	auto y = f.Read<float>();
	luabind::object(l, Vector2(x, y)).push(l);
}

void Lua_LFile_WriteVector4(lua_State *, LFile &f, const Vector4 &v)
{
	f.Write<float>(v.x);
	f.Write<float>(v.y);
	f.Write<float>(v.z);
	f.Write<float>(v.w);
}

void Lua_LFile_ReadVector4(lua_State *l, LFile &f)
{
	auto x = f.Read<float>();
	auto y = f.Read<float>();
	auto z = f.Read<float>();
	auto w = f.Read<float>();
	luabind::object(l, Vector4(x, y, z, w)).push(l);
}

void Lua_LFile_WriteVector(lua_State *, LFile &f, const Vector3 &v)
{
	f.Write<float>(v.x);
	f.Write<float>(v.y);
	f.Write<float>(v.z);
}

DLLNETWORK void Lua_LFile_ReadAngles(lua_State *l, LFile &f)
{
	float p = f.Read<float>();
	float y = f.Read<float>();
	float r = f.Read<float>();
	luabind::object(l, EulerAngles(p, y, r)).push(l);
}

DLLNETWORK void Lua_LFile_WriteAngles(lua_State *, LFile &f, EulerAngles ang)
{
	f.Write<float>(ang.p);
	f.Write<float>(ang.y);
	f.Write<float>(ang.r);
}

DLLNETWORK void Lua_LFile_Seek(lua_State *, LFile &f, uint32_t pos) { f.Seek(pos); }

DLLNETWORK void Lua_LFile_Tell(lua_State *l, LFile &f)
{
	long long pos = f.Tell();
	Lua::PushNumber<long long>(l, pos);
}

DLLNETWORK void Lua_LFile_Eof(lua_State *l, LFile &f) { Lua::PushBool(l, f.Eof()); }

DLLNETWORK void Lua_LFile_IgnoreComments(lua_State *, LFile &f) { f.IgnoreComments(); }
DLLNETWORK void Lua_LFile_IgnoreComments(lua_State *, LFile &f, std::string start) { f.IgnoreComments(start); }
DLLNETWORK void Lua_LFile_IgnoreComments(lua_State *, LFile &f, std::string start, std::string end) { f.IgnoreComments(start, end); }
void Lua_LFile_Read(lua_State *l, LFile &f, uint32_t size)
{
	DataStream ds;
	ds->Resize(size);
	f.Read(ds->GetData(), size);
	Lua::Push<DataStream>(l, ds);
}
void Lua_LFile_Read(lua_State *, LFile &f, ::DataStream &ds, uint32_t size)
{
	auto offset = ds->GetOffset();
	ds->Resize(offset + size);
	f.Read(ds->GetData() + offset, size);
}
void Lua_LFile_Write(lua_State *, LFile &f, ::DataStream &ds)
{
	auto offset = ds->GetOffset();
	f.Write(ds->GetData() + offset, ds->GetInternalSize() - offset);
}
void Lua_LFile_Write(lua_State *, LFile &f, ::DataStream &ds, uint32_t size)
{
	auto offset = ds->GetOffset();
	f.Write(ds->GetData() + offset, size - offset);
}
void Lua_LFile_GetPath(lua_State *l, LFile &f)
{
	if(!f.IsValid())
		return;
	auto file = f.GetHandle();
	auto fileName = file->GetFileName();
	if(!fileName.has_value())
		return;
	Lua::PushString(l, *fileName);
}

////////////////////////////////////

std::string Lua::file::to_relative_path(const std::string &path)
{
	auto opath = util::Path::CreateFile(path);
	opath.Canonicalize();
	if(ustring::compare<std::string_view>(opath.GetFront(), "addons", false)) {
		opath.PopFront();
		opath.PopFront();
	}
	return opath.GetString();
}
static bool is_extension_blacklisted(const std::string &ext)
{
	using namespace ustring::string_switch_ci;
	switch(hash(ext)) {
	// Windows File Extensions
	case "exe"_:
	case "dll"_:
	// case "bat"_:
	case "cmd"_:
	case "com"_:
	case "vbs"_:
	case "ps1"_:
	case "msi"_:
	case "jar"_:
	case "app"_:
	case "scr"_:
	case "reg"_:
	case "vb"_:
	case "js"_:
	case "wsf"_:
	case "pif"_:
	case "pyc"_:
	case "psd1"_:
		// case "sh"_:
		return true;

	// Linux File Extensions
	case "so"_:
	case "ko"_:
	case "run"_:
	case "out"_:
	case "AppImage"_:
	case "deb"_:
	case "rpm"_:
	case "bash"_:
	case "zsh"_:
	case "csh"_:
	case "awk"_:
	case "pl"_:
	case "php"_:
	case "py"_:
	case "rb"_:
	case "class"_:
	case "war"_:
	case "cgi"_:

	// Code files
	case "h"_:
	case "hpp"_:
	case "cpp"_:

	// Shader files
	case "ptx"_:
	case "cubin"_:
		return true;
	default:
		return false;
	}
	return false;
}
static const std::vector<std::string> pathBlacklist {
  "cache/chromium",
  "cache/shader.cache",
  "cache/shaders",
};
bool Lua::file::validate_write_operation(lua_State *l, std::string &path, std::string &outRootPath)
{
	std::string ext;
	if(ufile::get_extension(path, &ext) && is_extension_blacklisted(ext))
		return false;
	auto opath = util::Path::CreateFile(path);
	opath.Canonicalize();
	for(auto &item : pathBlacklist) {
		if(ustring::find(opath.GetString(), item, false) != std::string::npos)
			return false;
	}
	if(path.length() >= 7 && ustring::compare(path.c_str(), "addons", false, 6) && (path.at(6) == '/' || path.at(6) == '\\')) {
		// Validate that this is an addon path
		auto addonPath = opath;
		while(addonPath.GetComponentCount() > 2)
			addonPath.PopBack();

		if(ustring::compare<std::string_view>(addonPath.GetFront(), "addons", false) && FileManager::Exists(addonPath.GetString())) {
			opath.PopFront();
			opath.PopFront();
			outRootPath = addonPath.GetString();
			path = opath.GetString();
			return true;
		}
	}
	auto fname = FileManager::GetCanonicalizedPath(Lua::get_current_file(l));
	if(fname.length() < 8 || ustring::compare(fname.c_str(), "addons\\", false, 7) == false) {
		if(Lua::get_extended_lua_modules_enabled()) {
			outRootPath = "";
			return true;
		}
		Con::cwar << "File write-operations can only be performed by Lua-scripts inside an addon!" << Con::endl;
		return false;
	}
	auto br = fname.find('\\', 8);
	auto prefix = ustring::substr(fname, 0, br + 1);
	outRootPath = prefix;
	path = FileManager::GetCanonicalizedPath(path);
	if(is_permitted_root_dir(util::Path {path}.GetFront()))
		outRootPath = ""; // Special case; We'll allow file writes in the cache directory
	return true;
}

// Also used in wv_sqlite module
bool Lua::file::validate_write_operation(lua_State *l, std::string &path)
{
	std::string prefix;
	if(validate_write_operation(l, path, prefix) == false)
		return false;
	path = prefix + path;
	return true;
}

std::shared_ptr<LFile> Lua::file::Open(lua_State *l, std::string path, FileOpenMode openMode, fsys::SearchFlags searchFlags)
{
	std::string mode {};
	if((openMode & FileOpenMode::Read) != FileOpenMode::None)
		mode += "r";
	else if((openMode & FileOpenMode::Write) != FileOpenMode::None)
		mode += "w";
	else if((openMode & FileOpenMode::Append) != FileOpenMode::None)
		mode += "a";
	else
		return 0;
	if((openMode & FileOpenMode::Binary) != FileOpenMode::None)
		mode += "b";
	if((openMode & FileOpenMode::Update) != FileOpenMode::None)
		mode += "+";
	if((openMode & (FileOpenMode::Write | FileOpenMode::Append)) != FileOpenMode::None) // Write mode
	{
		if(validate_write_operation(l, path) == false)
			return 0;
	}
	auto f = std::make_shared<LFile>();
	if(f->Construct(path.c_str(), mode.c_str(), searchFlags) == false)
		return nullptr;
	return f;
}

bool Lua::file::CreateDir(lua_State *l, std::string path)
{
	if(validate_write_operation(l, path) == false)
		return false;
	return FileManager::CreateDirectory(path.c_str());
}

bool Lua::file::CreatePath(lua_State *l, std::string path)
{
	if(validate_write_operation(l, path) == false)
		return false;
	return FileManager::CreatePath(path.c_str());
}

bool Lua::file::Delete(lua_State *l, std::string ppath)
{
	auto path = ppath;
	if(validate_write_operation(l, path) == false)
		return false;
	if(Lua::get_extended_lua_modules_enabled() && FileManager::Exists(path) == false && FileManager::Exists(ppath))
		return filemanager::remove_file(ppath);
	return filemanager::remove_file(path);
}

bool Lua::file::DeleteDir(lua_State *l, std::string ppath)
{
	auto path = ppath;
	if(validate_write_operation(l, path) == false)
		return false;
	if(Lua::get_extended_lua_modules_enabled() && FileManager::Exists(path) == false && FileManager::Exists(ppath))
		return filemanager::remove_directory(ppath);
	return filemanager::remove_directory(path);
}

std::shared_ptr<LFile> Lua::file::open_external_asset_file(lua_State *l, const std::string &path, const std::optional<std::string> &game)
{
	auto dllHandle = util::initialize_external_archive_manager(engine->GetNetworkState(l));
	if(dllHandle == nullptr)
		return nullptr;
	auto *fOpenFile = dllHandle->FindSymbolAddress<void (*)(const std::string &, VFilePtr &, const std::optional<std::string> &)>("open_archive_file");
	if(fOpenFile == nullptr)
		return nullptr;
	VFilePtr f = nullptr;
	fOpenFile(path, f, game);
	if(f == nullptr)
		return nullptr;
	auto lf = std::make_shared<LFile>();
	lf->Construct(f);
	return lf;
}

void Lua::file::find_external_game_resource_files(lua_State *l, const std::string &path, luabind::object &outFiles, luabind::object &outDirs)
{
	outFiles = luabind::newtable(l);
	outDirs = luabind::newtable(l);
	auto dllHandle = util::initialize_external_archive_manager(engine->GetNetworkState(l));
	if(dllHandle == nullptr)
		return;
	auto *fFindFiles = dllHandle->FindSymbolAddress<void (*)(const std::string &, std::vector<std::string> *, std::vector<std::string> *)>("find_files");
	if(fFindFiles == nullptr)
		return;
	std::vector<std::string> files {};
	std::vector<std::string> directories {};
	fFindFiles(path, &files, &directories);

	uint32_t idx = 1;
	for(auto &f : files)
		outFiles[idx++] = f;

	idx = 1;
	for(auto &d : directories)
		outDirs[idx++] = d;
}

luabind::object Lua::file::FindLuaFiles(lua_State *l, const std::string &path, fsys::SearchFlags searchFlag)
{
	std::vector<std::string> files;
	FileManager::FindFiles((path + "/*." + Lua::FILE_EXTENSION).c_str(), &files, nullptr, searchFlag);
	FileManager::FindFiles((path + "/*." + Lua::FILE_EXTENSION_PRECOMPILED).c_str(), &files, nullptr, searchFlag);

	auto t = luabind::newtable(l);
	auto idx = 1;
	for(auto &f : files) {
		ufile::remove_extension_from_filename(f);
		f += Lua::DOT_FILE_EXTENSION;
		t[idx++] = f;
	}
	return t;
}

void Lua::file::Find(lua_State *l, const std::string &path, fsys::SearchFlags searchFlags, luabind::object &outFiles, luabind::object &outDirs)
{
	std::vector<std::string> files;
	std::vector<std::string> dirs;
	FileManager::FindFiles(path.c_str(), &files, &dirs, searchFlags);

	outFiles = luabind::newtable(l);
	uint32_t idx = 1;
	for(auto &f : files)
		outFiles[idx++] = f;

	outDirs = luabind::newtable(l);
	idx = 1;
	for(auto &d : dirs)
		outDirs[idx++] = d;
}

luabind::object Lua::file::Read(lua_State *l, const std::string &path)
{
	auto f = FileManager::OpenFile(path.c_str(), "rb");
	if(f == NULL)
		return {};
	std::string str = f->ReadString();
	return luabind::object {l, str};
}

bool Lua::file::Write(lua_State *l, std::string strPath, const std::string &content)
{
	strPath = FileManager::GetCanonicalizedPath(strPath);
	if(validate_write_operation(l, strPath) == false)
		return false;
	auto fullPath = util::Path::CreateFile(strPath);

	auto path = std::string {fullPath.GetPath()};
	FileManager::CreatePath(path.data());

	auto f = FileManager::OpenFile(fullPath.GetString().c_str(), "w");
	if(f == NULL || f->GetType() != VFILE_LOCAL)
		return false;
	auto freal = std::static_pointer_cast<VFilePtrInternalReal>(f);
	freal->Write(content.c_str(), content.length());
	return true;
}

std::string Lua::file::GetCanonicalizedPath(const std::string &path)
{
	auto r = FileManager::GetCanonicalizedPath(path);
	ustring::replace(r, "\\", "/");
	return r;
}

luabind::object Lua::file::GetFileExtension(lua_State *l, const std::string &path, const std::vector<std::string> &exts)
{
	std::string ext;
	auto r = ufile::get_extension(path, &ext);
	if(r == false)
		return {};
	for(auto &extOther : exts) {
		if(ext == extOther)
			return luabind::object {l, ext};
	}
	return {};
}

luabind::object Lua::file::GetFileExtension(lua_State *l, const std::string &path)
{
	std::string ext;
	auto r = ufile::get_extension(path, &ext);
	if(r == false)
		return {};
	return luabind::object {l, ext};
}
bool Lua::file::ComparePath(const std::string &path0, const std::string &path1) { return FileManager::GetCanonicalizedPath(path0) == FileManager::GetCanonicalizedPath(path1); }
