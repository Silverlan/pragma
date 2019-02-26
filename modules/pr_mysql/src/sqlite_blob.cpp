#include "sqlite_blob.hpp"
#include "sqlite_connection.hpp"
#include <sqlite3.h>

sqlite::Blob::Blob(sqlite3_blob *pBlob,sql::BaseConnection &con)
	: m_pBlob(pBlob),m_con(con.shared_from_this())
{}
sqlite::Blob::~Blob() {Close();}
sql::Result sqlite::Blob::Close()
{
	if(m_pBlob == nullptr)
		return sql::Result::Ok;
	auto r = sqlite3_blob_close(m_pBlob);
	m_pBlob = nullptr;
	return static_cast<sql::Result>(r);
}
int32_t sqlite::Blob::GetBytes() const {return sqlite3_blob_bytes(m_pBlob);}
void sqlite::Blob::Read(int32_t size,int32_t offset,const std::function<void(sql::Result,DataStream)> &callback)
{
	if(m_con.expired() == true)
	{
		callback(sql::Result::Error,nullptr);
		return;
	}
	auto con = m_con;
	std::weak_ptr<Blob> pBlob = shared_from_this();
	m_con.lock()->ScheduleThreadCall([this,con,size,offset,callback,pBlob]() {
		if(pBlob.expired() == true)
		{
			auto pCon = con.lock();
			if(pCon != nullptr)
				pCon->SchedulePollCall([callback]() {callback(sql::Result::Misuse,nullptr);});
			return;
		}
		sql::Result r;
		auto ds = Read(size,offset,r);
		auto pCon = con.lock();
		if(pCon != nullptr)
			pCon->SchedulePollCall([r,ds,callback]() {callback(r,ds);});
	});
}
DataStream sqlite::Blob::Read(int32_t size,int32_t offset,sql::Result &result)
{
	DataStream ds(size);
	result = static_cast<sql::Result>(sqlite3_blob_read(m_pBlob,ds->GetData(),size,offset));
	if(result != sql::Result::Ok)
		return nullptr;
	return ds;
}
void sqlite::Blob::Write(DataStream &ds,int32_t size,int32_t offset,const std::function<void(sql::Result)> &callback)
{
	if(m_con.expired() == true)
	{
		callback(sql::Result::Error);
		return;
	}
	auto con = m_con;
	std::weak_ptr<Blob> pBlob = shared_from_this();
	m_con.lock()->ScheduleThreadCall([this,ds,size,offset,callback,pBlob,con]() {
		if(pBlob.expired() == true)
		{
			auto pCon = con.lock();
			if(pCon != nullptr)
				pCon->SchedulePollCall([callback]() {callback(sql::Result::Misuse);});
			return;
		}
		auto r = Write(const_cast<DataStream&>(ds),size,offset);
		auto pCon = con.lock();
		if(pCon != nullptr)
			pCon->SchedulePollCall([callback,r]() {callback(r);});
	});
}
sql::Result sqlite::Blob::Write(DataStream &ds,int32_t size,int32_t offset) {return static_cast<sql::Result>(sqlite3_blob_write(m_pBlob,ds->GetData(),size,offset));}
sql::Result sqlite::Blob::Reopen(int64_t rowId) {return static_cast<sql::Result>(sqlite3_blob_reopen(m_pBlob,rowId));}
