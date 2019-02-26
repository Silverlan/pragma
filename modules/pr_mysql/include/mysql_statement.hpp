#ifndef __MYSQL_STATEMENT_HPP__
#define __MYSQL_STATEMENT_HPP__

#include "sql_statement.hpp"

namespace sql {class Statement; class ResultSet; class PreparedStatement; class SQLException;};
namespace mysql
{
	class Connection;
	class Statement
		: public sql::BaseStatement
	{
	public:
		Statement(sql::Statement *pStmt,sql::BaseConnection &con);
		// Statement will be destroyed on thread!
		virtual ~Statement() override;
		
		virtual bool IsMySQL() const override;
		virtual bool IsSQLite() const override;
		virtual bool IsPrepared() const override;

		void SetQuery(const std::string &query);
	protected:
		friend Connection;

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
		
		virtual void ExecThreadCall(const std::function<void(sql::Result)> &errorHandle,const std::function<void(sql::BaseConnection&)> &callback) const override;
		virtual void Reset();
		sql::Result UpdateLastResult(const sql::SQLException &e) const;
		sql::Result UpdateLastResult() const;
	protected:
		std::shared_ptr<sql::Statement> m_pStmt = nullptr;
		std::shared_ptr<sql::ResultSet> m_result = nullptr;
		std::string m_query;
	};

	class PreparedStatement
		: public Statement,
		public sql::BasePreparedStatement
	{
	public:
		PreparedStatement(sql::PreparedStatement *pStmt,sql::BaseConnection &con);
		virtual bool IsPrepared() const override;
	protected:
		sql::Result AsyncBindValue(const std::function<void(void)> &bindFunc);

		virtual void ExecThreadCall(const std::function<void(sql::Result)> &errorHandle,const std::function<void(sql::BaseConnection&)> &callback) const override;
		virtual sql::Result AsyncBindDouble(int32_t column,double d) override;
		virtual sql::Result AsyncBindInt(int32_t column,int32_t i) override;
		virtual sql::Result AsyncBindInt64(int32_t column,int64_t i) override;
		virtual sql::Result AsyncBindNull(int32_t column) override;
		virtual sql::Result AsyncBindText(int32_t column,const std::string &text) override;
		virtual sql::Result AsyncBindBlob(int32_t column,DataStream &ds) override;
		virtual sql::Result AsyncClearBindings() override;
		virtual sql::Result AsyncExecuteQuery() override;
		virtual sql::Result AsyncExecute() override;
	private:
		sql::PreparedStatement &m_pStmtPrep;

		virtual void Reset() override;
	};
};

#endif
