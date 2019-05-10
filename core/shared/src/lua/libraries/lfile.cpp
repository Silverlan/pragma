#include "stdafx_shared.h"
#include "pragma/lua/libraries/lfile.h"
#include "luasystem.h"
#include <sharedutils/util_file.h>
#include <sharedutils/util_library.hpp>

extern DLLENGINE Engine *engine;

LFile::LFile()
{}

void LFile::Construct(const VFilePtr &f) {m_file = f;}

bool LFile::Construct(const char *path,const char *mode,fsys::SearchFlags fsearchmode)
{
	m_file = FileManager::OpenFile(path,mode,fsearchmode);
	return (m_file != nullptr) ? true : false;
}

LFile::~LFile() {Close();}

VFilePtr LFile::GetHandle()
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

bool LFile::IsValid() const {return (m_file != NULL) ? true : false;}

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

bool LFile::Read(void *buf,unsigned long long l)
{
	if(!IsValid())
		return false;
	m_file->Read(buf,l);
	return true;
}

void LFile::Write(void *c,unsigned long long l)
{
	if(!IsValid())
		return;
	if(m_file->GetType() != VFILE_LOCAL)
		return;
	auto freal = std::static_pointer_cast<VFilePtrInternalReal>(m_file);
	freal->Write(c,l);
}

void LFile::WriteString(std::string str)
{
	if(!IsValid())
		return;
	if(m_file->GetType() != VFILE_LOCAL)
		return;
	auto freal = std::static_pointer_cast<VFilePtrInternalReal>(m_file);
	freal->WriteString(str);
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

void LFile::IgnoreComments(std::string start,std::string end)
{
	if(!IsValid())
		return;
	m_file->IgnoreComments(start,end);
}

////////////////////////////////////

DLLNETWORK void Lua_LFile_Close(lua_State*,LFile &f) {f.Close();}

DLLNETWORK void Lua_LFile_Size(lua_State *l,LFile &f)
{
	long long size = f.Size();
	Lua::PushInt<long long>(l,size);
}

DLLNETWORK void Lua_LFile_ReadLine(lua_State *l,LFile &f)
{
	Lua::PushString(l,f.ReadLine());
}

DLLNETWORK void Lua_LFile_ReadString(lua_State *l,LFile &f,uint32_t len)
{
	if(!f.IsValid())
		return;
	std::string out;
	out.resize(len);
	f.Read(&out[0],len);
	Lua::PushString(l,out);

}

DLLNETWORK void Lua_LFile_ReadString(lua_State *l,LFile &f)
{
	if(!f.IsValid())
		return;
	auto file = f.GetHandle();
	std::string str = file->ReadString();
	lua_pushstring(l,str.c_str());
}

DLLNETWORK void Lua_LFile_WriteString(lua_State*,LFile &f,std::string str,bool bNullTerminated)
{
	if(!f.IsValid())
		return;
	auto file = f.GetHandle();
	if(file->GetType() != VFILE_LOCAL)
		return;
	auto freal = std::static_pointer_cast<VFilePtrInternalReal>(file);
	if(bNullTerminated == true)
		freal->WriteString(str);
	else
		freal->Write(str.data(),str.length());
}

DLLNETWORK void Lua_LFile_WriteString(lua_State *l,LFile &f,std::string str)
{
	Lua_LFile_WriteString(l,f,str,true);
}

void Lua_LFile_ReadVector(lua_State *l,LFile &f)
{
	float x = f.Read<float>();
	float y = f.Read<float>();
	float z = f.Read<float>();
	luabind::object(l,Vector3(x,y,z)).push(l);
}

void Lua_LFile_WriteVector2(lua_State*,LFile &f,const Vector2 &v)
{
	f.Write<float>(v.x);
	f.Write<float>(v.y);
}

void Lua_LFile_ReadVector2(lua_State *l,LFile &f)
{
	auto x = f.Read<float>();
	auto y = f.Read<float>();
	luabind::object(l,Vector2(x,y)).push(l);
}

void Lua_LFile_WriteVector4(lua_State*,LFile &f,const Vector4 &v)
{
	f.Write<float>(v.x);
	f.Write<float>(v.y);
	f.Write<float>(v.z);
	f.Write<float>(v.w);
}

void Lua_LFile_ReadVector4(lua_State *l,LFile &f)
{
	auto x = f.Read<float>();
	auto y = f.Read<float>();
	auto z = f.Read<float>();
	auto w = f.Read<float>();
	luabind::object(l,Vector4(x,y,z,w)).push(l);
}

void Lua_LFile_WriteVector(lua_State*,LFile &f,const Vector3 &v)
{
	f.Write<float>(v.x);
	f.Write<float>(v.y);
	f.Write<float>(v.z);
}

DLLNETWORK void Lua_LFile_ReadAngles(lua_State *l,LFile &f)
{
	float p = f.Read<float>();
	float y = f.Read<float>();
	float r = f.Read<float>();
	luabind::object(l,EulerAngles(p,y,r)).push(l);
}

DLLNETWORK void Lua_LFile_WriteAngles(lua_State*,LFile &f,EulerAngles ang)
{
	f.Write<float>(ang.p);
	f.Write<float>(ang.y);
	f.Write<float>(ang.r);
}

DLLNETWORK void Lua_LFile_Seek(lua_State*,LFile &f,uint32_t pos) {f.Seek(pos);}

DLLNETWORK void Lua_LFile_Tell(lua_State *l,LFile &f)
{
	long long pos = f.Tell();
	Lua::PushNumber<long long>(l,pos);
}

DLLNETWORK void Lua_LFile_Eof(lua_State *l,LFile &f)
{
	Lua::PushBool(l,f.Eof());
}

DLLNETWORK void Lua_LFile_IgnoreComments(lua_State*,LFile &f)
{
	f.IgnoreComments();
}
DLLNETWORK void Lua_LFile_IgnoreComments(lua_State*,LFile &f,std::string start)
{
	f.IgnoreComments(start);
}
DLLNETWORK void Lua_LFile_IgnoreComments(lua_State*,LFile &f,std::string start,std::string end)
{
	f.IgnoreComments(start,end);
}
void Lua_LFile_Read(lua_State *l,LFile &f,uint32_t size)
{
	DataStream ds;
	ds->Resize(size);
	f.Read(ds->GetData(),size);
	Lua::Push<DataStream>(l,ds);
}
void Lua_LFile_Read(lua_State*,LFile &f,::DataStream &ds,uint32_t size)
{
	auto offset = ds->GetOffset();
	ds->Resize(offset +size);
	f.Read(ds->GetData() +offset,size);
}
void Lua_LFile_Write(lua_State*,LFile &f,::DataStream &ds)
{
	auto offset = ds->GetOffset();
	f.Write(ds->GetData() +offset,ds->GetInternalSize() -offset);
}
void Lua_LFile_Write(lua_State*,LFile &f,::DataStream &ds,uint32_t size)
{
	auto offset = ds->GetOffset();
	f.Write(ds->GetData() +offset,size -offset);
}

////////////////////////////////////

// Also used in wv_sqlite module
bool Lua::file::validate_write_operation(lua_State *l,std::string &path)
{
	auto fname = FileManager::GetCanonicalizedPath(Lua::get_current_file(l));
	if(fname.length() < 8 || ustring::compare(fname.c_str(),"addons\\",false,7) == false)
	{
		Con::cwar<<"WARNING: File write-operations can only be performed by Lua-scripts inside an addon!"<<Con::endl;
		return false;
	}
	auto br = fname.find(FileManager::GetDirectorySeparator(),8);
	auto prefix = ustring::sub(fname,0,br +1);
	path = prefix +path;
	return true;
}

int Lua::file::Open(lua_State *l)
{
	std::string path = luaL_checkstring(l,1);
	auto openMode = static_cast<FileOpenMode>(Lua::CheckInt(l,2));
	std::string mode {};
	if((openMode &FileOpenMode::Read) != FileOpenMode::None)
		mode += "r";
	else if((openMode &FileOpenMode::Write) != FileOpenMode::None)
		mode += "w";
	else if((openMode &FileOpenMode::Append) != FileOpenMode::None)
		mode += "a";
	else
		return 0;
	if((openMode &FileOpenMode::Binary) != FileOpenMode::None)
		mode += "b";
	if((openMode &FileOpenMode::Update) != FileOpenMode::None)
		mode += "+";
	if((openMode &(FileOpenMode::Write | FileOpenMode::Append)) != FileOpenMode::None) // Write mode
	{
		if(validate_write_operation(l,path) == false)
			return 0;
	}
	fsys::SearchFlags fsearchmode;
	if(!lua_isnoneornil(l,3))
		fsearchmode = static_cast<fsys::SearchFlags>(Lua::CheckInt<unsigned int>(l,3));
	else
		fsearchmode = fsys::SearchFlags::All;
	auto f = std::make_shared<LFile>();
	if(f->Construct(path.c_str(),mode.c_str(),fsearchmode) == false)
		return 0;
	luabind::object(l,f).push(l);
	return 1;
}

int Lua::file::CreateDir(lua_State *l)
{
	std::string path = luaL_checkstring(l,1);
	if(validate_write_operation(l,path) == false)
		return 0;
	lua_pushboolean(l,FileManager::CreateDirectory(path.c_str()));
	return 1;
}

int Lua::file::CreatePath(lua_State *l)
{
	std::string path = luaL_checkstring(l,1);
	if(validate_write_operation(l,path) == false)
		return 0;
	lua_pushboolean(l,FileManager::CreatePath(path.c_str()));
	return 1;
}

int Lua::file::Exists(lua_State *l)
{
	std::string path = luaL_checkstring(l,1);
	fsys::SearchFlags fsearchmode;
	if(!lua_isnoneornil(l,2))
		fsearchmode = static_cast<fsys::SearchFlags>(Lua::CheckInt<unsigned int>(l,2));
	else
		fsearchmode = fsys::SearchFlags::All;
	lua_pushboolean(l,FileManager::Exists(path.c_str(),fsearchmode));
	return 1;
}

int Lua::file::Delete(lua_State *l)
{
	std::string path = luaL_checkstring(l,1);
	if(validate_write_operation(l,path) == false)
		return 0;
	lua_pushboolean(l,FileManager::RemoveFile(path.c_str()));
	return 1;
}

int Lua::file::IsDir(lua_State *l)
{
	std::string path = luaL_checkstring(l,1);
	fsys::SearchFlags fsearchmode;
	if(!lua_isnoneornil(l,2))
		fsearchmode = static_cast<fsys::SearchFlags>(Lua::CheckInt<unsigned int>(l,2));
	else
		fsearchmode = fsys::SearchFlags::All;
	lua_pushboolean(l,FileManager::IsDir(path.c_str(),fsearchmode));
	return 1;
}

int Lua::file::GetAttributes(lua_State *l)
{
	std::string path = luaL_checkstring(l,1);
	Lua::PushInt<unsigned long long>(l,FileManager::GetFileAttributes(path.c_str()));
	return 1;
}

int Lua::file::GetFlags(lua_State *l)
{
	std::string path = luaL_checkstring(l,1);
	fsys::SearchFlags fsearchmode;
	if(!lua_isnoneornil(l,2))
		fsearchmode = static_cast<fsys::SearchFlags>(Lua::CheckInt<unsigned int>(l,2));
	else
		fsearchmode = fsys::SearchFlags::All;
	Lua::PushInt<unsigned long long>(l,FileManager::GetFileFlags(path.c_str(),fsearchmode));
	return 1;
}

int32_t Lua::file::find_external_game_resource_files(lua_State *l)
{
	std::string path = Lua::CheckString(l,1);
	auto dllHandle = engine->GetNetworkState(l)->LoadLibraryModule("mount_external/pr_mount_external");
	if(dllHandle == nullptr)
	{
		Lua::CreateTable(l);
		Lua::CreateTable(l);
		return 2;
	}
	auto *fFindFiles = dllHandle->FindSymbolAddress<void(*)(const std::string&,std::vector<std::string>*,std::vector<std::string>*)>("find_files");
	if(fFindFiles == nullptr)
	{
		Lua::CreateTable(l);
		Lua::CreateTable(l);
		return 2;
	}
	std::vector<std::string> files {};
	std::vector<std::string> directories {};
	fFindFiles(path,&files,&directories);

	auto tFiles = Lua::CreateTable(l);
	auto idx = 1;
	for(auto &fName : files)
	{
		Lua::PushInt(l,idx++);
		Lua::PushString(l,fName);
		Lua::SetTableValue(l,tFiles);
	}

	auto tDirs = Lua::CreateTable(l);
	idx = 1;
	for(auto &fDir : directories)
	{
		Lua::PushInt(l,idx++);
		Lua::PushString(l,fDir);
		Lua::SetTableValue(l,tDirs);
	}
	return 2;
}

int Lua::file::FindLuaFiles(lua_State *l)
{
	std::string path = Lua::CheckString(l,1);
	fsys::SearchFlags fsearchmode;
	if(Lua::IsSet(l,2) == true)
		fsearchmode = static_cast<fsys::SearchFlags>(Lua::CheckInt<unsigned int>(l,2));
	else
		fsearchmode = fsys::SearchFlags::All;
	std::vector<std::string> files;
	FileManager::FindFiles((path +"/*.lua").c_str(),&files,nullptr,fsearchmode);
	FileManager::FindFiles((path +"/*.clua").c_str(),&files,nullptr,fsearchmode);

	auto t = Lua::CreateTable(l);
	auto idx = 1;
	for(auto &f : files)
	{
		ufile::remove_extension_from_filename(f);
		f += ".lua";
		Lua::PushInt(l,idx++);
		Lua::PushString(l,f);
		Lua::SetTableValue(l,t);
	}
	return 1;
}

int Lua::file::Find(lua_State *l)
{
	std::string path = luaL_checkstring(l,1);
	fsys::SearchFlags fsearchmode;
	if(!lua_isnoneornil(l,2))
		fsearchmode = static_cast<fsys::SearchFlags>(Lua::CheckInt<unsigned int>(l,2));
	else
		fsearchmode = fsys::SearchFlags::All;
	std::vector<std::string> files;
	std::vector<std::string> dirs;
	FileManager::FindFiles(path.c_str(),&files,&dirs,fsearchmode);

	lua_newtable(l);
	int topFiles = lua_gettop(l);
	lua_newtable(l);
	int topDirs = lua_gettop(l);
	for(unsigned int i=0;i<files.size();i++)
	{
		lua_pushstring(l,files[i].c_str());
		lua_rawseti(l,topFiles,i +1);
	}

	for(unsigned int i=0;i<dirs.size();i++)
	{
		lua_pushstring(l,dirs[i].c_str());
		lua_rawseti(l,topDirs,i +1);
	}
	return 2;
}

int Lua::file::Read(lua_State *l)
{
	std::string path = luaL_checkstring(l,1);
	fsys::SearchFlags fsearchmode;
	if(!lua_isnoneornil(l,2))
		fsearchmode = static_cast<fsys::SearchFlags>(Lua::CheckInt<unsigned int>(l,2));
	else
		fsearchmode = fsys::SearchFlags::All;
	auto f = FileManager::OpenFile(path.c_str(),"rb");
	if(f == NULL)
		return 0;
	std::string str = f->ReadString();
	lua_pushstring(l,str.c_str());
	return 1;
}

int Lua::file::Write(lua_State *l)
{
	std::string path = FileManager::GetCanonicalizedPath(luaL_checkstring(l,1));
	if(validate_write_operation(l,path) == false)
		return 0;
	std::string content = luaL_checkstring(l,2);
	auto f = FileManager::OpenFile(path.c_str(),"w");
	if(f == NULL || f->GetType() != VFILE_LOCAL)
	{
		Lua::PushBool(l,false);
		return 1;
	}
	auto freal = std::static_pointer_cast<VFilePtrInternalReal>(f);
	freal->Write(content.c_str(),content.length());
	Lua::PushBool(l,true);
	return 1;
}

int Lua::file::GetCanonicalizedPath(lua_State *l)
{
	auto *path = Lua::CheckString(l,1);
	auto r = FileManager::GetCanonicalizedPath(path);
	ustring::replace(r,"\\","/");
	Lua::PushString(l,r);
	return 1;
}

int Lua::file::GetFilePath(lua_State *l)
{
	auto *path = Lua::CheckString(l,1);
	auto r = ufile::get_path_from_filename(path);
	Lua::PushString(l,r);
	return 1;
}
int Lua::file::GetFileName(lua_State *l)
{
	auto *path = Lua::CheckString(l,1);
	auto r = ufile::get_file_from_filename(path);
	Lua::PushString(l,r);
	return 1;
}
int Lua::file::GetFileExtension(lua_State *l)
{
	auto *path = Lua::CheckString(l,1);
	std::string ext;
	auto r = ufile::get_extension(path,&ext);
	if(r == false)
		return 0;
	Lua::PushString(l,ext);
	return 1;
}
int Lua::file::RemoveFileExtension(lua_State *l)
{
	std::string fpath = Lua::CheckString(l,1);
	ufile::remove_extension_from_filename(fpath);
	Lua::PushString(l,fpath);
	return 1;
}
int Lua::file::GetSize(lua_State *l)
{
	auto *path = Lua::CheckString(l,1);
	fsys::SearchFlags fsearchmode;
	if(Lua::IsSet(l,2))
		fsearchmode = static_cast<fsys::SearchFlags>(Lua::CheckInt(l,2));
	else
		fsearchmode = fsys::SearchFlags::All;
	auto size = FileManager::GetFileSize(path,fsearchmode);
	Lua::PushInt(l,size);
	return 1;
}
int Lua::file::ComparePath(lua_State *l)
{
	auto p0 = FileManager::GetCanonicalizedPath(Lua::CheckString(l,1));
	auto p1 = FileManager::GetCanonicalizedPath(Lua::CheckString(l,2));
	Lua::PushBool(l,p0 == p1);
	return 1;
}
