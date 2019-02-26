#ifndef __SQL_VALUE_HPP__
#define __SQL_VALUE_HPP__

#include <cinttypes>
#include <string>
#include <sharedutils/datastream.h>
#include <memory>
#include "sql_type.hpp"

namespace sql
{
	class Value
	{
	public:
		Value(long double v);
		Value(int32_t v);
		Value(int64_t v);
		Value(const std::string &v);
		Value(const DataStream &v);
		Value(nullptr_t);
		~Value()=default;

		sql::Type GetType() const;
		long double GetDouble() const;
		int32_t GetInt() const;
		int64_t GetInt64() const;
		const std::string &GetText() const;
		const DataStream &GetBlob() const;
		std::string ToString() const;

	protected:
		Value()=default;
	private:
		std::shared_ptr<void> m_data = nullptr;
		sql::Type m_type = sql::Type::Initial;
	};
	using PValue = std::shared_ptr<Value>;
};

#endif
