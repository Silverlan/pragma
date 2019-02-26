#include "sql_connection.hpp"
#include "sql_statement.hpp"

sql::BaseConnection::~BaseConnection() {}
sql::Result sql::BaseConnection::Close()
{
	if(m_thread != nullptr)
	{
		m_bEndThread = true;
		m_thread->join();
		m_thread = nullptr;
	}

	// We can safely release statements now, since thread is already complete
	while(m_deleteStatements.empty() == false)
		m_deleteStatements.pop();

	if(IsOpen() == false)
		return Result::Ok;
	return CloseConnection();
}
sql::Result sql::BaseConnection::GetResultCode() const {return m_lastResult;}
const std::string &sql::BaseConnection::GetResultErrorMessage() const {return m_lastMessage;}
void sql::BaseConnection::SetLastResult(sql::Result result,const std::string &msg)
{
	m_lastResultMutex.lock();
		m_asLastResult = result;
		m_asLastMessage = msg;
	m_lastResultMutex.unlock();
}

void sql::BaseConnection::CallOnQueueComplete(const std::function<void(void)> &callback)
{
	ScheduleThreadCall([this,callback]() {
		SchedulePollCall(callback);
	});
}

void sql::BaseConnection::InitializeThread()
{
	if(m_thread != nullptr)
		return;
	m_thread = std::make_unique<std::thread>([this]() {
		while(m_bEndThread == false)
		{
			m_statementMutex.lock();
				// Statements have to be released on thread;
				// Release all statements scheduled for deletion here
				while(m_deleteStatements.empty() == false)
					m_deleteStatements.pop();
			m_statementMutex.unlock();
			m_queueMutex.lock();
				while(m_threadQueue.empty() == false && m_bEndThread == false)
				{
					m_threadQueue.front()();
					m_threadQueue.pop();
				}
			m_bWaitForCompletion = false;
			m_queueMutex.unlock();
		}
		m_bWaitForCompletion = false;
	});
}
void sql::BaseConnection::SchedulePollCall(const std::function<void(void)> &f)
{
	m_pollMutex.lock();
		m_pollQueue.push(f);
	m_pollMutex.unlock();
}
void sql::BaseConnection::Poll()
{
	m_pollMutex.lock();
		m_lastResultMutex.lock();
			m_lastResult = m_asLastResult;
			m_lastMessage = m_asLastMessage;
		m_lastResultMutex.unlock();
		while(m_pollQueue.empty() == false)
		{
			m_pollQueue.front()();
			m_pollQueue.pop();
		}
	m_pollMutex.unlock();
}
void sql::BaseConnection::Wait()
{
	if(m_thread != nullptr)
	{
		m_bWaitForCompletion = true;
		while(m_bWaitForCompletion == true && m_bEndThread == false);
	}
	Poll();
}
std::function<void(void*)> sql::BaseConnection::GetStatementDeleter(const std::function<void(void*)> &fDelete)
{
	std::weak_ptr<::sql::BaseConnection> wpConnection = shared_from_this();
	return [this,wpConnection,fDelete](void *statement) {
		if(wpConnection.expired() == true)
		{
			fDelete(statement);
			return;
		}
		m_deleteStatements.push(std::shared_ptr<void>(statement,fDelete));
	};
}
void sql::BaseConnection::Execute(const std::string &sql,const std::function<void(sql::Result)> &callback)
{
	ScheduleThreadCall([this,sql,callback]() {
		auto rows = std::make_shared<std::vector<std::vector<PValue>>>();
		auto r = AsyncExecute(sql);
		SchedulePollCall([callback,r]() mutable {
			callback(r);
		});
	});
}
void sql::BaseConnection::Execute(const std::string &sql,const std::shared_ptr<std::vector<sql::PValue>> &values,const std::function<void(sql::Result)> &callback)
{
	ScheduleThreadCall([this,sql,callback,values]() {
		auto rows = std::make_shared<std::vector<std::vector<PValue>>>();
		auto r = AsyncExecute(sql,*values);
		SchedulePollCall([callback,r]() mutable {
			callback(r);
		});
	});
}
void sql::BaseConnection::ExecuteQueryAndFetchRows(const std::string &sql,const std::function<void(sql::Result,std::shared_ptr<QueryData>&)> &callback)
{
	ScheduleThreadCall([this,sql,callback]() {
		auto queryData = std::make_shared<QueryData>();
		auto r = AsyncExecuteQueryAndFetchRows(sql,*queryData);
		SchedulePollCall([callback,r,queryData]() mutable {
			callback(r,queryData);
		});
	});
}
void sql::BaseConnection::ExecuteQueryAndFetchRows(const std::string &sql,const std::shared_ptr<std::vector<sql::PValue>> &values,const std::function<void(sql::Result,std::shared_ptr<QueryData>&)> &callback)
{
	ScheduleThreadCall([this,sql,callback,values]() {
		auto queryData = std::make_shared<QueryData>();
		auto r = AsyncExecuteQueryAndFetchRows(sql,*values,*queryData);
		SchedulePollCall([callback,r,queryData]() mutable {
			callback(r,queryData);
		});
	});
}
sql::Result sql::BaseConnection::AsyncExecute(const std::string &sql)
{
	auto statement = std::shared_ptr<sql::BaseStatement>{nullptr};
	auto r = AsyncCreateStatement(sql,statement);
	if(r != sql::Result::Ok)
		return r;
	r = statement->AsyncExecute();
	return r;
}
sql::Result sql::BaseConnection::AsyncExecute(const std::string &sql,const std::vector<sql::PValue> &values)
{
	auto statement = std::shared_ptr<sql::BasePreparedStatement>{nullptr};
	auto r = AsyncCreatePreparedStatement(sql,statement);
	if(r != sql::Result::Ok)
		return r;
	int32_t colIdx = 1;
	for(auto &val : values)
	{
		r = statement->AsyncBindValue(colIdx++,val);
		if(r != sql::Result::Ok)
			return r;
	}
	r = dynamic_cast<sql::BaseStatement*>(statement.get())->AsyncExecute();
	return r;
}
sql::Result sql::BaseConnection::AsyncExecuteQueryAndFetchRows(const std::string &sql,QueryData &queryData)
{
	auto statement = std::shared_ptr<sql::BaseStatement>{nullptr};
	auto r = AsyncCreateStatement(sql,statement);
	if(r != sql::Result::Ok)
		return r;
	r = statement->AsyncExecuteQueryAndFetchRows(queryData);
	return r;
}
sql::Result sql::BaseConnection::AsyncExecuteQueryAndFetchRows(const std::string &sql,const std::vector<sql::PValue> &values,QueryData &queryData)
{
	auto statement = std::shared_ptr<sql::BasePreparedStatement>{nullptr};
	auto r = AsyncCreatePreparedStatement(sql,statement);
	if(r != sql::Result::Ok)
		return r;
	int32_t colIdx = 1;
	for(auto &val : values)
	{
		r = statement->AsyncBindValue(colIdx++,val);
		if(r != sql::Result::Ok)
			return r;
	}
	r = dynamic_cast<sql::BaseStatement*>(statement.get())->AsyncExecuteQueryAndFetchRows(queryData);
	return r;
}
void sql::BaseConnection::ScheduleThreadCall(const std::function<void(void)> &f)
{
	InitializeThread();
	m_queueMutex.lock();
		m_threadQueue.push(f);
	m_queueMutex.unlock();
}

