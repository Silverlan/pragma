// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.libraries.time;

double Lua::time::cur_time(lua::State *l)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	pragma::Game *game = state->GetGameState();
	return game->CurTime();
}

double Lua::time::real_time(lua::State *l)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	pragma::Game *game = state->GetGameState();
	return game->RealTime();
}

double Lua::time::delta_time(lua::State *l)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	pragma::Game *game = state->GetGameState();
	return game->DeltaTickTime();
}

uint64_t Lua::time::time_since_epoch(lua::State *l)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	pragma::Game *game = state->GetGameState();
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

template<typename TDurationTypeSrc>
int64_t convert_duration(TDurationTypeSrc duration, pragma::util::DurationType durationTypeDst)
{
	switch(durationTypeDst) {
	case pragma::util::DurationType::NanoSeconds:
		return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
	case pragma::util::DurationType::MicroSeconds:
		return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
	case pragma::util::DurationType::MilliSeconds:
		return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
	case pragma::util::DurationType::Seconds:
		return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
	case pragma::util::DurationType::Minutes:
		return std::chrono::duration_cast<std::chrono::minutes>(duration).count();
	case pragma::util::DurationType::Hours:
		return std::chrono::duration_cast<std::chrono::hours>(duration).count();
	}
	return 0;
}

int64_t Lua::time::convert_duration(int64_t duration, pragma::util::DurationType srcType, pragma::util::DurationType dstType)
{
	switch(srcType) {
	case pragma::util::DurationType::NanoSeconds:
		::convert_duration<std::chrono::nanoseconds>(std::chrono::nanoseconds {duration}, dstType);
		break;
	case pragma::util::DurationType::MicroSeconds:
		::convert_duration<std::chrono::microseconds>(std::chrono::microseconds {duration}, dstType);
		break;
	case pragma::util::DurationType::MilliSeconds:
		::convert_duration<std::chrono::milliseconds>(std::chrono::milliseconds {duration}, dstType);
		break;
	case pragma::util::DurationType::Seconds:
		::convert_duration<std::chrono::seconds>(std::chrono::seconds {duration}, dstType);
		break;
	case pragma::util::DurationType::Minutes:
		::convert_duration<std::chrono::minutes>(std::chrono::minutes {duration}, dstType);
		break;
	case pragma::util::DurationType::Hours:
		::convert_duration<std::chrono::hours>(std::chrono::hours {duration}, dstType);
		break;
	default:
		return duration;
	}
	return 0;
}
