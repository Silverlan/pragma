#include "sqlite_connection.hpp"
#include "sqlite_statement.hpp"
#include "sqlite_blob.hpp"
#include <sqlite3.h>

sqlite::Connection::Connection()
{}
sqlite::Connection::~Connection() {Close();}
void sqlite::Connection::Connect(const std::string &name,const std::function<void(sql::Result)> &callback,int32_t *flags)
{
	ScheduleThreadCall([this,name,flags,callback]() {
		sqlite3 *pDb;
		auto r = ::sql::Result::Ok;
		if(flags == nullptr)
			r = static_cast<::sql::Result>(sqlite3_open(name.c_str(),&pDb));
		else
			r = static_cast<::sql::Result>(sqlite3_open_v2(name.c_str(),&pDb,*flags,nullptr));
		r = UpdateLastResult(r);
		m_pSql = pDb;
		SchedulePollCall([this,callback,r]() mutable {
			callback(r);
		});
	});
}
sql::Result sqlite::Connection::Connect(const std::string &name,int32_t *flags)
{
	auto r = sql::Result::Ok;
	Connect(name,[&r](sql::Result ar) {r = ar;},flags);
	Wait();
	return r;
}
sql::Result sqlite::Connection::CloseConnection()
{
	auto r = sqlite3_close_v2(m_pSql);
	m_pSql = nullptr;
	UpdateLastResult();
	return static_cast<sql::Result>(r);
}
bool sqlite::Connection::IsOpen() const {return (m_pSql != nullptr) ? true : false;}

std::shared_ptr<sqlite::Blob> sqlite::Connection::OpenBlob(const std::string &dbName,const std::string &tableName,const std::string columnName,int64_t rowId,bool bReadOnly,int32_t &result)
{
	sqlite3_blob *pBlob;
	result = sqlite3_blob_open(m_pSql,dbName.c_str(),tableName.c_str(),columnName.c_str(),rowId,(bReadOnly == true) ? 0 : 1,&pBlob);
	UpdateLastResult();
	if(result != SQLITE_OK)
		return nullptr;
	return std::make_shared<sqlite::Blob>(pBlob,*this);
}
std::shared_ptr<sqlite::Blob> sqlite::Connection::OpenBlob(const std::string &tableName,const std::string columnName,int64_t rowId,bool bReadOnly,int32_t &result) {return OpenBlob("main",tableName,columnName,rowId,bReadOnly,result);}
void sqlite::Connection::CreateStatement(const std::string &sql,const std::function<void(sql::Result,std::shared_ptr<sql::BaseStatement>)> &callback)
{
	ScheduleThreadCall([this,sql,callback]() {
		sqlite3_stmt *pStmt = nullptr;
		auto result = sqlite3_prepare_v2(m_pSql,sql.c_str(),-1,&pStmt,nullptr);
		UpdateLastResult();
		if(result != SQLITE_OK)
		{
			SchedulePollCall(std::bind(callback,static_cast<sql::Result>(result),std::shared_ptr<sql::BaseStatement>{nullptr}));
			return;
		}
		auto statement = std::shared_ptr<sqlite::Statement>(new sqlite::Statement(pStmt,*this),GetStatementDeleter([](void *ptr) {delete static_cast<Statement*>(ptr);}));
		SchedulePollCall(std::bind(callback,sql::Result::Ok,statement));
	});
}
void sqlite::Connection::CreatePreparedStatement(const std::string &sql,const std::function<void(sql::Result,std::shared_ptr<sql::BasePreparedStatement>)> &callback)
{
	ScheduleThreadCall([this,sql,callback]() {
		sqlite3_stmt *pStmt = nullptr;
		auto result = sqlite3_prepare_v2(m_pSql,sql.c_str(),-1,&pStmt,nullptr);
		UpdateLastResult();
		if(result != SQLITE_OK)
		{
			SchedulePollCall(std::bind(callback,static_cast<sql::Result>(result),std::shared_ptr<sql::BasePreparedStatement>{nullptr}));
			return;
		}
		auto statement = std::shared_ptr<sqlite::PreparedStatement>(new sqlite::PreparedStatement(pStmt,*this),GetStatementDeleter([](void *ptr) {delete static_cast<PreparedStatement*>(ptr);}));
		SchedulePollCall(std::bind(callback,sql::Result::Ok,statement));
	});
}
std::string sqlite::Connection::ResultToString(sql::Result result) const {return sqlite3_errstr(umath::to_integral(result));}
sql::Result sqlite::Connection::UpdateLastResult(sql::Result r,bool bTranslateStepCodes)
{
	auto *msg = sqlite3_errmsg(m_pSql);
	SetLastResult(r,msg);
	return r;
}
sql::Result sqlite::Connection::UpdateLastResult(bool bTranslateStepCodes)
{
	auto code = static_cast<sql::Result>(sqlite3_extended_errcode(m_pSql));
	if(bTranslateStepCodes == true && (code == sql::Result::Row || code == sql::Result::Done))
		code = sql::Result::Ok;
	return UpdateLastResult(code,bTranslateStepCodes);
}
sql::Result sqlite::Connection::AsyncCreateStatement(const std::string &sql,std::shared_ptr<sql::BaseStatement> &statement)
{
	sqlite3_stmt *pStmt = nullptr;
	auto result = sqlite3_prepare_v2(m_pSql,sql.c_str(),-1,&pStmt,nullptr);
	UpdateLastResult();
	if(result != SQLITE_OK)
		return static_cast<sql::Result>(result);
	statement = std::shared_ptr<sqlite::Statement>(new sqlite::Statement(pStmt,*this),GetStatementDeleter([](void *ptr) {delete static_cast<Statement*>(ptr);}));
	return sql::Result::Ok;
}
sql::Result sqlite::Connection::AsyncCreatePreparedStatement(const std::string &sql,std::shared_ptr<sql::BasePreparedStatement> &statement)
{
	sqlite3_stmt *pStmt = nullptr;
	auto result = sqlite3_prepare_v2(m_pSql,sql.c_str(),-1,&pStmt,nullptr);
	UpdateLastResult();
	if(result != SQLITE_OK)
		return static_cast<sql::Result>(result);
	statement = std::shared_ptr<sqlite::PreparedStatement>(new sqlite::PreparedStatement(pStmt,*this),GetStatementDeleter([](void *ptr) {delete static_cast<PreparedStatement*>(ptr);}));
	return sql::Result::Ok;
}
void sqlite::Connection::IsDatabaseReadOnly(const std::string &dbName,std::function<void(sql::Result,bool)> &callback)
{
	ScheduleThreadCall([this,callback,dbName]() {
		auto r = sqlite3_db_readonly(m_pSql,dbName.c_str()) == 1;
		UpdateLastResult();
		SchedulePollCall(std::bind(callback,sql::Result::Ok,r));
	});
}
bool sqlite::Connection::DatabaseExists(const std::string &dbName,std::function<void(sql::Result,bool)> &callback)
{
	ScheduleThreadCall([this,callback,dbName]() {
		auto r = sqlite3_db_readonly(m_pSql,dbName.c_str()) != -1;
		UpdateLastResult();
		SchedulePollCall(std::bind(callback,sql::Result::Ok,r));
	});
}
void sqlite::Connection::GetLastInsertRowId(const std::function<void(sql::Result,int64_t)> &callback)
{
	ScheduleThreadCall([this,callback]() {
		auto rowId = sqlite3_last_insert_rowid(m_pSql);
		UpdateLastResult();
		SchedulePollCall(std::bind(callback,sql::Result::Ok,rowId));
	});
}
int32_t sqlite::Connection::CreateFunction(const std::string &funcName,int32_t argCount,bool bDeterministic,const std::function<void(sqlite3_context*,int,sqlite3_value**)> &xFunc)
{
	using UserData = std::remove_const_t<std::remove_reference_t<decltype(xFunc)>>;
	auto flags = SQLITE_UTF8;
	if(bDeterministic == true)
		flags |= SQLITE_DETERMINISTIC;
	auto *ptr = new UserData(xFunc);
	return sqlite3_create_function_v2(m_pSql,funcName.c_str(),argCount,flags,ptr,[](sqlite3_context *context,int32_t argc,sqlite3_value **argv) {
		auto *f = static_cast<UserData*>(sqlite3_user_data(context));
		(*f)(context,argc,argv);
	},nullptr,nullptr,[](void *data) {
		delete static_cast<UserData*>(data);
	});
}
int32_t sqlite::Connection::CreateFunction(const std::string &funcName,int32_t argCount,bool bDeterministic,const std::function<void(sqlite3_context*,int,sqlite3_value**)> &xStep,const std::function<void(sqlite3_context*)> &xFinal)
{
	using UserData = std::pair<std::remove_const_t<std::remove_reference_t<decltype(xStep)>>,std::remove_const_t<std::remove_reference_t<decltype(xFinal)>>>;
	auto flags = SQLITE_UTF8;
	if(bDeterministic == true)
		flags |= SQLITE_DETERMINISTIC;
	auto *ptr = new UserData(xStep,xFinal);
	return sqlite3_create_function_v2(m_pSql,funcName.c_str(),argCount,flags,ptr,nullptr,[](sqlite3_context *context,int32_t argc,sqlite3_value **argv) {
		auto *f = static_cast<UserData*>(sqlite3_user_data(context));
		f->first(context,argc,argv);
	},[](sqlite3_context *context) {
		auto *f = static_cast<UserData*>(sqlite3_user_data(context));
		f->second(context);
	},[](void *data) {
		delete static_cast<UserData*>(data);
	});
}
int32_t sqlite::Connection::DeleteFunction(const std::string &funcName,int32_t argCount,bool bDeterministic)
{
	auto flags = SQLITE_UTF8;
	if(bDeterministic == true)
		flags |= SQLITE_DETERMINISTIC;
	return sqlite3_create_function_v2(m_pSql,funcName.c_str(),argCount,flags,nullptr,nullptr,nullptr,nullptr,nullptr);
}
