#include "stdafx_openvr.h"
#include "wvmodule.h"
#include <Windows.h>
#include "lopenvr.h"
#include "vr_instance.hpp"
#include <sharedutils/functioncallback.h>
#include <luainterface.hpp>
#include <pragma_module.hpp>
#ifdef _DEBUG
#include <iostream>
#endif

#pragma comment(lib,"lua51.lib")
#pragma comment(lib,"luasystem.lib")
#pragma comment(lib,"openvr_api.lib")
#pragma comment(lib,"luabind.lib")
#pragma comment(lib,"vfilesystem.lib")
#pragma comment(lib,"sharedutils.lib")
#pragma comment(lib,"mathutil.lib")
#pragma comment(lib,"prosper.lib")
#pragma comment(lib,"anvil.lib")
#pragma comment(lib,"vulkan-1.lib")

#ifdef USE_OPENGL_OFFSCREEN_CONTEXT
#pragma comment(lib,"glfw3dll.lib")
#ifdef _DEBUG
#pragma comment(lib,"glew64sd.lib")
#else
#pragma comment(lib,"glew64s.lib")
#endif
#pragma comment(lib,"opengl32.lib")
//#pragma comment(lib,"glu32.lib")
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
