#ifndef __MYSQL_CONNECTION_HPP__
#define __MYSQL_CONNECTION_HPP__

#include <cinttypes>
#include <string>
#include <memory>
#include <functional>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include "sql_connection.hpp"
#include "sql_result.hpp"

namespace sql {class SQLException;};
namespace mysql
{
	class Blob;
	class Statement;
	class Connection
		: public sql::BaseConnection
	{
	public:
		Connection();
		virtual ~Connection() override;

		void IsDatabaseReadOnly(const std::function<void(sql::Result,bool)> &callback);
		sql::Result Connect(const std::string &ip,const std::string &user,const std::string &password,const std::string &dbName);
		void Connect(const std::string &ip,const std::string &user,const std::string &password,const std::string &dbName,const std::function<void(sql::Result)> &callback);
		virtual bool IsOpen() const override;

		virtual void CreateStatement(const std::string &sql,const std::function<void(sql::Result,std::shared_ptr<sql::BaseStatement>)> &callback) override;
		virtual void CreatePreparedStatement(const std::string &sql,const std::function<void(sql::Result,std::shared_ptr<sql::BasePreparedStatement>)> &callback) override;

		virtual void GetLastInsertRowId(const std::function<void(sql::Result,int64_t)> &callback) override;

		//std::shared_ptr<Blob> OpenBlob(const std::string &tableName,const std::string columnName,int64_t rowId,bool bReadOnly,int32_t &result);
		//std::shared_ptr<Blob> OpenBlob(const std::string &dbName,const std::string &tableName,const std::string columnName,int64_t rowId,bool bReadOnly,int32_t &result);
	protected:
		virtual sql::Result CloseConnection() override;
		virtual sql::Result AsyncCreateStatement(const std::string &sql,std::shared_ptr<sql::BaseStatement> &statement) override;
		virtual sql::Result AsyncCreatePreparedStatement(const std::string &sql,std::shared_ptr<sql::BasePreparedStatement> &statement) override;
		sql::Result UpdateLastResult(const sql::SQLException &e);
		sql::Result UpdateLastResult();
		friend Statement;
	private:
		std::unique_ptr<sql::Connection> m_pSql = nullptr;
	};
};

#endif
