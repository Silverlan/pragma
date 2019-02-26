#include "lsql_value.hpp"

void Lua::sql::Value::GetType(lua_State *l,::sql::PValue &value) {Lua::PushInt(l,umath::to_integral(value->GetType()));}
void Lua::sql::Value::GetDouble(lua_State *l,::sql::PValue &value) {Lua::PushNumber(l,value->GetDouble());}
void Lua::sql::Value::GetInt(lua_State *l,::sql::PValue &value) {Lua::PushInt(l,value->GetInt());}
void Lua::sql::Value::GetInt64(lua_State *l,::sql::PValue &value) {Lua::PushInt(l,value->GetInt64());}
void Lua::sql::Value::GetText(lua_State *l,::sql::PValue &value) {Lua::PushString(l,value->GetText());}
void Lua::sql::Value::GetBlob(lua_State *l,::sql::PValue &value) {Lua::Push<DataStream>(l,value->GetBlob());}
void Lua::sql::Value::ToString(lua_State *l,::sql::PValue &value) {Lua::PushString(l,value->ToString());}
