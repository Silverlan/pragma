#include "sqlite_connection.hpp"
#include "sqlite_statement.hpp"
#include "sqlite_blob.hpp"
#include <sqlite3.h>

sqlite::Connection::Connection(sqlite3 *pSql)
	: m_pSql(pSql)
{}
sqlite::Connection::~Connection() {Close();}
int32_t sqlite::Connection::Close()
{
	if(m_pSql == nullptr)
		return SQLITE_OK;
	if(m_thread != nullptr)
	{
		m_bEndThread = true;
		m_thread->join();
		m_thread = nullptr;
	}
	auto r = sqlite3_close_v2(m_pSql);
	m_pSql = nullptr;
	return r;
}

void sqlite::Connection::SetUserData(const std::shared_ptr<void> &userData) {m_userData = userData;}
std::shared_ptr<void> sqlite::Connection::GetUserData() const {return m_userData;}

std::string sqlite::Connection::GetResultMessage(int32_t code) {return sqlite3_errstr(code);}
std::shared_ptr<sqlite::Blob> sqlite::Connection::OpenBlob(const std::string &dbName,const std::string &tableName,const std::string columnName,int64_t rowId,bool bReadOnly,int32_t &result)
{
	sqlite3_blob *pBlob;
	result = sqlite3_blob_open(m_pSql,dbName.c_str(),tableName.c_str(),columnName.c_str(),rowId,(bReadOnly == true) ? 0 : 1,&pBlob);
	if(result != SQLITE_OK)
		return nullptr;
	return std::make_shared<sqlite::Blob>(pBlob,*this);
}
std::shared_ptr<sqlite::Blob> sqlite::Connection::OpenBlob(const std::string &tableName,const std::string columnName,int64_t rowId,bool bReadOnly,int32_t &result) {return OpenBlob("main",tableName,columnName,rowId,bReadOnly,result);}
std::shared_ptr<sqlite::Statement> sqlite::Connection::PrepareStatement(const std::string &statement,int32_t &result)
{
	sqlite3_stmt *pStmt;
	result = sqlite3_prepare_v2(m_pSql,statement.c_str(),-1,&pStmt,nullptr);
	if(result != SQLITE_OK)
		return nullptr;
	return std::make_shared<sqlite::Statement>(pStmt,*this);
}
void sqlite::Connection::PrepareStatement(const std::string &statement,const std::function<void(int32_t,std::shared_ptr<Statement>)> &callback)
{
	ScheduleThreadCall([this,statement,callback]() {
		int32_t r;
		auto pStatement = PrepareStatement(statement,r);
		SchedulePollCall([r,pStatement,callback]() {callback(r,pStatement);});
	});
}
int32_t sqlite::Connection::Exec(const std::string &sql,const std::function<int32_t(int32_t,char**,char**)> &callback)
{
	auto r = sqlite3_exec(m_pSql,sql.c_str(),[](void *userData,int32_t numColumns,char **colStrings,char **colNames) -> int32_t {
		auto *f = static_cast<const std::function<int32_t(int32_t,char**,char**)>*>(userData);
		return (*f)(numColumns,colStrings,colNames);
	},const_cast<void*>(static_cast<const void*>(&callback)),nullptr);
	return r;
}
int32_t sqlite::Connection::GetResult() const {return sqlite3_extended_errcode(m_pSql);}
std::string sqlite::Connection::GetResultMessage() const {return sqlite3_errmsg(m_pSql);}
bool sqlite::Connection::IsDatabaseReadOnly(const std::string &dbName) const {return sqlite3_db_readonly(m_pSql,dbName.c_str()) == 1;}
bool sqlite::Connection::DatabaseExists(const std::string &dbName) const {return sqlite3_db_readonly(m_pSql,dbName.c_str()) != -1;}
int64_t sqlite::Connection::GetLastInsertRowId() const {return sqlite3_last_insert_rowid(m_pSql);}
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

void sqlite::Connection::InitializeThread()
{
	if(m_thread != nullptr)
		return;
	m_thread = std::make_unique<std::thread>([this]() {
		while(m_bEndThread == false)
		{
			m_queueMutex.lock();
				while(m_threadQueue.empty() == false && m_bEndThread == false)
				{
					m_threadQueue.front()();
					m_threadQueue.pop();
				}
			m_queueMutex.unlock();
		}
	});
}
void sqlite::Connection::SchedulePollCall(const std::function<void(void)> &f)
{
	m_pollMutex.lock();
		m_pollQueue.push(f);
	m_pollMutex.unlock();
}
void sqlite::Connection::Poll()
{
	m_pollMutex.lock();
		while(m_pollQueue.empty() == false)
		{
			m_pollQueue.front()();
			m_pollQueue.pop();
		}
	m_pollMutex.unlock();
}
void sqlite::Connection::ScheduleThreadCall(const std::function<void(void)> &f)
{
	InitializeThread();
	m_queueMutex.lock();
		m_threadQueue.push(f);
	m_queueMutex.unlock();
}
