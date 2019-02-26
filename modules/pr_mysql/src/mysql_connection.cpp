#include "mysql_connection.hpp"
#include "mysql_statement.hpp"
#include <sharedutils/util_disarmable_deleter.hpp>
//#include "sqlite_statement.hpp"
//#include "sqlite_blob.hpp"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/metadata.h>


mysql::Connection::Connection()
{}
mysql::Connection::~Connection() {Close();}

void mysql::Connection::Connect(const std::string &ip,const std::string &user,const std::string &password,const std::string &dbName,const std::function<void(sql::Result)> &callback)
{
	ScheduleThreadCall([this,ip,user,password,dbName,callback]() {
		try
		{
			auto *driver = get_driver_instance();
			auto *con = driver->connect(ip,user,password);
			auto sqlCon = std::shared_ptr<::sql::Connection>(con,util::DisarmableDeleter<::sql::Connection>());
			sqlCon->setSchema(dbName);
			UpdateLastResult();
			SchedulePollCall([this,callback,sqlCon]() mutable {
				m_pSql = std::unique_ptr<::sql::Connection>(util::DisarmableDeleter<::sql::Connection>::release(sqlCon));
				callback(sql::Result::Ok);
			});
		}
		catch(const sql::SQLException &e)
		{
			auto r = UpdateLastResult(e);
			SchedulePollCall([r,callback]() {
				callback(r);
			});
		}
	});
}
sql::Result mysql::Connection::UpdateLastResult(const sql::SQLException &e)
{
	auto result = static_cast<sql::Result>(e.getErrorCode());
	auto *msg = e.what();
	if(msg != "" && result == sql::Result::Ok)
		result = sql::Result::Internal; // For some reason SQL exceptions returna code of 0 (=Result::Ok) in some cases, even though there was an error
	SetLastResult(result,msg);
	return result;
}
sql::Result mysql::Connection::UpdateLastResult() {SetLastResult(sql::Result::Ok,""); return sql::Result::Ok;}
sql::Result mysql::Connection::Connect(const std::string &ip,const std::string &user,const std::string &password,const std::string &dbName)
{
	auto r = sql::Result::Ok;
	Connect(ip,user,password,dbName,[&r](sql::Result ar) {r = ar;});
	Wait();
	return r;
}

bool mysql::Connection::IsOpen() const {return (m_pSql != nullptr) ? true : false;}
sql::Result mysql::Connection::AsyncCreateStatement(const std::string &sql,std::shared_ptr<sql::BaseStatement> &statement)
{
	try
	{
		auto *ptStmt = m_pSql->createStatement();
		statement = std::shared_ptr<sql::BaseStatement>(new Statement(ptStmt,*this),GetStatementDeleter([](void *ptr) {delete static_cast<Statement*>(ptr);}));
		static_cast<mysql::Statement&>(*statement).SetQuery(sql);
		UpdateLastResult();
	}
	catch(const sql::SQLException &e)
	{
		return UpdateLastResult(e);
	}
	return sql::Result::Ok;
}
sql::Result mysql::Connection::AsyncCreatePreparedStatement(const std::string &sql,std::shared_ptr<sql::BasePreparedStatement> &statement)
{
	try
	{
		auto *ptStmt = m_pSql->prepareStatement(sql);
		statement = std::shared_ptr<sql::BasePreparedStatement>(new PreparedStatement(ptStmt,*this),GetStatementDeleter([](void *ptr) {delete static_cast<PreparedStatement*>(ptr);}));
		static_cast<mysql::PreparedStatement&>(*statement).SetQuery(sql);
		UpdateLastResult();
	}
	catch(const sql::SQLException &e)
	{
		return UpdateLastResult(e);
	}
	return sql::Result::Ok;
}
sql::Result mysql::Connection::CloseConnection()
{
	try
	{
		m_pSql = nullptr;
		UpdateLastResult();
	}
	catch(const sql::SQLException &e) {return UpdateLastResult(e);}
	return sql::Result::Ok;
}

/*std::shared_ptr<mysql::Blob> mysql::Connection::OpenBlob(const std::string &dbName,const std::string &tableName,const std::string columnName,int64_t rowId,bool bReadOnly,int32_t &result)
{
	sqlite3_blob *pBlob;
	result = sqlite3_blob_open(m_pSql,dbName.c_str(),tableName.c_str(),columnName.c_str(),rowId,(bReadOnly == true) ? 0 : 1,&pBlob);
	if(result != SQLITE_OK)
		return nullptr;
	return std::make_shared<mysql::Blob>(pBlob,*this);
}
std::shared_ptr<mysql::Blob> mysql::Connection::OpenBlob(const std::string &tableName,const std::string columnName,int64_t rowId,bool bReadOnly,int32_t &result) {return OpenBlob("main",tableName,columnName,rowId,bReadOnly,result);}
void mysql::Connection::PrepareStatement(const std::string &statement,const std::function<void(int32_t,std::shared_ptr<Statement>)> &callback)
{
	ScheduleThreadCall([this,statement,callback]() {
		int32_t r;
		auto pStatement = PrepareStatement(statement,r);
		SchedulePollCall([r,pStatement,callback]() {callback(r,pStatement);});
	});
}*/
		
void mysql::Connection::CreateStatement(const std::string &sql,const std::function<void(sql::Result,std::shared_ptr<sql::BaseStatement>)> &callback)
{
	ScheduleThreadCall([this,callback,sql]() {
		auto statement = std::shared_ptr<sql::BaseStatement>{nullptr};
		auto r = AsyncCreateStatement(sql,statement);
		SchedulePollCall([r,statement,callback]() {
			callback(r,statement);
		});
	});
}
void mysql::Connection::CreatePreparedStatement(const std::string &sql,const std::function<void(sql::Result,std::shared_ptr<sql::BasePreparedStatement>)> &callback)
{
	ScheduleThreadCall([this,callback,sql]() {
		try
		{
			auto *ptStmt = m_pSql->prepareStatement(sql);
			auto statement = std::shared_ptr<sql::BasePreparedStatement>(new PreparedStatement(ptStmt,*this),GetStatementDeleter([](void *ptr) {delete static_cast<PreparedStatement*>(ptr);}));
			UpdateLastResult();
			SchedulePollCall([statement,callback]() {
				callback(sql::Result::Ok,statement);
			});
		}
		catch(const sql::SQLException &e)
		{
			auto r = UpdateLastResult(e);
			SchedulePollCall([r,callback]() {
				callback(r,nullptr);
			});
		}
	});
}
void mysql::Connection::IsDatabaseReadOnly(const std::function<void(sql::Result,bool)> &callback)
{
	ScheduleThreadCall([this,callback]() {
		try
		{
			auto *meta = m_pSql->getMetaData();
			auto bReadOnly = meta->isReadOnly();
			UpdateLastResult();
			SchedulePollCall([bReadOnly,callback]() {
				callback(sql::Result::Ok,bReadOnly);
			});
		}
		catch(const sql::SQLException &e)
		{
			auto r = UpdateLastResult(e);
			SchedulePollCall([r,callback]() {
				callback(r,nullptr);
			});
		}
	});
}
void mysql::Connection::GetLastInsertRowId(const std::function<void(sql::Result,int64_t)> &callback)
{
	ScheduleThreadCall([this,callback]() {
		try
		{
			auto ptStmt = std::unique_ptr<sql::Statement>(m_pSql->createStatement());
			auto *rs = ptStmt->executeQuery("SELECT @@identity AS id");
			if(rs != nullptr && rs->next() == true)
			{
				auto id = rs->getInt64(0);
				UpdateLastResult();
				SchedulePollCall([id,callback]() {
					callback(sql::Result::Ok,id);
				});
			}
			else
			{
				UpdateLastResult();
				SchedulePollCall([callback]() {
					callback(sql::Result::Ok,{});
				});
			}
		}
		catch(const sql::SQLException &e)
		{
			auto r = UpdateLastResult(e);
			SchedulePollCall([r,callback]() {
				callback(r,{});
			});
		}
	});
}


