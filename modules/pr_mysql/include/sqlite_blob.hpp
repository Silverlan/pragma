#ifndef __SQLITE_BLOB_HPP__
#define __SQLITE_BLOB_HPP__

#include <cinttypes>
#include <sharedutils/datastream.h>
#include <functional>
#include "sql_result.hpp"

namespace sql {class BaseConnection;};

struct sqlite3_blob;
namespace sqlite
{
	class Blob
		: public std::enable_shared_from_this<Blob>
	{
	private:
		sqlite3_blob *m_pBlob = nullptr;
		std::weak_ptr<sql::BaseConnection> m_con;
	public:
		Blob(sqlite3_blob *pBlob,sql::BaseConnection &con);
		~Blob();
		sql::Result Close();
		int32_t GetBytes() const;
		void Read(int32_t size,int32_t offset,const std::function<void(sql::Result,DataStream)> &callback);
		DataStream Read(int32_t size,int32_t offset,sql::Result &result);
		void Write(DataStream &ds,int32_t size,int32_t offset,const std::function<void(sql::Result)> &callback);
		sql::Result Write(DataStream &ds,int32_t size,int32_t offset);
		sql::Result Reopen(int64_t rowId);
	};
};

#endif
