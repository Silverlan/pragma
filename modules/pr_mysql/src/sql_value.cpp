#include "sql_value.hpp"

sql::Value::Value(long double v)
	: m_data(std::make_shared<double>(v)),
	m_type(sql::Type::Double)
{}
sql::Value::Value(int32_t v)
	: m_data(std::make_shared<int32_t>(v)),
	m_type(sql::Type::Integer)
{}
sql::Value::Value(int64_t v)
	: m_data(std::make_shared<int64_t>(v)),
	m_type(sql::Type::BigInt)
{}
sql::Value::Value(const std::string &v)
	: m_data(std::make_shared<std::string>(v)),
	m_type(sql::Type::VarChar)
{}
sql::Value::Value(const DataStream &v)
	: m_data(std::make_shared<DataStream>(v)),
	m_type(sql::Type::Binary)
{}
sql::Value::Value(nullptr_t)
	: m_data(nullptr),
	m_type(sql::Type::Null)
{}

sql::Type sql::Value::GetType() const {return m_type;}
long double sql::Value::GetDouble() const {return (m_type == sql::Type::Double) ? *static_cast<long double*>(m_data.get()) : 0.0;}
int32_t sql::Value::GetInt() const {return (m_type == sql::Type::Integer) ? *static_cast<int32_t*>(m_data.get()) : 0;}
int64_t sql::Value::GetInt64() const {return (m_type == sql::Type::BigInt) ? *static_cast<int64_t*>(m_data.get()) : 0ll;}
const std::string &sql::Value::GetText() const
{
	static std::string r = "";
	return (m_type == sql::Type::VarChar) ? *static_cast<std::string*>(m_data.get()) : r;
}
const DataStream &sql::Value::GetBlob() const
{
	static DataStream r = nullptr;
	return (m_type == sql::Type::Binary) ? *static_cast<DataStream*>(m_data.get()) : r;
}
std::string sql::Value::ToString() const
{
	switch(m_type)
	{
		case Type::Double:
			return std::to_string(GetDouble());
		case Type::Integer:
			return std::to_string(GetInt());
		case Type::BigInt:
			return std::to_string(GetInt64());
		case Type::VarChar:
			return GetText();
		case Type::Binary:
		case Type::Null:
		default:
			return "";
	}
}
