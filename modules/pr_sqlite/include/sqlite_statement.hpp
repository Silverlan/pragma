#ifndef __SQLITE_STATEMENT_HPP__
#define __SQLITE_STATEMENT_HPP__

#include <cinttypes>
#include <string>
#include <memory>
#include <functional>
#include <sharedutils/datastream.h>

class sqlite3_stmt;
namespace sqlite
{
	class Connection;
	class Statement
		: public std::enable_shared_from_this<Statement>
	{
	private:
		sqlite3_stmt *m_pStmt = nullptr;
		std::weak_ptr<Connection> m_con;
	public:
		Statement(sqlite3_stmt *pStmt,Connection &con);
		~Statement();
		int32_t Finalize();
		int32_t Step();
		void Step(const std::function<void(int32_t)> &callback);
		int32_t Reset();

		int32_t GetType(int32_t column) const;
		int32_t GetBytes(int32_t column) const;
		double GetDouble(int32_t column) const;
		int32_t GetInt(int32_t column) const;
		int64_t GetInt64(int32_t column) const;
		std::string GetText(int32_t column) const;
		DataStream GetBlob(int32_t column) const;
		void GetBlob(int32_t column,const std::function<void(DataStream)> &callback);
		int32_t GetCount() const;
		std::string GetColumnName(int32_t column) const;

		int32_t BindDouble(int32_t column,double d);
		int32_t BindInt(int32_t column,int32_t i);
		int32_t BindInt64(int32_t column,int64_t i);
		int32_t BindNull(int32_t column);
		int32_t BindText(int32_t column,const std::string &text);
		int32_t BindBlob(int32_t column,DataStream &ds);
		int32_t ClearBindings();

		std::string GetSQLText() const;
		std::string GetExpandedSQLText() const;

		bool IsReadOnly() const;
		std::string GetDatabaseName(int32_t column) const;
		std::string GetTableName(int32_t column) const;
		std::string GetOriginName(int32_t column) const;
	};
};

#endif