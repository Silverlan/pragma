#ifdef __MYSQL_H__
#define __MYSQL_H__

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <sharedutils/functioncallback.h>
#include "util/unifiedptr.h"

class __declspec(dllexport) MySQLEvent
{
public:
	MySQLEvent()
		: eventid(-1)
	{}
	MySQLEvent(MySQLEvent&)=delete;
	MySQLEvent &operator=(const MySQLEvent&)=delete;
	int eventid;
	CallbackHandle callback;
	std::vector<std::unique_ptr<UnifiedBasePtr>> userData;
	template<class T>
		void SetUserData(size_t idx,T t);
};

template<class T>
	void MySQLEvent::SetUserData(size_t idx,T t)
{
	userData.resize(idx +1);
	userData[idx] = std::make_unique<UnifiedPtr<T>>(t);
}

class MySQLStatement;
class MySQLResult;
class MySQLPreparedStatement;
class __declspec(dllexport) MySQL
{
public:
	friend MySQLStatement;
	friend MySQLPreparedStatement;
	friend MySQLResult;
	static std::mutex MUTEX_LAST_ERROR;
	static std::mutex MUTEX_DRIVER;
	static std::mutex MUTEX_EVENT_QUEUE;
	static std::string LAST_ERROR;
	static std::queue<MySQLEvent*> EVENT_QUEUE;
	static unsigned int CONNECT_THREAD_COUNT;
	static std::mutex MUTEX_CONNECT_THREAD;
	std::queue<MySQLEvent*> CALL_QUEUE;
protected:
	bool m_bValid;
	sql::Connection *m_con;
	std::thread *m_thread;
	std::mutex *m_lockMutex;
	std::mutex *m_queueMutex;
	std::condition_variable *m_condition;
	void PushQueueEvent(MySQLEvent *ev);
public:
	MySQL(sql::Connection *con);
	~MySQL();
	MySQLPreparedStatement *PrepareStatement(std::string query);
	sql::Connection *GetConnection();
	bool IsValid();
	static void Think();
	static MySQL *Connect(std::string host,std::string user,std::string pass,std::string db,CallbackHandle callback=CallbackHandle());
	bool SetSchema(std::string db,CallbackHandle callback=CallbackHandle());
	MySQLStatement *CreateStatement();
};

class __declspec(dllexport) MySQLStatement
{
public:
	friend MySQL;
protected:
	MySQL *m_sql;
	sql::Statement *m_statement;
	MySQLStatement(MySQL *sql,sql::Statement *statement);
public:
	virtual ~MySQLStatement();
	MySQL *GetOwner();
	sql::Statement *GetStatement();
	virtual MySQLResult *ExecuteQuery(std::string query,CallbackHandle callback=CallbackHandle());
	virtual bool Execute(std::string query,CallbackHandle callback=CallbackHandle());
};

class __declspec(dllexport) MySQLPreparedStatement
	: public MySQLStatement
{
public:
	friend MySQL;
protected:
	MySQLPreparedStatement(MySQL *sql,sql::PreparedStatement *smt);
public:
	virtual ~MySQLPreparedStatement();
	void SetBigInt(unsigned int pIdx,std::string i);
	void SetBoolean(unsigned int pIdx,bool b);
	void SetDouble(unsigned int pIdx,double d);
	void SetInt(unsigned int pIdx,int i);
	void SetInt64(unsigned int pIdx,int64_t i);
	void SetString(unsigned int pIdx,std::string s);
	void SetUInt(unsigned int pIdx,unsigned int i);
	void SetUInt64(unsigned int pIdx,uint64_t i);
	virtual MySQLResult *ExecuteQuery(CallbackHandle callback=CallbackHandle());
	virtual bool Execute(CallbackHandle callback=CallbackHandle());
};

class __declspec(dllexport) MySQLResult
{
public:
	friend MySQLStatement;
protected:
	MySQL *m_sql;
	sql::ResultSet *m_result;
	template<class T>
		bool GetValue(T *r,T(sql::ResultSet::*f)(const sql::SQLString&) const,std::string &col);
	template<class T>
		bool GetValue(T *r,T(sql::ResultSet::*f)(uint32_t) const,uint32_t col);
public:
	MySQLResult(MySQL *sql,sql::ResultSet *res);
	~MySQLResult();
	MySQL *GetOwner();
	bool FetchNext();
	bool GetBoolean(bool *r,std::string &col);
	bool GetBoolean(bool *r,uint32_t col);
	bool GetDouble(long double *r,std::string &col);
	bool GetDouble(long double *r,uint32_t col);
	bool GetInt(int *r,std::string &col);
	bool GetInt(int *r,uint32_t col);
	bool GetInt64(int64_t *r,std::string &col);
	bool GetInt64(int64_t *r,uint32_t col);
	bool GetString(std::string *r,std::string &col);
	bool GetString(std::string *r,uint32_t col);
	bool GetUInt(unsigned int *r,std::string &col);
	bool GetUInt(unsigned int *r,uint32_t col);
	bool GetUInt64(uint64_t *r,std::string &col);
	bool GetUInt64(uint64_t *r,uint32_t col);
};

template<class T>
	bool MySQLResult::GetValue(T *r,T(sql::ResultSet::*f)(const sql::SQLString&) const,std::string &col)
{
	try {
		*r = (m_result->*f)(col);
	}
	catch(sql::SQLException &e)
	{
		MySQL::LAST_ERROR = e.what();
		return false;
	}
	return true;
}
template<class T>
	bool MySQLResult::GetValue(T *r,T(sql::ResultSet::*f)(uint32_t) const,uint32_t col)
{
	try {
		*r = (m_result->*f)(col);
	}
	catch(sql::SQLException &e)
	{
		MySQL::LAST_ERROR = e.what();
		return false;
	}
	return true;
}

#endif