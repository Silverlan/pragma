#ifndef __SQL_CONNECTION_HPP__
#define __SQL_CONNECTION_HPP__

#include <cinttypes>
#include <string>
#include <memory>
#include <functional>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include "sql_result.hpp"
#include "sql_value.hpp"

namespace sql
{
	enum class ConnectionType : uint32_t
	{
		MySQL = 0u,
		SQLite
	};

	struct QueryData
	{
		std::vector<std::string> columnNames;
		std::vector<std::vector<PValue>> rows;
	};

	class Connection;
	class BaseStatement;
	class BasePreparedStatement;
	class BaseConnection
		: public std::enable_shared_from_this<BaseConnection>
	{
	public:
		virtual ~BaseConnection();
		virtual bool IsOpen() const=0;

		// Close() has to be called in destructors of derived classes!
		Result Close();

		sql::Result GetResultCode() const;
		const std::string &GetResultErrorMessage() const;

		void ScheduleThreadCall(const std::function<void(void)> &f);
		void SchedulePollCall(const std::function<void(void)> &f);
		void Poll();
		void Wait();
		void CallOnQueueComplete(const std::function<void(void)> &callback);

		void Execute(const std::string &sql,const std::function<void(sql::Result)> &callback);
		void Execute(const std::string &sql,const std::shared_ptr<std::vector<sql::PValue>> &values,const std::function<void(sql::Result)> &callback);
		void ExecuteQueryAndFetchRows(const std::string &sql,const std::function<void(sql::Result,std::shared_ptr<QueryData>&)> &callback);
		void ExecuteQueryAndFetchRows(const std::string &sql,const std::shared_ptr<std::vector<sql::PValue>> &values,const std::function<void(sql::Result,std::shared_ptr<QueryData>&)> &callback);

		// Asynchronous
		virtual void CreateStatement(const std::string &sql,const std::function<void(sql::Result,std::shared_ptr<sql::BaseStatement>)> &callback)=0;
		virtual void CreatePreparedStatement(const std::string &sql,const std::function<void(sql::Result,std::shared_ptr<sql::BasePreparedStatement>)> &callback)=0;
		virtual void GetLastInsertRowId(const std::function<void(sql::Result,int64_t)> &callback)=0;
	protected:
		BaseConnection()=default;
		virtual Result CloseConnection()=0;
		void SetLastResult(sql::Result result,const std::string &msg);

		virtual sql::Result AsyncCreateStatement(const std::string &sql,std::shared_ptr<sql::BaseStatement> &statement)=0;
		virtual sql::Result AsyncCreatePreparedStatement(const std::string &sql,std::shared_ptr<sql::BasePreparedStatement> &statement)=0;
		sql::Result AsyncExecute(const std::string &sql);
		sql::Result AsyncExecute(const std::string &sql,const std::vector<sql::PValue> &values);
		sql::Result AsyncExecuteQueryAndFetchRows(const std::string &sql,QueryData &queryData);
		sql::Result AsyncExecuteQueryAndFetchRows(const std::string &sql,const std::vector<sql::PValue> &values,QueryData &rows);

		std::function<void(void*)> GetStatementDeleter(const std::function<void(void*)> &fDelete);
	private:
		std::mutex m_statementMutex;
		std::queue<std::shared_ptr<void>> m_deleteStatements = {};

		std::unique_ptr<std::thread> m_thread = nullptr;
		std::atomic<bool> m_bEndThread = false;
		std::atomic<bool> m_bWaitForCompletion = false;

		std::queue<std::function<void(void)>> m_threadQueue;
		std::mutex m_queueMutex;

		std::queue<std::function<void(void)>> m_pollQueue;
		std::mutex m_pollMutex;

		void InitializeThread();

		sql::Result m_lastResult = sql::Result::Ok;
		std::string m_lastMessage = {};

		// On thread
		std::mutex m_lastResultMutex;
		sql::Result m_asLastResult = sql::Result::Ok;
		std::string m_asLastMessage = {};
	};
};

#endif
