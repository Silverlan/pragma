#ifndef __SQL_STATEMENT_HPP__
#define __SQL_STATEMENT_HPP__

#include <cinttypes>
#include <string>
#include <memory>
#include <functional>
#include <sharedutils/datastream.h>
#include "sql_result.hpp"
#include "sql_type.hpp"
#include "sql_value.hpp"

namespace sql
{
	class BaseConnection;
	class QueryData;
	class BaseStatement
		: public std::enable_shared_from_this<BaseStatement>
	{
	public:
		virtual ~BaseStatement();

		virtual bool IsMySQL() const=0;
		virtual bool IsSQLite() const=0;
		virtual bool IsPrepared() const=0;
		
		void Finalize(const std::function<void(sql::Result)> &callback=nullptr);
		void Step(const std::function<void(sql::Result)> &callback);
		void ExecuteQuery(const std::function<void(sql::Result)> &callback);
		void Execute(const std::function<void(sql::Result)> &callback);
		void Reset(const std::function<void(sql::Result)> &callback);
		void FindColumn(const std::string &colName,const std::function<void(sql::Result,uint32_t)> &callback);

		void IsReadOnly(int32_t column,const std::function<void(sql::Result,bool)> &callback) const;
		void GetType(int32_t column,const std::function<void(sql::Result,sql::Type)> &callback) const;
		void GetDouble(int32_t column,const std::function<void(sql::Result,long double)> &callback) const;
		void GetInt(int32_t column,const std::function<void(sql::Result,int32_t)> &callback) const;
		void GetInt64(int32_t column,const std::function<void(sql::Result,int64_t)> &callback) const;
		void GetText(int32_t column,const std::function<void(sql::Result,const std::string&)> &callback) const;
		void GetBlob(int32_t column,const std::function<void(sql::Result,const DataStream&)> &callback) const;
		void GetColumnCount(const std::function<void(sql::Result,int32_t)> &callback) const;
		void GetColumnName(int32_t column,const std::function<void(sql::Result,const std::string&)> &callback) const;
		void GetRowCount(const std::function<void(sql::Result,size_t)> &callback) const;

		//void GetBlob(int32_t column,const std::function<void(sql::Result,DataStream)> &callback);
		void ExecuteQueryAndFetchRows(const std::function<void(sql::Result,std::shared_ptr<QueryData>&)> &callback);

		void GetValue(int32_t column,PValue &val,const std::function<void(sql::Result)> &callback) const;
		void GetValue(int32_t column,sql::Type type,PValue &val,const std::function<void(sql::Result)> &callback) const;

		// Helper-function; Calls wait on the connection
		void Wait();
	protected:
		friend BaseConnection;
		BaseStatement(BaseConnection &con);

		virtual void ExecThreadCall(const std::function<void(sql::Result)> &errorHandle,const std::function<void(BaseConnection&)> &callback) const=0;

		virtual sql::Result AsyncFinalize()=0;
		virtual sql::Result AsyncStep()=0;
		virtual sql::Result AsyncExecuteQuery()=0;
		virtual sql::Result AsyncExecute()=0;
		virtual sql::Result AsyncReset()=0;
		virtual sql::Result AsyncFindColumn(const std::string &colName,uint32_t &colId)=0;

		virtual sql::Result AsyncIsReadOnly(int32_t column,bool &bReadOnly) const=0;
		virtual sql::Result AsyncGetType(int32_t column,sql::Type &type) const=0;
		virtual sql::Result AsyncGetDouble(int32_t column,long double &v) const=0;
		virtual sql::Result AsyncGetInt(int32_t column,int32_t &v) const=0;
		virtual sql::Result AsyncGetInt64(int32_t column,int64_t &v) const=0;
		virtual sql::Result AsyncGetText(int32_t column,std::string &v) const=0;
		virtual sql::Result AsyncGetBlob(int32_t column,DataStream &v) const=0;
		virtual sql::Result AsyncGetColumnCount(int32_t &colCount) const=0;
		virtual sql::Result AsyncGetColumnName(int32_t column,std::string &colName) const=0;
		virtual sql::Result AsyncGetRowCount(size_t &rowCount) const=0;
		sql::Result AsyncGetValue(int32_t column,PValue &val) const;
		sql::Result AsyncGetValue(int32_t column,PValue &val,sql::Type type) const;

		sql::Result AsyncExecuteQueryAndFetchRows(QueryData &rows);

		std::weak_ptr<BaseConnection> m_con = {};
	};

	class BasePreparedStatement
	{
	public:
		virtual bool IsPrepared() const=0;
		void BindDouble(int32_t column,double d,const std::function<void(sql::Result)> &callback);
		void BindInt(int32_t column,int32_t i,const std::function<void(sql::Result)> &callback);
		void BindInt64(int32_t column,int64_t i,const std::function<void(sql::Result)> &callback);
		void BindNull(int32_t column,const std::function<void(sql::Result)> &callback);
		void BindText(int32_t column,const std::string &text,const std::function<void(sql::Result)> &callback);
		void BindBlob(int32_t column,DataStream &ds,const std::function<void(sql::Result)> &callback);
		void BindValue(int32_t column,const sql::PValue &val,const std::function<void(sql::Result)> &callback);
		void ClearBindings(const std::function<void(sql::Result)> &callback);
	protected:
		friend BaseConnection;
		BasePreparedStatement()=default;

		virtual void ExecThreadCall(const std::function<void(sql::Result)> &errorHandle,const std::function<void(BaseConnection&)> &callback) const=0;

		virtual sql::Result AsyncBindDouble(int32_t column,double d)=0;
		virtual sql::Result AsyncBindInt(int32_t column,int32_t i)=0;
		virtual sql::Result AsyncBindInt64(int32_t column,int64_t i)=0;
		virtual sql::Result AsyncBindNull(int32_t column)=0;
		virtual sql::Result AsyncBindText(int32_t column,const std::string &text)=0;
		virtual sql::Result AsyncBindBlob(int32_t column,DataStream &ds)=0;
		virtual sql::Result AsyncClearBindings()=0;
		sql::Result AsyncBindValue(int32_t column,const sql::PValue &val);
	};
};

#endif
