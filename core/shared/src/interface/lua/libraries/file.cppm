// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:scripting.lua.libraries.file;

export import pragma.lua;

export {
	class DLLNETWORK LFile {
	  public:
		LFile();
		~LFile();
	  private:
		std::shared_ptr<ufile::IFile> m_file = nullptr;
	  public:
		void Construct(const pragma::fs::VFilePtr &f);
		void Construct(const std::shared_ptr<ufile::IFile> &f);
		bool Construct(const char *path, pragma::fs::FileMode fileMode, pragma::fs::SearchFlags fsearchmode = pragma::fs::SearchFlags::All, std::string *optOutErr = nullptr);
		bool IsValid() const;
		std::shared_ptr<ufile::IFile> GetHandle();
		void Close();
		unsigned long long Size();
		long long Tell();
		void Seek(unsigned long long pos);
		bool Read(void *buf, unsigned long long l);
		void Write(void *c, unsigned long long l);
		template<class T>
		void Write(T t)
		{
			Write(static_cast<void *>(&t), sizeof(T));
		}
		template<class T>
		T Read()
		{
			char buf[sizeof(T)];
			Read(buf, sizeof(T));
			return (*(T *)&(buf[0]));
		}
		void WriteString(std::string str);
		std::string ReadLine();
		std::string ReadString();
		bool Eof();
		void IgnoreComments(std::string start = "//", std::string end = "\n");
	};

#define lua_lfile_datatype(datatype, suffix, luapush)                                                                                                                                                                                                                                            \
	inline DLLNETWORK void Lua_LFile_Write##suffix(lua::State *, LFile &f, datatype d) { f.Write<datatype>(d); }                                                                                                                                                                                 \
	inline DLLNETWORK void Lua_LFile_Read##suffix(lua::State *l, LFile &f) { luapush(l, f.Read<datatype>()); }

	DLLNETWORK void Lua_LFile_Close(lua::State *l, LFile &f);
	DLLNETWORK void Lua_LFile_Size(lua::State *l, LFile &f);
	DLLNETWORK void Lua_LFile_ReadLine(lua::State *l, LFile &f);

	lua_lfile_datatype(int8_t, Int8, Lua::PushInt);
	lua_lfile_datatype(uint8_t, UInt8, Lua::PushInt);
	lua_lfile_datatype(int32_t, Int32, Lua::PushInt);
	lua_lfile_datatype(uint32_t, UInt32, Lua::PushInt);
	lua_lfile_datatype(int16_t, Int16, Lua::PushInt);
	lua_lfile_datatype(uint16_t, UInt16, Lua::PushInt);
	inline DLLNETWORK void Lua_LFile_WriteInt64(lua::State *, LFile &f, int32_t d) { f.Write<int64_t>(d); }
	inline DLLNETWORK void Lua_LFile_ReadInt64(lua::State *l, LFile &f) { Lua::PushInt(l, f.Read<int64_t>()); }
	inline DLLNETWORK void Lua_LFile_WriteUInt64(lua::State *, LFile &f, uint32_t d) { f.Write<int64_t>(d); }
	inline DLLNETWORK void Lua_LFile_ReadUInt64(lua::State *l, LFile &f) { Lua::PushInt(l, f.Read<uint64_t>()); }
	lua_lfile_datatype(bool, Bool, Lua::PushBool);
	inline DLLNETWORK void Lua_LFile_WriteChar(lua::State *, LFile &f, const std::string &d) { f.Write<char>(d.front()); }
	inline DLLNETWORK void Lua_LFile_ReadChar(lua::State *l, LFile &f) { Lua::PushString(l, std::string(1, f.Read<char>())); }
	lua_lfile_datatype(float, Float, Lua::PushNumber);
	lua_lfile_datatype(double, Double, Lua::PushNumber);
	lua_lfile_datatype(long double, LongDouble, Lua::PushNumber);

	DLLNETWORK void Lua_LFile_ReadString(lua::State *l, LFile &f, uint32_t len);
	DLLNETWORK void Lua_LFile_ReadString(lua::State *l, LFile &f);
	DLLNETWORK void Lua_LFile_WriteString(lua::State *l, LFile &f, std::string str, bool bNullTerminated);
	DLLNETWORK void Lua_LFile_WriteString(lua::State *l, LFile &f, std::string str);
	DLLNETWORK void Lua_LFile_ReadVector(lua::State *l, LFile &f);
	DLLNETWORK void Lua_LFile_WriteVector(lua::State *l, LFile &f, const Vector3 &v);
	DLLNETWORK void Lua_LFile_ReadVector2(lua::State *l, LFile &f);
	DLLNETWORK void Lua_LFile_WriteVector2(lua::State *l, LFile &f, const Vector2 &v);
	DLLNETWORK void Lua_LFile_ReadVector4(lua::State *l, LFile &f);
	DLLNETWORK void Lua_LFile_WriteVector4(lua::State *l, LFile &f, const Vector4 &v);
	DLLNETWORK void Lua_LFile_ReadAngles(lua::State *l, LFile &f);
	DLLNETWORK void Lua_LFile_WriteAngles(lua::State *l, LFile &f, EulerAngles ang);
	DLLNETWORK void Lua_LFile_Seek(lua::State *l, LFile &f, uint32_t pos);
	DLLNETWORK void Lua_LFile_Tell(lua::State *l, LFile &f);
	DLLNETWORK void Lua_LFile_Eof(lua::State *l, LFile &f);
	DLLNETWORK void Lua_LFile_IgnoreComments(lua::State *l, LFile &f);
	DLLNETWORK void Lua_LFile_IgnoreComments(lua::State *l, LFile &f, std::string start);
	DLLNETWORK void Lua_LFile_IgnoreComments(lua::State *l, LFile &f, std::string start, std::string end);
	DLLNETWORK void Lua_LFile_Read(lua::State *l, LFile &f, uint32_t size);
	DLLNETWORK void Lua_LFile_Read(lua::State *l, LFile &f, pragma::util::DataStream &ds, uint32_t size);
	DLLNETWORK void Lua_LFile_Write(lua::State *l, LFile &f, pragma::util::DataStream &ds);
	DLLNETWORK void Lua_LFile_Write(lua::State *l, LFile &f, pragma::util::DataStream &ds, uint32_t size);
	DLLNETWORK void Lua_LFile_GetPath(lua::State *l, LFile &f);

	////////////////////////////////////

	namespace Lua {
		namespace file {
			DLLNETWORK bool validate_write_operation(lua::State *l, std::string &path);
			DLLNETWORK bool validate_write_operation(lua::State *l, std::string &path, std::string &outRootPath);
			DLLNETWORK std::string to_relative_path(const std::string &path);

			DLLNETWORK std::pair<std::shared_ptr<LFile>, std::optional<std::string>> Open(lua::State *l, std::string path, pragma::fs::FileMode openMode, pragma::fs::SearchFlags searchFlags = pragma::fs::SearchFlags::All);
			DLLNETWORK bool CreateDir(lua::State *l, std::string path);
			DLLNETWORK bool CreatePath(lua::State *l, std::string path);
			DLLNETWORK bool Delete(lua::State *l, std::string path);
			DLLNETWORK bool DeleteDir(lua::State *l, std::string path);
			DLLNETWORK void Find(lua::State *l, const std::string &path, pragma::fs::SearchFlags searchFlags, luabind::object &outFiles, luabind::object &outDirs);
			DLLNETWORK luabind::object FindLuaFiles(lua::State *l, const std::string &path, pragma::fs::SearchFlags searchFlags = pragma::fs::SearchFlags::All);
			DLLNETWORK void find_external_game_resource_files(lua::State *l, const std::string &path, luabind::object &outFiles, luabind::object &outDirs);
			DLLNETWORK std::shared_ptr<LFile> open_external_asset_file(lua::State *l, const std::string &path, const std::optional<std::string> &game = {});
			DLLNETWORK luabind::object Read(lua::State *l, const std::string &path);
			DLLNETWORK bool Write(lua::State *l, std::string path, const std::string &content);
			DLLNETWORK std::string GetCanonicalizedPath(const std::string &path);
			DLLNETWORK luabind::object GetFileExtension(lua::State *l, const std::string &path);
			DLLNETWORK luabind::object GetFileExtension(lua::State *l, const std::string &path, const std::vector<std::string> &exts);
			DLLNETWORK bool ComparePath(const std::string &path0, const std::string &path1);
		};
	};
};
