#include "sqlite_statement.hpp"
#include "sqlite_connection.hpp"
#include <sqlite3.h>
#include <sharedutils/util_string.h>

sqlite::Statement::Statement(sqlite3_stmt *pStmt,sql::BaseConnection &con)
	: sql::BaseStatement(con),m_pStmt(pStmt)
{}
sqlite::Statement::~Statement() {AsyncFinalize();}
bool sqlite::Statement::IsMySQL() const {return false;}
bool sqlite::Statement::IsSQLite() const {return true;}
void sqlite::Statement::ExecThreadCall(const std::function<void(sql::Result)> &errorHandle,const std::function<void(sql::BaseConnection&)> &callback) const
{
	if(m_con.expired() == true)
	{
		errorHandle(sql::Result::Misuse);
		return;
	}
	m_con.lock()->ScheduleThreadCall([this,callback,errorHandle]() {
		auto &con = m_con.lock();
		if(m_pStmt == nullptr)
		{
			con->SchedulePollCall(std::bind(errorHandle,sql::Result::Misuse));
			return;
		}
		callback(static_cast<sqlite::Connection&>(*con));
	});
}
sql::Result sqlite::Statement::UpdateLastResult(bool bTranslateStepCodes) const
{
	if(m_con.expired())
		return sql::Result::Ok;
	return static_cast<sqlite::Connection&>(*m_con.lock()).UpdateLastResult(bTranslateStepCodes);
}
sql::Result sqlite::Statement::AsyncFinalize()
{
	auto r = static_cast<sql::Result>(sqlite3_finalize(m_pStmt));
	m_pStmt = nullptr;
	return UpdateLastResult();
}
sql::Result sqlite::Statement::AsyncStep()
{
	auto r = static_cast<sql::Result>(sqlite3_step(m_pStmt));
	return UpdateLastResult(false);
}
bool sqlite::Statement::IsPrepared() const {return false;}
sql::Result sqlite::Statement::AsyncExecuteQuery() {return AsyncExecute();}
sql::Result sqlite::Statement::AsyncExecute()
{
	auto r = sql::Result::Ok;
	while((r=AsyncStep()) == sql::Result::Row);
	if(r == sql::Result::Done)
		r = sql::Result::Ok;
	return r;
}
sql::Result sqlite::Statement::AsyncReset()
{
	if(m_pStmt == nullptr)
		return sql::Result::Misuse;
	auto r = static_cast<sql::Result>(sqlite3_reset(m_pStmt));
	return UpdateLastResult();
}
sql::Result sqlite::Statement::AsyncFindColumn(const std::string &colName,uint32_t &colId)
{
	auto numColumns = sqlite3_column_count(m_pStmt);
	for(auto i=decltype(numColumns){0};i<numColumns;++i)
	{
		auto *name = sqlite3_column_name(m_pStmt,i);
		if(ustring::compare(colName.c_str(),name,false) == true)
		{
			colId = i;
			return UpdateLastResult();
		}
	}
	return sql::Result::NotFound;
}
sql::Result sqlite::Statement::AsyncIsReadOnly(int32_t column,bool &bReadOnly) const
{
	bReadOnly = sqlite3_stmt_readonly(m_pStmt);
	return UpdateLastResult();
}
sql::Result sqlite::Statement::AsyncGetType(int32_t column,sql::Type &type) const
{
	auto liteType = sqlite3_column_type(m_pStmt,column -1);
	auto r = UpdateLastResult();
	switch(liteType)
	{
		case SQLITE_INTEGER:
			type = sql::Type::Integer;
			break;
		case SQLITE_FLOAT:
			type = sql::Type::Real;
			break;
		case SQLITE_BLOB:
			type = sql::Type::Binary;
			break;
		case SQLITE_NULL:
			type = sql::Type::Null;
			break;
		case SQLITE_TEXT:
			type = sql::Type::VarChar;
			break;
		default:
			return sql::Result::Internal;
	}
	return r;
}
sql::Result sqlite::Statement::AsyncGetDouble(int32_t column,long double &v) const
{
	v = sqlite3_column_double(m_pStmt,column -1);
	return UpdateLastResult();
}
sql::Result sqlite::Statement::AsyncGetInt(int32_t column,int32_t &v) const
{
	v = sqlite3_column_int(m_pStmt,column -1);
	return UpdateLastResult();
}
sql::Result sqlite::Statement::AsyncGetInt64(int32_t column,int64_t &v) const
{
	v = sqlite3_column_int64(m_pStmt,column -1);
	return UpdateLastResult();
}
sql::Result sqlite::Statement::AsyncGetText(int32_t column,std::string &v) const
{
	auto *text = sqlite3_column_text(m_pStmt,column -1);
	v = (text != nullptr) ? reinterpret_cast<const char*>(text) : "";
	return UpdateLastResult();
}
sql::Result sqlite::Statement::AsyncGetBlob(int32_t column,DataStream &v) const
{
	auto size = sqlite3_column_bytes(m_pStmt,column -1);
	auto *data = sqlite3_column_blob(m_pStmt,column -1);
	auto r = UpdateLastResult();
	if(data == nullptr || r != sql::Result::Ok)
		return r;
	v = DataStream(size);
	v->Write(reinterpret_cast<const uint8_t*>(data),size);
	v->SetOffset(0);
	return r;
}
sql::Result sqlite::Statement::AsyncGetColumnCount(int32_t &colCount) const
{
	colCount = sqlite3_column_count(m_pStmt);
	return UpdateLastResult();
}
sql::Result sqlite::Statement::AsyncGetColumnName(int32_t column,std::string &colName) const
{
	colName = sqlite3_column_name(m_pStmt,column -1);
	return UpdateLastResult();
}
sql::Result sqlite::Statement::AsyncGetRowCount(size_t &rowCount) const
{
	rowCount = 0;
	return sql::Result::Internal;
}
sql::Result sqlite::Statement::Reset()
{
	if(m_pStmt == nullptr)
		return sql::Result::Misuse;
	auto r = static_cast<sql::Result>(sqlite3_reset(m_pStmt));
	UpdateLastResult();
	return r;
}
void sqlite::Statement::GetBytes(int32_t column,const std::function<void(sql::Result,int32_t)> &callback) const
{
	ExecThreadCall(std::bind(callback,std::placeholders::_1,int32_t{}),[this,callback,column](sql::BaseConnection &con) {
		auto v = sqlite3_column_bytes(m_pStmt,column);
		UpdateLastResult();
		con.SchedulePollCall(std::bind(callback,sql::Result::Ok,v));
	});
}
std::string sqlite::Statement::GetSQLText() const
{
	auto *str = sqlite3_sql(m_pStmt);
	UpdateLastResult();
	if(str == nullptr)
		return "";
	return str;
}
std::string sqlite::Statement::GetExpandedSQLText() const
{
	auto *str = sqlite3_expanded_sql(m_pStmt);
	UpdateLastResult();
	if(str == nullptr)
		return "";
	auto r = std::string(str);
	sqlite3_free(str);
	return r;
}

std::string sqlite::Statement::GetDatabaseName(int32_t column) const {return sqlite3_column_database_name(m_pStmt,column);}
std::string sqlite::Statement::GetTableName(int32_t column) const {return sqlite3_column_table_name(m_pStmt,column);}
std::string sqlite::Statement::GetOriginName(int32_t column) const {return sqlite3_column_origin_name(m_pStmt,column);}

//////////////////////

sqlite::PreparedStatement::PreparedStatement(sqlite3_stmt *pStmt,sql::BaseConnection &con)
	: Statement(pStmt,con),sql::BasePreparedStatement()
{}
bool sqlite::PreparedStatement::IsPrepared() const {return true;}
void sqlite::PreparedStatement::ExecThreadCall(const std::function<void(sql::Result)> &errorHandle,const std::function<void(sql::BaseConnection&)> &callback) const {Statement::ExecThreadCall(errorHandle,callback);}
sql::Result sqlite::PreparedStatement::AsyncBindValue(const std::function<int32_t(void)> &bindFunc)
{
	auto r = static_cast<sql::Result>(bindFunc());
	return UpdateLastResult();
}
sql::Result sqlite::PreparedStatement::AsyncBindDouble(int32_t column,double d) {return AsyncBindValue(std::bind(&sqlite3_bind_double,m_pStmt,column,d));}
sql::Result sqlite::PreparedStatement::AsyncBindInt(int32_t column,int32_t i) {return AsyncBindValue(std::bind(&sqlite3_bind_int,m_pStmt,column,i));}
sql::Result sqlite::PreparedStatement::AsyncBindInt64(int32_t column,int64_t i) {return AsyncBindValue(std::bind(&sqlite3_bind_int64,m_pStmt,column,i));}
sql::Result sqlite::PreparedStatement::AsyncBindNull(int32_t column) {return AsyncBindValue(std::bind(&sqlite3_bind_null,m_pStmt,column));}
sql::Result sqlite::PreparedStatement::AsyncBindText(int32_t column,const std::string &text) {return AsyncBindValue(std::bind(&sqlite3_bind_text,m_pStmt,column,text.c_str(),-1,SQLITE_TRANSIENT));}
sql::Result sqlite::PreparedStatement::AsyncBindBlob(int32_t column,DataStream &ds) {return AsyncBindValue(std::bind(&sqlite3_bind_blob64,m_pStmt,column,ds->GetData(),ds->GetSize(),SQLITE_TRANSIENT));}
sql::Result sqlite::PreparedStatement::AsyncClearBindings() {return AsyncBindValue(std::bind(&sqlite3_clear_bindings,m_pStmt));}

sql::Result sqlite::PreparedStatement::Reset()
{
	auto r = Statement::Reset();
	auto r2 = static_cast<sql::Result>(sqlite3_clear_bindings(m_pStmt));
	UpdateLastResult();
	return (r == sql::Result::Ok) ? r2 : r;
}

