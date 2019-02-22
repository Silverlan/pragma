#include "stdafx_shared.h"
#include "pragma/lua/libraries/ltime.hpp"
#include "pragma/util/util_duration_type.hpp"

extern DLLENGINE Engine *engine;
int Lua::time::cur_time(lua_State *l)
{
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	Lua::PushNumber(l,game->CurTime());
	return 1;
}

int Lua::time::real_time(lua_State *l)
{
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	Lua::PushNumber(l,game->RealTime());
	return 1;
}

int Lua::time::delta_time(lua_State *l)
{
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	Lua::PushNumber(l,game->DeltaTickTime());
	return 1;
}

int Lua::time::time_since_epoch(lua_State *l)
{
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	Lua::PushInt(l,std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count());
	return 1;
}

template<typename TDurationTypeSrc>
	void convert_duration(lua_State *l,TDurationTypeSrc duration,util::DurationType durationTypeDst)
{
	switch(durationTypeDst)
	{
		case util::DurationType::NanoSeconds:
			Lua::PushInt(l,std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count());
			break;
		case util::DurationType::MicroSeconds:
			Lua::PushInt(l,std::chrono::duration_cast<std::chrono::microseconds>(duration).count());
			break;
		case util::DurationType::MilliSeconds:
			Lua::PushInt(l,std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
			break;
		case util::DurationType::Seconds:
			Lua::PushInt(l,std::chrono::duration_cast<std::chrono::seconds>(duration).count());
			break;
		case util::DurationType::Minutes:
			Lua::PushInt(l,std::chrono::duration_cast<std::chrono::minutes>(duration).count());
			break;
		case util::DurationType::Hours:
			Lua::PushInt(l,std::chrono::duration_cast<std::chrono::hours>(duration).count());
			break;
	}
	Lua::PushInt(l,duration.count());
}

int Lua::time::convert_duration(lua_State *l)
{
	auto duration = Lua::CheckInt(l,1);
	auto durationTypeSrc = static_cast<util::DurationType>(Lua::CheckInt(l,2));
	auto durationTypeDst = static_cast<util::DurationType>(Lua::CheckInt(l,3));
	switch(durationTypeSrc)
	{
		case util::DurationType::NanoSeconds:
			::convert_duration<std::chrono::nanoseconds>(l,std::chrono::nanoseconds{duration},durationTypeDst);
			break;
		case util::DurationType::MicroSeconds:
			::convert_duration<std::chrono::microseconds>(l,std::chrono::microseconds{duration},durationTypeDst);
			break;
		case util::DurationType::MilliSeconds:
			::convert_duration<std::chrono::milliseconds>(l,std::chrono::milliseconds{duration},durationTypeDst);
			break;
		case util::DurationType::Seconds:
			::convert_duration<std::chrono::seconds>(l,std::chrono::seconds{duration},durationTypeDst);
			break;
		case util::DurationType::Minutes:
			::convert_duration<std::chrono::minutes>(l,std::chrono::minutes{duration},durationTypeDst);
			break;
		case util::DurationType::Hours:
			::convert_duration<std::chrono::hours>(l,std::chrono::hours{duration},durationTypeDst);
			break;
		default:
			Lua::PushInt(l,duration);
			break;
	}
	return 1;
}
