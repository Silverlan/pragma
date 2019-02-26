#include "sqlite_blob.hpp"
#include "sqlite_connection.hpp"
#include <sqlite3.h>

sqlite::Blob::Blob(sqlite3_blob *pBlob,Connection &con)
	: m_pBlob(pBlob),m_con(con.shared_from_this())
{}
sqlite::Blob::~Blob() {Close();}
int32_t sqlite::Blob::Close()
{
	if(m_pBlob == nullptr)
		return SQLITE_OK;
	auto r = sqlite3_blob_close(m_pBlob);
	m_pBlob = nullptr;
	return r;
}
int32_t sqlite::Blob::GetBytes() const {return sqlite3_blob_bytes(m_pBlob);}
void sqlite::Blob::Read(int32_t size,int32_t offset,const std::function<void(int32_t,DataStream)> &callback)
{
	if(m_con.expired() == true)
	{
		callback(SQLITE_ERROR,nullptr);
		return;
	}
	auto con = m_con;
	std::weak_ptr<Blob> pBlob = shared_from_this();
	m_con.lock()->ScheduleThreadCall([this,con,size,offset,callback,pBlob]() {
		if(pBlob.expired() == true)
		{
			auto pCon = con.lock();
			if(pCon != nullptr)
				pCon->SchedulePollCall([callback]() {callback(SQLITE_MISUSE,nullptr);});
			return;
		}
		int32_t r;
		auto ds = Read(size,offset,r);
		auto pCon = con.lock();
		if(pCon != nullptr)
			pCon->SchedulePollCall([r,ds,callback]() {callback(r,ds);});
	});
}
DataStream sqlite::Blob::Read(int32_t size,int32_t offset,int32_t &result)
{
	DataStream ds(size);
	auto r = sqlite3_blob_read(m_pBlob,ds->GetData(),size,offset);
	if(r != SQLITE_OK)
		return nullptr;
	return ds;
}
void sqlite::Blob::Write(DataStream &ds,int32_t size,int32_t offset,const std::function<void(int32_t)> &callback)
{
	if(m_con.expired() == true)
	{
		callback(SQLITE_ERROR);
		return;
	}
	auto con = m_con;
	std::weak_ptr<Blob> pBlob = shared_from_this();
	m_con.lock()->ScheduleThreadCall([this,ds,size,offset,callback,pBlob,con]() {
		if(pBlob.expired() == true)
		{
			auto pCon = con.lock();
			if(pCon != nullptr)
				pCon->SchedulePollCall([callback]() {callback(SQLITE_MISUSE);});
			return;
		}
		auto r = Write(const_cast<DataStream&>(ds),size,offset);
		auto pCon = con.lock();
		if(pCon != nullptr)
			pCon->SchedulePollCall([callback,r]() {callback(r);});
	});
}
int32_t sqlite::Blob::Write(DataStream &ds,int32_t size,int32_t offset) {return sqlite3_blob_write(m_pBlob,ds->GetData(),size,offset);}
int32_t sqlite::Blob::Reopen(int64_t rowId) {return sqlite3_blob_reopen(m_pBlob,rowId);}
