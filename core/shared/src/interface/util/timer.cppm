// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/lua/ldefinitions.h"
#include <sharedutils/functioncallback.h>

export module pragma.shared:util.timer;

export import :game.enums;
export import :util.timer_handle;

export {
	class Game;
	class DLLNETWORK Timer {
	public:
		Timer(float delay, unsigned int reps, LuaFunctionObject luaFunction, TimerType timetype = TimerType::CurTime);
		Timer(float delay, unsigned int reps, const CallbackHandle &hCallback, TimerType timetype = TimerType::CurTime);
		~Timer();
		void Update(Game *game);
		void Start(Game *game);
		void Pause();
		void Stop();
		void Remove(Game *game);
		bool IsValid();
		bool IsRunning();
		bool IsPaused();
		void InvalidateHandle(TimerHandle *hTimer);
		float GetTimeLeft();
		void SetTimeInterval(float time);
		float GetTimeInterval();
		unsigned int GetRepetitionsLeft();
		void SetRepetitions(unsigned int rep);
		std::shared_ptr<TimerHandle> CreateHandle();
		void SetCall(Game *game, LuaFunctionObject luaFunction);
		void SetCall(Game *game, const CallbackHandle &hCallback);

		void Call(Game *game);
	private:
		TimerType m_timeType;
		float m_delay;
		unsigned int m_reps;
		LuaFunctionObject m_luaFunction {};
		CallbackHandle m_callback;
		double m_start;
		bool m_bRemove;
		bool m_bRunning;
		bool m_bIsValid;
		std::vector<std::shared_ptr<TimerHandle>> m_handles;

		double GetCurTime(Game *game);
		double GetDeltaTime(Game *game);
	protected:
		float m_next;
		virtual void Reset();
		Timer();
	};
};
