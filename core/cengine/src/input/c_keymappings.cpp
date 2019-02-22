#include "stdafx_cengine.h"
#include "pragma/c_engine.h"
#include "pragma/input/inputhelper.h"
#include <pragma/clientstate/clientstate.h>

void CEngine::MapKey(short c,std::string cmd)
{
	UnmapKey(c);
	if(cmd.empty())
		return;
	m_keyMappings.insert(std::unordered_map<short,KeyBind>::value_type(c,KeyBind(cmd)));
}

void CEngine::MapKey(short c,int function)
{
	UnmapKey(c);
	m_keyMappings.insert(std::unordered_map<short,KeyBind>::value_type(c,KeyBind(function)));
}

void CEngine::MapKey(short c,std::unordered_map<std::string,std::vector<std::string>> &binds)
{
	std::stringstream cmdNew;
	for(auto it=binds.begin();it!=binds.end();++it)
	{
		cmdNew<<it->first;
		auto &argv = it->second;
		for(auto itArg = argv.begin();itArg!=argv.end();++itArg)
			cmdNew<<" "<<*itArg;
		cmdNew<<";";
	}
	MapKey(c,cmdNew.str());
}

void CEngine::AddKeyMapping(short c,std::string cmd)
{
	auto it = m_keyMappings.find(c);
	if(it == m_keyMappings.end())
	{
		MapKey(c,cmd);
		return;
	}
	auto &keyBind = it->second;
	auto &bind = keyBind.GetBind();
	std::unordered_map<std::string,std::vector<std::string>> binds;
	auto callback = [&binds](std::string cmd,std::vector<std::string> &argv) {
		auto it = binds.find(cmd);
		if(it != binds.end())
			return;
		binds.insert(std::unordered_map<std::string,std::vector<std::string>>::value_type(cmd,argv)).first;
	};
	ustring::get_sequence_commands(cmd,callback);
	ustring::get_sequence_commands(bind,callback);
	MapKey(c,binds);
}
void CEngine::RemoveKeyMapping(short c,std::string cmd)
{
	auto it = m_keyMappings.find(c);
	if(it == m_keyMappings.end())
		return;
	auto &keyBind = it->second;
	auto &bind = keyBind.GetBind();
	std::unordered_map<std::string,std::vector<std::string>> binds;
	auto callback = [&binds,&cmd](std::string cmdBind,std::vector<std::string> &argv) {
		auto it = binds.find(cmdBind);
		if(it != binds.end() || cmdBind == cmd)
			return;
		binds.insert(std::unordered_map<std::string,std::vector<std::string>>::value_type(cmdBind,argv)).first;
	};
	ustring::get_sequence_commands(bind,callback);
	MapKey(c,binds);
}

void CEngine::UnmapKey(short c)
{
	auto i = m_keyMappings.find(c);
	if(i == m_keyMappings.end())
		return;
	if(i->second.GetType() == KeyBind::Type::Function)
	{
		NetworkState *client = GetClientState();
		if(client != NULL)
		{
			Game *game = client->GetGameState();
			if(game != NULL)
			{
				lua_removereference(game->GetLuaState(),i->second.GetFunction());
			}
		}
	}
	m_keyMappings.erase(i);
}

void CEngine::ClearLuaKeyMappings()
{
	NetworkState *client = GetClientState();
	lua_State *lua = NULL;
	if(client != NULL)
	{
		Game *game = client->GetGameState();
		if(game != NULL)
			lua = game->GetLuaState();
	}
	std::unordered_map<short,KeyBind>::iterator i,j;
	for(i=m_keyMappings.begin();i!=m_keyMappings.end();)
	{
		if(i->second.GetType() == KeyBind::Type::Function)
		{
			j = i;
			++i;
			if(lua != NULL)
				lua_removereference(lua,j->second.GetFunction());
			m_keyMappings.erase(j);
		}
		else ++i;
	}
}

const std::unordered_map<short,KeyBind> &CEngine::GetKeyMappings() const {return m_keyMappings;}

void CEngine::ClearKeyMappings()
{
	NetworkState *client = GetClientState();
	lua_State *lua = NULL;
	if(client != NULL)
	{
		Game *game = client->GetGameState();
		if(game != NULL)
			lua = game->GetLuaState();
	}
	std::unordered_map<short,KeyBind>::iterator it;
	for(it=m_keyMappings.begin();it!=m_keyMappings.end();++it)
	{
		if(it->second.GetType() == KeyBind::Type::Function)
		{
			if(lua != NULL)
				lua_removereference(lua,it->second.GetFunction());
		}
	}
	m_keyMappings.clear();
}

////////////////////////////

#include "pragma/console/c_cvar_keymappings.h"
extern CEngine *c_engine;
DLLCENGINE void CMD_bind_keys(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string>&)
{
	for(int i=0;i<(sizeof(BIND_KEYS) /sizeof(BIND_KEYS[0]));i++)
		Con::cout<<BIND_KEYS[i]<<Con::endl;
}

DLLCENGINE void CMD_bind(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string> &argv)
{
	if(argv.size() <= 1) return;
	short c;
	if(!StringToKey(argv[0],&c))
	{
		Con::cout<<"\""<<argv[0]<<"\" isn't a valid key. Use 'bind_keys' to get a list of all available keys"<<Con::endl;
		return;
	}
	c_engine->MapKey(c,argv[1]);
}

DLLCENGINE void CMD_unbind(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string> &argv)
{
	if(argv.empty()) return;
	short c;
	if(!StringToKey(argv[0],&c))
	{
		Con::cout<<"\""<<argv[0]<<"\" isn't a valid key. Use 'bind_keys' to get a list of all available keys"<<Con::endl;
		return;
	}
	c_engine->UnmapKey(c);
}

DLLCENGINE void CMD_unbindall(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string>&) {c_engine->ClearKeyMappings();}

DLLCENGINE void CMD_keymappings(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string>&)
{
	auto &mappings = c_engine->GetKeyMappings();
	std::string key;
	for(auto &pair : mappings)
	{
		if(KeyToString(pair.first,&key))
		{
			Con::cout<<key<<": ";
			if(pair.second.GetType() == KeyBind::Type::Regular)
				Con::cout<<"\""<<pair.second.GetBind()<<"\"";
			else Con::cout<<"function";
			Con::cout<<Con::endl;
		}
	}
}
