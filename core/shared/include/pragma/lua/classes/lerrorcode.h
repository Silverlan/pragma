#ifndef __LERRORCODE_H__
#define __LERRORCODE_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
#include <system_error>

#undef GetMessage

class DLLNETWORK ErrorCode
{
protected:
	std::string m_message;
	int32_t m_value;
	std::unique_ptr<std::error_code> m_error;
public:
	ErrorCode();
	ErrorCode(const std::string &msg,int32_t value);
	ErrorCode(const std::error_code &err);
	ErrorCode(const ErrorCode &other);
	bool IsError() const;
	std::string GetMessage() const;
	int32_t GetValue() const;
};

DLLNETWORK std::ostream &operator<<(std::ostream &out,const ErrorCode &err);

//////////////////////////////

DLLNETWORK void Lua_ErrorCode_GetMessage(lua_State *l,ErrorCode &err);
DLLNETWORK void Lua_ErrorCode_GetValue(lua_State *l,ErrorCode &err);
DLLNETWORK void Lua_ErrorCode_IsError(lua_State *l,ErrorCode &err);

#endif