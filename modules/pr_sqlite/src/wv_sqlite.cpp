#include "sqlite_connection.hpp"
#include "sqlite_blob.hpp"
#include "sqlite_statement.hpp"
#include "sqlite_value.hpp"
#include <luasystem.h>
#include <luainterface.hpp>
#include <sqlite3.h>
#include <ishared.hpp>
#include <util/functioncallback.h>
#include <filesystem.h>
#include <util/util_string.h>
#include <iostream>
#include <pragma_module.hpp>

#pragma comment(lib,"lua51.lib")
#pragma comment(lib,"luasystem.lib")
#pragma comment(lib,"luasystem.lib")
#pragma comment(lib,"luabind.lib")
#pragma comment(lib,"sharedutils.lib")
#pragma comment(lib,"mathutil.lib")
#pragma comment(lib,"ishared.lib")
#pragma comment(lib,"vfilesystem.lib")

namespace Lua
{
	namespace sqlite
	{
		void register_lua_library(Lua::Interface &l);
		int32_t open(lua_State *l);
		int32_t result_to_string(lua_State *l);

		namespace Connection
		{
			void Close(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection);
			void PrepareStatement(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &statement);
			void PrepareStatement(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &statement,const luabind::object &oAsyncCallback);
			void OpenBlob(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &tableName,const std::string &columnName,int64_t rowId,bool bReadOnly);
			void OpenBlob(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &tableName,const std::string &columnName,int64_t rowId);
			void OpenBlob(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &dbName,const std::string &tableName,const std::string &columnName,int64_t rowId,bool bReadOnly);
			void OpenBlob(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &dbName,const std::string &tableName,const std::string &columnName,int64_t rowId);
			void Exec(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &sql,const luabind::object &oCallback);
			void Exec(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &sql);
			void GetResult(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection);
			void GetResultMessage(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection);
			void IsDatabaseReadOnly(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &dbName);
			void DatabaseExists(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &dbName);

			void CreateFunction(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &funcName,int32_t argCount,bool bDeterministic,const luabind::object &oXFunc);
			void CreateFunction(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &funcName,int32_t argCount,bool bDeterministic,const luabind::object &oXStep,const luabind::object &oXFinal);
			void DeleteFunction(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &funcName,int32_t argCount,bool bDeterministic);
		};

		namespace Statement
		{
			void Finalize(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement);
			void Step(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement);
			void Step(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,const luabind::object &oAsynCallback);
			void Reset(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement);
			void GetType(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column);
			void GetBytes(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column);
			void GetDouble(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column);
			void GetInt(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column);
			void GetInt64(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column);
			void GetText(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column);
			void GetBlob(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column);
			void GetBlob(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column,const luabind::object &oAsyncCallback);
			void GetCount(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement);
			void GetColumnName(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column);

			void BindDouble(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column,double d);
			void BindInt(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column,int32_t i);
			void BindInt64(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column,int64_t i);
			void BindNull(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column);
			void BindText(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column,const std::string &text);
			void BindBlob(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column,DataStream &ds);
			void ClearBindings(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement);
			void IsReadOnly(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement);
			void GetSQLText(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement);
			void GetExpandedSQLText(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement);
			void GetDatabaseName(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column);
			void GetTableName(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column);
			void GetOriginName(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column);
		};

		namespace Blob
		{
			void Close(lua_State *l,std::shared_ptr<::sqlite::Blob> &blob);
			void GetBytes(lua_State *l,std::shared_ptr<::sqlite::Blob> &blob);
			void Read(lua_State *l,std::shared_ptr<::sqlite::Blob> &blob,int32_t size,int32_t offset);
			void Read(lua_State *l,std::shared_ptr<::sqlite::Blob> &blob,int32_t size,int32_t offset,const luabind::object &oAsyncCallback);
			void Write(lua_State *l,std::shared_ptr<::sqlite::Blob> &blob,DataStream &ds,int32_t size,int32_t offset);
			void Write(lua_State *l,std::shared_ptr<::sqlite::Blob> &blob,DataStream &ds,int32_t size,int32_t offset,const luabind::object &oAsyncCallback);
			void Reopen(lua_State *l,std::shared_ptr<::sqlite::Blob> &blob,int64_t rowId);
		};

		namespace Value
		{
			void GetType(lua_State *l,::sqlite::Value &value);
			void GetNumericType(lua_State *l,::sqlite::Value &value);
			void GetBytes(lua_State *l,::sqlite::Value &value);
			void GetDouble(lua_State *l,::sqlite::Value &value);
			void GetInt(lua_State *l,::sqlite::Value &value);
			void GetInt64(lua_State *l,::sqlite::Value &value);
			void GetText(lua_State *l,::sqlite::Value &value);
			void GetBlob(lua_State *l,::sqlite::Value &value);
		};
	};
};

extern "C"
{
	void PRAGMA_EXPORT pragma_initialize_lua(Lua::Interface &l)
	{
		if(l.GetIdentifier() == "gui")
			return;
		Lua::sqlite::register_lua_library(l);
	}
};

void Lua::sqlite::register_lua_library(Lua::Interface &l)
{
	Lua::RegisterLibrary(l.GetState(),"sqlite",{
		{"open",Lua::sqlite::open},
		{"result_to_string",Lua::sqlite::result_to_string}
	});

	Lua::RegisterLibraryEnums(l.GetState(),"sqlite",{
		{"FOPEN_READONLY",SQLITE_OPEN_READONLY},
		{"FOPEN_READWRITE",SQLITE_OPEN_READWRITE},
		{"FOPEN_CREATE",SQLITE_OPEN_CREATE},
		{"FOPEN_DELETEONCLOSE",SQLITE_OPEN_DELETEONCLOSE},
		{"FOPEN_EXCLUSIVE",SQLITE_OPEN_EXCLUSIVE},
		{"FOPEN_AUTOPROXY",SQLITE_OPEN_AUTOPROXY},
		{"FOPEN_URI",SQLITE_OPEN_URI},
		{"FOPEN_MEMORY",SQLITE_OPEN_MEMORY},
		{"FOPEN_MAIN_DB",SQLITE_OPEN_MAIN_DB},
		{"FOPEN_TEMP_DB",SQLITE_OPEN_TEMP_DB},
		{"FOPEN_TRANSIENT_DB",SQLITE_OPEN_TRANSIENT_DB},
		{"FOPEN_MAIN_JOURNAL",SQLITE_OPEN_MAIN_JOURNAL},
		{"FOPEN_TEMP_JOURNAL",SQLITE_OPEN_TEMP_JOURNAL},
		{"FOPEN_SUBJOURNAL",SQLITE_OPEN_SUBJOURNAL},
		{"FOPEN_MASTER_JOURNAL",SQLITE_OPEN_MASTER_JOURNAL},
		{"FOPEN_NOMUTEX",SQLITE_OPEN_NOMUTEX},
		{"FOPEN_FULLMUTEX",SQLITE_OPEN_FULLMUTEX},
		{"FOPEN_SHAREDCACHE",SQLITE_OPEN_SHAREDCACHE},
		{"FOPEN_PRIVATECACHE",SQLITE_OPEN_PRIVATECACHE},
		{"FOPEN_WAL",SQLITE_OPEN_WAL},

		{"RESULT_OK",SQLITE_OK},
		{"RESULT_ERROR",SQLITE_ERROR},
		{"RESULT_INTERNAL",SQLITE_INTERNAL},
		{"RESULT_PERM",SQLITE_PERM},
		{"RESULT_ABORT",SQLITE_ABORT},
		{"RESULT_BUSY",SQLITE_BUSY},
		{"RESULT_LOCKED",SQLITE_LOCKED},
		{"RESULT_NOMEM",SQLITE_NOMEM},
		{"RESULT_READONLY",SQLITE_READONLY},
		{"RESULT_INTERRUPT",SQLITE_INTERRUPT},
		{"RESULT_IOERR",SQLITE_IOERR},
		{"RESULT_CORRUPT",SQLITE_CORRUPT},
		{"RESULT_NOTFOUND",SQLITE_NOTFOUND},
		{"RESULT_FULL",SQLITE_FULL},
		{"RESULT_CANTOPEN",SQLITE_CANTOPEN},
		{"RESULT_PROTOCOL",SQLITE_PROTOCOL},
		{"RESULT_EMPTY",SQLITE_EMPTY},
		{"RESULT_SCHEMA",SQLITE_SCHEMA},
		{"RESULT_TOOBIG",SQLITE_TOOBIG},
		{"RESULT_CONSTRAINT",SQLITE_CONSTRAINT},
		{"RESULT_MISMATCH",SQLITE_MISMATCH},
		{"RESULT_MISUSE",SQLITE_MISUSE},
		{"RESULT_NOLFS",SQLITE_NOLFS},
		{"RESULT_AUTH",SQLITE_AUTH},
		{"RESULT_FORMAT",SQLITE_FORMAT},
		{"RESULT_RANGE",SQLITE_RANGE},
		{"RESULT_NOTADB",SQLITE_NOTADB},
		{"RESULT_NOTICE",SQLITE_NOTICE},
		{"RESULT_WARNING",SQLITE_WARNING},
		{"RESULT_ROW",SQLITE_ROW},
		{"RESULT_DONE",SQLITE_DONE},

		{"RESULT_IOERR_READ",SQLITE_IOERR_READ},
		{"RESULT_IOERR_SHORT_READ",SQLITE_IOERR_SHORT_READ},
		{"RESULT_IOERR_WRITE",SQLITE_IOERR_WRITE},
		{"RESULT_IOERR_FSYNC",SQLITE_IOERR_FSYNC},
		{"RESULT_IOERR_DIR_FSYNC",SQLITE_IOERR_DIR_FSYNC},
		{"RESULT_IOERR_TRUNCATE",SQLITE_IOERR_TRUNCATE},
		{"RESULT_IOERR_FSTAT",SQLITE_IOERR_FSTAT},
		{"RESULT_IOERR_UNLOCK",SQLITE_IOERR_UNLOCK},
		{"RESULT_IOERR_RDLOCK",SQLITE_IOERR_RDLOCK},
		{"RESULT_IOERR_DELETE",SQLITE_IOERR_DELETE},
		{"RESULT_IOERR_BLOCKED",SQLITE_IOERR_BLOCKED},
		{"RESULT_IOERR_NOMEM",SQLITE_IOERR_NOMEM},
		{"RESULT_IOERR_ACCESS",SQLITE_IOERR_ACCESS},
		{"RESULT_IOERR_CHECKRESERVEDLOCK",SQLITE_IOERR_CHECKRESERVEDLOCK},
		{"RESULT_IOERR_LOCK",SQLITE_IOERR_LOCK},
		{"RESULT_IOERR_CLOSE",SQLITE_IOERR_CLOSE},
		{"RESULT_IOERR_DIR_CLOSE",SQLITE_IOERR_DIR_CLOSE},
		{"RESULT_IOERR_SHMOPEN",SQLITE_IOERR_SHMOPEN},
		{"RESULT_IOERR_SHMSIZE",SQLITE_IOERR_SHMSIZE},
		{"RESULT_IOERR_SHMLOCK",SQLITE_IOERR_SHMLOCK},
		{"RESULT_IOERR_SHMMAP",SQLITE_IOERR_SHMMAP},
		{"RESULT_IOERR_SEEK",SQLITE_IOERR_SEEK},
		{"RESULT_IOERR_DELETE_NOENT",SQLITE_IOERR_DELETE_NOENT},
		{"RESULT_IOERR_MMAP",SQLITE_IOERR_MMAP},
		{"RESULT_IOERR_GETTEMPPATH",SQLITE_IOERR_GETTEMPPATH},
		{"RESULT_IOERR_CONVPATH",SQLITE_IOERR_CONVPATH},
		{"RESULT_IOERR_VNODE",SQLITE_IOERR_VNODE},
		{"RESULT_IOERR_AUTH",SQLITE_IOERR_AUTH},
		{"RESULT_LOCKED_SHAREDCACHE",SQLITE_LOCKED_SHAREDCACHE},
		{"RESULT_BUSY_RECOVERY",SQLITE_BUSY_RECOVERY},

		{"RESULT_BUSY_SNAPSHOT",SQLITE_BUSY_SNAPSHOT},
		{"RESULT_CANTOPEN_NOTEMPDIR",SQLITE_CANTOPEN_NOTEMPDIR},
		{"RESULT_CANTOPEN_ISDIR",SQLITE_CANTOPEN_ISDIR},
		{"RESULT_CANTOPEN_FULLPATH",SQLITE_CANTOPEN_FULLPATH},
		{"RESULT_CANTOPEN_CONVPATH",SQLITE_CANTOPEN_CONVPATH},
		{"RESULT_CORRUPT_VTAB",SQLITE_CORRUPT_VTAB},
		{"RESULT_READONLY_RECOVERY",SQLITE_READONLY_RECOVERY},
		{"RESULT_READONLY_CANTLOCK",SQLITE_READONLY_CANTLOCK},
		{"RESULT_READONLY_ROLLBACK",SQLITE_READONLY_ROLLBACK},
		{"RESULT_READONLY_DBMOVED",SQLITE_READONLY_DBMOVED},
		{"RESULT_ABORT_ROLLBACK",SQLITE_ABORT_ROLLBACK},
		{"RESULT_CONSTRAINT_CHECK",SQLITE_CONSTRAINT_CHECK},
		{"RESULT_CONSTRAINT_COMMITHOOK",SQLITE_CONSTRAINT_COMMITHOOK},
		{"RESULT_CONSTRAINT_FOREIGNKEY",SQLITE_CONSTRAINT_FOREIGNKEY},
		{"RESULT_CONSTRAINT_FUNCTION",SQLITE_CONSTRAINT_FUNCTION},
		{"RESULT_CONSTRAINT_NOTNULL",SQLITE_CONSTRAINT_NOTNULL},
		{"RESULT_CONSTRAINT_PRIMARYKEY",SQLITE_CONSTRAINT_PRIMARYKEY},
		{"RESULT_CONSTRAINT_TRIGGER",SQLITE_CONSTRAINT_TRIGGER},
		{"RESULT_CONSTRAINT_UNIQUE",SQLITE_CONSTRAINT_UNIQUE},
		{"RESULT_CONSTRAINT_VTAB",SQLITE_CONSTRAINT_VTAB},
		{"RESULT_CONSTRAINT_ROWID",SQLITE_CONSTRAINT_ROWID},
		{"RESULT_NOTICE_RECOVER_WAL",SQLITE_NOTICE_RECOVER_WAL},
		{"RESULT_NOTICE_RECOVER_ROLLBACK",SQLITE_NOTICE_RECOVER_ROLLBACK},
		{"RESULT_WARNING_AUTOINDEX",SQLITE_WARNING_AUTOINDEX},
		{"RESULT_AUTH_USER",SQLITE_AUTH_USER},
		{"RESULT_OK_LOAD_PERMANENTLY",SQLITE_OK_LOAD_PERMANENTLY},

		{"TYPE_INTEGER",SQLITE_INTEGER},
		{"TYPE_FLOAT",SQLITE_FLOAT},
		{"TYPE_BLOB",SQLITE_BLOB},
		{"TYPE_NULL",SQLITE_NULL},
		{"TYPE_TEXT",SQLITE3_TEXT}
	});

	auto &modSqlite = l.RegisterLibrary("sqlite");
	auto classDefConnection = luabind::class_<std::shared_ptr<::sqlite::Connection>>("Connection");
	classDefConnection.def("Close",&Connection::Close);
	classDefConnection.def("PrepareStatement",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Connection>&,const std::string&,const luabind::object&)>(&Connection::PrepareStatement));
	classDefConnection.def("PrepareStatement",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Connection>&,const std::string&)>(&Connection::PrepareStatement));
	classDefConnection.def("OpenBlob",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Connection>&,const std::string&,const std::string&,int64_t,bool)>(&Connection::OpenBlob));
	classDefConnection.def("OpenBlob",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Connection>&,const std::string&,const std::string&,int64_t)>(&Connection::OpenBlob));
	classDefConnection.def("OpenBlob",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Connection>&,const std::string&,const std::string&,const std::string&,int64_t,bool)>(&Connection::OpenBlob));
	classDefConnection.def("OpenBlob",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Connection>&,const std::string&,const std::string&,const std::string&,int64_t)>(&Connection::OpenBlob));
	classDefConnection.def("Exec",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Connection>&,const std::string&,const luabind::object&)>(&Connection::Exec));
	classDefConnection.def("Exec",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Connection>&,const std::string&)>(&Connection::Exec));
	classDefConnection.def("GetResult",&Connection::GetResult);
	classDefConnection.def("GetResultMessage",&Connection::GetResultMessage);
	classDefConnection.def("IsDatabaseReadOnly",&Connection::IsDatabaseReadOnly);
	classDefConnection.def("DatabaseExists",&Connection::DatabaseExists);
	classDefConnection.def("CreateFunction",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Connection>&,const std::string&,int32_t,bool,const luabind::object&,const luabind::object&)>(&Connection::CreateFunction));
	classDefConnection.def("CreateFunction",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Connection>&,const std::string&,int32_t,bool,const luabind::object&)>(&Connection::CreateFunction));
	classDefConnection.def("DeleteFunction",&Connection::DeleteFunction);
	modSqlite[classDefConnection];

	auto classDefStatement = luabind::class_<std::shared_ptr<::sqlite::Statement>>("Statement");
	classDefStatement.def("Finalize",&Statement::Finalize);
	classDefStatement.def("Step",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Statement>&,const luabind::object&)>(&Statement::Step));
	classDefStatement.def("Step",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Statement>&)>(&Statement::Step));
	classDefStatement.def("Reset",&Statement::Reset);
	classDefStatement.def("GetType",&Statement::GetType);
	classDefStatement.def("GetBytes",&Statement::GetBytes);
	classDefStatement.def("GetDouble",&Statement::GetDouble);
	classDefStatement.def("GetInt",&Statement::GetInt);
	classDefStatement.def("GetInt64",&Statement::GetInt64);
	classDefStatement.def("GetText",&Statement::GetText);
	classDefStatement.def("GetBlob",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Statement>&,int32_t,const luabind::object&)>(&Statement::GetBlob));
	classDefStatement.def("GetBlob",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Statement>&,int32_t)>(&Statement::GetBlob));
	classDefStatement.def("GetCount",&Statement::GetCount);
	classDefStatement.def("GetColumnName",&Statement::GetColumnName);

	classDefStatement.def("BindDouble",&Statement::BindDouble);
	classDefStatement.def("BindInt",&Statement::BindInt);
	classDefStatement.def("BindInt64",&Statement::BindInt64);
	classDefStatement.def("BindNull",&Statement::BindNull);
	classDefStatement.def("BindText",&Statement::BindText);
	classDefStatement.def("BindBlob",&Statement::BindBlob);
	classDefStatement.def("ClearBindings",&Statement::ClearBindings);
	classDefStatement.def("IsReadOnly",&Statement::IsReadOnly);
	classDefStatement.def("GetSQLText",&Statement::GetSQLText);
	classDefStatement.def("GetExpandedSQLText",&Statement::GetExpandedSQLText);
	classDefStatement.def("GetDatabaseName",&Statement::GetDatabaseName);
	classDefStatement.def("GetTableName",&Statement::GetTableName);
	classDefStatement.def("GetOriginName",&Statement::GetOriginName);
	modSqlite[classDefStatement];

	auto classDefBlob = luabind::class_<std::shared_ptr<::sqlite::Blob>>("Blob");
	classDefBlob.def("Close",&Blob::Close);
	classDefBlob.def("GetBytes",&Blob::GetBytes);
	classDefBlob.def("Read",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Blob>&,int32_t,int32_t,const luabind::object&)>(&Blob::Read));
	classDefBlob.def("Read",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Blob>&,int32_t,int32_t)>(&Blob::Read));
	classDefBlob.def("Write",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Blob>&,DataStream&,int32_t,int32_t,const luabind::object&)>(&Blob::Write));
	classDefBlob.def("Write",static_cast<void(*)(lua_State*,std::shared_ptr<::sqlite::Blob>&,DataStream&,int32_t,int32_t)>(&Blob::Write));
	classDefBlob.def("Reopen",&Blob::Reopen);
	modSqlite[classDefBlob];

	auto classDefValue = luabind::class_<::sqlite::Value>("Value");
	classDefValue.def("GetType",&Value::GetType);
	classDefValue.def("GetNumericType",&Value::GetNumericType);
	classDefValue.def("GetBytes",&Value::GetBytes);
	classDefValue.def("GetDouble",&Value::GetDouble);
	classDefValue.def("GetInt",&Value::GetInt);
	classDefValue.def("GetInt64",&Value::GetInt64);
	classDefValue.def("GetText",&Value::GetText);
	classDefValue.def("GetBlob",&Value::GetBlob);
	modSqlite[classDefValue];
}
// Also used in engine in lfile.cpp
static bool add_addon_prefix(lua_State *l,std::string &path)
{
	auto fname = FileManager::GetCanonicalizedPath(Lua::get_current_file(l));
	if(fname.length() < 8 || ustring::compare(fname.c_str(),"addons\\",false,7) == false)
	{
		std::cout<<"WARNING: SQLite databases can only be opened by Lua-scripts inside an addon!"<<std::endl;
		return false;
	}
	auto br = fname.find(FileManager::GetDirectorySeparator(),8);
	auto prefix = ustring::sub(fname,0,br +1);
	path = prefix +path;
	return true;
}

int32_t Lua::sqlite::open(lua_State *l)
{
	std::string fname = Lua::CheckString(l,1);
	if(fname != ":memory:")
	{
		if(add_addon_prefix(l,fname) == false)
			return 0;
	}
	sqlite3 *pDb;
	int32_t r;
	if(Lua::IsSet(l,2) == false)
		r = sqlite3_open(fname.c_str(),&pDb);
	else
	{
		auto flags = Lua::CheckInt(l,2);
		r = sqlite3_open_v2(fname.c_str(),&pDb,flags,nullptr);
	}
	Lua::PushInt(l,r);
	if(r == SQLITE_OK)
	{
		auto con = std::make_shared<::sqlite::Connection>(pDb);
		Lua::Push<std::shared_ptr<::sqlite::Connection>>(l,con);

		CallbackHandle hCb = FunctionCallback<void>::Create(nullptr);
		std::weak_ptr<::sqlite::Connection> wpCon = con;
		hCb.get<Callback<void>>()->SetFunction([hCb,wpCon]() mutable {
			if(wpCon.expired() == false)
				wpCon.lock()->Poll();
			else if(hCb.IsValid())
				hCb.Remove();
		});
		ishared::add_callback(l,ishared::Callback::Think,hCb);
		return 2;
	}
	return 1;
}
int32_t Lua::sqlite::result_to_string(lua_State *l)
{
	auto errCode = Lua::CheckInt(l,1);
	Lua::PushString(l,::sqlite::Connection::GetResultMessage(errCode));
	return 1;
}

void Lua::sqlite::Connection::Close(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection)
{
	auto r = connection->Close();
	Lua::PushInt(l,r);
}
void Lua::sqlite::Connection::PrepareStatement(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &statement)
{
	int32_t r;
	auto pStatement = connection->PrepareStatement(statement,r);
	Lua::PushInt(l,r);
	if(r == SQLITE_OK)
		Lua::Push<std::shared_ptr<::sqlite::Statement>>(l,pStatement);
}
void Lua::sqlite::Connection::PrepareStatement(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &statement,const luabind::object &oAsyncCallback)
{
	Lua::CheckFunction(l,3);
	connection->PrepareStatement(statement,[l,oAsyncCallback](int32_t result,std::shared_ptr<::sqlite::Statement> pStatement) {
		oAsyncCallback.push(l);
		int32_t argCount = 1;
		Lua::PushInt(l,result);
		if(result == SQLITE_OK)
		{
			Lua::Push<std::shared_ptr<::sqlite::Statement>>(l,pStatement);
			++argCount;
		}
		ishared::protected_lua_call(l,argCount,0);
	});
}
void Lua::sqlite::Connection::OpenBlob(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &tableName,const std::string &columnName,int64_t rowId,bool bReadOnly)
{
	int32_t r;
	auto pBlob = connection->OpenBlob(tableName,columnName,rowId,bReadOnly,r);
	Lua::PushInt(l,r);
	if(r == SQLITE_OK)
		Lua::Push<std::shared_ptr<::sqlite::Blob>>(l,pBlob);
}
void Lua::sqlite::Connection::OpenBlob(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &tableName,const std::string &columnName,int64_t rowId) {OpenBlob(l,connection,tableName,columnName,rowId,false);}
void Lua::sqlite::Connection::OpenBlob(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &dbName,const std::string &tableName,const std::string &columnName,int64_t rowId,bool bReadOnly)
{
	int32_t r;
	auto pBlob = connection->OpenBlob(dbName,tableName,columnName,rowId,bReadOnly,r);
	Lua::PushInt(l,r);
	if(r == SQLITE_OK)
		Lua::Push<std::shared_ptr<::sqlite::Blob>>(l,pBlob);
}
void Lua::sqlite::Connection::OpenBlob(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &dbName,const std::string &tableName,const std::string &columnName,int64_t rowId) {OpenBlob(l,connection,dbName,tableName,columnName,rowId,false);}
void Lua::sqlite::Connection::Exec(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &sql,const luabind::object &oCallback)
{
	Lua::CheckFunction(l,3);
	std::string errMsg;
	auto r = connection->Exec(sql,[l,&oCallback](int32_t numColumns,char **colStrings,char **colNames) -> int32_t {
		oCallback.push(l);
		Lua::PushInt(l,numColumns);
		auto tColStrings = Lua::CreateTable(l);
		auto tColNames = Lua::CreateTable(l);
		for(auto i=decltype(numColumns){0};i<numColumns;++i)
		{
			Lua::PushInt(l,i +1);
			if(colStrings[i] != nullptr)
				Lua::PushString(l,colStrings[i]);
			else
				Lua::PushNil(l);
			Lua::SetTableValue(l,tColStrings);

			Lua::PushInt(l,i +1);
			Lua::PushString(l,colNames[i]);
			Lua::SetTableValue(l,tColNames);
		}
		if(ishared::protected_lua_call(l,3,1) == true)
		{
			if(Lua::IsBool(l,-1))
			{
				auto b = Lua::CheckBool(l,-1);
				Lua::Pop(l,1);
				return (b == true) ? 0 : 1;
			}
			Lua::Pop(l,1);
		}
		return 0;
	});
	Lua::PushInt(l,r);
	Lua::PushString(l,errMsg);
}
void Lua::sqlite::Connection::Exec(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &sql)
{
	auto r = connection->Exec(sql,nullptr);
	Lua::PushInt(l,r);
}
void Lua::sqlite::Connection::GetResult(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection) {Lua::PushInt(l,connection->GetResult());}
void Lua::sqlite::Connection::GetResultMessage(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection) {Lua::PushString(l,connection->GetResultMessage());}
void Lua::sqlite::Connection::IsDatabaseReadOnly(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &dbName) {Lua::PushBool(l,connection->IsDatabaseReadOnly(dbName));}
void Lua::sqlite::Connection::DatabaseExists(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &dbName) {Lua::PushBool(l,connection->DatabaseExists(dbName));}
void Lua::sqlite::Connection::CreateFunction(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &funcName,int32_t argCount,bool bDeterministic,const luabind::object &oXFunc)
{
	Lua::CheckFunction(l,5);
	auto r = connection->CreateFunction(funcName,argCount,bDeterministic,[l,oXFunc](sqlite3_context *context,int32_t argc,sqlite3_value **argv) {
		oXFunc.push(l);
		Lua::PushInt(l,argc);
		auto t = Lua::CreateTable(l);
		for(auto i=decltype(argc){0};i<argc;++i)
		{
			Lua::PushInt(l,i +1);
			Lua::Push<::sqlite::Value>(l,::sqlite::Value(argv[i]));
			Lua::SetTableValue(l,t);
		}
		ishared::protected_lua_call(l,2,0);
	});
	Lua::PushInt(l,r);
}
void Lua::sqlite::Connection::CreateFunction(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &funcName,int32_t argCount,bool bDeterministic,const luabind::object &oXStep,const luabind::object &oXFinal)
{
	Lua::CheckFunction(l,5);
	Lua::CheckFunction(l,6);
	auto r = connection->CreateFunction(funcName,argCount,bDeterministic,[l,oXStep](sqlite3_context *context,int32_t argc,sqlite3_value **argv) {
		oXStep.push(l);
		Lua::PushInt(l,argc);
		auto t = Lua::CreateTable(l);
		for(auto i=decltype(argc){0};i<argc;++i)
		{
			Lua::PushInt(l,i +1);
			Lua::Push<::sqlite::Value>(l,::sqlite::Value(argv[i]));
			Lua::SetTableValue(l,t);
		}
		ishared::protected_lua_call(l,2,0);
	},[l,oXFinal](sqlite3_context *context) {
		oXFinal.push(l);
		ishared::protected_lua_call(l,0,0);
	});
	Lua::PushInt(l,r);
}
void Lua::sqlite::Connection::DeleteFunction(lua_State *l,std::shared_ptr<::sqlite::Connection> &connection,const std::string &funcName,int32_t argCount,bool bDeterministic) {Lua::PushInt(l,connection->DeleteFunction(funcName,argCount,bDeterministic));}

void Lua::sqlite::Statement::Finalize(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement) {Lua::PushInt(l,statement->Finalize());}
void Lua::sqlite::Statement::Step(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement) {Lua::PushInt(l,statement->Step());}
void Lua::sqlite::Statement::Step(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,const luabind::object &oAsynCallback)
{
	Lua::CheckFunction(l,2);
	statement->Step([l,oAsynCallback](int32_t result) {
		oAsynCallback.push(l);
		Lua::PushInt(l,result);
		ishared::protected_lua_call(l,1,0);
	});
}
void Lua::sqlite::Statement::Reset(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement) {Lua::PushInt(l,statement->Reset());}
void Lua::sqlite::Statement::GetType(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column) {Lua::PushInt(l,statement->GetType(column));}
void Lua::sqlite::Statement::GetBytes(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column) {Lua::PushInt(l,statement->GetBytes(column));}
void Lua::sqlite::Statement::GetDouble(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column) {Lua::PushNumber(l,statement->GetDouble(column));}
void Lua::sqlite::Statement::GetInt(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column) {Lua::PushInt(l,statement->GetInt(column));}
void Lua::sqlite::Statement::GetInt64(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column) {Lua::PushInt(l,statement->GetInt64(column));}
void Lua::sqlite::Statement::GetText(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column) {Lua::PushString(l,statement->GetText(column));}
void Lua::sqlite::Statement::GetBlob(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column) {Lua::Push<DataStream>(l,statement->GetBlob(column));}
void Lua::sqlite::Statement::GetBlob(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column,const luabind::object &oAsyncCallback)
{
	Lua::CheckFunction(l,3);
	statement->GetBlob(column,[l,oAsyncCallback](DataStream ds) {
		oAsyncCallback.push(l);
		Lua::Push<DataStream>(l,ds);
		ishared::protected_lua_call(l,1,0);
	});
}
void Lua::sqlite::Statement::GetCount(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement) {Lua::PushInt(l,statement->GetCount());}
void Lua::sqlite::Statement::GetColumnName(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column) {Lua::PushString(l,statement->GetColumnName(column));}
void Lua::sqlite::Statement::BindDouble(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column,double d) {Lua::PushInt(l,statement->BindDouble(column,d));}
void Lua::sqlite::Statement::BindInt(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column,int32_t i) {Lua::PushInt(l,statement->BindInt(column,i));}
void Lua::sqlite::Statement::BindInt64(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column,int64_t i) {Lua::PushInt(l,statement->BindInt64(column,i));}
void Lua::sqlite::Statement::BindNull(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column) {Lua::PushInt(l,statement->BindNull(column));}
void Lua::sqlite::Statement::BindText(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column,const std::string &text) {Lua::PushInt(l,statement->BindText(column,text));}
void Lua::sqlite::Statement::BindBlob(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column,DataStream &ds) {Lua::PushInt(l,statement->BindBlob(column,ds));}
void Lua::sqlite::Statement::ClearBindings(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement) {Lua::PushInt(l,statement->ClearBindings());}
void Lua::sqlite::Statement::IsReadOnly(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement) {Lua::PushBool(l,statement->IsReadOnly());}
void Lua::sqlite::Statement::GetSQLText(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement) {Lua::PushString(l,statement->GetSQLText());}
void Lua::sqlite::Statement::GetExpandedSQLText(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement) {Lua::PushString(l,statement->GetExpandedSQLText());}
void Lua::sqlite::Statement::GetDatabaseName(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column) {Lua::PushString(l,statement->GetDatabaseName(column));}
void Lua::sqlite::Statement::GetTableName(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column) {Lua::PushString(l,statement->GetTableName(column));}
void Lua::sqlite::Statement::GetOriginName(lua_State *l,std::shared_ptr<::sqlite::Statement> &statement,int32_t column) {Lua::PushString(l,statement->GetOriginName(column));}

void Lua::sqlite::Blob::Close(lua_State *l,std::shared_ptr<::sqlite::Blob> &blob) {Lua::PushInt(l,blob->Close());}
void Lua::sqlite::Blob::GetBytes(lua_State *l,std::shared_ptr<::sqlite::Blob> &blob) {Lua::PushInt(l,blob->GetBytes());}
void Lua::sqlite::Blob::Read(lua_State *l,std::shared_ptr<::sqlite::Blob> &blob,int32_t size,int32_t offset,const luabind::object &oAsyncCallback)
{
	Lua::CheckFunction(l,4);
	blob->Read(size,offset,[l,oAsyncCallback](int32_t result,DataStream ds) {
		oAsyncCallback.push(l);
		int32_t argCount = 1;
		Lua::PushInt(l,result);
		if(result == SQLITE_OK)
		{
			Lua::Push<DataStream>(l,ds);
			++argCount;
		}
		ishared::protected_lua_call(l,argCount,0);
	});
}
void Lua::sqlite::Blob::Read(lua_State *l,std::shared_ptr<::sqlite::Blob> &blob,int32_t size,int32_t offset)
{
	int32_t result;
	auto ds = blob->Read(size,offset,result);
	Lua::PushInt(l,result);
	if(result != SQLITE_OK)
		return;
	Lua::Push<DataStream>(l,ds);
}
void Lua::sqlite::Blob::Write(lua_State *l,std::shared_ptr<::sqlite::Blob> &blob,DataStream &ds,int32_t size,int32_t offset,const luabind::object &oAsyncCallback)
{
	Lua::CheckFunction(l,5);
	blob->Write(ds,size,offset,[l,oAsyncCallback](int32_t result) {
		oAsyncCallback.push(l);
		Lua::PushInt(l,result);
		ishared::protected_lua_call(l,1,0);
	});
}
void Lua::sqlite::Blob::Write(lua_State *l,std::shared_ptr<::sqlite::Blob> &blob,DataStream &ds,int32_t size,int32_t offset) {Lua::PushInt(l,blob->Write(ds,size,offset));}
void Lua::sqlite::Blob::Reopen(lua_State *l,std::shared_ptr<::sqlite::Blob> &blob,int64_t rowId) {Lua::PushInt(l,blob->Reopen(rowId));}

void Lua::sqlite::Value::GetType(lua_State *l,::sqlite::Value &value) {Lua::PushInt(l,value.GetType());}
void Lua::sqlite::Value::GetNumericType(lua_State *l,::sqlite::Value &value) {Lua::PushInt(l,value.GetNumericType());}
void Lua::sqlite::Value::GetBytes(lua_State *l,::sqlite::Value &value) {Lua::PushInt(l,value.GetBytes());}
void Lua::sqlite::Value::GetDouble(lua_State *l,::sqlite::Value &value) {Lua::PushNumber(l,value.GetDouble());}
void Lua::sqlite::Value::GetInt(lua_State *l,::sqlite::Value &value) {Lua::PushInt(l,value.GetInt());}
void Lua::sqlite::Value::GetInt64(lua_State *l,::sqlite::Value &value) {Lua::PushInt(l,value.GetInt64());}
void Lua::sqlite::Value::GetText(lua_State *l,::sqlite::Value &value) {Lua::PushString(l,value.GetText());}
void Lua::sqlite::Value::GetBlob(lua_State *l,::sqlite::Value &value) {Lua::Push<DataStream>(l,value.GetBlob());}
