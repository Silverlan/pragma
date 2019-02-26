#include "sql_statement.hpp"
#include "sql_connection.hpp"

sql::BaseStatement::BaseStatement(BaseConnection &con)
	: m_con(con.shared_from_this())
{}

sql::BaseStatement::~BaseStatement() {}

void sql::BaseStatement::Wait()
{
	if(m_con.expired() == true)
		return;
	m_con.lock()->Wait();
}
void sql::BaseStatement::Finalize(const std::function<void(sql::Result)> &callback)
{
	ExecThreadCall(std::bind(callback,std::placeholders::_1),[this,callback](BaseConnection &con) {
		auto r = AsyncFinalize();
		if(callback != nullptr)
			con.SchedulePollCall(std::bind(callback,r));
	});
}
void sql::BaseStatement::Step(const std::function<void(sql::Result)> &callback)
{
	ExecThreadCall(std::bind(callback,std::placeholders::_1),[this,callback](BaseConnection &con) {
		auto r = AsyncStep();
		con.SchedulePollCall(std::bind(callback,r));
	});
}
void sql::BaseStatement::ExecuteQuery(const std::function<void(sql::Result)> &callback)
{
	ExecThreadCall(std::bind(callback,std::placeholders::_1),[this,callback](BaseConnection &con) {
		auto r = AsyncExecuteQuery();
		con.SchedulePollCall(std::bind(callback,r));
	});
}
void sql::BaseStatement::Execute(const std::function<void(sql::Result)> &callback)
{
	ExecThreadCall(std::bind(callback,std::placeholders::_1),[this,callback](BaseConnection &con) {
		auto r = AsyncExecute();
		con.SchedulePollCall(std::bind(callback,r));
	});
}
void sql::BaseStatement::Reset(const std::function<void(sql::Result)> &callback)
{
	ExecThreadCall(std::bind(callback,std::placeholders::_1),[this,callback](BaseConnection &con) {
		auto r = AsyncReset();
		con.SchedulePollCall(std::bind(callback,r));
	});
}
void sql::BaseStatement::FindColumn(const std::string &colName,const std::function<void(sql::Result,uint32_t)> &callback)
{
	ExecThreadCall(std::bind(callback,std::placeholders::_1,0u),[this,callback,colName](BaseConnection &con) {
		auto colId = 0u;
		auto r = AsyncFindColumn(colName,colId);
		con.SchedulePollCall(std::bind(callback,r,colId));
	});
}
void sql::BaseStatement::GetType(int32_t column,const std::function<void(sql::Result,sql::Type)> &callback) const
{
	ExecThreadCall(std::bind(callback,std::placeholders::_1,sql::Type::Initial),[this,callback,column](BaseConnection &con) {
		auto type = sql::Type::Initial;
		auto r = AsyncGetType(column,type);
		con.SchedulePollCall(std::bind(callback,r,type));
	});
}
void sql::BaseStatement::IsReadOnly(int32_t column,const std::function<void(sql::Result,bool)> &callback) const
{
	ExecThreadCall(std::bind(callback,std::placeholders::_1,true),[this,callback,column](BaseConnection &con) {
		auto bReadOnly = true;
		auto r = AsyncIsReadOnly(column,bReadOnly);
		con.SchedulePollCall(std::bind(callback,r,bReadOnly));
	});
}
void sql::BaseStatement::GetDouble(int32_t column,const std::function<void(sql::Result,long double)> &callback) const
{
	ExecThreadCall(std::bind(callback,std::placeholders::_1,0.0l),[this,callback,column](BaseConnection &con) {
		auto v = 0.0l;
		auto r = AsyncGetDouble(column,v);
		con.SchedulePollCall(std::bind(callback,r,v));
	});
}
void sql::BaseStatement::GetInt(int32_t column,const std::function<void(sql::Result,int32_t)> &callback) const
{
	ExecThreadCall(std::bind(callback,std::placeholders::_1,0),[this,callback,column](BaseConnection &con) {
		auto v = 0;
		auto r = AsyncGetInt(column,v);
		con.SchedulePollCall(std::bind(callback,r,v));
	});
}
void sql::BaseStatement::GetInt64(int32_t column,const std::function<void(sql::Result,int64_t)> &callback) const
{
	ExecThreadCall(std::bind(callback,std::placeholders::_1,0ll),[this,callback,column](BaseConnection &con) {
		auto v = 0ll;
		auto r = AsyncGetInt64(column,v);
		con.SchedulePollCall(std::bind(callback,r,v));
	});
}
void sql::BaseStatement::GetText(int32_t column,const std::function<void(sql::Result,const std::string&)> &callback) const
{
	ExecThreadCall(std::bind(callback,std::placeholders::_1,std::string{}),[this,callback,column](BaseConnection &con) {
		auto v = std::string{};
		auto r = AsyncGetText(column,v);
		con.SchedulePollCall(std::bind(callback,r,v));
	});
}
void sql::BaseStatement::GetBlob(int32_t column,const std::function<void(sql::Result,const DataStream&)> &callback) const
{
	ExecThreadCall(std::bind(callback,std::placeholders::_1,DataStream{}),[this,callback,column](BaseConnection &con) {
		auto v = DataStream{};
		auto r = AsyncGetBlob(column,v);
		con.SchedulePollCall(std::bind(callback,r,v));
	});
}
void sql::BaseStatement::GetColumnCount(const std::function<void(sql::Result,int32_t)> &callback) const
{
	ExecThreadCall(std::bind(callback,std::placeholders::_1,0),[this,callback](BaseConnection &con) {
		auto v = 0;
		auto r = AsyncGetColumnCount(v);
		con.SchedulePollCall(std::bind(callback,r,v));
	});
}
void sql::BaseStatement::GetColumnName(int32_t column,const std::function<void(sql::Result,const std::string&)> &callback) const
{
	ExecThreadCall(std::bind(callback,std::placeholders::_1,std::string{}),[this,callback,column](BaseConnection &con) {
		auto v = std::string{};
		auto r = AsyncGetColumnName(column,v);
		con.SchedulePollCall(std::bind(callback,r,v));
	});
}
void sql::BaseStatement::GetRowCount(const std::function<void(sql::Result,size_t)> &callback) const
{
	ExecThreadCall(std::bind(callback,std::placeholders::_1,size_t{}),[this,callback](BaseConnection &con) {
		auto v = size_t{};
		auto r = AsyncGetRowCount(v);
		con.SchedulePollCall(std::bind(callback,r,v));
	});
}
void sql::BaseStatement::ExecuteQueryAndFetchRows(const std::function<void(sql::Result,std::shared_ptr<QueryData>&)> &callback)
{
	std::shared_ptr<QueryData> r{};
	ExecThreadCall(std::bind(callback,std::placeholders::_1,r),[this,callback](BaseConnection &con) {
		auto data = std::make_shared<QueryData>();
		auto r = AsyncExecuteQueryAndFetchRows(*data);
		con.SchedulePollCall(std::bind(callback,r,std::ref(data)));
	});
}
sql::Result sql::BaseStatement::AsyncGetValue(int32_t column,PValue &val,sql::Type type) const
{
	auto r = sql::Result::Ok;
	switch(type)
	{
		case sql::Type::BigInt:
		{
			int64_t v = -1;
			r = AsyncGetInt64(column,v);
			if(r != sql::Result::Ok)
				return r;
			val = std::make_shared<sql::Value>(v);
			break;
		}
		case sql::Type::Bit:
		case sql::Type::Enum:
		case sql::Type::Char:
		case sql::Type::Integer:
		case sql::Type::Date:
		case sql::Type::MediumInt:
		case sql::Type::Numeric:
		case sql::Type::SmallInt:
		case sql::Type::Time:
		case sql::Type::TimeStamp:
		case sql::Type::TinyInt:
		case sql::Type::Year:
		{
			int32_t v = -1;
			r = AsyncGetInt(column,v);
			if(r != sql::Result::Ok)
				return r;
			val = std::make_shared<sql::Value>(v);
			break;
		}
		case sql::Type::Binary:
		{
			DataStream v {};
			r = AsyncGetBlob(column,v);
			if(r != sql::Result::Ok)
				return r;
			val = std::make_shared<sql::Value>(v);
			break;
		}
		case sql::Type::Decimal:
		case sql::Type::Double:
		case sql::Type::Real:
		{
			auto v = 0.0l;
			r = AsyncGetDouble(column,v);
			if(r != sql::Result::Ok)
				return r;
			val = std::make_shared<sql::Value>(v);
			break;
		}
		case sql::Type::VarChar:
		{
			std::string v {};
			r = AsyncGetText(column,v);
			if(r != sql::Result::Ok)
				return r;
			val = std::make_shared<sql::Value>(v);
			break;
		}
		case sql::Type::Geometry:
		case sql::Type::LongVarBinary:
		case sql::Type::LongVarChar:
		case sql::Type::Null:
		case sql::Type::Set:
		case sql::Type::Unknown:
		case sql::Type::VarBinary:
		{
			val = std::make_shared<sql::Value>(nullptr);
			break;
		}
		default:
			return sql::Result::Internal;
	}
	return r;
}
sql::Result sql::BaseStatement::AsyncGetValue(int32_t column,PValue &val) const
{
	auto type = sql::Type::Initial;
	auto r = AsyncGetType(column,type);
	if(r != sql::Result::Ok)
		return r;
	return AsyncGetValue(column,val,type);
}
sql::Result sql::BaseStatement::AsyncExecuteQueryAndFetchRows(QueryData &queryData)
{
	auto r = AsyncExecuteQuery();
	if(r != sql::Result::Ok)
		return r;
	auto colCount = 0;
	r = AsyncGetColumnCount(colCount);
	if(r != sql::Result::Ok)
		return r;
	queryData.columnNames.resize(colCount);
	for(auto i=decltype(colCount){0};i<colCount;++i)
	{
		r = AsyncGetColumnName(i +1,queryData.columnNames.at(i));
		if(r != sql::Result::Ok)
			return r;
	}
	std::vector<sql::Type> types {};
	types.reserve(colCount);
	auto rowCount = size_t{0};
	r = AsyncGetRowCount(rowCount);
	queryData.rows.reserve(rowCount);
	auto rowId = 0;
	while((r=AsyncStep()) == sql::Result::Row)
	{
		if(rowId == 0)
		{
			// We can't fetch the types in SQLite until we've stepped at least once
			for(auto i=decltype(colCount){0};i<colCount;++i)
			{
				auto type = sql::Type::Initial;
				r = AsyncGetType(i +1,type);
				if(r != sql::Result::Ok)
					return r;
				types.push_back(type);
			}
		}
		if(queryData.rows.capacity() == queryData.rows.size()) // In case AsyncGetRowCount does not return correct row count
			queryData.rows.reserve(queryData.rows.size() +100);
		queryData.rows.push_back({});
		auto &rowValues = queryData.rows.back();
		rowValues.reserve(colCount);
		for(auto i=decltype(colCount){0};i<colCount;++i)
		{
			auto val = sql::PValue{};
			r = AsyncGetValue(i +1,val,types.at(i));
			if(r != sql::Result::Ok)
				return r;
			rowValues.push_back(val);
		}
		++rowId;
	}
	if(r != sql::Result::Done)
		return r;
	return sql::Result::Ok;
}
void sql::BasePreparedStatement::BindDouble(int32_t column,double d,const std::function<void(sql::Result)> &callback)
{
	ExecThreadCall(std::bind(callback,std::placeholders::_1),[this,callback,column,d](BaseConnection &con) {
		auto r = AsyncBindDouble(column,d);
		con.SchedulePollCall(std::bind(callback,r));
	});
}
void sql::BasePreparedStatement::BindInt(int32_t column,int32_t i,const std::function<void(sql::Result)> &callback)
{
	ExecThreadCall(std::bind(callback,std::placeholders::_1),[this,callback,column,i](BaseConnection &con) {
		auto r = AsyncBindInt(column,i);
		con.SchedulePollCall(std::bind(callback,r));
	});
}
void sql::BasePreparedStatement::BindInt64(int32_t column,int64_t i,const std::function<void(sql::Result)> &callback)
{
	ExecThreadCall(std::bind(callback,std::placeholders::_1),[this,callback,column,i](BaseConnection &con) {
		auto r = AsyncBindInt64(column,i);
		con.SchedulePollCall(std::bind(callback,r));
	});
}
void sql::BasePreparedStatement::BindNull(int32_t column,const std::function<void(sql::Result)> &callback)
{
	ExecThreadCall(std::bind(callback,std::placeholders::_1),[this,callback,column](BaseConnection &con) {
		auto r = AsyncBindNull(column);
		con.SchedulePollCall(std::bind(callback,r));
	});
}
void sql::BasePreparedStatement::BindText(int32_t column,const std::string &text,const std::function<void(sql::Result)> &callback)
{
	ExecThreadCall(std::bind(callback,std::placeholders::_1),[this,callback,column,text](BaseConnection &con) {
		auto r = AsyncBindText(column,text);
		con.SchedulePollCall(std::bind(callback,r));
	});
}
void sql::BasePreparedStatement::BindBlob(int32_t column,DataStream &ds,const std::function<void(sql::Result)> &callback)
{
	ExecThreadCall(std::bind(callback,std::placeholders::_1),[this,callback,column,ds](BaseConnection &con) mutable {
		auto r = AsyncBindBlob(column,ds);
		con.SchedulePollCall(std::bind(callback,r));
	});
}
void sql::BasePreparedStatement::BindValue(int32_t column,const sql::PValue &val,const std::function<void(sql::Result)> &callback)
{
	ExecThreadCall(std::bind(callback,std::placeholders::_1),[this,callback,column,val](BaseConnection &con) mutable {
		auto r = AsyncBindValue(column,val);
		con.SchedulePollCall(std::bind(callback,r));
	});
}
void sql::BasePreparedStatement::ClearBindings(const std::function<void(sql::Result)> &callback)
{
	ExecThreadCall(std::bind(callback,std::placeholders::_1),[this,callback](BaseConnection &con) mutable {
		auto r = AsyncClearBindings();
		con.SchedulePollCall(std::bind(callback,r));
	});
}
sql::Result sql::BasePreparedStatement::AsyncBindValue(int32_t column,const sql::PValue &val)
{
	switch(val->GetType())
	{
		case sql::Type::Double:
			return AsyncBindDouble(column,val->GetDouble());
		case sql::Type::Integer:
			return AsyncBindInt(column,val->GetInt());
		case sql::Type::BigInt:
			return AsyncBindInt64(column,val->GetInt64());
		case sql::Type::VarChar:
			return AsyncBindText(column,val->GetText());
		case sql::Type::Binary:
			return AsyncBindBlob(column,const_cast<DataStream&>(val->GetBlob()));
		case sql::Type::Null:
			return AsyncBindNull(column);
	}
	return sql::Result::Misuse;
}

