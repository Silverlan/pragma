#include "stdafx_engine.h"
#include "pragma/lua/lua_error_handling.hpp"
#include "pragma/lua/luaapi.h"
#include <pragma/console/convars.h>
#include "pragma/console/cvar.h"
#include "pragma/console/debugconsole.h"
#include "pragma/lua/lua_doc.hpp"
#include "pragma/console/util_console_color.hpp"
#include <pragma/debug/debug_lua_zerobrane.hpp>
#include <sharedutils/util.h>
#include <sharedutils/util_file.h>
#include <stack>

extern DLLENGINE Engine *engine;

static void print_lua_error_message(lua_State *l,const std::stringstream &ssMsg)
{
	auto colorMode = Lua::GetErrorColorMode(l);
#ifdef _WIN32
	switch(colorMode)
	{
		case Lua::ErrorColorMode::White:
			Con::attr(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
			break;
		case Lua::ErrorColorMode::Cyan:
			Con::attr(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
			break;
		case Lua::ErrorColorMode::Magenta:
			Con::attr(FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
			break;
	}
#else
	switch(colorMode)
	{
		case Lua::ErrorColorMode::White:
			std::cout<<"\033[37;1m";
			break;
		case Lua::ErrorColorMode::Cyan:
			std::cout<<"\033[36;1m";
			break;
		case Lua::ErrorColorMode::Magenta:
			std::cout<<"\033[35;1m";
			break;
	}
#endif
	Con::cout<<ssMsg.str()<<Con::endl;
}

static auto cvOpenEditorOnError = GetConVar("lua_open_editor_on_error");
static bool print_code_snippet(std::stringstream &ssMsg,std::string fname,uint32_t lineId,const std::string &prefix="")
{
	std::string ext;
	if(ufile::get_extension(fname,&ext) == true && ext == "clua")
	{
		//ssMsg<<"[PRECOMPILED - NO DEBUG INFORMATION AVAILABLE]";
		return false;
	}
	fname = FileManager::GetNormalizedPath(fname);
	auto br = fname.find_first_of(FileManager::GetDirectorySeparator());
	while(br != std::string::npos && fname.substr(0,br) != "lua")
	{
		fname = fname.substr(br +1);
		br = fname.find_first_of(FileManager::GetDirectorySeparator());
	}
	auto f = FileManager::OpenFile(fname.c_str(),"r");
	if(f != nullptr)
	{
		// Attempt to open file in ZeroBrane
		if(cvOpenEditorOnError->GetBool())
		{
			struct ZBFileInfo
			{
				ZBFileInfo(const std::string &fileName,uint32_t lineId)
					: fileName(fileName),lineId(lineId),
					tp(std::chrono::high_resolution_clock::now())
				{}
				std::string fileName;
				uint32_t lineId;
				std::chrono::high_resolution_clock::time_point tp;
			};
			static std::vector<ZBFileInfo> zbFileInfo;
			static std::chrono::high_resolution_clock::time_point tLastFileOpened {};
			auto t = std::chrono::high_resolution_clock::now();
			if(std::chrono::duration_cast<std::chrono::seconds>(t -tLastFileOpened).count() > 5)
			{
				for(auto it=zbFileInfo.begin();it!=zbFileInfo.end();)
				{
					auto &info = *it;
					if(std::chrono::duration_cast<std::chrono::seconds>(t -info.tp).count() > 90)
						it = zbFileInfo.erase(it);
					else
						++it;
				}
				auto it = std::find_if(zbFileInfo.begin(),zbFileInfo.end(),[&fname,lineId](const ZBFileInfo &info) {
					return ustring::compare(info.fileName,fname,false) && info.lineId == lineId;
				});
				if(it == zbFileInfo.end())
				{
					if(it != zbFileInfo.end())
						zbFileInfo.erase(it);
					zbFileInfo.push_back({fname,lineId});

					tLastFileOpened = t;
					std::string fullLocalPath;
					if(FileManager::FindLocalPath(fname,fullLocalPath) == false)
						fullLocalPath = fname;
					debug::open_file_in_zerobrane(util::get_program_path() +'/' +fullLocalPath,lineId);
				}
			}
			//auto hWndForeground = GetForegroundWindow();
			//auto hWndConsole = GetConsoleWindow();
			//if(hWndForeground == hWndConsole)
		}

		char c = 0;
		uint32_t curLineId = 1;
		auto linePrint = static_cast<uint32_t>(umath::max(static_cast<int32_t>(lineId) -2,static_cast<int32_t>(1)));
		auto lastLinePrint = lineId +2;
		while(!f->Eof() && curLineId < linePrint)
		{
			f->Read(&c,1);
			if(c == '\n')
				++curLineId;
		}
		if(!f->Eof() && curLineId == linePrint)
		{
			ssMsg<<prefix;
			for(auto i=linePrint;i<=lastLinePrint;++i)
			{
				auto l = f->ReadLine();
				if(i == lineId)
					ssMsg<<"\n  > ";
				else
					ssMsg<<"\n    ";
				ssMsg<<l;
				if(f->Eof())
					break;
			}
			return true;
		}
	}
	return false;
}

const auto maxLuaPathLen = 120u;
static void strip_path_until_lua_dir(std::string &shortSrc)
{
	auto c = FileManager::GetDirectorySeparator();
	auto br = shortSrc.find(c);
	uint32_t offset = 0;
	auto bFound = false;
	auto luaPath = std::string("lua") +c;
	while(br != std::string::npos && shortSrc.length() >= offset +luaPath.length() && (bFound = ustring::compare(shortSrc.data() +offset,luaPath.c_str(),false,luaPath.length())) == false)
	{
		offset = br +1;
		br = shortSrc.find(c,br +1);
	}
	if(bFound == false)
		return;
	shortSrc = ustring::sub(shortSrc,offset +luaPath.length());
	if(shortSrc.length() > maxLuaPathLen)
		shortSrc = "..." +shortSrc.substr(shortSrc.size() -maxLuaPathLen);
}

static void transform_path(const lua_Debug &d,std::string &errPath)
{
	auto start = errPath.find("[string \"");
	if(start == std::string::npos)
		return;
	auto end = errPath.find_first_of(']',start);
	if(end == std::string::npos)
		return;
	auto qt0 = errPath.find_first_of('\"',start);
	auto qt1 = errPath.find_first_of('\"',qt0 +1);
	if(qt0 < end && qt1 < end)
	{
		auto path = FileManager::GetCanonicalizedPath(d.source);

		// Remove program path from absolute path
		auto programPath = FileManager::GetCanonicalizedPath(util::get_program_path());
		ustring::to_lower(programPath);
		auto lpath = path;
		ustring::to_lower(lpath);
		if(ustring::substr(lpath,0,programPath.length()) == programPath)
			path = path.substr(programPath.length() +1);
		strip_path_until_lua_dir(path);
		if(path.length() > maxLuaPathLen)
			path = "..." +path.substr(path.size() -maxLuaPathLen);
		errPath = ustring::substr(errPath,0,qt0 +1) +path +ustring::substr(errPath,qt1);
	}
}

int Lua::HandleTracebackError(lua_State *l)
{
	if(!Lua::IsString(l,-1))
		return 1;
	std::stringstream ssMsg;
	lua_Debug d {};
	int32_t level = 1;
	auto bFoundSrc = false;
	auto bNl = false;
	while(bFoundSrc == false && lua_getstack(l,level,&d) == 1)
	{
		if(lua_getinfo(l,"Sln",&d) != 0 && (strcmp(d.what,"Lua") == 0 || strcmp(d.what,"main") == 0))
		{
			bFoundSrc = true;
			break;
		}
		++level;
	}
	std::string errMsg = Lua::ToString(l,-1);

	ssMsg<<"[LUA] ";
	if(bFoundSrc == true)
	{
		if(!errMsg.empty() && errMsg.front() != '[')
		{
			std::string shortSrc = d.source;
			auto c = FileManager::GetDirectorySeparator();
			auto br = shortSrc.find(c);
			uint32_t offset = 0;
			auto bFound = false;
			auto luaPath = std::string("lua") +c;
			while(br != std::string::npos && shortSrc.length() >= offset +luaPath.length() && (bFound = ustring::compare(shortSrc.data() +offset,luaPath.c_str(),false,luaPath.length())) == false)
			{
				offset = br +1;
				br = shortSrc.find(c,br +1);
			}
			if(bFound == true)
			{
				shortSrc = ustring::sub(shortSrc,offset);
				if(shortSrc.length() > maxLuaPathLen)
					shortSrc = "..." +shortSrc.substr(shortSrc.size() -maxLuaPathLen);
				shortSrc = "[string \"" +shortSrc +"\"]";
			}
			std::stringstream ssErrMsg;
			ssErrMsg<<shortSrc<<":"<<d.currentline<<": "<<errMsg;
			errMsg = ssErrMsg.str();
		}
		transform_path(d,errMsg);
		ssMsg<<errMsg;
		bNl = print_code_snippet(ssMsg,d.source,d.currentline,":");
	}
	else
		ssMsg<<errMsg;
	if(level != 1)
	{
		level = 1;
		if(bNl == true)
			ssMsg<<"\n\n";
		else
			ssMsg<<":\n";
		ssMsg<<"    Callstack:";
		while(lua_getstack(l,level,&d) == 1)
		{
			if(lua_getinfo(l,"Sln",&d) != 0)
			{
				std::string t(level *4,' ');
				if(level >= 10)
				{
					ssMsg<<t<<"...";
					break;
				}
				else
					ssMsg<<"\n"<<t<<level<<": "<<(d.name != nullptr ? d.name : "?")<<"["<<d.linedefined<<":"<<d.lastlinedefined<<"] ["<<d.what<<":"<<d.namewhat<<"] : "<<d.short_src<<":"<<d.currentline;
			}
			++level;
		}
	}
	print_lua_error_message(l,ssMsg);
	Lua::doc::print_documentation("ents:ApplyViewRotationOffset(0.5)"); // TODO
	util::reset_console_color();
	Con::cout<<"You can use the console command 'lua_help <name>' to get more information about a specific function/library/etc."<<Con::endl;
	Con::cout<<Con::endl;
	return 1;
}

static void handle_syntax_error(lua_State *l,Lua::StatusCode r,const std::string *fileName)
{
	if(r != Lua::StatusCode::ErrorSyntax && r != Lua::StatusCode::ErrorFile)
		return;
	if(Lua::IsString(l,-1))
	{
		std::string err = Lua::ToString(l,-1);
		//transform_path(err);
		std::stringstream ssMsg;
		ssMsg<<"[LUA] ";
		auto brSt = err.find('[');
		auto brEn = err.find(']',brSt +1);
		auto bErrPrinted = false;
		if(brSt != std::string::npos && brEn != std::string::npos)
		{
			auto qSt = err.find('\"',brSt);
			auto qEn = err.find('\"',qSt +1);
			if(qSt != std::string::npos && qEn != std::string::npos)
			{
				std::string errWithFullPath = {};
				if(fileName != nullptr)
					errWithFullPath = err.substr(0,qSt +1u) +ustring::sub(*fileName,4) +err.substr(qEn);
				else
					errWithFullPath = err;
				auto f = (fileName != nullptr) ? *fileName : err.substr(qSt +1,qEn -qSt -1);
				auto cSt = err.find(':',brEn +1);
				auto cEn = err.find(':',cSt +1);
				if(cSt != std::string::npos && cEn != std::string::npos)
				{
					auto lineId = util::to_int(err.substr(cSt +1,cEn -cSt -1));
					ssMsg<<errWithFullPath;
					bErrPrinted = true;
					print_code_snippet(ssMsg,f,lineId,":");
				}
			}
		}
		if(bErrPrinted == false)
			ssMsg<<err;
		print_lua_error_message(l,ssMsg);
	}
}
void Lua::HandleSyntaxError(lua_State *l,Lua::StatusCode r,const std::string &fileName)
{
	handle_syntax_error(l,r,&fileName);
}

void Lua::HandleSyntaxError(lua_State *l,Lua::StatusCode r)
{
	handle_syntax_error(l,r,nullptr);
}

void Lua::initialize_error_handler()
{
	luabind::set_pcall_callback([](lua_State *l) -> void {
		std::stringstream ssMsg;
		std::string luaMsg = Lua::ToString(l,-1);

		lua_Debug d {};
		int32_t level = 1;
		auto bFoundSrc = false;
		while(bFoundSrc == false && lua_getstack(l,level,&d) == 1)
		{
			if(lua_getinfo(l,"Sln",&d) != 0 && (strcmp(d.what,"Lua") == 0 || strcmp(d.what,"main") == 0))
			{
				bFoundSrc = true;
				break;
			}
			++level;
		}

		ssMsg<<"[LUA] ";
		if(bFoundSrc == true)
		{
			if(!luaMsg.empty() && luaMsg.front() != '[')
			{
				std::string shortSrc = d.source;
				strip_path_until_lua_dir(shortSrc);
				shortSrc = "[string \"" +shortSrc +"\"]";

				std::stringstream ssErrMsg;
				ssErrMsg<<shortSrc<<":"<<d.currentline<<": "<<luaMsg;
				luaMsg = ssErrMsg.str();
			}
		}
		transform_path(d,luaMsg);
		ssMsg<<luaMsg;
		auto bNl = print_code_snippet(ssMsg,(d.source != nullptr) ? d.source : "",d.currentline,":");
		if(level != 1)
		{
			level = 1;
			if(bNl == true)
				ssMsg<<"\n\n";
			else
				ssMsg<<":\n";
			ssMsg<<"    Callstack:";
			while(lua_getstack(l,level,&d) == 1)
			{
				if(lua_getinfo(l,"Sln",&d) != 0)
				{
					std::string t(level *4,' ');
					if(level >= 10)
					{
						ssMsg<<t<<"...";
						break;
					}
					else
					{
						std::string src = d.source;
						strip_path_until_lua_dir(src);
						transform_path(d,src);
						src = "[string \"" +src +"\"]";
						ssMsg<<"\n"<<t<<level<<": "<<(d.name != nullptr ? d.name : "?")<<"["<<d.linedefined<<":"<<d.lastlinedefined<<"] ["<<d.what<<":"<<d.namewhat<<"] : "<<src<<":"<<d.currentline;
					}
				}
				++level;
			}
		}
		print_lua_error_message(l,ssMsg);
	});
}

