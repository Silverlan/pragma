#include "mysql_statement.hpp"
#include "mysql_connection.hpp"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/parameter_metadata.h>

mysql::Statement::Statement(sql::Statement *pStmt,sql::BaseConnection &con)
	: sql::BaseStatement(con),m_pStmt(pStmt)
{}
mysql::Statement::~Statement() {AsyncFinalize();}
bool mysql::Statement::IsMySQL() const {return true;}
bool mysql::Statement::IsSQLite() const {return false;}
sql::Result mysql::Statement::AsyncGetColumnCount(int32_t &colCount) const
{
	try
	{
		auto *meta = m_result->getMetaData();
		if(meta == nullptr)
			throw sql::SQLException("","",umath::to_integral(sql::Result::Internal));
		colCount = meta->getColumnCount();
		UpdateLastResult();
		return sql::Result::Ok;
	}
	catch(const sql::SQLException &e) {return UpdateLastResult(e);}
	return sql::Result::Ok;
}
sql::Result mysql::Statement::AsyncFinalize()
{
	try
	{
		m_pStmt = nullptr;
		return sql::Result::Ok;
	}
	catch(const sql::SQLException &e) {return UpdateLastResult(e);}
	return sql::Result::Ok;
}
sql::Result mysql::Statement::AsyncStep()
{
	try
	{
		auto r = m_result->next();
		return (r == true) ? sql::Result::Row : sql::Result::Done;
	}
	catch(const sql::SQLException &e) {return UpdateLastResult(e);}
	return sql::Result::Ok;
}
sql::Result mysql::Statement::AsyncReset()
{
	try
	{
		Reset();
		return sql::Result::Ok;
	}
	catch(const sql::SQLException &e) {return UpdateLastResult(e);}
	return sql::Result::Ok;
}
sql::Result mysql::Statement::AsyncFindColumn(const std::string &colName,uint32_t &colId)
{
	try
	{
		colId = m_result->findColumn(colName);
		return UpdateLastResult();
	}
	catch(const sql::SQLException &e) {return UpdateLastResult(e);}
	return sql::Result::Ok;
}

sql::Result mysql::Statement::AsyncIsReadOnly(int32_t column,bool &bReadOnly) const
{
	try
	{
		auto *meta = m_result->getMetaData();
		if(meta == nullptr)
			throw sql::SQLException("","",umath::to_integral(sql::Result::Internal));
		bReadOnly = !meta->isWritable(column);
		return UpdateLastResult();
	}
	catch(const sql::SQLException &e) {return UpdateLastResult(e);}
	return sql::Result::Ok;
}
sql::Result mysql::Statement::AsyncGetType(int32_t column,sql::Type &type) const
{
	try
	{
		auto *meta = m_result->getMetaData();
		if(meta == nullptr)
			throw sql::SQLException("","",umath::to_integral(sql::Result::Internal));
		type = static_cast<sql::Type>(meta->getColumnType(column));
		return UpdateLastResult();
	}
	catch(const sql::SQLException &e) {return UpdateLastResult(e);}
	return sql::Result::Ok;
}
sql::Result mysql::Statement::AsyncGetDouble(int32_t column,long double &v) const
{
	try
	{
		v = m_result->getDouble(column);
		return UpdateLastResult();
	}
	catch(const sql::SQLException &e) {return UpdateLastResult(e);}
	return sql::Result::Ok;
}
sql::Result mysql::Statement::AsyncGetInt(int32_t column,int32_t &v) const
{
	try
	{
		v = m_result->getInt(column);
		return UpdateLastResult();
	}
	catch(const sql::SQLException &e) {return UpdateLastResult(e);}
	return sql::Result::Ok;
}
sql::Result mysql::Statement::AsyncGetInt64(int32_t column,int64_t &v) const
{
	try
	{
		v = m_result->getInt64(column);
		return UpdateLastResult();
	}
	catch(const sql::SQLException &e) {return UpdateLastResult(e);}
	return sql::Result::Ok;
}
sql::Result mysql::Statement::AsyncGetText(int32_t column,std::string &v) const
{
	try
	{
		v = m_result->getString(column);
		return UpdateLastResult();
	}
	catch(const sql::SQLException &e) {return UpdateLastResult(e);}
	return sql::Result::Ok;
}
sql::Result mysql::Statement::AsyncGetBlob(int32_t column,DataStream &v) const
{
	try
	{
		auto *blob = m_result->getBlob(column);
		auto r = (blob != nullptr) ? sql::Result::Ok : sql::Result::Mismatch;
		DataStream ds {};
		if(r == sql::Result::Ok)
		{
			std::streamsize readSize = 0ull;
			auto offset = ds->GetOffset();
			do
			{
				// TODO: Assign ifstream directly to DataStream to avoid having to copy
				// data in the first place
				auto size = ds->GetSize();
				offset = ds->GetOffset();
				if(size -offset < 1'024)
					ds->Resize(offset +1'024);
				auto *data = ds->GetData() +ds->GetOffset();
				readSize = blob->readsome(reinterpret_cast<char*>(data),1'024);
			}
			while(readSize > 0);
		}
		v = ds;
		return UpdateLastResult();
	}
	catch(const sql::SQLException &e) {return UpdateLastResult(e);}
	return sql::Result::Ok;
}
sql::Result mysql::Statement::AsyncGetColumnName(int32_t column,std::string &colName) const
{
	try
	{
		auto *meta = m_result->getMetaData();
		if(meta == nullptr)
			throw sql::SQLException("","",umath::to_integral(sql::Result::Internal));
		colName = meta->getColumnName(column);
		return UpdateLastResult();
	}
	catch(const sql::SQLException &e) {return UpdateLastResult(e);}
	return sql::Result::Ok;
}
sql::Result mysql::Statement::AsyncGetRowCount(size_t &rowCount) const
{
	try
	{
		rowCount = m_result->rowsCount();
		return UpdateLastResult();
	}
	catch(const sql::SQLException &e) {return UpdateLastResult(e);}
	return sql::Result::Ok;
}

void mysql::Statement::ExecThreadCall(const std::function<void(sql::Result)> &errorHandle,const std::function<void(sql::BaseConnection&)> &callback) const
//void mysql::Statement::ExecThreadCall(const std::function<void(sql::Result)> &errorHandle,const std::function<void(mysql::Connection&)> &callback,bool bCheckResult) const
{
	if(m_con.expired() == true)
	{
		errorHandle(sql::Result::Misuse);
		return;
	}
	m_con.lock()->ScheduleThreadCall([this,errorHandle,callback]() {
		auto &con = m_con.lock();
		if(m_result == nullptr)
		{
			con->SchedulePollCall([errorHandle]() {
				errorHandle(sql::Result::Misuse);
			});
			return;
		}
		try {callback(static_cast<mysql::Connection&>(*con));}
		catch(const sql::SQLException &e)
		{
			auto r = UpdateLastResult(e);
			con->SchedulePollCall([errorHandle,r]() {
				errorHandle(static_cast<sql::Result>(r));
			});
		}
	});
}

void mysql::Statement::SetQuery(const std::string &query) {m_query = query;}
bool mysql::Statement::IsPrepared() const {return false;}
sql::Result mysql::Statement::AsyncExecuteQuery()
{
	try
	{
		auto *result = m_pStmt->executeQuery(m_query);
		m_result = std::unique_ptr<sql::ResultSet>(result);
		UpdateLastResult();
	}
	catch(const sql::SQLException &e) {return UpdateLastResult(e);}
	return sql::Result::Ok;
}
sql::Result mysql::Statement::AsyncExecute()
{
	try
	{
		auto result = (m_pStmt->execute(m_query) == true) ? sql::Result::Ok : sql::Result::Error;
		UpdateLastResult();
	}
	catch(const sql::SQLException &e) {return UpdateLastResult(e);}
	return sql::Result::Ok;
}
sql::Result mysql::Statement::UpdateLastResult(const sql::SQLException &e) const
{
	if(m_con.expired())
		return sql::Result::Ok;
	return static_cast<Connection&>(*m_con.lock()).UpdateLastResult(e);
}
sql::Result mysql::Statement::UpdateLastResult() const
{
	if(m_con.expired())
		return sql::Result::Ok;
	return static_cast<Connection&>(*m_con.lock()).UpdateLastResult();
}
void mysql::Statement::Reset()
{
	m_result = nullptr;
	UpdateLastResult();
}
///////////////////

mysql::PreparedStatement::PreparedStatement(sql::PreparedStatement *pStmt,sql::BaseConnection &con)
	: Statement(pStmt,con),sql::BasePreparedStatement(),m_pStmtPrep(*pStmt)
{}
void mysql::PreparedStatement::ExecThreadCall(const std::function<void(sql::Result)> &errorHandle,const std::function<void(sql::BaseConnection&)> &callback) const {Statement::ExecThreadCall(errorHandle,callback);}
sql::Result mysql::PreparedStatement::AsyncBindValue(const std::function<void(void)> &bindFunc)
{
	bindFunc();
	return UpdateLastResult();
}
bool mysql::PreparedStatement::IsPrepared() const {return true;}
sql::Result mysql::PreparedStatement::AsyncBindDouble(int32_t column,double d) {return AsyncBindValue(std::bind(&sql::PreparedStatement::setDouble,&m_pStmtPrep,column,d));}
sql::Result mysql::PreparedStatement::AsyncBindInt(int32_t column,int32_t i) {return AsyncBindValue(std::bind(&sql::PreparedStatement::setInt,&m_pStmtPrep,column,i));}
sql::Result mysql::PreparedStatement::AsyncBindInt64(int32_t column,int64_t i) {return AsyncBindValue(std::bind(&sql::PreparedStatement::setInt64,&m_pStmtPrep,column,i));}
sql::Result mysql::PreparedStatement::AsyncBindNull(int32_t column) {return AsyncBindValue(std::bind(&sql::PreparedStatement::setNull,&m_pStmtPrep,column,0));}
sql::Result mysql::PreparedStatement::AsyncBindText(int32_t column,const std::string &text) {return AsyncBindValue(std::bind(&sql::PreparedStatement::setString,&m_pStmtPrep,column,std::ref(text)));}
sql::Result mysql::PreparedStatement::AsyncBindBlob(int32_t column,DataStream &ds)
{
	struct membuf : std::streambuf
	{
		membuf(char *begin,char *end) {
			this->setg(begin,begin,end);
		}
	};
	return AsyncBindValue([this,column,ds]() mutable {
		auto *data = reinterpret_cast<char*>(ds->GetData());
		membuf sbuf(data,data +ds->GetSize());
		std::istream s(&sbuf);
		m_pStmtPrep.setBlob(column,&s);
	});
}
sql::Result mysql::PreparedStatement::AsyncClearBindings() {return AsyncBindValue(std::bind(&sql::PreparedStatement::clearParameters,&m_pStmtPrep));}
sql::Result mysql::PreparedStatement::AsyncExecuteQuery()
{
	try
	{
		auto *result = m_pStmtPrep.executeQuery();
		m_result = std::unique_ptr<sql::ResultSet>(result);
		UpdateLastResult();
	}
	catch(const sql::SQLException &e) {return UpdateLastResult(e);}
	return sql::Result::Ok;
}
sql::Result mysql::PreparedStatement::AsyncExecute()
{
	try
	{
		auto result = (m_pStmtPrep.execute() == true) ? sql::Result::Ok : sql::Result::Error;
		UpdateLastResult();
	}
	catch(const sql::SQLException &e) {return UpdateLastResult(e);}
	return sql::Result::Ok;
}
void mysql::PreparedStatement::Reset()
{
	Statement::Reset();
	m_pStmtPrep.clearParameters();
	UpdateLastResult();
}

