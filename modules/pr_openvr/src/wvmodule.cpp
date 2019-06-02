#include "stdafx_openvr.h"
#include "wvmodule.h"
#include <Windows.h>
#include "lopenvr.h"
#include "vr_instance.hpp"
#include <sharedutils/functioncallback.h>
#include <luainterface.hpp>
#include <pragma/pragma_module.hpp>
#ifdef _DEBUG
#include <iostream>
#endif

extern std::unique_ptr<::openvr::Instance> s_vrInstance;

class Game;
class CGame;
extern "C"
{
	void PRAGMA_EXPORT pragma_initialize_lua(Lua::Interface &l)
	{
		if(l.GetIdentifier() != "cl")
			return;
		Lua::openvr::register_lua_library(l);
	}
	void PRAGMA_EXPORT pragma_terminate_lua(Lua::Interface &l)
	{
		if(l.GetIdentifier() != "cl")
			return;
		if(s_vrInstance != nullptr)
			s_vrInstance->ClearScene();
		Lua::openvr::lib::close(nullptr);
	}
	/*
	void PRAGMA_EXPORT InitializeLua(Lua::Interface &l)
	{
		if(l.GetIdentifier() != "cl")
			return;
		if(IState::is_game_active() == true)
			Lua::openvr::register_lua_library(l.GetState());
		cbGameStarted = IState::add_callback(IState::Callback::OnGameStart,FunctionCallback<void,CGame*>::Create([](CGame *game) {
			if(cbGameInitialized.IsValid())
				cbGameInitialized.Remove();
			cbGameInitialized = IState::add_callback(IState::Callback::OnGameInitialized,FunctionCallback<void,Game*>::Create([](Game *game) {
				if(s_vrInstance != nullptr)
					s_vrInstance->InitializeScene();
			}));

			if(cbLuaInitialized.IsValid())
				cbLuaInitialized.Remove();
			cbLuaInitialized = IState::add_callback(IState::Callback::OnLuaInitialized,FunctionCallback<void,lua_State*>::Create([](lua_State *l) {
				Lua::openvr::register_lua_library(l);
			}));
		}));
		cbGameEnd = IState::add_callback(IState::Callback::EndGame,FunctionCallback<void,CGame*>::Create([](CGame *game) {
			if(s_vrInstance != nullptr)
				s_vrInstance->ClearScene();
			clear_callbacks();
		}));
	}*/
};
