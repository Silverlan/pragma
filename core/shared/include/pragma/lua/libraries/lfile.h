/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __LFILE_H__
#define __LFILE_H__
#include "pragma/networkdefinitions.h"
#include <fsys/filesystem.h>
#include <pragma/lua/luaapi.h>
#include <mathutil/glmutil.h>
#include <pragma/math/angle/wvangle.h>
#include "luasystem.h"
#include "pragma/lua/ldefinitions.h"
#include <sharedutils/datastream.h>

enum class FileOpenMode : uint32_t
{
	None = 0u,
	Read = 1u,
	Write = Read<<1u,
	Append = Write<<1u,
	Update = Append<<1u,
	Binary = Update<<1u
};
REGISTER_BASIC_BITWISE_OPERATORS(FileOpenMode);

class DLLNETWORK LFile
{
public:
	LFile();
	~LFile();
private:
	VFilePtr m_file;
public:
	void Construct(const VFilePtr &f);
	bool Construct(const char *path,const char *mode,fsys::SearchFlags fsearchmode=fsys::SearchFlags::All);
	bool IsValid() const;
	VFilePtr GetHandle();
	void Close();
	unsigned long long Size();
	long long Tell();
	void Seek(unsigned long long pos);
	bool Read(void *buf,unsigned long long l);
	void Write(void *c,unsigned long long l);
	template<class T>
		void Write(T t);
	template<class T>
		T Read();
	void WriteString(std::string str);
	std::string ReadLine();
	std::string ReadString();
	bool Eof();
	void IgnoreComments(std::string start="//",std::string end="\n");
};

lua_registercheck(File,LFile);

template<class T> void LFile::Write(T t) {Write(static_cast<void*>(&t),sizeof(T));}

template<class T>
	inline T LFile::Read()
{
	char buf[sizeof(T)];
	Read(buf,sizeof(T));
	return (*(T*)&(buf[0]));
}

#define lua_lfile_datatype(datatype,suffix,luapush) \
	inline DLLNETWORK void Lua_LFile_Write##suffix(lua_State*,LFile &f,datatype d) {f.Write<datatype>(d);} \
	inline DLLNETWORK void Lua_LFile_Read##suffix(lua_State *l,LFile &f) {luapush(l,f.Read<datatype>());}

DLLNETWORK void Lua_LFile_Close(lua_State *l,LFile &f);
DLLNETWORK void Lua_LFile_Size(lua_State *l,LFile &f);
DLLNETWORK void Lua_LFile_ReadLine(lua_State *l,LFile &f);

lua_lfile_datatype(int8_t,Int8,Lua::PushInt);
lua_lfile_datatype(uint8_t,UInt8,Lua::PushInt);
lua_lfile_datatype(int32_t,Int32,Lua::PushInt);
lua_lfile_datatype(uint32_t,UInt32,Lua::PushInt);
lua_lfile_datatype(int16_t,Int16,Lua::PushInt);
lua_lfile_datatype(uint16_t,UInt16,Lua::PushInt);
inline DLLNETWORK void Lua_LFile_WriteInt64(lua_State*,LFile &f,int32_t d) {f.Write<int64_t>(d);}
inline DLLNETWORK void Lua_LFile_ReadInt64(lua_State *l,LFile &f) {Lua::PushInt(l,f.Read<int64_t>());}
inline DLLNETWORK void Lua_LFile_WriteUInt64(lua_State*,LFile &f,uint32_t d) {f.Write<int64_t>(d);}
inline DLLNETWORK void Lua_LFile_ReadUInt64(lua_State *l,LFile &f) {Lua::PushInt(l,f.Read<uint64_t>());}
lua_lfile_datatype(bool,Bool,lua_pushboolean);
inline DLLNETWORK void Lua_LFile_WriteChar(lua_State*,LFile &f,const std::string &d) {f.Write<char>(d.front());}
inline DLLNETWORK void Lua_LFile_ReadChar(lua_State *l,LFile &f) {Lua::PushString(l,std::string(1,f.Read<char>()));}
lua_lfile_datatype(float,Float,Lua::PushNumber);
lua_lfile_datatype(double,Double,Lua::PushNumber);
lua_lfile_datatype(long double,LongDouble,Lua::PushNumber);

DLLNETWORK void Lua_LFile_ReadString(lua_State *l,LFile &f,uint32_t len);
DLLNETWORK void Lua_LFile_ReadString(lua_State *l,LFile &f);
DLLNETWORK void Lua_LFile_WriteString(lua_State *l,LFile &f,std::string str,bool bNullTerminated);
DLLNETWORK void Lua_LFile_WriteString(lua_State *l,LFile &f,std::string str);
DLLNETWORK void Lua_LFile_ReadVector(lua_State *l,LFile &f);
DLLNETWORK void Lua_LFile_WriteVector(lua_State *l,LFile &f,const Vector3 &v);
DLLNETWORK void Lua_LFile_ReadVector2(lua_State *l,LFile &f);
DLLNETWORK void Lua_LFile_WriteVector2(lua_State *l,LFile &f,const Vector2 &v);
DLLNETWORK void Lua_LFile_ReadVector4(lua_State *l,LFile &f);
DLLNETWORK void Lua_LFile_WriteVector4(lua_State *l,LFile &f,const Vector4 &v);
DLLNETWORK void Lua_LFile_ReadAngles(lua_State *l,LFile &f);
DLLNETWORK void Lua_LFile_WriteAngles(lua_State *l,LFile &f,EulerAngles ang);
DLLNETWORK void Lua_LFile_Seek(lua_State *l,LFile &f,uint32_t pos);
DLLNETWORK void Lua_LFile_Tell(lua_State *l,LFile &f);
DLLNETWORK void Lua_LFile_Eof(lua_State *l,LFile &f);
DLLNETWORK void Lua_LFile_IgnoreComments(lua_State *l,LFile &f);
DLLNETWORK void Lua_LFile_IgnoreComments(lua_State *l,LFile &f,std::string start);
DLLNETWORK void Lua_LFile_IgnoreComments(lua_State *l,LFile &f,std::string start,std::string end);
DLLNETWORK void Lua_LFile_Read(lua_State *l,LFile &f,uint32_t size);
DLLNETWORK void Lua_LFile_Read(lua_State *l,LFile &f,::DataStream &ds,uint32_t size);
DLLNETWORK void Lua_LFile_Write(lua_State *l,LFile &f,::DataStream &ds);
DLLNETWORK void Lua_LFile_Write(lua_State *l,LFile &f,::DataStream &ds,uint32_t size);
DLLNETWORK void Lua_LFile_GetPath(lua_State *l,LFile &f);

////////////////////////////////////

namespace Lua
{
	namespace file
	{
		DLLNETWORK bool validate_write_operation(lua_State *l,std::string &path);
		DLLNETWORK bool validate_write_operation(lua_State *l,std::string &path,std::string &outRootPath);

		DLLNETWORK std::shared_ptr<LFile> Open(lua_State *l,std::string path,FileOpenMode openMode,fsys::SearchFlags searchFlags=fsys::SearchFlags::All);
		DLLNETWORK bool CreateDir(lua_State *l,std::string path);
		DLLNETWORK bool CreatePath(lua_State *l,std::string path);
		DLLNETWORK bool Delete(lua_State *l,std::string path);
		DLLNETWORK void Find(lua_State *l,const std::string &path,fsys::SearchFlags searchFlags,luabind::object &outFiles,luabind::object &outDirs);
		DLLNETWORK luabind::object FindLuaFiles(lua_State *l,const std::string &path,fsys::SearchFlags searchFlags=fsys::SearchFlags::All);
		DLLNETWORK void find_external_game_resource_files(lua_State *l,const std::string &path,luabind::object &outFiles,luabind::object &outDirs);
		DLLNETWORK std::shared_ptr<LFile> open_external_asset_file(lua_State *l,const std::string &path);
		DLLNETWORK luabind::object Read(lua_State *l,const std::string &path);
		DLLNETWORK bool Write(lua_State *l,std::string path,const std::string &content);
		DLLNETWORK std::string GetCanonicalizedPath(const std::string &path);
		DLLNETWORK luabind::object GetFileExtension(lua_State *l,const std::string &path);
		DLLNETWORK bool ComparePath(const std::string &path0,const std::string &path1);
		DLLNETWORK std::string RemoveFileExtension(const std::string &path);
	};
};

#endif