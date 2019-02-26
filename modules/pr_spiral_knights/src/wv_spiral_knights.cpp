#include "wv_spiral_knights.hpp"
#include <pragma/ishared.hpp>
#include <luainterface.hpp>
#include <luasystem.h>
#include <sharedutils/util.h>
#include <fsys/filesystem.h>
#include <pragma/pragma_module.hpp>

#pragma comment(lib,"lua51.lib")
#pragma comment(lib,"luasystem.lib")
#pragma comment(lib,"luabind.lib")
#pragma comment(lib,"sharedutils.lib")
#pragma comment(lib,"mathutil.lib")
#pragma comment(lib,"vfilesystem.lib")
#pragma comment(lib,"shared.lib")
#pragma comment(lib,"ishared.lib")

void Lua::spiral_knights::register_lua_library(Lua::Interface &l)
{
	auto &modConvert = l.RegisterLibrary("convert",std::unordered_map<std::string,int32_t(*)(lua_State*)>{
		{"spiral_knights_binary_to_xml",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) {
			std::string mdlPath = FileManager::GetCanonicalizedPath(Lua::CheckString(l,1));
			if(ishared::validate_file_write_operation(l,mdlPath) == false)
			{
				Lua::PushBool(l,false);
				return 1;
			}
			auto cwd = util::get_program_path() +"\\addons\\spiral_knights\\";
			auto cmd = "java -jar \"" +cwd +"modules\\sk_binary_to_xml.jar\" \"" +mdlPath +"\"";

			auto exitCode = 0u;
			auto bSuccess = util::start_and_wait_for_command(cmd.c_str(),nullptr,&exitCode);

			Lua::PushBool(l,bSuccess == true && exitCode == 0u);
			return 1;
		})}
	});
}

extern "C"
{
	void PRAGMA_EXPORT pragma_initialize_lua(Lua::Interface &l)
	{
		Lua::spiral_knights::register_lua_library(l);
	}
};
