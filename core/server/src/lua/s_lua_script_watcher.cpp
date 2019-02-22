#include "stdafx_server.h"
#include "pragma/lua/s_lua_script_watcher.h"
#include "pragma/networking/resourcemanager.h"

extern DLLSERVER ServerState *server;
extern DLLSERVER SGame *s_game;

void SLuaDirectoryWatcherManager::OnLuaFileChanged(const std::string &path)
{
	if(IsLuaFile(path) == false)
		return;
	LuaDirectoryWatcherManager::OnLuaFileChanged(path);

	auto *res = ResourceManager::FindResource("lua\\" +path.substr(0,path.length() -3) +"clua");
	if(res == nullptr)
		return;
	s_game->UpdateLuaCache(res->fileName);
	server->SendResourceFile(res->fileName);
}
