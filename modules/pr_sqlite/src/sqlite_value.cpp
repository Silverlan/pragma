#include "sqlite_value.hpp"
#include <sqlite3.h>

sqlite::Value::Value(sqlite3_value *pValue)
	: m_pValue(pValue)
{}

int32_t sqlite::Value::GetType() const {return sqlite3_value_type(m_pValue);}
int32_t sqlite::Value::GetNumericType() const {return sqlite3_value_numeric_type(m_pValue);}
int32_t sqlite::Value::GetBytes() const {return sqlite3_value_bytes(m_pValue);}
double sqlite::Value::GetDouble() const {return sqlite3_value_double(m_pValue);}
int32_t sqlite::Value::GetInt() const {return sqlite3_value_int(m_pValue);}
int64_t sqlite::Value::GetInt64() const {return sqlite3_value_int64(m_pValue);}
std::string sqlite::Value::GetText() const
{
	auto *text = sqlite3_value_text(m_pValue);
	if(text == nullptr)
		return "";
	return reinterpret_cast<const char*>(text);
}
DataStream sqlite::Value::GetBlob() const
{
	auto size = sqlite3_value_bytes(m_pValue);
	auto *data = sqlite3_value_blob(m_pValue);
	DataStream r(size);
	r->Write(reinterpret_cast<const uint8_t*>(data),size);
	r->SetOffset(0);
	return r;
}
