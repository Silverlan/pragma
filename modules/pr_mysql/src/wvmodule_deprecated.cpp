#if 0
#include "wvmodule.h"
#include "mysql.h"
#include "lmysql.h"
#include <Windows.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <luasystem.h>
#include <sharedutils/functioncallback.h>
#include <ishared.hpp>
#include <luainterface.hpp>

sql::Driver *sql_driver = NULL;
static CallbackHandle think_callback;

#pragma comment(lib,"lua51.lib")
#pragma comment(lib,"luasystem.lib")
#pragma comment(lib,"mysqlcppconn.lib")
#pragma comment(lib,"luabind.lib")
#pragma comment(lib,"vfilesystem.lib")
#pragma comment(lib,"sharedutils.lib")
#pragma comment(lib,"ishared.lib")

static bool InitializeMySQL(Lua::Interface &l,std::string *err)
{
	try {
		sql_driver = get_driver_instance();
	}
	catch(sql::SQLException &e)
	{
		*err = e.what();
		return false;
	}
	think_callback = FunctionCallback<void>::Create(&MySQL::Think);
	lua_State *lua = l.GetState();
	ishared::add_callback(lua,ishared::Callback::Think,think_callback);

	Lua::RegisterLibrary(lua,"mysql",{
		{"connect",Lua::mysql::connect},
		{"get_error",Lua::mysql::get_error},
	});

	auto &modMySQL = l.RegisterLibrary("mysql");
	auto classDefConnection = luabind::class_<MySQL>("MySQLConnection");
	classDefConnection.def("SetSchema",static_cast<void(*)(lua_State*,MySQL&,std::string)>(&Lua::mysql::Connection::SetSchema));
	classDefConnection.def("SetSchema",static_cast<void(*)(lua_State*,MySQL&,std::string,luabind::object)>(&Lua::mysql::Connection::SetSchema));
	classDefConnection.def("CreateStatement",&Lua::mysql::Connection::CreateStatement);
	classDefConnection.def("GetVersion",&Lua::mysql::Connection::GetVersion);
	classDefConnection.def("GetDatabaseProductName",&Lua::mysql::Connection::GetDatabaseProductName);
	classDefConnection.def("PrepareStatement",&Lua::mysql::Connection::PrepareStatement);
	classDefConnection.def("Release",&Lua::mysql::Connection::Release);
	modMySQL[classDefConnection];

	auto classDefStatement = luabind::class_<MySQLStatement>("MySQLStatement");
	classDefStatement.def("Execute",static_cast<void(*)(lua_State*,MySQLStatement&,std::string)>(&Lua::mysql::Statement::Execute));
	classDefStatement.def("Execute",static_cast<void(*)(lua_State*,MySQLStatement&,std::string,luabind::object)>(&Lua::mysql::Statement::Execute));
	classDefStatement.def("ExecuteQuery",static_cast<void(*)(lua_State*,MySQLStatement&,std::string)>(&Lua::mysql::Statement::ExecuteQuery));
	classDefStatement.def("ExecuteQuery",static_cast<void(*)(lua_State*,MySQLStatement&,std::string,luabind::object)>(&Lua::mysql::Statement::ExecuteQuery));
	classDefStatement.def("Release",&Lua::mysql::Statement::Release);
	modMySQL[classDefStatement];

	auto classDefPreparedStatement = luabind::class_<MySQLPreparedStatement,MySQLStatement>("MySQLPreparedStatement");
	classDefPreparedStatement.def("SetBigInt",&Lua::mysql::PreparedStatement::SetBigInt);
	classDefPreparedStatement.def("SetBoolean",&Lua::mysql::PreparedStatement::SetBoolean);
	classDefPreparedStatement.def("SetDouble",&Lua::mysql::PreparedStatement::SetDouble);
	classDefPreparedStatement.def("SetInt",&Lua::mysql::PreparedStatement::SetInt);
	classDefPreparedStatement.def("SetInt64",&Lua::mysql::PreparedStatement::SetInt64);
	classDefPreparedStatement.def("SetString",&Lua::mysql::PreparedStatement::SetString);
	classDefPreparedStatement.def("SetUInt",&Lua::mysql::PreparedStatement::SetUInt);
	classDefPreparedStatement.def("SetUInt64",&Lua::mysql::PreparedStatement::SetUInt64);
	classDefPreparedStatement.def("Execute",static_cast<void(*)(lua_State*,MySQLPreparedStatement&)>(&Lua::mysql::PreparedStatement::Execute));
	classDefPreparedStatement.def("Execute",static_cast<void(*)(lua_State*,MySQLPreparedStatement&,luabind::object)>(&Lua::mysql::PreparedStatement::Execute));
	classDefPreparedStatement.def("ExecuteQuery",static_cast<void(*)(lua_State*,MySQLPreparedStatement&)>(&Lua::mysql::PreparedStatement::ExecuteQuery));
	classDefPreparedStatement.def("ExecuteQuery",static_cast<void(*)(lua_State*,MySQLPreparedStatement&,luabind::object)>(&Lua::mysql::PreparedStatement::ExecuteQuery));
	modMySQL[classDefPreparedStatement];

	auto classDefResult = luabind::class_<MySQLResult>("MySQLResult");
	classDefResult.def("FetchNext",&Lua::mysql::Result::FetchNext);
	classDefResult.def("GetBoolean",(void(*)(lua_State*,MySQLResult&,std::string))&Lua::mysql::Result::GetBoolean);
	classDefResult.def("GetBoolean",(void(*)(lua_State*,MySQLResult&,int))&Lua::mysql::Result::GetBoolean);
	classDefResult.def("GetDouble",(void(*)(lua_State*,MySQLResult&,std::string))&Lua::mysql::Result::GetDouble);
	classDefResult.def("GetDouble",(void(*)(lua_State*,MySQLResult&,int))&Lua::mysql::Result::GetDouble);
	classDefResult.def("GetInt",(void(*)(lua_State*,MySQLResult&,std::string))&Lua::mysql::Result::GetInt);
	classDefResult.def("GetInt",(void(*)(lua_State*,MySQLResult&,int))&Lua::mysql::Result::GetInt);
	classDefResult.def("GetInt64",(void(*)(lua_State*,MySQLResult&,std::string))&Lua::mysql::Result::GetInt64);
	classDefResult.def("GetInt64",(void(*)(lua_State*,MySQLResult&,int))&Lua::mysql::Result::GetInt64);
	classDefResult.def("GetString",(void(*)(lua_State*,MySQLResult&,std::string))&Lua::mysql::Result::GetString);
	classDefResult.def("GetString",(void(*)(lua_State*,MySQLResult&,int))&Lua::mysql::Result::GetString);
	classDefResult.def("GetUInt",(void(*)(lua_State*,MySQLResult&,std::string))&Lua::mysql::Result::GetUInt);
	classDefResult.def("GetUInt",(void(*)(lua_State*,MySQLResult&,int))&Lua::mysql::Result::GetUInt);
	classDefResult.def("GetUInt64",(void(*)(lua_State*,MySQLResult&,std::string))&Lua::mysql::Result::GetUInt64);
	classDefResult.def("GetUInt64",(void(*)(lua_State*,MySQLResult&,int))&Lua::mysql::Result::GetUInt64);
	classDefResult.def("Release",&Lua::mysql::Result::Release);
	modMySQL[classDefResult];
	return true;
}

static void CloseMySQL()
{
	MySQL::Think();
	for(;;)
	{
		MySQL::MUTEX_CONNECT_THREAD.lock();
			if(MySQL::CONNECT_THREAD_COUNT == 0) // Wait for all main threads to end
			{
				MySQL::MUTEX_CONNECT_THREAD.unlock();
				break;
			}
		MySQL::MUTEX_CONNECT_THREAD.unlock();
	}
	//if(think_callback != NULL) // Automatically released by the engine
	//	delete think_callback;
}

namespace pragma
{
	void __declspec(dllexport) InitializeLua(Lua::Interface &l)
	{
		std::string err;
		if(InitializeMySQL(l,&err) == false)
			std::cout<<"WARNING: Unable to initialize MySQL module: "<<err<<std::endl;
	}
	void __declspec(dllexport) Attach() {}
	void __declspec(dllexport) Detach() {CloseMySQL();}
};
#endif