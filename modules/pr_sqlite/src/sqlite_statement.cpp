#include "sqlite_statement.hpp"
#include "sqlite_connection.hpp"
#include <sqlite3.h>

sqlite::Statement::Statement(sqlite3_stmt *pStmt,Connection &con)
	: m_pStmt(pStmt),m_con(con.shared_from_this())
{}
sqlite::Statement::~Statement() {Finalize();}
int32_t sqlite::Statement::Finalize()
{
	if(m_pStmt == nullptr)
		return SQLITE_OK;
	auto r = sqlite3_finalize(m_pStmt);
	m_pStmt = nullptr;
	return r;
}
int32_t sqlite::Statement::Step()
{
	if(m_pStmt == nullptr)
		return SQLITE_INTERNAL;
	return sqlite3_step(m_pStmt);
}
void sqlite::Statement::Step(const std::function<void(int32_t)> &callback)
{
	if(m_pStmt == nullptr)
	{
		callback(SQLITE_INTERNAL);
		return;
	}
	auto con = m_con;
	std::weak_ptr<Statement> pStatement = shared_from_this();
	m_con.lock()->ScheduleThreadCall([this,callback,pStatement,con]() {
		if(pStatement.expired() == true)
		{
			auto pCon = con.lock();
			if(pCon != nullptr)
				pCon->SchedulePollCall([callback]() {callback(SQLITE_MISUSE);});
			return;
		}
		auto r = Step();
		auto pCon = con.lock();
		if(pCon != nullptr)
			pCon->SchedulePollCall([r,callback]() {callback(r);});
	});
}
int32_t sqlite::Statement::Reset()
{
	if(m_pStmt == nullptr)
		return SQLITE_INTERNAL;
	return sqlite3_reset(m_pStmt);
}
int32_t sqlite::Statement::GetType(int32_t column) const {return sqlite3_column_type(m_pStmt,column);}
int32_t sqlite::Statement::GetBytes(int32_t column) const {return sqlite3_column_bytes(m_pStmt,column);}
double sqlite::Statement::GetDouble(int32_t column) const {return sqlite3_column_double(m_pStmt,column);}
int32_t sqlite::Statement::GetInt(int32_t column) const {return sqlite3_column_int(m_pStmt,column);}
int64_t sqlite::Statement::GetInt64(int32_t column) const {return sqlite3_column_int64(m_pStmt,column);}
std::string sqlite::Statement::GetText(int32_t column) const
{
	auto *text = sqlite3_column_text(m_pStmt,column);
	if(text == nullptr)
		return "";
	return reinterpret_cast<const char*>(text);
}
DataStream sqlite::Statement::GetBlob(int32_t column) const
{
	auto size = sqlite3_column_bytes(m_pStmt,column);
	auto *data = sqlite3_column_blob(m_pStmt,column);
	DataStream r(size);
	r->Write(reinterpret_cast<const uint8_t*>(data),size);
	r->SetOffset(0);
	return r;
}
void sqlite::Statement::GetBlob(int32_t column,const std::function<void(DataStream)> &callback)
{
	if(m_con.expired() == true)
	{
		callback(nullptr);
		return;
	}
	auto con = m_con;
	std::weak_ptr<Statement> pStatement = shared_from_this();
	m_con.lock()->ScheduleThreadCall([this,column,callback,pStatement,con]() {
		if(pStatement.expired() == true)
		{
			auto pCon = con.lock();
			if(pCon != nullptr)
				pCon->SchedulePollCall([callback]() {callback(nullptr);});
			return;
		}
		auto ds = GetBlob(column);
		auto pCon = con.lock();
		if(pCon != nullptr)
			pCon->SchedulePollCall([ds,callback]() {callback(ds);});
	});
}
int32_t sqlite::Statement::GetCount() const {return sqlite3_column_count(m_pStmt);}
std::string sqlite::Statement::GetColumnName(int32_t column) const {return sqlite3_column_name(m_pStmt,column);}
int32_t sqlite::Statement::BindDouble(int32_t column,double d) {return sqlite3_bind_double(m_pStmt,column,d);}
int32_t sqlite::Statement::BindInt(int32_t column,int32_t i) {return sqlite3_bind_int(m_pStmt,column,i);}
int32_t sqlite::Statement::BindInt64(int32_t column,int64_t i) {return sqlite3_bind_int64(m_pStmt,column,i);}
int32_t sqlite::Statement::BindNull(int32_t column) {return sqlite3_bind_null(m_pStmt,column);}
int32_t sqlite::Statement::BindText(int32_t column,const std::string &text) {return sqlite3_bind_text(m_pStmt,column,text.c_str(),-1,SQLITE_TRANSIENT);}
int32_t sqlite::Statement::BindBlob(int32_t column,DataStream &ds) {return sqlite3_bind_blob64(m_pStmt,column,ds->GetData(),ds->GetSize(),SQLITE_TRANSIENT);}
int32_t sqlite::Statement::ClearBindings() {return sqlite3_clear_bindings(m_pStmt);}
bool sqlite::Statement::IsReadOnly() const {return (sqlite3_stmt_readonly(m_pStmt)) != 0;}
std::string sqlite::Statement::GetSQLText() const
{
	auto *str = sqlite3_sql(m_pStmt);
	if(str == nullptr)
		return "";
	return str;
}
std::string sqlite::Statement::GetExpandedSQLText() const
{
	auto *str = sqlite3_expanded_sql(m_pStmt);
	if(str == nullptr)
		return "";
	auto r = std::string(str);
	sqlite3_free(str);
	return r;
}

std::string sqlite::Statement::GetDatabaseName(int32_t column) const {return sqlite3_column_database_name(m_pStmt,column);}
std::string sqlite::Statement::GetTableName(int32_t column) const {return sqlite3_column_table_name(m_pStmt,column);}
std::string sqlite::Statement::GetOriginName(int32_t column) const {return sqlite3_column_origin_name(m_pStmt,column);}
