#ifndef __SQLITE_BLOB_HPP__
#define __SQLITE_BLOB_HPP__

#include <cinttypes>
#include <sharedutils/datastream.h>
#include <functional>

class sqlite3_blob;
namespace sqlite
{
	class Connection;
	class Blob
		: public std::enable_shared_from_this<Blob>
	{
	private:
		sqlite3_blob *m_pBlob = nullptr;
		std::weak_ptr<Connection> m_con;
	public:
		Blob(sqlite3_blob *pBlob,Connection &con);
		~Blob();
		int32_t Close();
		int32_t GetBytes() const;
		void Read(int32_t size,int32_t offset,const std::function<void(int32_t,DataStream)> &callback);
		DataStream Read(int32_t size,int32_t offset,int32_t &result);
		void Write(DataStream &ds,int32_t size,int32_t offset,const std::function<void(int32_t)> &callback);
		int32_t Write(DataStream &ds,int32_t size,int32_t offset);
		int32_t Reopen(int64_t rowId);
	};
};

#endif
