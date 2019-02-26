#ifndef __SQLITE_CONNECTION_HPP__
#define __SQLITE_CONNECTION_HPP__

#include <cinttypes>
#include <string>
#include <memory>
#include <functional>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>

class sqlite3;
class sqlite3_context;
class sqlite3_value;
namespace sqlite
{
	class Statement;
	class Blob;
	class Connection
		: public std::enable_shared_from_this<Connection>
	{
	private:
		mutable std::shared_ptr<void> m_userData;
		sqlite3 *m_pSql = nullptr;
		std::unique_ptr<std::thread> m_thread = nullptr;
		std::atomic<bool> m_bEndThread = false;

		std::queue<std::function<void(void)>> m_threadQueue;
		std::mutex m_queueMutex;

		std::queue<std::function<void(void)>> m_pollQueue;
		std::mutex m_pollMutex;

		void InitializeThread();
	public:
		static std::string GetResultMessage(int32_t code);
		Connection(sqlite3 *pSql);
		~Connection();
		void ScheduleThreadCall(const std::function<void(void)> &f);
		void SchedulePollCall(const std::function<void(void)> &f);
		void Poll();
		void SetUserData(const std::shared_ptr<void> &userData);
		std::shared_ptr<void> GetUserData() const;

		int32_t Close();
		std::shared_ptr<Statement> PrepareStatement(const std::string &statement,int32_t &result);
		void PrepareStatement(const std::string &statement,const std::function<void(int32_t,std::shared_ptr<Statement>)> &callback);
		int32_t Exec(const std::string &sql,const std::function<int32_t(int32_t,char**,char**)> &callback=nullptr);
		int32_t GetResult() const;
		std::string GetResultMessage() const;
		bool IsDatabaseReadOnly(const std::string &dbName) const;
		bool DatabaseExists(const std::string &dbName) const;
		int64_t GetLastInsertRowId() const;

		std::shared_ptr<Blob> OpenBlob(const std::string &tableName,const std::string columnName,int64_t rowId,bool bReadOnly,int32_t &result);
		std::shared_ptr<Blob> OpenBlob(const std::string &dbName,const std::string &tableName,const std::string columnName,int64_t rowId,bool bReadOnly,int32_t &result);

		int32_t CreateFunction(const std::string &funcName,int32_t argCount,bool bDeterministic,const std::function<void(sqlite3_context*,int,sqlite3_value**)> &xFunc);
		int32_t CreateFunction(const std::string &funcName,int32_t argCount,bool bDeterministic,const std::function<void(sqlite3_context*,int,sqlite3_value**)> &xStep,const std::function<void(sqlite3_context*)> &xFinal);
		int32_t DeleteFunction(const std::string &funcName,int32_t argCount,bool bDeterministic);
	};
};

#endif
