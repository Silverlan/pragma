/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/lua/libraries/ltime.hpp"
#include "pragma/util/util_duration_type.hpp"

#pragma optimize("",off)
extern DLLENGINE Engine *engine;
double Lua::time::cur_time(lua_State *l)
{
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	return game->CurTime();
}

double Lua::time::real_time(lua_State *l)
{
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	return game->RealTime();
}

double Lua::time::delta_time(lua_State *l)
{
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	return game->DeltaTickTime();
}

uint64_t Lua::time::time_since_epoch(lua_State *l)
{
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

template<typename TDurationTypeSrc>
	int64_t convert_duration(TDurationTypeSrc duration,util::DurationType durationTypeDst)
{
	switch(durationTypeDst)
	{
		case util::DurationType::NanoSeconds:
			return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
		case util::DurationType::MicroSeconds:
			return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
		case util::DurationType::MilliSeconds:
			return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
		case util::DurationType::Seconds:
			return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
		case util::DurationType::Minutes:
			return std::chrono::duration_cast<std::chrono::minutes>(duration).count();
		case util::DurationType::Hours:
			return std::chrono::duration_cast<std::chrono::hours>(duration).count();
	}
	return 0;
}

int64_t Lua::time::convert_duration(int64_t duration,util::DurationType srcType,util::DurationType dstType)
{
	switch(srcType)
	{
		case util::DurationType::NanoSeconds:
			::convert_duration<std::chrono::nanoseconds>(std::chrono::nanoseconds{duration},dstType);
			break;
		case util::DurationType::MicroSeconds:
			::convert_duration<std::chrono::microseconds>(std::chrono::microseconds{duration},dstType);
			break;
		case util::DurationType::MilliSeconds:
			::convert_duration<std::chrono::milliseconds>(std::chrono::milliseconds{duration},dstType);
			break;
		case util::DurationType::Seconds:
			::convert_duration<std::chrono::seconds>(std::chrono::seconds{duration},dstType);
			break;
		case util::DurationType::Minutes:
			::convert_duration<std::chrono::minutes>(std::chrono::minutes{duration},dstType);
			break;
		case util::DurationType::Hours:
			::convert_duration<std::chrono::hours>(std::chrono::hours{duration},dstType);
			break;
		default:
			return duration;
	}
	return 0;
}
#pragma optimize("",on)
