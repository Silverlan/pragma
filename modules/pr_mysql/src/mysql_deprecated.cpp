#if 0
#include "wvmodule.h"
#include "mysql.h"
#include <sharedutils/functioncallback.h>

#define SQL_EVENT_CONNECT 0
#define SQL_EVENT_SETSCHEMA 1
#define SQL_EVENT_EXECUTE_QUERY 2
#define SQL_EVENT_EXECUTE 3
#define SQL_EVENT_EXECUTE_QUERY_PREPARED 4
#define SQL_EVENT_EXECUTE_PREPARED 5

#define SQL_EXCEPTION \
	MySQL::MUTEX_LAST_ERROR.lock(); \
		MySQL::LAST_ERROR = e.what(); \
	MySQL::MUTEX_LAST_ERROR.unlock();

extern sql::Driver *sql_driver;

std::mutex MySQL::MUTEX_LAST_ERROR;
std::mutex MySQL::MUTEX_DRIVER;
std::mutex MySQL::MUTEX_EVENT_QUEUE;
std::string MySQL::LAST_ERROR = "";
std::queue<MySQLEvent*> MySQL::EVENT_QUEUE;

std::mutex MySQL::MUTEX_CONNECT_THREAD;
unsigned int MySQL::CONNECT_THREAD_COUNT = 0;

static bool MySQL_SetSchema(MySQL *sql,std::string db)
{
	sql::Connection *con = sql->GetConnection();
	try {
		con->setSchema(db);
	}
	catch(sql::SQLException &e)
	{
		MySQL::LAST_ERROR = e.what();
		return false;
	}
	return true;
}

static MySQLResult *MySQL_ExecuteQuery(MySQLStatement *statement,std::string query)
{
	sql::ResultSet *res;
	try {
		sql::Statement *sqlStatement = statement->GetStatement();
		res = sqlStatement->executeQuery(query);
	}
	catch(sql::SQLException &e)
	{
		MySQL::LAST_ERROR = e.what();
		return NULL;
	}
	return new MySQLResult(statement->GetOwner(),res);
}

static bool MySQL_Execute(MySQLStatement *statement,std::string query)
{
	bool b;
	try {
		sql::Statement *sqlStatement = statement->GetStatement();
		b = sqlStatement->execute(query);
	}
	catch(sql::SQLException &e)
	{
		MySQL::LAST_ERROR = e.what();
		return false;
	}
	return true;
}

static MySQLResult *MySQL_ExecuteQuery(MySQLPreparedStatement *statement)
{
	sql::ResultSet *res;
	try {
		sql::PreparedStatement *sqlStatement = static_cast<sql::PreparedStatement*>(statement->GetStatement());
		res = sqlStatement->executeQuery();
	}
	catch(sql::SQLException &e)
	{
		MySQL::LAST_ERROR = e.what();
		return NULL;
	}
	return new MySQLResult(statement->GetOwner(),res);
}

static bool MySQL_Execute(MySQLPreparedStatement *statement)
{
	bool b;
	try {
		sql::PreparedStatement *sqlStatement = static_cast<sql::PreparedStatement*>(statement->GetStatement());
		b = sqlStatement->execute();
	}
	catch(sql::SQLException &e)
	{
		MySQL::LAST_ERROR = e.what();
		return false;
	}
	return true;
}

static bool MySQLThread_lock(MySQL *sql) {return !sql->IsValid() || !sql->CALL_QUEUE.empty();}
static void MySQLThread(MySQL *sql,std::mutex *lockMutex,std::mutex *queueMutex,std::condition_variable *var)
{
	queueMutex->lock();
		std::unique_lock<std::mutex> lock(*lockMutex);
		bool bValid = sql->IsValid();
	queueMutex->unlock();
	auto n = std::bind(&MySQLThread_lock,sql);
	while(bValid)
	{
		var->wait(lock,n);
		queueMutex->lock();
			if(!sql->CALL_QUEUE.empty())
			{
				MySQLEvent *ev = sql->CALL_QUEUE.front();
				sql->CALL_QUEUE.pop();
				switch(ev->eventid)
				{
				case SQL_EVENT_SETSCHEMA:
					{
						if(!ev->userData.empty())
						{
							auto db = ev->userData[0]->GetValue<std::string>();
							bool b = MySQL_SetSchema(sql,db);

							MySQLEvent *evCallback = new MySQLEvent;
							evCallback->eventid = ev->eventid;
							evCallback->SetUserData(0,sql);
							evCallback->SetUserData(1,b);
							evCallback->callback = ev->callback;
							MySQL::EVENT_QUEUE.push(evCallback);
						}
						break;
					}
				case SQL_EVENT_EXECUTE_QUERY:
					{
						if(ev->userData.size() >= 2)
						{
							auto *statement = ev->userData[0]->GetValue<MySQLStatement*>();
							auto query = ev->userData[1]->GetValue<std::string>();
							auto *result = MySQL_ExecuteQuery(statement,query);

							MySQLEvent *evCallback = new MySQLEvent;
							evCallback->eventid = ev->eventid;
							evCallback->SetUserData(0,sql);
							evCallback->SetUserData(1,result);
							evCallback->callback = ev->callback;
							MySQL::EVENT_QUEUE.push(evCallback);
						}
						break;
					}
				case SQL_EVENT_EXECUTE:
					{
						if(ev->userData.size() >= 2)
						{
							auto *statement = ev->userData[0]->GetValue<MySQLStatement*>();
							auto query = ev->userData[1]->GetValue<std::string>();
							auto result = MySQL_Execute(statement,query);

							MySQLEvent *evCallback = new MySQLEvent;
							evCallback->eventid = ev->eventid;
							evCallback->SetUserData(0,sql);
							evCallback->SetUserData(1,result);
							evCallback->callback = ev->callback;
							MySQL::EVENT_QUEUE.push(evCallback);
						}
						break;
					}
				case SQL_EVENT_EXECUTE_QUERY_PREPARED:
					{
						if(ev->userData.size() >= 1)
						{
							auto *statement = ev->userData[0]->GetValue<MySQLPreparedStatement*>();
							auto result = MySQL_ExecuteQuery(statement);

							MySQLEvent *evCallback = new MySQLEvent;
							evCallback->eventid = SQL_EVENT_EXECUTE_QUERY;
							evCallback->SetUserData(0,sql);
							evCallback->SetUserData(1,result);
							evCallback->callback = ev->callback;
							MySQL::EVENT_QUEUE.push(evCallback);
						}
						break;
					}
				case SQL_EVENT_EXECUTE_PREPARED:
					{
						if(ev->userData.size() >= 1)
						{
							auto *statement = ev->userData[0]->GetValue<MySQLPreparedStatement*>();
							auto result = MySQL_Execute(statement);

							MySQLEvent *evCallback = new MySQLEvent;
							evCallback->eventid = SQL_EVENT_EXECUTE;
							evCallback->SetUserData(0,sql);
							evCallback->SetUserData(1,result);
							evCallback->callback = ev->callback;
							MySQL::EVENT_QUEUE.push(evCallback);
							break;
						}
					}
				}
				delete ev;
			}
			bValid = sql->IsValid();
		queueMutex->unlock();
	}
}

MySQL::MySQL(sql::Connection *con)
	: m_con(con),m_bValid(true)
{
	m_lockMutex = new std::mutex;
	m_queueMutex = new std::mutex;
	m_condition = new std::condition_variable;
	m_thread = new std::thread(&MySQLThread,this,m_lockMutex,m_queueMutex,m_condition);
}

MySQL::~MySQL()
{
	m_queueMutex->lock();
		m_bValid = false;
	m_queueMutex->unlock();
	m_condition->notify_one();
	m_thread->join();
	delete m_thread;
	delete m_condition;
	delete m_lockMutex;
	delete m_queueMutex;
}

MySQLPreparedStatement *MySQL::PrepareStatement(std::string query) {return new MySQLPreparedStatement(this,m_con->prepareStatement(query));}

bool MySQL::IsValid() {return m_bValid;}
sql::Connection *MySQL::GetConnection() {return m_con;}

void MySQL::PushQueueEvent(MySQLEvent *ev)
{
	MySQL::CALL_QUEUE.push(ev);
	m_condition->notify_one();
}

bool MySQL::SetSchema(std::string db,CallbackHandle callback)
{
	if(m_bValid == false)
		return false;
	if(callback == nullptr)
		return MySQL_SetSchema(this,db);
	MySQLEvent *ev = new MySQLEvent;
	ev->eventid = SQL_EVENT_SETSCHEMA;
	ev->callback = callback;
	ev->SetUserData(0,db);
	PushQueueEvent(ev);
	return true;
}

MySQLStatement *MySQL::CreateStatement()
{
	if(m_bValid == false)
		return NULL;
	sql::Statement *statement;
	try {
		statement = m_con->createStatement();
	}
	catch(sql::SQLException &e)
	{
		MySQL::LAST_ERROR = e.what();
		return NULL;
	}
	return new MySQLStatement(this,statement);
}

///////////////////////

MySQLPreparedStatement::MySQLPreparedStatement(MySQL *sql,sql::PreparedStatement *smt)
	: MySQLStatement(sql,smt)
{}
MySQLPreparedStatement::~MySQLPreparedStatement()
{}

void MySQLPreparedStatement::SetBigInt(unsigned int pIdx,std::string i) {static_cast<sql::PreparedStatement*>(m_statement)->setBigInt(pIdx,i);}
void MySQLPreparedStatement::SetBoolean(unsigned int pIdx,bool b) {static_cast<sql::PreparedStatement*>(m_statement)->setBoolean(pIdx,b);}
void MySQLPreparedStatement::SetDouble(unsigned int pIdx,double d) {static_cast<sql::PreparedStatement*>(m_statement)->setDouble(pIdx,d);}
void MySQLPreparedStatement::SetInt(unsigned int pIdx,int i) {static_cast<sql::PreparedStatement*>(m_statement)->setInt(pIdx,i);}
void MySQLPreparedStatement::SetInt64(unsigned int pIdx,int64_t i) {static_cast<sql::PreparedStatement*>(m_statement)->setInt64(pIdx,i);}
void MySQLPreparedStatement::SetString(unsigned int pIdx,std::string s) {static_cast<sql::PreparedStatement*>(m_statement)->setString(pIdx,s);}
void MySQLPreparedStatement::SetUInt(unsigned int pIdx,unsigned int i) {static_cast<sql::PreparedStatement*>(m_statement)->setUInt(pIdx,i);}
void MySQLPreparedStatement::SetUInt64(unsigned int pIdx,uint64_t i) {static_cast<sql::PreparedStatement*>(m_statement)->setUInt64(pIdx,i);}
MySQLResult *MySQLPreparedStatement::ExecuteQuery(CallbackHandle callback)
{
	if(m_sql->IsValid() == false)
		return NULL;
	if(callback == nullptr)
		return MySQL_ExecuteQuery(this);
	MySQLEvent *ev = new MySQLEvent;
	ev->eventid = SQL_EVENT_EXECUTE_QUERY_PREPARED;
	ev->SetUserData(0,this);
	ev->callback = callback;
	m_sql->PushQueueEvent(ev);
	return NULL;
}
bool MySQLPreparedStatement::Execute(CallbackHandle callback)
{
	if(m_sql->IsValid() == false)
		return false;
	if(callback == nullptr)
		return MySQL_Execute(this);
	MySQLEvent *ev = new MySQLEvent;
	ev->eventid = SQL_EVENT_EXECUTE_PREPARED;
	ev->SetUserData(0,this);
	ev->callback = callback;
	m_sql->PushQueueEvent(ev);
	return false;
}

///////////////////////

MySQLStatement::MySQLStatement(MySQL *sql,sql::Statement *statement)
	: m_sql(sql),m_statement(statement)
{}

MySQLStatement::~MySQLStatement()
{
	delete m_statement;
}

sql::Statement *MySQLStatement::GetStatement() {return m_statement;}
MySQL *MySQLStatement::GetOwner() {return m_sql;}

MySQLResult *MySQLStatement::ExecuteQuery(std::string query,CallbackHandle callback)
{
	if(m_sql->IsValid() == false)
		return NULL;
	if(callback == nullptr)
		return MySQL_ExecuteQuery(this,query);
	MySQLEvent *ev = new MySQLEvent;
	ev->eventid = SQL_EVENT_EXECUTE_QUERY;
	ev->SetUserData(0,this);
	ev->SetUserData(1,query);
	ev->callback = callback;
	m_sql->PushQueueEvent(ev);
	return NULL;
}

bool MySQLStatement::Execute(std::string query,CallbackHandle callback)
{
	if(m_sql->IsValid() == false)
		return false;
	if(callback == nullptr)
		return MySQL_Execute(this,query);
	MySQLEvent *ev = new MySQLEvent;
	ev->eventid = SQL_EVENT_EXECUTE;
	ev->SetUserData(0,this);
	ev->SetUserData(1,query);
	ev->callback = callback;
	m_sql->PushQueueEvent(ev);
	return false;
}

///////////////////////

MySQLResult::MySQLResult(MySQL *sql,sql::ResultSet *res)
	: m_sql(sql),m_result(res)
{

}

MySQLResult::~MySQLResult()
{
	delete m_result;
}

bool MySQLResult::FetchNext() {return m_result->next();}
MySQL *MySQLResult::GetOwner() {return m_sql;}

static bool (sql::ResultSet::*res_getBoolean_str)(const sql::SQLString&) const = &sql::ResultSet::getBoolean;
static bool (sql::ResultSet::*res_getBoolean_int)(uint32_t) const = &sql::ResultSet::getBoolean;
static long double (sql::ResultSet::*res_getDouble_str)(const sql::SQLString&) const = &sql::ResultSet::getDouble;
static long double (sql::ResultSet::*res_getDouble_int)(uint32_t) const = &sql::ResultSet::getDouble;
static int (sql::ResultSet::*res_getInt_str)(const sql::SQLString&) const = &sql::ResultSet::getInt;
static int (sql::ResultSet::*res_getInt_int)(uint32_t) const = &sql::ResultSet::getInt;
static int64_t (sql::ResultSet::*res_getInt64_str)(const sql::SQLString&) const = &sql::ResultSet::getInt64;
static int64_t (sql::ResultSet::*res_getInt64_int)(uint32_t) const = &sql::ResultSet::getInt64;
static sql::SQLString (sql::ResultSet::*res_getString_str)(const sql::SQLString&) const = &sql::ResultSet::getString;
static sql::SQLString (sql::ResultSet::*res_getString_int)(uint32_t) const = &sql::ResultSet::getString;
static unsigned int (sql::ResultSet::*res_getUInt_str)(const sql::SQLString&) const = &sql::ResultSet::getUInt;
static unsigned int (sql::ResultSet::*res_getUInt_int)(uint32_t) const = &sql::ResultSet::getUInt;
static uint64_t (sql::ResultSet::*res_getUInt64_str)(const sql::SQLString&) const = &sql::ResultSet::getUInt64;
static uint64_t (sql::ResultSet::*res_getUInt64_int)(uint32_t) const = &sql::ResultSet::getUInt64;

bool MySQLResult::GetBoolean(bool *r,std::string &col) {return GetValue<bool>(r,res_getBoolean_str,col);}
bool MySQLResult::GetBoolean(bool *r,uint32_t col) {return GetValue<bool>(r,res_getBoolean_int,col);}
bool MySQLResult::GetDouble(long double *r,std::string &col) {return GetValue<long double>(r,res_getDouble_str,col);}
bool MySQLResult::GetDouble(long double *r,uint32_t col) {return GetValue<long double>(r,res_getDouble_int,col);}
bool MySQLResult::GetInt(int *r,std::string &col) {return GetValue<int>(r,res_getInt_str,col);}
bool MySQLResult::GetInt(int *r,uint32_t col) {return GetValue<int>(r,res_getInt_int,col);}
bool MySQLResult::GetInt64(int64_t *r,std::string &col) {return GetValue<int64_t>(r,res_getInt64_str,col);}
bool MySQLResult::GetInt64(int64_t *r,uint32_t col) {return GetValue<int64_t>(r,res_getInt64_int,col);}
bool MySQLResult::GetString(std::string *r,std::string &col)
{
	sql::SQLString sqlStr;
	bool b = GetValue<sql::SQLString>(&sqlStr,res_getString_str,col);
	*r = sqlStr;
	return b;
}
bool MySQLResult::GetString(std::string *r,uint32_t col)
{
	sql::SQLString sqlStr;
	bool b = GetValue<sql::SQLString>(&sqlStr,res_getString_int,col);
	*r = sqlStr;
	return b;
}
bool MySQLResult::GetUInt(unsigned int *r,std::string &col) {return GetValue<unsigned int>(r,res_getUInt_str,col);}
bool MySQLResult::GetUInt(unsigned int *r,uint32_t col) {return GetValue<unsigned int>(r,res_getUInt_int,col);}
bool MySQLResult::GetUInt64(uint64_t *r,std::string &col) {return GetValue<uint64_t>(r,res_getUInt64_str,col);}
bool MySQLResult::GetUInt64(uint64_t *r,uint32_t col) {return GetValue<uint64_t>(r,res_getUInt64_int,col);}

///////////////////////

void MySQL::Think()
{
	MySQL::MUTEX_EVENT_QUEUE.lock();
	while(!MySQL::EVENT_QUEUE.empty())
	{
		MySQLEvent *ev = MySQL::EVENT_QUEUE.front();
		switch(ev->eventid)
		{
		case SQL_EVENT_CONNECT:
			{
				if(ev->userData.size() >= 1)
				{
					auto *sql = ev->userData[0]->GetValue<MySQL*>();
					auto tcb = ev->callback;
					if(tcb != nullptr)
						tcb(sql);
				}
				break;
			}
		case SQL_EVENT_SETSCHEMA:
			{
				if(ev->userData.size() >= 2)
				{
					auto *sql = ev->userData[0]->GetValue<MySQL*>();
					auto tcb = ev->callback;
					auto r = ev->userData[1]->GetValue<bool>();
					if(tcb != nullptr)
						tcb(sql,r);
				}
				break;
			}
		case SQL_EVENT_EXECUTE_QUERY:
			{
				if(ev->userData.size() >= 2)
				{
					auto *sql = ev->userData[0]->GetValue<MySQL*>();
					auto tcb = ev->callback;
					auto *result = ev->userData[1]->GetValue<MySQLResult*>();
					if(tcb != nullptr)
						tcb(sql,result);
				}
				break;
			}
		case SQL_EVENT_EXECUTE:
			{
				if(ev->userData.size() >= 2)
				{
					auto *sql = ev->userData[0]->GetValue<MySQL*>();
					auto tcb = ev->callback;
					auto result = ev->userData[1]->GetValue<bool>();
					if(tcb != nullptr)
						tcb(sql,result);
				}
				break;
			}
		}
		delete ev;
		MySQL::EVENT_QUEUE.pop();
	}
	MySQL::MUTEX_EVENT_QUEUE.unlock();
}

static void MySQLConnect(std::string host,std::string user,std::string pass,std::string db,CallbackHandle cb,MySQL **result)
{
	MySQL::MUTEX_CONNECT_THREAD.lock();
		MySQL::CONNECT_THREAD_COUNT++;
	MySQL::MUTEX_CONNECT_THREAD.unlock();
	sql::Connection *con;
	MySQL::MUTEX_DRIVER.lock();
	try {
		con = sql_driver->connect(host,user,pass);
		con->setSchema(db);
	}
	catch(sql::SQLException &e)
	{
		MySQL::MUTEX_DRIVER.unlock();
		SQL_EXCEPTION;
		if(cb == nullptr)
			*result = NULL;
		else
		{
			MySQLEvent *ev = new MySQLEvent;
			ev->eventid = 0;
			ev->SetUserData<MySQL*>(0,nullptr);
			ev->callback = cb;
			MySQL::MUTEX_EVENT_QUEUE.lock();
				MySQL::EVENT_QUEUE.push(ev);
			MySQL::MUTEX_EVENT_QUEUE.unlock();
		}
		return;
	}
	MySQL::MUTEX_DRIVER.unlock();
	if(cb == nullptr)
		*result = new MySQL(con);
	else
	{
		MySQLEvent *ev = new MySQLEvent;
		ev->eventid = 0;
		ev->SetUserData(0,new MySQL(con));
		ev->callback = cb;
		MySQL::MUTEX_EVENT_QUEUE.lock();
			MySQL::EVENT_QUEUE.push(ev);
		MySQL::MUTEX_EVENT_QUEUE.unlock();
	}
	MySQL::MUTEX_CONNECT_THREAD.lock();
		MySQL::CONNECT_THREAD_COUNT--;
	MySQL::MUTEX_CONNECT_THREAD.unlock();
}

MySQL *MySQL::Connect(std::string host,std::string user,std::string pass,std::string db,CallbackHandle callback)
{
	if(callback != nullptr)
	{
		std::thread t(&MySQLConnect,host,user,pass,db,callback,(MySQL**)(NULL));
		t.detach();
		return NULL;
	}
	MySQL *sql = NULL;
	MySQLConnect(host,user,pass,db,CallbackHandle(),&sql);
	return sql;
}

#endif