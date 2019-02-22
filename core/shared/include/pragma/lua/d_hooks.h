#define lua_callhook(nwstate,hookname,numargs,numresults,pushargs,getresults) \
	if(nwstate != NULL) \
	{ \
		std::unordered_map<std::string,int> *hooks; \
		if(nwstate->GetHooks(hookname,&hooks)) \
		{ \
			std::unordered_map<std::string,int>::iterator ithook; \
			for(ithook=hooks->begin();ithook!=hooks->end();ithook++) \
			{ \
				lua_State *m_lua = nwstate->GetLuaState(); \
				lua_rawgeti(m_lua,LUA_REGISTRYINDEX,ithook->second); \
				pushargs; \
				int err = lua_pcall(m_lua,numargs,numresults,0); \
				if(err == 0) \
					getresults; \
				else \
					lua_err(m_lua,err); \
			} \
		} \
	}

#define lua_callhook_sh(hookname,numargs,numresults,pushargs,getresults) \
	lua_callhook(GetClientState(),hookname,numargs,numresults,pushargs,getresults); \
	lua_callhook(GetServerState(),hookname,numargs,numresults,pushargs,getresults);

#define lua_callhook_0(nwstate,hookname) \
	lua_callhook(nwstate,hookname,0,0,,)

#define lua_callhook_sh_0(hookname) \
	lua_callhook_0(GetClientState(),hookname); \
	lua_callhook_0(GetServerState(),hookname);
