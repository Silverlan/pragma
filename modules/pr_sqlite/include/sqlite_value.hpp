#ifndef __SQLITE_VALUE_HPP__
#define __SQLITE_VALUE_HPP__

#include <cinttypes>
#include <string>
#include <sharedutils/datastream.h>

class sqlite3_value;
namespace sqlite
{
	class Value
	{
	private:
		sqlite3_value *m_pValue = nullptr;
	public:
		Value(sqlite3_value *pValue);

		int32_t GetType() const;
		int32_t GetNumericType() const;
		int32_t GetBytes() const;
		double GetDouble() const;
		int32_t GetInt() const;
		int64_t GetInt64() const;
		std::string GetText() const;
		DataStream GetBlob() const;
	};
};

#endif
