#ifndef __SQLITE_CONNECTION_HPP__
#define __SQLITE_CONNECTION_HPP__

#include "sql_connection.hpp"

struct sqlite3;
struct sqlite3_context;
struct sqlite3_value;
namespace sqlite
{
	class Statement;
	class Blob;
	class Connection
		: public sql::BaseConnection
	{
	public:
		Connection();
		virtual ~Connection() override;

		sql::Result Connect(const std::string &name,int32_t *flags=nullptr);
		void Connect(const std::string &name,const std::function<void(sql::Result)> &callback,int32_t *flags=nullptr);
		virtual bool IsOpen() const override;
		bool DatabaseExists(const std::string &dbName,std::function<void(sql::Result,bool)> &callback);
		void IsDatabaseReadOnly(const std::string &dbName,std::function<void(sql::Result,bool)> &callback);

		std::string ResultToString(sql::Result result) const;
		virtual void GetLastInsertRowId(const std::function<void(sql::Result,int64_t)> &callback) override;
		virtual void CreateStatement(const std::string &sql,const std::function<void(sql::Result,std::shared_ptr<sql::BaseStatement>)> &callback) override;
		virtual void CreatePreparedStatement(const std::string &sql,const std::function<void(sql::Result,std::shared_ptr<sql::BasePreparedStatement>)> &callback) override;

		std::shared_ptr<Blob> OpenBlob(const std::string &tableName,const std::string columnName,int64_t rowId,bool bReadOnly,int32_t &result);
		std::shared_ptr<Blob> OpenBlob(const std::string &dbName,const std::string &tableName,const std::string columnName,int64_t rowId,bool bReadOnly,int32_t &result);

		int32_t CreateFunction(const std::string &funcName,int32_t argCount,bool bDeterministic,const std::function<void(sqlite3_context*,int,sqlite3_value**)> &xFunc);
		int32_t CreateFunction(const std::string &funcName,int32_t argCount,bool bDeterministic,const std::function<void(sqlite3_context*,int,sqlite3_value**)> &xStep,const std::function<void(sqlite3_context*)> &xFinal);
		int32_t DeleteFunction(const std::string &funcName,int32_t argCount,bool bDeterministic);
	protected:
		virtual sql::Result AsyncCreateStatement(const std::string &sql,std::shared_ptr<sql::BaseStatement> &statement) override;
		virtual sql::Result AsyncCreatePreparedStatement(const std::string &sql,std::shared_ptr<sql::BasePreparedStatement> &statement) override;
		virtual sql::Result CloseConnection() override;
		sql::Result UpdateLastResult(sql::Result r,bool bTranslateStepCodes=true);
		sql::Result UpdateLastResult(bool bTranslateStepCodes=true);
		friend Statement;
	private:
		sqlite3 *m_pSql = nullptr;
	};
};

#endif
