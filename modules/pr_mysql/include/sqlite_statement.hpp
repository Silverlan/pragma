#ifndef __SQLITE_STATEMENT_HPP__
#define __SQLITE_STATEMENT_HPP__

#include "sql_statement.hpp"

struct sqlite3_stmt;
namespace sqlite
{
	class Connection;
	class Statement
		: public sql::BaseStatement
	{
	public:
		Statement(sqlite3_stmt *pStmt,sql::BaseConnection &con);
		// Statement will be destroyed on thread!
		virtual ~Statement() override;

		virtual bool IsMySQL() const override;
		virtual bool IsSQLite() const override;
		virtual bool IsPrepared() const override;

		void GetBytes(int32_t column,const std::function<void(sql::Result,int32_t)> &callback) const;
		std::string GetSQLText() const;
		std::string GetExpandedSQLText() const;

		std::string GetDatabaseName(int32_t column) const;
		std::string GetTableName(int32_t column) const;
		std::string GetOriginName(int32_t column) const;
	protected:
		virtual sql::Result Reset();

		virtual void ExecThreadCall(const std::function<void(sql::Result)> &errorHandle,const std::function<void(sql::BaseConnection&)> &callback) const override;
		virtual sql::Result AsyncFinalize() override;
		virtual sql::Result AsyncStep() override;
		virtual sql::Result AsyncExecuteQuery() override;
		virtual sql::Result AsyncExecute() override;
		virtual sql::Result AsyncReset() override;
		virtual sql::Result AsyncFindColumn(const std::string &colName,uint32_t &colId) override;

		virtual sql::Result AsyncIsReadOnly(int32_t column,bool &bReadOnly) const override;
		virtual sql::Result AsyncGetType(int32_t column,sql::Type &type) const override;
		virtual sql::Result AsyncGetDouble(int32_t column,long double &v) const override;
		virtual sql::Result AsyncGetInt(int32_t column,int32_t &v) const override;
		virtual sql::Result AsyncGetInt64(int32_t column,int64_t &v) const override;
		virtual sql::Result AsyncGetText(int32_t column,std::string &v) const override;
		virtual sql::Result AsyncGetBlob(int32_t column,DataStream &v) const override;
		virtual sql::Result AsyncGetColumnCount(int32_t &colCount) const override;
		virtual sql::Result AsyncGetColumnName(int32_t column,std::string &colName) const override;
		virtual sql::Result AsyncGetRowCount(size_t &rowCount) const override;

		sql::Result UpdateLastResult(bool bTranslateStepCodes=true) const;
		sqlite3_stmt *m_pStmt = nullptr;
	};
	class PreparedStatement
		: public Statement,
		public sql::BasePreparedStatement
	{
	public:
		PreparedStatement(sqlite3_stmt *pStmt,sql::BaseConnection &con);
		virtual bool IsPrepared() const override;
	protected:
		virtual void ExecThreadCall(const std::function<void(sql::Result)> &errorHandle,const std::function<void(sql::BaseConnection&)> &callback) const override;
		virtual sql::Result AsyncBindDouble(int32_t column,double d) override;
		virtual sql::Result AsyncBindInt(int32_t column,int32_t i) override;
		virtual sql::Result AsyncBindInt64(int32_t column,int64_t i) override;
		virtual sql::Result AsyncBindNull(int32_t column) override;
		virtual sql::Result AsyncBindText(int32_t column,const std::string &text) override;
		virtual sql::Result AsyncBindBlob(int32_t column,DataStream &ds) override;
		virtual sql::Result AsyncClearBindings() override;
	private:
		virtual sql::Result Reset() override;
		sql::Result AsyncBindValue(const std::function<int32_t(void)> &bindFunc);
	};
};

#endif
